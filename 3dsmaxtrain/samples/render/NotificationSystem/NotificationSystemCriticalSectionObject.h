#pragma once

//**************************************************************************/
// Copyright (c) 2017 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
// AUTHOR: Tristan Lapalme
//***************************************************************************/

namespace Max
{;
namespace NotificationAPI
{;

class NotificationSystemCriticalSectionObject {

public:

	NotificationSystemCriticalSectionObject();
	~NotificationSystemCriticalSectionObject();

	void Enter();
	void Leave();

private:

	CRITICAL_SECTION m_criticalSection;
};

inline NotificationSystemCriticalSectionObject::NotificationSystemCriticalSectionObject() {

	InitializeCriticalSection(&m_criticalSection);
}

inline NotificationSystemCriticalSectionObject::~NotificationSystemCriticalSectionObject() {

	DeleteCriticalSection(&m_criticalSection);
}

inline void NotificationSystemCriticalSectionObject::Enter() {

	EnterCriticalSection(&m_criticalSection);
}

inline void NotificationSystemCriticalSectionObject::Leave() {

	LeaveCriticalSection(&m_criticalSection);
}

class NotificationSystemAutoCriticalSection
{
public:

	// The constructor acquires the critical section object
	NotificationSystemAutoCriticalSection(NotificationSystemCriticalSectionObject& obj);
	// The destructor releases the critical section object
	~NotificationSystemAutoCriticalSection();

	// Enables releasing a critical section prematurely
	void Release();

	// Acquires a new critical section (after releasing the previous one)
	void Acquire(NotificationSystemCriticalSectionObject& obj);

private:
	// disable copy constructor and assignment operator
	NotificationSystemAutoCriticalSection(const NotificationSystemAutoCriticalSection&);
	NotificationSystemAutoCriticalSection& operator=(const NotificationSystemAutoCriticalSection&);

	NotificationSystemCriticalSectionObject* mObj;
};

inline NotificationSystemAutoCriticalSection::NotificationSystemAutoCriticalSection(NotificationSystemCriticalSectionObject& obj)
	: mObj(&obj)
{
	mObj->Enter();
}

inline NotificationSystemAutoCriticalSection::~NotificationSystemAutoCriticalSection()
{
	if (mObj != nullptr)
	{
		mObj->Leave();
	}
}

inline void NotificationSystemAutoCriticalSection::Release()
{
	if (mObj != nullptr)
	{
		mObj->Leave();
		mObj = nullptr;
	}
}

inline void NotificationSystemAutoCriticalSection::Acquire(NotificationSystemCriticalSectionObject& obj)
{
	Release();
	mObj = &obj;
	mObj->Enter();
}

};//end of namespace NotificationAPI
};//end of namespace Max