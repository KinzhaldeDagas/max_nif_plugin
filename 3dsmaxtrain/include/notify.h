/**********************************************************************
 *<
	FILE: notify.h

	DESCRIPTION: Include file for event notification support

	CREATED BY: Tom Hudson

	HISTORY: Created 8 April 1995

 *>	Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "assert1.h"
#ifdef MAX_ASSERTS_ACTIVE
#include <algorithm>
#endif
#include "maxheap.h"
#include "coreexp.h"
#include "strbasic.h"
#include "maxtypes.h"
#include "notifyParams.h"

// Pre-defined Max system notification codes
/**
	\defgroup NotificationCodes System Notification Codes
	The following pre-defined system notification codes may be passed to the global functions 
	RegisterNotification(), UnRegisterNotification(), BroadcastNotification().
	
	\sa Structure NotifyInfo, Class Interface, Class Bitmap, Class RenderGlobalContext.
*/
 
//!@{

//! \brief Helper class to assign a param type to a notification code. See GetNotifyParam.
// Template specialization for other codes is done in macro DEFINE_NOTIFY_CODE.
template <NotifyCode code>
struct notify_param_helper { using type = void*; }; // The default type is void*

//! \brief Helper type alias to assign a param type to a notification code. See GetNotifyParam.
//! For example, notify_param_t<NOTIFY_FILE_PRE_OPEN> is the type of the callParam sent with the NOTIFY_FILE_PRE_OPEN notification.
template <NotifyCode code>
using notify_param_t = typename notify_param_helper<code>::type;

/** !\brief Use this function to retrieve the callParam that is associated with one or more notification codes.
* It is recommended to do a null check on the value returned by this function.
* This will fail to compile if all notification codes don't have the same associated param type.
\code
void MyCallback(void* param, NotifyInfo* info)
{
	switch (info->intcode)
	{
	case NOTIFY_FILE_PRE_OPEN:
	{
		if (auto callParam1 = GetNotifyParam<NOTIFY_FILE_PRE_OPEN>(info))
		{
			... // callParam1 will resolve to FileIOType*
		}
		break;
	}
	case NOTIFY_FILE_POST_OPEN:
	{
		if (auto callParam2 = GetNotifyParam<NOTIFY_FILE_POST_OPEN>(info))
		{
			... // callParam2 will resolve to NotifyPostOpenParam*
		}
		break;
	}
	case NOTIFY_NODE_CREATED:
	case NOTIFY_NODE_UNHIDE:
	case NOTIFY_NODE_UNFREEZE:
	{
		if (auto callParam3 = GetNotifyParam<NOTIFY_NODE_CREATED, NOTIFY_NODE_UNHIDE, NOTIFY_NODE_UNFREEZE>(info))
		{
			... // callParam3 will resolve to INode*
		}
		break;
	}
	}
}
\endcode
*/
template <NotifyCode code, NotifyCode... more_codes>
inline notify_param_t<code> GetNotifyParam(NotifyInfo* info)
{
	using result_t = notify_param_t<code>;
	if constexpr (sizeof...(more_codes) > 0)
	{
		static constexpr bool allSameParamType = (std::is_same_v<result_t, notify_param_t<more_codes>> && ...);
		static_assert(allSameParamType, "All notification codes don't have the same associated param type");
	}
#ifdef MAX_ASSERTS_ACTIVE
	if (info)
	{
		// Raise a DbgAssert if the notification code sent at run time is different to what was expected at compile time
		static constexpr NotifyCode allCodes[] = { code, more_codes... };
		const bool goodCode = std::any_of(std::begin(allCodes), std::end(allCodes),
			[&](NotifyCode acode) { return acode == info->intcode; });
		DbgAssert(goodCode && "Unexpected notification code in GetNotifyParam");
	}
#endif
	void* paramPtr = info ? info->callParam : nullptr;
	if constexpr (std::is_pointer_v<result_t>)
	{
		return static_cast<result_t>(paramPtr);
	}
	else
	{
		return static_cast<result_t>(reinterpret_cast<intptr_t>(paramPtr));
	}
}


//! \brief Macro to define a notification code and associated type, and register it for GetNotifyParam.
#define DEFINE_NOTIFY_CODE(notifyCodeName, codeVal, callParamType)\
inline constexpr NotifyCode notifyCodeName = codeVal;\
template <> struct notify_param_helper<notifyCodeName> { using type = callParamType; };


//! \brief Sent if the user changes the unit setting
#define NOTIFY_UNITS_CHANGE				0x00000001
//! \brief Sent if the user changes the time format setting
#define NOTIFY_TIMEUNITS_CHANGE	 		0x00000002 
//! \brief Sent if the user changes the viewport layout
#define NOTIFY_VIEWPORT_CHANGE			0x00000003 
//! \brief Sent if the user changes the reference coordinate system.
#define NOTIFY_SPACEMODE_CHANGE	 		0x00000004 
//! \brief Sent before 3ds Max system is reset
#define NOTIFY_SYSTEM_PRE_RESET	 		0x00000005 
//! \brief Sent after 3ds Max system is reset
#define NOTIFY_SYSTEM_POST_RESET 		0x00000006 
//! \brief Sent before a new scene is requested. 
//! NotifyInfo::callParam is \ref new_scene_options "New Scene Options"
DEFINE_NOTIFY_CODE(NOTIFY_SYSTEM_PRE_NEW, 0x00000007, DWORD*)
//! \brief Sent after a new scene requested has been serviced. 
//! NotifyInfo::callParam is \ref new_scene_options "New Scene Options"
DEFINE_NOTIFY_CODE(NOTIFY_SYSTEM_POST_NEW, 0x00000008, DWORD*)
//! \brief Sent before a file is opened. NotifyInfo::callParam is a pointer to type FileIOType.
DEFINE_NOTIFY_CODE(NOTIFY_FILE_PRE_OPEN, 0x00000009, FileIOType*);
//! \brief Sent after a file is opened successfully. 
/* NotifyInfo::callParam is a pointer to a NotifyPostOpenParam */
DEFINE_NOTIFY_CODE(NOTIFY_FILE_POST_OPEN, 0x0000000A, NotifyPostOpenParam*);
//! \brief Sent before a file is merged.
/** When merge is called to load an XRef, you can determine this by testing 
void * <b>callParam</b>. The result is <b>not NULL </b> if an XRef is being loaded. */
DEFINE_NOTIFY_CODE(NOTIFY_FILE_PRE_MERGE, 0x0000000B, DWORD*);
//! \brief Sent before a file is saved (NotifyInfo::callParam is a pointer to a string (MCHAR *) of the file name).
DEFINE_NOTIFY_CODE(NOTIFY_FILE_PRE_SAVE, 0x0000000D, const MCHAR*);
//! \brief Sent after a file is saved (NotifyInfo::callParam is a pointer to a string (MCHAR *) of the file name).
DEFINE_NOTIFY_CODE(NOTIFY_FILE_POST_SAVE, 0x0000000E, const MCHAR*);
//! \brief Sent after a file open fails.  NotifyInfo::callParam is a pointer to type FileIOType. 
DEFINE_NOTIFY_CODE(NOTIFY_FILE_OPEN_FAILED, 0x0000000F, FileIOType*);
//! \brief Sent before an old version file is saved. 
/* NotifyInfo::callParam is a pointer to DWORD, which stores the version of the file to be saved - the same value returned by ISave::SavingVersion(). */
DEFINE_NOTIFY_CODE(NOTIFY_FILE_PRE_SAVE_OLD, 0x00000010, DWORD*);
//! \brief Sent after an old version file is saved
#define NOTIFY_FILE_POST_SAVE_OLD		0x00000011 
//! \brief Sent after the selection set has changed
#define NOTIFY_SELECTIONSET_CHANGED		0x00000012 
//! \brief Sent after a bitmap is reloaded.
/** The NotifyInfo::callParam is passed the MCHAR * to the 
bitmap file name. This is used for updating bitmaps that have changed. The 
callParam is used to pass the name of the bitmap file in case it is used in 
multiple changes. If the callParam is NULL, this notification applies to all 
bitmaps, as is the case when the input file gamma changes.  */
DEFINE_NOTIFY_CODE(NOTIFY_BITMAP_CHANGED, 0x00000013, const MCHAR*);
//! \brief Sent before rendering starts
DEFINE_NOTIFY_CODE(NOTIFY_PRE_RENDER, 0x00000014, RendParams*)
//! \brief Sent after rendering has finished.
#define NOTIFY_POST_RENDER				0x00000015 
//! \brief Sent before rendering each frame.
/** NotifyInfo::callParam is passed as a 
pointer to the <b>RenderGlobalContext</b>. At the time of this call, the scene <b>must not</b> be modified. 
The renderer has already called <b>GetRenderMesh()</b> on all the object instances, and the materials and 
lights are already updated. If you don't modify anything that is rendered, then it is safe to use this 
callback. */
DEFINE_NOTIFY_CODE(NOTIFY_PRE_RENDERFRAME, 0x00000016, RenderGlobalContext*);
//! \brief Sent after rendering each
/** NotifyInfo::callParam is passed a pointer to the RenderGlobalContext.
\sa NOTIFY_PRE_RENDERFRAME */
DEFINE_NOTIFY_CODE(NOTIFY_POST_RENDERFRAME, 0x00000017, RenderGlobalContext*);
//! \brief Sent before a file is imported.
#define NOTIFY_PRE_IMPORT				0x00000018 
//! \brief Sent after a file is imported successfully
/** NotifyInfo::callParam is a pointer to a string (MCHAR *) of the file name. */
DEFINE_NOTIFY_CODE(NOTIFY_POST_IMPORT, 0x00000019, const MCHAR*);
//! \brief Sent if a file import fails or is cancelled
DEFINE_NOTIFY_CODE(NOTIFY_IMPORT_FAILED, 0x0000001A, NotifyImpExpFailedParam*)
//! \brief Sent before a file is exported
#define NOTIFY_PRE_EXPORT				0x0000001B 
//! \brief Sent after a file is exported successfully
/** NotifyInfo::callParam is a pointer to a string (MCHAR *) of the file name. */
DEFINE_NOTIFY_CODE(NOTIFY_POST_EXPORT, 0x0000001C, const MCHAR*);
//! \brief Sent if a export fails or is cancelled
DEFINE_NOTIFY_CODE(NOTIFY_EXPORT_FAILED, 0x0000001D, NotifyImpExpFailedParam*)

