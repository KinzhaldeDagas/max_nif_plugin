//**************************************************************************/
// Copyright (c) 2007 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#pragma once

#include "weatherdata.h"
// forward declaration
class WeatherFileCache;

#pragma managed(push, on)

#using <WeatherData.dll>

WeatherHeaderInfo ConvertHeaderToStruct(WeatherData::WeatherHeader^ header);
DaylightWeatherData ConvertEntryToStruct(WeatherFileCache* cache, WeatherData::WeatherEntry^ entry, bool firstConversion);

#pragma managed(pop)

