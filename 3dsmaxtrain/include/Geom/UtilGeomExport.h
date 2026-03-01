//**************************************************************************/
// Copyright (c) 1998-2022 Autodesk, Inc.
// All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license
//  agreement provided at the time of installation or download, or which
//  otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
#pragma once

// The Util library exports the Save/Load implementations.
#if defined(Geom_EXPORTS)
#define UTILGEOMEXPORT
#else
#if defined(_WIN32) || defined(_WIN64)
#if defined(BUILD_UTILGEOMETRY)
#define UTILGEOMEXPORT __declspec(dllexport)
#else
#define UTILGEOMEXPORT __declspec(dllimport)
#endif
#elif defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
#if defined(BUILD_UTILGEOMETRY)
#define UTILGEOMEXPORT __attribute__((visibility("default")))
#else
#define UTILGEOMEXPORT __attribute__((visibility("default")))
#endif
#endif
#endif
