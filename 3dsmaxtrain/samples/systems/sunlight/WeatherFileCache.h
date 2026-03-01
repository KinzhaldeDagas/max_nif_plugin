//**************************************************************************/
// Copyright (c) 2007 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
/**********************************************************************
 *<
	FILE: weatherdata.h

	DESCRIPTION:	Weather Data UI and various strtucs

	CREATED BY: Michael Zyracki

	HISTORY: Created Nov 2007

 *>	Copyright (c) 2007, All Rights Reserved.
 **********************************************************************/
#pragma once

#include <list>
#include <vector>
#include <strclass.h>
#include <maxtypes.h>
#include "weatherdata.h"

// forward declarations
class ILoad;
class ISave;

class WeatherFileCache
{
public:
	WeatherFileCache();
	virtual ~WeatherFileCache();

	virtual void Clear();

	//non virtual functions 
	WeatherHeaderInfo ReadHeader(const TCHAR* filename);
	int ReadWeatherData(const TCHAR* filename); //returns the number of entries read

	TSTR mFileName;
	WeatherHeaderInfo mWeatherHeaderInfo;
	Tab<DaylightWeatherDataEntries*> mDataPeriods;
	int mNumEntries;

	BOOL ValidPerFrame(DaylightWeatherDataEntries::iterator iter,Int3 &MDY,
									 Int3 &HMS, WeatherCacheFilter::FramePer framePer);

	//used mainly by UI
	int GetRealNumEntries(WeatherUIData &data, BOOL useRangeAndPerFrame);
	//key function that finds iterator that points to that passed in time, based upon 
	//the passed in Month Day Year and Hour Minute Secs.  Note that 
	void GetValidIter(DaylightWeatherDataEntries::iterator &iter, int &currentPeriod, Int3 &MDY, Int3  &HMS,
		WeatherCacheFilter &filter,int overrideDir =0); //0 means don't override, dir is 0 match, 1 pos, -1, negative.

	void CheckIter(DaylightWeatherDataEntries::iterator &iter,int &currentPeriod, int dir,WeatherCacheFilter &filter);
	BOOL ValidIter(DaylightWeatherDataEntries::iterator iter,WeatherCacheFilter &filter);
	
	int FindDayOfWeek(int year, int month, int day);

	//this gets the 'real' entry based upon the skip flags and such.
	DaylightWeatherDataEntries::iterator GetFilteredNthEntry(int nth,WeatherCacheFilter &filter);
	int FindNthEntry(Int3 &MDY, Int3 &HMS,WeatherCacheFilter &filter);
	//if next >current returns 1, else -1 if next<current else 0 if the same
	int DateCompare(Int3 &currentMDY, Int3 &currentHMS, Int3 &nextMDY, Int3 &nextHMS);

	bool SubtractHour() const { return mSubtractHour; }
	void SubtractHour(bool val) { mSubtractHour = val; }

protected:
	//some files start at hour 1, some start at hour 0. We expect hour 0 to be the first hour.
	//so we check this.
	bool mSubtractHour;

};


class WeatherFileCacheFiltered : public WeatherFileCache
{
public:
	WeatherFileCacheFiltered(): WeatherFileCache(){};
	~WeatherFileCacheFiltered(){Clear();}
	void Clear();

	DaylightWeatherDataVector mDataVector;

	//filter data to to the vector
	int FilterEntries(WeatherUIData &data);
	//returns the nth based upon the time value.
	DaylightWeatherData GetNthEntry(TimeValue t) const;
};

