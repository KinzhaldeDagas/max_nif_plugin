//**************************************************************************/
// Copyright (c) 2022 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#pragma once

#include "decomp.h"

// DllExport for gtests.
#ifdef DllExport
#undef DllExport
#endif

#ifdef BUILD_CTRL
#define DllExport __declspec( dllexport )
#else
#define DllExport __declspec( dllimport )
#endif

class MatrixMath
{
public:
	// Move from basisParts towards effectingParts using the weighting parameters.
	// Base the interpolation of each component on the list controller implementations: position, rotation, scaling list controllers.
	// Does not work well with non-uniform scaling, the rotation components sign gets is all messed-up with teh scaling rotation.
	// - 'effectingPosScaleWeight' is separated from the rotation weight to allow TM list to follow rotation list implementation
	//		which is un-normalized (average param).
	// - 'effectingRotationWeight' will be clampped between [-1,1].
	// - weightIsAgainstBasis: false=weight against identity. true=for rotation and scale, this behaves like a lerp from basis to target.
	DllExport static AffineParts BlendAffineParts
		(const AffineParts& basisParts, const AffineParts& effectingParts,
		float effectingPosScaleWeight, float effectingRotationWeight, bool weightIsAgainstBasis);
};
