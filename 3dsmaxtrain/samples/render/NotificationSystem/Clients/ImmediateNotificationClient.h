#pragma once

//**************************************************************************/
// Copyright (c) 2013 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
// AUTHOR: David Lanier
//***************************************************************************/

// src/include
#include <NotificationAPI/NotificationAPI_Subscription.h>
// Std headers
#include <list>
#include <map>

namespace Max
{;
namespace NotificationAPI
{;
using namespace MaxSDK::NotificationAPI;


    //Sorts out all referencing messages and callback the subscribers
	class ImmediateNotificationClient : 
        public IImmediateNotificationClient
	{
        bool    m_NotificationsEnabled;

	public:
        ImmediateNotificationClient();
		virtual ~ImmediateNotificationClient();
		
        //From INotificationclient
        virtual int                     VersionNumber()const{return 1;};
        virtual void                    EnableNotifications(bool enable){m_NotificationsEnabled = enable;};
        virtual bool                    NotificationsEnabled(void)const{return m_NotificationsEnabled;};
        virtual void                    DebugPrintToFile(FILE* file)const;

        //From IImmediateNotificationClient
		//user data associated to this 'thing' is owned and memory managed by the customer
		virtual bool MonitorNode				    (INode& node,		NotifierType type, size_t monitoredEvents, INotificationCallback& callback, void* userData) override;
		virtual bool MonitorMaterial			    (Mtl& mtl,			size_t monitoredEvents, INotificationCallback& callback, void* userData) override;
		virtual bool MonitorTexmap				    (Texmap& texmap,	size_t monitoredEvents, INotificationCallback& callback, void* userData) override;
        virtual bool MonitorReferenceTarget(ReferenceTarget& refTarg, size_t monitoredEvents, INotificationCallback& callback, void* userData) override;
		virtual bool MonitorViewport                (int viewID, size_t monitoredEvents, INotificationCallback& callback, void* userData) override;
		virtual bool MonitorRenderEnvironment	    (size_t monitoredEvents, INotificationCallback& callback, void* userData) override;
		virtual bool MonitorScene			        (size_t monitoredEvents, INotificationCallback& callback, void* userData) override;
        virtual bool MonitorRenderSettings          (size_t monitoredEvents, INotificationCallback& callback, void* userData) override;
        virtual bool StopMonitoringNode(INode& node, size_t monitoredEvents, INotificationCallback& callback, void* user_data) override;
        virtual bool StopMonitoringMaterial(Mtl& mtl, size_t monitoredEvents, INotificationCallback& callback, void* user_data) override;
        virtual bool StopMonitoringTexmap(Texmap& texmap, size_t monitoredEvents, INotificationCallback& callback, void* user_data) override;
        virtual bool StopMonitoringReferenceTarget(ReferenceTarget& refTarg, size_t monitoredEvents, INotificationCallback& callback, void* userData) override;
        virtual bool StopMonitoringViewport(int viewID, size_t monitoredEvents, INotificationCallback& callback, void* user_data) override;
        virtual bool StopMonitoringRenderEnvironment(size_t monitoredEvents, INotificationCallback& callback, void* user_data) override;
        virtual bool StopMonitoringRenderSettings(size_t monitoredEvents, INotificationCallback& callback, void* user_data) override;
        virtual bool StopMonitoringScene(size_t monitoredEvents, INotificationCallback& callback, void* user_data) override;

        virtual void GetClassName(MSTR& s) const { s = _M("Graphics.ImmediateNotificationClient"); }

    private:

	};

    
	
};//end of namespace NotificationAPI
};//end of namespace Max