//**************************************************************************/
// Copyright (c) 2021 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
//
// FILE:        PolyModeRightClickProcessor.h
// DESCRIPTION: Template right click processor class used to implement
//              clean right click cancel operations for the
//              PolyModeSelectionProcessor class
// AUTHOR:      Lee Betchen
//
// HISTORY:	    April 2021 Creation
//**************************************************************************/

#pragma once

#include <objmode.h>
#include <mouseman.h>
#include <maxapi.h>
#include <assert1.h>

// Class PolyModeRightClickProcessor is a right click processor designed to work in tandem with an owning
// PolyModeSelectionProcessor leftClickProc processing left clicks, in order to support clean right click cancellation of the
// actions implemented by PolyModeSelectionProcessor. When leftClickProc receives the MOUSE_INIT/MOUSE_UNINIT signal, it should
// call Activate/Deactivate on behalf of its owned instance of this class. Then, any right click signal received while a
// defined operation is underway in leftClickProc, that is, while leftClickProc->GetCurrentAction() returns a value
// other than PolyModeSelectionProcessor::N_POLY_ACTIONS, will trigger a call to PolyModeSelectionProcessor::AbortOperation on
// behalf of leftClickProc. Any other call be based through to the "base" right click processor defined in the MouseManager at
// the time that Activate() is called. Note that PolyModeSelectionProcessorType should refer to some specialization of the
// PolyModeSelectionProcessor template class.
template <typename PolyModeSelectionProcessorType>
class PolyModeRightClickProcessor: public MouseCallBack
{

// Public member functions:
public:

    // Constructor.
    //
    // leftClickProc: pointer to the PolyModeSelectionProcessor entity which owns this object; input
    PolyModeRightClickProcessor(PolyModeSelectionProcessorType* leftClickProc);

    // Destructor.
    virtual ~PolyModeRightClickProcessor();

    // Sets the MouseManager to use this object as the right click processor.
    void Activate();

    // Resets the MouseManager to use the original right click processor that was used prior to the last call to Activate, if
    // this object is currently the active right click processor.
    void Deactivate();

    // Virtual functions inherited from MouseCallBack; see mouseman.h.
    virtual int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m) override;
    virtual void pan(IPoint2 offset) override;
    virtual int override(int mode) override;
    virtual BOOL SupportTransformGizmo() override;
    virtual void DeactivateTransformGizmo() override;
    virtual BOOL SupportAutoGrid() override;
    virtual BOOL TolerateOrthoMode() override;

// Private variables:
private:

    // Invalid index or count value
    static constexpr int m_invalidIndex = -1;

    // The point count for m_baseProc, if it is defined, or m_invalidIndex if this object is not presently activated
    int m_nPointsBaseProc;

    // PolyModeSelectionProcessor entity which owns this object
    PolyModeSelectionProcessorType* m_leftClickProc;

    // Base right click processor for which we are substituting, and to which we should pass through, when we are active in the
    // MouseManager, or a null pointer, otherwise
    MouseCallBack* m_baseProc;

// Private functions:
private:

    // Default constructor; hidden to force specification of required parameters.
    PolyModeRightClickProcessor() = delete;

    // Copy and move constructors; should never be called.
    //
    // rightClickProc: a right click processor object; input
    PolyModeRightClickProcessor(const PolyModeRightClickProcessor& rightClickProc) = delete;
    PolyModeRightClickProcessor(const PolyModeRightClickProcessor&& rightClickProc) = delete;

    // Copy and move assignment operators; should never be called.
    //
    // rightClickProc: a right click processor object; input
    PolyModeRightClickProcessor& operator=(const PolyModeRightClickProcessor& rightClickProc) = delete;
    PolyModeRightClickProcessor& operator=(const PolyModeRightClickProcessor&& rightClickProc) = delete;
};

template <typename PolyModeSelectionProcessorType>
PolyModeRightClickProcessor<PolyModeSelectionProcessorType>::PolyModeRightClickProcessor(PolyModeSelectionProcessorType* leftClickProc):
    MouseCallBack(),
    m_nPointsBaseProc(m_invalidIndex),
    m_leftClickProc(leftClickProc),
    m_baseProc(nullptr)
{
    DbgAssert(m_leftClickProc != nullptr);
}


template <typename PolyModeSelectionProcessorType>
PolyModeRightClickProcessor<PolyModeSelectionProcessorType>::~PolyModeRightClickProcessor()
{

    // Ensure that we are no longer the active right click processor
    Deactivate();
}

template <typename PolyModeSelectionProcessorType>
void PolyModeRightClickProcessor<PolyModeSelectionProcessorType>::Activate()
{

    // Store the right click processor currently defined in the mouse manager, and substitute ourselves, so that we can
    // properly manage a right click cancel
    Interface* coreInterface = GetCOREInterface();
    DbgAssert(coreInterface != nullptr);
    MouseManager* mouseManager = coreInterface->GetMouseManager();
    if (mouseManager != nullptr)
    {
        DbgAssert((mouseManager->GetMouseProc(LEFT_BUTTON)) == m_leftClickProc);
        m_baseProc = mouseManager->GetMouseProc(RIGHT_BUTTON);
        m_nPointsBaseProc = mouseManager->SetNumPoints(1, RIGHT_BUTTON);
        DbgAssert(m_nPointsBaseProc > 0);
        mouseManager->SetMouseProc(this, RIGHT_BUTTON, m_nPointsBaseProc);
    }
}

