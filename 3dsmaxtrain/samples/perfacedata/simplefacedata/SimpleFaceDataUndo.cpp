//**************************************************************************/
// Copyright (c) 2006 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
/**********************************************************************
	FILE: SimpleFaceDataAttrib.cpp

	DESCRIPTION:	Implementation file for restoreObj's for SimpleFaceDataAttrib.
					Class defs for:
					- name undo
					- single value undo
					- value list undo

	AUTHOR: ktong - created 02.11.2006
***************************************************************************/


#include "SimpleFaceDataUndo.h"
#include "SimpleFaceDataCommon.h"

// ****************************************************************************
// channel name undo object
// ****************************************************************************
//-----------------------------------------------------------------------------
NameUndo::NameUndo(Animatable* pObj, const Class_ID &channelID, int type)
//-----------------------------------------------------------------------------
{
	DbgAssert(pObj);

	mpObj = pObj;
	mChannelID = channelID;
	mType = type;

	std::unique_ptr<ISimpleFPChannel>pCon = SimpleFaceDataAttrib::WrapFaceChannel(SimpleFaceDataCommon::FindFaceDataChanFromObj(mpObj, mChannelID), mType);
	if (pCon->GetChannel()) {
		mOldName = pCon->GetChannelName();
	}
}
//-----------------------------------------------------------------------------
NameUndo::~NameUndo()
//-----------------------------------------------------------------------------
{
}
//-----------------------------------------------------------------------------
void	NameUndo::Restore(int isUndo)
//-----------------------------------------------------------------------------
{
	std::unique_ptr<ISimpleFPChannel>pCon = SimpleFaceDataAttrib::WrapFaceChannel(SimpleFaceDataCommon::FindFaceDataChanFromObj(mpObj, mChannelID), mType);
	if (pCon->GetChannel()) {
		if (isUndo) {
			mNewName = pCon->GetChannelName();
		}
		pCon->SetChannelName(mOldName);
	}
}
//-----------------------------------------------------------------------------
void	NameUndo::Redo()
//-----------------------------------------------------------------------------
{
	std::unique_ptr<ISimpleFPChannel>pCon = SimpleFaceDataAttrib::WrapFaceChannel(SimpleFaceDataCommon::FindFaceDataChanFromObj(mpObj, mChannelID), mType);
	if (pCon->GetChannel()) {
		pCon->SetChannelName(mNewName);
	}
}
//-----------------------------------------------------------------------------
int		NameUndo::Size()
//-----------------------------------------------------------------------------
{
	return ( sizeof(NameUndo) );
}
//-----------------------------------------------------------------------------
void	NameUndo::EndHold()
//-----------------------------------------------------------------------------
{
}
//-----------------------------------------------------------------------------
TSTR	NameUndo::Description()
//-----------------------------------------------------------------------------
{
	return TSTR(_T("NameUndo"));
}
//-----------------------------------------------------------------------------
INT_PTR	NameUndo::Execute(int /*cmd*/, ULONG_PTR /*arg1*/, ULONG_PTR /*arg2*/, ULONG_PTR /*arg3*/)
//-----------------------------------------------------------------------------
{
	return -1;
}

// ****************************************************************************
// channel value list undo object
// ****************************************************************************
//-----------------------------------------------------------------------------
ValueListUndo::ValueListUndo(Animatable* pObj, const Class_ID &channelID, int type)
//-----------------------------------------------------------------------------
{
	DbgAssert(pObj);

	mpObj = pObj;
	mChannelID = channelID;
	mType = type;

	std::unique_ptr<ISimpleFPChannel>pCon = SimpleFaceDataAttrib::WrapFaceChannel(SimpleFaceDataCommon::FindFaceDataChanFromObj(mpObj, mChannelID), mType);
	if (pCon->GetChannel()) {
		pCon->GetValues(mOldValue);
	}
}

//-----------------------------------------------------------------------------
ValueListUndo::~ValueListUndo()
//-----------------------------------------------------------------------------
{
}