//! \defgroup NameChangeNotifications The name change notifications.
/*! These kinds of notifications are sent when the name of a target has been changed.
All these notifications use the struct NameChange as their call parameter.
\see NameChange
\see NOTIFY_NODE_RENAMED,NOTIFY_SCENESTATE_RENAME,NOTIFY_NAMED_SEL_SET_RENAMED
*/

//! \ingroup NameChangeNotifications
//! \brief Sent if a is node renamed.
/** See <b>\\MAXSDK\\SAMPLES\\OBJECTS\\LIGHT.CPP</b> for an example of this notification in use. */
DEFINE_NOTIFY_CODE(NOTIFY_NODE_RENAMED, 0x0000001E, NameChange*)

//! \brief Sent before the progress bar is displayed.
/** The progress bar is displayed, for example, when the Render 
Preview command is run. If a plug-in uses a modeless window it should hide the window between this event and 
\ref NOTIFY_POST_PROGRESS. */
#define NOTIFY_PRE_PROGRESS				0x0000001F 
//! \brief Sent after the progress bar is finished
#define NOTIFY_POST_PROGRESS			0x00000020 
//! \brief Sent when the modify panel focuses on a new object because of opening the Modify panel or changing selection.
#define NOTIFY_MODPANEL_SEL_CHANGED		0x00000021 
//! \brief Sent when the common renderer parameters have changed via the render dialog
#define NOTIFY_RENDPARAM_CHANGED		0x00000023

/** \name Material Library File Notifications */
//!@{ 
//! \brief Sent before loading a material library
#define NOTIFY_MATLIB_PRE_OPEN			0x00000024 
//! \brief Sent after loading a material library.
/** NotifyInfo::callParam is a pointer to <b>MtlBaseLib</b> if success, otherwise NULL. */
DEFINE_NOTIFY_CODE(NOTIFY_MATLIB_POST_OPEN, 0x00000025, MtlBaseLib*)
//! \brief Sent before saving a material library
#define NOTIFY_MATLIB_PRE_SAVE			0x00000026 
//! \brief Sent after saving a material library
#define NOTIFY_MATLIB_POST_SAVE			0x00000027 
//! \brief Sent before merging a material library
#define NOTIFY_MATLIB_PRE_MERGE			0x00000028 
//! \brief Sent after merging a material library
#define NOTIFY_MATLIB_POST_MERGE		0x00000029 
//!@}

//! \brief Sent if a File Link Bind fails
#define NOTIFY_FILELINK_BIND_FAILED		0x0000002A 
//! \brief Sent if a File Link Detach fails
#define NOTIFY_FILELINK_DETACH_FAILED	0x0000002B 
//! \brief Sent if a File Link Reload fails
#define NOTIFY_FILELINK_RELOAD_FAILED	0x0000002C 
//! \brief Sent if a File Link Attach fails
#define NOTIFY_FILELINK_ATTACH_FAILED	0x0000002D 
//! \brief Sent before a File Link Bind
#define NOTIFY_FILELINK_PRE_BIND		0x00000030 
//! \brief Sent after a successful File Link Bind
#define NOTIFY_FILELINK_POST_BIND		0x00000031 
//! \brief Sent before a File Link Detach
#define NOTIFY_FILELINK_PRE_DETACH		0x00000032 
//! \brief Sent after a successful File Link Detach
#define NOTIFY_FILELINK_POST_DETACH		0x00000033 
//! \brief Sent before a File Link Reload (partial, full, or dynamic)
#define NOTIFY_FILELINK_PRE_RELOAD		0x00000034 
//! \brief Sent after a successful File Link Reload (partial, full, or dynamic)
#define NOTIFY_FILELINK_POST_RELOAD		0x00000035 
//! \brief Sent before a File Link Attach
#define NOTIFY_FILELINK_PRE_ATTACH		0x00000036 
//! \brief Sent after a successful File Link
/** \sa NOTIFY_FILELINK_POST_RELOAD_PRE_PRUNE */
#define NOTIFY_FILELINK_POST_ATTACH		0x00000037 

//! \brief Sent before the renderer starts evaluating objects; NotifyInfo::callParam is a pointer to TimeValue.
/** The NotifyInfo::callParam is passed as a pointer to TimeValue. 
Renderer plugins must broadcast this notification before they start evaluation 
scene objects. This notification allows plugins (such as modifiers or base objects) 
to perform a custom task before the renderer evaluates them. The custom task is 
usually one that would be invalid to be performed during a call to INode::EvalWorldState().*/
DEFINE_NOTIFY_CODE(NOTIFY_RENDER_PREEVAL, 0x00000039, TimeValue*)
//! \brief Sent when a node is created (NotifyInfo::callParam is a pointer to the node).
DEFINE_NOTIFY_CODE(NOTIFY_NODE_CREATED, 0x0000003A, INode*)
//! \brief Sent when a node is linked (NotifyInfo::callParam is a pointer to the node).
DEFINE_NOTIFY_CODE(NOTIFY_NODE_LINKED, 0x0000003B, INode*)
//! \brief Sent when a node is unlinked (NotifyInfo::callParam is a pointer to the node).
DEFINE_NOTIFY_CODE(NOTIFY_NODE_UNLINKED, 0x0000003C, INode*)
//! \brief Sent when a node is hidden (NotifyInfo::callParam is a pointer to the node).
DEFINE_NOTIFY_CODE(NOTIFY_NODE_HIDE, 0x0000003D, INode*)
//! \brief Sent when a node is unhidden (NotifyInfo::callParam is a pointer to the node).
DEFINE_NOTIFY_CODE(NOTIFY_NODE_UNHIDE, 0x0000003E, INode*)
//! \brief Sent when a node is frozen (NotifyInfo::callParam is a pointer to the node).
DEFINE_NOTIFY_CODE(NOTIFY_NODE_FREEZE, 0x0000003F, INode*)
//! \brief Sent when a node is unfrozen (NotifyInfo::callParam is a pointer to the node).
DEFINE_NOTIFY_CODE(NOTIFY_NODE_UNFREEZE, 0x00000040, INode*)
//! \brief Node is about to get a new material (NotifyInfo::callParam is a pointer to the node).
DEFINE_NOTIFY_CODE(NOTIFY_NODE_PRE_MTL, 0x00000041, INode*)
//! \brief Node just got a new material (NotifyInfo::callParam is a pointer to the node).
DEFINE_NOTIFY_CODE(NOTIFY_NODE_POST_MTL, 0x00000042, INode*)
//! \brief Node just added to scene (NotifyInfo::callParam is a pointer to the node).
DEFINE_NOTIFY_CODE(NOTIFY_SCENE_ADDED_NODE, 0x00000043, INode*)
//! \brief Node just removed from scene (NotifyInfo::callParam is a pointer to the node).
DEFINE_NOTIFY_CODE(NOTIFY_SCENE_PRE_DELETED_NODE, 0x00000044, INode*)
//! \brief Node just removed from scene
#define NOTIFY_SCENE_POST_DELETED_NODE	0x00000045 

//! \brief selected nodes will be deleted. (NotifyInfo::callParam is a pointer to Tab<INode*>)
DEFINE_NOTIFY_CODE(NOTIFY_SEL_NODES_PRE_DELETE, 0x00000046, Tab<INode*>*)
//! \brief Selected nodes just deleted.
#define NOTIFY_SEL_NODES_POST_DELETE	0x00000047 

// following sent in 2025.1 and later
//! \brief Nodes will be deleted. (NotifyInfo::callParam is a pointer to const INodeTab)
DEFINE_NOTIFY_CODE(NOTIFY_NODES_PRE_DELETE, 0x000000CE, const INodeTab*)
//! \brief Nodes just deleted.
#define NOTIFY_NODES_POST_DELETE 0x000000CF

//! \brief Sent when main window gets an WM_ENABLE (BOOL enabled)
DEFINE_NOTIFY_CODE(NOTIFY_WM_ENABLE, 0x00000048, BOOL)

//! \brief 3ds Max is about to exit,  (system shutdown starting)
#define NOTIFY_SYSTEM_SHUTDOWN			0x00000049 
//! \brief 3ds Max just went live
#define NOTIFY_SYSTEM_STARTUP			0x00000050 

//! \brief A plug-in was just loaded. (NotifyInfo::callParam is a pointer to DllDesc).
DEFINE_NOTIFY_CODE(NOTIFY_PLUGIN_LOADED, 0x00000051, DllDesc*)

//! \brief Last broadcast before exit, after the scene is destroyed.
/** Most plugins will not live long enough to 
receive the notification. It is important to unregister this notification when 
your plugin dies. If not, 3ds Max will try to notify objects that no longer exist. */
#define NOTIFY_SYSTEM_SHUTDOWN2			0x00000052 

//! \brief Sent when Animate UI mode is activated
#define NOTIFY_ANIMATE_ON				0x00000053 
//! \brief Sent when Animate UI mode is de-activated
#define NOTIFY_ANIMATE_OFF				0x00000054 

//! \brief Sent by the system when one or more custom colors have changed.
/** Plug-ins should listen to this notification if they use any of the custom colors
registered with the system. See \ref viewportDrawingColors.
If a plug-in has created a toolbar with a MaxBmpFileIcons object on it, 
it should register for this notification, and call ICustToolbar::ResetIconImages() in response to it. 
See classes ICustToolbar and MAXBmpFileIcon. */
#define NOTIFY_COLOR_CHANGE				0x00000055 
//! \brief Sent just before the current edit object is about to change
/** This notification is sent whenever the object returned by Interface::GetCurEditObject() changes. */
#define NOTIFY_PRE_EDIT_OBJ_CHANGE  	0x00000056 
//! \brief Sent just after the current edit object changes
/** This notification is sent whenever the object returned by Interface::GetCurEditObject() changes. */
#define NOTIFY_POST_EDIT_OBJ_CHANGE  	0x00000057 

