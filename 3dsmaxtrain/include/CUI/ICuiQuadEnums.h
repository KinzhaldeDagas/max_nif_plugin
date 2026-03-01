//**************************************************************************/
// Copyright (c) 2022 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
#pragma once

#include <cstdint>
#include <windows.h>
#include <WinUser.h>

namespace MaxSDK::CUI
{
/*! \defgroup right click modifier flags
This is a list of flags that determine modifier keys in effect when right clicking for a quad menu */
//!@{
enum RightClickModifiersFlags : uint8_t
{
	kNoModifier = 0,		//!< No modifiers
	kShiftModifier = 1,		//!< The shift modifiers is pressed
	kAltModifier = 2,		//!< The alt modifiers is pressed
	kControlModifier = 4	//!< The control modifiers is pressed
};
//!@}

/** \brief a combination of RightClickModifiersFlags */
using RightClickModifiers = uint8_t;

/** \brief The possible positions of quad inside a quad menu. */
enum class QuadPosition : uint8_t
{
	kTopLeft = 0,
	kTopRight,
	kBottomRight,
	kBottomLeft,
	kInvalid
};

/** \brief Queries the state of the modifier keys and
returns the appropriate right click modifier flags. */
inline RightClickModifiers GetSystemRightClickModifiers()
{
	const bool controlPressed = GetKeyState(VK_CONTROL) & 0x8000;
	const bool altPressed = GetKeyState(VK_MENU) & 0x8000;
	const bool shiftPressed = GetKeyState(VK_SHIFT) & 0x8000;
	return controlPressed * kControlModifier | altPressed * kAltModifier | shiftPressed * kShiftModifier;
}

} //namespace MaxSDK::CUI
