#pragma once

//**************************************************************************/
// Copyright (c) 2013 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
// DESCRIPTION: Notification API internal (private header)
// AUTHOR: David Lanier
//***************************************************************************/


#include "NotificationAPI/NotificationAPI_Subscription.h"
#include "NotificationAPI/NotificationAPI_Events.h"
#include "IMergeableEvent.h"

namespace Max
{;
namespace NotificationAPI
{;
using namespace MaxSDK::NotificationAPI;

    class GenericEvent : public IGenericEvent, public IMergeableEvent
	{	
		NotifierType	m_NotififerType;
		size_t		    m_EventType;

	public:
		GenericEvent(NotifierType notififerType, size_t eventType);
		virtual ~GenericEvent(){};

		//Gives access to the type of notifier and the user data the plugin (usually renderer has added)
		virtual NotifierType	GetNotifierType	(void)const{return m_NotififerType;};
		virtual size_t		    GetEventType	(void)const{return m_EventType;};
        virtual void		    DebugPrintToFile(FILE* f)const;

        // -- inherited from IMergeableEvent
        virtual MergeResult merge_from(IMergeableEvent& old_event);
	};

};//end of namespace NotificationAPI
};//end of namespace Max