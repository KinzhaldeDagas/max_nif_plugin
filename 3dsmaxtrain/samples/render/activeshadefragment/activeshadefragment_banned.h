//**************************************************************************/
// Copyright (c) 2018 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
#pragma once

// std::float_denorm_style is deprecated in C++23. It is used in header half.h
// Silence deprecation warnings in this project
#define _SILENCE_CXX23_DENORM_DEPRECATION_WARNING

// Encapsulates further nested banned.h errors in standard headers
#include <ios> //Include system headers before 3dsmax_banned.h, to avoid function deprecation from Windows SDK and VC 14 headers.
#include <format>
#include <functional>
#include <3dsmax_banned.h>
