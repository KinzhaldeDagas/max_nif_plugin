//**************************************************************************/
// Copyright (c) 2021 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
//
// FILE:        PolyModeSelectionProcessor.h
// DESCRIPTION: Template selection processor class used to manage mouse
//              interaction for various command modes in MNMesh-based
//              entities such as Edit Poly and Editable Poly
// AUTHOR:      Lee Betchen
//
// HISTORY:	    April 2021 Creation
//**************************************************************************/

#pragma once

#include <objmode.h>
#include <mouseman.h>
#include <maxapi.h>
#include <assert1.h>
#include <hold.h>
#include <max.h>
#include <ICUIMouseConfigManager.h>
#include "PolyModeRightClickProcessor.h"

// Class PolyModeSelectionProcessor is a selection processor, derived from BaseSelectionProcessor, which may be used in
// place of that class for a particular mode, e.g. move mode, for an MNMesh-based entity of type PolyType (generally an
// Editable Poly object or Edit Poly modifier) to allow any action from the list defined by the PolyModeActionType enumeration
// to be tied to any hotkey combination list in the PolyModeHotkeyType enumeration, for any particular mesh geometry selection
// type enumerated in PMeshSelLevel. If an action has not been defined for a particular combination of hotkey and selection
// type, or if the action does not make sense for a particular selection type, e.g. action POLY_ACTION_EXTRUDE for selection
// type MNM_SL_OBJECT, the base class treatment defined in BaseSelectionProcessor is employed. Note that the precise form
// of the operation is dependent upon the related mode transformation encapsulated in transMod, e.g. for move mode, with the
// POLY_ACTION_CLONE action and MNM_SL_FACE selection type, dragging moves the new face, while in scale mode, dragging resizes
// the face.
template <typename BaseSelectionProcessor, typename PolyType>
class PolyModeSelectionProcessor: public BaseSelectionProcessor
{

// Public types:
public:

    // Enumeration indexing the hotkey combinations for dragging operation in the relevant mode supported by this class; any
    // combination not supported here is passed through to the BaseSelectionProcessor's treatment 
    enum PolyModeHotkeyType
    {
        POLY_HOTKEY_SHIFT      = 0,
        POLY_HOTKEY_CTRL_SHIFT = 1,
        N_POLY_HOTKEYS         = 2
    };

    // Enumeration indexing the possible actions which may be bound to any of the hotkey combinations indexed by
    // PolyMoveModeHotkeyType; these are the actions performed before the transformation defined by transMode
    enum PolyModeActionType
    {
        POLY_ACTION_CLONE                 = 0,    // Creates a copy of the selected entities and their sub-entities (e.g. vertices and edges for a face selection); transMode acts on the new entites
        POLY_ACTION_EXTRUDE               = 1,    // "Splits" the borders of the selected entities, generating edges and faces to connect the original and cloned borders; transMode acts on the original selection
        POLY_ACTION_EXTRUDE_POSTPROCESSED = 2,    // As POLY_ACTION_EXTRUDE, but performs additional postprocessing after the transformation is applied to the selection
        N_POLY_ACTIONS                    = 3
    };

// Public member functions:
public:

    // Constructor. Note that all passed pointers must be non-null.
    //
    // transMode: object implementing transformation for geometric subobjects of object object in the relevant mode, e.g.
    //            translation for move mode; input
    // object:    the object that we are to operate upon; input
    // objParam:  interface entity; input
    PolyModeSelectionProcessor(TransformModBox* transMode, BaseObject* object, IObjParam* objParam);

    // Destructor.
    virtual ~PolyModeSelectionProcessor();

