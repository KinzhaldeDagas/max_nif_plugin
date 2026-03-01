//**************************************************************************/
// Copyright (c) 2021 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
//
// FILE:        WaitingStatus.cpp
// DESCRIPTION: Helper object managing display of status information while
//              in "waiting status" during a calculation
// AUTHOR:      Lee Betchen
//
// HISTORY:	    October 2021 Creation
//**************************************************************************/

#include "WaitingStatus.h"
#include <GraphicsConstants.h>
#include <assert1.h>
#include <maxapi.h>
#include <mesh.h>

WaitingStatus::WaitingStatus():
    MaxSDK::Util::Noncopyable(),
    m_isMessageSet(false),
    m_isWaitingCursorSet(false),
    m_oldStatusMsg(_T("")),
    m_oldCursor(nullptr)
{
}

WaitingStatus::~WaitingStatus()
{

    // Revert the cursor and status bar, if we are presently in "waiting status"
    ClearWaitingStatus();
}

void WaitingStatus::SetWaitingStatus(const TCHAR* statusMsg)
{

    // Revert the cursor and status bar, if we were already in "waiting status;" this means that the "old" cursor and status
    // message seen below will be that before originally entering "waiting status," so that when we exit that status, we will
    // revert to that inital state
    ClearWaitingStatus();

    // Set the cursor to the standard Windows waiting cursor, storing a pointer to the cursor that was displayed on entry to
    // "waiting status"
    HCURSOR waitCursor = LoadCursor(nullptr, IDC_WAIT);
    m_oldCursor = SetCursor(waitCursor);
    m_isWaitingCursorSet = true;

    // If a status message was specified, display it in the Max status bar, storing a copy of the message displayed previously
    if (statusMsg != nullptr)
    {
        Interface10* coreInterface10 = GetCOREInterface10();
        DbgAssert(coreInterface10 != nullptr);
        m_oldStatusMsg = coreInterface10->GetPrompt();
        Interface* coreInterface = GetCOREInterface();
        DbgAssert(coreInterface != nullptr);
        coreInterface->ReplacePrompt(statusMsg);
        m_isMessageSet = true;
    }
}

void WaitingStatus::ClearWaitingStatus()
{

    // If we are presently displaying the Windows waiting cursor, revert to the displayed before we entered "waiting status"
    DbgAssert(m_isWaitingCursorSet || (m_oldCursor == nullptr));
    if (m_isWaitingCursorSet)
    {
        SetCursor(m_oldCursor);
        m_oldCursor = nullptr;
        m_isWaitingCursorSet = false;
    }

    // If we are presently displaying a status message, revert to the message displayed before we entered "waiting status"
    if (m_isMessageSet)
    {
        Interface* coreInterface = GetCOREInterface();
        DbgAssert(coreInterface != nullptr);
        coreInterface->ReplacePrompt(m_oldStatusMsg);
        m_oldStatusMsg = _T("");
        m_isMessageSet = false;
    }
}
