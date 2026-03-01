//**************************************************************************/
// Copyright (c) 2009 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#pragma once

#include <iparamb2.h>
#include <Noncopyable.h>
// forward declarations
class Class_ID;
class Interface;
class Object;

class NatLightAssemblyClassDesc : public ClassDesc2, MaxSDK::Util::Noncopyable
{
public:
	int IsPublic();
	void* Create(BOOL loading = FALSE);
	const TCHAR* ClassName();
	const TCHAR* NonLocalizedClassName();
	SClass_ID SuperClassID();
	Class_ID ClassID();

	const TCHAR* InternalName();
	HINSTANCE HInstance();	// returns owning module handle

	// The driven controllers don't appear in any of the drop down lists, 
	// so they just return a null string.
	const TCHAR* Category();

	void ResetClassParams(BOOL fileReset);

	Object* CreateSun(Interface* ip, const Class_ID* clsID = NULL);
	Object* CreateSky(Interface* ip, const Class_ID* clsID = NULL);

	static bool IsValidSun(ClassDesc& cd);
	static bool IsValidSky(ClassDesc& cd);
	
	static NatLightAssemblyClassDesc* GetInstance();
	static void Destroy();
	
private:
	NatLightAssemblyClassDesc();
	virtual ~NatLightAssemblyClassDesc();
	
	bool IsPublicClass(const Class_ID& id);

	Class_ID mSunID;
	Class_ID mSkyID;
	bool mSunValid;
	bool mSkyValid;
	
	static NatLightAssemblyClassDesc* sInstance;
};

