//**************************************************************************/
// Copyright (c) 2013 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#pragma once

#include "Notifiers/MaxNotifier.h"


namespace Max
{;
namespace NotificationAPI
{;
using namespace MaxSDK::NotificationAPI;



	class MaxSceneNotifier : public MaxNotifier
	{
    protected:
        static void ProcessNotifications(void *param, NotifyInfo *info); //To receive events from RegisterNotification(ProcessNotifications, this, NOTIFY_NODE_CREATED);

	public:
		MaxSceneNotifier();

        void Notify_SceneNodeAdded(INode& node);
        void Notify_SceneNodeRemoved(INode& node);

        //From MaxNotifier
        virtual void DebugPrintToFile(FILE* f, size_t indent)const;

    protected:

        // Protected destructor forces going through delete_this()
        virtual ~MaxSceneNotifier();

	};


} } // namespaces