//! \brief Sent when radiosity processing is started.
/** The radiosity_process notifications are designed to be broadcast by radiosity plugins (derived from
class RadiosityEffect). The broadcast must be implemented in the plugin for the notification to work. */
DEFINE_NOTIFY_CODE(NOTIFY_RADIOSITYPROCESS_STARTED, 0x00000058, TimeChangeCallback*)
//! \brief Sent when radiosity processing is stopped, but not done.
/** The radiosity_process notifications are designed to be broadcast by radiosity plugins (derived from
class RadiosityEffect). The broadcast must be implemented in the plugin for the notification to work. */
DEFINE_NOTIFY_CODE(NOTIFY_RADIOSITYPROCESS_STOPPED, 0x00000059, TimeChangeCallback*)
//! \brief Sent when radiosity processing is reset.
/** The radiosity_process notifications are designed to be broadcast by radiosity plugins (derived from
class RadiosityEffect). The broadcast must be implemented in the plugin for the notification to work. */
DEFINE_NOTIFY_CODE(NOTIFY_RADIOSITYPROCESS_RESET, 0x0000005A, TimeChangeCallback*)
//! \brief Sent when radiosity processing is complete.
/** The radiosity_process notifications are designed to be broadcast by radiosity plugins (derived from
class RadiosityEffect). The broadcast must be implemented in the plugin for the notification to work. */
DEFINE_NOTIFY_CODE(NOTIFY_RADIOSITYPROCESS_DONE, 0x0000005B, TimeChangeCallback*)

//! \brief Sent when lighting unit display system is changed
#define NOTIFY_LIGHTING_UNIT_DISPLAY_SYSTEM_CHANGE		0x0000005C 

// 10/29/01 - 1:57pm --MQM-- 
// These are helpful for any plugins needing to know when we're starting a reflect/refract map,
// or when we're starting the actual frame.

//! \brief Sent when starting to render a reflect/refract map
DEFINE_NOTIFY_CODE(NOTIFY_BEGIN_RENDERING_REFLECT_REFRACT_MAP, 0x0000005D, RenderGlobalContext*)
//! \brief Sent when starting to render the full frame.
DEFINE_NOTIFY_CODE(NOTIFY_BEGIN_RENDERING_ACTUAL_FRAME, 0x0000005E, RenderGlobalContext*)
//! \brief Sent when starting to render a tone-mapping image.
DEFINE_NOTIFY_CODE(NOTIFY_BEGIN_RENDERING_TONEMAPPING_IMAGE, 0x0000005F, RenderGlobalContext*)

//! \brief Sent when the radiosity plugin is changed (a new one is assigned)
#define NOTIFY_RADIOSITY_PLUGIN_CHANGED     0x00000060 

// [LAM - 3/13/02] Broadcast on scene undo/redo
//! \brief Sent on scene undo (NotifyInfo::callParam is a pointer to a string (MCHAR*) with the undo entry name).
DEFINE_NOTIFY_CODE(NOTIFY_SCENE_UNDO, 0x00000061, const MCHAR*)
//! \brief Sent on scene redo (NotifyInfo::callParam is a pointer to a string (MCHAR*) with the redo entry name).
DEFINE_NOTIFY_CODE(NOTIFY_SCENE_REDO, 0x00000062, const MCHAR*)
//! \brief Sent when manipulate mode ends
#define NOTIFY_MANIPULATE_MODE_OFF			0x00000063 
//! \brief Sent when manipulate mode starts
#define NOTIFY_MANIPULATE_MODE_ON			0x00000064 

// 020607  --prs.
/** \name XRef System Notifications
 These notices typically surround Merge notices */
//!@{
//! \brief Sent before an XRef scene is merged
#define NOTIFY_SCENE_XREF_PRE_MERGE			0x00000065 
//! \brief Sent after an XRef scene is successfully merged (NotifyInfo::callParam is a INode* pointer to the scene XRef tree).
DEFINE_NOTIFY_CODE(NOTIFY_SCENE_XREF_POST_MERGE, 0x00000066, INode*)
//! \brief Sent before an XRef object is merged
#define NOTIFY_OBJECT_XREF_PRE_MERGE		0x00000067 
//! \brief Sent after an XRef object is successfully merged
#define NOTIFY_OBJECT_XREF_POST_MERGE		0x00000068 
//!@}

// [J.Zhao - 6/10/02]
//! \brief Sent before a mirror operation begins.
/** NotifyInfo::callParam is a pointer to Tab<INode*> consisting 
of the nodes currently in the selection list that the mirror tool is to be applied to. */
DEFINE_NOTIFY_CODE(NOTIFY_PRE_MIRROR_NODES, 0x00000069, Tab<INode*>*)
//! \brief Sent after a mirror operation ends.
/** NotifyInfo::callParam is a pointer to Tab<INode*> consisting of 
the nodes currently in the selection list that the mirror tool is to be applied to. */
DEFINE_NOTIFY_CODE(NOTIFY_POST_MIRROR_NODES, 0x0000006A, Tab<INode*>*)

// [bayboro | 1may2002] Broadcast on node cloning
//! \brief Sent after a node is cloned but before theHold.Accept(..) (NotifyInfo::callParam is a pointer to the node).
DEFINE_NOTIFY_CODE(NOTIFY_NODE_CLONED, 0x0000006B, INode*)

// [J.Zhao - 10/4/02] The following two notifications may be broadcast
// when NotifyDependents from outside the recursion, that is, not during
// the traversal of reference targets.
// Right now, for examples, the very sources of NotifyDependents() of
//  - \ref REFMSG_MOUSE_CYCLE_STARTED
//  - \ref REFMSG_MOUSE_CYCLE_COMPLETED
// are bracketed by the notifications.

//! \brief Sent before NotifyDependents from outside the recursion
/** that is, not during the traversal of reference targets */
#define NOTIFY_PRE_NOTIFYDEPENDENTS			0x0000006C 
//! \brief Sent after NotifyDependents from outside the recursion
/** That is, not during the traversal of reference targets */
#define NOTIFY_POST_NOTIFYDEPENDENTS		0x0000006D 

//[hutchij 10/26/02]
//! \brief Sent by Mtl::RefAdded(). NotifyInfo::callParam is a Mtl pointer.
DEFINE_NOTIFY_CODE(NOTIFY_MTL_REFADDED, 0x0000006E, Mtl*)
//! \brief Sent by Mtl::FefDeleted(). NotifyInfo::callParam is a Mtl pointer.
DEFINE_NOTIFY_CODE(NOTIFY_MTL_REFDELETED, 0x0000006F, Mtl*)


//watje TIMERANGE CALLBACK
//watje time range call back for CS
//! \brief Sent after the animate time range has been changed
#define NOTIFY_TIMERANGE_CHANGE				0x00000070 

//aszabo|dec.04.02|
//! \brief Sent before a modifier is added to an object.
/** The NotifyInfo structure pointer callParam is passed a 
pointer to a NotifyModAddDelParam. */
DEFINE_NOTIFY_CODE(NOTIFY_PRE_MODIFIER_ADDED, 0x00000071, NotifyModAddDelParam*)
//! \brief Sent after a modifier is added to an object.
/** The NotifyInfo structure pointer callParam is passed a 
pointer to a NotifyModAddDelParam. */
DEFINE_NOTIFY_CODE(NOTIFY_POST_MODIFIER_ADDED, 0x00000072, NotifyModAddDelParam*)
//! \brief Sent before a modifier is deleted from an object.
/** The NotifyInfo structure pointer callParam is passed a 
pointer to a NotifyModAddDelParam. */
DEFINE_NOTIFY_CODE(NOTIFY_PRE_MODIFIER_DELETED, 0x00000073, NotifyModAddDelParam*)
//! \brief Sent after a modifier is deleted from an object.
/** The NotifyInfo structure pointer callParam is passed a 
pointer to a NotifyModAddDelParam. */
DEFINE_NOTIFY_CODE(NOTIFY_POST_MODIFIER_DELETED, 0x00000074, NotifyModAddDelParam*)

// See Node Property Change Notifications, below (codes 0x74 through 0x85)

// CA - 1/23/03
//! \brief Sent after all of the new objects for a reload have been created
/** But, before any objects have been deleted. */
#define NOTIFY_FILELINK_POST_RELOAD_PRE_PRUNE	0x00000085

// aszabo|jan.24.03|
//! \brief Sent before each set of clones is created.
/** For example, if there are N nodes cloned C times, the 
notification is sent C times. The NotifyInfo::callParam for NOTIFY_PRE_NODES_CLONED is a pointer to the array of nodes 
that will be cloned (the original nodes): INodeTab* origNodes */
DEFINE_NOTIFY_CODE(NOTIFY_PRE_NODES_CLONED, 0x00000086, const INodeTab*)
//! \brief Sent after each set of clones is created.
/** For example, if there are N nodes cloned C times, the 
notification is sent C times. The NotifyInfo::callParam for NOTIFY_POST_NODES_CLONED is a pointer to NotifyPostNodesCloned */
DEFINE_NOTIFY_CODE(NOTIFY_POST_NODES_CLONED, 0x00000087, NotifyPostNodesCloned*)

// xavier robitaille | 03.02.07
//! \brief Sent before a system path changes.
/** Originally intended to notify the tool palette if the Catalog's dir changes from the Configure Path dialog. */
DEFINE_NOTIFY_CODE(NOTIFY_SYSTEM_PRE_DIR_CHANGE, 0x00000089, int)
//! \brief Sent after a system path has changed.
/** Originally intended to notify the tool palette if the Catalog's dir changes from the Configure Path dialog. */
DEFINE_NOTIFY_CODE(NOTIFY_SYSTEM_POST_DIR_CHANGE, 0x0000008A, int)

//! \brief Schematic view notification.
/** NotifyInfo::callParam is a pointer to the index of the schematic view (int*) */
DEFINE_NOTIFY_CODE(NOTIFY_SV_SELECTIONSET_CHANGED, 0x0000008C, int*)
//! \brief Schematic view notification
/** NotifyInfo::callParam is IGraphNode* */
DEFINE_NOTIFY_CODE(NOTIFY_SV_DOUBLECLICK_GRAPHNODE, 0x0000008D, IGraphNode*)

