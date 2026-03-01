/**********************************************************************
 *<
	FILE: EditPolyUI.cpp

	DESCRIPTION: UI code for Edit Poly Modifier

	CREATED BY: Steve Anderson,

	HISTORY: created March 2004

 *>	Copyright (c) 2004 Discreet, All Rights Reserved.
 **********************************************************************/

#include "epoly.h"
#include "EditPoly.h"
#include "EditPolyUI.h"
#include <Util/IDlgShowingStatusForPolyObj.h>

static EPolyPopupPosition theEditPolyPopupPosition;
EPolyPopupPosition *ThePopupPosition() { return &theEditPolyPopupPosition; }

void EPolyPopupPosition::Scan (HWND hWnd) {
	if (!hWnd || !IsWindow(hWnd)) return;
	RECT rect;
	GetWindowRect (hWnd, &rect);
	mPosition[0] = rect.left;
	mPosition[1] = rect.top;
	mPosition[2] = rect.right - rect.left - 1;
	mPosition[3] = rect.bottom - rect.top;
	mPositionSet = true;
}

void EPolyPopupPosition::Set (HWND hWnd) {
	if (!hWnd) return;
	if (mPositionSet) 
		SetWindowPos (hWnd, NULL, mPosition[0], mPosition[1],
			mPosition[2], mPosition[3], SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	else
		CenterWindow (hWnd, GetCOREInterface()->GetMAXHWnd());
}

static EditPolyOperationProc theEditPolyOperationProc;
EditPolyOperationProc *TheOperationDlgProc () { return &theEditPolyOperationProc; }

void EditPolyOperationProc::UpdateUI (HWND hWnd, TimeValue t, int paramID) {
	HWND hDlgItem;
	static TSTR buf;

	if (paramID<0) {
		// Call this function on everything:
		UpdateUI (hWnd, t, epm_extrude_spline_node);
		UpdateUI (hWnd, t, epm_extrude_spline_align);
		UpdateUI (hWnd, t, epm_tess_type);
		UpdateUI (hWnd, t, epm_bridge_selected);
		UpdateUI (hWnd, t, epm_align_type);
		UpdateUI (hWnd, t, epm_hinge_base);
		UpdateUI (hWnd, t, epm_extrude_face_type);
		UpdateUI (hWnd, t, epm_bevel_type);
		return;
	}

	switch (paramID)
	{
	case epm_extrude_spline_node:
		hDlgItem = GetDlgItem (hWnd, IDC_EXTRUDE_PICK_SPLINE);
		if (hDlgItem)
		{
			ICustButton *but = GetICustButton (hDlgItem);
			if (but) {
				INode* splineNode = NULL;
				mpMod->getParamBlock()->GetValue (epm_extrude_spline_node, t, splineNode);
				if (splineNode) but->SetText (splineNode->GetName());
				else but->SetText (GetString (IDS_EDITPOLY_PICK_SPLINE));
				ReleaseICustButton(but);
			}
		}
		break;

	case epm_extrude_spline_align:
		hDlgItem = GetDlgItem (hWnd, IDC_EXTRUDE_ROTATION_LABEL);
		if (hDlgItem) EnableWindow (hDlgItem, mpMod->getParamBlock()->GetInt (epm_extrude_spline_align));
		break;

	case epm_align_type:
		CheckRadioButton (hWnd, IDC_EDITPOLY_ALIGN_VIEW, IDC_EDITPOLY_ALIGN_PLANE,
			mpMod->getParamBlock()->GetInt(epm_align_type) ? IDC_EDITPOLY_ALIGN_PLANE : IDC_EDITPOLY_ALIGN_VIEW);
		break;

	case epm_tess_type:
		hDlgItem = GetDlgItem (hWnd, IDC_TESS_TENSION_LABEL);
		if (hDlgItem) {
			int faceType = mpMod->getParamBlock()->GetInt (epm_tess_type, t);
			EnableWindow (hDlgItem, !faceType);
			ISpinnerControl *spin = GetISpinner (GetDlgItem (hWnd, IDC_TESS_TENSIONSPIN));
			if (spin) {
				spin->Enable (!faceType);
				ReleaseISpinner (spin);
			}
		}
		break;

	case epm_bridge_selected:
		hDlgItem = GetDlgItem (hWnd, IDC_BRIDGE_PICK_TARG1);
		if (hDlgItem) {
			bool specific = (mpMod->getParamBlock()->GetInt(epm_bridge_selected) == 0);
			ICustButton *but = GetICustButton (hDlgItem);
			if (but) {
				but->Enable (specific);
				ReleaseICustButton (but);
			}
			but = GetICustButton (GetDlgItem (hWnd, IDC_BRIDGE_PICK_TARG2));
			if (but) {
				but->Enable (specific);
				ReleaseICustButton (but);
			}

			EnableWindow (GetDlgItem (hWnd, IDC_BRIDGE_TARGET_1), specific);
			EnableWindow (GetDlgItem (hWnd, IDC_BRIDGE_TARGET_2), specific);
			UpdateUI (hWnd, t, epm_bridge_target_1);
			UpdateUI (hWnd, t, epm_bridge_target_2);
		}
		break;

	case epm_hinge_base:
		hDlgItem = GetDlgItem (hWnd, IDC_HINGE_PICK_EDGE);
		if (hDlgItem) {
			ModContextList list;
			INodeTab nodes;
			GetCOREInterface()->GetModContexts (list, nodes);
			int hinge = -1;
         int i;
			for (i=0; i<list.Count(); i++) {
				hinge = mpMod->EpModGetHingeEdge (nodes[i]);
				if (hinge>-1) break;
			}
			if (hinge<0) SetDlgItemText (hWnd, IDC_HINGE_PICK_EDGE, GetString (IDS_PICK_HINGE));
			else {
				if (list.Count() == 1) buf.printf (_T("%s %d"), GetString (IDS_EDGE), hinge+1);
				else buf.printf (_T("%s - %s %d\n"), nodes[i]->GetName(), GetString (IDS_EDGE), hinge+1);
				SetDlgItemText (hWnd, IDC_HINGE_PICK_EDGE, buf);
			}
		}
		break;

	case epm_bridge_target_1:
		hDlgItem = GetDlgItem (hWnd, IDC_BRIDGE_PICK_TARG1);
		if (hDlgItem) {
			bool specific = (mpMod->getParamBlock()->GetInt(epm_bridge_selected) == 0);
			bool edgeOrBorder = (	mpMod->GetEPolySelLevel() == EPM_SL_BORDER ||
							mpMod->GetEPolySelLevel() == EPM_SL_EDGE )  ;
			int targ = mpMod->getParamBlock ()->GetInt (epm_bridge_target_1);
			if (targ==0) SetDlgItemText (hWnd, IDC_BRIDGE_PICK_TARG1, GetString (
				edgeOrBorder ? IDS_BRIDGE_PICK_EDGE_1 : IDS_BRIDGE_PICK_POLYGON_1));
			else {
				buf.printf (GetString (edgeOrBorder ? IDS_BRIDGE_WHICH_EDGE : IDS_BRIDGE_WHICH_POLYGON), targ);
				SetDlgItemText (hWnd, IDC_BRIDGE_PICK_TARG1, buf);
			}
		}
		break;

	case epm_bridge_target_2:
		hDlgItem = GetDlgItem (hWnd, IDC_BRIDGE_PICK_TARG2);
		if (hDlgItem) {
			bool specific = (mpMod->getParamBlock()->GetInt(epm_bridge_selected) == 0);
			bool EdgeOrBorder = (	mpMod->GetEPolySelLevel() == EPM_SL_BORDER ||
				mpMod->GetEPolySelLevel() == EPM_SL_EDGE )  ;
			int targ = mpMod->getParamBlock ()->GetInt (epm_bridge_target_2);
			if (targ==0) SetDlgItemText (hWnd, IDC_BRIDGE_PICK_TARG2, GetString (
				EdgeOrBorder ? IDS_BRIDGE_PICK_EDGE_2 : IDS_BRIDGE_PICK_POLYGON_2));
			else {
				buf.printf (GetString (EdgeOrBorder ? IDS_BRIDGE_WHICH_EDGE : IDS_BRIDGE_WHICH_POLYGON), targ);
				SetDlgItemText (hWnd, IDC_BRIDGE_PICK_TARG2, buf);
			}
		}
		break;

	case epm_extrude_face_type:
		hDlgItem = GetDlgItem(hWnd, IDC_FACE_EXTRUDE_BIAS);
		if (hDlgItem) {
			auto extrudeType = mpMod->getParamBlock()->GetInt(epm_extrude_face_type);
			auto localNormSelected = extrudeType == 1;
			auto *bias = GetICustEdit(hDlgItem);
			if (bias) {
				bias->Enable(localNormSelected);
				ReleaseICustEdit(bias);
			}
			auto *biasSpin = GetISpinner(GetDlgItem(hWnd, IDC_FACE_EXTRUDE_BIAS_SPIN));
			if (biasSpin) {
				biasSpin->Enable(localNormSelected);
				ReleaseISpinner(biasSpin);
			}
		}
		break;

	case epm_bevel_type:
		hDlgItem = GetDlgItem(hWnd, IDC_BEVEL_BIAS);
		if (hDlgItem) {
			auto bevelType = mpMod->getParamBlock()->GetInt(epm_bevel_type);
			auto localNormSelected = bevelType == 1;
			auto *bias = GetICustEdit(hDlgItem);
			if (bias) {
				bias->Enable(localNormSelected);
				ReleaseICustEdit(bias);
			}
			auto *biasSpin = GetISpinner(GetDlgItem(hWnd, IDC_BEVEL_BIAS_SPIN));
			if (biasSpin) {
				biasSpin->Enable(localNormSelected);
				ReleaseISpinner(biasSpin);
			}
		}
		break;
	}
}

void EditPolyOperationProc::MaybeEnableControls(HWND hWnd, TimeValue t)
{

    // Handle enabling/disabling the controls for the Chamfer Edge, depending on what mode we're in.
    if (GetDlgItem(hWnd, IDC_EDGE_CHAMFER_INVERT)) 
    {
        int miteringType, smooth;
        mpMod->getParamBlock()->GetValue(epm_edge_chamfer_mitering_type, t, miteringType);
        mpMod->getParamBlock()->GetValue(epm_edge_chamfer_smooth, t, smooth);

        // Determine settings based upon mitering type
        bool enableBias = false;
        bool tensionEnabled = false;
        bool depthEnabled = false;
        bool useQuadIntersections = false;
        int chamferType = EP_STANDARD_EDGE_CHAMFER;
        switch (miteringType)
        {
        case EP_EDGE_CHAMFER_QUAD_MITER:
            enableBias = true;
            tensionEnabled = true;
            chamferType = EP_QUAD_EDGE_CHAMFER;            
            break;
        case EP_EDGE_CHAMFER_UNIFORM_MITER:
        case EP_EDGE_CHAMFER_RADIAL_MITER:
        case EP_EDGE_CHAMFER_PATCH_MITER:
            enableBias = true;
            depthEnabled = true;
            useQuadIntersections = true;
            chamferType = EP_QUAD_EDGE_CHAMFER;
            break;
        default:
            break;
        }

        // Legacy chamfer options
        mpMod->getParamBlock()->SetValue(epm_edge_chamfer_type, t, chamferType);
        mpMod->getParamBlock()->SetValue(epm_edge_chamfer_quad_intersections, t, useQuadIntersections);

        // Enable tension spinner when under quad mitering type (show but disable for Tri mitering type).
        ShowWindow(GetDlgItem(hWnd, IDC_EDGE_CHAMFER_TENSION_LABEL), !depthEnabled);
        ShowWindow(GetDlgItem(hWnd, IDC_EDGE_CHAMFER_TENSION), !depthEnabled);
        EnableWindow(GetDlgItem(hWnd, IDC_EDGE_CHAMFER_TENSION_LABEL), tensionEnabled);
        EnableWindow(GetDlgItem(hWnd, IDC_EDGE_CHAMFER_TENSION), tensionEnabled);
        ISpinnerControl* spin = GetISpinner(GetDlgItem(hWnd, IDC_EDGE_CHAMFER_TENSION_SPIN));
        if (spin)
        {
            ShowWindow(GetDlgItem(hWnd, IDC_EDGE_CHAMFER_TENSION_SPIN), !depthEnabled);
            spin->Enable(tensionEnabled);
            ReleaseISpinner(spin);
        }

        // Enable depth spinner when under uniform, patch, or radial mitering type
        ShowWindow(GetDlgItem(hWnd, IDC_EDGE_CHAMFER_DEPTH_LABEL), depthEnabled);
        ShowWindow(GetDlgItem(hWnd, IDC_EDGE_CHAMFER_DEPTH), depthEnabled);
        EnableWindow(GetDlgItem(hWnd, IDC_EDGE_CHAMFER_DEPTH_LABEL), depthEnabled);
        EnableWindow(GetDlgItem(hWnd, IDC_EDGE_CHAMFER_DEPTH), depthEnabled);
        spin = GetISpinner(GetDlgItem(hWnd, IDC_EDGE_CHAMFER_DEPTH_SPIN));
        if (spin)
        {
            ShowWindow(GetDlgItem(hWnd, IDC_EDGE_CHAMFER_DEPTH_SPIN), depthEnabled);
            spin->Enable(depthEnabled);
            ReleaseISpinner(spin);
        }

        // Disable end bias spinner when under tri mitering type.
        EnableWindow(GetDlgItem(hWnd, IDC_EDGE_CHAMFER_END_BIAS_LABEL), enableBias);
        spin = GetISpinner(GetDlgItem(hWnd, IDC_EDGE_CHAMFER_END_BIAS_SPIN));
        if (spin)
        {
            spin->Enable(enableBias);
            ReleaseISpinner(spin);
        }

        // Disable radial bias spinner when under tri mitering type.
        EnableWindow(GetDlgItem(hWnd, IDC_EDGE_CHAMFER_RADIAL_BIAS_LABEL), enableBias);
        spin = GetISpinner(GetDlgItem(hWnd, IDC_EDGE_CHAMFER_RADIAL_BIAS_SPIN));
        if (spin)
        {
            spin->Enable(enableBias);
            ReleaseISpinner(spin);
        }

        // Enable smooth controls.
        bool enableSmoothControls = smooth == 1;
        EnableWindow (GetDlgItem (hWnd, IDC_EDGE_CHAMFER_SMOOTH_ALL), enableSmoothControls);
        EnableWindow (GetDlgItem (hWnd, IDC_EDGE_CHAMFER_SMOOTH_CHAMFERS), enableSmoothControls);
        EnableWindow (GetDlgItem (hWnd, IDC_EDGE_CHAMFER_THRESHOLD_LABEL), enableSmoothControls);
        spin = GetISpinner(GetDlgItem(hWnd, IDC_EDGE_CHAMFER_THRESHOLD_SPIN));
        if(spin) {
            spin->Enable(enableSmoothControls);
            ReleaseISpinner(spin);
        }
    }

    // If we are displaying the vertex chamfer dialog, enable or disable the smoothing controls depending upon whether or not
    // smoothing is active
    HWND smoothingControl = GetDlgItem(hWnd, IDC_VERTEX_CHAMFER_SMOOTH);
    if (smoothingControl != NULL)
    {
        BOOL doSmooth = FALSE;
        Interval interval = FOREVER;
		IParamBlock2* params = mpMod->getParamBlock();
        params->GetValue(epm_vertex_chamfer_smooth, t, doSmooth, interval);
        HWND smoothAll = GetDlgItem(hWnd, IDC_VERTEX_CHAMFER_SMOOTH_ALL);
        EnableWindow(smoothAll, doSmooth);
        HWND smoothChamfers = GetDlgItem(hWnd, IDC_VERTEX_CHAMFER_SMOOTH_CHAMFERS);
        EnableWindow(smoothChamfers, doSmooth);
        HWND thresholdLabel = GetDlgItem(hWnd, IDC_VERTEX_CHAMFER_THRESHOLD_LABEL);
        EnableWindow(thresholdLabel, doSmooth);
        HWND thresholdSpinner = GetDlgItem(hWnd, IDC_VERTEX_CHAMFER_THRESHOLD_SPIN);
        ISpinnerControl* spinnerThreshold = GetISpinner(thresholdSpinner);
        if (spinnerThreshold != NULL)
        {
            spinnerThreshold->Enable(doSmooth);
            ReleaseISpinner(spinnerThreshold);
        }
    }
}

INT_PTR EditPolyOperationProc::DlgProc (TimeValue t, IParamMap2 *map,
										HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	ICustButton* but = NULL;
	ISpinnerControl *spin = nullptr;
	MaxSDK::Util::IDlgShowingStatusForPolyObj* pDlgShowingStatus = NULL;
	HWND hOpDlg;

	switch (msg) {
	case WM_INITDIALOG:
		// Place dialog in correct position:
		theEditPolyPopupPosition.Set (hWnd);

		MaybeEnableControls(hWnd, t);		// Enable/disable various controls depending on the system state

		but = GetICustButton (GetDlgItem (hWnd, IDC_EXTRUDE_PICK_SPLINE));
		if (but) {
			but->SetType (CBT_CHECK);
			ReleaseICustButton(but);
		}

		but = GetICustButton (GetDlgItem (hWnd, IDC_HINGE_PICK_EDGE));
		if (but) {
			but->SetType (CBT_CHECK);
			ReleaseICustButton(but);
		}

		// Right-click on spinner won't reset to default values set in the param block. 
		// To do so - and be consistent with the spinner, do it manually. 
		if (GetDlgItem(hWnd, IDC_EDGE_CHAMFER_TENSION))
		{
			spin = GetISpinner(GetDlgItem(hWnd, IDC_EDGE_CHAMFER_SPIN));
			spin->SetResetValue(1.0f);
			ReleaseISpinner(spin);
			spin = GetISpinner(GetDlgItem(hWnd, IDC_EDGE_CHAMFER_SEGMENTS_SPIN));
			spin->SetResetValue(1);
			ReleaseISpinner(spin);
			spin = GetISpinner(GetDlgItem(hWnd, IDC_EDGE_CHAMFER_TENSION_SPIN));
			spin->SetResetValue(1.0f);
			ReleaseISpinner(spin);
			spin = GetISpinner(GetDlgItem(hWnd, IDC_EDGE_CHAMFER_DEPTH_SPIN));
			spin->SetResetValue(0.5f);
			ReleaseISpinner(spin);
			spin = GetISpinner(GetDlgItem(hWnd, IDC_EDGE_CHAMFER_END_BIAS_SPIN));
			spin->SetResetValue(0.5f);
			ReleaseISpinner(spin);
			spin = GetISpinner(GetDlgItem(hWnd, IDC_EDGE_CHAMFER_RADIAL_BIAS_SPIN));
			spin->SetResetValue(0.0f);
			ReleaseISpinner(spin);
			spin = GetISpinner(GetDlgItem(hWnd, IDC_EDGE_CHAMFER_THRESHOLD_SPIN));
			spin->SetResetValue(30.0f);
		}

		if (GetDlgItem (hWnd, IDC_BRIDGE_PICK_TARG1)) {
			but = GetICustButton (GetDlgItem (hWnd, IDC_BRIDGE_PICK_TARG1));
			if (but) {
				but->SetType (CBT_CHECK);
				ReleaseICustButton(but);
			}

			but = GetICustButton (GetDlgItem (hWnd, IDC_BRIDGE_PICK_TARG2));
			if (but) {
				but->SetType (CBT_CHECK);
				ReleaseICustButton(but);
			}

			bool Border = (mpMod->GetEPolySelLevel() == EPM_SL_BORDER );
			bool Edge   = (mpMod->GetEPolySelLevel() == EPM_SL_EDGE )  ;

			if (Border )
			{
				SetDlgItemText (hWnd, IDC_BRIDGE_SPECIFIC, GetString (IDS_BRIDGE_SPECIFIC_BORDERS) );
				SetDlgItemText (hWnd, IDC_BRIDGE_SELECTED, GetString (IDS_BRIDGE_BORDER_SELECTION) );
			}
			else if (Edge )
			{
				SetDlgItemText (hWnd, IDC_BRIDGE_SPECIFIC, GetString (IDS_BRIDGE_SPECIFIC_EDGES ));
				SetDlgItemText (hWnd, IDC_BRIDGE_SELECTED, GetString (IDS_BRIDGE_EDGE_SELECTION ));
			}
			else 
			{
				SetDlgItemText (hWnd, IDC_BRIDGE_SPECIFIC, GetString (IDS_BRIDGE_SPECIFIC_POLYGONS ));
				SetDlgItemText (hWnd, IDC_BRIDGE_SELECTED, GetString (IDS_BRIDGE_POLYGON_SELECTION ));
			}
			
			SetDlgItemText (hWnd, IDC_BRIDGE_TARGET_1, GetString (
				(Edge || Border) ? IDS_BRIDGE_EDGE_1 : IDS_BRIDGE_POLYGON_1));
			SetDlgItemText (hWnd, IDC_BRIDGE_TARGET_2, GetString (
				(Edge || Border) ? IDS_BRIDGE_EDGE_2 : IDS_BRIDGE_POLYGON_2));
		}

		TheConstraintUIHandler()->Initialize (hWnd, mpMod);

		UpdateUI (hWnd, t, -1);

		pDlgShowingStatus = MaxSDK::Util::GetDlgShowingStatusForPolyObj();
		if(pDlgShowingStatus)
		{
			pDlgShowingStatus->SetDlgShowingForPolyObj(true);
		}
		break;

	case WM_PAINT:
		return false;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_EXTRUDE_PICK_SPLINE:
			if (!mpMod) return false;
			mpMod->EpModEnterPickMode (ep_mode_pick_shape);
			break;
		case IDC_CONSTRAINT_NONE:
			TheConstraintUIHandler()->Set (hWnd, mpMod,0);
			break;
		case IDC_CONSTRAINT_EDGE:
			TheConstraintUIHandler()->Set (hWnd, mpMod,1);
			break;
		case IDC_CONSTRAINT_FACE:
			TheConstraintUIHandler()->Set (hWnd, mpMod,2);
			break;
		case IDC_CONSTRAINT_NORMAL:
			TheConstraintUIHandler()->Set (hWnd, mpMod,3);
			break;
		case IDC_APPLY:
			if (!mpMod) return false;
			theHold.Begin ();
			mpMod->EpModCommitAndRepeat (t);
			theHold.Accept (GetString (IDS_APPLY));
			mpMod->EpModRefreshScreen ();
			break;

		case IDOK:
			if (!mpMod) return false;
			// Commit only if not animating.
			theHold.Begin ();
			mpMod->EpModCommitUnlessAnimating (t);
			theHold.Accept (GetString (IDS_EDITPOLY_COMMIT));
			mpMod->EpModCloseOperationDialog();
			mpMod->EpModRefreshScreen ();
			pDlgShowingStatus = MaxSDK::Util::GetDlgShowingStatusForPolyObj();
			if(pDlgShowingStatus)
			{
				pDlgShowingStatus->SetDlgShowingForPolyObj(false);
			}
			break;

		case IDCANCEL:
			if (!mpMod) return false;
			mpMod->EpModCancel ();
			// (Dialog automatically destroyed.)
			mpMod->EpModRefreshScreen ();
			pDlgShowingStatus = MaxSDK::Util::GetDlgShowingStatusForPolyObj();
			if(pDlgShowingStatus)
			{
				pDlgShowingStatus->SetDlgShowingForPolyObj(false);
			}
			break;

		case IDC_HINGE_PICK_EDGE:
			mpMod->EpModToggleCommandMode (ep_mode_pick_hinge);
			break;

		case IDC_BRIDGE_PICK_TARG1:
			mpMod->EpModToggleCommandMode (ep_mode_pick_bridge_1);
			break;

		case IDC_BRIDGE_PICK_TARG2:
			mpMod->EpModToggleCommandMode (ep_mode_pick_bridge_2);
			break;

		case IDC_EDITPOLY_ALIGN_UPDATE:
			theHold.Begin ();
			mpMod->UpdateAlignParameters (t);
			theHold.Accept (GetString (IDS_UPDATE_ALIGN));
			mpMod->EpModRefreshScreen();
			break;

		case IDC_EDITPOLY_ALIGN_VIEW:
			theHold.Begin ();
			mpMod->getParamBlock()->SetValue (epm_align_type, t, 0);
			mpMod->UpdateAlignParameters (t);
			theHold.Accept (GetString (IDS_UPDATE_ALIGN));
			mpMod->EpModRefreshScreen();
			break;

		case IDC_EDITPOLY_ALIGN_PLANE:
			theHold.Begin ();
			mpMod->getParamBlock()->SetValue (epm_align_type, t, 1);
			mpMod->UpdateAlignParameters (t);
			theHold.Accept (GetString (IDS_UPDATE_ALIGN));
			mpMod->EpModRefreshScreen();
			break;

		case IDC_MS_SEP_BY_SMOOTH:
		case IDC_MS_SEP_BY_MATID:
		case IDC_TESS_EDGE:
		case IDC_TESS_FACE:
		case IDC_EXTYPE_GROUP:
		case IDC_EXTYPE_LOCAL:
		case IDC_EXTYPE_BY_POLY:
		case IDC_BEVTYPE_GROUP:
		case IDC_BEVTYPE_LOCAL:
		case IDC_BEVTYPE_BY_POLY:
		case IDC_INSETTYPE_GROUP:
		case IDC_INSETTYPE_BY_POLY:
		case IDC_BRIDGE_SPECIFIC:
		case IDC_BRIDGE_SELECTED:
		case IDC_RELAX_HOLD_BOUNDARY:
		case IDC_RELAX_HOLD_OUTER:
		case IDC_EXTRUDE_ALIGN_NORMAL:
		case IDC_REVERSETRI:
		case IDC_CHAMFER_VERTEX_OPEN:
		case IDC_EDGE_CHAMFER_SMOOTH_ALL:
		case IDC_EDGE_CHAMFER_SMOOTH_CHAMFERS:
			// Following is necessary because Modeless ParamMap2's don't send such a message.
			mpMod->EpModRefreshScreen ();
			break;
		case IDC_EDGE_CHAMFER_OPEN:
		{
			mpMod->getParamBlock()->EnableNotifications(false);
			mpMod->getParamBlock()->SetValue(epm_edge_chamfer_invert, t, false);
			mpMod->getParamBlock()->EnableNotifications(true);
			mpMod->EpModRefreshScreen();
		}
			break;
		case IDC_EDGE_CHAMFER_INVERT:
		{
			mpMod->getParamBlock()->EnableNotifications(false);
			mpMod->getParamBlock()->SetValue(epm_chamfer_edge_open, t, false);
			mpMod->getParamBlock()->EnableNotifications(true);
			mpMod->EpModRefreshScreen();
		}
			break;
		case IDC_EDGE_CHAMFER_SMOOTH:
		case IDC_EDGE_CHAMFER_MITERING_TYPE:
			MaybeEnableControls(hWnd, t);
			mpMod->EpModRefreshScreen ();
			break;

		}
		break;

	case CC_SPINNER_CHANGE:
	case CC_SPINNER_BUTTONUP:
		// Following is necessary because Modeless ParamMap2's don't send such a message:
		mpMod->EpModRefreshScreen();
		break;

	case WM_DESTROY:
		theEditPolyPopupPosition.Scan (hWnd);
		hOpDlg = mpMod->GetDlgHandle (ep_animate);
		if (hOpDlg) {
			ICustButton *pBut = GetICustButton (GetDlgItem (hOpDlg, IDC_EDITPOLY_SHOW_SETTINGS));
			if (pBut) {
				pBut->SetCheck (false);
				ReleaseICustButton (pBut);
			}
		}
		break;

	default:
		return false;
	}

	return true;
}

