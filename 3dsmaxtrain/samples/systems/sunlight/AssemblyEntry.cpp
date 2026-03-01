//**************************************************************************/
// Copyright (c) 2009 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#pragma unmanaged

#include "AssemblyEntry.h"
#include "DaylightSystemFactory.h"
#include "DaylightSystemFactory2.h"
#include "natLight.h"
#include "NatLightAssemblyClassDesc.h"
#include "SunDriverCreateMode.h"
#include "CityList.h"

#pragma managed

namespace SunlightSystem
{

void AssemblyEntry::AssemblyMain()
{
	NatLightAssembly::InitializeStaticObjects();
	DaylightSystemFactory::RegisterInstance();
	DaylightSystemFactory2::RegisterInstance();
}

void AssemblyEntry::AssemblyShutdown()
{
	SunDriverCreateMode::Destroy();
	DaylightSystemFactory2::Destroy();
	DaylightSystemFactory::Destroy();
	NatLightAssembly::DestroyStaticObjects();
	NatLightAssemblyClassDesc::Destroy();
	CityList::Destroy();
}

}
