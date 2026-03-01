//**************************************************************************/
// Copyright (c) 2021 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
//
// FILE:        WaitingStatus.h
// DESCRIPTION: Header information for a helper object managing display of
//              status information while in "waiting status" during a
//              calculation
// AUTHOR:      Lee Betchen
//
// HISTORY:	    October 2021 Creation
//**************************************************************************/

#pragma once

#include <Noncopyable.h>
#include <strclass.h>
#include <Windows.h>

// Class WaitingStatus is intended to be used during lengthy computations, to manage display of a status message in the Max
// status bar, along with the Windows waiting cursor. In particular, calling SetWaitingStatus enters "waiting status," in which
// the waiting cursor is displayed, optionally along with a status message. Note that if the destructor of an instance of this
// class is called while in "waiting status," the cursor and status bar are reverted to their states before modification by the
// clas instance.
class WaitingStatus: public MaxSDK::Util::Noncopyable
{

// Public member functions:
public:

    // Constructor
    WaitingStatus();

    // Destructor. As noted in the class description, if the destructor is called while the class instance is in "waiting
    // status," the cursor and status bar are reverted, as if ClearWaitingStatus was called.
    virtual ~WaitingStatus();

    // Enters "waiting status," setting the displayed cursor to the Windows waiting cursor and, if statusMsg is not null,
    // displaying the specified status message in the Max status bar. Note that if this function is called while already in
    // "waiting status," ClearWaitingStatus is called internally, before setting any new status message.
    //
    // statusMsg: specifies the status message to be display in the Max status bar, if statusMsg is not null; input
    void SetWaitingStatus(const TCHAR* statusMsg = nullptr);

    // Exits "waiting status," reverting the cursor to the that which was set before entering "waiting status." If this object
    // was set to display a status message on the last call to SetWaitingStatus, the message in the status bar is reverted to
    // that which was displayed before entering "waiting status."
    void ClearWaitingStatus();

// Private variables:
private:

    // Flag indicating whether this object is presently displaying a status message in the Max status bar.
    bool m_isMessageSet;

    // Flag indicating whether this object is presently displaying the Windows waiting cursor.
    bool m_isWaitingCursorSet;

    // If m_isMessageSet is true, m_oldStatusMsg is the message that was displayed in the status bar before entering "waiting
    // status," otherwise, an empty string.
    TSTR m_oldStatusMsg;

    // If m_isWaitingCursorSet is true, m_oldCursor is a pointer to the cursor that was displayed before entering "waiting
    // status," otherwise, m_oldCursor = nullptr.
    HCURSOR m_oldCursor;
};
