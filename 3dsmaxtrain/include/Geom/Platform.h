//**************************************************************************/
// Copyright (c) 2022 Autodesk, Inc.
// All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license
//  agreement provided at the time of installation or download, or which
//  otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
#pragma once
#include <cstdint>

#undef GEOM_WINDOWS
#undef GEOM_UNIX
#undef GEOM_LINUX
#undef GEOM_MACOS
#undef GEOM_FORCEINLINE
#undef GEOM_32BIT
#undef GEOM_64BIT

#if defined(_WIN32) || defined(_WIN64)
#define GEOM_WINDOWS 1
#elif defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
#define GEOM_UNIX 1

#if defined(__linux__)
#define GEOM_LINUX 1
#else
#define GEOM_MACOS 1
#endif
#endif

#if defined(GEOM_WINDOWS)
#define GEOM_FORCEINLINE __forceinline
#else
#define GEOM_FORCEINLINE __attribute__((always_inline))
#endif

#if INTPTR_MAX == INT32_MAX
#define GEOM_32BIT 1
#else
#define GEOM_64BIT 1
#endif

