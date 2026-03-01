//**************************************************************************/
// Copyright (c) 2020 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
//
// FILE:        EditPolyModContextEnumProc.cpp
// DESCRIPTION: ModContext enumeration helper class for the EditPolyMod
//              modifier
// AUTHOR:      Lee Betchen
//
// HISTORY:     November 2020 Creation
//**************************************************************************/

#include "EditPolyModContextEnumProc.h"
#include "EditPoly.h"

EditPolyModContextEnumProc::EditPolyModContextEnumProc(EditPolyMod* owner):
    ModContextEnumProc(),
    m_owner(owner)
{
    DbgAssert(m_owner != nullptr);
}

EditPolyModContextEnumProc::~EditPolyModContextEnumProc()
{
}

BOOL EditPolyModContextEnumProc::proc(ModContext* mc)
{

    // We want only ModContext entities representing object to which modifier m_owner is presently applied, but we may possibly
    // receive a ModContext mc corresponding to a deleted node that m_owner was formerly applied to; add mc to our list only if
    // it corresponds to a current scene node
    int indexNode = 0;
    INode* node = m_owner->GetNodeFromModContext(mc, indexNode);
    if (node != nullptr)
    {
        m_contexts.Append(1, &mc, 10);
        m_contextNodes.Append(1, &node, 10);
    }

    // Return flag indicating that we should continue to process any remaining ModContext entities
    return TRUE;
}
