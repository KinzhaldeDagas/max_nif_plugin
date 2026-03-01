#pragma once

/**********************************************************************
 *<
   FILE: MaterialSwitcher.h

   DESCRIPTION:  Material Switcher Header File

   CREATED BY: Mathew Kaustinen

   HISTORY: Created 4 August 2022

 *>   Copyright (c) 2022 Autodesk, Inc.
 **********************************************************************/

#pragma once

#include "max.h"
#include "imtl.h"
#include "texutil.h"
#include "buildver.h"
#include "AssetManagement\iassetmanager.h"
#include "resource.h"
#include <istdplug.h>
#include <maxtypes.h>
#include "stdmat.h"
#include "macrorec.h"
#include "gport.h"
#include "3dsmaxport.h"


#include <ifnpub.h>
#include "maxscript\maxscript.h"
#include "maxscript\util\Listener.h"

extern TCHAR *GetString(int id);

extern HINSTANCE hInstance;
