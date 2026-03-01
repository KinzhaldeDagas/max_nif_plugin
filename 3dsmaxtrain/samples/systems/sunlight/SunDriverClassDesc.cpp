//**************************************************************************/
// Copyright (c) 2009 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
#pragma unmanaged
#include "SunDriverClassDesc.h"
#include "SunDriverCreateMode.h"
#include "sunclass.h"
#include "sunlight.h"
#include <dllutilities.h>
#include <Geom/matrix3.h>


SunDriverClassDesc::SunDriverClassDesc()
{
	mClassName =  MaxSDK::GetResourceStringAsMSTR(IDS_SUN_CLASS);
}
SunDriverClassDesc::~SunDriverClassDesc()
{

}

int SunDriverClassDesc::IsPublic() 
{ 
	return 1; 
}

void * SunDriverClassDesc::Create(BOOL) 
{ 
	return new SunDriver(false); 
}

// This method returns the name of the class.  This name appears 
// in the button for the plug-in in the MAX user interface.
const TCHAR* SunDriverClassDesc::ClassName() 
{
	return mClassName.data();
}

const TCHAR* SunDriverClassDesc::NonLocalizedClassName()
{
	return _T("Sunlight");
}

// This is the method of the class descriptor that actually begins the 
// creation process in the viewports.
int SunDriverClassDesc::BeginCreate(Interface *i)
{
	// Save the interface pointer passed in.  This is used to call 
	// methods provided by MAX itself.
	IObjCreate *iob = i->GetIObjCreate();

	SunDriverCreateMode::GetInstance()->Begin( iob, this, false );
	// Set the current command mode to the SunDriverCreateMode.
	iob->PushCommandMode( SunDriverCreateMode::GetInstance() );

	return TRUE;
}

// This is the method of the class descriptor that terminates the 
// creation process.
int SunDriverClassDesc::EndCreate(Interface *i)
{
	SunDriverCreateMode::GetInstance()->End();
	// Remove the command mode from the command stack.
	i->RemoveMode( SunDriverCreateMode::GetInstance() );
	return TRUE;
}

SClass_ID SunDriverClassDesc::SuperClassID() 
{ 
	return SYSTEM_CLASS_ID; 
} 

Class_ID SunDriverClassDesc::ClassID() 
{ 
	return SUNLIGHT_CLASS_ID; 
}

const TCHAR* SunDriverClassDesc::Category() 
{ 
	return _T("");  
}
