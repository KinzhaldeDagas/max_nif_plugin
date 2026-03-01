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

#include "IMergeableEvent.h"

#include <NotificationAPI/NotificationAPI_Events.h>

namespace Max
{;
namespace NotificationAPI
{;
using namespace MaxSDK::NotificationAPI;

	//The plugin receives a INodeEvent instance when something is changed in a node
	class NodeEvent : public virtual INodeEvent, public IMergeableEvent
	{
	public:
		NodeEvent(NotifierType notifierType, NodeEventType eventType, INode* pNode);

        // -- inherited form IGenericEvent
		virtual NotifierType	                        GetNotifierType	    (void)const{return m_NotififerType;};
		virtual size_t		                            GetEventType	    (void)const{return m_EventType;};
        virtual void		                            DebugPrintToFile(FILE* f)const;
        // -- inherited from INodeEvent
		virtual INode*			                        GetNode			    (void)const{return m_pNode;};

        // -- inherited from IMergeableEvent
        virtual MergeResult merge_from(IMergeableEvent& old_event);

    private:

        NotifierType m_NotififerType;
        NodeEventType m_EventType;
        INode* m_pNode;
	};

    class NodeParamBlockEvent : public INodeParamBlockEvent, public NodeEvent
    {
    public:

        NodeParamBlockEvent(NotifierType notifierType, NodeEventType eventType, INode* pNode, const ParamBlockData& paramBlockData);

        // -- inherited from INodeParamBlockEvent
        virtual const MaxSDK::Array<ParamBlockData>& GetParamBlockData() const override;

        //Non const function to modify the content when we merge 2 NodeEvent instances with paramblockdata
        MaxSDK::Array<ParamBlockData>& GetParamBlockData();

    private:
        MaxSDK::Array<ParamBlockData> m_ParamsChangedInPBlock; //Used only in case of EventType_Node_ParamBlock
    };

    class NodePropertyEvent : public INodePropertyEvent, public NodeEvent
    {
    public:
        NodePropertyEvent(NotifierType notifierType, NodeEventType eventType, INode* pNode, const PartID part_id);

        // -- inherited from INodePropertyEvent
        virtual PartID GetPropertyID() const override;

    private:

        PartID m_part_id;
    };

	
};//end of namespace NotificationAPI
};//end of namespace Max