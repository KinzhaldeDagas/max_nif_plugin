//**************************************************************************/
// Copyright (c) 2007 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
/*===========================================================================*\
	FILE: suntypes.h

	DESCRIPTION: Some type definitions for the sunlight system.

	HISTORY: Adapted by John Hutchinson 10/08/97 
			

	Copyright (c) 1996, All Rights Reserved.
 \*==========================================================================*/

#pragma once

struct JulianCalendar { 
	double days; 
	int subday;
	long epoch;
}; 

struct uTimevect { 
	unsigned short i; 
	unsigned short j;
	unsigned short k;
}; 


