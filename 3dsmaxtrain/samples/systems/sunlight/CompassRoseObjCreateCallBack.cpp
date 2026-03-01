//**************************************************************************/
// Copyright (c) 2009 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
#pragma unmanaged
#include "CompassRoseObjCreateCallBack.h"
#include "compass.h"
#include <mouseman.h>


void CompassRoseObjCreateCallBack::SetObj(CompassRoseObject *obj) 
{ 
	ob = obj; 
}

int CompassRoseObjCreateCallBack::proc( ViewExp *vpt,
	int msg, 
	int point, 
	int flags, 
	IPoint2 m, 
	Matrix3& mat ) 
{ 

	if ( ! vpt || ! vpt->IsAlive() )
	{
		// why are we here
		DbgAssert(!_T("Invalid viewport!"));
		return FALSE;
	}

	if (msg == MOUSE_POINT)
	{
		if (point == 0)
		{
			ob->suspendSnap = TRUE;
			p0 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
			mat.SetTrans(p0);
		}
		else
		{
			ob->suspendSnap = FALSE;
			return 0;
		}
	}
	else if (msg==MOUSE_MOVE)
	{
		p1 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
		ob->axisLength = std::max(AXIS_LENGTH, Length(p1-p0));
		PostMessage(ob->hParams, RU_UPDATE, 0, 0);
	}
	else if (msg == MOUSE_ABORT)
	{     
		return CREATE_ABORT;
	}
	else if (msg == MOUSE_FREEMOVE)
	{
		vpt->SnapPreview(m,m,NULL,SNAP_IN_PLANE);
	}
	return 1;
}
