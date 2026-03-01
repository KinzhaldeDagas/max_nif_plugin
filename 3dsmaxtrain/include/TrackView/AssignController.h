// ===========================================================================
// Copyright 2023 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
// ===========================================================================

//**************************************************************************/
// DESCRIPTION: Reusable methods for accessing the assigning control action and dialog.
//		Part of core.dll.
// AUTHOR: Autodesk Inc.
//**************************************************************************/
#pragma once

#include "../TrackView/TreeEntry.h"
#include <Geom/geom_span.hpp>
#include "../containers/Array.h"
#include "../control.h"
#include "../maxapi.h"
#include "../maxtypes.h"

class QMenu;
class QWidget;

namespace MaxSDK
{
	/** Procedure to assign new controllers to the entries in the 'list':
	-# Display the assign controller dialog
	-# Disables the command panel if its on the motion panel
	-# Replaces the selected controllers with the new selected one
	-# Re-enabled the command panel
	-# Redraw views.
	\param dialogParent Parent to the assign controller dialog.
	\param list List of destinations for the new controller.
	\param newControllersOut List of the newly created controllers corresponding to the input 'list'.
	\param clearMot Should be true if during the assignment of the new controllers the motion panel should be closed.
	\return Returns true if successful. */
	CoreExport bool PerformAssignController(QWidget* dialogParent, const geo::span<TreeEntry> list, MaxSDK::Array<Control*>& newControllersOut, bool clearMot = true);

	/** \brief Try to open the floating dialog for the Animatable described by the entry parameter.
	\param entry The Animatable whose parameter dialog we want to open with its parent context.
	\param time This time represents the horizontal position of where the user right clicked to
	bring up the modal edit track parameters dialog. See the flags below for when
	this parameter is valid.
	\param handle The handle to the parent window that should be used to create the dialogs.
	\param objParams An interface pointer available for calling functions in 3ds Max.
	\param flags One or more of the following values:\n\n
	<b>EDITTRACK_FCURVE</b>\n
	The user is in the function curve editor.\n\n
	<b>EDITTRACK_TRACK</b>\n
	The user is in one of the track views.\n\n
	<b>EDITTRACK_SCENE</b>\n
	The user is editing a path in the scene.\n\n
	<b>EDITTRACK_BUTTON</b>\n
	The user invoked by choosing the properties button. In this case the time
	parameter is NOT valid.\n\n
	<b>EDITTRACK_MOUSE</b>\n
	The user invoked by right clicking with the mouse. In this case the time
	parameter is valid. */
	CoreExport void OpenEditTrackParams(MaxSDK::TreeEntry entry, TimeValue time, HWND handle, IObjParam* objParams, DWORD flags);

	/** Populate a QMenu with actions to manipulate controllers.
	\param contextMenu Existing QMenu to add the action items to.
	\param parent Parent for any dialogs that open as a result of the added actions.
	\param entries List of entries to perform the added actions upon.
	\param suspendCommandPanel True to clear the command panel during paste or assign control actions. Ex: If contextMenu is popped from a UI control in the command panel, then pass false.
	\param enabledState Set to false to disable all added menu actions.
	\return Returns true if successful. */
	CoreExport bool PopulateControllerContextMenu(QMenu* contextMenu, QWidget* parent, geo::span<MaxSDK::TreeEntry> entries, bool suspendCommandPanel, bool enabledState);

}
