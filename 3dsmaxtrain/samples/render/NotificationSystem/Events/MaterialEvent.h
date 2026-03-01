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

	//The plugin receives a IMaterialEvent instance when something is changed in a Material
	class MaterialEvent : public IMaterialEvent, public IMergeableEvent
	{
		MaterialEventType		                    m_EventType;
		Mtl*			                m_pMtl;
        MaxSDK::Array<ParamBlockData>   m_ParamsChangedInPBlock; //Used only in case of EventType_Material_ParamBlock

	public:
		MaterialEvent(MaterialEventType eventType, Mtl* pMtl, const ParamBlockData* paramblockData = NULL);
		virtual ~MaterialEvent(){};

		virtual NotifierType	                        GetNotifierType	    (void)const{return NotifierType_Material;};
		virtual size_t		                            GetEventType	    (void)const{return m_EventType;};
		virtual Mtl*			                        GetMtl			    (void)const{return m_pMtl;};
        virtual const MaxSDK::Array<ParamBlockData>&    GetParamBlockData   (void)const{return m_ParamsChangedInPBlock;};
        virtual void		                            DebugPrintToFile    (FILE* f)const;

        MaxSDK::Array<ParamBlockData>&                  GetParamBlockData   (void){return m_ParamsChangedInPBlock;}//Non-const, is used to change the array in Notification API when you merge 2 MaterialEvent dealing with paramblocks

        // -- inherited from IMergeableEvent
        virtual MergeResult merge_from(IMergeableEvent& old_event);
	};


};//end of namespace NotificationAPI
};//end of namespace Max