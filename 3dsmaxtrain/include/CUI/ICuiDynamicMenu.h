//**************************************************************************/
// Copyright (c) 2022 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
#pragma once

#include "ICuiMenu.h"

namespace MaxSDK::CUI
{
//-----------------------------------------------------------------------------
// ICuiDynamicMenu
//-----------------------------------------------------------------------------
class ICuiDynamicMenu : public ICuiMenu
{
public:
	static constexpr Interface_ID sInterfaceID = Interface_ID(0x649f687d, 0x6ac3037e);

	/// Constants used in AddItem
	enum DynamicMenuFlags
	{
		kDisabled = 1 << 0, //< Item is disabled (can't be selected)
		kChecked = 1 << 1, //< Item has a check mark beside it or is pressed 
	};

	/** \brief Adds a sub menu with the specified title.
	* \param title The title of the menu.
		* \return A pointer to the newly created dynamic menu.*/
	virtual ICuiDynamicMenu* AddSubMenu(const MSTR& title) = 0;

	/** \brief Adds a new menu item associated with the action identified by its ActionTableId and persistent ActionId.
		* \param tableId The unique ID for the ActionTable.
		* \param persistentActionId The persistent id of the action in the ActionTable to be executed by clicking on the created menu item.
		* \param title The title of the menu item. If no title is provided the default text of the ActionItem is used.
		* \return A pointer to the newly created action menu item or nullptr if the menu item creation failed. */
	virtual ICuiActionMenuItem* AddAction(ActionTableId tableId, const MSTR& persistentActionId, const MSTR& title = MSTR()) = 0;

	/** \brief Adds a new menu item associated with the action identified by its ActionTableId and ActionId.
		* \param tableId The unique ID for the ActionTable.
		* \param persistentActionId The persistent id of the action in the ActionTable to be executed by clicking on the created menu item.
		* \param title The title of the menu item. If no title is provided the default text of the ActionItem is used.
		* \return A pointer to the newly created action menu item or nullptr if the menu item creation failed. */
	virtual ICuiActionMenuItem* AddAction(ActionTableId tableId, int persistentActionId, const MSTR& title = MSTR()) = 0;

	/** \brief Adds a new action item associated with the macroScript action identified by its name and category.
	 * \param macroScriptName The name of the macroScript to be executed by clicking on the created menu item.
	 * \param macroScriptCategory The category of the macroScript to be executed by clicking on the created menu item.
	 * \param title The title of the menu item. If no title is provided the button text of the macroScript is used.
	 * \return A pointer to the newly created action menu item or nullptr if the menu item creation failed. */
	virtual ICuiActionMenuItem* AddAction(
			const MSTR& macroScriptName, const MSTR& macroScriptCategory, const MSTR& title = MSTR()) = 0; 

	/** \brief Adds a separator item.*/
	virtual ICuiMenuItem* AddSeparator() = 0;

	/** \brief Adds a command id based item to the dynamic menu.
		* \param itemCmdId The ID for the menu item.
		* \param title The name to appear for the menu item.
		* \param flags One or more of the DynamicMenuFlags.
		* \param iconName	The name of the multi-resolution icon to load. 
		*					The path, basename and the extension of the file is used to find matching icons.
		*					The Extension may be omitted and .png / .svg is assumed to be the default extension.
		*					The icon loading location and naming convention is described in detail inside
		*					the documentation of MaxSDK::LoadMaxMultiResIcon)
		* \param toolTip The tool tip of the menu item. Note that it is not displayed if it is equal to the title.
		* \return A pointer to the newly created action menu item or nullptr if the menu item creation failed. 
		* \see MaxSDK::LoadMaxMultiResIcon */
	virtual ICuiActionMenuItem* AddItem(UINT itemCmdId, const MSTR& title, DWORD flags = 0,
			const MSTR& iconName = MSTR(), const MSTR& toolTip = MSTR()) = 0;

	/** \brief Creates a command id based item to the dynamic menu with the given unique id.
		* \param menuItemId The unique id of the menu item to be inserted into the menu.
		* \param itemCmdId The ID for the menu item.
		* \param title The name to appear for the menu item.
		* \param flags One or more of the DynamicMenuFlags.
		* \param beforeId The unique id of the menu item where the newly created menu item will be inserted before.
		* \return A pointer to the newly created action menu item or nullptr if the menu item creation failed. */
	virtual ICuiActionMenuItem* CreateItem(MaxSDK::MaxGuid menuItemId, UINT itemCmdId, const MSTR& title,
		DWORD flags = 0, const MaxSDK::MaxGuid& beforeId = MaxSDK::MaxGuid()) = 0;
};
} // namespace MaxSDK::CUI
