//**************************************************************************/
// Copyright (c) 2013 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
//Author : David Lanier

#pragma once

// Max SDK
#include <Noncopyable.h>
#include <assert1.h>
// STD
#include <stdio.h>

class INode;

namespace Max
{;
namespace NotificationAPI
{;

//Events monitored for camera or light viewports
#define IR_CAM_OR_LIGHT_MONITORED_EVENTS  Max::NotificationAPI::EventType_Node_ParamBlock |\
		                                  Max::NotificationAPI::EventType_Node_Transform  |\
		                                  Max::NotificationAPI::EventType_Node_Deleted

//Events monitored for viewports (non camera or lights)
#define IR_VIEWPORT_MONITORED_EVENTS Max::NotificationAPI::EventType_View_Properties  | Max::NotificationAPI::EventType_View_Active

class InteractiveRenderingViewData  : public MaxSDK::Util::Noncopyable
{
protected:
    int         m_ViewID;
    INode*      m_CameraOrLightNode;//May be NULL if we are monitoring a viewport

public:
    InteractiveRenderingViewData();//default constructor
    InteractiveRenderingViewData(int viewId, INode* pCameraOrLightNode);//!< Constructor
    virtual ~InteractiveRenderingViewData();//!< Destructor
    
    //Get 
    virtual int     GetViewID                       ()const{return m_ViewID;}
    virtual INode*  GetCameraOrLightNode            ()const{return m_CameraOrLightNode;}
    virtual bool    IsAViewport                     ()const{DbgAssert(m_ViewID >= 0); return NULL == m_CameraOrLightNode;}//Should be initialized
    
    //Set
    virtual void    UpdateViewAndNode   (int viewId, INode* pCameraOrLightNode);//When we change the monitored view to another view, we update the data

    virtual void DebugPrintToFile(FILE* file)const;
};


}}; //end of namespace 