//! \brief Sent before the renderer changes
/** NotifyInfo::callParam is the RenderSettingID enum value */
DEFINE_NOTIFY_CODE(NOTIFY_PRE_RENDERER_CHANGE, 0x0000008E, RenderSettingID)
//! \brief Sent after the renderer changes
/** NotifyInfo::callParam is the RenderSettingID enum value */
DEFINE_NOTIFY_CODE(NOTIFY_POST_RENDERER_CHANGE, 0x0000008F, RenderSettingID)

//! \brief Sent before a schematic view layout change is made.
/** NotifyInfo::callParam is a pointer to the index of the schematic view (int*) */
DEFINE_NOTIFY_CODE(NOTIFY_SV_PRE_LAYOUT_CHANGE, 0x00000090, int*)
//! \brief Sent after a schematic view layout change is made.
/** NotifyInfo::callParam is a pointer to the index of the schematic view (int*) */
DEFINE_NOTIFY_CODE(NOTIFY_SV_POST_LAYOUT_CHANGE, 0x00000091, int*)

//! \brief Sent AFTER object categories were marked to be hidden/unhidden.
/** Clients registered for this notification can retrieve the categories whose hidden 
state have changed by retrieving the category flags by calling DWORD Interface::GetHideByCategoryFlags() */
#define NOTIFY_BY_CATEGORY_DISPLAY_FILTER_CHANGED	0x00000092 

//! \brief Sent AFTER custom display filters have been activated/deactivated
/** Results in changes to some objects hidden state. Clients registered for this 
notification can retrieve the active custom display filters by checking their On/Off 
state using BOOL Interface::GetDisplayFilter(int index) */
#define NOTIFY_CUSTOM_DISPLAY_FILTER_CHANGED	0x00000093 

//! \brief Sent after a layer is added to layer manager
/** NotifyInfo::callParam is a pointer to ILayer. */
DEFINE_NOTIFY_CODE(NOTIFY_LAYER_CREATED, 0x00000094, ILayer*)
//! \brief Sent before layer is removed from layer manager; NotifyInfo::callParam is a pointer to ILayer.
DEFINE_NOTIFY_CODE(NOTIFY_LAYER_DELETED, 0x00000095, ILayer*)
//! \brief NotifyInfo::callParam is a pointer to LayerChange
/** newLayer and oldLayer can be NULL when switching between layers, during create,
and when loading files Layers may not be present in layer manager when sent during file load/merge  */
DEFINE_NOTIFY_CODE(NOTIFY_NODE_LAYER_CHANGED, 0x00000096, LayerChange*)
 
//! \brief Sent when a tabbed dialog is created.
/** NotifyInfo::callParam is point to the dialogID (Class_ID). */
DEFINE_NOTIFY_CODE(NOTIFY_TABBED_DIALOG_CREATED, 0x00000097, const Class_ID*)
//! \brief Sent when a tabbed dialog is deleted.
/** NotifyInfo::callParam is a pointer to the dialogID (Class_ID). */
DEFINE_NOTIFY_CODE(NOTIFY_TABBED_DIALOG_DELETED, 0x00000098, const Class_ID*)

//! \brief Sent by BaseNode::SetName.
/** NotifyInfo::callParam is pointer to NodeNameSetParam */
DEFINE_NOTIFY_CODE(NOTIFY_NODE_NAME_SET, 0x00000099, NodeNameSetParam*)

//! \brief Sent by the Material Editor when the "use texture in hardware shader" button is pressed.
/* This allows the standard material to force a rebuild of the hardware shader.  
Param is a pointer to the material being effected. */
DEFINE_NOTIFY_CODE(NOTIFY_HW_TEXTURE_CHANGED, 0x0000009A, MtlBase*)

//! \brief Sent by MAXScript during its initialization
/** Occurs immediately before it scans the registered plugin classes and 
wraps them in MAXClass values any runtime defined classes created in this callback will be detected by MXS any 
core interfaces installed in this callback will be detected by MXS */
#define NOTIFY_MXS_STARTUP					0x0000009B 

//! \brief Sent by MAXScript when it has completed its initialization 
#define NOTIFY_MXS_POST_STARTUP				0x0000009C 

//! \brief Sent before an action item is executed via a keyboard shortcut (hotkey). NotifyInfo::callParam is ActionItem*
DEFINE_NOTIFY_CODE(NOTIFY_ACTION_ITEM_HOTKEY_PRE_EXEC, 0x0000009D, ActionItem*)

//! \brief Sent after an action item is executed via a keyboard shortcut (hotkey). NotifyInfo::callParam is ActionItem*
DEFINE_NOTIFY_CODE(NOTIFY_ACTION_ITEM_HOTKEY_POST_EXEC, 0x0000009E, ActionItem*)

// CCJ - 12.7.2004
/** \name Scene State Notifications
All the SceneState notifications have NotifyInfo::callParam as a const MCHAR* with the SceneState name except 
for \ref NOTIFY_SCENESTATE_RENAME */
//!@{
//! \brief Sent before a Scene State is saved.
DEFINE_NOTIFY_CODE(NOTIFY_SCENESTATE_PRE_SAVE, 0x0000009F, const MCHAR*)
//! \brief Sent after a Scene State is saved.
DEFINE_NOTIFY_CODE(NOTIFY_SCENESTATE_POST_SAVE, 0x000000A0, const MCHAR*)
//! \brief Sent before a Scene State is restored.
DEFINE_NOTIFY_CODE(NOTIFY_SCENESTATE_PRE_RESTORE, 0x000000A1, const MCHAR*)
//! \brief Sent after a Scene State is restored.
DEFINE_NOTIFY_CODE(NOTIFY_SCENESTATE_POST_RESTORE, 0x000000A2, const MCHAR*)
//! \brief Sent after a Scene State is deleted.
DEFINE_NOTIFY_CODE(NOTIFY_SCENESTATE_DELETE, 0x000000A3, const MCHAR*)
//! \ingroup NameChangeNotifications
//! \brief Sent after a Scene State is renamed.
/** NotifyInfo::callParam is pointer to NameChange */
DEFINE_NOTIFY_CODE(NOTIFY_SCENESTATE_RENAME, 0x000000A4, NameChange*)
//!@}

// NH 20-Dec-04
/** \name Undo/Redo Notifications */
//!@{
//! \brief Sent before an Undo starts.
DEFINE_NOTIFY_CODE(NOTIFY_SCENE_PRE_UNDO, 0x000000A5, const MCHAR*)
//! \brief Sent before a Redo starts.
/** Allows a developer to get an accurate picture of what the undo/redo is doing, 
and not having to rely on calling RestoreOrRedoing().  This solves a problem of evaluating a node during an 
undo when the hold system was active, but wasn't initiated from either script or the UI.  An example is right-
clicking to cancel object creation */
DEFINE_NOTIFY_CODE(NOTIFY_SCENE_PRE_REDO, 0x000000A6, const MCHAR*)
//! \brief Sent to make the previous undo notification more explicit, and match the PRE_UNDO.
#define NOTIFY_SCENE_POST_UNDO NOTIFY_SCENE_UNDO 
//! \brief Sent to make the previous undo notification more explicit, and match the PRE_UNDO.
#define NOTIFY_SCENE_POST_REDO NOTIFY_SCENE_REDO 
//!@}

// Added LAM 19-Jan-05
//! \brief Sent when MAXScript has been shut down.
/** No access to any MXS value should be made after this notification, including calls to event handlers */
#define NOTIFY_MXS_SHUTDOWN					0x000000A7 

//Added NH 21-Jan-05
//! \brief This is sent by the D3D GFX (Dx9) when a device reset is going to happen.
/** This can allow Hardware shader to release their resources allocated in the DEFAULT_POOL */
#define NOTIFY_D3D_PRE_DEVICE_RESET			0x000000A8 
//! \brief This is sent by the D3D GFX (Dx9) when a device reset has happened.
/** This can allow Hardware shader to release their resources allocated in the DEFAULT_POOL */
#define NOTIFY_D3D_POST_DEVICE_RESET		0x000000A9 

// JH 2-18-05
//! \brief Used to suspend material change tracking in VIZ
/** In VIZ, the tool palette system listens for additions and removals to the scene material 
library and tracks reference additions and removals to existing materials. Upon resumption, 
the scene material lib will be traversed and changes reflected in the palette. */
#define NOTIFY_TOOLPALETTE_MTL_SUSPEND		0x000000AA

//! \brief Used to resume material change tracking in VIZ
/** In VIZ, the tool palette system listens for additions and removals to the scene material 
library and tracks reference additions and removals to existing materials. Upon resumption, 
the scene material lib will be traversed and changes reflected in the palette. */
#define NOTIFY_TOOLPALETTE_MTL_RESUME		0x000000AB

//! \brief Provides a notification that a ClassDesc is being replaced by another one.
/** This occurs when the dll containing a deferred loaded plugin is loaded. 
NotifyInfo::callParam is pointer to ClassDescReplaced. */
DEFINE_NOTIFY_CODE(NOTIFY_CLASSDESC_REPLACED, 0x000000AC, ClassDescReplaced*)

/** \name File I/O Notifications 
All contain NotifyInfo::callParam that points to a NotifyFileProcessParam.
See \ref notify_file_process_type */
//!@{
DEFINE_NOTIFY_CODE(NOTIFY_FILE_PRE_OPEN_PROCESS, 0x000000AD, NotifyFileProcessParam*)
DEFINE_NOTIFY_CODE(NOTIFY_FILE_POST_OPEN_PROCESS, 0x0000008B, NotifyFileProcessParam*)
DEFINE_NOTIFY_CODE(NOTIFY_FILE_PRE_SAVE_PROCESS, 0x000000AE, NotifyFileProcessParam*)
DEFINE_NOTIFY_CODE(NOTIFY_FILE_POST_SAVE_PROCESS, 0x000000AF, NotifyFileProcessParam*)
//!@}

