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

namespace Max
{;
namespace NotificationAPI
{;
using namespace MaxSDK::NotificationAPI;


	class MaxRenderEnvironmentNotifier : public MaxNotifier
	{
	public:
		MaxRenderEnvironmentNotifier();

        void NotifyEvent_BackgroundColor();
        void NotifyEvent_EnvironmentMap();
        void NotifyEvent_EnvironmentMapState();

        //From MaxNotifier
        virtual void DebugPrintToFile(FILE* f, size_t indent)const;

    protected:
        // Protected destructor forces going through delete_this()
        virtual ~MaxRenderEnvironmentNotifier();

	};


} } // namespaces