    // Assigns an action to a hotkey combination for dragging operations in the relevant mode with the specified selection
    // type. By default, all combinations of hotkey and selection type have no defined action, so that their treatment is
    // managed by the base class.
    //
    // hotkey:     the key combination to which an operation is to be assigned; input
    // action:     the action to be performed when the key combination indicated by hotkey is pressed during a dragging
    //             operation in the relevant mode with selection type selection, or N_POLY_ACTIONS to clear hotkey's presently
    //             assigned action, so that treatment will be managed by the base class; input
    // actionName: action name to display in the undo list for the operation involving the specified key combination and
    //             selection type, not used if action = N_POLY_ACTIONS; input
    // selection:  mesh geometry selection type for which the key combination indicated by hotkey is to trigger action action,
    //             or selection = MNM_SL_CURRENT to assign action to hotkey for all selection types; input
    void SetHotkeyAction(PolyModeHotkeyType hotkey, PolyModeActionType action, TCHAR* actionName, int selection = MNM_SL_CURRENT);

    // Aborts the present operation, if GetCurrentAction() indicates a valid action presently being processed by this class.
    // Resets the state to indicate that the operation has been terminated.
    //
    // window: handle for the current window; input
    void AbortOperation(HWND window);

    // Returns an enumeration value indicating the present operation, or the value N_POLY_ACTIONS if no valid action is
    // presently being processed.
    inline PolyModeActionType GetCurrentAction()
    {
        return m_actionCurr;
    }

    // Virtual functions inherited from MouseCallBack; see mouseman.h.
    virtual int proc(HWND hwnd, int msg, int point, int flags, IPoint2 m) override;

// Private variables:
private:

    // Number of possible mesh geometry selection types; note that MNM_SL_CURRENT itself is a dummy value that indicates the
    // present selction type, whatever it may be
    static constexpr int m_nMeshSelectionTypes = MNM_SL_CURRENT;

    // Action currently being processed, if any; if no defined action is being processed, the invalid value N_POLY_ACTIONS is
    // set
    PolyModeActionType m_actionCurr;

    // Name string for action currently being processed, if any; if no defined action is being processed, the empty string is
    // set
    TSTR m_actionNameCurr;

    // Entry m_hotkeyActions[key][sel] is the action to be performed if the hotkey combination indicated by key is used during
    // a dragging operation in the relevant mode, with selection type sel according to the PMeshSelLevel enumeration, or
    // N_POLY_ACTIONS if combination key is to be passed through to the base class treatment for selection type sel
    PolyModeActionType m_hotkeyActions[N_POLY_HOTKEYS][m_nMeshSelectionTypes];

    // If m_hotkeyActions[key][sel] != N_POLY_ACTIONS, then m_hotkeyActionNames[key][sel] is the associated name string,
    // otherwise, m_hotkeyActionNames[key][sel] is set to the empty string
    TSTR m_hotkeyActionNames[N_POLY_HOTKEYS][m_nMeshSelectionTypes];

    // Position vector of the mouse cursor at the beginning of the present operation m_actionCurr, if m_actionCurr is a valid
    // operation
    IPoint2 m_mousePointBegin;

    // Flag indicating, for the present operation m_actionCurr, if m_actionCurr is a valid operation, whether the mouse has at
    // any point in the operation been moved from m_mousePointBegin by more than a small tolerance
    bool m_hasMouseMoved;

    // Object implementing transformation for geometric subobjects of object object in the relevant mode, e.g. translation for
    // move mode
    TransformModBox* m_transMode;

    // While we are active as our MouseManager's left click processor, we substitute m_rightClickProc in the MouseManager as
    // the right click processor, in order to cleanly treat a right click cancel
    PolyModeRightClickProcessor<PolyModeSelectionProcessor<BaseSelectionProcessor, PolyType>>* m_rightClickProc;

// Private member functions
private:

    // Default constructor; hidden to force specification of required parameters.
    PolyModeSelectionProcessor() = delete;

    // Copy and move constructors; should never be called.
    //
    // selectionProc: a selection processor object; input
    PolyModeSelectionProcessor(const PolyModeSelectionProcessor& selectionProc) = delete;
    PolyModeSelectionProcessor(const PolyModeSelectionProcessor&& selectionProc) = delete;

