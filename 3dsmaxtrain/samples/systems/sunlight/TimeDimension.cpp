//**************************************************************************/
// Copyright (c) 2009 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
#pragma unmanaged
#include "TimeDimension.h"
#include "autovis.h"



DimType TimeDimension::DimensionType() 
{
	return DIM_CUSTOM;
}

// Enforce range limits. Out-of-range values are reset to valid limits.
float TimeDimension::Convert(float value)
{
	// Convert seconds to hours.
	if (value < 0.0f)
		return 0.0f;
	else if (value >= SECS_PER_DAY)
		value = SECS_PER_DAY - 1;
	return value/3600.0f;
}

float TimeDimension::UnConvert(float value)
{
	// Convert hours to seconds.
	if (value < 0.0f)
		return 0.0f;
	else if (value >= 24.0f)
		return SECS_PER_DAY - 1;
	return value*3600.0f;
}
