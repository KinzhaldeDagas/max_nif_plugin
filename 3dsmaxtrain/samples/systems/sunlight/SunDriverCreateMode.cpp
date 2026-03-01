//**************************************************************************/
// Copyright (c) 2009 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
#pragma unmanaged
#include "SunDriverCreateMode.h"
#include "sunlight.h"
#include <dllutilities.h>
#include <maxapi.h>
#include "MouseCursors.h"

#define CID_RINGCREATE	CID_USER + 0x509C2DF4

SunDriverCreateMode* SunDriverCreateMode::sInstance = nullptr;

SunDriverCreateMode* SunDriverCreateMode::GetInstance()
{
	if(nullptr == sInstance)
	{
		sInstance = new SunDriverCreateMode();
	}
	
	return sInstance;
}

void SunDriverCreateMode::Destroy()
{
	delete sInstance;
	sInstance = nullptr;
}

SunDriverCreateMode::SunDriverCreateMode() :
	ip(nullptr)
{ }

SunDriverCreateMode::~SunDriverCreateMode()
{ 
	ip = nullptr;
}

// These two methods just call the creation proc method of the same
// name. 
// This creates a new sun driver object and starts the editing
// of the objects parameters.  This is called just before the 
// command mode is pushed on the stack to begin the creation
// process.
void SunDriverCreateMode::Begin( IObjCreate *ioc, ClassDesc *desc, bool daylight ) 
{ 
	ip=ioc;
	proc.Begin( ioc, desc, daylight ); 
}

// This terminates the editing of the sun drivers parameters.
// This is called just before the command mode is removed from
// the command stack.
void SunDriverCreateMode::End() 
{ 
	proc.End(); 
}

// This returns the type of command mode this is.  See the online
// help under this method for a list of the available choices.
// In this case we are a creation command mode.
int SunDriverCreateMode::Class() 
{ 
	return CREATE_COMMAND; 
}

// Returns the ID of the command mode. This value should be the 
// constant CID_USER plus some random value chosen by the developer.
int SunDriverCreateMode::ID() 
{ 
	return CID_RINGCREATE; 
}

// This method returns a pointer to the mouse proc that will
// handle the user/mouse interaction.  It also establishes the number 
// of points that may be accepted by the mouse proc.  In this case
// we set the number of points to 100000.  The user process will 
// terminate before this (!) after the mouse proc returns FALSE.
// The mouse proc returned from this method is an instance of
// SunDriverCreationManager.  Note that that class is derived
// from MouseCallBack.
MouseCallBack *SunDriverCreateMode::MouseProc(int *numPoints) 
{ 
	*numPoints = 100000; return &proc; 
}

// This method is called to flag nodes in the foreground plane.
// We just return the standard CHANGE_FG_SELECTED value to indicate
// that selected nodes will go into the foreground.  This allows
// the system to speed up screen redraws.  See the Advanced Topics
// section on Foreground / Background planes for more details.
ChangeForegroundCallback *SunDriverCreateMode::ChangeFGProc() 
{ 
	return CHANGE_FG_SELECTED; 
}

// This method returns TRUE if the command mode needs to change the
// foreground proc (using ChangeFGProc()) and FALSE if it does not. 
BOOL SunDriverCreateMode::ChangeFG( CommandMode *oldMode ) 
{ 
	return (oldMode->ChangeFGProc() != CHANGE_FG_SELECTED); 
}

// This method is called when a command mode becomes active.  We
// don't need to do anything at this time so our implementation is NULL
void SunDriverCreateMode::EnterMode() 
{
	SetCursor(UI::MouseCursors::LoadMouseCursor(UI::MouseCursors::Crosshair));
	ip->PushPrompt( MaxSDK::GetResourceStringAsMSTR(IDS_SUN_CREATE_PROMPT).data() );
}

// This method is called when the command mode is replaced by 
// another mode.  Again, we don't need to do anything.
void SunDriverCreateMode::ExitMode() 
{
	ip->PopPrompt();
	SetCursor(LoadCursor(NULL, IDC_ARROW));
}

BOOL SunDriverCreateMode::IsSticky() 
{ 
	return FALSE; 
}

#if defined(NO_MOTION_PANEL)
bool SunDriverCreateMode::IsActive() const 
{
	return proc.IsActive(); 
}
#endif	// defined(NO_MOTION_PANEL)

