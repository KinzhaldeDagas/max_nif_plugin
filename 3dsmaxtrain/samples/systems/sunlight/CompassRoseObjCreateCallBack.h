//**************************************************************************/
// Copyright (c) 2009 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#pragma once

#include <maxapi.h>
// forward declaration
class CompassRoseObject;

class CompassRoseObjCreateCallBack : public CreateMouseCallBack 
{
public:
	int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat );
	void SetObj(CompassRoseObject *obj);
	
private:
	Point3 p0;
	Point3 p1;
	CompassRoseObject *ob;
};


