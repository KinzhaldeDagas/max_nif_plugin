//**************************************************************************/
// Copyright (c) 2023 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#pragma once

#include "MaxHeap.h"
#include "maxtypes.h"
#include "strbasic.h"

class ActionItem;
class Animatable;
class ClassDesc;
class DllDesc;
class FPValue;
class FrameRendParams;
class IGraphNode;
class ILayer;
class INode;
class INodeTab;
class Modifier;
class ModContext;
class Mtl;
class MtlBase;
class MtlBaseLib;
class RenderGlobalContext;
class RendParams;
class TimeChangeCallback;
class View;
class ViewParams;
namespace MaxSDK::CUI
{
	class ICuiMenuManager;
	class ICuiQuadMenuManager;
}
template <class T> class Tab;
enum CloneType : int;
enum RenderSettingID : int;

//! \brief Type of all notification codes
// using NotifyCode = int;  // this causes problems with doxygen used for creating .net wrapper
typedef int NotifyCode;

// Notification information structure -- Passed to NOTIFYPROC to inform it what
// it's being notified about...
/*! 3ds Max supports a system where a plug-in can ask to receive a callback
when certain events occur. These are events such as the system unit
settings changing, system time setting changing, or the user executing
File/Reset, File/New, etc. \n\n
This structure is part of this system. It is available in release 2.0
and later only. The Interface class has related methods for registering callbacks. \n\n
The plug-in creates a callback function to process the notification.
The notification callback function (NOTIFYPROC) is defined as follows:\n
<code>using NOTIFYPROC = void (*)(void *param, NotifyInfo *info);</code> \n
The NotifyInfo structure is passed to the <b>NOTIFYPROC</b> to inform
it of what it's being notified about. \n\n
The sample code below shows how this system may be used.
\par Sample Code:
\code
// Declare the callback function
static void TimeUnitsChanged(void *param, NotifyInfo *info)
{
// Handle the units changing...
}
// Register the callback
RegisterNotification(TimeUnitsChanged,this, NOTIFY_TIMEUNITS_CHANGE);

// When done, unregister the callback
UnRegisterNotification(TimeUnitsChanged,this, NOTIFY_TIMEUNITS_CHANGE);
\endcode
\par See Functions:
RegisterNotification(NOTIFYPROC proc, void* param, NotifyCode code); \n
UnRegisterNotification(NOTIFYPROC proc, void* param, NotifyCode code); \n
BroadcastNotification(NotifyCode code); \n
BroadcastNotification(NotifyCode code, void* callParam); \n
UnRegisterNotification(NOTIFYPROC proc, void *param);
*/
struct NotifyInfo : public MaxHeapOperators
{
	NotifyCode intcode = 0;
	void* callParam = nullptr;  // this param can be passed in with BroadcastNotification;
};

//! \brief The name change call parameter structure.
/*! The call parameter that accompanies any notification that involves
	a name change of an object(\ref NameChangeNotifications), which contains
	information of both the old name and the new name of the changed
	object.
*/
struct NameChange : public MaxHeapOperators
{
	//! \brief The old name of the changed object.
	const MCHAR* oldname = nullptr;
	//! \brief The new name that is assigned to the changed object.
	const MCHAR* newname = nullptr;
};

//! \brief The object redefinition call parameter structure.
/*! Instance of this structure is passed as call param on NOTIFY_OBJECT_DEFINITION_CHANGE_BEGIN/NOTIFY_OBJECT_DEFINITION_CHANGE_END
	broadcast notifications.
*/
struct ObjectDefinitionChangeNotificationParam : public MaxHeapOperators
{
	//! \brief The types of object definition redefinition.
	enum ObjectDefinitionType {
		//! \brief A scripted plugin/scripted custom attribute. The definition passed will be a ClassDesc2.
		kScriptedPlugin = 0
	};
	//! \brief The type of object definition being redefined
	ObjectDefinitionType m_type{};
	//! \brief A pointer whose type is dependent on the type of object definition being redefined
	void* m_definition = nullptr;
	ObjectDefinitionChangeNotificationParam(ObjectDefinitionType type, void* definition) : m_type(type), m_definition(definition) {}
private:
	ObjectDefinitionChangeNotificationParam();
};

