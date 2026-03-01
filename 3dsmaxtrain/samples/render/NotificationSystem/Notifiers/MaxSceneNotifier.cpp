//**************************************************************************/
// Copyright (c) 2013 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#include "MaxSceneNotifier.h"

#include "Events/SceneNodeEvent.h"
#include "NotificationManager.h"

// Max SDK
#include <notify.h>

namespace Max
{;
namespace NotificationAPI
{;
using namespace MaxSDK::NotificationAPI;


MaxSceneNotifier::MaxSceneNotifier()
{
    RegisterNotification(ProcessNotifications, this, NOTIFY_NODE_CREATED);
    RegisterNotification(ProcessNotifications, this, NOTIFY_SCENE_ADDED_NODE);
    RegisterNotification(ProcessNotifications, this, NOTIFY_SCENE_PRE_DELETED_NODE);
}

MaxSceneNotifier::~MaxSceneNotifier()
{
    UnRegisterNotification(ProcessNotifications, this, NOTIFY_NODE_CREATED);
    UnRegisterNotification(ProcessNotifications, this, NOTIFY_SCENE_ADDED_NODE);
    UnRegisterNotification(ProcessNotifications, this, NOTIFY_SCENE_PRE_DELETED_NODE);
}

void MaxSceneNotifier::ProcessNotifications(void *param, NotifyInfo *info)
{
    if(DbgVerify((info != nullptr) && (param != nullptr)))
    {
	    MaxSceneNotifier* pSceneNotifier = reinterpret_cast<MaxSceneNotifier*>(param);
	    switch(info->intcode)
	    {
            case NOTIFY_SCENE_ADDED_NODE:
		    case NOTIFY_NODE_CREATED:
		        {
                    INode* pNewNode = GetNotifyParam<NOTIFY_SCENE_ADDED_NODE, NOTIFY_NODE_CREATED>(info);
                    if(DbgVerify(pNewNode != nullptr))
                    {
                        pSceneNotifier->Notify_SceneNodeAdded(*pNewNode);
                    }
                }
                break;
            case NOTIFY_SCENE_PRE_DELETED_NODE:
                {
                    INode* const node = GetNotifyParam<NOTIFY_SCENE_PRE_DELETED_NODE>(info);
                    if(DbgVerify(node != nullptr))
                    {
                        pSceneNotifier->Notify_SceneNodeRemoved(*node);
                    }
                }
                break;
            default:
                DbgAssert(false);
                break;
        }
    }
}

void MaxSceneNotifier::DebugPrintToFile(FILE* file, size_t indent)const
{
    if (NULL == file){
		DbgAssert(0 && _T("file is NULL"));
		return;
	}

    TSTR indentString = _T("");
    for (size_t i=0;i<indent;++i){
        indentString += TSTR(_T("\t"));
    }

    _ftprintf(file, indentString);
    _ftprintf(file, _T("** Scene Notifier data : **\n"));
    
    //Print base class
   __super::DebugPrintToFile(file, ++indent);
}

void MaxSceneNotifier::Notify_SceneNodeAdded(INode& node)
{
    NotifyEvent(SceneNodeEvent::MakeNodeAddedEvent(&node));
}

void MaxSceneNotifier::Notify_SceneNodeRemoved(INode& node)
{
    NotifyEvent(SceneNodeEvent::MakeNodeRemovedEvent(&node));
}

} } // namespaces
