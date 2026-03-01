//**************************************************************************/
// Copyright (c) 2008 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
// DESCRIPTION: Geometry Checker Button Prototype
// AUTHOR: Michael Zyracki created 2008
//***************************************************************************/

#ifndef __GEOMETRY_BUTTON_H
#define __GEOMETRY_BUTTON_H

#include <IViewportButton.h>

TCHAR *GetString(int id);

//label is the location of the button. output is location of the output text.
static const int LABELX = 80;
static const int LABELY = 18;
static const int OUTPUTX = 80;
static const int OUTPUTY = 32;
static const int OFFSET = 16; //offset between txt

//main button for showing the checker and stats and picking different checkers
class GeometryButton : public ViewportTextButton
{
public:
	GeometryButton();
	~GeometryButton();
	//we override the OnAction implementation since we will pop up a menu
	bool OnAction(HWND hwnd,GraphicsWindow *gw,IPoint2 hitLoc, IViewportButton::Action action);
	//override display and getting the rect since we need to center the button based upon the viewport, which can change.
	void Display(TimeValue t, ViewExp *vpt, int flags);		
	void GetViewportRect( TimeValue t, ViewExp *vpt, Rect *rect );

	//since the button is now centered we need to reset the location
	virtual void ResetLocation(ViewExp *vpt);
    bool ShowInActiveViewportOnly(){return true;}

};

//button for bringing up the 'configure' button which pops up a dialog
class ConfigureButton : public GeometryButton
{
public:
	ConfigureButton():GeometryButton(){};
	~ConfigureButton(){};
	//we override the OnAction implementation since we will pop up a configure menu
	bool OnAction(HWND hwnd,GraphicsWindow *gw,IPoint2 hitLoc, IViewportButton::Action action);
	//since the button is now centered we need to reset the location
	void ResetLocation(ViewExp *vpt);
};

//button for perfoming an update when auto update is turned off
class AutoUpdateButton : public GeometryButton
{
public:
	AutoUpdateButton():GeometryButton(){};
	~AutoUpdateButton(){};
	//we override the OnAction implementation since we perform an update and redo the checker
	bool OnAction(HWND hwnd,GraphicsWindow *gw,IPoint2 hitLoc, IViewportButton::Action action);
	//since the button is now centered we need to reset the location
	void ResetLocation(ViewExp *vpt);};

#endif