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
#include "TexmapEvent.h"

// src/include
#include "../NotificationAPIUtils.h"
// Max sdk
#include <imtl.h>
// std includes
#include <time.h>


namespace Max{

namespace NotificationAPI{

//---------------------------------------------------------------------------
//----------			class TexmapEvent							---------
//---------------------------------------------------------------------------

TexmapEvent::TexmapEvent(TexmapEventType updateType, Texmap* pTexmap, const ParamBlockData* paramblockData /*= NULL*/)
{
	m_EventType	                = updateType;
	DbgAssert(NULL != pTexmap);
	m_pTexmap		            = pTexmap;
    m_ParamsChangedInPBlock.removeAll();
    if (NULL != paramblockData){
        m_ParamsChangedInPBlock.append(*paramblockData);
    }
}

void TexmapEvent::DebugPrintToFile(FILE* file)const
{
    if (! file){
        DbgAssert(0 && _T("file is NULL"));
        return;
    }
    
    time_t ltime;
	time(&ltime);

    _ftprintf(file, _T("\n** TexmapEvent parameters : %s **\n"), _tctime(&ltime) );

    size_t indent = 1;
    Utils::DebugPrintToFileNotifierType(file, GetNotifierType(), m_pTexmap, indent);
    Utils::DebugPrintToFileEventType(file, GetNotifierType(), m_EventType, indent);

    TSTR indentString = _T("");
    for (size_t i=0;i<indent;++i){
        indentString += TSTR(_T("\t"));
    }

    const size_t NumParams = m_ParamsChangedInPBlock.length();
    _ftprintf(file, indentString);
    _ftprintf(file, _T("** Number of Param IDs changed from pblock : %llu **\n"), NumParams);
    TSTR indentStringPlus = indentString + TSTR(_T("\t"));
    for (size_t iParam=0;iParam<NumParams;++iParam){
        _ftprintf(file, indentString);
        _ftprintf(file, _T("** ParamblockData #%llu : **\n"), iParam);
        Utils::DebugPrintParamBlockData(file, m_ParamsChangedInPBlock[iParam], indent+1);
    }
}

TexmapEvent::MergeResult TexmapEvent::merge_from(IMergeableEvent& generic_old_event)
{
    // Check if both events refer to the same node
    TexmapEvent* old_event = dynamic_cast<TexmapEvent*>(&generic_old_event);
    if((old_event != nullptr) && (GetTexmap() == old_event->GetTexmap()) && (GetEventType() == old_event->GetEventType()))
    {
        if(GetEventType() == EventType_Texmap_ParamBlock)
        {
            // Merge all param block change data into the old/existing event
            Utils::MergeParamBlockDatas(old_event->GetParamBlockData(), GetParamBlockData());

            // Keep the old/existing event
            return MergeResult::Merged_KeepOld;
        }
        else
        {
            //Same node and same update type, so only store the last update.
            return MergeResult::Merged_KeepNew;
        }
    }

    return MergeResult::NotMerged;
}

};//end of namespace NotificationAPI
};//end of namespace Max
