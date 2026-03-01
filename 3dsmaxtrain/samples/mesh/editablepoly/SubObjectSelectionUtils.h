#pragma once

//*****************************************************************************
// Copyright 2020 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license agreement
// provided at the time of installation or download, or which otherwise accompanies
// this software in either electronic or hard copy form.
//
//*****************************************************************************
//
//	FILE: PolyLoopSelection.h
//  DESCRIPTION: Utilities related to sub-object selection on an MNMesh
//	CREATED BY:	Nicholas Frechette
//*****************************************************************************

#include <mesh.h>
#include <mnmesh.h>

inline MeshSubHitRec* GetClosestHitRec(SubObjHitList& hitList)
{
	if (hitList.IsEmpty())
		return nullptr;

	MeshSubHitRec* closest = nullptr;
	DWORD minDist = ~0;
	for (MeshSubHitRec& rec : hitList)
	{
		if (rec.dist < minDist)
		{
			closest = &rec;
			minDist = rec.dist;
		}
	}

	return closest;
}

inline SubObjHitList* GetClosestHitList(SubObjHitList& vertHitList, SubObjHitList& edgeHitList, SubObjHitList& faceHitList, int& closestSelectionLevel)
{
	const MeshSubHitRec* closestVert = GetClosestHitRec(vertHitList);
	const MeshSubHitRec* closestEdge = GetClosestHitRec(edgeHitList);
	const MeshSubHitRec* closestFace = GetClosestHitRec(faceHitList);

	// Pick the closest thing we can select when multi selection is used
	// We proceed by order of priority because:
	//    - The closest face is always at 0 pixels from the cursor (right underneath it)
	//    - If an edge and a vertex are within our selection radius, the edge will almost always
	//      be closer than the vertex which makes selecting vertices impossible
	// To resolve this issue by using the closest vertex, or the closest edge, or the closest face
	// in that order.
	SubObjHitList* hitList = nullptr;
	if (closestVert != nullptr)
	{
		// We found a vertex, use it
		hitList = &vertHitList;
		closestSelectionLevel = MNM_SL_VERTEX;
	}
	else if (closestEdge != nullptr)
	{
		// We found an edge, use it
		hitList = &edgeHitList;
		closestSelectionLevel = MNM_SL_EDGE;
	}
	else if (closestFace != nullptr)
	{
		// We found a face, use it
		hitList = &faceHitList;
		closestSelectionLevel = MNM_SL_FACE;
	}

	return hitList;
}