    // Copy and move assignment operators; should never be called.
    //
    // selectionProc: a selection processor object; input
    PolyModeSelectionProcessor& operator=(const PolyModeSelectionProcessor& selectionProc) = delete;
    PolyModeSelectionProcessor& operator=(const PolyModeSelectionProcessor&& selectionProc) = delete;

    // Begin the present operation m_actionCurr, initializing class members. Corresponds to the MOUSE_POINT message at point
    // zero. Should not be called if m_actionCurr is not a valid operation.
    //
    // isProcessed:  when true, the present case was processed, with success status indicated by hasSucceeded, otherwise
    //               processing should be passed to base class for this operation; output
    // hasSucceeded: flag indicating whether processing was successful; output
    // window:       handle for the current window; input
    // flags:        the MOUSE_XXX bitwise flags indicating mouse/keyboard input; input
    // mousePoint:   present mouse position vector, after application of offset; input
    void BeginOperation(bool& isProcessed, BOOL& hasSucceeded, HWND window, int flags, const IPoint2& mousePoint);

    // Update the parameters of the present operation m_actionCurr as appropriate, based upon the new mouse position
    // mousePoint. Corresponds to the MOUSE_MOVE message. Should not be called if m_actionCurr is not a valid operation.
    //
    // isProcessed:  when true, the present case was processed, with success status indicated by hasSucceeded, otherwise
    //               processing should be passed to base class for this operation; output
    // hasSucceeded: flag indicating whether processing was successful; output
    // window:       handle for the current window; input
    // point:        the present stage of the mouse operation currently being processed; input
    // flags:        the MOUSE_XXX bitwise flags indicating mouse/keyboard input; input
    // mousePoint:   present mouse position vector, after application of offset; input
    void UpdateMousePoint(bool& isProcessed, BOOL& hasSucceeded, HWND window, int point, int flags, const IPoint2& mousePoint);

    // Complete the present operation m_actionCurr, committing the results, and reset the state of this object. Corresponds to
    // the MOUSE_POINT message, with point point equal to the final point for operation m_actionCurr. Should not be called if
    // m_actionCurr is not a valid operation.
    //
    // isProcessed:  when true, the present case was processed, with success status indicated by hasSucceeded, otherwise
    //               processing should be passed to base class for this operation; output
    // hasSucceeded: flag indicating whether processing was successful; output
    // window:       handle for the current window; input
    // point:        the present stage of the mouse operation currently being processed; input
    // flags:        the MOUSE_XXX bitwise flags indicating mouse/keyboard input; input
    // mousePoint:   present mouse position vector, after application of offset; input
    void CompleteOperation(bool& isProcessed, BOOL& hasSucceeded, HWND window, int point, int flags, const IPoint2& mousePoint);
};

template <typename BaseSelectionProcessor, typename PolyType>
PolyModeSelectionProcessor<BaseSelectionProcessor, PolyType>::PolyModeSelectionProcessor(TransformModBox* transMode, BaseObject* object, IObjParam* objParam):
    BaseSelectionProcessor(transMode, object, objParam),
    m_actionCurr(N_POLY_ACTIONS),
    m_actionNameCurr(_T("")),
    m_mousePointBegin(0, 0),
    m_hasMouseMoved(false),
    m_transMode(transMode)
{
    DbgAssert(m_transMode != nullptr);
    DbgAssert(object != nullptr);
    DbgAssert(objParam != nullptr);

    // Instantiate our right click processor
    m_rightClickProc = new PolyModeRightClickProcessor<PolyModeSelectionProcessor<BaseSelectionProcessor, PolyType>>(this);

    // Initialize arrays of actions and action name indices for each combination of hotkey and selection type
    for (int key = 0; key != N_POLY_HOTKEYS; ++key)
    {
        for (int sel = 0; sel != m_nMeshSelectionTypes; ++sel)
        {
            m_hotkeyActions[key][sel] = N_POLY_ACTIONS;
            m_hotkeyActionNames[key][sel] = _T("");
        }
    }
}

