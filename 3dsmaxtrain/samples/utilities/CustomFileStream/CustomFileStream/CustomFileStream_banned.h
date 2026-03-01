//**************************************************************************/
// Copyright (c) 2018 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
#pragma once

#include <stdio.h>  // DllMain.cpp --> maxscript/maxscript.h --> stdio.h

// Encapsulates further nested banned.h errors in standard headers
#include <ios> // maxscript/maxscript.h --> kernel/exceptions.h -> strclass.h --> maxstring.h --> <ios>
#include <format>
#include <3dsmax_banned.h>
