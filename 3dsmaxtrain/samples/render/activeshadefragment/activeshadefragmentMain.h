//**************************************************************************/
// Copyright (c) 2014 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
/**********************************************************************
	DESCRIPTION: Hearder for activeshadefragment DLL
**********************************************************************/

#ifndef __activeshadefragment__H
#define __activeshadefragment__H

//#ifdef BLD_activeshadefragment
//#define activeshadefragmentExport __declspec( dllexport )
//#else
//#define activeshadefragmentExport __declspec( dllimport )
//#endif

#include "max.h"
#include "iparamb2.h"

extern ClassDesc2* GetActiveShadeFragmentDesc();

#endif
