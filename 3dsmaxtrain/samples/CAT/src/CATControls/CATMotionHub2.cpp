//////////////////////////////////////////////////////////////////////////////
//
//  Copyright 2009 Autodesk, Inc.  All rights reserved.
//  Copyright 2003 Character Animation Technologies.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.   
//
//////////////////////////////////////////////////////////////////////////////

#include "CatPlugins.h"
#include "ICATParent.h"

 // Rig Structure
#include "LimbData2.h"
#include "SpineData2.h"
#include "Hub.h"
#include "ArbBoneTrans.h"

// CATMotion
#include "CATHierarchyBranch2.h"

#include "CATMotionHub2.h"
#include "CATMotionLimb.h"
#include "CATMotionTail.h"
#include "CATp3.h"

#include "WeightShift.h"

class CATMotionHub2ClassDesc :public ClassDesc2 {
public:
	int 			IsPublic() { return FALSE; }
	void*			Create(BOOL loading = FALSE) { UNREFERENCED_PARAMETER(loading); return new CATMotionHub2(); }
	const TCHAR*	ClassName() { return GetString(IDS_CL_CATMOTIONHUB2); }
	const TCHAR*	NonLocalizedClassName() { return _T("CATMotionHub2"); }
	SClass_ID		SuperClassID() { return CTRL_MATRIX3_CLASS_ID; }
	Class_ID		ClassID() { return CATMOTIONHUB2_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }

