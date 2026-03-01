//**************************************************************************/
// Copyright (c) 2009 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
#pragma unmanaged
#include <strclass.h>
#include <maxscript/maxscript.h>
#include "DayDriverClassDesc.h"
#include "sunclass.h"
#include "SunDriverCreateMode.h"
#include "sunlight.h"

namespace 
{

	// Checks if an appropriate exposure control is currently applied, for this light type.
	// If not, the user is prompted with the option to automatically apply an approriate exposure control.
	// This function calls the underlying function implemented in MaxScript.
	// TO DO: Remove duplicated function in photometric Free Lights and Target Lights.
	void ValidateExposureControl(ClassDesc* classDesc)
	{
		TSTR className = classDesc->ClassName();
		className.Replace(_T(" "), _T("_") );

		// Set up the command to execute in MaxScript
		TSTR execString;
		execString.printf( _T("ValidateExposureControlForLight %s"), className );

		// Execute the command in MaxScript
		BOOL quietErrors = TRUE;
		ExecuteMAXScriptScript( execString, MAXScript::ScriptSource::Dynamic, quietErrors);
	}

}

int DayDriverClassDesc::IsPublic() 
{ 
	return 1; 
}

void * DayDriverClassDesc::Create(BOOL) 
{ 
	return new SunDriver(true); 
}

// This method returns the name of the class.  This name appears 
// in the button for the plug-in in the MAX user interface.
const TCHAR * DayDriverClassDesc::ClassName() 
{
	static MSTR name = MaxSDK::GetResourceStringAsMSTR(IDS_DAY_CLASS);
	return name.data();
}

const TCHAR* DayDriverClassDesc::NonLocalizedClassName()
{
	return _T("Daylight");
}

SClass_ID DayDriverClassDesc::SuperClassID() 
{ 
	return SYSTEM_CLASS_ID; 
} 

Class_ID DayDriverClassDesc::ClassID() 
{ 
	return DAYLIGHT_CLASS_ID; 
}

const TCHAR* DayDriverClassDesc::Category() 
{ 
	return _T("");  
}

// This is the method of the class descriptor that actually begins the 
// creation process in the viewports.
int DayDriverClassDesc::BeginCreate(Interface *i)
{
	ValidateExposureControl(this);

	// Save the interface pointer passed in.  This is used to call 
	// methods provided by MAX itself.
	IObjCreate *iob = i->GetIObjCreate();

	SunDriverCreateMode::GetInstance()->Begin( iob, this, true );
	// Set the current command mode to the SunDriverCreateMode.
	iob->PushCommandMode( SunDriverCreateMode::GetInstance() );

	return TRUE;
}

// This is the method of the class descriptor that terminates the 
// creation process.
int DayDriverClassDesc::EndCreate(Interface *i)
{
	SunDriverCreateMode::GetInstance()->End();
	// Remove the command mode from the command stack.
	i->RemoveMode( SunDriverCreateMode::GetInstance() );
	return TRUE;
}
