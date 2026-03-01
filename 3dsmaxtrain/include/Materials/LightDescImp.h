//**************************************************************************/
// Copyright (c) 2023 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
#pragma once

#include "..\imtl.h"

//* Basic implementation of LightDesc to use in ShadeContext implementations
struct LightDescImp : public LightDesc
{
	Point3 pos;
	Color col;
	BOOL Illuminate(ShadeContext& sc, Point3& normal, Color& color, Point3& dir, float& dot_nl, float&) override
	{
		dir = Normalize(pos - sc.P());
		dot_nl = DotProd(normal, dir);
		color = col;
		return TRUE;
	}
};
