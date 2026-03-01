//**************************************************************************/
// Copyright (c) 2022 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
#pragma once

#include "..\actiontableTypedefs.h"
#include "..\ifnpub.h"
#include "..\MaxGuid.h"
#include "..\strclass.h"
#include "..\tab.h"

class ActionItem;
class IPoint2;

namespace MaxSDK::CUI
{

class ICuiActionMenuItem;
class ICuiMenu;

//-----------------------------------------------------------------------------
// ICuiMenuItem
//-----------------------------------------------------------------------------
class ICuiMenuItem : public FPMixinInterface
{
public:
	virtual ~ICuiMenuItem() = default;
	static constexpr Interface_ID sInterfaceID = Interface_ID(0x144c4de1, 0x511a31b4);

	/** \brief Returns the unique id associated with this menu item.*/
	virtual MaxSDK::MaxGuid GetId() const = 0;

	/** \brief Returns if menu item represents a separator.*/
	virtual bool IsSeparator() const = 0;
	/** \brief Returns if menu item is an action based menu item.*/
	virtual bool IsAction() const = 0;

	/** \brief Returns this menu item as ICuiActionMenuItem if the item is an action based menu item.*/
	virtual ICuiActionMenuItem* GetAction() const = 0;

	/** \brief Returns if menu item represents a sub menu.*/
	virtual bool IsSubMenu() const = 0;
	/** \brief Returns a pointer to the sub menu if the item represents a sub menu.*/
	virtual ICuiMenu* GetSubMenu() const = 0;
	/** \brief Returns if menu item represents a dynamic sub menu.*/
	virtual bool IsDynamicMenu() const = 0;

	/** \brief Returns the parent menu of this menu item.
	* \return The parent menu. */
	virtual ICuiMenu* GetParent() const = 0;
	/**! \brief Moves a menu item to a specific menu/menu bar, and at a specific position.
	* \param parentMenuId The unique id identifying the menu or menu bar where to insert the menu item.
	* \param beforeId The unique id of the menu item where the newly created menu item will be inserted before. */
	virtual bool Move(const MaxSDK::MaxGuid& parentMenuId, const MaxSDK::MaxGuid& beforeId = MaxSDK::MaxGuid()) = 0;
};

//-----------------------------------------------------------------------------
// ICuiActionMenuItem
//-----------------------------------------------------------------------------
class ICuiActionMenuItem : public ICuiMenuItem
{
public:
	static constexpr Interface_ID sInterfaceID = Interface_ID(0x702322a9, 0x22d63141);

	/** \brief Returns the ActionItem associated to this menu item. */
	virtual ActionItem* GetActionItem() const = 0;

	/** \brief Sets the display title of the menu item. This string becomes the default (non-localized) title, and will
	 * be returned by GetTitle().
	 * \param title The title of the menu item. */
	virtual void SetTitle(const MSTR& title) = 0;

	/** \brief Returns the display title of the menu item. If a localized title exists, it is returned, otherwise the
	 * non-localized (default) string is returned. If the title string is set by SetTitle(), it is considered the
	 * default string, and is always returned by this method.*/
	virtual MSTR GetTitle() const = 0;

	/** \brief In case the underlying actionItem is a placeholder for a dynamic menu
	* this method sets whether the dynamic menu should be displayed 'flat'.
	* Using this option will omit the creation of a sub menu entry for this dynamic menu action in the parent menu and 
	* will instead place all of its dynamically created menu items directly into the parent menu.
	* \param isFlat Set to true to display this menu item's content at the same level as this item, 
	* set to false to create a menu hierarchy. */
	virtual void SetIsMenuFlat(bool isFlat) = 0;

	/** \brief Returns true if the sub menu item should be
	displayed 'flat', otherwise false. */
	virtual bool IsMenuFlat() const = 0;
};

//-----------------------------------------------------------------------------
// ICuiMenu
//-----------------------------------------------------------------------------
class ICuiMenu : public FPMixinInterface
{
public:
	virtual ~ICuiMenu() = default;
	static constexpr Interface_ID sInterfaceID = Interface_ID(0x34787710, 0x3f5d54a7);

	/** \brief Returns the unique id associated with this menu.*/
	virtual MaxSDK::MaxGuid GetId() const = 0;

	/** \brief Sets the display title of the menu. This string becomes the default (non-localized) title, and will
	 * be returned by GetTitle().
	 * \param title The title of the menu. */
	virtual void SetTitle(const MSTR& title) = 0;

	/** \brief Returns the display title of the menu. If a localized title exists, it is returned, otherwise the
	 * non-localized (default) string is returned. If the title string is set by SetTitle(), it is considered the
	 * default string, and is always returned by this method.*/
	virtual MSTR GetTitle() const = 0;

	/** \brief Sets whether the sub menu should be displayed 'flat'.
	* Using this option will omit the creation of a sub menu entry for this menu in the parent menu and 
	* will instead place all of its menu items directly into the parent menu.
	* \param isFlat Set to true to display this menu item's content at the same level as this item, 
	* set to false to create a menu hierarchy.*/
	virtual void SetIsFlat(bool isFlat) = 0;

	/** \brief Returns true if the sub menu should be
	displayed 'flat', otherwise false. */
	virtual bool IsFlat() const = 0;

	/** \brief Returns the parent menu of this menu.
	* \return The parent menu, or nullptr if this menu is a top level menu. */
	virtual ICuiMenu* GetParent() const = 0;

	/** \brief Returns a list of menus items owned by this menu.
	* \return A list of menu items. */
	virtual Tab<ICuiMenuItem*> GetMenuItems() const = 0;

