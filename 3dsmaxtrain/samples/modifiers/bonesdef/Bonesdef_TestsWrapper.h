//**************************************************************************/
// Copyright (c) 2018 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#pragma once

#include "Bonesdef_VertexWeights.h"
#include <memory>
#include <functional>

class BonesdefTestsWrapper : public MaxHeapOperators
{
public:
	__declspec(dllexport) static bool Init(std::function<void(int, int)> iTester, std::function<void(float, float)> fTester);
	__declspec(dllexport) static bool Clean();

	__declspec(dllexport) static void TestWeightCount();
	__declspec(dllexport) static void TestBoneIndex();
	__declspec(dllexport) static void TestWeight();
	__declspec(dllexport) static void TestNormalizeWeights();


private:
	static void BindFloatCallback(std::function<void(float, float)> fct);
	static void BindIntegerCallback(std::function<void(int, int)> fct);
	static int GetWeightCount(int vertexId);
	static int GetBoneIndex(int vertexId, int boneIndex);
	static float GetWeight(int vertexId, int boneIndex);
	static void NormalizeWeights(int vertexId);
	static float GetNormalizedWeight(int vertexId, int boneIndex);
	//
	static BoneVertexMgr gBoneVertexMgr;
	static Tab<VertexListClass*> gVertexPtrList;
	static std::function<void(float, float)> FloatTestHandler;
	static std::function<void(int, int)> IntegerTestHandler;
};