template <typename BaseSelectionProcessor, typename PolyType>
PolyModeSelectionProcessor<BaseSelectionProcessor, PolyType>::~PolyModeSelectionProcessor()
{
    if (m_rightClickProc != nullptr)
    {
        delete m_rightClickProc;
    }
}

template <typename BaseSelectionProcessor, typename PolyType>
void PolyModeSelectionProcessor<BaseSelectionProcessor, PolyType>::SetHotkeyAction(PolyModeHotkeyType hotkey, PolyModeActionType action, TCHAR* actionName, int selection)
{
    constexpr int nActionTypesTot = N_POLY_ACTIONS + 1;
    constexpr bool isActionValidForSelection[nActionTypesTot][m_nMeshSelectionTypes] = {{true, true, true, true},
                                                                                        {false, true, true, true},
                                                                                        {false, true, true, true},
                                                                                        {true, true, true, true}};

    // Check in debug mode to ensure a valid hotkey combination and selection type
    DbgAssert(hotkey != N_POLY_HOTKEYS);
    DbgAssert((selection >= MNM_SL_OBJECT) && (selection <= MNM_SL_CURRENT));

    // If action = N_POLY_ACTIONS, then ensure that we specify the empty string in m_hotkeyActionNames, otherwise, set the
    // specified string
    TSTR nameString = _T("");
    if (action != N_POLY_ACTIONS)
    {
        DbgAssert(actionName != nullptr);
        nameString = actionName;
    }

    // If selection = MNM_SL_CURRENT, then set the action for the specified hotkey combination for all mesh geometry selection
    // types, otherwise, set only for the specified type
    int selectionBegin = selection;
    int nSelections = 1;
    if (selection == MNM_SL_CURRENT)
    {
        selectionBegin = 0;
        nSelections = m_nMeshSelectionTypes;
    }

    for (int sel = 0; sel != nSelections; ++sel)
    {
        DbgAssert((selectionBegin + sel) < m_nMeshSelectionTypes);
        if (isActionValidForSelection[action][selectionBegin + sel])
        {
            m_hotkeyActions[hotkey][selectionBegin + sel] = action;
            m_hotkeyActionNames[hotkey][selectionBegin + sel] = nameString;
        }
    }
}

template <typename BaseSelectionProcessor, typename PolyType>
void PolyModeSelectionProcessor<BaseSelectionProcessor, PolyType>::AbortOperation(HWND window)
{
    static constexpr int endPoint = 1;

    // Abort operation via the undo system, if one is presently underway
    if (m_actionCurr != N_POLY_ACTIONS)
    {
        PolyType* polyObject = static_cast<PolyType*>(this->obj);
        DbgAssert(polyObject != nullptr);
        TimeValue timeCurr = this->ip->GetTime();
        m_transMode->proc(window, MOUSE_POINT, endPoint, 0, m_mousePointBegin);
        switch (m_actionCurr)
        {
        case POLY_ACTION_EXTRUDE_POSTPROCESSED:

            // For the postprocessed extrude operation, we must first allow the operation to complete properly before
            // cancellation, in order to ensure that the undo object is in the expected state
            polyObject->FinalizeExtrudeSelSubComponentsPostprocessed(timeCurr);
            break;
        default:
            break;
        }

        theHold.SuperCancel();
        this->ip->RedrawViews(timeCurr);
    }

    // Reset member variable to indicate an undefined action
    m_actionCurr = N_POLY_ACTIONS;
    m_actionNameCurr = _T("");
    m_hasMouseMoved = false;
    m_mousePointBegin = IPoint2(0, 0);
}

