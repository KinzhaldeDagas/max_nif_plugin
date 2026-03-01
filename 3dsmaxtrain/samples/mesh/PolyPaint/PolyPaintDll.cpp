//**************************************************************************/
// Copyright (c) 2010 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

// include only after you've included your dll's resource.h
#include <WindowsDefines.h>
#include <plugapi.h>
#include "PolyPaint.h"

BOOL WINAPI DllMain(HINSTANCE hinstDLL, ULONG fdwReason, LPVOID) 
{
	if (DLL_PROCESS_ATTACH == fdwReason)
	{
		MaxSDK::Util::UseLanguagePackLocale();
		DisableThreadLibraryCalls(hinstDLL);
	}
	else if (DLL_PROCESS_DETACH == fdwReason)
	{
		MeshPaintMgr::DestroyInstance();
	}
	return(TRUE);
}



