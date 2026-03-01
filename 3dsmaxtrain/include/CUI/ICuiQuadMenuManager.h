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
#include "..\ifnpub.h"
#include "ICuiQuadEnums.h"

namespace MaxSDK::CUI
{
/** \defgroup quadMenuGuids Well known quad menu guids.
	Define some menu guids used by the code of MAX */
/// @{
const MaxSDK::MaxGuid kViewportQuadContextId { "ac7c70f8-3f86-4ff5-a510-e4fd6a9c368e" };
const MaxSDK::MaxGuid kViewportQuadMenuId { "9cf17647-303f-400e-938a-a55058804393" };
const MaxSDK::MaxGuid kSchematicViewQuadContextId { "1557a560-3b86-4879-bc03-b365c3ec5011" };
const MaxSDK::MaxGuid kSchematicViewQuadMenuId { "7856ba83-8035-48e3-8ced-f19c6925870e" };
const MaxSDK::MaxGuid kAssetTrackingQuadContextId { "cecf146f-dac3-462e-b313-f714c77d3546" };
const MaxSDK::MaxGuid kAssetTrackingQuadMenuId { "5fd6e18c-35e3-4a3e-ae77-51908066af0e" };
const MaxSDK::MaxGuid kUnwrapUVWQuadContextId { "a40ccd46-9bf7-4ed1-85d3-60c4890b5eeb" };
const MaxSDK::MaxGuid kUnwrapUVWQuadMenuId { "aec4565f-ff3f-44ba-8041-fe36c8063f4b" };
const MaxSDK::MaxGuid kReshadeQuadContextId { "da30427a-372d-49e7-86c5-0f06330616de" };
const MaxSDK::MaxGuid kReshadeQuadMenuId { "db6292d1-72b1-4c7b-995d-f8dac71e5541" }; 
const MaxSDK::MaxGuid kTrackViewQuadContextId { "c280cc06-c034-49c4-99eb-a0e40f2b85d3" };
const MaxSDK::MaxGuid kTrackViewQuadMenuId { "adf775df-e23d-422e-8916-6f949522beda" };
const MaxSDK::MaxGuid kTrackViewKeyQuadClassicMenuId { "58e0de85-3a59-4164-9213-eee1746de22c" };
const MaxSDK::MaxGuid kTrackViewKeyQuadMenuId { "897a0edf-03cc-48a3-8dbd-7dc5404ec852" };
const MaxSDK::MaxGuid kRetimerSpanTypeContextId { "c5a6163e-08bd-43b1-9333-b380bfb62dfe" };
const MaxSDK::MaxGuid kRetimerSpanTypeMenuId { "f51d4fc2-52a3-40b3-9fdb-5f08d8da95e3" };
///@}


// Forward declarations
class ICuiQuadMenuContext;
class ICuiMenu;
class ICuiMenuItem;
class ICuiQuadMenu;

//-----------------------------------------------------------------------------
// ICuiQuadMenuManager
//-----------------------------------------------------------------------------
class ICuiQuadMenuManager : public FPMixinInterface
{
public:
	virtual ~ICuiQuadMenuManager() = default;
	static constexpr Interface_ID sInterfaceID = Interface_ID(0x6ecd32ae, 0x710a24ec);

	/** \brief Creates a new quad menu context with the given unique id, and title.
	* \param id The unique id of the context to be created.
	* \param title The name for the newly created quad menu context. 
	* \return A pointer to the newly created quad menu context or 
	nullptr if the quad menu context creation failed. */
	virtual ICuiQuadMenuContext* CreateQuadMenuContext(const MaxSDK::MaxGuid& id, const MSTR& title) = 0;

	/** \brief Returns a list of all quad menu contexts.*/
	virtual Tab<ICuiQuadMenuContext*> GetContexts() const = 0;

	/** \brief Returns the associated quad menu context for a given unique id.
	* \param id The unique id of the context which should be retrieved.
	* \return A pointer to the looked up quad menu context or nullptr if no context
	for the given id could be found. */
	virtual ICuiQuadMenuContext* GetContextById(const MaxSDK::MaxGuid& id) const = 0;

	/** \brief Returns the associated quad menu for a given unique id.
	* \param id The unique id of the quad menu which should be retrieved.
	* \return A pointer to the looked up quad menu or nullptr if no quad menu
	for the given id could be found. */
	virtual ICuiQuadMenu* GetQuadMenuById(const MaxSDK::MaxGuid& id) const = 0;

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

	/** \brief Loads the specified user quad menu configuration or a default 3ds Max quad menu configuration.
	* \param quadmenuFile The file name of the quad menu configuration to be loaded.
	* \param logToFile If true, messages are logged to the cui log file and the max log file.
	* \return true if the new configuration could be loaded, false otherwise. */
	virtual bool LoadConfiguration(const MSTR& quadmenuFile, bool logToFile = false) = 0;

	/** \brief Returns path to default configuration.
	* \return the path to the default configuration.
	*/
	virtual MSTR GetDefaultConfiguration() const = 0;

	/** \brief Set current configuration path, loads configuration only if path is different.
	* \param newConfigPath The file name of the quad menu configuration to be loaded.
	* \param logToFile If true, messages are logged to the cui log file and the max log file.
	* \return true if the provided path has been set.
	*/
	virtual bool SetCurrentConfiguration(const MSTR& newConfigPath, bool logToFile = false) = 0;

	/** \brief Returns the current loaded quad menu configuration file. */
	virtual MSTR GetCurrentConfiguration() const = 0;
    
	/** \brief Push a viewport context id on the stack of viewport contexts. */
	virtual void PushViewportContextId(const MaxSDK::MaxGuid& id) = 0;

	/** \brief Pop a viewport context from the stack of viewport contexts. */
	virtual void PopViewportContextId() = 0;

	/** \brief Find from the stack of viewport context, which context should handle a given modifier. */
	virtual ICuiQuadMenuContext* GetViewportContextByModifiers(RightClickModifiers mods) const = 0;

	/** \brief Create a localization resource file for a quad menu file.
	* \param menuFileName the path of the menu file to localize
	* \param outputFileName when successful, the not-normalized path where the resource file was written
	* \param outFolder the path of the folder where the res file should be generated
	* \return true if everything went ok */
	virtual bool CreateLocalizationFile(const MSTR& menuFileName, MSTR& outputFileName, const MSTR& outFolder = _T("")) = 0;

	/** \brief Returns the message log file path. */
	virtual MSTR GetMessageLogFile() const = 0;

	/** \brief Helper function to pop up a quad menu from the provided context according to current keyboard modifiers
	* \see GetContextById, GetRightClickMenuByModifiers, GetSystemRightClickModifiers
	* \param contextId Id of quad context to get the quad menu from
	* \param hWnd handle to parent window of quad menu
	* \return true if both context and quad were found, false otherwise
	* \code Implementation:
		if (auto* context = GetContextById(contextId))
		{
			if (auto* quadMenu = context->GetRightClickMenuByModifiers())
			{
				quadMenu->Popup(hWnd);
				return true;
			}
		}
		return false;
	* \endcode
	*/
	virtual bool PopupQuadFromContext(const MaxSDK::MaxGuid& contextId, HWND hWnd = nullptr) const = 0;
};

} // namespace MaxSDK::CUI