template <typename BaseSelectionProcessor, typename PolyType>
int PolyModeSelectionProcessor<BaseSelectionProcessor, PolyType>::proc(HWND hwnd, int msg, int point, int flags, IPoint2 m)
{

    // If we are at the beginning of a mouse operation, check flags and selection type determine if an action defined here has
    // been triggered
    if ((msg == MOUSE_POINT) && (point == 0))
    {
        PolyType* polyObject = static_cast<PolyType*>(this->obj);
        DbgAssert(polyObject != nullptr);
        int selection = polyObject->GetMNSelLevel();
        DbgAssert((selection >= MNM_SL_OBJECT) && (selection < MNM_SL_CURRENT));

        // We are at the beginnning of a mouse operation; note that our actions are disabled if we are in Maya mode, or if
        // an action override is active
        MaxSDK::CUI::IMouseConfigManager* mouseConfigManager = MaxSDK::CUI::GetIMouseConfigManager();
        bool isMayaMode = mouseConfigManager->GetMayaSelection();
        IActionItemOverrideManager* actionOverrideManager = static_cast<IActionItemOverrideManager*>(GetCOREInterface(IACTIONITEMOVERRIDEMANAGER_INTERFACE));
        const ActionItem* overridingAction = actionOverrideManager->GetOverridingActionItem();
        if ((!isMayaMode) && (overridingAction == nullptr))
        {

            // Defined actions are enabled; check flags and selection level, and set appropriate action m_actionCurr if one
            // has been triggered, otherwise, set invalid action to indicate that treatment should be handled entirely by
            // the base class
            m_actionCurr = N_POLY_ACTIONS;
            m_actionNameCurr = _T("");
            PolyModeHotkeyType hotkey = N_POLY_HOTKEYS;
            if (flags & MOUSE_SHIFT)
            {
                hotkey = POLY_HOTKEY_SHIFT;
                if (flags & MOUSE_CTRL)
                {
                    hotkey = POLY_HOTKEY_CTRL_SHIFT;
                }
            }

            if (hotkey != N_POLY_HOTKEYS)
            {
                m_actionCurr = m_hotkeyActions[hotkey][selection];
                m_actionNameCurr = m_hotkeyActionNames[hotkey][selection];
            }
        }
    }

    // If we have encountered an operation which is to be processed by this class, process now as appropriate to the message
    // received
    BOOL hasSucceeded = TRUE;
    bool isProcessed = false;
    IPoint2& mouseOffset = this->Offset();
    if (m_actionCurr != N_POLY_ACTIONS)
    {
        IPoint2 offsetMousePoint = m + mouseOffset;
        switch (msg)
        {
        case MOUSE_POINT:

            // Click point event;  as we presently support only two-point operations, we must either begin the operation, or
            // finalize it, depending upon point
            if (point == 0)
            {
                BeginOperation(isProcessed, hasSucceeded, hwnd, flags, offsetMousePoint);
            }
            else
            {
                DbgAssert(point == 1);
                CompleteOperation(isProcessed, hasSucceeded, hwnd, point, flags, offsetMousePoint);
            }

            break;
        case MOUSE_MOVE:

            // Mouse moved; update parameters of the present operation as appropriate
            UpdateMousePoint(isProcessed, hasSucceeded, hwnd, point, flags, offsetMousePoint);
            break;
        case MOUSE_KEYBOARD:

            // Keyboard signal; disregard and keep processing the present action
            isProcessed = true;
            break;
        default:
            break;
        }
    }

    // If we receive an initialization/uninitialization message, initialize/uninitialize ourselves before passing through to
    // the base class variant
    switch (msg)
    {
    case MOUSE_INIT:

        // Initialization event; set m_rightClickProc as the right click processor, so that we may cleanly handle right click
        // cancel signals
        m_rightClickProc->Activate();
        break;
    case MOUSE_UNINIT:

        // Uninitialization event; reset the original right click processor
        m_rightClickProc->Deactivate();
        break;
    default:
        break;
    }

    // If we have an operation which is not defined here, or if messsage msg is not supported for the present operation, or in
    // the case of an initialization/uninitialization event, call the base class variant to complete processing, ensuring that
    // we abort and clear any operation previously in progress
    if (!isProcessed)
    {
        AbortOperation(hwnd);
        if (hasSucceeded)
        {
            hasSucceeded = BaseSelectionProcessor::proc(hwnd, msg, point, flags, m);
        }
    }

    // If our operation failed, clear the mouse pointer offset vector, consistent with the base class variant, and abort any
    // current operation
    if (!hasSucceeded)
    {
        mouseOffset = IPoint2(0, 0);
        AbortOperation(hwnd);
    }

    // Return a flag indicating the success status of the operation
    return hasSucceeded;
}

