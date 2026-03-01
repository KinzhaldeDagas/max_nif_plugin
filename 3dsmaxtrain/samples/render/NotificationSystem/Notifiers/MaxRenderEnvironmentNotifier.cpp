//**************************************************************************/
// Copyright (c) 2013 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#include "MaxRenderEnvironmentNotifier.h"

#include "Events/GenericEvent.h"

namespace Max
{;
namespace NotificationAPI
{;
using namespace MaxSDK::NotificationAPI;


MaxRenderEnvironmentNotifier::MaxRenderEnvironmentNotifier()
{
	
}

MaxRenderEnvironmentNotifier::~MaxRenderEnvironmentNotifier()
{
}

void MaxRenderEnvironmentNotifier::NotifyEvent_BackgroundColor()
{
    NotifyEvent(GenericEvent(NotifierType_RenderEnvironment, EventType_RenderEnvironment_BackgroundColor));
}

void MaxRenderEnvironmentNotifier::NotifyEvent_EnvironmentMap()
{
    NotifyEvent(GenericEvent(NotifierType_RenderEnvironment, EventType_RenderEnvironment_EnvironmentMap));
}

void MaxRenderEnvironmentNotifier::NotifyEvent_EnvironmentMapState()
{
    NotifyEvent(GenericEvent(NotifierType_RenderEnvironment, EventType_RenderEnvironment_EnvironmentMapState));
}

void MaxRenderEnvironmentNotifier::DebugPrintToFile(FILE* file, size_t indent)const
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
    _ftprintf(file, _T("** Render Environment Notifier data : **\n"));
    
    //Print base class
   __super::DebugPrintToFile(file, ++indent);
}

} } // namespaces
