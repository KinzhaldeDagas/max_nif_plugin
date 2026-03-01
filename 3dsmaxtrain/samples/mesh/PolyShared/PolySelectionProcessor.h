//**************************************************************************/
// Copyright (c) 2021 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
//
// FILE:        PolySelectionProcessor.h
// DESCRIPTION: Template selection processor class used to manage mouse
//              interaction for selection operations in MNMesh-based
//              entities such as Edit Poly, Editable Poly, and Poly Select
// AUTHOR:      Lee Betchen
//
// HISTORY:	    November 2021 Creation
//**************************************************************************/

#pragma once

#include <objmode.h>
#include <mouseman.h>
#include <maxapi.h>
#include <assert1.h>
#include <max.h>

// Class PolySelectionProcessor is a selection processor, derived from BaseSelectionProcessor, which may be used in place of
// that class for processing selection operations, or operations for a particular mode, e.g. move mode, for an MNMesh-based
// entity of type PolyType (generally an Editable Poly object, or Edit Poly or Poly Select modifier). All processing is simply
// passed through to the BaseSelectionProcessor class,  with the exception that in the case where we may be processing a point
// selection operation, that is, when the function proc is called with msg = MOUSE_POINT or msg = MOUSE_DBLCLICK, calls to
// object->BeginPointSelectionHitTesting() and object->EndPointSelectionHitTesting() are performed before and after,
// respectively, the call to BaseSelectionProcess::proc.
template <typename BaseSelectionProcessor, typename PolyType>
class PolySelectionProcessor: public BaseSelectionProcessor
{

// Public member functions:
public:

    // Constructor. Note that all passed pointers must be non-null.
    //
    // transMode: object implementing transformation for geometric subobjects of object object in the relevant mode, e.g.
    //            translation for move mode; input
    // object:    the object that we are to operate upon; input
    // objParam:  entity; input
    PolySelectionProcessor(TransformModBox* transMode, BaseObject* object, IObjParam* objParam);

    // Destructor.
    virtual ~PolySelectionProcessor();

    // Virtual functions inherited from MouseCallBack; see mouseman.h.
    virtual int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m) override;

// Private member functions
private:

    // Default constructor; hidden to force specification of required parameters.
    PolySelectionProcessor() = delete;

    // Copy and move constructors; should never be called.
    //
    // selectionProc: a selection processor object; input
    PolySelectionProcessor(const PolySelectionProcessor& selectionProc) = delete;
    PolySelectionProcessor(const PolySelectionProcessor&& selectionProc) = delete;

    // Copy and move assignment operators; should never be called.
    //
    // selectionProc: a selection processor object; input
    PolySelectionProcessor& operator=(const PolySelectionProcessor& selectionProc) = delete;
    PolySelectionProcessor& operator=(const PolySelectionProcessor&& selectionProc) = delete;
};

template <typename BaseSelectionProcessor, typename PolyType>
PolySelectionProcessor<BaseSelectionProcessor, PolyType>::PolySelectionProcessor(TransformModBox* transMode, BaseObject* object, IObjParam* objParam):
    BaseSelectionProcessor(transMode, object, objParam)
{
}

template <typename BaseSelectionProcessor, typename PolyType>
PolySelectionProcessor<BaseSelectionProcessor, PolyType>::~PolySelectionProcessor()
{
}

template <typename BaseSelectionProcessor, typename PolyType>
int PolySelectionProcessor<BaseSelectionProcessor, PolyType>::proc(HWND hwnd, int msg, int point, int flags, IPoint2 m)
{
    PolyType* polyObject = static_cast<PolyType*>(this->obj);
    DbgAssert(polyObject != nullptr);

    // Notify object polyObject on MOUSE_POINT or MOUSE_DBLCLICK messages that we may be processing a point selection operation
    bool isPossiblePointSelection = false;
    switch (msg)
    {
    case MOUSE_POINT:
    case MOUSE_DBLCLICK:
        polyObject->BeginPointSelectionHitTesting();
        isPossiblePointSelection = true;
        break;
    default:
        break;
    }

    // Call the base class variant to perform processing
    BOOL hasSucceeded = BaseSelectionProcessor::proc(hwnd, msg, point, flags, m);

    // If we notified objecy PolyObject of a possible point selection operation above, send a notification that the operation
    // has finished processing
    if (isPossiblePointSelection)
    {
        polyObject->EndPointSelectionHitTesting();
    }

    // Return a flag indicating the success status of the operation
    return hasSucceeded;
}