//! \brief Sent after a ClassDesc was successfully loaded from a plugin dll. 
/*!	\remarks For each non NULL class descriptor returned by a plugin's ClassDesc* LibClassDesc(int i)
	method, the system broadcasts this notification. The call parameter will be a pointer to the ClassDesc. 
	\see ~{ Required DLL Functions  }~ */
DEFINE_NOTIFY_CODE(NOTIFY_CLASSDESC_LOADED, 0x000000B0, ClassDesc*)

/** \brief This provides a notification that a toolbar configuration is being loaded, prior to load.
    NotifyInfo::callParam is a pointer to a string (MCHAR*) and is NULL if the default file is being loaded, or the full path to the file being loaded
*/
DEFINE_NOTIFY_CODE(NOTIFY_TOOLBARS_PRE_LOAD, 0x000000B1, const MCHAR*)
/** \brief This provides a notification that a toolbar configuration is being loaded, after the load.
    NotifyInfo::callParam is a pointer to a string (MCHAR*) and is NULL if the default file is being loaded, or the full path to the file being loaded
*/
DEFINE_NOTIFY_CODE(NOTIFY_TOOLBARS_POST_LOAD, 0x000000B2, const MCHAR*)

/** \name Asset Tracking System Notifications
These notification bracket ATS traversal of the entire scene (or ReferenceTarget hierarchy, 
in the case of a general retarget action) and repath any assets that should be repathed.  
No parameters are sent with these notifications. */
//!@{
//! \brief Sent before the ATS traversal starts
#define NOTIFY_ATS_PRE_REPATH_PHASE				0x000000B3
//! \brief Sent after the ATS traversal completes
#define NOTIFY_ATS_POST_REPATH_PHASE			0x000000B4
//!@}

/*!	\name Bitmap Proxy System Notifications
Any specific operation during which proxies should be disable can be surrounded 
by these notifications. This method of disabling proxies is different from actually 
disabling the bitmap proxies through the proxy manager; these notifications do not 
actually result in re-loading of any bitmaps. Rather, the Bitmap Texture will only 
load the high-res image when it is asked to, which makes the process efficient.
\note One should ideally NOT broadcast these notifications. Instead, use class 
BitmapProxyTemporaryDisableGuard; it is safer as it handles multiple nested disables. */
//!@{
#define NOTIFY_PROXY_TEMPORARY_DISABLE_START	0x000000B5
#define NOTIFY_PROXY_TEMPORARY_DISABLE_END		0x000000B6
//!@}

/** Allows a plugin to query the system for the status of the file before max saves the max scene file. */
/** NotifyInfo::callParam is a pointer to a NotifyFileCheckStatus. Plugins that
register for this notification can add to the iStatus member as needed, but should not clear any bits
already in iStatus. Valid status masks for iStatus are defined next to the definition of NotifyFileCheckStatus.
This is only sent when a scene is saved normally, not for a hold/fetch nor for an autosave. */
DEFINE_NOTIFY_CODE(NOTIFY_FILE_CHECK_STATUS, 0x000000B7, NotifyFileCheckStatus*)

//! \name Named Selection Sets Notifications.
//! \brief The following notifications will be broadcast whenever any operation has been
//!		   done to a named selection set.
//! \note These notifications only apply to named selection sets of objects but not sub-object ones.
//!@{
//! \brief Sent when a Named selection set is created either in the UI, or via maxscript.
/*! NotifyInfo::callParam is a pointer to a string (const MCHAR*) of the name of the created named set.
*/
DEFINE_NOTIFY_CODE(NOTIFY_NAMED_SEL_SET_CREATED, 0x000000B8, const MCHAR*)
//! \brief Sent when a Named selection set is deleted either in the UI, or via maxscript.
/*! NotifyInfo::callParam is a pointer to a string (const MCHAR*) of the name of the deleted named set.
*/
DEFINE_NOTIFY_CODE(NOTIFY_NAMED_SEL_SET_DELETED, 0x000000B9, const MCHAR*)
//! \ingroup NameChangeNotifications
//! \brief Sent when a Named selection set name is changed, either in the old UI, or via maxscript.
/*! NotifyInfo::callParam is a pointer to the struct NameChange of the changed named set.
*/
DEFINE_NOTIFY_CODE(NOTIFY_NAMED_SEL_SET_RENAMED, 0x000000BC, NameChange*)
//! \brief The following notification will be sent when the node set of a named selection set begins 
//!		   to be changed. For e.g., add/remove nodes to/from a named selection set,or completely
//!		   replace the node set of a named selection set.
/*! NotifyInfo::callParam is a pointer to a string (const MCHAR*) of the name of the modified named set.
	\note When modifying a specified named selection set, it will first be deleted then created with
		  the same name when deleted. So both NOTIFY_NAMED_SEL_SET_CREATED and NOTIFY_NAMED_SEL_SET_DELETED
		  will be broadcast during this process. In order to stress that this is a modification to an 
		  existing named selection set rather than two irrelevant operations as deleting one set then 
		  creating another set, we send two additional notifications as NOTIFY_NAMED_SEL_SET_PRE_MODIFY
		  and NOTIFY_NAMED_SEL_SET_POST_MODIFY in the very beginning and ending of a modification
		  to a named set. For users who needs to be informed of a modification of a named set, they should
		  register to these two notifications and ignore only NOTIFY_NAMED_SEL_SET_DELETED and 
		  NOTIFY_NAMED_SEL_SET_CREATED(even this applies to the current state of the code) that is received 
		  between the pair of NOTIFY_NAMED_SEL_SET_PRE_MODIFY and NOTIFY_NAMED_SEL_SET_POST_MODIFY.
*/
DEFINE_NOTIFY_CODE(NOTIFY_NAMED_SEL_SET_PRE_MODIFY, 0x000000CA, const MCHAR*)
//! \brief The following notification will be sent when the node set of a named selection set has been changed.
DEFINE_NOTIFY_CODE(NOTIFY_NAMED_SEL_SET_POST_MODIFY, 0x000000CB, const MCHAR*)
//!@}

//! \brief Sent when the sub-object level changes in the modify panel
DEFINE_NOTIFY_CODE(NOTIFY_MODPANEL_SUBOBJECTLEVEL_CHANGED, 0x000000BA, NumberChange*)

// NH 13-May-2006

//! \brief Sent when a bitmap fails to load in the DirectX Shader Material
/** This is usually when DirectX does not support the file format. The developer 
can register for this notification so that they can convert the the format into a 
DirectX texture resource. The developer is responsible for maintaining this resource, 
and can register for NOTIFY_D3D_POST_DEVICE_RESET to release and rebuild the resource on Lost device 
situations The callParam is a NotifyFailedDxMatTexLoad.
From the material the developer can access the IEffectParser and use LoadTexture using the paramName.  The 
forceUpdate will be set if this was an auto update of the bitmap from disk, or the user hit the reload button 
from the UI.  The developer will need to release and rebuild the bitmap under these situations. */
DEFINE_NOTIFY_CODE(NOTIFY_FAILED_DIRECTX_MATERIAL_TEXTURE_LOAD, 0x000000BB, NotifyFailedDxMatTexLoad*)

//! \brief Sent just after deleting all refs in existing scene.
/** This notification is sent after wiping the existing scene. Examples of when this occurs is 
immediately before loading in the new scene from a file, and when doing a file new */
#define NOTIFY_POST_SCENE_RESET							0x000000BD

//! \brief Sent just after animation layers are enabled on some nodes in the scene.
/** This notification is sent after layers are enabled, and new layer controller constructed.
NotifyInfo::callParam is a pointer to the nodes which are having animation layers enabled. */
DEFINE_NOTIFY_CODE(NOTIFY_ANIM_LAYERS_ENABLED, 0x000000BE, Tab<INode*>*)

//! \brief Sent just after animation layers are disabled on some nodes in the scene.
/** This notification is sent after layers are disabled on some nodes, and the layer ends up deleted.
NotifyInfo::callParam is a pointer to the nodes which are having animation layers disabled. */
DEFINE_NOTIFY_CODE(NOTIFY_ANIM_LAYERS_DISABLED, 0x000000BF, Tab<INode*>*)

//! \brief Sent just before an action item is overridden and IActionItemOverride::StartOveride is called.
/** NotifyInfo::callParam is a pointer to the ActionItem. */
DEFINE_NOTIFY_CODE(NOTIFY_ACTION_ITEM_PRE_START_OVERRIDE, 0x000000C0, ActionItem*)

//! \brief Sent just after an action item is overridden and after IActionItemOverride::StartOveride is called.
/*! NotifyInfo::callParam is a pointer to the ActionItem. */
DEFINE_NOTIFY_CODE(NOTIFY_ACTION_ITEM_POST_START_OVERRIDE, 0x000000C1, ActionItem*)

//! \brief Sent just before an action item finishes it's override and IActionItemOverride::EndOverride is called.
/** NotifyInfo::callParam is a pointer to the ActionItem. */
DEFINE_NOTIFY_CODE(NOTIFY_ACTION_ITEM_PRE_END_OVERRIDE, 0x000000C2, ActionItem*)

//! \brief Sent just after an action item finishes it's override and after IActionItemOverride::EndOverride is called. */
/** NotifyInfo::callParam is a pointer to the ActionItem. */
DEFINE_NOTIFY_CODE(NOTIFY_ACTION_ITEM_POST_END_OVERRIDE, 0x000000C3, ActionItem*)

// aszabo|dec.11.02|

