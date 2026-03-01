//**************************************************************************/
// Copyright (c) 2009 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
#pragma unmanaged
#include "DrivenControlPosClassDesc.h"
#include "sunclass.h"
#include "sunlight.h"
#include <dllutilities.h>


int DrivenControlMatrix3ClassDesc::IsPublic() 
{ 
	return 0; 
}

void * DrivenControlMatrix3ClassDesc::Create(BOOL) 
{ 
	return new DrivenControl(false); 
}

const TCHAR * DrivenControlMatrix3ClassDesc::ClassName() 
{
	static MSTR class_name = MaxSDK::GetResourceStringAsMSTR(IDS_DRIVEN_POS_CLASS);
	return class_name.data();
}

const TCHAR* DrivenControlMatrix3ClassDesc::NonLocalizedClassName()
{
	return _T("Sunlight Driven Controller");
}

SClass_ID DrivenControlMatrix3ClassDesc::SuperClassID() 
{ 
	return CTRL_MATRIX3_CLASS_ID; 
}

Class_ID DrivenControlMatrix3ClassDesc::ClassID() 
{ 
	return Class_ID(DRIVEN_CONTROL_CID1, DRIVEN_CONTROL_CID2); 
}

// The driven controllers don't appear in any of the drop down lists, 
// so they just return a null string.
const TCHAR* DrivenControlMatrix3ClassDesc::Category() 
{ 
	return _T(""); 
}


