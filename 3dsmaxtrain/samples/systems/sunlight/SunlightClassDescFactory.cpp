//**************************************************************************/
// Copyright (c) 2009 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#pragma unmanaged

#include "SunlightClassDescFactory.h"
#include "SunDriverClassDesc.h"
#include "DayDriverClassDesc.h"
#include "DrivenControlPosClassDesc.h"
#include "DrivenControlFloatClassDesc.h"
#include "DayDrivenControlPosClassDesc.h"
#include "DayDrivenControlFloatClassDesc.h"
#include "CompassRoseObjClassDesc.h"
#include "NatLightAssemblyClassDesc.h"
#include "SunPositioner.h"
#include "PhysicalSunSkyEnv.h"

#pragma managed

using namespace ManagedServices;
using namespace System::Collections::Generic;


namespace SunlightSystem
{


IEnumerable<ClassDescWrapper^>^ SunlightClassDescFactory::CreateClassDescs()
{
	return gcnew cli::array<ClassDescWrapper^> {
		gcnew ClassDescOwner(new SunDriverClassDesc()),
		gcnew ClassDescOwner(new DayDriverClassDesc()),
		gcnew ClassDescOwner(new DrivenControlMatrix3ClassDesc()),
		gcnew ClassDescOwner(new LegacyDrivenControlPosClassDesc()),
		gcnew ClassDescOwner(new DrivenControlFloatClassDesc()),
		gcnew ClassDescOwner(new DayDrivenControlMatrix3ClassDesc()),
		gcnew ClassDescOwner(new LegacyDayDrivenControlPosClassDesc()),
		gcnew ClassDescOwner(new DayDrivenControlFloatClassDesc()),
		gcnew ClassDescOwner(new CompassRoseObjClassDesc()),
        gcnew ClassDescReference(NatLightAssemblyClassDesc::GetInstance()),
        gcnew ClassDescReference(&SunPositionerObject::get_class_descriptor()),
        gcnew ClassDescReference(&PhysicalSunSkyEnv::get_class_descriptor())
	};
}


}

