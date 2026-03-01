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

#include <NotificationAPI/NotificationAPI_Events.h>
#include "IMergeableEvent.h"

namespace Max
{;
namespace NotificationAPI
{;
using namespace MaxSDK::NotificationAPI;

	//The plugin receives a ITexmapEvent instance when something is changed in a Texmap
	class TexmapEvent : public ITexmapEvent, public IMergeableEvent
	{
		size_t		                    m_EventType;
		Texmap*			                m_pTexmap;
        MaxSDK::Array<ParamBlockData>   m_ParamsChangedInPBlock; //Used only in case of EventType_Texmap_ParamBlock

	public:
		TexmapEvent(TexmapEventType eventType, Texmap* pTexmap, const ParamBlockData* paramBlockData = NULL);
		virtual ~TexmapEvent(){};

		virtual NotifierType	                        GetNotifierType	    (void)const{return NotifierType_Texmap;};
		virtual size_t		                            GetEventType	    (void)const{return m_EventType;};
		virtual Texmap*			                        GetTexmap		    (void)const{return m_pTexmap;};
        virtual const MaxSDK::Array<ParamBlockData>&    GetParamBlockData   (void)const{return m_ParamsChangedInPBlock;};

        virtual void		                            DebugPrintToFile    (FILE* f)const;

         MaxSDK::Array<ParamBlockData>&                 GetParamBlockData   (void){return m_ParamsChangedInPBlock;}//Non-const, is used to change the array in Notification API when you merge 2 TexmapEvent dealing with paramblocks

         // -- inherited from IMergeableEvent
         virtual MergeResult merge_from(IMergeableEvent& old_event);
	};

};//end of namespace NotificationAPI
};//end of namespace Max