//-----------------------------------------------------------------------------
void	ValueListUndo::Restore(int isUndo)
//-----------------------------------------------------------------------------
{
	std::unique_ptr<ISimpleFPChannel>pCon = SimpleFaceDataAttrib::WrapFaceChannel(SimpleFaceDataCommon::FindFaceDataChanFromObj(mpObj, mChannelID), mType);
	if (pCon->GetChannel()) {
		if (isUndo) {
			pCon->GetValues(mNewValue);
		}
		pCon->SetValues(&mOldValue);
	}
}
//-----------------------------------------------------------------------------
void	ValueListUndo::Redo()
//-----------------------------------------------------------------------------
{
	std::unique_ptr<ISimpleFPChannel>pCon = SimpleFaceDataAttrib::WrapFaceChannel(SimpleFaceDataCommon::FindFaceDataChanFromObj(mpObj, mChannelID), mType);
	if (pCon->GetChannel()) {
		pCon->SetValues(&mNewValue);
	}
}
//-----------------------------------------------------------------------------
int		ValueListUndo::Size()
//-----------------------------------------------------------------------------
{
	return ( sizeof(ValueListUndo) );
}
//-----------------------------------------------------------------------------
void	ValueListUndo::EndHold()
//-----------------------------------------------------------------------------
{
}
//-----------------------------------------------------------------------------
TSTR	ValueListUndo::Description()
//-----------------------------------------------------------------------------
{
	return TSTR(_T("ValueListUndo"));
}
//-----------------------------------------------------------------------------
INT_PTR	ValueListUndo::Execute(int /*cmd*/, ULONG_PTR /*arg1*/, ULONG_PTR /*arg2*/, ULONG_PTR /*arg3*/)
//-----------------------------------------------------------------------------
{
	return -1;
}
// ****************************************************************************
// channel value undo object
// ****************************************************************************
//-----------------------------------------------------------------------------
ValueUndo::ValueUndo(Animatable* pObj, const Class_ID &channelID, ULONG at, int type)
//-----------------------------------------------------------------------------
{
	DbgAssert(pObj);

	mpObj = pObj;
	mChannelID = channelID;
	mAt = at;
	mType = type;

	std::unique_ptr<ISimpleFPChannel>pCon = SimpleFaceDataAttrib::WrapFaceChannel(SimpleFaceDataCommon::FindFaceDataChanFromObj(mpObj, mChannelID), mType);
	if (pCon->GetChannel()) {
		pCon->GetValue(mAt, mOldValue);
		mNewValue = mOldValue;
	}
}

//-----------------------------------------------------------------------------
ValueUndo::~ValueUndo()
//-----------------------------------------------------------------------------
{
}

//-----------------------------------------------------------------------------
void	ValueUndo::Restore(int isUndo)
//-----------------------------------------------------------------------------
{
	std::unique_ptr<ISimpleFPChannel>pCon = SimpleFaceDataAttrib::WrapFaceChannel(SimpleFaceDataCommon::FindFaceDataChanFromObj(mpObj, mChannelID), mType);
	if (pCon->GetChannel()) {
		if (isUndo) {
			pCon->GetValue(mAt, mNewValue);
		}
		pCon->SetValue(mAt, &mOldValue);
	}
}
//-----------------------------------------------------------------------------
void	ValueUndo::Redo()
//-----------------------------------------------------------------------------
{
	std::unique_ptr<ISimpleFPChannel>pCon = SimpleFaceDataAttrib::WrapFaceChannel(SimpleFaceDataCommon::FindFaceDataChanFromObj(mpObj, mChannelID), mType);
	if (pCon->GetChannel()) {
		pCon->SetValue(mAt, &mNewValue);
	}
}
//-----------------------------------------------------------------------------
int		ValueUndo::Size()
//-----------------------------------------------------------------------------
{
	return ( sizeof(ValueUndo) );
}
//-----------------------------------------------------------------------------
void	ValueUndo::EndHold()
//-----------------------------------------------------------------------------
{
}
//-----------------------------------------------------------------------------
TSTR	ValueUndo::Description()
//-----------------------------------------------------------------------------
{
	return TSTR(_T("ValueUndo"));
}
//-----------------------------------------------------------------------------
INT_PTR	ValueUndo::Execute(int /*cmd*/, ULONG_PTR /*arg1*/, ULONG_PTR /*arg2*/, ULONG_PTR /*arg3*/)
//-----------------------------------------------------------------------------
{
	return -1;
}

// EOF
