//**************************************************************************/
// Copyright (c) 2006 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
/**********************************************************************
 	FILE: SimpleFaceManagerUndo.h

	DESCRIPTION: Header for restore objects for SimpleFaceManager.
					- add channel
					- remove channel

	AUTHOR: ktong - created 02.16.2006
/***************************************************************************/

#include "SimpleFaceData.h"
#include "SimpleFaceDataCommon.h"

// ****************************************************************************
// Add channel undo object
// Can't gain access to store THE channel pointer on undo, so have to make 
// a new one during redo.
// Used by SimpleFaceDataAttrib
// ****************************************************************************
class AddChannelUndo : public RestoreObj
{
protected:
	Animatable* mpObj;			// the channel host object
	IFaceDataChannel* mpCon;	// a copy of the data channel to undo
	Class_ID mChannelID;		// the id of the channel to undo
public:
	AddChannelUndo(Animatable* pObj, const Class_ID &channelID);
	~AddChannelUndo();
	void	Restore(int isUndo);
	void	Redo();
	int		Size();
	void	EndHold();
	TSTR	Description();
	INT_PTR	Execute(int cmd, ULONG_PTR arg1, ULONG_PTR arg2, ULONG_PTR arg3);
};

// ****************************************************************************
// Remove channel undo object
// Can't gain access to store THE channel pointer on creation or redo, so 
// have to make a new one on undo.
// Used by SimpleFaceDataAttrib
// ****************************************************************************
class RemoveChannelUndo : public RestoreObj
{
protected:
	Animatable* mpObj;			// the channel host object
	IFaceDataChannel* mpCon;	// a copy of the data channel to undo
	Class_ID mChannelID;		// the id of the channel to undo
public:
	RemoveChannelUndo(Animatable* pObj, const Class_ID &channelID);
	~RemoveChannelUndo();
	void	Restore(int isUndo);
	void	Redo();
	int		Size();
	void	EndHold();
	TSTR	Description();
	INT_PTR	Execute(int cmd, ULONG_PTR arg1, ULONG_PTR arg2, ULONG_PTR arg3);
};