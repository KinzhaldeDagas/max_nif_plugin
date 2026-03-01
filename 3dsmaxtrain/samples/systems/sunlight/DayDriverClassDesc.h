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

class DayDriverClassDesc : public ClassDesc {
public:
	int IsPublic();
	void* Create(BOOL loading = FALSE);

	// This method returns the name of the class.  This name appears 
	// in the button for the plug-in in the MAX user interface.
	const TCHAR* ClassName();
	const TCHAR* NonLocalizedClassName();
	int BeginCreate(Interface *i);
	int EndCreate(Interface *i);
	SClass_ID SuperClassID();
	Class_ID ClassID();
	const TCHAR* Category();
};


