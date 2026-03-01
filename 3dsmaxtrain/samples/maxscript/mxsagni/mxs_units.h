//**************************************************************************/
// Copyright (c) 2008 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
// DESCRIPTION: Maxscript code for accessing system units.
// AUTHOR: Larry Minton
// DATE: 
// August 7 2008, factored out of avg_DLX.cpp to this file. Chris Johnson
//**************************************************************************/
#pragma once

#include <maxscript/maxscript.h>

//  =================================================
//  MAX Unit methods
//  =================================================
//  Maxscript syntax:
/* 
	units.DisplayType
	units.SystemType
	units.SystemScale
	units.MetricType
	units.USType
	units.USFrac
	units.CustomName
	units.CustomValue
	units.CustomUnit
*/

Value* formatValue_cf(Value** arg_list, int count);
Value* decodeValue_cf(Value** arg_list, int count);
Value* getUnitDisplayType();
Value* setUnitDisplayType(Value* val);

Value* getUnitSystemType();
Value* setUnitSystemType(Value* val);

Value* getUnitSystemScale();
Value* setUnitSystemScale(Value* val);

Value* getMetricDisplay();
Value* setMetricDisplay(Value* val);

Value* getUSDisplay();
Value* setUSDisplay(Value* val);

Value* getUSFrac();
Value* setUSFrac(Value* val);

Value* getCustomName();
Value* setCustomName(Value* val);

Value* getCustomValue();
Value* setCustomValue(Value* val);

Value* getCustomUnit();
Value* setCustomUnit(Value* val);