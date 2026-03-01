//**************************************************************************/
// Copyright (c) 2009 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#pragma once

#include <ParamDimension.h>

class TimeDimension : public ParamDimension 
{
public:
	DimType DimensionType();
	
	// Enforce range limits. Out-of-range values are reset to valid limits.
	float Convert(float value);
	
	float UnConvert(float value);
};

