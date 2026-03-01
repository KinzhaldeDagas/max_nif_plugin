//**************************************************************************/
// Copyright (c) 2022 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
#pragma once

#include "..\MaxGuid.h"

class QMenuBar;
class QMenu;

namespace MaxSDK::CUI
{
//-----------------------------------------------------------------------------
// IQtMenuExt
//-----------------------------------------------------------------------------
class IQtMenuExt
{
public:
	virtual ~IQtMenuExt() = default;

	/** \brief Returns the unique menu id associated with this Qt menu extension.*/
	virtual MaxSDK::MaxGuid GetId() const = 0;

	/** \brief Creates a new Qt menu bar from the underlying ICuiMenu structure.
	Ownership of the new menu bar takes the caller.
	\param deferred If true the menu item structure will be created deferred before
	the menu bar painting otherwise it'll be created directly. */
	virtual QMenuBar* CreateQtMenuBar(bool deferred = true) = 0;

	/** \brief Creates a new Qt menu from the underlying ICuiMenu structure.
	Ownership of the new menu takes the caller.
	\param deferred If true the menu item structure will be created deferred before
	the menu painting otherwise it'll be created directly. */
	virtual QMenu* CreateQtMenu(bool deferred = true) = 0;
};

//-----------------------------------------------------------------------------
// IWinMenuExt
//-----------------------------------------------------------------------------
class IWinMenuExt
{
public:
	virtual ~IWinMenuExt() = default;

	/** \brief Returns the unique menu id associated with this Window menu extension.*/
	virtual MaxSDK::MaxGuid GetId() const = 0;

	/** \brief This method will create a new windows menu and return it's handle. */
	virtual HMENU CreateWindowsMenu() = 0;

	/** \brief This method will update the current windows menu. */
	virtual void UpdateWindowsMenu() = 0;

	/** \brief This method returns the handle to the current windows menu. */
	virtual HMENU GetCurWindowsMenu() = 0;

	/** \brief This method executes an action based on the provided command
	 * \param cmdId The command ID of the action to execute. */
	virtual void ExecuteAction(int cmdId) = 0;

	/** \brief
	 * \param cmdId The command ID. */
	virtual bool CommandIDInRange(int cmdId) = 0;
};

} // namespace MaxSDK::CUI
