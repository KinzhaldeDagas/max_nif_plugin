//**************************************************************************/
// Copyright (c) 2013 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
//Author : David Lanier

#include "InteractiveRenderingUtils.h"

// max sdk
#include <maxapi.h>
#include <render.h>
#include <NotificationAPI/NotificationAPIUtils.h> //MaxSDK header
#include <../NotificationSystem/NotificationAPIUtils.h> //Internal header

using namespace Max::InteractiveRendering;

bool Utils::IsActiveShadeViewLocked(void)
{
	// If we are not using the active view that means we are locked to another view
	return (!MaxSDK::NotificationAPIUtils::IsUsingActiveView(RS_IReshade));
}

void Utils::GetViewParams(ViewParams& outViewParams, ViewExp &viewportExp)
{
	outViewParams.projType = viewportExp.IsPerspView() ? PROJ_PERSPECTIVE : PROJ_PARALLEL;
	int perspective;
	float mat[4][4], hither, yon;
	Matrix3 invTM;
	viewportExp.getGW()->getCameraMatrix(mat, &invTM, &perspective, &hither, &yon);
	outViewParams.hither = hither;
	outViewParams.yon = yon;
	if (outViewParams.projType==PROJ_PERSPECTIVE)
	{
		outViewParams.fov = Max::NotificationAPI::Utils::GetFOVFromViewExpUsingSafeFrame(outViewParams.fov, viewportExp);//Taking into account safe frame if it is on
	}
	else
	{
		const float view_default_width = 400.0;
		GraphicsWindow* gw = viewportExp.getGW();
		outViewParams.zoom = viewportExp.GetScreenScaleFactor(
			Point3(0.0f, 0.0f, 0.0f));
		outViewParams.zoom *= gw->getWinSizeX() / ( gw->getWinSizeY() * view_default_width);
		outViewParams.zoom = Max::NotificationAPI::Utils::GetOrthoZoomFromViewExpUsingSafeFrame(outViewParams.zoom, viewportExp);//Taking into account safe frame if it is on
	}
	viewportExp.GetAffineTM(outViewParams.affineTM);
	outViewParams.prevAffineTM = outViewParams.affineTM;
}