/** \name Node Property Change Notifications
NotifyInfo::callParam is a pointer to the list of nodes (INodeTab*) that is about to change or has changed. */
//!@{
DEFINE_NOTIFY_CODE(NOTIFY_PRE_NODE_GENERAL_PROP_CHANGED,	0x00000075, INodeTab*) //!< Sent before an INode's general property is changed.
DEFINE_NOTIFY_CODE(NOTIFY_POST_NODE_GENERAL_PROP_CHANGED,	0x00000076, INodeTab*) //!< Sent After an INode's general property is changed.
DEFINE_NOTIFY_CODE(NOTIFY_PRE_NODE_GI_PROP_CHANGED,			0x00000077, INodeTab*) //!< Sent before an INode's global illumination property is changed.
DEFINE_NOTIFY_CODE(NOTIFY_POST_NODE_GI_PROP_CHANGED,		0x00000078, INodeTab*) //!< Sent after an INode's global illumination property is changed.
DEFINE_NOTIFY_CODE(NOTIFY_PRE_NODE_MENTALRAY_PROP_CHANGED,	0x00000079, INodeTab*) //!< Sent before an INode's mental ray property is changed.
DEFINE_NOTIFY_CODE(NOTIFY_POST_NODE_MENTALRAY_PROP_CHANGED,	0x00000080, INodeTab*) //!< Sent after an INode's mental ray illumination property is changed.
//! \brief Sent before an INode's bone property is changed.
/**  Note that bone properties can only be changed through a function published interface. Thus it is called on a per node
	basis, and not on a collection of INodes. Therefore while the callParam for the notification may return an INodeTab 
	for legacy compatibility, the INodeTab will always only contain one object, not multiple objects. 
	Note: This event will not broadcast for transform matrix changes. */
DEFINE_NOTIFY_CODE(NOTIFY_PRE_NODE_BONE_PROP_CHANGED, 0x00000081, INodeTab*)
//! \brief Sent after an INode's bone property is changed.
/**  Note that bone properties can only be changed through a function published interface. Thus it is called on a per node
	basis, and not on a collection of INodes. Therefore while the callParam for the notification may return an INodeTab 
	for legacy compatibility, the INodeTab will always only contain one object, not multiple objects. 
	Note: This event will not broadcast for transform matrix changes. */
DEFINE_NOTIFY_CODE(NOTIFY_POST_NODE_BONE_PROP_CHANGED, 0x00000082, INodeTab*)
DEFINE_NOTIFY_CODE(NOTIFY_PRE_NODE_USER_PROP_CHANGED,  0x00000083, INodeTab*) //!< Sent before an INode's user property is changed.
DEFINE_NOTIFY_CODE(NOTIFY_POST_NODE_USER_PROP_CHANGED, 0x00000084, INodeTab*) //!< Sent after an INode's user property is changed.

DEFINE_NOTIFY_CODE(NOTIFY_PRE_NODE_RENDER_PROP_CHANGED,		0x000000C4, INodeTab*) //!< Sent before an INode's render property is changed.
DEFINE_NOTIFY_CODE(NOTIFY_POST_NODE_RENDER_PROP_CHANGED,	0x000000C5, INodeTab*) //!< Sent after an INode's render property is changed.
DEFINE_NOTIFY_CODE(NOTIFY_PRE_NODE_DISPLAY_PROP_CHANGED,	0x000000C6, INodeTab*) //!< Sent before an INode's display property is changed.
DEFINE_NOTIFY_CODE(NOTIFY_POST_NODE_DISPLAY_PROP_CHANGED,	0x000000C7, INodeTab*) //!< Sent after an INode's display property is changed.
DEFINE_NOTIFY_CODE(NOTIFY_PRE_NODE_BASIC_PROP_CHANGED,		0x000000C8, INodeTab*) //!< Sent before an INode's basic property is changed.
DEFINE_NOTIFY_CODE(NOTIFY_POST_NODE_BASIC_PROP_CHANGED,		0x000000C9, INodeTab*) //!< Sent after an INode's basic property is changed.

// Note that 0x000000CA - 0x000000CF used above
//!@}

//! \brief Sent when selection lock is triggered. */
/** NotifyInfo::callParam is NULL. */
#define NOTIFY_SELECTION_LOCK					0x000000D0
//! \brief Sent when selection unlock is triggered. */
/** NotifyInfo::callParam is NULL. */
#define NOTIFY_SELECTION_UNLOCK					0x000000D1

//! \brief Sent when an image viewer (including the rendered frame window) is opened, before it becomes visible. */
/** NotifyInfo::callParam is an FPValue, whose fpi member (of type FPInterface*) points to an interface to the VFB.
	The interface is intended for use only in MaxScript.  Note the interface object will be destroyed when the VFB is closed.
	The interface object may be used to add rollouts to the window in MaxScript. */
DEFINE_NOTIFY_CODE(NOTIFY_PRE_IMAGE_VIEWER_DISPLAY, 0x000000D2, FPValue*)
//! \brief Sent when an image viewer (including the rendered frame window) is opened, after it becomes visible. */
/** NotifyInfo::callParam is an FPValue, whose fpi member (of type FPInterface*) points to an interface to the VFB.
	The interface is intended for use only in MaxScript.  Note the interface object will be destroyed when the VFB is closed. */
DEFINE_NOTIFY_CODE(NOTIFY_POST_IMAGE_VIEWER_DISPLAY, 0x000000D3, FPValue*)
//! \brief Sent by an image viewer (or the rendered frame window) to request that its custom rollouts update their UI. */
/** NotifyInfo::callParam is an FPValue, whose fpi member (of type FPInterface*) points to an interface to the VFB.
	The interface is intended for use only in MaxScript.  Note the interface object will be destroyed when the VFB is closed. */
DEFINE_NOTIFY_CODE(NOTIFY_IMAGE_VIEWER_UPDATE, 0x000000D4, FPValue*)

/** \name Custom Attributes Notifications
These notifications are sent after a custom attribute is added or removed
from a custom attribute container.  
NotifyInfo::callParam is a pointer to a NotifyCustomAttribute structure */
//!@{
//! \brief Sent after a custom attribute is added to an Animatable
DEFINE_NOTIFY_CODE(NOTIFY_CUSTOM_ATTRIBUTES_ADDED, 0x000000D5, NotifyCustomAttribute*)
//! \brief Sent after a custom attribute is removed from an Animatable
DEFINE_NOTIFY_CODE(NOTIFY_CUSTOM_ATTRIBUTES_REMOVED, 0x000000D6, NotifyCustomAttribute*)
//!@}

//! \brief Sent after OS theme has been changed
#define NOTIFY_OS_THEME_CHANGED					0x000000D7

//! \brief Sent when the current active viewport is changed.  
/*! Typically this notification is sent when the user has switched viewports. 
 * To get notifications of any other type of viewport change, please use NOTIFY_VIEWPORT_CHANGE */
#define NOTIFY_ACTIVE_VIEWPORT_CHANGED			0x000000D8

//! \brief NOTIFY_PRE_MAXMAINWINDOW_SHOW is being sent when main window of 3ds Max is about to show.
#define NOTIFY_PRE_MAXMAINWINDOW_SHOW			0x000000DB

//! \brief NOTIFY_POST_MAXMAINWINDOW_SHOW is being sent immediately after the main window of 3ds Max is shown.
#define NOTIFY_POST_MAXMAINWINDOW_SHOW			0x000000DC

//! \brief Provides a notification that a new ClassDesc is being registered.
/** This occurs when dlls containing plugins are loaded, and when a ClassDesc is dynamically created and registered 
with 3ds Max (for example, when a scripted plugin is defined). When loading a dll containing plugins, for each plugin
you will get a NOTIFY_CLASSDESC_LOADED notification followed by a NOTIFY_CLASSDESC_ADDED notification. When a 
ClassDesc is dynamically created and registered with 3ds Max, you will get just a NOTIFY_CLASSDESC_ADDED notification.
CallParam is pointer to ClassDesc */
DEFINE_NOTIFY_CODE(NOTIFY_CLASSDESC_ADDED, 0x000000DD, ClassDesc*)

//! \brief NOTIFY_OBJECT_DEFINITION_CHANGE_BEGIN is sent immediately before object instances are updated to a new object definition
/*! MAXScript allows scripted plugin and scripted custom attribute definitions to be updated by re-evaluating 
the definition script. The new definition can add or remove local variables, parameter blocks, parameter block
items, rollouts, and rollout controls. After the new definition is evaluated, existing plugin instances are 
converted to this new definition. This notification is sent immediately before the instances are converted to 
the new definition.
The NotifyInfo structure pointer callParam is passed a pointer to a ObjectDefinitionChangeNotificationParam struct instance. 
Currently the only valid type value is kScriptedPlugin, which signifies a scripted plugin/custom attribute class description, 
and the definition is a pointer to ClassDesc2
A plugin may use this notification to, for example, control the rebuilding of rollout displays for instances of the plugins using
the definition. 
\sa REFMSG_OBJECT_DEFINITION_CHANGE_BEGIN, ObjectDefinitionChangeNotificationParam
*/
DEFINE_NOTIFY_CODE(NOTIFY_OBJECT_DEFINITION_CHANGE_BEGIN, 0x000000DE, ObjectDefinitionChangeNotificationParam*)

//! \brief NOTIFY_OBJECT_DEFINITION_CHANGE_END is sent immediately after object instances are updated to a new object definition
/*! MAXScript allows scripted plugin and scripted custom attribute definitions to be updated by re-evaluating 
the definition script. The new definition can add or remove local variables, parameter blocks, parameter block
items, rollouts, and rollout controls. After the new definition is evaluated, existing plugin instances are 
converted to this new definition. This notification is sent immediately after the instances are converted to 
the new definition.
The NotifyInfo structure pointer callParam is passed a pointer to a ObjectDefinitionChangeNotificationParam struct instance. 
Currently the only valid type value is kScriptedPlugin, which signifies a scripted plugin/custom attribute class description, 
and the definition is a pointer to ClassDesc2
A plugin may use this notification to, for example, control the rebuilding of rollout displays for instances of the plugins using
the definition.
\sa REFMSG_OBJECT_DEFINITION_CHANGE_END, ObjectDefinitionChangeNotificationParam
*/
DEFINE_NOTIFY_CODE(NOTIFY_OBJECT_DEFINITION_CHANGE_END, 0x000000DF, ObjectDefinitionChangeNotificationParam*)


//! \brief Sent when a MtlBase is about to show its UI with Associated parameter rollouts when being edited in a material editor. 
/*! NotifyInfo::callParam is a pointer to the MtlBase that is about to show its UI. */
DEFINE_NOTIFY_CODE(NOTIFY_MTLBASE_PARAMDLG_PRE_OPEN, 0x000000E0, MtlBase*)

