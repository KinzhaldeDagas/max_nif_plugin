// maxclient_pch.h: Pre-compiled header for maxclient

#pragma once

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include "../../../../include/MaxWindowsVersion.h"

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <atlbase.h>
extern CComModule _Module;
#pragma warning(push)
#pragma warning(disable : 4265)
#include <atlcom.h>
#pragma warning(pop)

#import "comsrv.tlb"  no_namespace named_guids exclude("_SYSTEMTIME") // From 3dswin/src/maxsdk/samples/gup/comsrv/comsrvgup.vcxproj
