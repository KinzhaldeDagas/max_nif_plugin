//**************************************************************************/
// Copyright (c) 2022 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
#pragma once

#include "..\ifnpub.h"
#include "..\MaxGuid.h"

namespace MaxSDK::CUI
{

/** \defgroup menuGuids Well known menu guids.
	Define some menu guids used by the code of MAX */
/// @{
const MaxSDK::MaxGuid kMainMenuId { "b4779ebb-a6f0-4815-9777-57c01c0b584c" };
const MaxSDK::MaxGuid kMaterialEditorMenuBarId { "19b889a1-e731-4b1a-b8ca-cdb1e51c4028" };
const MaxSDK::MaxGuid kSchematicViewMenuBarId { "e81dc994-6a38-4051-8e7a-e5f0349d91c1" };
const MaxSDK::MaxGuid kTrackViewMenuBarId { "be2fdd36-21be-4985-9869-3200df5b5f07" };
const MaxSDK::MaxGuid kAssetTrackingMenuBarId { "f2052ff3-05e9-4b24-84b2-18af5e60cc3b" };
const MaxSDK::MaxGuid kParameterCollectorMenuBarId { "144c1cb5-350e-4a3a-ad1e-ea77244fc9c9" };
const MaxSDK::MaxGuid kViewportMenuBarId { "41bb347d-76d1-46b6-b294-407f5b07b433" };
const MaxSDK::MaxGuid kTrackViewDopeSheetMenuBarId { "6eca4ccd-ff2f-4fc0-a464-08200a0e28aa" };
const MaxSDK::MaxGuid kUVWUnwrapMenuBarId { "ba107c75-ee75-4712-8e8b-f2d90a87d8aa" };
const MaxSDK::MaxGuid kSMEMenuBarId { "dd49ba13-7f3a-4a0d-bf22-72921bce4bdf" };
///@}


class ICuiMenu;
class ICuiMenuItem;
class IQtMenuExt;
class IWinMenuExt;

//-----------------------------------------------------------------------------
// ICuiMenuManager
//-----------------------------------------------------------------------------
class ICuiMenuManager : public FPMixinInterface
{
public:
	virtual ~ICuiMenuManager() = default;
	static constexpr Interface_ID sInterfaceID = Interface_ID(0x2c73022f, 0x92734408);

	/** \brief Creates a new top level menu with specified unique id.
	* Top level menus can be used as menu bars or standalone context menus.
	* \param id The unique id of the menu to be created.
	* \param title The title of the menu. 
	* \return A pointer to the newly created menu, or nullptr if the creation failed. */
	virtual ICuiMenu* CreateTopLevelMenu(const MaxSDK::MaxGuid& id, const MSTR& title) = 0;

	/** \brief Returns a list of all top level menus.
	* \return A list of top level menus. */
	virtual Tab<ICuiMenu*> GetTopLevelMenus() const = 0;

	/** \brief Returns the associated menu for a given unique id.
	* \param id The unique id of the menu which should be retrieved.
	* \return A pointer to the looked up menu or nullptr if no menu
	for the given id could be found. */
	virtual ICuiMenu* GetMenuById(const MaxSDK::MaxGuid& id) const = 0;

	/** \brief Returns the associated menu item for a given unique id.
	* \param id The unique id of the menu item which should be retrieved.
	* \return A pointer to the looked up menu item or nullptr if no menu
	item for the given id could be found. */
	virtual ICuiMenuItem* GetMenuItemById(const MaxSDK::MaxGuid& id) const = 0;

	/** \brief Returns the 3dsMax main menu bar. */
	virtual ICuiMenu* GetMainMenuBar() const = 0;

	/** \brief Loads the specified user menu configuration or a default 3ds Max menu configuration.
	* \param menuFile The file name of the menu configuration to be loaded.
	* \param logToFile If true, messages are logged to the cui log file and the max log file.
	* \return true if the new configuration could be loaded, false otherwise. */
	virtual bool LoadConfiguration(const MSTR& menuFile, bool logToFile = false) = 0;

	/** \brief Returns path to default configuration.
	* \return the path to the default configuration.
	*/
	virtual MSTR GetDefaultConfiguration() const = 0;

	/** \brief Set current configuration path, loads configuration only if path is different.
	* \param newConfigPath The file name of the menu configuration to be loaded.
	* \param logToFile If true, messages are logged to the cui log file and the max log file.
	* \return true if the provided path has been set.
	*/
	virtual bool SetCurrentConfiguration(const MSTR& newConfigPath, bool logToFile = false) = 0;

	/** \brief Returns the current loaded menu configuration file. */
	virtual MSTR GetCurrentConfiguration() const = 0;

	/** \brief Returns the Qt menu extension for a given menu.
	* The Qt menu extension can be used to create a Qt specific menu or menu bar.
	* \param id The unique id of the top level menu for which a Qt menu extension should be retrieved.
	* \return A pointer to the Qt menu extension. */
	virtual IQtMenuExt* GetQtMenuExt(const MaxSDK::MaxGuid& id) = 0;

	/** \brief Returns the Windows menu extension for a given menu.
	* The Windows menu extension can be used to create a win32 specific menu.
	* \param id The unique id of the top level menu for which a windows menu extension should be retrieved.
	* \return A pointer to the windows menu extension. */
	virtual IWinMenuExt* GetWinMenuExt(const MaxSDK::MaxGuid& id) = 0;

	/** \brief Create a localization resource file for a menu file.
	* \param menuFileName the path of the menu file to localize
	* \param outputFileName when successful, the not-normalized path where the resource file was written
	* \param outFolder the path of the folder where the res file should be generated
	* \return true if everything went ok */
	virtual bool CreateLocalizationFile(const MSTR& menuFileName, MSTR& outputFileName, const MSTR& outFolder = _T("")) = 0;

	/** \brief Returns the message log file path. */
	virtual MSTR GetMessageLogFile() const = 0;
};

} // namespace MaxSDK::CUI
