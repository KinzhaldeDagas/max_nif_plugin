//**************************************************************************/
// Copyright (c) 2006 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
/**********************************************************************
	FILE: SimpleFaceDataCommon.cpp

	DESCRIPTION:	Utility functions used by SimpleFaceDataAttrib

	CREATED BY: Kuo-Cheng Tong

	AUTHOR: ktong - created 01.30.2006
***************************************************************************/

#include "SimpleFaceDataCommon.h"
#include "ICustAttribContainer.h"

#include <triobj.h>
#include <polyobj.h>

extern Object* Get_Object_Or_XRef_BaseObject(Object *obj);

// ****************************************************************************
// Dependent enum callback
// ****************************************************************************
FindParentObjFromAttribCB::FindParentObjFromAttribCB(Animatable* pSelf)
{
	mpParent = NULL;
	mpSelf = pSelf;
}
FindParentObjFromAttribCB::~FindParentObjFromAttribCB()
{
}
Animatable* FindParentObjFromAttribCB::GetParent()
{
	return mpParent;
}
//-----------------------------------------------------------------------------
int FindParentObjFromAttribCB::proc(ReferenceMaker* rmaker)
// Traverse up dependencies to find the owner object of a custom attribute.
// The CC should be IMMEDIATELY above the CA. If I've encountered a non CC,
// I'm going the wrong way (there might be other things pointing at the CA, but
// the CC definitely won't be at the other end of that, so SKIP the branch).
//-----------------------------------------------------------------------------
{
	if (rmaker->SuperClassID() == REF_MAKER_CLASS_ID && 
			rmaker->ClassID() == CUSTATTRIB_CONTAINER_CLASS_ID) 
	{
		ICustAttribContainer* cc = static_cast<ICustAttribContainer*>(rmaker);
		if (cc) 
		{
			mpParent = cc->GetOwner();
		}
	}
	if (mpParent) 
	{
		return DEP_ENUM_HALT;
	} 
	else if (static_cast<Animatable*>(rmaker) == mpSelf) 
	{
		return DEP_ENUM_CONTINUE;
	} 
	else 
	{
		return DEP_ENUM_SKIP;
	}
}

// ****************************************************************************
// Static function defs
// ****************************************************************************
//-----------------------------------------------------------------------------
Animatable* SimpleFaceDataCommon::FindParentObjFromAttrib(CustAttrib* pCA)
//-----------------------------------------------------------------------------
{
	FindParentObjFromAttribCB theFinder(pCA);
	if (pCA) {
		pCA->DoEnumDependents(&theFinder);
	}
	return theFinder.GetParent();
}


//-----------------------------------------------------------------------------
IFaceDataMgr* SimpleFaceDataCommon::FindFaceDataFromObj(Animatable* pObj)
//-----------------------------------------------------------------------------
{
	IFaceDataMgr* pFDMgr = NULL;
	
	if (pObj && pObj->IsSubClassOf(triObjectClassID)) {
		TriObject* pTri = static_cast<TriObject*>(pObj);
		Mesh* pMesh = &(pTri->GetMesh());
		
		// Get the face-data manager from the incoming object
		pFDMgr = static_cast<IFaceDataMgr*>(pMesh->GetInterface( FACEDATAMGR_INTERFACE ));
	} else if (pObj && pObj->IsSubClassOf(polyObjectClassID)) {
		PolyObject* pPoly = static_cast<PolyObject*>(pObj);
		MNMesh* pMesh = &(pPoly->GetMesh());
		
		// Get the face-data manager from the incoming object
		pFDMgr = static_cast<IFaceDataMgr*>(pMesh->GetInterface( FACEDATAMGR_INTERFACE ));
	}
	return pFDMgr;
}

//-----------------------------------------------------------------------------
CustAttrib* SimpleFaceDataCommon::FindAttribFromParent(Animatable* pAnim, const Class_ID& cid)
//-----------------------------------------------------------------------------
{
	CustAttrib* pCA = NULL;
	if (pAnim) {
		ICustAttribContainer* cc = pAnim->GetCustAttribContainer();
		if (cc) {
			for (int i = 0; i < cc->GetNumCustAttribs(); i++) {
				CustAttrib* ca = cc->GetCustAttrib(i);
				if (ca->ClassID() == cid) {
					pCA = ca;
					break;
				}
			}
		}		
	}
	return pCA;
}

//-----------------------------------------------------------------------------
Object* SimpleFaceDataCommon::FindBaseFromObject(Object* pObj)
//-----------------------------------------------------------------------------
{
	if (pObj) {
		return Get_Object_Or_XRef_BaseObject(pObj);
	} else {
		return NULL;
	}
}

//-----------------------------------------------------------------------------
ULONG SimpleFaceDataCommon::GetNumFacesFromObject(Animatable* pObj)
//-----------------------------------------------------------------------------
{
	ULONG numFaces = 0;
	if (pObj->IsSubClassOf(triObjectClassID) ) {
		TriObject* pTri = static_cast<TriObject*>(pObj);
		Mesh* pMesh = &(pTri->GetMesh());
		numFaces = pMesh->numFaces;
	} else if (pObj->IsSubClassOf(polyObjectClassID) ) {
		PolyObject* pPoly = static_cast<PolyObject*>(pObj);
		MNMesh* pMesh = &(pPoly->GetMesh());
		numFaces = pMesh->numf;
	}
	return numFaces;
}

//-----------------------------------------------------------------------------
IFaceDataChannel* SimpleFaceDataCommon::FindFaceDataChanFromObj(Animatable* pObj, const Class_ID& cid)
//-----------------------------------------------------------------------------
{
	IFaceDataMgr* pFDMgr = SimpleFaceDataCommon::FindFaceDataFromObj(pObj);
	if (pFDMgr) {
		return pFDMgr->GetFaceDataChan(cid);
	}
	return NULL;
}

// EOF