template <typename BaseSelectionProcessor, typename PolyType>
void PolyModeSelectionProcessor<BaseSelectionProcessor, PolyType>::BeginOperation(bool& isProcessed, BOOL& hasSucceeded, HWND window, int flags, const IPoint2& mousePoint)
{
    constexpr int hitCheckOcclusion = 1 << (HITFLAG_STARTUSERBIT + 1);

    // Check in debug mode to ensure a valid state
    DbgAssert(m_actionCurr != N_POLY_ACTIONS);

    // Set the mouse begin position for this operation
    m_mousePointBegin = mousePoint;
    m_hasMouseMoved = false;

    // If the user has clicked on a selected component, or the selection is frozen, proceed with execution of the operation,
    // acting on the present selection
    isProcessed = false;
    hasSucceeded = TRUE;
    BOOL isSelectionValid = this->ip->SelectionFrozen();
    if (!isSelectionValid)
    {
        IPoint2 selectionPoint = mousePoint;
        ViewExp& viewport = this->ip->GetViewExp(window);
        int testFlags = HIT_SELONLY | hitCheckOcclusion;
        BOOL useTransformGizmo = this->ip->GetUseTransformGizmo();
        BOOL isTransformGizmoSupported = this->SupportTransformGizmo();
        if (useTransformGizmo && isTransformGizmoSupported)
        {
            testFlags = testFlags | HIT_SWITCH_GIZMO;
        }

        isSelectionValid = this->HitTest(&viewport, &selectionPoint, HITTYPE_POINT, testFlags);
    }

    // If a valid selection was found above, process the operation based upon the selection, otherwise, terminate the
    // operation; note that in the latter case, we flag the operation as undefined here to avoid an unbalanced call in the hold
    // system
    if (isSelectionValid)
    {
        theHold.SuperBegin();
        TimeValue timeCurr = this->ip->GetTime();
        PolyType* polyObject = static_cast<PolyType*>(this->obj);
        DbgAssert(polyObject != nullptr);
        switch (m_actionCurr)
        {
        case POLY_ACTION_CLONE:
	        polyObject->CloneSelSubComponents(timeCurr);
            break;
        case POLY_ACTION_EXTRUDE:
	        polyObject->ExtrudeSelSubComponents(timeCurr);
            break;
        case POLY_ACTION_EXTRUDE_POSTPROCESSED:
	        polyObject->ExtrudeSelSubComponents(timeCurr, true);
            break;
        default:
            break;
        }

        this->ip->RedrawViews(timeCurr);
        isProcessed = true;
    }
    else
    {
        m_actionCurr = N_POLY_ACTIONS;
        m_actionNameCurr = _T("");
    }
}

