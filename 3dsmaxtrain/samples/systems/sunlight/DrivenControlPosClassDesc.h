//**************************************************************************/
// Copyright (c) 2009 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#pragma once

#include <maxapi.h>

class DrivenControlMatrix3ClassDesc : public ClassDesc 
{
public:
	int IsPublic();
	void* Create(BOOL loading = FALSE);
	const TCHAR* ClassName();
	const TCHAR* NonLocalizedClassName();
	SClass_ID SuperClassID();
	Class_ID ClassID();
	// The driven controllers don't appear in any of the drop down lists, 
	// so they just return a null string.
	const TCHAR* Category();
};

// In Rampage, we corrected the superclass ID (from POSITION).  To support old files, 
// we need to have a ClassDesc that matches the old SClassID
class LegacyDrivenControlPosClassDesc : public DrivenControlMatrix3ClassDesc
{
	SClass_ID SuperClassID() { return CTRL_POSITION_CLASS_ID; }
	const TCHAR* ClassName()  { return nullptr; } // will not expose via mxs
	const TCHAR* NonLocalizedClassName() { return nullptr; }
	// Keeps from displaying itself in the track view filter list.
	INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0)
	{
		if (cmd == I_EXEC_CTRL_BYPASS_TREE_VIEW)
		return 1; // TODO: turn this off
		return ClassDesc::Execute(cmd, arg1, arg2, arg3);
	} 
};


