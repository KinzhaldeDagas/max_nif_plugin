//**************************************************************************/
// Copyright (c) 2007 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
#pragma once

#include <Noncopyable.h>
#include <IDaylightSystem.h>

class DaylightSystemFactory : public IDaylightSystemFactory, public MaxSDK::Util::Noncopyable 
{
public:

	#pragma warning(push)
	#pragma warning(disable:4793)
	DECLARE_DESCRIPTOR_NDC(DaylightSystemFactory); 
	#pragma warning(pop)

	INode* Create(IDaylightSystem*& pDaylight);
	
	static void RegisterInstance();
	static void Destroy();

private:
	// The single instance of this class
	static DaylightSystemFactory* sTheDaylightSystemFactory;
};