	/**! \brief Moves a sub menu to a specific menu/menu bar, and at a specific position.
	* \param parentMenuId The unique id identifying the menu or menu bar where to insert the menu item.
	* \param beforeId The unique id of the menu item where the newly created menu item will be inserted before. */
	virtual bool Move(const MaxSDK::MaxGuid& parentMenuId, const MaxSDK::MaxGuid& beforeId = MaxSDK::MaxGuid()) = 0;

	/** \brief Creates a new sub menu associated with the given unique id.
	* \param id The unique id of the menu to be created.
	* \param title The title of the menu.
	* \param beforeId The unique id of the menu item where the newly created menu item will be inserted before.
	* \return A pointer to the newly created sub menu or nullptr if the menu creation failed. */
	virtual ICuiMenu* CreateSubMenu(
		const MaxSDK::MaxGuid& id, const MSTR& title, const MaxSDK::MaxGuid& beforeId = MaxSDK::MaxGuid()) = 0;

	/** \brief Creates a new menu item associated with the action identified by its ActionId.
	* \param id The unique id of the menu item to be created.
	* \param tableId The unique ID for the ActionTable.
	* \param persistentActionId The persistent id of the action in the ActionTable to be executed by clicking on the created menu item.
	* \param title The title of the menu item. If no title is provided the default text of the ActionItem is used.
	* \param beforeId The unique id of the menu item before which the newly created menu item will be inserted.
	* \return A pointer to the newly created action menu item or nullptr if the menu item creation failed. 
	* \note For macroScript actions please use the in 3ds Max 2026 introduced CreateAction(const MaxSDK::MaxGuid& id,
	* const MSTR& macroScriptName, const MSTR& macroScriptCategory, ...) overload that uses the macroScript name and
	* category for action item identification.
	* For 3ds Max 2025 use this generic method for referencing macroScript actions by menu items. All defined macroScript actions 
	* are part of one specific MaxScript action table. The `tableId` for all the macroScript actions is always `647394`. 
	* The macroScript action itself is identified by its `persistentActionId` in that action table, which is constructed 
	* of the macro name concatenated with a backtick and the macroScript's non-localized category.
	* Example of referencing a macroScript earlier than in 3ds Max 2026:
	* @code
	* menu->CreateAction(MaxSDK::MaxGuid("2ef0e436-9542-4c27-86d7-36516874fa39"), 647394, _T("MenuAction1`Menu Demo Category"));
	* @endcode
	*/
	virtual ICuiActionMenuItem* CreateAction(const MaxSDK::MaxGuid& id, ActionTableId tableId,
		const MSTR& persistentActionId, const MSTR& title = MSTR(), const MaxSDK::MaxGuid& beforeId = MaxSDK::MaxGuid()) = 0;

	/** \brief Creates a new menu item associated with the action identified by its ActionId.
	* \param id The unique id of the menu item to be created.
	* \param tableId The unique ID for the ActionTable.
	* \param persistentActionId The persistent id of the action in the ActionTable to be executed by clicking on the created menu item.
	* \param title The title of the menu item. If no title is provided the default text of the
	* ActionItem is used.
	* \param beforeId The unique id of the menu item before which the newly created menu item will be inserted.
	* \return A pointer to the newly created action menu item or nullptr if the menu item creation failed. */
	virtual ICuiActionMenuItem* CreateAction(const MaxSDK::MaxGuid& id, ActionTableId tableId, int persistentActionId,
		const MSTR& title = MSTR(), const MaxSDK::MaxGuid& beforeId = MaxSDK::MaxGuid()) = 0;

	/** \brief Creates a new menu item associated with the macroScript action identified by its name and category.
	* \param id The unique id of the menu item to be created.
	* \param macroScriptName The name of the macroScript to be executed by clicking on the created menu item.
	* \param macroScriptCategory The category of the macroScript to be executed by clicking on the created menu item.
	* \param title The title of the menu item. If no title is provided the button text of the macroScript is used.
	* \param beforeId The unique id of the menu item before which the newly created menu item will be inserted.
	* \return A pointer to the newly created action menu item or nullptr if the menu item creation failed. */
	virtual ICuiActionMenuItem* CreateAction(const MaxSDK::MaxGuid& id, const MSTR& macroScriptName,
			const MSTR& macroScriptCategory, const MSTR& title = MSTR(),
			const MaxSDK::MaxGuid& beforeId = MaxSDK::MaxGuid()) = 0; 

	/** \brief Creates a new separator item with the given unique id.
	* \param id The unique id of the separator item to be inserted into the menu.
	* \param beforeId The unique id of the menu item before which the newly created menu item will be inserted.
	* \return A pointer to the newly created separator menu item or nullptr if the menu item creation failed. */
	virtual ICuiMenuItem* CreateSeparator(
		const MaxSDK::MaxGuid& id, const MaxSDK::MaxGuid& beforeId = MaxSDK::MaxGuid()) = 0;

	/** \brief Deletes the menu item with the given unique id.
	* \param id The unique id of the item to be deleted.
	* \return true if the item was present and has been deleted. */
	virtual bool DeleteItem(
		const MaxSDK::MaxGuid & id) = 0;

	/** \brief Pops up a context menu at the given position.
	* \param pos The screen position to display the menu.
	*/
	virtual void Popup(const IPoint2& pos) = 0;
	/** \brief Pops up a context menu at current mouse position. */
	virtual void Popup() = 0;
};
} // namespace MaxSDK::CUI
