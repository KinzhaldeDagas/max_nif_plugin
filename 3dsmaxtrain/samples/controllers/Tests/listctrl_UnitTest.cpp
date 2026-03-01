#include "MatrixMath.h"
#ifndef DISABLE_UNIT_TESTS
#include "ctrl.h"
#include "istdplug.h"
#include "control.h"
#include "iparamb2.h"
#include <vector>
#include <Tests/DefaultMockCoreInterface.h>
#include <../core/coreInterface.h>
#include "control.h"

#include <gtest/gtest.h>

namespace {
using namespace MaxSDK;

constexpr float epsilon = 0.0001f;

void CompareAffineParts(const AffineParts expected, const AffineParts result, bool compareRotations = true)
{
	if(!expected.t.Equals(result.t, epsilon))
	{
		EXPECT_NEAR(expected.t.x, result.t.x, epsilon);
		EXPECT_NEAR(expected.t.y, result.t.y, epsilon);
		EXPECT_NEAR(expected.t.z, result.t.z, epsilon);
	}
	if(compareRotations && !expected.q.Equals(result.q, epsilon))
	{
		EXPECT_NEAR(expected.q.x, result.q.x, epsilon);
		EXPECT_NEAR(expected.q.y, result.q.y, epsilon);
		EXPECT_NEAR(expected.q.z, result.q.z, epsilon);
		EXPECT_NEAR(expected.q.w, result.q.w, epsilon);
	}
	if(!expected.k.Equals(result.k, epsilon))
	{
		EXPECT_NEAR(expected.k.x, result.k.x, epsilon);
		EXPECT_NEAR(expected.k.y, result.k.y, epsilon);
		EXPECT_NEAR(expected.k.z, result.k.z, epsilon);
	}
	EXPECT_NEAR(expected.f, result.f, epsilon);
	if (!(expected.k * expected.f).Equals(Point3(1, 1, 1), epsilon))
	{
		//Check scale rotation only if there is a scale factor present.
		EXPECT_NEAR(expected.u.x, result.u.x, epsilon);
		EXPECT_NEAR(expected.u.y, result.u.y, epsilon);
		EXPECT_NEAR(expected.u.z, result.u.z, epsilon);
		EXPECT_NEAR(expected.u.w, result.u.w, epsilon);
	}
}

void CompareTMs(const Matrix3 tm1, const Matrix3 tm2)
{
	AffineParts parts1;
	AffineParts parts2;
	decomp_affine(tm1, &parts1);
	decomp_affine(tm2, &parts2);
	CompareAffineParts(parts1, parts2);
}

TEST(listctrl, AverageWeights)
{
	theHold.Suspend();
	
	constexpr size_t num_controllers = 10;

	 IListControl* pos_list = reinterpret_cast<IListControl*>(GetPositionListDesc()->Create(false));

	// Compute total weight.
	float total_weight = 0.f;
	for (size_t i = 0; i < num_controllers; ++i)
	{
		float weight = float(i);
		total_weight += weight;
	}

	// Set the positions and compute the normalized position.
	Point3 normalized_pos{ 0.f, 0.f, 0.f };
	for (size_t i = 0; i < num_controllers; ++i)
	{
		Point3 pos{ float(i), -float(i), 1.f };
		Control* posctrl = NewDefaultPositionController();
		posctrl->SetValue(0, &pos);
		pos_list->AssignController(posctrl, int(i));
		float weight = float(i);
		pos_list->GetParamBlock(0)->SetValue(kListCtrlWeight, 0, weight, int(i));
		normalized_pos += pos * (weight / total_weight);
	}

	// Get the values and get the weights.
	for (size_t i = 0; i < num_controllers; ++i)
	{
		// Check the pos was set ok.
		Point3 pos{ float(i), -float(i), 1.f };
		Control* posctrl = dynamic_cast<Control*>(pos_list->SubAnim(int(i)));
		ASSERT_NE(posctrl, nullptr);
		Point3 p{ 0.f, 0.f, 0.f };
		Interval interval{ FOREVER };
		posctrl->GetValue(0, &p, interval);
		EXPECT_EQ(p, pos);

		// Check the weights were set ok.
		float get_value_weight = 0.f;
		pos_list->GetParamBlock(0)->GetValue(kListCtrlWeight, 0, get_value_weight, interval, int(i));
		EXPECT_EQ(get_value_weight, float(i));

		float get_subctrl_weight = pos_list->GetSubCtrlWeight(int(i), 0);
		EXPECT_EQ(get_subctrl_weight, float(i));
	}

	// Enable average, check if the position averaged ok.
	Interval interval{ FOREVER };
	Point3 p{ 0.f, 0.f, 0.f };
	pos_list->GetParamBlock(0)->SetValue(kListCtrlAverage, 0, TRUE);
	BOOL average = FALSE;
	pos_list->GetParamBlock(0)->GetValue(kListCtrlAverage, 0, average, interval);
	EXPECT_EQ(average, TRUE);

	pos_list->GetValue(0, &p, interval);
	EXPECT_NEAR(p.x, normalized_pos.x, epsilon);
	EXPECT_NEAR(p.y, normalized_pos.y, epsilon);
	EXPECT_NEAR(p.z, normalized_pos.z, epsilon);

	theHold.Resume();
}

void TestRotationBlend(Quat A, Quat B, float blendAmount, Quat expectedResult)
{
	Matrix3 rotMatA;
	Matrix3 rotMatB;
	rotMatA.SetRotate(A);
	rotMatB.SetRotate(B);
	AffineParts rotAParts;
	AffineParts rotBParts;
	decomp_affine(rotMatA, &rotAParts);
	decomp_affine(rotMatB, &rotBParts);

	const AffineParts resultParts = MatrixMath::BlendAffineParts(rotAParts, rotBParts, blendAmount, blendAmount, true);
	// extract rotations again

	// using Quat::Equals(): we need to compare to apart.q and -apart.q because [q0, q] and [-q0, -q] represent the same rotation
	EXPECT_TRUE(expectedResult.Equals(resultParts.q) || expectedResult.Equals(-resultParts.q));

	// using Matrix3::Equals()
	Matrix3 expectedM, blendedM;
	expectedResult.MakeMatrix(expectedM);
	resultParts.q.MakeMatrix(blendedM);
	EXPECT_TRUE(expectedM.Equals(blendedM));
}

TEST(listctrl, TestMatrixBlendTranslation)
{
	Matrix3 transMat;
	transMat.SetTrans(Point3(0, 0, 10));
	AffineParts identityParts;
	AffineParts transParts;
	identityParts.k.Set(1, 1, 1); //Default scale of 1;
	identityParts.f = 1.0f;
	decomp_affine(transMat, &transParts);

	AffineParts resultParts = MatrixMath::BlendAffineParts(identityParts, transParts, 0.5f, 0.5f, true);
	EXPECT_NEAR(resultParts.t.x, 0.f, epsilon);
	EXPECT_NEAR(resultParts.t.y, 0.f, epsilon);
	EXPECT_NEAR(resultParts.t.z, 5.0f, epsilon);

	// Try a negative weight case
	resultParts = MatrixMath::BlendAffineParts(identityParts, transParts, -1.f, -1.f, true);
	EXPECT_NEAR(resultParts.t.x, 0.f, epsilon);
	EXPECT_NEAR(resultParts.t.y, 0.f, epsilon);
	EXPECT_NEAR(resultParts.t.z, -transParts.t.z, epsilon);

	// Try a doubled up wieght case
	resultParts = MatrixMath::BlendAffineParts(identityParts, transParts, 2.f, 2.f, true);
	EXPECT_NEAR(resultParts.t.x, 0.f, epsilon);
	EXPECT_NEAR(resultParts.t.y, 0.f, epsilon);
	EXPECT_NEAR(resultParts.t.z,  2.f * transParts.t.z, epsilon);
}

TEST(listctrl, TestMatrixBlendRotation_01)
{
	// identity
	const Quat q0;
	TestRotationBlend(q0, q0, 0.0f, q0);
	TestRotationBlend(q0, q0, 0.123f, q0);
	TestRotationBlend(q0, q0, 1.0f, q0);
	TestRotationBlend(q0, q0, 2.0f, q0);
	TestRotationBlend(q0, q0, -1.0f, q0);
}

TEST(listctrl, TestMatrixBlendRotation_02)
{
	Quat rot, q0, halfRot;
	rot.SetEuler(90.0f * PI / 180.0f, 0.f, 0.f); // 90 degrees on x
	halfRot.SetEuler(45.0f * PI / 180.0f, 0.f, 0.f); // 45 degrees on x
	
	TestRotationBlend(rot, q0, 0.5f, halfRot);
	TestRotationBlend(rot, q0, 0.f, rot);
	TestRotationBlend(rot, q0, 1.f, q0);
}

TEST(listctrl, TestMatrixBlendRotation_03)
{
	// near 180 degree rotation
	Quat rot, resultRot, q0;
	rot.SetEuler(0.0f, 0.f, 179 * PI / 180.0f); // 180 degrees on z
	resultRot.SetEuler(0.f, 0.f, 89.5f * PI / 180.0f); // 90 degrees on z
	TestRotationBlend(rot, q0, 0.5f, resultRot);
	TestRotationBlend(rot, q0, 0.f, rot);
	TestRotationBlend(rot, q0, 1.f, q0);
}

TEST(listctrl, TestMatrixBlendRotation_04)
{
	// Negative weight
	Quat rot;
	rot.SetEuler(0.0f, 0.f, 90.f * PI / 180.0f); // 90 degrees

	auto getAngle = [](Quat q)-> float
	{
		AngAxis aa(q);
		return aa.angle * 180.f / PI;
	};

	Matrix3 rotMat;
	rotMat.SetRotate(rot);
	AffineParts parts;
	AffineParts identParts;
	identParts.k.Set(1, 1, 1); //Default scale of 1;
	identParts.f = 1.0f;
	decomp_affine(rotMat, &parts);
	AffineParts parts2 = MatrixMath::BlendAffineParts(identParts, parts, -1.0f, -1.0f, true);

	AngAxis originalAA(rot);
	AngAxis resultAA(parts2.q);
	
	// Angle will be positive but the axis will be flipped!
	EXPECT_NEAR(originalAA.angle * 180.f / PI, resultAA.angle * 180.f / PI, epsilon);
	EXPECT_TRUE(originalAA.axis.Equals(-resultAA.axis, epsilon));
	
}

TEST(listctrl, TestMatrixBlendScale)
{
	AffineParts identityParts;
	identityParts.k.Set(1, 1, 1); //Default scale of 1;
	identityParts.f = 1.0f;

	Matrix3 scaleMat;
	scaleMat.SetScale(Point3(4, 6, 8));
	AffineParts scaleParts;
	decomp_affine(scaleMat, &scaleParts);
	

	// sequential off
	AffineParts resultParts = MatrixMath::BlendAffineParts(identityParts, scaleParts, 0.5f, 0.5f, false);
	EXPECT_NEAR(resultParts.k.x, 2.5f, epsilon);
	EXPECT_NEAR(resultParts.k.y, 3.5f, epsilon);
	EXPECT_NEAR(resultParts.k.z, 4.5f, epsilon);

	// sequential on
	resultParts = MatrixMath::BlendAffineParts(identityParts, scaleParts, 0.5f, 0.5f, true);
	EXPECT_NEAR(resultParts.k.x, 2.5f, epsilon);
	EXPECT_NEAR(resultParts.k.y, 3.5f, epsilon);
	EXPECT_NEAR(resultParts.k.z, 4.5f, epsilon);
	
}

void SetupTMListTest(Control* tm_list, Quat startRot, Quat endRot, float startX, float endX)
{
	theHold.Suspend();
	
	// Check the blending of rotations:
	// 
	//    startRot     /\ Z        endRot
	//    |            |            |
	// X  |------------o------------|
	//    -10          0            10
	//    tm1                      tm2

	std::vector<Control*> subTMsControllers;
	subTMsControllers.emplace_back(NewDefaultMatrix3Controller());
	subTMsControllers.emplace_back(NewDefaultMatrix3Controller());
	ASSERT_NE(subTMsControllers[0], nullptr);
	ASSERT_NE(subTMsControllers[1], nullptr);

	//control 1
	SetXFormPacket pckt1;
	pckt1.tmAxis.IdentityMatrix();
	pckt1.tmParent.IdentityMatrix();
	PreRotateMatrix(pckt1.tmAxis, startRot);
	pckt1.tmAxis.SetTrans(Point3(startX, 0.f, 0.f));
	pckt1.command = XFORM_SET;
	subTMsControllers[0]->SetValue(100, &pckt1);

	//control 2
	SetXFormPacket pckt2;
	pckt2.tmAxis.IdentityMatrix();
	pckt2.tmParent.IdentityMatrix();
	PreRotateMatrix(pckt2.tmAxis, endRot);
	pckt2.tmAxis.SetTrans(Point3(endX, 0.f, 0.f));
	pckt2.command = XFORM_SET;
	subTMsControllers[1]->SetValue(100, &pckt2);

	tm_list->AssignController(subTMsControllers[0], 0);
	tm_list->AssignController(subTMsControllers[1], 1);

	theHold.Resume();
}

Quat BlendQuats(bool average, Quat input1, Quat input2, float weight1, float weight2)
{
	// Figure out the expected rotation
	Quat input1_;
	Quat input2_;
	if (average)
	{
		input1.MakeClosest(IdentQuat());
		if (fabs(weight1) > std::numeric_limits<float>::epsilon()) {
			input1_ = Slerp(IdentQuat(), input1, weight1);
		}
		else {
			input1_ = IdentQuat();
		}

		input2.MakeClosest(input1_);
		if (fabs(weight2) > std::numeric_limits<float>::epsilon()) {
			input2_ = Slerp(input1_, input2, weight2);
		}
		else {
			input2_ = input1_;
		}
		
	}
	else
	{
		input1.MakeClosest(IdentQuat());
		input2.MakeClosest(IdentQuat());

		if (fabs(weight1) > std::numeric_limits<float>::epsilon()) {
			input1_ = Slerp(IdentQuat(), input1, weight1);
			input1_ = IdentQuat() * input1_;
		}
		else {
			input1_ = IdentQuat();
		}
		

		if (fabs(weight2) > std::numeric_limits<float>::epsilon()) {
			input2_ = Slerp(IdentQuat(), input2, weight2);
			input2_ = input1_ * input2_;
		}
		else {
			input2_ = input1_;
		}
		
	}
	const Quat expectedQ = input2_;
	return expectedQ;
}

TEST(listctrl, TMList_BasicOneEntry)
{
	IListControl* tm_list = reinterpret_cast<IListControl*>(GetTransformListDesc()->Create(false));
	tm_list->AssignController(NewDefaultMatrix3Controller(), 0);

	Matrix3 tmOut;
	tm_list->GetValue(0, &tmOut, CTRL_RELATIVE);

	AffineParts parts;
	decomp_affine(tmOut, &parts);
	EXPECT_TRUE(parts.t.Equals(Point3(), epsilon));
	EXPECT_TRUE(parts.q.Equals(Quat(), epsilon));
	EXPECT_TRUE(parts.u.Equals(Quat(), epsilon));
	EXPECT_TRUE(parts.k.Equals(Point3(1,1,1), epsilon));
	EXPECT_NEAR(parts.f, 1.0f, epsilon);

	tm_list->DeleteMe();
}

TEST(listctrl, TMList_ParentTM_Trans)
{
	theHold.Suspend();
	
	Matrix3 parentTm;
	parentTm.SetTrans(Point3(50, 50, 0));
	const Point3 controlTans(-50, -50, 0);

	auto testResult = [&parentTm](Control* tm_list, Point3 expectedTrans)
	{
		Matrix3 tmOut = parentTm;
		tm_list->GetValue(0, &tmOut, CTRL_RELATIVE);

		AffineParts parts;
		decomp_affine(tmOut, &parts);
		if(!parts.t.Equals(expectedTrans, epsilon))
		{
			EXPECT_TRUE(parts.t.Equals(expectedTrans, epsilon));
		}
		EXPECT_TRUE(parts.q.Equals(Quat(), epsilon));
		EXPECT_TRUE(parts.u.Equals(Quat(), epsilon));
		EXPECT_TRUE(parts.k.Equals(Point3(1, 1, 1), epsilon));
		EXPECT_NEAR(parts.f, 1.0f, epsilon);
	};

	{
		// Test SetValue, XFORM_MOVE
		IListControl* tm_list = reinterpret_cast<IListControl*>(GetTransformListDesc()->Create(false));
		tm_list->AssignController(NewDefaultMatrix3Controller(), 0);

		//Setup controllers value with a transform move -> XFORM_MOVE
		SetXFormPacket movePacket(controlTans, parentTm);
		EXPECT_EQ(movePacket.command, XFORM_MOVE);
		tm_list->SetValue(0, &movePacket, 1, CTRL_RELATIVE);
		const Point3 expectedResult = parentTm.GetTrans() + controlTans;
		//const Point3 expectedResult = controlTans * parentTm;
		testResult(tm_list, expectedResult);
		tm_list->DeleteMe();
	}
	{
		// Test SetValue, XFORM_SET
		IListControl* tm_list = reinterpret_cast<IListControl*>(GetTransformListDesc()->Create(false));
		tm_list->AssignController(NewDefaultMatrix3Controller(), 0);

		//Setup controllers value with a transform move -> XFORM_SET
		Matrix3 transTM;
		transTM.SetTrans(controlTans);
		SetXFormPacket setTMPacket(transTM, parentTm);
		EXPECT_EQ(setTMPacket.command, XFORM_SET);
		tm_list->SetValue(0, &setTMPacket, 1, CTRL_ABSOLUTE);
		testResult(tm_list, controlTans);
		tm_list->DeleteMe();
	}

	{
		// Test SetValue, XFORM_MOVE but when there are two blended sub tms

		auto checkTheValue = [&](IListControl* tm_list)
		{
			// test that we only get parent tm
			testResult(tm_list, Point3(50, 50, 0));
			
			//Setup controllers value with a transform move -> XFORM_MOVE
			SetXFormPacket movePacket(controlTans, parentTm);
			EXPECT_EQ(movePacket.command, XFORM_MOVE);
			tm_list->SetValue(0, &movePacket, 1, CTRL_RELATIVE);
			const Point3 expectedResult = parentTm.GetTrans() + controlTans;
			//const Point3 expectedResult = controlTans * parentTm;
			testResult(tm_list, expectedResult);

			//Now Get with average on
			tm_list->GetParamBlock(0)->SetValue(kListCtrlSequential, 0, TRUE);
			tm_list->GetParamBlock(0)->SetValue(kListCtrlAverage, 0, TRUE);
			BOOL average = FALSE;
			BOOL sequential = FALSE;
			tm_list->GetParamBlock(0)->GetValue(kListCtrlSequential, 0, average);
			tm_list->GetParamBlock(0)->GetValue(kListCtrlAverage, 0, sequential);
			EXPECT_EQ(average, TRUE);
			EXPECT_EQ(sequential, TRUE);
			const Point3 expectedResult2 = parentTm.GetTrans() + (controlTans / 2.0f);
			//const Point3 expectedResult2 = (controlTans / 2.0f) * parentTm;
			testResult(tm_list, expectedResult2);
		};
		{
			IListControl* tm_list = reinterpret_cast<IListControl*>(GetTransformListDesc()->Create(false));
			tm_list->AssignController(NewDefaultMatrix3Controller(), 0);
			tm_list->AssignController(NewDefaultMatrix3Controller(), 1);

			checkTheValue(tm_list);
			tm_list->DeleteMe();
		}
		{
			//Switch which controller is the active one, should be the same result.
			IListControl* tm_list2 = reinterpret_cast<IListControl*>(GetTransformListDesc()->Create(false));
			tm_list2->AssignController(NewDefaultMatrix3Controller(), 0);
			tm_list2->AssignController(NewDefaultMatrix3Controller(), 1);
			tm_list2->SetActive(1);

			checkTheValue(tm_list2);
			tm_list2->DeleteMe();
		}
	}

	theHold.Resume();
}

TEST(listctrl, TMList_SetValue_Blending)
{
	theHold.Suspend();
	
	// TODO: KZ, the TransformListControl::SetValue is not finalized. Test part of the current situation.

	// Test the SetValue code path while there is Sub tm blending.
	const float startZRot = -45.f * PI / 180.f;
	const float endZRot = 45.f * PI / 180.f;
	const float startX = -10.f;
	const float endX = 10.f;

	SetCOREInterface(new DefaultMockCoreInterface);

	// Check the blending of rotations:
	// 
	//    -45 z-rot    /\ Z        +45 z-rot
	//    d            |            b
	// X  |------------o------------|
	//    -10          0            10
	//    tm1                      tm2

	auto setWeights = [](Control* tm_list, float weight)
	{
		const float weight1 = 1.0f - weight;
		const float weight2 = weight;
		tm_list->GetParamBlock(0)->SetValue(kListCtrlWeight, 0, weight1, 0);
		tm_list->GetParamBlock(0)->SetValue(kListCtrlWeight, 0, weight2, 1);
	};

	{
		// Translation
		IListControl* tm_list = reinterpret_cast<IListControl*>(GetTransformListDesc()->Create(false));
		SetupTMListTest(tm_list, Quat(AngAxis(Point3(0, 0, 1), startZRot)), Quat(AngAxis(Point3(0, 0, 1), endZRot)), startX, endX);
		tm_list->GetParamBlock(0)->SetValue(kListCtrlSequential, 0, TRUE);
		tm_list->GetParamBlock(0)->SetValue(kListCtrlWeightAgainstMode, 0, TRUE); // keep in sync with sequential
		tm_list->GetParamBlock(0)->SetValue(kListCtrlAverage, 0, TRUE);
		{
			// Set the blending to 50%
			setWeights(tm_list, .5f);
			Matrix3 tmOut;
			tm_list->GetValue(0, &tmOut, CTRL_RELATIVE);
			// Sanity check
			EXPECT_TRUE(tmOut.GetTrans().Equals(Point3(), epsilon));
		}
		{
			// Try setting a value and see how that impacts the result with blending
			Matrix3 parentTm;
			parentTm.SetTrans(Point3(10, 10, 10));
			Point3 setTrans(10, 0, 0);
			SetXFormPacket movePacket(Point3(10, 0, 0), parentTm);
			EXPECT_EQ(movePacket.command, XFORM_MOVE);
			// This SetValue at 50% applied half the of "setTrans".
			tm_list->SetValue(0, &movePacket, 1, CTRL_RELATIVE);

			//Check what happened to the active controller
			const Point3 expected0Val = Point3(startX, 0.f, 0.f) + setTrans;
			{
				setWeights(tm_list, 0.f);
				Matrix3 tmOut0;
				tm_list->GetValue(0, &tmOut0, CTRL_RELATIVE);
				EXPECT_TRUE(tmOut0.GetTrans().Equals(expected0Val, epsilon));
			}

			// Inactive controller should be unaffected
			const Point3 expected100Val(endX, 0.f, 0.f);
			{
				setWeights(tm_list, 1.f);
				Matrix3 tmOut100;
				tm_list->GetValue(0, &tmOut100, CTRL_RELATIVE);
				EXPECT_TRUE(tmOut100.GetTrans().Equals(expected100Val, epsilon));
			}

			// At 50%
			{
				setWeights(tm_list, .5f);
				Matrix3 tmOut50;
				tm_list->GetValue(0, &tmOut50, CTRL_RELATIVE);
				Point3 expected50Val = expected0Val + (expected100Val - expected0Val) * 0.5f;
				EXPECT_TRUE(tmOut50.GetTrans().Equals(expected50Val, epsilon));
			}
		}
		tm_list->DeleteMe();
	}

	// TODO
	//{
	//	// Rotation
	//	IListControl* tm_list = reinterpret_cast<IListControl*>(GetTransformListDesc()->Create(false));
	//	SetupTMListTest(tm_list, startZRot, endZRot, startX, endX);
	//	{
	//		// Set the blending to 50%
	//		setWeights(tm_list, 50.f);
	//		Matrix3 tmOut;
	//		tm_list->GetValue(0, &tmOut, CTRL_RELATIVE);
	//		// Sanity check
	//		EXPECT_TRUE(tmOut.GetTrans().Equals(Point3(), epsilon));
	//	}
	//	{
	//		// Try setting a value and see how that impacts the result with blending
	//		const AngAxis parentAngAxis(Point3(1, 0, 0), 85.f * PI / 180.f);
	//		const Quat parentRot(parentAngAxis);
	//		Matrix3 parentTm;
	//		parentRot.MakeMatrix(parentTm);
	//		Quat setRotation(AngAxis(Point3(0,1,0), 30.f * PI / 180.f));
	//		SetXFormPacket rotatePacket(setRotation, FALSE, parentTm);
	//		EXPECT_EQ(rotatePacket.command, XFORM_ROTATE);
	//		// This SetValue at 50% applied half the of "rotatePacket".
	//		tm_list->SetValue(0, &rotatePacket, 1, CTRL_RELATIVE);

	//		// Inactive controller should be unaffected
	//		Quat expected100Val = Quat(AngAxis(Point3(0, 0, -1), endZRot));
	//		{
	//			setWeights(tm_list, 100.f);
	//			Matrix3 tmOut100;
	//			tm_list->GetValue(0, &tmOut100, CTRL_RELATIVE);
	//			AffineParts parts;
	//			decomp_affine(tmOut100, &parts);

	//			AngAxis actualParts(parts.q);
	//			float actualAngle = actualParts.angle * 180.f / PI;
	//			AngAxis expectedaaa(expected100Val);
	//			float expectedangle = expectedaaa.angle * 180.f / PI;

	//			EXPECT_NEAR(actualAngle, expectedangle, epsilon);
	//			EXPECT_TRUE(actualParts.axis.Equals(expectedaaa.axis, epsilon));
	//		}
	//		
	//		//Check what happened to the active controller
	//		// start rotation + half the set value
	//		const Quat halfSetValue = Slerp(IdentQuat(), setRotation, 0.5f);
	//		const Quat expected0Val = Quat(AngAxis(Point3(0, 0, 1), startZRot)) * halfSetValue;
	//		{
	//			setWeights(tm_list, 0.f);
	//			Matrix3 tmOut0;
	//			tm_list->GetValue(0, &tmOut0, CTRL_RELATIVE);
	//			AffineParts parts;
	//			decomp_affine(tmOut0, &parts);
	//			
	//			AngAxis aaa(parts.q);
	//			float angle = aaa.angle * 180.f / PI;
	//			AngAxis expectedaaa(expected0Val);
	//			float expectedangle = expectedaaa.angle * 180.f / PI;
	//			
	//			EXPECT_TRUE(parts.q.Equals(expected0Val, epsilon));
	//		}

	//		// At 50%
	//		{
	//			setWeights(tm_list, 50.f);
	//			Matrix3 tmOut50;
	//			tm_list->GetValue(0, &tmOut50, CTRL_RELATIVE);
	//			AffineParts parts;
	//			decomp_affine(tmOut50, &parts);
	//			AngAxis difference(expected100Val - expected0Val);
	//			Quat halfDifference(AngAxis(difference.axis, difference.angle * 0.5f));
	//			Quat expected50Val = expected0Val + halfDifference;
	//			EXPECT_TRUE(parts.q.Equals(expected50Val, epsilon));
	//		}
	//	}
	//	tm_list->DeleteMe();

	//}
	{
		// Scale
		// TODO: validate this is the correct behavior first.
	}
	{
		// Set transform
		// TODO: validate this is the correct behavior first.
	}

	SetCOREInterface(nullptr);

	theHold.Resume();
}

TEST(listctrl, TMList_OneTMVersusBlending)
{
	// Compare a TM blended against identity against the value of that 1 tm.
	AffineParts identityParts;
	identityParts.k.Set(1, 1, 1);
	identityParts.f = 1.0f;

	Matrix3 tm;
	AffineParts tmParts;
	{
		Matrix3 tmTrans;
		tmTrans.SetTrans(Point3(10, 5, 0));

		Matrix3 tmScale;
		tmScale.SetScale(Point3(1, 2, 4));

		Matrix3 tmRotation;
		Quat rot(AngAxis(Point3(0, 1, 0), 45.f * PI / 180.f));
		rot.MakeMatrix(tmRotation);

		tm = tmTrans * tm;
		tm = tmScale * tm;
		tm = tmRotation * tm;
	}
	decomp_affine(tm, &tmParts);

	{
		const AffineParts resultParts = MatrixMath::BlendAffineParts(identityParts, tmParts, 1.0f, 1.0f, true);		
		CompareAffineParts(tmParts, resultParts);
	}
	{
		const AffineParts resultParts = MatrixMath::BlendAffineParts(identityParts, tmParts, 0.0f, 0.0f, true);
		CompareAffineParts(identityParts, resultParts);
	}
	{
		const AffineParts resultParts = MatrixMath::BlendAffineParts(tmParts, identityParts, 0.0f, 0.0f, true);
		CompareAffineParts(tmParts, resultParts);
	}

}

namespace {
void SetWeightsOnControls(Control* tm_list, Control* comp_list, float weight1, float weight2)
{
	// Adjust the weights
	tm_list->GetParamBlock(0)->SetValue(kListCtrlWeight, 0, weight1, 0);
	tm_list->GetParamBlock(0)->SetValue(kListCtrlWeight, 0, weight2, 1);
	comp_list->GetParamBlock(0)->SetValue(kListCtrlWeight, 0, weight1, 0);
	comp_list->GetParamBlock(0)->SetValue(kListCtrlWeight, 0, weight2, 1);
}

void CompareTest(Control* tm_list, Control* comp_list, Matrix3 parentTM, BOOL average, BOOL sequential)
{
	//set average or not
	tm_list->GetParamBlock(0)->SetValue(kListCtrlAverage, 0, average);
	tm_list->GetParamBlock(0)->SetValue(kListCtrlSequential, 0, sequential); //have to change both of these for tm_list to compare
	tm_list->GetParamBlock(0)->SetValue(kListCtrlWeightAgainstMode, 0, sequential); // keep in sync with sequential
	comp_list->GetParamBlock(0)->SetValue(kListCtrlAverage, 0, average);

	// Should be same
	SetWeightsOnControls(tm_list, comp_list, 1.f, 0.f);
	Matrix3 tm1 = parentTM;
	Matrix3 tm2 = parentTM;
	tm_list->GetValue(0, &tm1, CTRL_RELATIVE);
	comp_list->GetValue(0, &tm2, CTRL_RELATIVE);
	CompareTMs(tm1, tm2);
	SetWeightsOnControls(tm_list, comp_list, 0.f, 1.f);
	tm1 = parentTM;
	tm2 = parentTM;
	tm_list->GetValue(0, &tm1, CTRL_RELATIVE);
	comp_list->GetValue(0, &tm2, CTRL_RELATIVE);
	CompareTMs(tm1, tm2);

	// Try some other ones
	std::vector<int> weights{ 50, 1, 99, 45, 55, 5, 95, 130 };
	for (int w : weights)
	{
		float weight1;
		float weight2;
		if(w <= 100.f)
		{
			weight1 = (100.f - (float)w) / 100.f;
			weight2 = (float)w / 100.f;
		}
		else
		{
			// Rotation list does not normalize so check that the TM list doesn't either for rotations only.
			weight1 = w / 100.f;
			weight2 = 0.5f;
		}
		SetWeightsOnControls(tm_list, comp_list, weight1, weight2);
		tm1 = parentTM;
		tm2 = parentTM;
		tm_list->GetValue(0, &tm1, CTRL_RELATIVE);
		comp_list->GetValue(0, &tm2, CTRL_RELATIVE);
		if(!tm1.Equals(tm2, 0.0001f))
		{
			AffineParts part1;
			AffineParts part2;
			decomp_affine(tm1, &part1);
			decomp_affine(tm2, &part2);
			CompareAffineParts(part1, part2);
		}
	}
}
}

TEST(listctrl, TMList_Compare_RotationList_PRS)
{
	theHold.Suspend();
	// Compare the results of the tm list with the position list with a rotation component
	SetCOREInterface(new DefaultMockCoreInterface);

	IListControl* tm_list = reinterpret_cast<IListControl*>(GetTransformListDesc()->Create(false));
	std::vector<Control*> subTMsControllers;
	subTMsControllers.emplace_back(CreatePRSControl());
	subTMsControllers.emplace_back(CreatePRSControl());
	std::vector<Control*> subRotations;
	subRotations.emplace_back(reinterpret_cast<Control*>(GetEulerCtrlDesc()->Create(false)));
	subRotations.emplace_back(reinterpret_cast<Control*>(GetEulerCtrlDesc()->Create(false)));
	ASSERT_NE(subTMsControllers[0], nullptr);
	ASSERT_NE(subTMsControllers[1], nullptr);
	ASSERT_NE(subRotations[0], nullptr);
	ASSERT_NE(subRotations[1], nullptr);
	
	IListControl* rot_list = reinterpret_cast<IListControl*>(GetRotationListDesc()->Create(false));

	// assign position controllers to entries in position list
	rot_list->AssignController(subRotations[0], 0);
	rot_list->AssignController(subRotations[1], 1);

	// Share everything except the rotation list is the PRS's rotation control in tm list.
	Control* prs_rot_list = CreatePRSControl();
	ASSERT_NE(prs_rot_list, nullptr);
	
	prs_rot_list->ReplaceReference(1, rot_list);
	Control* posCtrl = reinterpret_cast<Control*>(prs_rot_list->SubAnim(0));
	Control* scaleCtrl = reinterpret_cast<Control*>(prs_rot_list->SubAnim(2));
	ASSERT_NE(posCtrl, nullptr);
	ASSERT_NE(scaleCtrl, nullptr);
	subTMsControllers[0]->ReplaceReference(0, posCtrl);
	subTMsControllers[1]->ReplaceReference(0, posCtrl);
	subTMsControllers[0]->ReplaceReference(1, subRotations[0]);
	subTMsControllers[1]->ReplaceReference(1, subRotations[1]);
	subTMsControllers[0]->ReplaceReference(2, scaleCtrl);
	subTMsControllers[1]->ReplaceReference(2, scaleCtrl);

	// Assign PRS to tm list
	tm_list->AssignController(subTMsControllers[0], 0);
	tm_list->AssignController(subTMsControllers[1], 1);

	// setup some rotations
	Quat rot1 = EulerToQuat(-100.f / 180.f * PI, 1.f / 180.f * PI, 50.f / 180.f * PI);
	Quat rot2 = EulerToQuat(-1.f / 180.f * PI, -20.f / 180.f * PI, -300.f / 180.f * PI);
	subRotations[0]->SetValue(0, &rot1);
	subRotations[1]->SetValue(0, &rot2);

	// setup some other values
	Point3 trans(10, 20, 30);
	posCtrl->SetValue(0, &trans);
	//ScaleValue scaleV(Point3(1,2,1), Quat());
	//scaleCtrl->SetValue(0, &scaleV);

	auto compareTest = [](Control* tm_list, Control* comp_list, Control* prs_comp_list, Matrix3 parentTM, BOOL average)
	{
		//set average or not
		tm_list->GetParamBlock(0)->SetValue(kListCtrlAverage, 0, average);
		tm_list->GetParamBlock(0)->SetValue(kListCtrlSequential, 0, average);
		tm_list->GetParamBlock(0)->SetValue(kListCtrlWeightAgainstMode, 0, average); // keep in sync with sequential
		comp_list->GetParamBlock(0)->SetValue(kListCtrlAverage, 0, average);

		// Should be same
		SetWeightsOnControls(tm_list, comp_list, 1.f, 0.f);
		Matrix3 tm1 = parentTM;
		Matrix3 tm2 = parentTM;
		tm_list->GetValue(0, &tm1, CTRL_RELATIVE);
		prs_comp_list->GetValue(0, &tm2, CTRL_RELATIVE);
		EXPECT_TRUE(tm1.Equals(tm2, 0.0001f));
		SetWeightsOnControls(tm_list, comp_list, 0.f, 1.f);
		tm1 = parentTM;
		tm2 = parentTM;
		tm_list->GetValue(0, &tm1, CTRL_RELATIVE);
		prs_comp_list->GetValue(0, &tm2, CTRL_RELATIVE);
		EXPECT_TRUE(tm1.Equals(tm2, 0.0001f));

		// Try some other ones
		std::vector<float> weights{ 50, 1, 99, 45, 55, 5, 95, 130 };
		for (float w : weights)
		{
			float weight1;
			float weight2;
			if(w <= 100)
			{
				weight1 = (100.f - w) / 100.f;
				weight2 = w / 100.f;
			}
			else
			{
				// a case for weights that add up to more than 100.
				weight1 = w / 100.f;
				weight2 = 0.5;
			}
			
			SetWeightsOnControls(tm_list, comp_list, weight1, weight2);
			tm1 = parentTM;
			tm2 = parentTM;
			tm_list->GetValue(0, &tm1, CTRL_RELATIVE);
			prs_comp_list->GetValue(0, &tm2, CTRL_RELATIVE);

			// remove the position component when weighting above total 100% because it won't be the same as the PRS.
			if (w > 100.f)
			{
				tm1.SetTrans(Point3(0, 0, 0));
				tm2.SetTrans(Point3(0, 0, 0));
			}

			if(!tm1.Equals(tm2, 0.0001f)) {
				CompareTMs(tm1, tm2);
			}
		}
	};

	Matrix3 parentTM;
	compareTest(tm_list, rot_list, prs_rot_list, parentTM, false);
	// KZ, average being on results in relative rotation variations between sub TM's and that will always differ compared to a PRS with pos list.
	compareTest(tm_list, rot_list, prs_rot_list, parentTM, true);

	AffineParts parentParts;
	parentParts.t.Set(44, 55, 66);
	parentParts.q = Quat(AngAxis(Point3(4, 5, -88), 67.f / 180.f * PI));
	//parentParts.u
	parentParts.k.Set(1, 1, 1); //parentParts.k.Set(0.5, 2, 1);
	parentParts.f = 1.0f;
	comp_affine(parentParts, parentTM);

	compareTest(tm_list, rot_list, prs_rot_list, parentTM, false);
	// KZ, average being on results in relative rotation variations between sub TM's and that will always differ compared to a PRS with pos list.
	compareTest(tm_list, rot_list, prs_rot_list, parentTM, true);

	SetCOREInterface(nullptr);

	theHold.Resume();
}

TEST(listctrl, TMList_Compare_PositionList)
{
	theHold.Suspend();
	// Compare the results of the tm list with the position list

	IListControl* tm_list = reinterpret_cast<IListControl*>(GetTransformListDesc()->Create(false));
	std::vector<Control*> subTMsControllers;
	subTMsControllers.emplace_back(CreatePRSControl());
	subTMsControllers.emplace_back(CreatePRSControl());
	std::vector<Control*> subPositions;
	subPositions.emplace_back(reinterpret_cast<Control*>(subTMsControllers[0]->SubAnim(0)));
	subPositions.emplace_back(reinterpret_cast<Control*>(subTMsControllers[1]->SubAnim(0)));
	ASSERT_NE(subTMsControllers[0], nullptr);
	ASSERT_NE(subTMsControllers[1], nullptr);
	ASSERT_NE(subPositions[0], nullptr);
	ASSERT_NE(subPositions[1], nullptr);
	IListControl* pos_list = reinterpret_cast<IListControl*>(GetPositionListDesc()->Create(false));

	// Assign PRS to tm list
	tm_list->AssignController(subTMsControllers[0], 0);
	tm_list->AssignController(subTMsControllers[1], 1);

	// assign position controllers to entries in position list
	pos_list->AssignController(subPositions[0], 0);
	pos_list->AssignController(subPositions[1], 1);

	// setup some rotations
	Point3 pos1(100, -10, 45);
	Point3 pos2(-50, 33, -200);
	subPositions[0]->SetValue(0, &pos1);
	subPositions[1]->SetValue(0, &pos2);

	Matrix3 parentTM;
	CompareTest(tm_list, pos_list, parentTM, false, false);
	//CompareTest(tm_list, pos_list, parentTM, true, true);

	//AffineParts parentParts;
	//parentParts.t.Set(44, 55, 66);
	//parentParts.q = Quat(AngAxis(Point3(41, 15, -50), 110.f / 180.f * PI));
	////parentParts.u
	//parentParts.k.Set(1, 1, 1); //parentParts.k.Set(0.5, 2, 1);
	//parentParts.f = 1.0f;
	//comp_affine(parentParts, parentTM);

	//CompareTest(tm_list, pos_list, parentTM, false, false);
	//CompareTest(tm_list, pos_list, parentTM, true, true);

	theHold.Resume();
}

TEST(listctrl, TMList_Compare_RotationList)
{
	theHold.Suspend();
	// Compare the results of the tm list with the rotation list
	SetCOREInterface(new DefaultMockCoreInterface);

	IListControl* tm_list = reinterpret_cast<IListControl*>(GetTransformListDesc()->Create(false));
	std::vector<Control*> subTMsControllers;
	subTMsControllers.emplace_back(CreatePRSControl());
	subTMsControllers.emplace_back(CreatePRSControl());
	std::vector<Control*> subRotations;
	subRotations.emplace_back(reinterpret_cast<Control*>(GetEulerCtrlDesc()->Create(false)));
	subRotations.emplace_back(reinterpret_cast<Control*>(GetEulerCtrlDesc()->Create(false)));
	ASSERT_NE(subTMsControllers[0], nullptr);
	ASSERT_NE(subTMsControllers[1], nullptr);
	ASSERT_NE(subRotations[0], nullptr);
	ASSERT_NE(subRotations[1], nullptr);
	IListControl* rot_list = reinterpret_cast<IListControl*>(GetRotationListDesc()->Create(false));

	// Assign PRS to tm list
	tm_list->AssignController(subTMsControllers[0], 0);
	tm_list->AssignController(subTMsControllers[1], 1);

	//assign rotation controllers to PRS
	subTMsControllers[0]->ReplaceReference(1, dynamic_cast<RefTargetHandle>(subRotations[0]));
	subTMsControllers[1]->ReplaceReference(1, dynamic_cast<RefTargetHandle>(subRotations[1]));

	// assign rotation controllers to entires in rotation list
	rot_list->AssignController(subRotations[0], 0);
	rot_list->AssignController(subRotations[1], 1);

	// setup some rotations
	Quat rot1 = EulerToQuat(-100.f / 180.f * PI, 1.f / 180.f * PI, 50.f / 180.f * PI);
	Quat rot2 = EulerToQuat(-1.f / 180.f * PI, -20.f / 180.f * PI, -300.f / 180.f * PI);
	subRotations[0]->SetValue(0, &rot1);
	subRotations[1]->SetValue(0, &rot2);

	Matrix3 parentTM;
	CompareTest(tm_list, rot_list, parentTM, false, false);
	CompareTest(tm_list, rot_list, parentTM, true, true);
	
	AffineParts parentParts;
	parentParts.t.Set(44, 55, 66);
	parentParts.q = Quat(AngAxis(Point3(4, 5, -88), 67.f / 180.f * PI));
	//parentParts.u
	parentParts.k.Set(1, 1, 1); //parentParts.k.Set(0.5, 2, 1);
	parentParts.f = 1.0f;
	comp_affine(parentParts, parentTM);
	
	CompareTest(tm_list, rot_list, parentTM, false, false);
	CompareTest(tm_list, rot_list, parentTM, true, true);

	SetCOREInterface(nullptr);
	theHold.Resume();
}

// KZ, tm_list cannot be compared to the scale_list because:
// Scale_list componds the scales going down the list and weights the difference:
//		the scales together: (2 * 50% ) * (4 * 50%) = 1.5 * 2 = 3
// tm_list weights between the scales and adds them: (2 at 50% ) towards (4 at 50%) = 1.5 towards 2 = 1.75
// 
TEST(listctrl, TMList_Compare_ScaleList)
{
	theHold.Suspend();
	// Compare the results of the tm list with the position list

	IListControl* tm_list = reinterpret_cast<IListControl*>(GetTransformListDesc()->Create(false));
	std::vector<Control*> subTMsControllers;
	subTMsControllers.emplace_back(CreatePRSControl());
	subTMsControllers.emplace_back(CreatePRSControl());
	std::vector<Control*> subScales;
	subScales.emplace_back(reinterpret_cast<Control*>(subTMsControllers[0]->SubAnim(2)));
	subScales.emplace_back(reinterpret_cast<Control*>(subTMsControllers[1]->SubAnim(2)));
	ASSERT_NE(subTMsControllers[0], nullptr);
	ASSERT_NE(subTMsControllers[1], nullptr);
	ASSERT_NE(subScales[0], nullptr);
	ASSERT_NE(subScales[1], nullptr);
	IListControl* scale_list = reinterpret_cast<IListControl*>(GetScaleListDesc()->Create(false));

	// Assign PRS to tm list
	tm_list->AssignController(subTMsControllers[0], 0);
	tm_list->AssignController(subTMsControllers[1], 1);

	// assign position controllers to entries in position list
	scale_list->AssignController(subScales[0], 0);
	scale_list->AssignController(subScales[1], 1);

	// setup some scales
	ScaleValue scale1(Point3(2, 3, 4), Quat());
	ScaleValue scale2(Point3(1, 2, 2), Quat());
	
	subScales[0]->SetValue(0, &scale1);
	subScales[1]->SetValue(0, &scale2);

	Matrix3 parentTM;
	CompareTest(tm_list, scale_list, parentTM, false, true);
	CompareTest(tm_list, scale_list, parentTM, true, true);

	AffineParts parentParts;
	parentParts.t.Set(44, 55, 66);
	parentParts.k.Set(0.5, 2, 1);
	parentParts.f = 1.0f;
	comp_affine(parentParts, parentTM);

	CompareTest(tm_list, scale_list, parentTM, false, true);
	CompareTest(tm_list, scale_list, parentTM, true, true);

	theHold.Resume();
}

TEST(listctrl, TMList_Sequential)
{
	theHold.Suspend();
	{
		// Test against hard coded expectation for rot and scale with sequential on and off.
		const float control1XRot = 90.f * PI / 180.f;
		const float control2YRot = 90.f * PI / 180.f;
		IListControl* tm_list = reinterpret_cast<IListControl*>(GetTransformListDesc()->Create(false));
		SetupTMListTest(tm_list,
			Quat(AngAxis(Point3(0, 0, 1), control1XRot)),
			Quat(AngAxis(Point3(0, 0, 1), control2YRot)), 0.0f, 0.0f);

		//Full weight
		tm_list->GetParamBlock(0)->SetValue(kListCtrlWeight, 0, 1.f, 0);
		tm_list->GetParamBlock(0)->SetValue(kListCtrlWeight, 0, 1.f, 1);

		// test sequential off first
		tm_list->GetParamBlock(0)->SetValue(kListCtrlSequential, 0, FALSE, 1);
		tm_list->GetParamBlock(0)->SetValue(kListCtrlWeightAgainstMode, 0, FALSE); // keep in sync with sequential

		Matrix3 resultTM;
		tm_list->GetValue(0, &resultTM, CTRL_RELATIVE);

		AffineParts parts;
		decomp_affine(resultTM, &parts);
		AngAxis aa(parts.q);

		// Expected result of 90X and 90Y = 180Z
		EXPECT_NEAR(aa.angle, PI, epsilon);
		EXPECT_NEAR(aa.axis.x, 0.f, epsilon);
		EXPECT_NEAR(aa.axis.y, 0.f, epsilon);
		EXPECT_NEAR(fabs(aa.axis.z), 1.0f, epsilon);
	}
	{
		//Test scale, the blend function is fine.
		AffineParts identParts;
		identParts.f = 1.0f;
		identParts.k = Point3(1,1,1);
		AffineParts part1;
		AffineParts part2;
		part1.f = 1.0f;
		part1.k = Point3(2,3,4);
		part2.f = 1.0f;
		part2.k = Point3(1,2,2);

		{
			// sequential off
			// scale additive, the previous result scaled by the next
			AffineParts res = MatrixMath::BlendAffineParts(identParts, part1, 0.5f, 0.5f, false/*sequential*/);
			EXPECT_NEAR(res.k.x, 1.5f, epsilon);
			EXPECT_NEAR(res.k.y, 2.f, epsilon);
			EXPECT_NEAR(res.k.z, 2.5f, epsilon);
			res = MatrixMath::BlendAffineParts(res, part2, 0.5f, 0.5f, false/*sequential*/);
			EXPECT_NEAR(res.k.x, 1.5f, epsilon);
			EXPECT_NEAR(res.k.y, 2.5f, epsilon);
			EXPECT_NEAR(res.k.z, 3.f, epsilon);
		}

		{
			// sequential off
			// scale is compounded, the previous result scaled by the next
			AffineParts res = MatrixMath::BlendAffineParts(identParts, part1, 0.5f, 0.5f, true/*sequential*/);
			EXPECT_NEAR(res.k.x, 1.5f, epsilon);
			EXPECT_NEAR(res.k.y, 2.f, epsilon);
			EXPECT_NEAR(res.k.z, 2.5f, epsilon);
			res = MatrixMath::BlendAffineParts(res, part2, 0.5f, 0.5f, true/*sequential*/);
			EXPECT_NEAR(res.k.x, 1.5f, epsilon);
			EXPECT_NEAR(res.k.y, 3.f, epsilon);
			EXPECT_NEAR(res.k.z, 3.75f, epsilon);
		}
	}
	// No need to test translation, there is no difference sequential on of off.

	theHold.Resume();
}

TEST(listctrl, TMList_ZeroWeight)
{
	theHold.Suspend();
	
	// Make sure we at least have identity when weight is zero
	const float startZRot = -45.f * PI / 180.f;
	const float endZRot = 45.f * PI / 180.f;
	const float startX = -10.f;
	const float endX = 10.f;
	IListControl* tm_list = reinterpret_cast<IListControl*>(GetTransformListDesc()->Create(false));
	SetupTMListTest(tm_list, Quat(AngAxis(Point3(0, 0, 1), startZRot)), Quat(AngAxis(Point3(0, 0, 1), endZRot)), startX, endX);

	//Set zero weight
	tm_list->GetParamBlock(0)->SetValue(kListCtrlWeight, 0, 0.f, 0);
	tm_list->GetParamBlock(0)->SetValue(kListCtrlWeight, 0, 0.f, 1);

	//Check that we at least get identity
	Matrix3 tm;
	tm_list->GetValue(0, &tm, CTRL_RELATIVE);
	EXPECT_TRUE(tm.Equals(Matrix3::Identity, epsilon));

	theHold.Resume();
}

//TEST(listctrl, TMList_Rotation_CHECK)
//{
//	// Make sure we at least have identity when weight is zero
//	const float rot1X = 45.f * PI / 180.f;
//	const float rot2Y = 45.f * PI / 180.f;
//	IListControl* tm_list = reinterpret_cast<IListControl*>(GetTransformListDesc()->Create(false));
//	SetupTMListTest(tm_list, Quat(AngAxis(Point3(1, 0, 0), rot1X)), Quat(AngAxis(Point3(0, 1, 0), rot2Y)), 0.0f, 0.0f);
//
//	//Set weight
//	tm_list->GetParamBlock(0)->SetValue(kListCtrlWeight, 0, 1.f, 0);
//	tm_list->GetParamBlock(0)->SetValue(kListCtrlWeight, 0, 1.f, 1);
//	
//	//Check that we at least get 45x and 45y rotations
//	Matrix3 tm;
//	tm_list->GetValue(0, &tm, CTRL_RELATIVE);
//	Quat resultQ(tm);
//	Point3 angles;
//	resultQ.GetEuler(&angles.x, &angles.y, &angles.z);
//	angles *= 180.f / PI;
//
//	Matrix3 tm2;
//	tm_list->GetValue(0, &tm2, CTRL_RELATIVE);
//	Quat resultQ2(tm2);
//	Point3 angles2;
//	resultQ2.GetEuler(&angles2.x, &angles2.y, &angles2.z);
//	angles2 *= 180.f / PI;
//
//	int a = 123;
//}


namespace ListControltestMethods
{
	int GetIndexModeIndex(IListControl* listControl)
	{
		if (listControl == nullptr) {
			EXPECT_TRUE(false);
			return -99999;
		}
		int index = 0;
		listControl->GetParamBlock(0)->GetValue(kListCtrlIndex, 0, index);
		return index;
	}
	void SetIndexModeIndex(IListControl* listControl, int index)
	{
		if (listControl == nullptr) {
			EXPECT_TRUE(false);
			return;
		}
		listControl->GetParamBlock(0)->SetValue(kListCtrlIndex, 0, index);
	}
	bool GetIndexMode(IListControl* listControl)
	{
		if (listControl == nullptr) {
			EXPECT_TRUE(false);
			return false;
		}
		BOOL indexMode = FALSE;
		listControl->GetParamBlock(0)->GetValue(kListCtrlIndexMode, 0, indexMode);
		return indexMode != 0;
	}
	void SetIndexMode(IListControl* listControl, bool indexMode)
	{
		if (listControl == nullptr) {
			EXPECT_TRUE(false);
			return;
		}
		const BOOL bIndexMode = indexMode;
		listControl->GetParamBlock(0)->SetValue(kListCtrlIndexMode, 0, bIndexMode);
	}
	bool GetAverage(IListControl* listControl)
	{
		if (listControl == nullptr) {
			EXPECT_TRUE(false);
			return false;
		}
		BOOL average = FALSE;
		listControl->GetParamBlock(0)->GetValue(kListCtrlAverage, 0, average);
		return average != 0;
	}
	void SetAverage(IListControl* listControl, bool average)
	{
		if (listControl == nullptr) {
			EXPECT_TRUE(false);
			return;
		}
		const BOOL bAverage = average;
		listControl->GetParamBlock(0)->SetValue(kListCtrlAverage, 0, bAverage);
	}
	bool GetSequential(IListControl* listControl)
	{
		if (listControl == nullptr) {
			EXPECT_TRUE(false);
			return false;
		}
		BOOL sequential = FALSE;
		listControl->GetParamBlock(0)->GetValue(kListCtrlSequential, 0, sequential);
		return sequential != 0;
	}
	void SetSequential(IListControl* listControl, bool sequential)
	{
		if (listControl == nullptr) {
			EXPECT_TRUE(false);
			return;
		}
		const BOOL bSequential = sequential;
		listControl->GetParamBlock(0)->SetValue(kListCtrlSequential, 0, bSequential);
	}