	const TCHAR*	InternalName() { return _T("CATMotionHub2"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle

};

static CATMotionHub2ClassDesc CATMotionHub2Desc;
ClassDesc2* GetCATMotionHub2Desc() { return &CATMotionHub2Desc; }

//weightshift_param_blk
static ParamBlockDesc2 CATMotionHub2_param_blk(CATMotionHub2::PBLOCK_REF, _T("CATMotionHub2 params"), 0, &CATMotionHub2Desc,
	P_AUTO_CONSTRUCT, CATMotionHub2::PBLOCK_REF,
	CATMotionHub2::PB_HUB, _T("Hub"), TYPE_REFTARG, P_NO_REF, 0,
		p_end,
	CATMotionHub2::PB_P3OFFSETROT, _T("OffsetRot"), TYPE_POINT3, P_ANIMATABLE, IDS_OFFSETROT,
		p_end,
	CATMotionHub2::PB_P3OFFSETPOS, _T("OffsetRot"), TYPE_POINT3, P_ANIMATABLE, IDS_OFFSETPOS,
		p_nonLocalizedName, _T("OffsetPos"),
		p_end,
	CATMotionHub2::PB_P3MOTIONROT, _T("MotionRot"), TYPE_POINT3, P_ANIMATABLE, IDS_CATMOTIONROT,
		p_nonLocalizedName, _T("CATMotionRot"),
		p_end,
	CATMotionHub2::PB_P3MOTIONPOS, _T("MotionPos"), TYPE_POINT3, P_ANIMATABLE, IDS_CATMOTIONPOS,
		p_nonLocalizedName, _T("CATMotionPos"),
		p_end,
	CATMotionHub2::PB_ROOT, _T("CATMotionRoot"), TYPE_REFTARG, P_NO_REF, 0,
		p_end,
	CATMotionHub2::PB_CATMOTIONHUB_TAB, _T("CATMotionHubs"), TYPE_REFTARG_TAB, 0, P_VARIABLE_SIZE + P_NO_REF, IDS_CL_CATMOTIONHUB,
		p_nonLocalizedName, _T("CATMotionHub"),
		p_end,
	CATMotionHub2::PB_CATMOTIONLIMB_TAB, _T("CATMotionLimbs"), TYPE_REFTARG_TAB, 0, P_VARIABLE_SIZE + P_NO_REF, IDS_CL_CATMOTIONLIMB,
		p_nonLocalizedName, _T("CATMotionLimb"),
		p_end,
	CATMotionHub2::PB_CATMOTIONTAIL_TAB, _T("CATMotionTails"), TYPE_REFTARG_TAB, 0, P_VARIABLE_SIZE + P_NO_REF, IDS_CL_CATMOTIONTAIL,
		p_nonLocalizedName, _T("CATMotionTail"),
		p_end,
	p_end
);

CATMotionHub2::CATMotionHub2() {
	pathoffset = 0.0f;
	pblock = NULL;
	CATMotionHub2Desc.MakeAutoParamBlocks(this);
}

CATMotionHub2::~CATMotionHub2() {
	DeleteAllRefs();
}

RefTargetHandle CATMotionHub2::Clone(RemapDir& remap)
{
	CATMotionHub2 *newCATMotionHub2 = new CATMotionHub2();
	remap.AddEntry(this, newCATMotionHub2);

	// Post patch registered in CloneParamBlock that handles the omitted P_NO_REF.
	IParamBlock2* clonedpblock = CloneParamBlock(pblock, remap);
	newCATMotionHub2->ReplaceReference(PBLOCK_REF, clonedpblock);

	// Clone manually the P_NO_REF's (previously these were not necessarily cloned here. Possible bugs introduced here).
	// PB_HUB, handled in back patch
	// PB_ROOT, handled in back patch
	// PB_CATMOTIONLIMB_TAB, originally in post patch, clone explicitly because we own them
	const int numLimbs = pblock->Count(PB_CATMOTIONLIMB_TAB);
	newCATMotionHub2->pblock->SetCount(PB_CATMOTIONLIMB_TAB, numLimbs);
	for (int i = 0; i < numLimbs; ++i)
	{
		ReferenceTarget* limb = pblock->GetReferenceTarget(PB_CATMOTIONLIMB_TAB, 0, i);
		newCATMotionHub2->pblock->SetValue(PB_CATMOTIONLIMB_TAB, 0, remap.CloneRef(limb), i);
	}
	// PB_CATMOTIONHUB_TAB, originally in post patch, clone explicitly because we own them
	const int numSpines = pblock->Count(PB_CATMOTIONHUB_TAB);
	newCATMotionHub2->pblock->SetCount(PB_CATMOTIONHUB_TAB, numSpines);
	for (int i = 0; i < numSpines; ++i)
	{
		ReferenceTarget* spine = pblock->GetReferenceTarget(PB_CATMOTIONHUB_TAB, 0, i);
		newCATMotionHub2->pblock->SetValue(PB_CATMOTIONHUB_TAB, 0, remap.CloneRef(spine), i);
	}
	// PB_CATMOTIONTAIL_TAB, originally in post patch, clone explicitly because we own them
	const int nmTails = pblock->Count(PB_CATMOTIONTAIL_TAB);
	newCATMotionHub2->pblock->SetCount(PB_CATMOTIONTAIL_TAB, nmTails);
	for (int i = 0; i < nmTails; ++i)
	{
		ReferenceTarget* tail = pblock->GetReferenceTarget(PB_CATMOTIONTAIL_TAB, 0, i);
		newCATMotionHub2->pblock->SetValue(PB_CATMOTIONTAIL_TAB, 0, remap.CloneRef(tail), i);
	}

	BaseClone(this, newCATMotionHub2, remap);
	return newCATMotionHub2;
}

void CATMotionHub2::Copy(Control *from)
{
	if (from->ClassID() == ClassID())
	{
		CATMotionHub2 *newctrl = (CATMotionHub2*)from;
		// a copy will construct its own pblock to keep the pblock-to-owner 1-to-1.
		ReplaceReference(PBLOCK_REF, CloneRefHierarchy(newctrl->pblock));
	}

	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);		// tell our dependents that we've changed.  (this is the default usage for this method)
}

Control* CATMotionHub2::GetOwningCATMotionController()
{
	// Have we already been deleted?
	if (pblock == NULL)
		return NULL;

	// Find our index
	int idx = -1;
	CATHierarchyRoot* pRoot = GetCATHierarchyRoot();
	if (pRoot != NULL)
		idx = pRoot->GetLayerIndex();

	// If we don't have a valid index, bail.
	if (idx < 0)
		return NULL;

	// Find the CATMotionHub of our inspine (if we have one).
	Hub* pOwner = GetHub();
	if (pOwner != NULL)
	{
		SpineData2* pInSpine = pOwner->GetInSpine();
		if (pInSpine != NULL)
		{
			Hub* pOwnerParent = pInSpine->GetBaseHub();
			if (pOwnerParent != NULL)
			{
				CATClipMatrix3* pOwnerLayerTrans = pOwnerParent->GetLayerTrans();
				if (pOwnerLayerTrans != NULL)
					return dynamic_cast<CATMotionController*>(pOwnerLayerTrans->GetLayer(idx));
			}
		}
	}

	// If we have no hub parent (ie - we are pelvis) then
	// our parent is the root.
	return pRoot;
}

void CATMotionHub2::Initialise(int index, Hub* hub, CATHierarchyRoot *root)
{
	DbgAssert(hub != nullptr);
	if(hub == nullptr) {
		return;
	}

	CATHierarchyBranch2 *branchHubGroup = (CATHierarchyBranch2*)root->AddBranch(hub, GetString(IDS_GROUP));
	CATHierarchyBranch2 *branchHub = (CATHierarchyBranch2*)branchHubGroup->AddBranch(hub);

	pblock->SetValue(PB_HUB, 0, hub);
	pblock->SetValue(PB_ROOT, 0, branchHub->GetCATRoot());

	Control *offsetPos = (Control*)CreateInstance(CTRL_POINT3_CLASS_ID, IPOINT3_CONTROL_CLASS_ID);
	Control *offsetRot = (Control*)CreateInstance(CTRL_POINT3_CLASS_ID, IPOINT3_CONTROL_CLASS_ID);
	branchHub->AddAttribute(offsetRot, GetString(IDS_OFFSETROT));
	branchHub->AddAttribute(offsetPos, GetString(IDS_OFFSETPOS));
	pblock->SetControllerByID(PB_P3OFFSETROT, 0, offsetRot, FALSE);
	pblock->SetControllerByID(PB_P3OFFSETPOS, 0, offsetPos, FALSE);

	Control *motionPos = (Control*)CreateInstance(CTRL_POINT3_CLASS_ID, IPOINT3_CONTROL_CLASS_ID);
	CATGraph *motionPosX = (CATGraph*)CreateInstance(CTRL_FLOAT_CLASS_ID, FOOTBEND_CLASS_ID);
	CATGraph *motionPosY = (CATGraph*)CreateInstance(CTRL_FLOAT_CLASS_ID, FOOTBEND_CLASS_ID);
	CATGraph *motionPosZ = (CATGraph*)CreateInstance(CTRL_FLOAT_CLASS_ID, FOOTBEND_CLASS_ID);

	Control *motionRot = (Control*)CreateInstance(CTRL_POINT3_CLASS_ID, IPOINT3_CONTROL_CLASS_ID);
	CATGraph *motionRotX = (CATGraph*)CreateInstance(CTRL_FLOAT_CLASS_ID, FOOTBEND_CLASS_ID);
	CATGraph *motionRotY = (CATGraph*)CreateInstance(CTRL_FLOAT_CLASS_ID, FOOTBEND_CLASS_ID);
	CATGraph *motionRotZ = (CATGraph*)CreateInstance(CTRL_FLOAT_CLASS_ID, FOOTBEND_CLASS_ID);

	if (hub->GetCATParentTrans()->GetLengthAxis() == Z) {
		motionPos->AssignController(motionPosX, X);
		motionPos->AssignController(motionPosY, Y);
		motionPos->AssignController(motionPosZ, Z);

		motionRot->AssignController(motionRotX, X);
		motionRot->AssignController(motionRotY, Y);
		motionRot->AssignController(motionRotZ, Z);
	}
	else {
		motionPosX->FlipValues();
		motionPos->AssignController(motionPosZ, X);
		motionPos->AssignController(motionPosY, Y);
		motionPos->AssignController(motionPosX, Z);

		motionRotX->FlipValues();
		motionRot->AssignController(motionRotZ, X);
		motionRot->AssignController(motionRotY, Y);
		motionRot->AssignController(motionRotX, Z);
	}

	int nNumDefaults = 20;

	// Every second hub in a spoine chain has its defaut vals inverted
	if (hub->GetInSpine() && !hub->GetInSpine()->GetBaseHub()->GetInSpine()) {
		if (!hub->GetInSpine()) {
			float dWeightShiftDefaultVals[] = { 0.0f, 0.0f, -0.1f, 0.333f, 0.333f, /**/ -10.0f, 25.0f, 0.0f, 0.333f, 0.333f, /**/ 0.0f, 50.0f, 0.1f, 0.333f, 0.333f, /**/ 10.0f, 75.0f, 0.0f, 0.333f, 0.333f };
			branchHub->AddAttribute(motionPosX, GetString(IDS_WEIGHTSHIFT), NULL, nNumDefaults, &dWeightShiftDefaultVals[0]);
		}
		else {
			branchHub->AddAttribute(motionPosX, GetString(IDS_WEIGHTSHIFT));
		}
		branchHub->AddAttribute(motionPosY, GetString(IDS_PUSH));

		float dLiftDefaultVals[] = { 3.0f, 0.0f, 0.0f, 0.333f, 0.333f, /**/ -3.0f, 25.0f, 0.0f, 0.333f, 0.333f, /**/ 3.0f, 50.0f, 0.0f, 0.333f, 0.333f, /**/ -3.0f, 75.0f, 0.0f, 0.333f, 0.333f };
		branchHub->AddAttribute(motionPosZ, GetString(IDS_LIFT), NULL, nNumDefaults, &dLiftDefaultVals[0]);

		// No default vals
		branchHub->AddAttribute(motionRotX, GetString(IDS_PITCH));

		float dRollDefaultVals[] = { 0.0f, 0.0f, 0.1f, 0.333f, 0.333f, /**/ 10.0f, 25.0f, 0.0f, 0.333f, 0.333f, /**/ 0.0f, 50.0f, -0.1f, 0.333f, 0.333f, /**/ -10.0f, 75.0f, 0.0f, 0.333f, 0.333f };
		branchHub->AddAttribute(motionRotY, GetString(IDS_ROLL), NULL, nNumDefaults, &dRollDefaultVals[0]);

		float dTwistDefaultVals[] = { -10.0f, 0.0f, 0.0f, 0.333f, 0.333f, /**/ 0.0f, 25.0f, 0.1f, 0.333f, 0.333f, /**/ 10.0f, 50.0f, 0.0f, 0.333f, 0.333f, /**/  0.0f, 75.0f, -0.1f, 0.333f, 0.333f };
		branchHub->AddAttribute(motionRotZ, GetString(IDS_TWIST), NULL, nNumDefaults, &dTwistDefaultVals[0]);

	}
	else {
		if (!hub->GetInSpine()) {
			float dWeightShiftDefaultVals[] = { 0.0f, 0.0f, 0.1f, 0.333f, 0.333f, /**/ 10.0f, 25.0f, 0.0f, 0.333f, 0.333f, /**/ 0.0f, 50.0f, -0.1f, 0.333f, 0.333f, /**/ -10.0f, 75.0f, 0.0f, 0.333f, 0.333f };
			branchHub->AddAttribute(motionPosX, GetString(IDS_WEIGHTSHIFT), NULL, nNumDefaults, &dWeightShiftDefaultVals[0]);
		}
		else {
			branchHub->AddAttribute(motionPosX, GetString(IDS_WEIGHTSHIFT));
		}
		branchHub->AddAttribute(motionPosY, GetString(IDS_PUSH));

		float dLiftDefaultVals[] = { -3.0f, 0.0f, 0.0f, 0.333f, 0.333f, /**/ 3.0f, 25.0f, 0.0f, 0.333f, 0.333f, /**/ -3.0f, 50.0f, 0.0f, 0.333f, 0.333f, /**/ 3.0f, 75.0f, 0.0f, 0.333f, 0.333f };
		branchHub->AddAttribute(motionPosZ, GetString(IDS_LIFT), NULL, nNumDefaults, &dLiftDefaultVals[0]);

		// No default vals
		branchHub->AddAttribute(motionRotX, GetString(IDS_PITCH));

		float dRollDefaultVals[] = { 0.0f, 0.0f, -0.1f, 0.333f, 0.333f, /**/ -10.0f, 25.0f, 0.0f, 0.333f, 0.333f, /**/ 0.0f, 50.0f, 0.1f, 0.333f, 0.333f, /**/ 10.0f, 75.0f, 0.0f, 0.333f, 0.333f };
		branchHub->AddAttribute(motionRotY, GetString(IDS_ROLL), NULL, nNumDefaults, &dRollDefaultVals[0]);

		float dTwistDefaultVals[] = { 10.0f, 0.0f, 0.0f, 0.333f, 0.333f, /**/ 0.0f, 25.0f, -0.1f, 0.333f, 0.333f, /**/ -10.0f, 50.0f, 0.0f, 0.333f, 0.333f, /**/  0.0f, 75.0f, 0.1f, 0.333f, 0.333f };
		branchHub->AddAttribute(motionRotZ, GetString(IDS_TWIST), NULL, nNumDefaults, &dTwistDefaultVals[0]);
	}

	pblock->SetControllerByID(PB_P3MOTIONPOS, 0, motionPos, FALSE);
	pblock->SetControllerByID(PB_P3MOTIONROT, 0, motionRot, FALSE);

	if (!hub->GetInSpine()) {
		// In some rigs, the Hum is parented to a node so that we can have an independant hip.
		// In that case, we want to install the controller on the parent bone layer instead of our own.
		INode* hubnode = hub->GetNode();
		if (!hubnode->GetParentNode()->IsRootNode() && hubnode->GetParentNode()->GetTMController()->ClassID() == ARBBONETRANS_CLASS_ID) {
			CATNodeControl* arbbone = (CATNodeControl*)hubnode->GetParentNode()->GetTMController();
			arbbone->GetLayerTrans()->SetLayer(index, this);
		}
		else {
			hub->GetLayerTrans()->SetLayer(index, this);
		}
	}
	else {
		hub->GetLayerTrans()->SetLayer(index, this);
	}

	int i;
	int numlimbs = hub->GetNumLimbs();
	pblock->SetCount(PB_CATMOTIONLIMB_TAB, numlimbs);
	for (i = 0; i < numlimbs; i++) {
		LimbData2* l = static_cast<LimbData2*>(hub->GetLimb(i));
		if (l != nullptr)
		{
			CATMotionLimb* catmotion_limb = (CATMotionLimb*)CreateInstance(CTRL_FLOAT_CLASS_ID, CATMOTIONLIMB_CLASS_ID);
			catmotion_limb->Initialise(index, l, branchHubGroup);
			pblock->SetValue(PB_CATMOTIONLIMB_TAB, 0, catmotion_limb, i);
		}
	}

	int numspines = hub->GetNumSpines();
	pblock->SetCount(PB_CATMOTIONHUB_TAB, numspines);
	for (i = 0; i < numspines; i++) {
		Hub* h = hub->GetSpine(i)->GetTipHub();
		if (h != nullptr)
		{
			CATMotionHub2* catmotion_hub = (CATMotionHub2*)CreateInstance(CTRL_MATRIX3_CLASS_ID, CATMOTIONHUB2_CLASS_ID);
			catmotion_hub->Initialise(index, h, root);
			pblock->SetValue(PB_CATMOTIONHUB_TAB, 0, catmotion_hub, i);
		}
	}

	int numtails = hub->GetNumTails();
	pblock->SetCount(PB_CATMOTIONTAIL_TAB, numtails);
	for (i = 0; i < numtails; i++) {
		TailData2* t = hub->GetTail(i);
		if (t != nullptr)
		{
			CATMotionTail* catmotiontail = (CATMotionTail*)CreateInstance(CTRL_POINT3_CLASS_ID, CATMOTIONTAIL_CLASS_ID);
			catmotiontail->Initialise(index, t, branchHubGroup);
			pblock->SetValue(PB_CATMOTIONTAIL_TAB, 0, catmotiontail, i);
		}
	}
}

//-------------------------------------------------------------
void CATMotionHub2::GetValue(TimeValue t, void * val, Interval&valid, GetSetMethod)
{
	valid.SetInstant(t);
	Hub* hub = GetHub();
	CATHierarchyRoot* root = GetCATHierarchyRoot();
	if (!hub || !root) return;

	ICATParentTrans *catparenttrans = hub->GetCATParentTrans();

	Matrix3 tmSetup;

	INode* hubnode = hub->GetNode();
	if (!hubnode->GetParentNode()->IsRootNode() && hubnode->GetParentNode()->GetTMController()->ClassID() == ARBBONETRANS_CLASS_ID) {
		CATNodeControl* arbbone = (CATNodeControl*)hubnode->GetParentNode()->GetTMController();
		tmSetup = arbbone->GetSetupMatrix();
	}
	else {
		tmSetup = hub->GetSetupMatrix();
	}
	float catunits = catparenttrans->GetCATUnits();
	int lenghtaxis = catparenttrans->GetLengthAxis();
	//////////////////////////////////////////////////////////////////////////
	Point3 p3SetupPos = tmSetup.GetTrans() * catunits;

	// Calculate a path offset based on our hierarchy
	pathoffset = 0.0f;
	SpineData2 *inspine = hub->GetInSpine();
	if (inspine) {
		int index = root->GetLayerIndex();
		if (index >= 0) {
			CATMotionHub2* inspine_catmotionhub = dynamic_cast<CATMotionHub2*>(inspine->GetBaseHub()->GetLayerTrans()->GetLayer(index));
			if (inspine_catmotionhub != nullptr) {
				pathoffset = inspine_catmotionhub->pathoffset;
			}
			Matrix3 tm = inspine->GetBaseTransform() * inspine->GetBaseHub()->GetSetupMatrix();
			if (lenghtaxis == Z)
				pathoffset += tm.GetRow(Z)[Y] * inspine->GetSpineLength() * catunits;
			else pathoffset += tm.GetRow(X)[Y] * inspine->GetSpineLength() * catunits;
		}
	}
	else pathoffset = p3SetupPos[Y];

	Matrix3 tmOrient(1);
	root->GettmPath(t, tmOrient, valid, pathoffset, FALSE);

	// relative spines keep thier positions
	if (inspine) { //&& inspine->GetAbsRel(t)>0.5f ){
		float absrel = inspine->GetAbsRel(t, valid);
		tmOrient.SetTrans(BlendVector(tmOrient.GetTrans(), (*(Matrix3*)val).GetTrans(), absrel));
		p3SetupPos[Y] *= absrel;
	}
	else p3SetupPos[Y] = 0.0f;

	*(Matrix3*)val = tmOrient;
	(*(Matrix3*)val).PreTranslate(p3SetupPos);
	tmSetup.NoTrans();
	//////////////////////////////////////////////////////////////////////////

	Point3 p3Pos = P3_IDENTITY;
	Point3 p3Rot = P3_IDENTITY;

	/////////////////////////////////////////
	// now we add on the effect of the footprints
	float movementmask = 1.0f;

	if (root->GetWalkMode() == CATHierarchyRoot::WALK_ON_PATHNODE) {
		int numlimbs = pblock->Count(PB_CATMOTIONLIMB_TAB);
		if (numlimbs > 0) {
			Point3 limbMove, totallimbMove(0, 0, 0);
			float limbZRot = 0.0f, totallimbZRot = 0.0f;
			int numlegs = 0;

			CATMotionLimb *limb;
			for (int i = 0; i < numlimbs; i++) {
				limb = (CATMotionLimb*)pblock->GetReferenceTarget(PB_CATMOTIONLIMB_TAB, t, i);
				if (!limb) continue;

				if (root->GetisStepMasks()) {
					movementmask *= limb->GetFootStepMask(t);
				}

				if (!limb->GetisLeg()) continue;
				limb->GetFootPrintPos(t, limbMove);
				totallimbMove += limbMove;

				limb->GetFootPrintRot(t, limbZRot);
				totallimbZRot += limbZRot;
				numlegs++;
			}
			if (numlegs > 0) {
				p3Pos += (totallimbMove / (float)numlegs);
				p3Rot[lenghtaxis] += RadToDeg(limbZRot / (float)numlegs);
			}
		}
		else {
			movementmask = GetFootStepMask(t);
		}
	}

	p3Pos += ((pblock->GetPoint3(PB_P3MOTIONPOS, t) * movementmask) + pblock->GetPoint3(PB_P3OFFSETPOS, t)) * catunits;
	p3Rot += (pblock->GetPoint3(PB_P3MOTIONROT, t) * movementmask) + pblock->GetPoint3(PB_P3OFFSETROT, t);

	/////////////////////////////////////////
	// build the final matrix
	if (!inspine || !inspine->GetSpineFK()) {
		// We don't want to translate off the tip of the FK spine.
		// the Procedural spine would clamp the motion, but the FK one doesn't
		(*(Matrix3*)val).PreTranslate(p3Pos);
	}

	float CATEuler[] = { DegToRad(p3Rot.x), DegToRad(p3Rot.y), DegToRad(p3Rot.z) };
	Quat qtCATRot;
	EulerToQuat(&CATEuler[0], qtCATRot);
	PreRotateMatrix(*(Matrix3*)val, qtCATRot);

	(*(Matrix3*)val) = tmSetup * (*(Matrix3*)val);

}

float CATMotionHub2::GetFootStepMask(TimeValue t)
{
	float movementmask = 1.0f;
	CATHierarchyRoot* root = GetCATHierarchyRoot();
	if (!root || root->GetWalkMode() != CATHierarchyRoot::WALK_ON_PATHNODE) return movementmask;

	int numlimbs = pblock->Count(PB_CATMOTIONLIMB_TAB);
	if (numlimbs > 0) {
		for (int i = 0; i < numlimbs; i++) {
			CATMotionLimb *limb = (CATMotionLimb*)pblock->GetReferenceTarget(PB_CATMOTIONLIMB_TAB, t, i);
			if (!limb) continue;
			movementmask *= limb->GetFootStepMask(t);
		}
	}
	else {
		SpineData2 *inspine = GetHub()->GetInSpine();
		if (inspine) {
			int index = root->GetLayerIndex();
			if (index >= 0)
			{
				CATMotionHub2* inspine_catmotionhub = dynamic_cast<CATMotionHub2*>(inspine->GetBaseHub()->GetLayerTrans()->GetLayer(index));
				DbgAssert(inspine_catmotionhub);
				if (inspine_catmotionhub != nullptr) {
					movementmask = inspine_catmotionhub->GetFootStepMask(t);	
				}
			}
		}
	}
	return movementmask;
}
void CATMotionHub2::CATMotionMessage(TimeValue t, UINT msg, int data)
{
	for (int i = 0; i < pblock->Count(PB_CATMOTIONLIMB_TAB); i++) {
		CATMotionLimb* limb = (CATMotionLimb*)pblock->GetReferenceTarget(PB_CATMOTIONLIMB_TAB, t, i);
		if (limb) limb->CATMotionMessage(t, msg, data);
	}
	for (int i = 0; i < pblock->Count(PB_CATMOTIONHUB_TAB); i++) {
		CATMotionHub2* childhub = (CATMotionHub2*)pblock->GetReferenceTarget(PB_CATMOTIONHUB_TAB, t, i);
		if (childhub) childhub->CATMotionMessage(t, msg, data);
	}
}

void CATMotionHub2::AddSystemNodes(INodeTab &nodes, SysNodeContext ctxt)
{
	for (int i = 0; i < pblock->Count(PB_CATMOTIONLIMB_TAB); i++) {
		CATMotionLimb* limb = (CATMotionLimb*)pblock->GetReferenceTarget(PB_CATMOTIONLIMB_TAB, 0, i);
		if (limb) limb->AddSystemNodes(nodes, ctxt);
	}
	for (int i = 0; i < pblock->Count(PB_CATMOTIONHUB_TAB); i++) {
		CATMotionHub2* childhub = (CATMotionHub2*)pblock->GetReferenceTarget(PB_CATMOTIONHUB_TAB, 0, i);
		if (childhub) childhub->AddSystemNodes(nodes, ctxt);
	}
}
