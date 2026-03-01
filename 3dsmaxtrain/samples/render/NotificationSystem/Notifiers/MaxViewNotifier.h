//**************************************************************************/
// Copyright (c) 2013 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#pragma once

#include "Notifiers/MaxNotifier.h"
#include <maxapi.h>

namespace Max
{;
namespace NotificationAPI
{;
using namespace MaxSDK::NotificationAPI;


    //Used to get notified when the viewport TM changes
	class MaxViewNotifier : public MaxNotifier, public RedrawViewsCallback 
	{
	public:
		MaxViewNotifier(int viewID);

        int GetMaxViewID()const;

        //From MaxNotifier
        virtual void DebugPrintToFile(FILE* f, size_t indent)const;

        //From RedrawViewsCallback
		virtual void proc(Interface *ip);

        void NotifyEvent_NodesFiltered(void);

        static void ProcessNotifications(void *param, NotifyInfo *info);

    protected:
        // Protected destructor forces going through delete_this()
        virtual ~MaxViewNotifier();

    protected:
        int     				m_ViewUndoID;
        INode*                  m_ViewNode;//Used to check if we have switched within the same view from a viewport to a camera/light or the other way round
        TSTR                    m_CurrentViewportLabel;//Used to track any changes of view inside the same viewport, say from perspective to Left by using the keyboard shortcuts

        // Set of properties which are monitored for change
        Matrix3                 m_StoredMatrix;
        BOOL m_view_persp;
        float m_view_fov;
        float m_view_focal_dist;

    private:

        virtual void UpdateOurStoredData(ViewExp& pViewExp);

	};


} } // namespaces
