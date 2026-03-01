//**************************************************************************/
// Copyright (c) 2009 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
#pragma unmanaged
#include <dllutilities.h>
#include "CompassRoseObjClassDesc.h"
#include "compass.h"
#include "sunlight.h"

namespace 
{

void resetPointParams() 
{
	CompassRoseObject::dlgShowAxis = TRUE;
	CompassRoseObject::dlgAxisLength = AXIS_LENGTH;
}

}

int CompassRoseObjClassDesc::IsPublic()
{ 
	return TRUE; 
}

void* CompassRoseObjClassDesc::Create(BOOL loading)
{ 
	return new CompassRoseObject; 
}

const TCHAR * CompassRoseObjClassDesc::ClassName() 
{ 
	static MSTR name = MaxSDK::GetResourceStringAsMSTR(IDS_DB_COMPASS_CDESC);
	return name.data();
}

const TCHAR* CompassRoseObjClassDesc::NonLocalizedClassName()
{
	return _T("Compass");
}

SClass_ID CompassRoseObjClassDesc::SuperClassID() 
{ 
	return HELPER_CLASS_ID; 
}

Class_ID CompassRoseObjClassDesc::ClassID() 
{ 
	return COMPASS_CLASS_ID; 
}

const TCHAR* CompassRoseObjClassDesc::Category()
{ 
	return _T(""); 
}

void CompassRoseObjClassDesc::ResetClassParams(BOOL fileReset) 
{ 
	if(fileReset)
	{
		resetPointParams(); 
	}
}
