//**************************************************************************/
// Copyright (c) 2013 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
// DESCRIPTION: Implementation
// AUTHOR: David Lanier
//***************************************************************************/

#include "SceneNodeEvent.h"

// src/include
#include "../NotificationAPIUtils.h"
// Max sdk
#include <inode.h>
// std includes
#include <time.h>


namespace Max
{;
namespace NotificationAPI
{;
using namespace MaxSDK::NotificationAPI;

//---------------------------------------------------------------------------
//----------			class SceneNodeEvent				    ---------
//---------------------------------------------------------------------------

SceneNodeEvent::SceneNodeEvent(const SceneEventType updateType, INode* const node)
    : m_EventType(updateType),
    m_node(node)
{

}

SceneNodeEvent::SceneNodeEvent(const SceneNodeEvent& from)
    : m_EventType(from.m_EventType),
    m_node(from.m_node)
{

}

SceneNodeEvent::~SceneNodeEvent()
{

}

SceneNodeEvent SceneNodeEvent::MakeNodeAddedEvent(INode* const node)
{
    return SceneNodeEvent(EventType_Scene_Node_Added, node);
}

SceneNodeEvent SceneNodeEvent::MakeNodeRemovedEvent(INode* const node)
{
    return SceneNodeEvent(EventType_Scene_Node_Removed, node);
}

void SceneNodeEvent::DebugPrintToFile(FILE* file)const
{
    if (! file){
        DbgAssert(0 && _T("file is NULL"));
        return;
    }
    
    time_t ltime;
	time(&ltime);

    _ftprintf(file, _T("\n** SceneNodeEvent parameters : %s **\n"), _tctime(&ltime) );

    size_t indent = 1;
    Utils::DebugPrintToFileNotifierType(file, GetNotifierType(), NULL, indent);
    Utils::DebugPrintToFileEventType(file, GetNotifierType(), m_EventType, indent);

     TSTR indentString = _T("");
    for (size_t i=0;i<indent;++i){
        indentString += TSTR(_T("\t"));
    }

	INode* pNode = m_node;
	if (!  pNode){ 
        DbgAssert(0 && _T("Invalid NULL node pointer"));
        _ftprintf(file, _T("** ERROR Node is NULL**\n"));
    }
    else
    {
        _ftprintf(file, indentString);
        _ftprintf(file, _T("** Node: \"%s\"**\n"), pNode->GetName());
    }
}

INode* SceneNodeEvent::GetNode() const 
{
    return m_node;
}

SceneNodeEvent::MergeResult SceneNodeEvent::merge_from(IMergeableEvent& generic_old_event)
{
    SceneNodeEvent* const old_event = dynamic_cast<SceneNodeEvent*>(&generic_old_event);
    if((old_event != nullptr) && (old_event->m_node == this->m_node))
    {
        // An addition nullifies a removal
        if(((old_event->m_EventType == EventType_Scene_Node_Removed) && (this->m_EventType == EventType_Scene_Node_Added))
            // A removal nullifies an addition
            || ((old_event->m_EventType == EventType_Scene_Node_Removed) && (this->m_EventType == EventType_Scene_Node_Added)))
        {
            return MergeResult::DiscardBoth;
        }
        else if(old_event->m_EventType == this->m_EventType)
        {
            // Discard duplicate events
            return MergeResult::Merged_KeepOld;
        }
    }

    return MergeResult::NotMerged;
}

};//end of namespace NotificationAPI
};//end of namespace Max
