//**************************************************************************/
// Copyright (c) 2007 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
#pragma unmanaged
#include "DaylightSystemFactory.h"
#include "DaylightSystemFactory2.h"

// Interface instance and descriptor
DaylightSystemFactory* DaylightSystemFactory::sTheDaylightSystemFactory = nullptr;

INode* DaylightSystemFactory::Create(IDaylightSystem*& pDaylight)
{
	IDaylightSystem2* daylight2 = NULL;
	INode* newNode = DaylightSystemFactory2::GetInstance().Create(daylight2, NULL, NULL);
	if (newNode)
	{
		pDaylight = dynamic_cast<IDaylightSystem*>(daylight2->GetInterface(DAYLIGHT_SYSTEM_INTERFACE));
	}
	return newNode;
}

void DaylightSystemFactory::RegisterInstance()
{
	if(nullptr == sTheDaylightSystemFactory)
	{
		sTheDaylightSystemFactory = new DaylightSystemFactory
		(
			DAYLIGHTSYSTEM_FACTORY_INTERFACE,
			_T("DaylightSystemFactory"), // interface name used by maxscript - don't localize it!
			0, 
			NULL, 
			FP_CORE,

			p_end
		);
	}
}

void DaylightSystemFactory::Destroy()
{
	delete sTheDaylightSystemFactory;
	sTheDaylightSystemFactory = nullptr;
}
