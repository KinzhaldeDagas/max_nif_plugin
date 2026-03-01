//**************************************************************************/
// Copyright (c) 2009 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
#pragma unmanaged
#include "DrivenControlFloatClassDesc.h"
#include "sunclass.h"
#include "sunlight.h"
#include <dllutilities.h>


int DrivenControlFloatClassDesc::IsPublic()
{ 
	return 0; 
}

void * DrivenControlFloatClassDesc::Create(BOOL) 
{ 
	return new DrivenControl(false); 
}

const TCHAR * DrivenControlFloatClassDesc::ClassName() 
{
	static MSTR class_name = MaxSDK::GetResourceStringAsMSTR(IDS_DRIVEN_FLOAT_CLASS);
	return class_name.data();
}

const TCHAR* DrivenControlFloatClassDesc::NonLocalizedClassName()
{
	return _T("Sunlight Driven Intensity Controller");
}

SClass_ID DrivenControlFloatClassDesc::SuperClassID() 
{ 
	return CTRL_FLOAT_CLASS_ID; 
}

Class_ID DrivenControlFloatClassDesc::ClassID() 
{ 
	return Class_ID(DRIVEN_CONTROL_CID1,DRIVEN_CONTROL_CID2); 
}

// The driven controllers don't appear in any of the drop down lists, 
// so they just return a null string.
const TCHAR* DrivenControlFloatClassDesc::Category()
{ 
	return _T(""); 
}
