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

#include "GenericEvent.h"

// src/include
#include "../NotificationAPIUtils.h"
// std includes
#include <time.h>

namespace Max{

namespace NotificationAPI{

//---------------------------------------------------------------------------
//----------			class GenericEvent							---------
//---------------------------------------------------------------------------

GenericEvent::GenericEvent(NotifierType notififerType, size_t updateType)
{
	m_NotififerType = notififerType;
	m_EventType	    = updateType;
}

void GenericEvent::DebugPrintToFile(FILE* file)const
{
    if (! file){
        DbgAssert(0 && _T("file is NULL"));
        return;
    }
    
    time_t ltime;
	time(&ltime);
    _ftprintf(file, _T("** GenericEvent parameters : %s**\n"), _tctime(&ltime) );

    size_t indent = 1;
    Utils::DebugPrintToFileNotifierType(file, m_NotififerType, NULL, indent); //No known notifier at this level
    Utils::DebugPrintToFileEventType(file, m_NotififerType, m_EventType, indent);

    TSTR indentString = _T("");
    for (size_t i=0;i<indent;++i){
        indentString += TSTR(_T("\t"));
    }
}

GenericEvent::MergeResult GenericEvent::merge_from(IMergeableEvent& generic_old_event)
{
    GenericEvent* const old_event = dynamic_cast<GenericEvent*>(&generic_old_event);
    if((old_event != nullptr) && (old_event->GetEventType() == GetEventType()))
    {
        return MergeResult::Merged_KeepNew;
    }
    else
    {
        return MergeResult::NotMerged;
    }
}


};//end of namespace NotificationAPI
};//end of namespace Max
