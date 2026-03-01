//**************************************************************************/
// Copyright (c) 2009 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
#pragma unmanaged
#include "DayDrivenControlPosClassDesc.h"
#include "sunclass.h"
#include "sunlight.h"
#include <dllutilities.h>


int DayDrivenControlMatrix3ClassDesc::IsPublic()
{ 
	return 0; 
}

void * DayDrivenControlMatrix3ClassDesc::Create(BOOL) 
{ 
	return new DrivenControl(true); 
}

const TCHAR * DayDrivenControlMatrix3ClassDesc::ClassName() 
{
	static MSTR name = MaxSDK::GetResourceStringAsMSTR(IDS_DAY_DRIVEN_POS_CLASS);
	return name.data();
}

const TCHAR* DayDrivenControlMatrix3ClassDesc::NonLocalizedClassName()
{
	return _T("Daylight Driven Controller");
}

SClass_ID DayDrivenControlMatrix3ClassDesc::SuperClassID() 
{ 
	return CTRL_MATRIX3_CLASS_ID; 
}

Class_ID DayDrivenControlMatrix3ClassDesc::ClassID() 
{ 
	return Class_ID(DAYLIGHT_DRIVEN_CONTROL_CID1, DAYLIGHT_DRIVEN_CONTROL_CID2); 
}

// The driven controllers don't appear in any of the drop down lists, 
// so they just return a null string.
const TCHAR* DayDrivenControlMatrix3ClassDesc::Category() 
{ 
	return _T(""); 
}
