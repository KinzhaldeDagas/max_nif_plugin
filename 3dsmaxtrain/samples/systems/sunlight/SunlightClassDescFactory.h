//**************************************************************************/
// Copyright (c) 2009 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#pragma once

#pragma managed(push, on)

#using <ManagedServices.dll>

namespace SunlightSystem
{

public ref class SunlightClassDescFactory : ManagedServices::ClassDescFactory
{
public:
	virtual System::Collections::Generic::IEnumerable<ManagedServices::ClassDescWrapper^>^ CreateClassDescs();
};


}

#pragma managed(pop)
