#pragma once

//**************************************************************************/
// Copyright (c) 2020 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
//
// FILE:        EditPolyModContextEnumProc.h
// DESCRIPTION: Header information for the EditPolyMod modifier's
//              ModContext enumeration helper class
// AUTHOR:      Lee Betchen
//
// HISTORY:     November 2020 Creation
//**************************************************************************/

#include <object.h>
#include <tab.h>
#include <inode.h>

class EditPolyMod;

// Helper class to fetch ModContext entities for each object to which the owning EditPolyMod modifier is applied.
class EditPolyModContextEnumProc: public ModContextEnumProc
{

// Public member functions:
public:

    // Constructor.
    //
    // owner: EditPolyMod modifier upon which this object operates; input
    EditPolyModContextEnumProc(EditPolyMod* owner);

    // Destructor.
    virtual ~EditPolyModContextEnumProc();

    // Returns the number of objects to which modifier owner is presently applied.
    inline int GetNumberOfObjects() const
    {
        int nObjects = m_contexts.Count();
        DbgAssert((m_contextNodes.Count()) == nObjects);
        return nObjects;
    }

    // Returns the ModContext and corresponding scene node for the specified object to which modifier owner is presently
    // applied.
    //
    // context:     ModContext entity corresponding to the specified object; output
    // node:        scene node corresponding to the specified object; output
    // indexObject: index, on the range [0, GetNumberOfObjects()), of the object for which to fetch information; input
    inline void GetInformationForObject(ModContext*& context, INode*& node, int indexObject) const
    {
        DbgAssert((indexObject >= 0) && (indexObject < (GetNumberOfObjects())));
        context = m_contexts[indexObject];
        node = m_contextNodes[indexObject];
    }

    // Virtual function inherited from ModContextEnumProc; see object.h.
    virtual BOOL proc(ModContext* mc) override;

// Private variables:
private:

    // The EditPolyMod modifier upon which this object operates.
    EditPolyMod* m_owner;

    // Table of pointers to each ModContext entity.
    Tab<ModContext*> m_contexts;

    // Entry m_contextNodes[obj] is the scene node corresponding to ModContext m_contexts[obj].
    Tab<INode*> m_contextNodes;

// Private member functions:
private:

    // Default constructor; hidden to force specification of required parameters.
    EditPolyModContextEnumProc() = delete;

    // Copy and move constructors; should never be called.
    //
    // editPolyEnumProc: a ModContext enumeration object; input
    EditPolyModContextEnumProc(const EditPolyModContextEnumProc& editPolyEnumProc) = delete;
    EditPolyModContextEnumProc(const EditPolyModContextEnumProc&& editPolyEnumProc) = delete;

    // Assignment operators; should never be called.
    //
    // editPolyEnumProc: a ModContext enumeration object; input
    EditPolyModContextEnumProc& operator=(const EditPolyModContextEnumProc& editPolyEnumProc) = delete;
    EditPolyModContextEnumProc& operator=(const EditPolyModContextEnumProc&& editPolyEnumProc) = delete;
};
