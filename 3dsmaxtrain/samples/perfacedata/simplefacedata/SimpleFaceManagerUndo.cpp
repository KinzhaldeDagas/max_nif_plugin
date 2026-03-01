//**************************************************************************/
// Copyright (c) 2006 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
/**********************************************************************
	FILE: SimpleFaceManagerUndo.cpp

	DESCRIPTION: Header for restore objects for SimpleFaceManager.
					- add channel
					- remove channel

	AUTHOR: ktong - created 02.16.2006
/***************************************************************************/

#include "SimpleFaceManagerUndo.h"

// ****************************************************************************
// add channel undo object
// ****************************************************************************
//-----------------------------------------------------------------------------
AddChannelUndo::AddChannelUndo(Animatable* pObj, const Class_ID &channelID)
//-----------------------------------------------------------------------------
{
	mpObj = pObj;
	mpCon = NULL;
	mChannelID = channelID;
}
//-----------------------------------------------------------------------------
AddChannelUndo::~AddChannelUndo()
//-----------------------------------------------------------------------------
{
	if (mpCon) {
		mpCon->DeleteThis();
		mpCon = NULL;
	}
}
//-----------------------------------------------------------------------------
void	AddChannelUndo::Restore(int isUndo)
//-----------------------------------------------------------------------------
{
	// remove the channel
	IFaceDataMgr* pFDMgr = SimpleFaceDataCommon::FindFaceDataFromObj(mpObj);
	if (pFDMgr) {
		IFaceDataChannel* pChan = pFDMgr->GetFaceDataChan(mChannelID);
		if (isUndo && pChan && (mpCon == NULL)) { //save sub-channels so I can add them back
			mpCon = pChan->CloneChannel();
		}
		pFDMgr->RemoveFaceDataChan(mChannelID);
	}
}
//-----------------------------------------------------------------------------
void	AddChannelUndo::Redo()
//-----------------------------------------------------------------------------
{
	IFaceDataMgr* pFDMgr = SimpleFaceDataCommon::FindFaceDataFromObj(mpObj);
	if (pFDMgr && (pFDMgr->GetFaceDataChan(mChannelID)== NULL) && mpCon) { //put the subchannels back, if I have any
		pFDMgr->AddFaceDataChan(mpCon->CloneChannel());
	}
}
//-----------------------------------------------------------------------------
int		AddChannelUndo::Size()
//-----------------------------------------------------------------------------
{
	return (sizeof(AddChannelUndo));
}
//-----------------------------------------------------------------------------
void	AddChannelUndo::EndHold()
//-----------------------------------------------------------------------------
{
}
//-----------------------------------------------------------------------------
TSTR	AddChannelUndo::Description()
//-----------------------------------------------------------------------------
{
	return TSTR(_T("AddChannelUndo"));
}
//-----------------------------------------------------------------------------
INT_PTR	AddChannelUndo::Execute(int /*cmd*/, ULONG_PTR /*arg1*/, ULONG_PTR /*arg2*/, ULONG_PTR /*arg3*/)
//-----------------------------------------------------------------------------
{
	return -1;
}

// ****************************************************************************
// remove channel undo object
// ****************************************************************************
//-----------------------------------------------------------------------------
RemoveChannelUndo::RemoveChannelUndo(Animatable* pObj, const Class_ID &channelID)
//-----------------------------------------------------------------------------
{
	mpObj = pObj;
	mpCon = NULL;
	mChannelID = channelID;

	IFaceDataMgr* pFDMgr = SimpleFaceDataCommon::FindFaceDataFromObj(mpObj);
	if (pFDMgr) {
		IFaceDataChannel* pChan = pFDMgr->GetFaceDataChan(mChannelID);
		if (pChan) { //save sub-channels so I can add them back
			mpCon = pChan->CloneChannel();
		}
	}
}
//-----------------------------------------------------------------------------
RemoveChannelUndo::~RemoveChannelUndo()
//-----------------------------------------------------------------------------
{
	if (mpCon) {
		mpCon->DeleteThis();
		mpCon = NULL;
	}
}
//-----------------------------------------------------------------------------
void	RemoveChannelUndo::Restore(int /*isUndo*/)
//-----------------------------------------------------------------------------
{
	// doesn't matter if isUndo or not, since restoring from nothing either way.
	IFaceDataMgr* pFDMgr = SimpleFaceDataCommon::FindFaceDataFromObj(mpObj);
	if (pFDMgr && (pFDMgr->GetFaceDataChan(mChannelID)== NULL) && (mpCon != NULL)) { //put the subchannels back, if I have any
		pFDMgr->AddFaceDataChan(mpCon->CloneChannel());
	}
}
//-----------------------------------------------------------------------------
void	RemoveChannelUndo::Redo()
//-----------------------------------------------------------------------------
{
	IFaceDataMgr* pFDMgr = SimpleFaceDataCommon::FindFaceDataFromObj(mpObj);
	if (pFDMgr) {
		pFDMgr->RemoveFaceDataChan(mChannelID);
	}
}
//-----------------------------------------------------------------------------
int		RemoveChannelUndo::Size()
//-----------------------------------------------------------------------------
{
	return (sizeof(RemoveChannelUndo));
}
//-----------------------------------------------------------------------------
void	RemoveChannelUndo::EndHold()
//-----------------------------------------------------------------------------
{
}
//-----------------------------------------------------------------------------
TSTR	RemoveChannelUndo::Description()
//-----------------------------------------------------------------------------
{
	return TSTR(_T("RemoveChannelUndo"));
}
//-----------------------------------------------------------------------------
INT_PTR	RemoveChannelUndo::Execute(int /*cmd*/, ULONG_PTR /*arg1*/, ULONG_PTR /*arg2*/, ULONG_PTR /*arg3*/)
//-----------------------------------------------------------------------------
{
	return -1;
}