//! \brief Sent when a MtlBase has finished its editing in the material editor and the UI with Associated parameter rollouts have been closed.
/*! NotifyInfo::callParam is a pointer to the MtlBase that has closed its UI. */
DEFINE_NOTIFY_CODE(NOTIFY_MTLBASE_PARAMDLG_POST_CLOSE, 0x000000E1, MtlBase*)

//! \brief Sent before the application theme is changed via IColorManager::SetAppFrameColorTheme().
/*!  NotifyInfo::callParam is NULL. */
#define NOTIFY_PRE_APP_FRAME_THEME_CHANGED				0x000000E2

//! \brief Sent when the application theme is changed via IColorManager::SetAppFrameColorTheme().
/*!  NotifyInfo::callParam is NULL. */
#define NOTIFY_APP_FRAME_THEME_CHANGED				0x000000E3


//! \brief Sent before a viewport is deleted. It usually occurs when removing a view tab.
/*! NotifyInfo::callParam  is the HWND of the viewport to be deleted. */
DEFINE_NOTIFY_CODE(NOTIFY_PRE_VIEWPORT_DELETE, 0x000000E4, HWND)

/** Sent before a CUI workspace is about to change 
	NotifyInfo::callParam is a pointer to a string (MCHAR*), containing the name of the workspace the CUI is changing to.
*/
DEFINE_NOTIFY_CODE(NOTIFY_PRE_WORKSPACE_CHANGE, 0x000000E5, const MCHAR*)

/** Sent after a CUI workspace has changed
	NotifyInfo::callParam is a pointer to a string (MCHAR*), containing the name of the workspace the CUI has changed to.
*/
DEFINE_NOTIFY_CODE(NOTIFY_POST_WORKSPACE_CHANGE, 0x000000E6, const MCHAR*)

/** Sent before the collection of system workspaces is about to change.
*/
#define NOTIFY_PRE_WORKSPACE_COLLECTION_CHANGE 0x000000E7

/** Sent after the collection of system workspaces has changed. 
*/
#define NOTIFY_POST_WORKSPACE_COLLECTION_CHANGE 0x000000E8

/** Sent after keyboard short-cut setting file(*.kbd) has changed. For example, loading/saving a new keyboard setting file.
*/
#define NOTIFY_KEYBOARD_SETTING_CHANGED 0x000000E9

/** Sent after mouse setting file has changed. For example, loading/saving a new mouse setting file.
*/
#define NOTIFY_MOUSE_SETTING_CHANGED 0x000000EA

/** \brief This provides a notification that a toolbar configuration is being saved, prior to save.
	NotifyInfo::callParam is a pointer to a string (MCHAR*), which is NULL for the default save file, or the full path to the file being saved.
*/
DEFINE_NOTIFY_CODE(NOTIFY_TOOLBARS_PRE_SAVE, 0x000000EB, const MCHAR*)
/**  \brief This provides a notification that a toolbar configuration is being saved, after the save.
	 NotifyInfo::callParam is a pointer to a string (MCHAR*), which is NULL for the default save file, or the full path to the file being saved.
*/
DEFINE_NOTIFY_CODE(NOTIFY_TOOLBARS_POST_SAVE, 0x000000EC, const MCHAR*)

//!  \brief Called when the main application window is activated.
/*! Note this message is called every time the main window is activated.  Use with caution.
 * If the intention is to do something on system startup, consider NOTIFY_SYSTEM_STARTUP instead.
 * This message is likely called before NOTIFY_SYSTEM_STARTUP, but it is not guaranteed that the
 * application is in a ready/idle state at that point.
*/
#define NOTIFY_APP_ACTIVATED					0x000000ED

//!  \brief Called when the main application window is deactivated.
/*!  Note this message is called every time the main window is deactivated.  Use with caution.
 *	 If the intention is to do something on system shutdown, consider NOTIFY_SYSTEM_SHUTDOWN instead.
*/
#define NOTIFY_APP_DEACTIVATED					0x000000EE

//!  \brief Called after the safe frame has been toggled on/off for the viewport
/*!  
 *   \note This sets a dirty flag on the view and some values on the ViewExp class
 *    might take a message pump to make sense with this update.
 */
DEFINE_NOTIFY_CODE(NOTIFY_VIEWPORT_SAFEFRAME_TOGGLE, 0x000000F2, BOOL*)

//! \brief Sent during shutdown immediately before DllDesc::CallShutdown is called on all plugin dlls
#define NOTIFY_PLUGINS_PRE_SHUTDOWN				0x000000F3

//! \brief Sent during shutdown immediately before DllDesc::Unload is called on all plugin dlls
#define NOTIFY_PLUGINS_PRE_UNLOAD				0x000000F4

//!  \brief Called after a menu file has been loaded and the menu bar updated
/*!  NotifyInfo::callParam is a pointer to a string (MCHAR*), which is the full path to the file that was loaded.
 */
DEFINE_NOTIFY_CODE(NOTIFY_CUI_MENUS_POST_LOAD, 0x000000F5, const MCHAR*)

//! \brief Sent after the parent of a layer was changed.
/*! NotifyInfo::callParam is a pointer on the struct LayerParentChange.
 *  layerChanged is the pointer on the layer that has changed parent. oldParent is the pointer on the old parent of the layer.
 *  The new parent can easily be obtained by calling layerChanged.GetParentLayer().
 *  The new parent can be NULL, which mean that the layer is at the top level.
 */
DEFINE_NOTIFY_CODE(NOTIFY_LAYER_PARENT_CHANGED, 0x000000F6, LayerParentChange*)

//! \brief Sent when an action item starts executing
/*! This notification is sent without regard of how the action item was invoked (hot-key, menu item, toolbar button, etc).
NotifyInfo::callParam is a pointer to the ActionItem instance.
 */
DEFINE_NOTIFY_CODE(NOTIFY_ACTION_ITEM_EXECUTION_STARTED, 0x000000F7, ActionItem*)

//! \brief Sent when an action item finished executing
/*! This notification is sent without regard of how the action item was invoked (hot-key, menu item, toolbar button, etc).
NotifyInfo::callParam is a pointer to the ActionItem instance.
*/
DEFINE_NOTIFY_CODE(NOTIFY_ACTION_ITEM_EXECUTION_ENDED, 0x000000F8, ActionItem*)

//! \brief Sent when the user starts creating a plug-in instance.
/*! Examples of when this notification is sent include interactive creation of objects 
via the Create and Modify panels or the main menu.
This notification is not sent when plug-in instances are created programmatically (SDK, Maxscript, python etc).
NotifyInfo::callParam is a pointer to the ClassDesc instance describing the plug-in whose instance is created.
*/
DEFINE_NOTIFY_CODE(NOTIFY_INTERACTIVE_PLUGIN_INSTANCE_CREATION_STARTED, 0x000000F9, ClassDesc*)

//! \brief Sent when the user ends creating a plug-in instance.
/*! This notification is not sent when plug-in instances are created programmatically (SDK, Maxscript, python etc).
NotifyInfo::callParam is a pointer to the ClassDesc instance describing the plug-in whose instance has been created.
See NOTIFY_INTERACTIVE_PLUGIN_INSTANCE_CREATION_STARTED.
*/
DEFINE_NOTIFY_CODE(NOTIFY_INTERACTIVE_PLUGIN_INSTANCE_CREATION_ENDED, 0x000000FA, ClassDesc*)

//! \brief Sent after the system Node Selection Processor has finished selecting nodes
#define NOTIFY_POST_NODE_SELECT_OPERATION	0x000000FC

//! \brief Sent before viewport tooltip pops. NotifyInfo::callParam is a INode* to the tip node.
DEFINE_NOTIFY_CODE(NOTIFY_PRE_VIEWPORT_TOOLTIP, 0x000000FD, INode*)

//! \brief Sent once initial welcome screen is done (either it did not show or it has been closed).
#define NOTIFY_WELCOMESCREEN_DONE			0x000000FE

#define NOTIFY_PLAYBACK_START				0x000000FF

#define NOTIFY_PLAYBACK_END					0x00000100

//! \brief Sent if the scene explorer needs to refresh its display
#define NOTIFY_SCENE_EXPLORER_NEEDS_UPDATE	0x00000101

//! \brief Sent at the very end of the load sequence, note that any animatables created by the scene load may have been deleted by this point.
#define NOTIFY_FILE_POST_OPEN_PROCESS_FINALIZED 0x00000102

//! \brief Sent at the very end of the merge sequence, note that any animatables created by the scene merge may have been deleted by this point.
#define NOTIFY_FILE_POST_MERGE_PROCESS_FINALIZED 0x00000103

//! \brief Sent before project folder is changed.
#define NOTIFY_PRE_PROJECT_FOLDER_CHANGE 0x00000104

//! \brief Sent after project folder is changed.
#define NOTIFY_POST_PROJECT_FOLDER_CHANGE 0x00000105

//! \brief Sent immediately before MAXScript loads its startup scripts. stdscripts will have already been read.
#define NOTIFY_PRE_MXS_STARTUP_SCRIPT_LOAD 0x00000106

//! \brief Sent when potentially starting to shut 3ds Max down, before checking for things that can cancel the shutdown, such as scene dirty or ExitMAXCallback callback object returning false.
#define NOTIFY_SYSTEM_SHUTDOWN_CHECK                0x00000108

//! \brief Sent if system shutdown was cancelled
#define NOTIFY_SYSTEM_SHUTDOWN_CHECK_FAILED         0x00000109

//! \brief Sent if system shutdown was not cancelled. and system shutdown is about to start
#define NOTIFY_SYSTEM_SHUTDOWN_CHECK_PASSED         0x0000010A

//! \brief Sent after a file is merged.
/* NotifyInfo::callParam is a pointer to NotifyPostMerge3Param.*/
DEFINE_NOTIFY_CODE(NOTIFY_FILE_POST_MERGE3, 0x0000010B, NotifyPostMerge3Param*)

//! \brief Sent when ActiveShade in the frame buffer is toggled on/off. 
//! NotifyInfo::callParam is a pointer to a BOOL which is TRUE if ActiveShade in the frame buffer is being switched on or FALSE if it's being switched off.
//! Please note that toggling ActiveShade in the viewport generates a different notification, namely NOTIFY_ACTIVESHADE_IN_VIEWPORT_TOGGLED
DEFINE_NOTIFY_CODE(NOTIFY_ACTIVESHADE_IN_FRAMEBUFFER_TOGGLED, 0x0000010C, BOOL*)

