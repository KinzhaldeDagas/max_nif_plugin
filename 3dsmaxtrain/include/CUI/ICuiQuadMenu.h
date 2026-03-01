//**************************************************************************/
// Copyright (c) 2022 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
#pragma once

#include "ICuiQuadEnums.h"
#include "..\ifnpub.h"
#include "..\MaxGuid.h"
#include "..\strclass.h"
#include "..\tab.h"

namespace MaxSDK::CUI
{
	class ICuiMenu;
	class ICuiQuadMenuContext;

	//-----------------------------------------------------------------------------
	// ICuiQuadMenu
	//-----------------------------------------------------------------------------
	class ICuiQuadMenu : public FPMixinInterface
	{
	public:
		virtual ~ICuiQuadMenu() = default;
		static constexpr Interface_ID sInterfaceID = Interface_ID(0x10a72d9f, 0x691a62f2);

		/** \brief Returns the unique id associated with this quad menu.*/
		virtual MaxSDK::MaxGuid GetId() const = 0;

		/** \brief Sets the display title of the quad menu. This string becomes the default (non-localized) title, and will
		 * be returned by GetTitle().
		 * \param title The title of the quad menu. */
		virtual void SetTitle(const MSTR& title) = 0;

		/** \brief Returns the display title of the quad menu. If a localized title exists, it is returned, otherwise the
		 * non-localized (default) string is returned. If the title string is set by SetTitle(), it is considered the
		 * default string, and is always returned by this method.*/
		virtual MSTR GetTitle() const = 0;

		/** \brief Creates a new menu at the specified quad position.
		* \param id The unique id of the menu to be created.
		* \param title The title for the newly created menu.
		* \param quadPos The section of the quad menu where the new menu will be positioned.
		* \return A pointer to the newly created menu or nullptr if the menu creation failed. */
		virtual ICuiMenu* CreateMenu(const MaxSDK::MaxGuid& id, const MSTR& title, QuadPosition quadPos) = 0;

		/** \brief Returns a list of the four menus associated with this quad menu. */
		virtual Tab<ICuiMenu*> GetMenus() const = 0;

		/** \brief Returns the menu at the specified quad position.
		*\param quadPos The quad position for the menu that should be retrieved. */
		virtual ICuiMenu* GetMenuByPos(QuadPosition quadPos) const = 0;

		/** \brief Pops up a quad menu. */
		virtual void Popup(HWND hwnd) = 0;

		/** \brief Returns the quad menu context to which this quad menu belongs.
		* \return The owning quad menu context. */
		virtual ICuiQuadMenuContext* GetContext() const = 0;
	};

	//-----------------------------------------------------------------------------
	// ICuiQuadMenuContect
	//-----------------------------------------------------------------------------
	class ICuiQuadMenuContext : public FPMixinInterface
	{
	public:
		virtual ~ICuiQuadMenuContext() = default;
		static constexpr Interface_ID sInterfaceID = Interface_ID(0x4369492e, 0x6a435608);

		/** \brief Returns the unique id associated with this quad menu context.*/
		virtual MaxSDK::MaxGuid GetId() const = 0;

		/** \brief Returns the display title of the quad menu context. If a localized title exists, it is returned, otherwise the
		 * non-localized (default) string is returned. */
		virtual MSTR GetTitle() const = 0;

		/** \brief Creates a new quad menu with the given unique id and title.
		* \param id The unique id of the quad menu to be created.
		* \param title The title for the newly created quad menu.
		* \return A pointer to the newly created quad menu or nullptr if the quad menu creation failed. */
		virtual ICuiQuadMenu* CreateQuadMenu(const MaxSDK::MaxGuid& id, const MSTR& title) = 0;

		/** \brief Returns a list of all quad menus belonging to this quad menu context.*/
		virtual Tab<ICuiQuadMenu*> GetQuadMenus() const = 0;

		/** \brief Associates right click modifiers with the specified quad menu.
		* \param quadMenu The quad menu to which the right click modifiers should be associated.
		* \param modifiers The right click modifiers which should be assigned to this quad menu.
		* \return True if the right click modifier for the specified quad menu were changed. */
		virtual bool SetRightClickModifiers(ICuiQuadMenu* quadMenu, RightClickModifiers modifiers) = 0;

		/** \brief Returns the right click modifiers for the specified quad menu.
		* \param quadMenu The quad menu for which the associated right click modifiers should be retrieved.*/
		virtual RightClickModifiers GetRightClickModifiers(ICuiQuadMenu* quadMenu) const = 0;

		/** \brief Returns the quad menu which is associated with the according right click modifier,
		or nullptr if no quad menu is assigned.
		* \see GetRightClickMenuByModifiers()*/
		virtual ICuiQuadMenu* GetRightClickMenuByModifiers(RightClickModifiers modifiers) const = 0;

		/** \brief Returns the quad menu which is associated with current pressed modifier keys,
		or nullptr if no quad menu is assigned.
		* \see GetSystemRightClickModifiers(), GetRightClickMenuByModifiers(RightClickModifiers modifiers)*/
		ICuiQuadMenu* GetRightClickMenuByModifiers() const
		{
			return GetRightClickMenuByModifiers(GetSystemRightClickModifiers());
		}
	};
} // namespace MaxSDK::CUI