template <typename BaseSelectionProcessor, typename PolyType>
void PolyModeSelectionProcessor<BaseSelectionProcessor, PolyType>::UpdateMousePoint(bool& isProcessed, BOOL& hasSucceeded, HWND window, int point, int flags, const IPoint2& mousePoint)
{
    constexpr int moveTolSq = 4;

    // Check in debug mode to ensure a valid state
    DbgAssert(m_actionCurr != N_POLY_ACTIONS);

    // If the mouse has not yet moved beyond tolerance from m_mousePointBegin, that is, if the operation has not fully begun,
    // check the new position vector against the begin point for this operation
    hasSucceeded = TRUE;
    if (!m_hasMouseMoved)
    {
        IPoint2 deltaMouse = mousePoint - m_mousePointBegin;
        int deltaLengthSq = (deltaMouse.x) * (deltaMouse.x) + (deltaMouse.y) * (deltaMouse.y);
        if (deltaLengthSq > moveTolSq)
        {
            m_hasMouseMoved = true;

            // Begin processing of operation in callback, as well; send the message for MOUSE_POINT zero
            hasSucceeded = m_transMode->proc(window, MOUSE_POINT, 0, flags, m_mousePointBegin);
        }
    }

    // Process the present message in the callback, if the operation has begun; this performs the actual mode operation, e.g.
    // moving cloned or extruded geometry in move mode
    if (m_hasMouseMoved && hasSucceeded)
    {
        hasSucceeded = m_transMode->proc(window, MOUSE_MOVE, point, flags, mousePoint);

        // Perform any intermediate postprocessing required by the action after the transformation is applied
        if (hasSucceeded)
        {
            PolyType* polyObject = static_cast<PolyType*>(this->obj);
            DbgAssert(polyObject != nullptr);
            TimeValue timeCurr = this->ip->GetTime();
            switch (m_actionCurr)
            {
            case POLY_ACTION_EXTRUDE_POSTPROCESSED:
                polyObject->UpdateExtrudeSelSubComponentsPostprocessed(timeCurr);
                break;
            default:
                break;
            }
        }
    }

    // Set flag to indicate that processing has occurred
    isProcessed = true;
}

template <typename BaseSelectionProcessor, typename PolyType>
void PolyModeSelectionProcessor<BaseSelectionProcessor, PolyType>::CompleteOperation(bool& isProcessed, BOOL& hasSucceeded, HWND window, int point, int flags, const IPoint2& mousePoint)
{
    DbgAssert(m_actionCurr != N_POLY_ACTIONS);

    // Proceed according to whether the mouse pointer has at any point moved beyond tolerance from m_mousePointBegin, that is,
    // whether the operation was ever properly begun
    isProcessed = true;
    hasSucceeded = TRUE;
    PolyType* polyObject = static_cast<PolyType*>(this->obj);
    DbgAssert(polyObject != nullptr);
    if (m_hasMouseMoved)
    {

        // Operations was actually begun; pass message to callback
        hasSucceeded = m_transMode->proc(window, MOUSE_POINT, point, flags, mousePoint);

        // Finalize operation, and reset member variable to indicate an undefined action
        if (hasSucceeded)
        {
            TimeValue timeCurr = this->ip->GetTime();
            switch (m_actionCurr)
            {
            case POLY_ACTION_CLONE:
                polyObject->AcceptCloneSelSubComponents(timeCurr);
                break;
            case POLY_ACTION_EXTRUDE_POSTPROCESSED:
                polyObject->FinalizeExtrudeSelSubComponentsPostprocessed(timeCurr);
                break;
            default:
                break;
            }

            theHold.SuperAccept(m_actionNameCurr);
            m_actionCurr = N_POLY_ACTIONS;
            m_actionNameCurr = _T("");
            m_hasMouseMoved = false;
            m_mousePointBegin = IPoint2(0, 0);
        }
    }
    else
    {
        // Operation did not properly begin
        AbortOperation(window);
    }

    // Deactivate the transform gizmo upon completion
    BOOL useTransformGizmo = this->ip->GetUseTransformGizmo();
    BOOL isTransformGizmoSupported = this->SupportTransformGizmo();
    if (useTransformGizmo && isTransformGizmoSupported)
    {
        this->DeactivateTransformGizmo();
    }
}