	bool GetWeightAgainstMode(IListControl* listControl)
	{
		if (listControl == nullptr) {
			EXPECT_TRUE(false);
			return false;
		}
		BOOL weightAgainstMode = FALSE;
		listControl->GetParamBlock(0)->GetValue(kListCtrlWeightAgainstMode, 0, weightAgainstMode);
		return weightAgainstMode != 0;
	}
	void SetWeightAgainstMode(IListControl* listControl, bool weightAgainstMode)
	{
		if (listControl == nullptr) {
			EXPECT_TRUE(false);
			return;
		}
		const BOOL bweightAgainstMode = weightAgainstMode;
		listControl->GetParamBlock(0)->SetValue(kListCtrlWeightAgainstMode, 0, bweightAgainstMode);
	}
};


void TestListControlAPI(IListControl* listControl, Control* control1, Control* control2)
{
	if(listControl == nullptr || control1 == nullptr || control2 == nullptr) {
		ASSERT_TRUE(false);
	}
	
	// Make sure I can add these to the list
	EXPECT_EQ(listControl->SuperClassID(), control1->SuperClassID());
	EXPECT_EQ(listControl->SuperClassID(), control2->SuperClassID());

	EXPECT_FALSE(ListControltestMethods::GetIndexMode(listControl));
	
	//count, and weight property
	EXPECT_EQ(0, listControl->GetListCount());
	EXPECT_EQ(nullptr, listControl->SubAnim(3));
	EXPECT_EQ(nullptr, listControl->SubAnim(-1));

	listControl->AssignController(control1, 0);
	EXPECT_EQ(1, listControl->GetListCount());
	listControl->AssignController(control2, 1);
	EXPECT_EQ(2, listControl->GetListCount());
	
	// getSubCtrl
	EXPECT_EQ(nullptr, listControl->GetSubCtrl(-1));
	EXPECT_EQ(control1, listControl->GetSubCtrl(0));
	EXPECT_EQ(control2, listControl->GetSubCtrl(1));
	EXPECT_EQ(nullptr, listControl->GetSubCtrl(2));
	
	//getname, setname
	EXPECT_EQ(L"", listControl->GetName(-1));
	EXPECT_EQ(L"", listControl->GetName(2));
	EXPECT_EQ(L"", listControl->SubAnimName(-1));
	EXPECT_EQ(_T("Available"), listControl->SubAnimName(2));
	// hard to check the names exactly.
	MSTR name;
	control1->GetClassName(name, false);
	EXPECT_EQ(listControl->GetName(0), name);
	EXPECT_EQ(listControl->GetName(0), listControl->SubAnimName(0));
	control2->GetClassName(name, false);
	EXPECT_EQ(listControl->GetName(1), name);
	EXPECT_EQ(listControl->GetName(1), listControl->SubAnimName(1));
	listControl->SetName(0, L"control1");
	listControl->SetName(1, L"control2");
	EXPECT_EQ(L"control1", listControl->GetName(0));
	EXPECT_EQ(L"control2", listControl->GetName(1));
	EXPECT_EQ(L"control1", listControl->SubAnimName(0));
	EXPECT_EQ(L"control2", listControl->SubAnimName(1));
	
	// getSubCtrlWeight

	EXPECT_EQ(0.0f, listControl->GetSubCtrlWeight(-1, 0)); //bad index
	EXPECT_EQ(1.0f, listControl->GetSubCtrlWeight(0, 0));
	EXPECT_EQ(1.0f, listControl->GetSubCtrlWeight(1, 0));
	EXPECT_EQ(0.0f, listControl->GetSubCtrlWeight(2, 0)); //bad index
	listControl->GetParamBlock(0)->SetValue(kListCtrlWeight, 0, .5f, 0);
	listControl->GetParamBlock(0)->SetValue(kListCtrlWeight, 0, 1.23f, 1);
	EXPECT_EQ(0.5f, listControl->GetSubCtrlWeight(0, 0));
	EXPECT_EQ(1.23f, listControl->GetSubCtrlWeight(1, 0));
	
	//active control: setactive, getactive
	EXPECT_EQ(0, listControl->GetActive());
	listControl->SetActive(99);
	EXPECT_EQ(1, listControl->GetActive());
	listControl->SetActive(-99);
	EXPECT_EQ(0, listControl->GetActive());
	listControl->SetActive(2);
	EXPECT_EQ(1, listControl->GetActive());
	
	EXPECT_EQ(0, ListControltestMethods::GetIndexModeIndex(listControl)); //should remain unchanged in weight mode
	listControl->SetActive(1);
	EXPECT_EQ(1, listControl->GetActive());
	EXPECT_EQ(0, ListControltestMethods::GetIndexModeIndex(listControl)); //should remain unchanged in weight mode
	
	//indexmode, change to index mode and check active and index
	ListControltestMethods::SetIndexMode(listControl, true);
	EXPECT_TRUE(ListControltestMethods::GetIndexMode(listControl));
	EXPECT_EQ(0, listControl->GetActive());
	EXPECT_EQ(0, ListControltestMethods::GetIndexModeIndex(listControl));
	listControl->SetActive(1); //set active
	EXPECT_EQ(1, ListControltestMethods::GetIndexModeIndex(listControl));
	EXPECT_EQ(1, listControl->GetActive());
	ListControltestMethods::SetIndexModeIndex(listControl, 0);
	EXPECT_EQ(0, listControl->GetActive());
	EXPECT_EQ(0, ListControltestMethods::GetIndexModeIndex(listControl));
	ListControltestMethods::SetIndexModeIndex(listControl, 1); //set index
	EXPECT_EQ(1, listControl->GetActive());
	EXPECT_EQ(1, ListControltestMethods::GetIndexModeIndex(listControl));
	ListControltestMethods::SetIndexModeIndex(listControl, 99); //set bad index, should be clampped
	EXPECT_EQ(1, listControl->GetActive());
	EXPECT_EQ(1, ListControltestMethods::GetIndexModeIndex(listControl));
	ListControltestMethods::SetIndexModeIndex(listControl, -2); //set bad index, should be clampped
	EXPECT_EQ(0, listControl->GetActive());
	EXPECT_EQ(0, ListControltestMethods::GetIndexModeIndex(listControl));

	// setIndexByName
	
	EXPECT_FALSE(listControl->SetIndexByName(L"aaa"));
	EXPECT_EQ(0, ListControltestMethods::GetIndexModeIndex(listControl)); //no change
	EXPECT_FALSE(listControl->SetIndexByName(L""));
	EXPECT_EQ(0, ListControltestMethods::GetIndexModeIndex(listControl)); //no change
	EXPECT_TRUE(listControl->SetIndexByName(L"2"));
	EXPECT_EQ(1, ListControltestMethods::GetIndexModeIndex(listControl)); //match "control2"
	EXPECT_EQ(1, listControl->GetActive());
	EXPECT_TRUE(listControl->SetIndexByName(L"1"));
	EXPECT_EQ(0, ListControltestMethods::GetIndexModeIndex(listControl)); //match "control1"
	EXPECT_EQ(0, listControl->GetActive());
	EXPECT_TRUE(listControl->SetIndexByName(L"2"));
	EXPECT_TRUE(listControl->SetIndexByName(L"CONT")); //match the first one, case-insensitive
	EXPECT_EQ(0, ListControltestMethods::GetIndexModeIndex(listControl)); //match "control1"
	EXPECT_EQ(0, listControl->GetActive());
	// switch back to weight and this should do nothing to the active control...
	ListControltestMethods::SetIndexMode(listControl, false);
	listControl->SetActive(2);
	ListControltestMethods::SetIndexMode(listControl, true);
	// index mode index should be unaffected
	EXPECT_EQ(0, ListControltestMethods::GetIndexModeIndex(listControl));
	EXPECT_EQ(0, listControl->GetActive());

	// KZ, Disable the testing of cut/paste/delete due to it causing crashes in undo system from a gtest.
	// There is maxscript coverage for this part in listctrl.Unittest.ms
	
	//// cut paste delete (check active control in both modes
	//for (int i = 0; i <= 1; i++)
	//{
	//	//Setup
	//	if (i == 0)
	//	{
	//		// Weight mode
	//		ListControltestMethods::SetIndexModeIndex(listControl, false);
	//	}
	//	else
	//	{
	//		// index mode
	//		ListControltestMethods::SetIndexModeIndex(listControl, true);
	//	}
	//	listControl->SetActive(1);
	//	EXPECT_EQ(1, listControl->GetActive());

	//	// Manipulate the list and check list and active control
	//	EXPECT_EQ(control1, listControl->GetSubCtrl(0));
	//	EXPECT_EQ(control2, listControl->GetSubCtrl(1));
	//	listControl->CutItem(2); //do nothing
	//	EXPECT_EQ(control1, listControl->GetSubCtrl(0));
	//	EXPECT_EQ(control2, listControl->GetSubCtrl(1));
	//	EXPECT_EQ(1, listControl->GetActive());
	//	listControl->CutItem(-1); //do nothing
	//	EXPECT_EQ(control1, listControl->GetSubCtrl(0));
	//	EXPECT_EQ(control2, listControl->GetSubCtrl(1));
	//	EXPECT_EQ(1, listControl->GetActive());
	//	// cut
	//	listControl->CutItem(1);
	//	EXPECT_EQ(control1, listControl->GetSubCtrl(0));
	//	EXPECT_EQ(nullptr, listControl->GetSubCtrl(1));
	//	EXPECT_EQ(0, listControl->GetActive());
	//	listControl->PasteItem(0);
	//	EXPECT_EQ(control2, listControl->GetSubCtrl(0));
	//	EXPECT_EQ(control1, listControl->GetSubCtrl(1));
	//	// the active index in both modes should get bumpped
	//	EXPECT_EQ(1, listControl->GetActive());

	//	// paste
	//	listControl->SetActive(1);
	//	EXPECT_EQ(1, listControl->GetActive());
	//	listControl->CutItem(0);
	//	EXPECT_EQ(control1, listControl->GetSubCtrl(0));
	//	EXPECT_EQ(nullptr, listControl->GetSubCtrl(1));
	//	EXPECT_EQ(0, listControl->GetActive());
	//	listControl->PasteItem(2); //do nothing
	//	EXPECT_EQ(control1, listControl->GetSubCtrl(0));
	//	EXPECT_EQ(nullptr, listControl->GetSubCtrl(1));
	//	EXPECT_EQ(0, listControl->GetActive());
	//	listControl->PasteItem(-1); //do nothing
	//	EXPECT_EQ(control1, listControl->GetSubCtrl(0));
	//	EXPECT_EQ(nullptr, listControl->GetSubCtrl(1));
	//	EXPECT_EQ(0, listControl->GetActive());
	//	listControl->PasteItem(1);
	//	EXPECT_EQ(control1, listControl->GetSubCtrl(0));
	//	EXPECT_EQ(control2, listControl->GetSubCtrl(1));
	//	EXPECT_EQ(0, listControl->GetActive());
	//	// delete
	//	listControl->SetActive(1);
	//	EXPECT_EQ(1, listControl->GetActive());
	//	listControl->DeleteItem(-1); //do nothing
	//	EXPECT_EQ(control1, listControl->GetSubCtrl(0));
	//	EXPECT_EQ(control2, listControl->GetSubCtrl(1));
	//	EXPECT_EQ(1, listControl->GetActive());
	//	listControl->DeleteItem(2); //do nothing
	//	EXPECT_EQ(control1, listControl->GetSubCtrl(0));
	//	EXPECT_EQ(control2, listControl->GetSubCtrl(1));
	//	EXPECT_EQ(1, listControl->GetActive());
	//	listControl->DeleteItem(0);
	//	EXPECT_EQ(control2, listControl->GetSubCtrl(0));
	//	EXPECT_EQ(nullptr, listControl->GetSubCtrl(1));
	//	EXPECT_EQ(0, listControl->GetActive());
	//	listControl->DeleteItem(0);
	//	EXPECT_EQ(nullptr, listControl->GetSubCtrl(0));
	//	EXPECT_EQ(nullptr, listControl->GetSubCtrl(1));
	//	EXPECT_EQ(0, listControl->GetActive()); // A default of 1

	//	// Add them back for the next pass
	//	listControl->AssignController(control1, 0);
	//	listControl->AssignController(control2, 1);
	//}

	// check boxes
	ListControltestMethods::SetIndexMode(listControl, false);
	EXPECT_EQ(false, ListControltestMethods::GetAverage(listControl));
	ListControltestMethods::SetAverage(listControl, true);
	EXPECT_EQ(true, ListControltestMethods::GetAverage(listControl));
	ListControltestMethods::SetIndexMode(listControl, true); //average is not used in index mode but still can be checked
	EXPECT_EQ(true, ListControltestMethods::GetAverage(listControl));
	ListControltestMethods::SetAverage(listControl, false);
	EXPECT_EQ(false, ListControltestMethods::GetAverage(listControl));
	
	// Only TM_list has this option
	if (listControl->SuperClassID() == CTRL_MATRIX3_CLASS_ID)
	{
		ListControltestMethods::SetIndexMode(listControl, false);
		EXPECT_EQ(false, ListControltestMethods::GetSequential(listControl));
		ListControltestMethods::SetSequential(listControl, true);
		EXPECT_EQ(true, ListControltestMethods::GetSequential(listControl));
		ListControltestMethods::SetIndexMode(listControl, true); //sequential is not used in index mode but still can be checked
		EXPECT_EQ(true, ListControltestMethods::GetSequential(listControl));
		ListControltestMethods::SetSequential(listControl, false);
		EXPECT_EQ(false, ListControltestMethods::GetSequential(listControl));
		
		ListControltestMethods::SetIndexMode(listControl, false);
		EXPECT_EQ(false, ListControltestMethods::GetWeightAgainstMode(listControl));
		ListControltestMethods::SetWeightAgainstMode(listControl, true);
		EXPECT_EQ(true, ListControltestMethods::GetWeightAgainstMode(listControl));
		ListControltestMethods::SetIndexMode(listControl, true); //weightAgainst is not used in index mode but still can be checked
		EXPECT_EQ(true, ListControltestMethods::GetWeightAgainstMode(listControl));
		ListControltestMethods::SetWeightAgainstMode(listControl, false);
		EXPECT_EQ(false, ListControltestMethods::GetWeightAgainstMode(listControl));
	}

	//Check tags
	EXPECT_EQ(_T(""), listControl->GetTag());
	listControl->SetTag(_T("new tag"));
	EXPECT_EQ(_T("new tag"),listControl->GetTag());
}

TEST(listctrl, List_APICoverage)
{
	theHold.Suspend();
	SetCOREInterface(new DefaultMockCoreInterface);
	
	//Sanity check for the list controller API
	IListControl* float_list = reinterpret_cast<IListControl*>(GetFloatListDesc()->Create(false));
	TestListControlAPI(float_list, CreateInterpFloat(), CreateInterpFloat());

	IListControl* point3_list = reinterpret_cast<IListControl*>(GetPoint3ListDesc()->Create(false));
	TestListControlAPI(point3_list, CreateInterpPoint3(), CreateInterpPoint3());

	IListControl* point4_list = reinterpret_cast<IListControl*>(GetPoint4ListDesc()->Create(false));
	TestListControlAPI(point4_list, CreateInterpPoint4(), CreateInterpPoint4());

	IListControl* pos_list = reinterpret_cast<IListControl*>(GetPositionListDesc()->Create(false));
	TestListControlAPI(pos_list, CreateInterpPosition(), CreateInterpPosition());

	IListControl* rot_list = reinterpret_cast<IListControl*>(GetRotationListDesc()->Create(false));
	TestListControlAPI(rot_list, CreateInterpRotation(), CreateInterpRotation());

	IListControl* scale_list = reinterpret_cast<IListControl*>(GetScaleListDesc()->Create(false));
	TestListControlAPI(scale_list, CreateInterpScale(), CreateInterpScale());

	IListControl* tm_list = reinterpret_cast<IListControl*>(GetTransformListDesc()->Create(false));
	TestListControlAPI(tm_list, CreatePRSControl(), CreatePRSControl());
	
	SetCOREInterface(nullptr);
	theHold.Resume();
}

TEST(listctrl, List_GetSetAllTypesModes)
{
	// All list control types
	// Get Set relative and absolute
	// index and weight mode
	theHold.Suspend();
	SetCOREInterface(new DefaultMockCoreInterface);

	auto setWeights = [](Control* listControl, float weight1, float weight2)
	{
		listControl->GetParamBlock(0)->SetValue(kListCtrlWeight, 0, weight1, 0);
		listControl->GetParamBlock(0)->SetValue(kListCtrlWeight, 0, weight2, 1);
	};

	auto AssignSubControllers = [](IListControl* listControl, Control* control1, Control* control2)
	{
		listControl->AssignController(control1, 0);
		listControl->AssignController(control2, 1);
	};

	auto testValues = [](IListControl* listControl, void (*compareValues)(void*, void*), void* expectedVal, void* testValue)
	{
		listControl->GetValue(0, testValue, CTRL_ABSOLUTE);
		compareValues(expectedVal, testValue);
	};

	auto testIndexMode = [&testValues](IListControl* listControl, void* val1, void* val2, void* testValue, void (*compareValues)(void*,void*))
	{
		ListControltestMethods::SetIndexMode(listControl, true);
		EXPECT_TRUE(ListControltestMethods::GetIndexMode(listControl));
		
		listControl->SetActive(0);
		listControl->SetValue(0, val1);
		testValues(listControl, compareValues, val1, testValue);
		listControl->SetActive(1);
		listControl->SetValue(0, val2);
		testValues(listControl, compareValues, val2, testValue);
		listControl->SetActive(0);
		testValues(listControl, compareValues, val1, testValue);
	};

	auto testWeightMode = [&testValues, &setWeights](IListControl* listControl, void* val1, void* val2, void* combinedVal, void* testValue, void (*compareValues)(void*, void*))
	{
		ListControltestMethods::SetIndexMode(listControl, false);
		EXPECT_FALSE(ListControltestMethods::GetIndexMode(listControl));

		//set value, currently zeroed
		listControl->SetActive(1);
		listControl->SetValue(0, val2);
		testValues(listControl, compareValues, val2, testValue);

		listControl->SetActive(0);
		if(listControl->SuperClassID() == CTRL_ROTATION_CLASS_ID)
		{
			// val1 needs to be an AxisAng rather than a Quat.
			AngAxis aa(*(Quat*)val1);
			listControl->SetValue(0, &aa, 1, CTRL_RELATIVE); // rotation this needs to be AxisAng.
		}
		else
		{
			listControl->SetValue(0, val1, 1, CTRL_RELATIVE);
		}
		testValues(listControl, compareValues, combinedVal, testValue); //relative so it'll be the combined value.
		
		listControl->SetActive(0);
		listControl->SetValue(0, combinedVal, 1, CTRL_ABSOLUTE);
		testValues(listControl, compareValues, combinedVal, testValue); //final result should be val1 because its absolute
		
		setWeights(listControl, 1.0f, 0.0f);
		testValues(listControl, compareValues, val1, testValue);

		setWeights(listControl, 1.0f, 0.0f);
		testValues(listControl, compareValues, val1, testValue);

		setWeights(listControl, 0.0f, 1.0f);
		testValues(listControl, compareValues, val2, testValue);

		setWeights(listControl, 1.0f, 1.0f);
		testValues(listControl, compareValues, combinedVal, testValue);
	};

	auto emptyListTests = [&testValues](IListControl* listControl, void* val, void* expected, void (*compareValues)(void*, void*))
	{
		// with an empty list controller, make sure that is bahves graceflly with GetValue and SetValue
		EXPECT_EQ(0, listControl->GetListCount());

		ListControltestMethods::SetIndexMode(listControl, false); //weight mode
		ListControltestMethods::SetAverage(listControl, false); // average off
		listControl->SetValue(0, val, CTRL_ABSOLUTE);
		testValues(listControl, compareValues, expected, val);
		listControl->SetValue(0, val, CTRL_RELATIVE);
		testValues(listControl, compareValues, expected, val);
		ListControltestMethods::SetAverage(listControl, true);  // average on
		listControl->SetValue(0, val, CTRL_ABSOLUTE);
		testValues(listControl, compareValues, expected, val);
		listControl->SetValue(0, val, CTRL_RELATIVE);
		testValues(listControl, compareValues, expected, val);
		ListControltestMethods::SetIndexMode(listControl, true); //index mode
		listControl->SetValue(0, val, CTRL_ABSOLUTE);
		testValues(listControl, compareValues, expected, val);
		listControl->SetValue(0, val, CTRL_RELATIVE);
		testValues(listControl, compareValues, expected, val);
	};

	// Call SetValue for each controller type
	{
		auto comparer = [](void* val1, void* val2)
		{
			float* v1 = static_cast<float*>(val1);
			float* v2 = static_cast<float*>(val2);
			EXPECT_NEAR(*v1, *v2, epsilon);
		};
		{
			//Index mode
			IListControl* float_list = reinterpret_cast<IListControl*>(GetFloatListDesc()->Create(false));
			AssignSubControllers(float_list, CreateInterpFloat(), CreateInterpFloat());

			float val1 = 100.f;
			float val2 = -100.f;
			float testValue = 0.f; //needed to hold getvalue results
			testIndexMode(float_list, &val1, &val2, &testValue, comparer);
		}
		{
			// Test weight mode
			IListControl* float_list = reinterpret_cast<IListControl*>(GetFloatListDesc()->Create(false));
			AssignSubControllers(float_list, CreateInterpFloat(), CreateInterpFloat());

			float val1 = 100.f;
			float val2 = -100.f;
			float testValue = 0.f; //needed to hold getvalue results
			float combinedVal = val1 + val2;
			testWeightMode(float_list, &val1, &val2, &combinedVal, &testValue, comparer);
		}
		{
			// Empty list GetValue, SetValue
			IListControl* float_list = reinterpret_cast<IListControl*>(GetFloatListDesc()->Create(false));
			float testVal = 0.0f;
			float expectedVal = 0.0f;
			emptyListTests(float_list, &testVal, &expectedVal, comparer);
		}
	}

	{
		auto comparer = [](void* val1, void* val2)
		{
			Point3* v1 = static_cast<Point3*>(val1);
			Point3* v2 = static_cast<Point3*>(val2);
			if(!v1->Equals(*v2, epsilon))
			{
				EXPECT_TRUE(v1->Equals(*v2, epsilon));
			}
		};
		{
			//Index mode
			IListControl* point3_list = reinterpret_cast<IListControl*>(GetPoint3ListDesc()->Create(false));
			AssignSubControllers(point3_list, CreateInterpPoint3(), CreateInterpPoint3());

			Point3 val1(1,5,10);
			Point3 val2(20,20,20);
			Point3 testValue; //needed to hold getvalue results
			testIndexMode(point3_list, &val1, &val2, &testValue, comparer);
		}
		{
			// Test weight mode
			IListControl* point3_list = reinterpret_cast<IListControl*>(GetPoint3ListDesc()->Create(false));
			AssignSubControllers(point3_list, CreateInterpPoint3(), CreateInterpPoint3());

			Point3 val1(1, 5, 10);
			Point3 val2(20, 20, 20);
			Point3 testValue; //needed to hold getvalue results
			Point3 combinedVal = val1 + val2;
			testWeightMode(point3_list, &val1, &val2, &combinedVal, &testValue, comparer);
		}
		{
			// Empty list GetValue, SetValue
			IListControl* point3_list = reinterpret_cast<IListControl*>(GetPoint3ListDesc()->Create(false));
			Point3 testVal;
			Point3 expectedVal;
			emptyListTests(point3_list, &testVal, &expectedVal, comparer);
		}
	}

	{
		auto comparer = [](void* val1, void* val2)
		{
			Point4* v1 = static_cast<Point4*>(val1);
			Point4* v2 = static_cast<Point4*>(val2);
			if (!v1->Equals(*v2, epsilon))
			{
				EXPECT_TRUE(v1->Equals(*v2, epsilon));
			}
		};
		{
			//Index mode
			IListControl* point4_list = reinterpret_cast<IListControl*>(GetPoint4ListDesc()->Create(false));
			AssignSubControllers(point4_list, CreateInterpPoint4(), CreateInterpPoint4());

			Point4 val1(1, 5, 10, 100);
			Point4 val2(20, 20, 20, 100);
			Point4 testValue; //needed to hold getvalue results
			testIndexMode(point4_list, &val1, &val2, &testValue, comparer);
		}
		{
			// Test weight mode
			IListControl* point4_list = reinterpret_cast<IListControl*>(GetPoint4ListDesc()->Create(false));
			// The default value of these RGBA controllers is not 0,0,0,1. So lets fix that for this test.
			Point4 defaultVal(0, 0, 0, 0);
			Control* subC1 = CreateInterpPoint4();
			subC1->SetValue(0, &defaultVal);
			Control* subC2 = CreateInterpPoint4();
			subC2->SetValue(0, &defaultVal);
			AssignSubControllers(point4_list, subC1, subC2);

			Point4 val1(1, 5, 10, -100);
			Point4 val2(20, 20, 20, -100);
			Point4 testValue; //needed to hold getvalue results
			Point4 combinedVal = val1 + val2;
			testWeightMode(point4_list, &val1, &val2, &combinedVal, &testValue, comparer);
		}
		{
			// Empty list GetValue, SetValue
			IListControl* point4_list = reinterpret_cast<IListControl*>(GetPoint4ListDesc()->Create(false));
			Point4 testVal;
			Point4 expectedVal;
			emptyListTests(point4_list, &testVal, &expectedVal, comparer);
		}
	}

	{
		auto comparer = [](void* val1, void* val2)
		{
			Point3* v1 = static_cast<Point3*>(val1);
			Point3* v2 = static_cast<Point3*>(val2);
			if (!v1->Equals(*v2, epsilon))
			{
				EXPECT_TRUE(v1->Equals(*v2, epsilon));
			}
		};
		{
			//Index mode
			IListControl* pos_list = reinterpret_cast<IListControl*>(GetPositionListDesc()->Create(false));
			AssignSubControllers(pos_list, CreateInterpPosition(), CreateInterpPosition());

			Point3 val1(1, 5, 10);
			Point3 val2(20, 20, 20);
			Point3 testValue; //needed to hold getvalue results
			testIndexMode(pos_list, &val1, &val2, &testValue, comparer);
		}
		{
			// Test weight mode
			IListControl* pos_list = reinterpret_cast<IListControl*>(GetPositionListDesc()->Create(false));
			AssignSubControllers(pos_list, CreateInterpPosition(), CreateInterpPosition());

			Point3 val1(1, 5, 10);
			Point3 val2(20, 20, 20);
			Point3 testValue; //needed to hold getvalue results
			Point3 combinedVal = val1 + val2;
			testWeightMode(pos_list, &val1, &val2, &combinedVal, &testValue, comparer);
		}
		{
			// Empty list GetValue, SetValue
			IListControl* pos_list = reinterpret_cast<IListControl*>(GetPositionListDesc()->Create(false));
			Point3 testVal;
			Point3 expectedVal;
			emptyListTests(pos_list, &testVal, &expectedVal, comparer);
		}
	}

	{
		auto comparer = [](void* val1, void* val2)
		{
			Quat* v1 = static_cast<Quat*>(val1);
			Quat* v2 = static_cast<Quat*>(val2);
			if (!v1->Equals(*v2, epsilon))
			{
				EXPECT_TRUE(v1->Equals(*v2, epsilon));
			}
		};
		{
			//Index mode
			IListControl* rot_list = reinterpret_cast<IListControl*>(GetRotationListDesc()->Create(false));
			AssignSubControllers(rot_list, CreateInterpRotation(), CreateInterpRotation());

			Quat val1(AngAxis(Point3(1,0,0), 90.f  * PI / 180.f));
			Quat val2(AngAxis(Point3(0,1,0), 90.f * PI / 180.f));
			Quat testValue; //needed to hold getvalue results
			testIndexMode(rot_list, &val1, &val2, &testValue, comparer);
		}
		{
			// Test weight mode
			IListControl* rot_list = reinterpret_cast<IListControl*>(GetRotationListDesc()->Create(false));
			AssignSubControllers(rot_list, CreateInterpRotation(), CreateInterpRotation());

			Quat val1(AngAxis(Point3(1, 0, 0), 90.f * PI / 180.f));
			Quat val2(AngAxis(Point3(0, 1, 0), 90.f * PI / 180.f));
			Quat testValue; //needed to hold getvalue results
			Quat combinedVal = val2 * val1;
			testWeightMode(rot_list, &val1, &val2, &combinedVal, &testValue, comparer);
		}
		{
			// Empty list GetValue, SetValue
			IListControl* rot_list = reinterpret_cast<IListControl*>(GetRotationListDesc()->Create(false));
			Quat testVal;
			Quat expectedVal;
			emptyListTests(rot_list, &testVal, &expectedVal, comparer);
		}
	}

	{
		auto comparer = [](void* val1, void* val2)
		{
			ScaleValue* v1 = static_cast<ScaleValue*>(val1);
			ScaleValue* v2 = static_cast<ScaleValue*>(val2);
			if (!v1->Equals(*v2, epsilon))
			{
				EXPECT_TRUE(v1->Equals(*v2, epsilon));
			}
		};
		{
			//Index mode
			IListControl* scale_list = reinterpret_cast<IListControl*>(GetScaleListDesc()->Create(false));
			AssignSubControllers(scale_list, CreateInterpScale(), CreateInterpScale());

			ScaleValue val1(Point3(1.f,2.f,0.5f));
			ScaleValue val2(Point3(0.5f, -0.5f, 0.5f));
			ScaleValue testValue; //needed to hold getvalue results
			testIndexMode(scale_list, &val1, &val2, &testValue, comparer);
		}
		{
			// Test weight mode
			// Scale controllers work differently than the others.
			// When setting a value it scales the sub controller value with respect to the final result.
			IListControl* scale_list = reinterpret_cast<IListControl*>(GetScaleListDesc()->Create(false));
			AssignSubControllers(scale_list, CreateInterpScale(), CreateInterpScale());

			ScaleValue val1(Point3(1.f, 2.f, 0.5f));
			ScaleValue val2(Point3(0.5f, -0.5f, 0.5f));
			ScaleValue testValue; //needed to hold getvalue results

			ListControltestMethods::SetIndexMode(scale_list, false);
			EXPECT_FALSE(ListControltestMethods::GetIndexMode(scale_list));

			//set value, currently zeroed
			scale_list->SetActive(1);
			scale_list->SetValue(0, &val2);
			testValues(scale_list, comparer, &val2, &testValue);

			scale_list->SetActive(0);
			scale_list->SetValue(0, &val1, 1, CTRL_RELATIVE);
			ScaleValue combinedVal(val1.s* val2.s);
			testValues(scale_list, comparer, &combinedVal, &testValue); //relative so it'll be the combined value.

			scale_list->SetActive(0);
			ScaleValue doubleScale(Point3(2,2,2)); //double will counter act val2
			scale_list->SetValue(0, &doubleScale, 1, CTRL_ABSOLUTE);
			{
				scale_list->GetValue(0, &testValue, CTRL_ABSOLUTE);
				comparer(&val2, &testValue);
			}

			setWeights(scale_list, 1.0f, 0.0f);
			ScaleValue noScale(Point3(1, 1, 1));
			testValues(scale_list, comparer, &noScale, &testValue);

			setWeights(scale_list, 0.0f, 1.0f);
			testValues(scale_list, comparer, &val2, &testValue);

			setWeights(scale_list, 1.0f, 1.0f);
			testValues(scale_list, comparer, &val2, &testValue);
		}
		{
			// Empty list GetValue, SetValue
			IListControl* scale_list = reinterpret_cast<IListControl*>(GetScaleListDesc()->Create(false));
			ScaleValue testVal(Point3(1,1,1));
			ScaleValue expectedVal(Point3(1, 1, 1));
			emptyListTests(scale_list, &testVal, &expectedVal, comparer);
		}
	}

	{
		IListControl* tm_list = reinterpret_cast<IListControl*>(GetTransformListDesc()->Create(false));
		AssignSubControllers(tm_list, CreatePRSControl(), CreatePRSControl());

		auto comparer = [](void* val1, void* val2)
		{
			Matrix3* v1 = static_cast<Matrix3*>(val1);
			Matrix3* v2 = static_cast<Matrix3*>(val2);
			if (!v1->Equals(*v2, epsilon))
			{
				EXPECT_TRUE(v1->Equals(*v2, epsilon));
			}
		};
		{
			//Index mode
			IListControl* tm_list = reinterpret_cast<IListControl*>(GetTransformListDesc()->Create(false));
			AssignSubControllers(tm_list, CreatePRSControl(), CreatePRSControl());
			
			ListControltestMethods::SetIndexMode(tm_list, true);
			EXPECT_TRUE(ListControltestMethods::GetIndexMode(tm_list));

			Interval dummyInterval;
			AngAxis rot = Quat(AngAxis(Point3(1, 0, 0), 45 / 180.f * PI));
			Matrix3 expectedTm;

			// Rotate
			Quat(rot).MakeMatrix(expectedTm);
			{
				// rotation packet
				SetXFormPacket newPacket(rot, 1, Matrix3(), Matrix3());
				tm_list->SetActive(0);
				tm_list->SetValue(0, &newPacket);
				Matrix3 result;
				tm_list->GetValue(0, &result, CTRL_RELATIVE);
				comparer(&expectedTm, &result);
			}

			// Move
			Point3 trans(10, 0, 0);
			expectedTm.SetTrans(trans);
			{
				//translation packet
				SetXFormPacket newPacket(trans,Matrix3(), Matrix3());
				tm_list->SetActive(0);
				tm_list->SetValue(0, &newPacket);
				Matrix3 result;
				tm_list->GetValue(0, &result, CTRL_RELATIVE);
				comparer(&expectedTm, &result);
			}

			// Scale
			Point3 scale(1.f, 2.f, 1.f);
			Matrix3 transTm;
			transTm.SetTrans(trans);
			Matrix3 rotTm;
			Quat(rot).MakeMatrix(rotTm);
			Matrix3 scaleTm;
			scaleTm.SetScale(scale);
			expectedTm = transTm * rotTm * scaleTm;
			{
				//translation packet
				SetXFormPacket newPacket(scale, 1, Matrix3(), Matrix3());
				tm_list->SetActive(0);
				tm_list->SetValue(0, &newPacket);
				Matrix3 result;
				tm_list->GetValue(0, &result, CTRL_RELATIVE);
				comparer(&expectedTm, &result);
			}

			//set XForm
			{
				Point3 trans2(0,12,34);
				Quat rot2(AngAxis(Point3(0,0,1), PI/2.f));
				Matrix3 mat2;
				rot2.MakeMatrix(mat2);
				mat2.SetTrans(trans2);

				//xform packet
				SetXFormPacket newPacket(mat2, Matrix3());
				tm_list->SetActive(0);
				tm_list->SetValue(0, &newPacket);
				Matrix3 result;
				tm_list->GetValue(0, &result, CTRL_RELATIVE);
				comparer(&mat2, &result);
				
			}
		}
		{
			// TODO:
			//// Test weight mode
			//IListControl* tm_list = reinterpret_cast<IListControl*>(GetTransformListDesc()->Create(false));
			//AssignSubControllers(tm_list, CreatePRSControl(), CreatePRSControl());

			//Matrix3 val1(AngAxis(Point3(1, 0, 0), 90.f * PI / 180.f));
			//Matrix3 val2(AngAxis(Point3(0, 1, 0), 90.f * PI / 180.f));
			//Matrix3 testValue; //needed to hold getvalue results
			//Matrix3 combinedVal = val2 * val1;
			//testWeightMode(rot_list, &val1, &val2, &combinedVal, &testValue, comparer);
		}
		{
			// Empty list GetValue, SetValue
			IListControl* tm_list = reinterpret_cast<IListControl*>(GetTransformListDesc()->Create(false));

			auto doAGetValue = [tm_list, &comparer]()
			{
				//Get value
				Matrix3 result;
				Matrix3 expectedVal;
				tm_list->GetValue(0, &result, CTRL_RELATIVE);
				comparer(&expectedVal, &result);
			};

			auto doASetValue = [tm_list]()
			{
				//translation packet
				Point3 trans(10, 0, 0);
				SetXFormPacket newPacket(trans, Matrix3(), Matrix3());
				tm_list->SetActive(0);
				tm_list->SetValue(0, &newPacket);
			};

			EXPECT_EQ(0, tm_list->GetListCount());
			ListControltestMethods::SetIndexMode(tm_list, false); //weight mode
			ListControltestMethods::SetAverage(tm_list, false); // average off
			doASetValue();
			doAGetValue();
			ListControltestMethods::SetAverage(tm_list, true);  // average on
			doASetValue();
			doAGetValue();
			ListControltestMethods::SetIndexMode(tm_list, true); //index mode
			doASetValue();
			doAGetValue();
		}
	}
	

	SetCOREInterface(nullptr);
	theHold.Resume();
}

TEST(listctrl, TMList_TMControlInterface)
{
	IListControl* tm_list = reinterpret_cast<IListControl*>(GetTransformListDesc()->Create(false));
	Control* prs1 = CreatePRSControl();
	Control* prs2 = CreatePRSControl();
	tm_list->AssignController(prs1, 0);
	tm_list->AssignController(prs2, 1);

	EXPECT_NE(prs1->GetPositionController(), nullptr);
	EXPECT_NE(prs2->GetPositionController(), nullptr);
	EXPECT_NE(prs1->GetRotationController(), nullptr);
	EXPECT_NE(prs2->GetRotationController(), nullptr);
	EXPECT_NE(prs1->GetScaleController(), nullptr);
	EXPECT_NE(prs2->GetScaleController(), nullptr);

	//Test the inheritance flags also
	EXPECT_EQ(prs1->GetInheritanceFlags(), DWORD(0));
	prs1->SetInheritanceFlags(INHERIT_SCL_Y, FALSE);
	EXPECT_TRUE(prs1->GetInheritanceFlags() != 0);

	// Make sure that the ml list hooked up to the active subn controller properly.
	EXPECT_EQ(prs1->GetPositionController(), tm_list->GetPositionController());
	EXPECT_EQ(prs1->GetRotationController(), tm_list->GetRotationController());
	EXPECT_EQ(prs1->GetScaleController(), tm_list->GetScaleController());
	EXPECT_EQ(prs1->GetInheritanceFlags(), tm_list->GetInheritanceFlags());
	tm_list->SetActive(1);
	EXPECT_EQ(prs2->GetPositionController(), tm_list->GetPositionController());
	EXPECT_EQ(prs2->GetRotationController(), tm_list->GetRotationController());
	EXPECT_EQ(prs2->GetScaleController(), tm_list->GetScaleController());
	EXPECT_EQ(prs2->GetInheritanceFlags(), tm_list->GetInheritanceFlags());

	tm_list->SetInheritanceFlags(INHERIT_ROT_Y, FALSE); // Should set PRS2's flags
	EXPECT_TRUE(prs2->GetInheritanceFlags() != 0);
	EXPECT_EQ(prs2->GetInheritanceFlags(), tm_list->GetInheritanceFlags());
	
}

} // namespace

#endif
