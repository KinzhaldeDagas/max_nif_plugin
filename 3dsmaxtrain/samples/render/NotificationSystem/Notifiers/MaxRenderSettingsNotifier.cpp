//**************************************************************************/
// Copyright (c) 2013 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#include "MaxRenderSettingsNotifier.h"

#include "Events/GenericEvent.h"

namespace Max
{;
namespace NotificationAPI
{;
using namespace MaxSDK::NotificationAPI;


MaxRenderSettingsNotifier::MaxRenderSettingsNotifier()
{
	
}

MaxRenderSettingsNotifier::~MaxRenderSettingsNotifier()
{
}

void MaxRenderSettingsNotifier::NotifyEvent_LockView()
{
    NotifyEvent(GenericEvent(NotifierType_RenderSettings, EventType_RenderSettings_LockView));
}

void MaxRenderSettingsNotifier::DebugPrintToFile(FILE* file, size_t indent)const
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
    _ftprintf(file, _T("** Render Settings Notifier data : **\n"));
    
    //Print base class
   __super::DebugPrintToFile(file, ++indent);
}

} } // namespaces