template <typename PolyModeSelectionProcessorType>
void PolyModeRightClickProcessor<PolyModeSelectionProcessorType>::Deactivate()
{

    // If we are presently set as the right click processor, substitue back the original
    Interface* coreInterface = GetCOREInterface();
    DbgAssert(coreInterface != nullptr);
    MouseManager* mouseManager = coreInterface->GetMouseManager();
    if (mouseManager != nullptr)
    {
        MouseCallBack* rightClickProc = mouseManager->GetMouseProc(RIGHT_BUTTON);
        if (rightClickProc == static_cast<MouseCallBack*>(this))
        {
            DbgAssert(m_baseProc != nullptr);
            DbgAssert(m_nPointsBaseProc != m_invalidIndex);
            mouseManager->SetMouseProc(m_baseProc, RIGHT_BUTTON, m_nPointsBaseProc);
            m_baseProc = nullptr;
            m_nPointsBaseProc = m_invalidIndex;
        }
    }
}

template <typename PolyModeSelectionProcessorType>
int PolyModeRightClickProcessor<PolyModeSelectionProcessorType>::proc(HWND hwnd, int msg, int point, int flags, IPoint2 m)
{

    // If an operation associated with a defined action is presently underway in m_leftClickProc, process the received message
    // here
    BOOL doContinue = TRUE;
    bool isProcessed = false;
    typename PolyModeSelectionProcessorType::PolyModeActionType actionCurr = m_leftClickProc->GetCurrentAction();
    if (actionCurr != PolyModeSelectionProcessorType::N_POLY_ACTIONS)
    {
        switch (msg)
        {
        case MOUSE_ABORT:
        case MOUSE_POINT:
        case MOUSE_MOVE:
        case MOUSE_DBLCLICK:
        case MOUSE_PROPCLICK:
        case MOUSE_SNAPCLICK:

            // On any sort of click-related event, or an abort signal, abort the operation underway in m_leftClickProc, and
            // return a flag indicating that processing of this operation should be terminated
            m_leftClickProc->AbortOperation(hwnd);
            doContinue = FALSE;
            isProcessed = true;
            break;
        default:

            // Any other message should be treated below
            break;
        }
    }

    // In the case of initialization or uninitialization, simply return without passing through to m_baseProc, regardless of
    // whether an operation is underway
    switch (msg)
    {
    case MOUSE_INIT:
    case MOUSE_UNINIT:
        isProcessed = true;
        break;
    default:
        break;
    }

    // If an operation associated with a defined action was not underway in m_leftClickProc, and the received message is not
    // an initialization or uninitialization, or if the message should anyways pass through, hand off processing to m_baseProc
    if ((!isProcessed) && (m_baseProc != nullptr))
    {
        doContinue = m_baseProc->proc(hwnd, msg, point, flags, m);
    }

    // Return flag indicating whether processing of this operation should continue
    return doContinue;
}

template <typename PolyModeSelectionProcessorType>
void PolyModeRightClickProcessor<PolyModeSelectionProcessorType>::pan(IPoint2 offset)
{
    if (m_baseProc != nullptr)
    {
        m_baseProc->pan(offset);
    }
    else
    {
        MouseCallBack::pan(offset);
    }
}

template <typename PolyModeSelectionProcessorType>
int PolyModeRightClickProcessor<PolyModeSelectionProcessorType>::override(int mode)
{
    int overrideMode = mode;
    if (m_baseProc != nullptr)
    {
        overrideMode = m_baseProc->override(mode);
    }
    else
    {
        overrideMode = MouseCallBack::override(mode);
    }

    return overrideMode;
}

template <typename PolyModeSelectionProcessorType>
BOOL PolyModeRightClickProcessor<PolyModeSelectionProcessorType>::SupportTransformGizmo()
{
    BOOL isSupported = FALSE;
    if (m_baseProc != nullptr)
    {
        isSupported = m_baseProc->SupportTransformGizmo();
    }
    else
    {
        isSupported = MouseCallBack::SupportTransformGizmo();
    }

    return isSupported;
}

template <typename PolyModeSelectionProcessorType>
void PolyModeRightClickProcessor<PolyModeSelectionProcessorType>::DeactivateTransformGizmo()
{
    if (m_baseProc != nullptr)
    {
        m_baseProc->DeactivateTransformGizmo();
    }
    else
    {
        MouseCallBack::DeactivateTransformGizmo();
    }
}

template <typename PolyModeSelectionProcessorType>
BOOL PolyModeRightClickProcessor<PolyModeSelectionProcessorType>::SupportAutoGrid()
{
    BOOL isSupported = FALSE;
    if (m_baseProc != nullptr)
    {
        isSupported = m_baseProc->SupportAutoGrid();
    }
    else
    {
        isSupported = MouseCallBack::SupportAutoGrid();
    }

    return isSupported;
}

template <typename PolyModeSelectionProcessorType>
BOOL PolyModeRightClickProcessor<PolyModeSelectionProcessorType>::TolerateOrthoMode()
{
    BOOL isTolerated = FALSE;
    if (m_baseProc != nullptr)
    {
        isTolerated = m_baseProc->TolerateOrthoMode();
    }
    else
    {
        isTolerated = MouseCallBack::TolerateOrthoMode();
    }

    return isTolerated;
}
