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

#include <ref.h>
#include <Geom/matrix3.h>
#include <NotificationAPI/NotificationAPI_Events.h>

#ifdef NOTIFICATION_SYSTEM_MODULE
    #define NotificationAPIExport __declspec(dllexport)
#else
    #define NotificationAPIExport __declspec(dllimport)
#endif

class ViewExp;

namespace Max
{
namespace NotificationAPI
{
namespace Utils
{
using namespace MaxSDK::NotificationAPI;

    void                        DebugPrintToFileEventType       (FILE* file, NotifierType notifierType, size_t updateType, size_t indent);
    void                        DebugPrintToFileNotifierType    (FILE* theFile, NotifierType type, RefTargetHandle, size_t indent);
    void                        DebugPrintToFileMonitoredEvents (FILE* file,    NotifierType notifierType, size_t monitoredEvents, size_t indent);
    INode*                      GetViewNode                     (ViewExp &viewportExp);
    INode*                      GetViewNode                     (ViewExp *viewportExp);
    void                        DebugPrintParamBlockData        (FILE* file, const ParamBlockData& pblockData, size_t indent);
    //GetLastUpdatedParamFromObject function
    //result is set in outResult, it is true if there were one parameter which was updated. false if none of the parameters was updated (when false, all ParamBlockData arrays are empty and type == UNKNOWN)
    //referenceTarget may be coming from a INode* , Mtl* , Texmpa* of whatever, 
    //outParamblockData is going to be filled by the function when outResult is true
    void                        GetLastUpdatedParamFromObject   (bool& outResult, ReferenceTarget& referenceTarget, ParamBlockData& outParamblockData);
    //Merge content of otherParamBlockData into inoutParamBlockData
    void                        MergeParamBlockDatas            (MaxSDK::Array<ParamBlockData>& inoutParamBlockDatas, const MaxSDK::Array<ParamBlockData>& otherParamBlockDatas); 

    //Get FOV from ViewExp taking into account safe frame if it is on
    NotificationAPIExport  float GetFOVFromViewExpUsingSafeFrame(float currentFOV, ViewExp& viewExp);

    //Get Zoom from ViewExp taking into account safe frame if it is on
    NotificationAPIExport  float GetOrthoZoomFromViewExpUsingSafeFrame(float currentZoom, ViewExp& viewExp);
};//end of namespace Utils
};//end of namespace NotificationAPI
};//end of namespace Max
