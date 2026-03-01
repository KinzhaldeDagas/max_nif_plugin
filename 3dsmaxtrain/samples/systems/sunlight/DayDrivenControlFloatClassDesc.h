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

class DayDrivenControlFloatClassDesc : public ClassDesc
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


