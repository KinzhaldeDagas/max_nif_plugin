//**************************************************************************/
// Copyright (c) 2009 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
#pragma unmanaged
#include "DayDrivenControlFloatClassDesc.h"
#include "sunclass.h"
#include "sunlight.h"
#include <dllutilities.h>



int DayDrivenControlFloatClassDesc::IsPublic() 
{ 
	return 0; 
}

void * DayDrivenControlFloatClassDesc::Create(BOOL) 
{ 
	return new DrivenControl(true); 
}

const TCHAR * DayDrivenControlFloatClassDesc::ClassName() 
{
	static MSTR name = MaxSDK::GetResourceStringAsMSTR(IDS_DAY_DRIVEN_FLOAT_CLASS);
	return name.data();
}

const TCHAR* DayDrivenControlFloatClassDesc::NonLocalizedClassName()
{
	return _T("Daylight Driven Intensity Controller");
}

SClass_ID DayDrivenControlFloatClassDesc::SuperClassID() 
{ 
	return CTRL_FLOAT_CLASS_ID; 
}

Class_ID DayDrivenControlFloatClassDesc::ClassID() 
{ 
	return Class_ID(DAYLIGHT_DRIVEN_CONTROL_CID1, DAYLIGHT_DRIVEN_CONTROL_CID2); 
}

// The driven controllers don't appear in any of the drop down lists, 
// so they just return a null string.
const TCHAR* DayDrivenControlFloatClassDesc::Category()
{ 
	return _T(""); 
}