void EditPolyMod::UpdateOperationDialogParams () {
	if (!mpDialogOperation) return;
	HWND hWnd = mpDialogOperation->GetHWnd();
	theEditPolyOperationProc.UpdateUI (hWnd, ip->GetTime(), -1);
}

bool EditPolyMod::EpModShowingOperationDialog ()
{
	PolyOperation *pop = GetPolyOperation ();
	MaxSDK::Util::IDlgShowingStatusForPolyObj* pDlgShowingStatus = MaxSDK::Util::GetDlgShowingStatusForPolyObj();
	if (pop != NULL&& pDlgShowingStatus && pDlgShowingStatus->GetDlgShowSettingForPolyObj())
	{
		if (mpDialogOperation != nullptr)
			return true;
		else
			return false;
	}
	else if(pop!=NULL&&pop->HasGrip())
	{
		return  pop->IsGripShown();
	}
	return (mpDialogOperation != NULL);
}

bool EditPolyMod::EpModShowOperationDialog ()
{
	if (!ip) return false;
	TimeValue t = ip->GetTime();
	PolyOperation *pop = GetPolyOperation ();
	if (pop == NULL) return false;

	// close the showing Grips/dialogs.
	EpModCloseOperationDialog();
    
	MaxSDK::Util::IDlgShowingStatusForPolyObj* pDlgShowingStatus = MaxSDK::Util::GetDlgShowingStatusForPolyObj();
	if(pDlgShowingStatus && pDlgShowingStatus->GetDlgShowSettingForPolyObj())
	{
		//okay doesn't have grip do the normal stuff
		if (mpDialogOperation) return true;

		int dialogID = pop->DialogID ();
		if (dialogID != 0)
		{
			mpDialogOperation = CreateModelessParamMap2 (
				ep_settings, mpParams, t, hInstance, MAKEINTRESOURCE (dialogID),
				GetCOREInterface()->GetMAXHWnd(), &theEditPolyOperationProc);

			if (mpDialogAnimate) {
				ICustButton *pBut = GetICustButton (GetDlgItem (mpDialogAnimate->GetHWnd(), IDC_EDITPOLY_SHOW_SETTINGS));
				if (pBut) {
					pBut->SetCheck (true);
					ReleaseICustButton (pBut);
				}
			}

			NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
			return true;
		}
		else
		{
			if (mpDialogAnimate) {
				ICustButton *pBut = GetICustButton (GetDlgItem (mpDialogAnimate->GetHWnd(), IDC_EDITPOLY_SHOW_SETTINGS));
				if (pBut) {
					pBut->SetCheck (false);
					ReleaseICustButton (pBut);
				}
			}
			return false;
		}

	}
	else
	{
		if(pop->HasGrip())
		{
			pop->ShowGrip(this);
			NotifyDependents (FOREVER, PART_DISPLAY, REFMSG_CHANGE);
			return true;
		}
	}
	return false;
}

void EditPolyMod::EpModCloseOperationDialog ()
{
	PolyOperation *pop = GetPolyOperation ();
	if(pop&&pop->HasGrip())
	{
		pop->HideGrip();
		//fall through maybe it has a dialog too?
	}
	if (!mpDialogOperation) return;

	theEditPolyPopupPosition.Scan (mpDialogOperation->GetHWnd());
	DestroyModelessParamMap2 (mpDialogOperation);
	mpDialogOperation = NULL;

	if (mpDialogAnimate) {
		ICustButton *pBut = GetICustButton (GetDlgItem (mpDialogAnimate->GetHWnd(), IDC_EDITPOLY_SHOW_SETTINGS));
		if (pBut) {
			pBut->SetCheck (false);
			ReleaseICustButton (pBut);
		}
	}
}