//! \brief The call parameter that accompanies the notification code NOTIFY_FILE_POST_OPEN
struct NotifyPostOpenParam : public MaxHeapOperators
{
	//! \brief the type of the file that was opened \see FileIOType
	FileIOType fileIOType{ FileIOType::IOTYPE_MAX };
	//! \brief fileSaveVersion stores the result of ILoad::GetFileSaveVersion()
	DWORD fileSaveVersion{ 0 };
	//! \brief fileSaveAsVersion stores the result of ILoad::FileSaveAsVersion()
	DWORD fileSaveAsVersion{ 0 };

	NotifyPostOpenParam(FileIOType fileIOType, DWORD fileSaveVersion, DWORD fileSaveAsVersion)
		: fileIOType(fileIOType), fileSaveVersion(fileSaveVersion), fileSaveAsVersion(fileSaveAsVersion) {}
};

//! \brief The call parameter that accompanies the notification code NOTIFY_FILE_POST_MERGE3
struct NotifyPostMerge3Param : public MaxHeapOperators
{
	//! \brief size of this struct
	const size_t structSize;
	// ! \brief path of the file being merged/xref
	const MCHAR* filePath{ nullptr };
	// ! \brief true on successful merge
	bool mergeSuccess{ false };

	// From NOTIFY_FILE_POST_MERGE2
	//! \brief fileSaveVersion stores the result of ILoad::GetFileSaveVersion()
	DWORD fileSaveVersion{ 0 };
	//! \brief fileSaveAsVersion stores the result of ILoad::FileSaveAsVersion()
	DWORD fileSaveAsVersion{ 0 };

	// From NOTIFY_FILE_POST_MERGE
	// ! \brief true for xref, false for merge
	bool isXrefMerge{ false };

	NotifyPostMerge3Param() : NotifyPostMerge3Param(sizeof(NotifyPostMerge3Param)) {}

protected:
	NotifyPostMerge3Param(size_t size) : structSize(size) {}
};

/** \anchor notify_file_process_type
\name File Process Type */
// Used in struct NotifyFileProcessParam
enum FileProcessType : int
{
	//* \brief iProcessType value for scene file save/load
	FILE_PROCESS_SCENE = 0x1,
	//* \brief iProcessType value for hold/fetch
	FILE_PROCESS_HOLD_FETCH = 0x2,
	//* \brief iProcessType value for auto backup
	FILE_PROCESS_AUTOBAK = 0x3
};

//! \brief The call parameter that accompanies the notification codes
// NOTIFY_FILE_PRE_OPEN_PROCESS, NOTIFY_FILE_POST_OPEN_PROCESS,
// NOTIFY_FILE_PRE_SAVE_PROCESS, NOTIFY_FILE_POST_SAVE_PROCESS
struct NotifyFileProcessParam : public MaxHeapOperators
{
	//! \brief The type of operation. See \ref notify_file_process_type
	FileProcessType iType{ 0 };
	//! \brief The full path of the file being processed
	const MCHAR* fname{ nullptr };
	//! \brief Whether the file open or save was successful. 
	bool fileOperationSuccessful{ false };
};

//! \brief The call parameter that accompanies the notification codes
// NOTIFY_PRE_MODIFIER_ADDED, NOTIFY_POST_MODIFIER_ADDED, 
// NOTIFY_PRE_MODIFIER_DELETED, NOTIFY_POST_MODIFIER_DELETED
struct NotifyModAddDelParam : public MaxHeapOperators
{
	INode* node = nullptr;
	Modifier* mod = nullptr;
	ModContext* mc = nullptr;
};

//! \brief The call parameter that accompanies the notification codes
//! NOTIFY_IMPORT_FAILED, NOTIFY_EXPORT_FAILED
struct NotifyImpExpFailedParam : public MaxHeapOperators
{
	//! \brief The full path to the file being imported/exported. This can be null if no file was selected.
	const MCHAR* filePath = nullptr;
	//! \brief The return code of the importer/exporter. Can be either IMPEXP_FAIL or IMPEXP_CANCEL.
	int impexpCode = 0;
};

