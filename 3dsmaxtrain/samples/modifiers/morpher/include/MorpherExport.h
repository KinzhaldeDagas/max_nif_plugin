//**************************************************************************/
// Copyright (c) 2010 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
#pragma once

#ifdef MorphExport
#undef MorphExport
#endif

#ifdef BUILD_MORPHER 
#define MorphExport __declspec( dllexport )
#else
#define MorphExport __declspec( dllimport )
#endif