//! \brief Sent before ActiveShade in the viewport is toggled on/off. NotifyInfo::callParam is a pointer to a BOOL which is TRUE if ActiveShade is being switched on or FALSE if it's being switched off.
//! Please note that toggling ActiveShade in the frame buffer generates a different notification, namely NOTIFY_ACTIVESHADE_IN_FRAMEBUFFER_TOGGLED
DEFINE_NOTIFY_CODE(NOTIFY_PRE_ACTIVESHADE_IN_VIEWPORT_TOGGLED, 0x0000010D, BOOL*)

//! \brief Sent after ActiveShade in the viewport is toggled on/off. NotifyInfo::callParam is a pointer to a BOOL which is TRUE if ActiveShade is being switched on or FALSE if it's being switched off.
//! Please note that toggling ActiveShade in the frame buffer generates a different notification, namely NOTIFY_ACTIVESHADE_IN_FRAMEBUFFER_TOGGLED
DEFINE_NOTIFY_CODE(NOTIFY_POST_ACTIVESHADE_IN_VIEWPORT_TOGGLED, 0x0000010E, BOOL*)

//! \brief Sent \b before the system color management settings changed, this
//! includes mode changes, system-wide gamma setting or changes to color config.
#define NOTIFY_COLOR_MANAGEMENT_PRE_CHANGE 0x0000010F

//! \brief Sent \b after the system color management settings are changed, this
//! includes mode changes, system-wide gamma setting or changes to color config.
//! At this point all the new settings will be available through the
//! IColorPipelineMgr interface but changes may not have propagated through the
//! system yet (for example bitmaps may still have the old color spaces assigned
//! to them). If you want to get notifications after the first wave of updates,
//! then please listen to the NOTIFY_COLOR_MANAGEMENT_POST_CHANGE2 message.
//! NotifyInfo::callParam is a pointer to uint32_t which holds one or more
//! MaxSDK::ColorMangement::NotificationChangeMask bits indicating what has
//! changed.
DEFINE_NOTIFY_CODE(NOTIFY_COLOR_MANAGEMENT_POST_CHANGE, 0x00000110, uint32_t*)

//! \brief Sent right \b after the NOTIFY_COLOR_MANAGEMENT_POST_CHANGE
//! notification, signaling that all entities listening to the
//! NOTIFY_COLOR_MANAGEMENT_POST_CHANGE notification has finished responding to
//! it. If there were changes in the input color spaces, at this point all the
//! bitmaps in the system will have new color spaces assigned to them.
//! NotifyInfo::callParam is a pointer to uint32_t which holds one or more
//! MaxSDK::ColorMangement::NotificationChangeMask bits indicating what has
//! changed.
DEFINE_NOTIFY_CODE(NOTIFY_COLOR_MANAGEMENT_POST_CHANGE2, 0x00000111, uint32_t*)

// //! \brief Sent when the quad menu structure is loaded.
//! NotifyInfo::callParam is a pointer to the ICuiQuadMenuManager loading the menu structure.
DEFINE_NOTIFY_CODE(NOTIFY_CUI_REGISTER_QUAD_MENUS, 0x00000112, MaxSDK::CUI::ICuiQuadMenuManager*)

//! \brief: Sent before menu structure is modified (for example before loading a mnxb file)
//!             The intent is to use this notification to clear any storage related to menu structure to avoid keeping dangling pointers
//! NotifyInfo::callParam is nullptr
#define NOTIFY_CUI_MENUS_INVALID 0x00000113

//! \brief: Sent after menu structure is modified (for example after loading a mnxb file)
//!             The intent is to use this notification to perform any additional ui updates after new menus are loaded
//! NotifyInfo::callParam is nullptr
#define NOTIFY_CUI_MENUS_VALID 0x00000114

//! \brief: Sent before quad menu structure is modified (for example before loading a qmnxb file)
//!             The intent is to use this notification to clear any storage related to menu structure to avoid keeping dangling pointers
//! NotifyInfo::callParam is nullptr
#define NOTIFY_CUI_QUAD_MENUS_INVALID 0x00000115

//! \brief: Sent after quad menu structure is modified (for example after loading a qmnxb file)
//!             The intent is to use this notification to perform any additional ui updates after new menus are loaded
//! NotifyInfo::callParam is nullptr
#define NOTIFY_CUI_QUAD_MENUS_VALID 0x00000116

//! \brief: Sent before menu structure is loaded 
//! NotifyInfo::callParam is a string (MCHAR*) containing the path of the file that will be loaded
DEFINE_NOTIFY_CODE(NOTIFY_CUI_MENUS_PRE_LOAD, 0x00000117, const MCHAR*)

//! \brief: Sent befoe quadmenu structure is loaded
//! NotifyInfo::callParam is a string (MCHAR*) containing the path of the file that will be loaded
DEFINE_NOTIFY_CODE(NOTIFY_CUI_QUAD_MENUS_PRE_LOAD, 0x00000118, const MCHAR*)

//! \brief: Sent after quadmenu structure is loaded
//! NotifyInfo::callParam is a string (MCHAR*) containing the path of the file that has been loaded
DEFINE_NOTIFY_CODE(NOTIFY_CUI_QUAD_MENUS_POST_LOAD, 0x00000119, const MCHAR*)

//! \brief Sent when the standard menu structure is loaded.
//! NotifyInfo::callParam is a pointer to the ICuiMenuManager loading the menu structure.
DEFINE_NOTIFY_CODE(NOTIFY_CUI_REGISTER_MENUS, 0x00000120, MaxSDK::CUI::ICuiMenuManager*)

// !!!!!!!!!!!!  If you are adding more notification codes, read the following!  !!!!!!!!!!!!

// Note #1: If you add more built-in notification codes, consider
//    increasing NUM_BUILTIN_NOTIFIERS in core\notifyManager.h - currently 0x0200

// Note #2: If you add more built-in notification codes, consider
//    adding them to MAXScript. See maxscrpt\maxcallbacksTable.h and update the define NUM_CALLBACKS

// Note #3: If you add more built-in notification codes, you may need to update
//    NOTIFY_CUSTOM_LOW in core\Tests\NotifyTest.cpp (~line 75) and update the comments above it

// Note #4: If you add more built-in notification codes, consider
//	  adding them to MaxPlus/facade/facade_notifications.h and/or
//    dll/ManagedServices/MaxNotificationListener.h

// Note #5: If you add more built-in notification codes, consider
//	adding them dll\Python\scripts\pymxs\notifications.py for Python

// Note #6: If you add more built-in notification codes, consider adding them under SystemNotificationCode in 
//	dll/ephere/Wrapper/Generation.config

// Start of messages for internal use only.
#define NOTIFY_INTERNAL_USE_START				0x70000000

/** \name New Scene Options
\anchor new_scene_options
Flag values in callParam for NOTIFY_SYSTEM_PRE_NEW and NOTIFY_SYSTEM_POST_NEW
indicating the type of new scene operation that was carried out.
*/
//!@{
//! \brief All objects are deleted, including their animation data
#define PRE_NEW_NEW_ALL 0x1
//!@}

//!@}	END OF NotificationCodes System Notification Codes

// The notification callback function
using NOTIFYPROC = void (*)(void* param, NotifyInfo* info);

// Integer versions -- For pre-defined MAX codes
/*! \remarks This global function is called to establish the connection between the event and the callback.
\param proc The callback function called when the event occurs
\param param A pointer to a parameter which will be passed to the callback function
\param code Specifies which notification to register. See \ref NotificationCodes.
\return Value Nonzero if the event was registered; otherwise zero.
*/
CoreExport int RegisterNotification(NOTIFYPROC proc, void* param, NotifyCode code);
/*! \remarks This global function is called to break the connection between the event
and the callback. After this function executes the callback is no
longer invoked when the event occurs.
\param proc The callback function called when the event occurs.
\param param This parameter must be identical to the param sent into RegisterNotification().
This function will only unregister a callback if this parameter equals
the param sent in to the RegisterNotification() function.
\param code Specifies which notification to unregister. See \ref NotificationCodes
\return Nonzero if the event was unregistered; otherwise zero. */
CoreExport int UnRegisterNotification(NOTIFYPROC proc, void* param, NotifyCode code);
// Unregister a callback from all codes
/*! \remarks This global function unregisters the callback from all codes
\param proc The callback function called when the event occurs.
\param param A pointer to a parameter which will be passed to the callback function.
\return Nonzero if the events were unregistered; otherwise zero.
*/
CoreExport int UnRegisterNotification(NOTIFYPROC proc, void* param);

/*! It is not recommended to use this function directly.
Consider using the templated version of this function for improved type safety
\param code Specifies which notification to broadcast. See \ref NotificationCodes */
CoreExport void BroadcastNotification(NotifyCode code);
/*! It is not recommended to use this function directly.
Consider using the templated version of this function for improved type safety
\param code Specifies which notification to broadcast. See \ref NotificationCodes
\param callParam This parameter is passed to the callback. */
CoreExport void BroadcastNotification(NotifyCode code, void* callParam);

/*! \remarks Calling this global function causes the callback
corresponding to the specified code to be called.
\param code Specifies which notification to broadcast. See \ref NotificationCodes */
template <NotifyCode code>
inline void BroadcastNotification()
{
	// Fail if something other than void* is expected and param is missing in function call
	static_assert(std::is_same_v<notify_param_t<code>, void*>, "Missing param for notification code");
	BroadcastNotification(code);
}

/*! \remarks This causes the callback corresponding to the specified code to be called
and passes the specified parameter along to the callback.
code Specifies which notification to broadcast. See \ref NotificationCodes
\param callParam This parameter is passed to the callback. */
template <NotifyCode code>
inline void BroadcastNotification(notify_param_t<code> callParam)
{
	if constexpr (std::is_pointer_v<decltype(callParam)>)
	{
		BroadcastNotification(code, (void*)callParam);
	}
	else
	{
		BroadcastNotification(code, (void*)static_cast<intptr_t>(callParam));
	}
}