//! \brief The call parameter that accompanies the notification code NOTIFY_POST_NODES_CLONED
struct NotifyPostNodesCloned : public MaxHeapOperators
{
	INodeTab* origNodes = nullptr;
	INodeTab* clonedNodes = nullptr;
	CloneType cloneType{};
};

//! \brief The call parameter that accompanies the notification code NOTIFY_NODE_LAYER_CHANGED
struct LayerChange : public MaxHeapOperators
{
	INode* node = nullptr;
	ILayer* oldlayer = nullptr;
	ILayer* newlayer = nullptr;
};

//! \brief The call parameter that accompanies the notification code NOTIFY_NODE_NAME_SET
struct NodeNameSetParam : public MaxHeapOperators
{
	const MCHAR* oldname = nullptr;
	const MCHAR* newname = nullptr;
	INode* node = nullptr;
};

/** \brief The call parameter that accompanies the notification code NOTIFY_CLASSDESC_REPLACED
	A pointer to an instance of this structure is passed to BroadcastNotification after a ClassDesc is
	replaced by another one in a ClassDirectory. This occurs when the dll containing a deferred loaded plugin
	is loaded. */
struct ClassDescReplaced : public MaxHeapOperators
{
	//! The old class descriptor
	const ClassDesc* oldClassDesc = nullptr;
	//! The new class descriptor
	const ClassDesc* newClassDesc = nullptr;
};

/** \name File Attributes  */
//!@{
//* \brief iStatus value for NOTIFY_FILE_CHECK_STATUS
/*	\see NOTIFY_FILE_CHECK_STATUS
	Allows the caller to set whether the file should be considered read only or not.
	If the iStatus flag is set with this flag:
	\code
		iStatus |= FILE_STATUS_READONLY;
	\endcode
	Then the system will not save a file, and a file save operation will exit early.
*/
#define FILE_STATUS_READONLY				0x1
//!@}

//! \brief The call parameter that accompanies the notification code NOTIFY_FILE_CHECK_STATUS
struct NotifyFileCheckStatus : public MaxHeapOperators
{
	const MCHAR* filePath = nullptr;
	int iStatus = 0;
};

//! \brief The call parameter that accompanies the notification code NOTIFY_MODPANEL_SUBOBJECTLEVEL_CHANGED
struct NumberChange : public MaxHeapOperators
{
	int newNumber = 0;
	int oldNumber = 0;
};

//! \brief The call parameter that accompanies the notification code NOTIFY_FAILED_DIRECTX_MATERIAL_TEXTURE_LOAD
struct NotifyFailedDxMatTexLoad : public MaxHeapOperators
{
	const MCHAR* fileName = nullptr;
	const MCHAR* paramName = nullptr;
	Mtl* mtl = nullptr;
	bool forceReload = false;
};

//! \brief The call parameter that accompanies the notification codes
//! NOTIFY_CUSTOM_ATTRIBUTES_ADDED and NOTIFY_CUSTOM_ATTRIBUTES_REMOVED
struct NotifyCustomAttribute : public MaxHeapOperators
{
	NotifyCustomAttribute(Animatable* owner = nullptr, Animatable* custAttr = nullptr) :
		m_owner(owner), m_customAttribute(custAttr) {};
	/*! \brief The owner of the custom attribute that was added or is about to be removed */
	Animatable* m_owner = nullptr;
	/*! \brief The custom attribuet that was added or is about to be removed */
	Animatable* m_customAttribute = nullptr;
};

//! \brief The call parameter that accompanies the notification code NOTIFY_LAYER_PARENT_CHANGED
struct LayerParentChange : public MaxHeapOperators
{
	//! \brief pointer on the layer that has changed parent
	ILayer* layerChanged = nullptr;
	//! \brief pointer on the old parent of the layer
	ILayer* oldParent = nullptr;
};
