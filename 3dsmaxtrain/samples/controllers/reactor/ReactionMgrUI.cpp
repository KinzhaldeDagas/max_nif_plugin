/******************************************************************************
DESCRIPTION:
	Implementation of UI classes that are reaction controller-related:
	ReactionDlg, StateListItem, ReactionListItem

CREATED BY:
	Adam Felt

HISTORY:

Copyright (c) 1998-2006, All Rights Reserved.
*******************************************************************************/
#include "reactionMgr.h"
#include "restoreObjs.h"
#include "maxicon.h"
#include <commctrl.h>

#include "3dsmaxport.h"

//////////////////////////////////////////////////////////////
//************************************************************

static INT_PTR CALLBACK ReactionDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static LRESULT CALLBACK SplitterControlProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

const int ReactionDlg::kMaxTextLength = 255;
const int ReactionDlg::kNumReactionColumns = 4;
const int ReactionDlg::kNumStateColumns = 5;

HIMAGELIST ReactionDlg::mIcons = nullptr;

SingleRefMaker theReactionDlg;
ReactionDlg* GetReactionDlg() { return (ReactionDlg*)theReactionDlg.GetRef(); }

void CreateReactionDlg()
{
	theReactionDlg.SetRef(new ReactionDlg((IObjParam*)GetCOREInterface(), GetCOREInterface()->GetMAXHWnd()));
}

Reactor* FindReactor(ICurve* curve)
{
	MyEnumProc dep(curve);
	curve->DoEnumDependents(&dep);

	for (int x = 0; x < dep.anims.Count(); x++)
	{
		Animatable* anim = dep.anims[x];
		DbgAssert(anim != nullptr);
		if (anim->SuperClassID() == REF_MAKER_CLASS_ID && anim->ClassID() == CURVE_CONTROL_CLASS_ID)
		{
			auto* cc = static_cast<ReferenceTarget*>(anim);
			MyEnumProc dep2(cc);
			cc->DoEnumDependents(&dep2);

			for (int i = 0; i < dep2.anims.Count(); i++)
			{
				if (auto* r = static_cast<Reactor*>(GetIReactorInterface(dep2.anims[i])))
					return r;
			}
		}
	}
	return nullptr;
}
ReactionListItem* GetReactionListItem(HWND hList, int row, int col)
{
	LVITEM item;
	item.mask = LVIF_PARAM;
	item.iItem = row;
	item.iSubItem = col;
	ListView_GetItem(hList, &item);

	return (ReactionListItem*)item.lParam;
}

StateListItem* GetStateListItem(HWND hList, int row, int col)
{
	LVITEM item;
	item.mask = LVIF_PARAM;
	item.iItem = row;
	item.iSubItem = col;
	ListView_GetItem(hList, &item);

	return (StateListItem*)item.lParam;
}

ReactionDlg::ReactionDlg(IObjParam* iop, HWND hParent)
{
	DbgAssert(GetReactionDlg() == nullptr);
	ip = iop;
	iCCtrl = nullptr;
	valid = false;
	selectionValid = stateSelectionValid = reactionListValid = false;
	origPt = nullptr;
	rListPos = sListPos = 1.0f / 3.0f;
	blockInvalidation = false;
	mCurPickMode = nullptr;
	reactionManager = nullptr;

	selectedNodes.SetCount(0, TRUE);

	theHold.Suspend();
	ReplaceReference(0, GetReactionManager());
	theHold.Resume();

	mIcons = ImageList_Create(16, 16, ILC_COLOR24 | ILC_MASK, 14, 0);
	TSTR IconFile = _T("ReactionManager");

	hWnd = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_REACTION_DIALOG), hParent, (DLGPROC)ReactionDlgProc, (LPARAM)this);
	TSTR title = GetString(IDS_REACTIONMGR_TITLE);
	SetWindowText(hWnd, title);
	ip->RegisterDlgWnd(hWnd);
	ip->RegisterTimeChangeCallback(this);

	LoadMAXFileIcon(IconFile, mIcons, kWindow, FALSE);
	SendMessage(hWnd, WM_SETICON, ICON_SMALL, DLGetClassLongPtr<LPARAM>(GetCOREInterface()->GetMAXHWnd(), GCLP_HICONSM));

	reactionDlgActionCB = new ReactionDlgActionCB<ReactionDlg>(this);
	ip->GetActionManager()->ActivateActionTable(reactionDlgActionCB, kReactionMgrActions);

	RegisterNotification(SelectionSetChanged, this, NOTIFY_SELECTIONSET_CHANGED);
	RegisterNotification(PreSceneNodeDeleted, this, NOTIFY_SCENE_PRE_DELETED_NODE);
	RegisterNotification(PreReset, this, NOTIFY_SYSTEM_PRE_RESET);
	RegisterNotification(PreOpen, this, NOTIFY_FILE_PRE_OPEN);
	RegisterNotification(PreOpen, this, NOTIFY_SYSTEM_PRE_NEW);
	RegisterNotification(PostOpen, this, NOTIFY_FILE_POST_OPEN);
	RegisterNotification(PostOpen, this, NOTIFY_FILE_OPEN_FAILED);
	RegisterNotification(PostOpen, this, NOTIFY_SYSTEM_POST_NEW);
}

void ReactionDlg::PreSceneNodeDeleted(void* param, NotifyInfo* info)
{
	ReactionDlg* dlg = (ReactionDlg*)param;
	INode* node = GetNotifyParam<NOTIFY_SCENE_PRE_DELETED_NODE>(info);
	if (!node) return;
	for (int i = dlg->selectedNodes.Count() - 1; i >= 0; i--)
	{
		if (dlg->selectedNodes[i] == node)
		{
			dlg->selectedNodes.Delete(i, 1);
			break;
		}
	}
}

void ReactionDlg::SelectionSetChanged(void* param, NotifyInfo* info)
{
	ReactionDlg* dlg = (ReactionDlg*)param;
	ICustButton* showBut = GetICustButton(GetDlgItem(dlg->hWnd, IDC_SHOW_SELECTED));
	if (showBut->IsChecked())
	{
		ICustButton* but = GetICustButton(GetDlgItem(dlg->hWnd, IDC_REFRESH));
		but->Enable();
		ReleaseICustButton(but);
	}
	ReleaseICustButton(showBut);
}

void ReactionDlg::PreReset(void* param, NotifyInfo* info)
{
	ReactionDlg* dlg = (ReactionDlg*)param;
	dlg->blockInvalidation = true;
	DestroyWindow(dlg->hWnd);
}

void ReactionDlg::PreOpen(void* param, NotifyInfo* info)
{
	DbgAssert(info);

	// Do not empty the reaction manager if we're just loading a render preset file
	if (info != nullptr && info->intcode == NOTIFY_FILE_PRE_OPEN && info->callParam)
	{
		FileIOType type = *GetNotifyParam<NOTIFY_FILE_PRE_OPEN>(info);
		if (type == IOTYPE_RENDER_PRESETS)
			return;
	}

	ReactionDlg* dlg = (ReactionDlg*)param;
	dlg->blockInvalidation = true;
	dlg->InvalidateReactionList();
	ListView_DeleteAllItems(GetDlgItem(dlg->hWnd, IDC_STATE_LIST));
	ListView_DeleteAllItems(GetDlgItem(dlg->hWnd, IDC_REACTION_LIST));
	if (dlg->iCCtrl != nullptr)
	{
		theHold.Suspend();
		dlg->ClearCurves();
		theHold.Resume();
	}
	// ListView_SetItemState(GetDlgItem(dlg->hWnd, IDC_REACTION_LIST), -1, 0, LVIS_SELECTED);
}

void ReactionDlg::PostOpen(void* param, NotifyInfo* info)
{
	if (auto* dlg = static_cast<ReactionDlg*>(param))
	{
		dlg->blockInvalidation = false;
		dlg->Invalidate();
	}
}

ReactionDlg::~ReactionDlg()
{
	ICustButton* pickBut = GetICustButton(GetDlgItem(hWnd, IDC_ADD_DRIVER));
	ICustButton* pickBut2 = GetICustButton(GetDlgItem(hWnd, IDC_ADD_DRIVEN));
	if (pickBut->IsChecked() || pickBut2->IsChecked())
		GetCOREInterface()->ClearPickMode();
	ReleaseICustButton(pickBut);
	ReleaseICustButton(pickBut2);

	if (mCurPickMode != nullptr)
	{
		delete mCurPickMode;
		mCurPickMode = nullptr;
	}

	UnRegisterNotification(SelectionSetChanged, this, NOTIFY_SELECTIONSET_CHANGED);
	UnRegisterNotification(PreSceneNodeDeleted, this, NOTIFY_SCENE_PRE_DELETED_NODE);
	UnRegisterNotification(PreReset, this, NOTIFY_SYSTEM_PRE_RESET);
	UnRegisterNotification(PreOpen, this, NOTIFY_FILE_PRE_OPEN);
	UnRegisterNotification(PreOpen, this, NOTIFY_SYSTEM_PRE_NEW);
	UnRegisterNotification(PostOpen, this, NOTIFY_FILE_POST_OPEN);
	UnRegisterNotification(PostOpen, this, NOTIFY_FILE_OPEN_FAILED);
	UnRegisterNotification(PostOpen, this, NOTIFY_SYSTEM_POST_NEW);

	ip->GetActionManager()->DeactivateActionTable(reactionDlgActionCB, kReactionMgrActions);
	delete reactionDlgActionCB;
	reactionDlgActionCB = nullptr;
	ReleaseICustEdit(floatWindow);

	ip->UnRegisterDlgWnd(hWnd);
	ip->UnRegisterTimeChangeCallback(this);

	ip = nullptr;
	theHold.Suspend();
	DeleteAllRefsFromMe();
	if (iCCtrl)
	{
		ClearCurves();
		iCCtrl->MaybeAutoDelete();
	}
	theHold.Resume();

	ImageList_Destroy(mIcons);
}

void ReactionDlg::Invalidate()
{
	if (!blockInvalidation)
	{
		valid = FALSE;
		InvalidateRect(hWnd, nullptr, FALSE);
	}
}

void ReactionDlg::SetupUI(HWND hWnd)
{
	this->hWnd = hWnd;
	theHold.Suspend();
	BuildCurveCtrl();
	SetupReactionList();
	SetupStateList();
	theHold.Resume();
	valid = FALSE;

	FillButton(hWnd, IDC_ADD_DRIVER, 0, 11, GetString(IDS_ADD_DRIVER), CBT_CHECK);
	FillButton(hWnd, IDC_ADD_DRIVEN, 1, 12, GetString(IDS_ADD_DRIVEN), CBT_CHECK);
	FillButton(hWnd, IDC_ADD_SELECTED, 2, 13, GetString(IDS_ADD_SELECTED), CBT_PUSH);
	FillButton(hWnd, IDC_DELETE_SELECTED, 3, 14, GetString(IDS_DELETE_SELECTED), CBT_PUSH);
	FillButton(hWnd, IDC_PREFERENCES, 4, 15, GetString(IDS_PREFERENCES), CBT_PUSH);
	FillButton(hWnd, IDC_REFRESH, 9, 20, GetString(IDS_REFRESH), CBT_PUSH);

	FillButton(hWnd, IDC_SET_STATE, 5, 16, GetString(IDS_SET_STATE), CBT_PUSH);
	FillButton(hWnd, IDC_NEW_STATE, 6, 17, GetString(IDS_NEW_STATE), CBT_PUSH);
	FillButton(hWnd, IDC_APPEND_STATE, 10, 21, GetString(IDS_APPEND_STATE), CBT_PUSH);
	FillButton(hWnd, IDC_DELETE_STATE, 3, 14, GetString(IDS_DELETE_STATE), CBT_PUSH);

	ICustButton* but = GetICustButton(GetDlgItem(hWnd, IDC_CREATE_STATES));
	but->SetType(CBT_CHECK);
	ReleaseICustButton(but);

	but = GetICustButton(GetDlgItem(hWnd, IDC_EDIT_STATE));
	but->SetType(CBT_CHECK);
	ReleaseICustButton(but);

	but = GetICustButton(GetDlgItem(hWnd, IDC_SHOW_SELECTED));
	but->SetType(CBT_CHECK);
	ReleaseICustButton(but);

	but = GetICustButton(GetDlgItem(hWnd, IDC_REFRESH));
	but->Disable();
	ReleaseICustButton(but);

	but = GetICustButton(GetDlgItem(hWnd, IDC_SPLITTER1));
	but->Disable();
	ReleaseICustButton(but);

	but = GetICustButton(GetDlgItem(hWnd, IDC_SPLITTER2));
	but->Disable();
	ReleaseICustButton(but);

	ShowWindow(GetDlgItem(hWnd, IDC_PREFERENCES), SW_HIDE);

	floatWindow = GetICustEdit(GetDlgItem(hWnd, IDC_EDIT_FLOAT));
	floatWindow->WantReturn(TRUE);
	ShowWindow(floatWindow->GetHwnd(), SW_HIDE);

	PerformLayout();
}

void ReactionDlg::BuildCurveCtrl()
{
	theHold.Suspend();
	iCCtrl = (ICurveCtl*)CreateInstance(REF_MAKER_CLASS_ID, CURVE_CONTROL_CLASS_ID);
	theHold.Resume();

	if (!iCCtrl)
		return;

	theHold.Suspend();
	iCCtrl->SetNumCurves(0, FALSE);
	iCCtrl->SetCCFlags(CC_SINGLESELECT | CC_DRAWBG | CC_DRAWGRID | CC_DRAWUTOOLBAR | CC_SHOWRESET | CC_DRAWLTOOLBAR |
			CC_DRAWSCROLLBARS | CC_AUTOSCROLL | CC_DRAWRULER | CC_RCMENU_MOVE_XY | CC_RCMENU_MOVE_X | CC_RCMENU_MOVE_Y |
			CC_RCMENU_SCALE | CC_RCMENU_INSERT_CORNER | CC_RCMENU_INSERT_BEZIER | CC_RCMENU_DELETE |
			CC_SHOW_CURRENTXVAL);
	iCCtrl->SetXRange(0.0f, 0.01f);
	iCCtrl->SetYRange(-100.0f, 100.0f);
	iCCtrl->RegisterResourceMaker(this);
	iCCtrl->ZoomExtents();
	iCCtrl->SetCustomParentWnd(hWnd);
	iCCtrl->SetMessageSink(hWnd);

	theHold.Resume();
	iCCtrl->SetActive(TRUE);
}

void ReactionDlg::FillButton(HWND hWnd, int butID, int iconIndex, int disabledIconIndex, TSTR toolText, CustButType type)
{
	ICustButton* iBut = GetICustButton(GetDlgItem(hWnd, butID));
	iBut->SetImage(mIcons, iconIndex, iconIndex, disabledIconIndex, disabledIconIndex, 16, 16);
	iBut->SetType(type);
	iBut->Execute(I_EXEC_CB_NO_BORDER);
	iBut->SetTooltip(TRUE, toolText);
	ReleaseICustButton(iBut);
}

void ReactionDlg::PerformLayout()
{
	const int buttonWidth = 24;
	const int bigButtonWidth = buttonWidth * 4;
	const int controlSpacing = 2;
	const int buttonSpacing = buttonWidth + controlSpacing;
	const int bigButtonSpacing = bigButtonWidth + controlSpacing;
	const int buttonHeight = 24;
	const int borderWidth = 6;

	int nextX = borderWidth;
	int nextY = borderWidth;
	SetWindowPos(GetDlgItem(hWnd, IDC_ADD_DRIVER), nullptr, nextX, nextY, buttonWidth, buttonHeight, SWP_NOZORDER);
	nextX += buttonSpacing;
	SetWindowPos(GetDlgItem(hWnd, IDC_ADD_DRIVEN), nullptr, nextX, nextY, buttonWidth, buttonHeight, SWP_NOZORDER);
	nextX += buttonSpacing;
	SetWindowPos(GetDlgItem(hWnd, IDC_ADD_SELECTED), nullptr, nextX, nextY, buttonWidth, buttonHeight, SWP_NOZORDER);
	nextX += buttonSpacing;
	SetWindowPos(GetDlgItem(hWnd, IDC_DELETE_SELECTED), nullptr, nextX, nextY, buttonWidth, buttonHeight, SWP_NOZORDER);
	nextX += buttonSpacing;
	SetWindowPos(GetDlgItem(hWnd, IDC_PREFERENCES), nullptr, nextX, nextY, buttonWidth, buttonHeight, SWP_NOZORDER);
	nextX += buttonSpacing;
	SetWindowPos(GetDlgItem(hWnd, IDC_SHOW_SELECTED), nullptr, nextX, nextY, bigButtonWidth, buttonHeight, SWP_NOZORDER);
	nextX += bigButtonSpacing;
	SetWindowPos(GetDlgItem(hWnd, IDC_REFRESH), nullptr, nextX, nextY, buttonWidth, buttonHeight, SWP_NOZORDER);

	nextX = borderWidth;
	nextY += buttonHeight + borderWidth;

	RECT rect;
	GetClientRect(hWnd, &rect);
	int height = ((rect.bottom - (4 * borderWidth)) * rListPos) - (buttonHeight + borderWidth);
	int width = rect.right - 2 * borderWidth;

	SetWindowPos(GetDlgItem(hWnd, IDC_REACTION_LIST), nullptr, nextX, nextY, width, height, SWP_NOZORDER);
	nextY += height;

	nextX = borderWidth;
	SetWindowPos(GetDlgItem(hWnd, IDC_SPLITTER1), nullptr, nextX, nextY, width, borderWidth - 2, SWP_NOZORDER);
	nextY += borderWidth;

	SetWindowPos(GetDlgItem(hWnd, IDC_CREATE_STATES), nullptr, nextX, nextY, bigButtonWidth, buttonHeight, SWP_NOZORDER);
	nextX += bigButtonSpacing;
	SetWindowPos(GetDlgItem(hWnd, IDC_NEW_STATE), nullptr, nextX, nextY, buttonWidth, buttonHeight, SWP_NOZORDER);
	nextX += buttonSpacing;
	SetWindowPos(GetDlgItem(hWnd, IDC_APPEND_STATE), nullptr, nextX, nextY, buttonWidth, buttonHeight, SWP_NOZORDER);
	nextX += buttonSpacing;
	SetWindowPos(GetDlgItem(hWnd, IDC_EDIT_STATE), nullptr, nextX, nextY, bigButtonWidth, buttonHeight, SWP_NOZORDER);
	nextX += bigButtonSpacing;
	SetWindowPos(GetDlgItem(hWnd, IDC_SET_STATE), nullptr, nextX, nextY, buttonWidth, buttonHeight, SWP_NOZORDER);
	nextX += buttonSpacing;
	SetWindowPos(GetDlgItem(hWnd, IDC_DELETE_STATE), nullptr, nextX, nextY, buttonWidth, buttonHeight, SWP_NOZORDER);
	nextY += buttonHeight + borderWidth;

	nextX = borderWidth;
	height = ((rect.bottom - (4 * borderWidth)) * sListPos) - (buttonHeight + borderWidth);
	SetWindowPos(GetDlgItem(hWnd, IDC_STATE_LIST), nullptr, nextX, nextY, width, height, SWP_NOZORDER);

	nextY += height;
	nextX = borderWidth;
	SetWindowPos(GetDlgItem(hWnd, IDC_SPLITTER2), nullptr, nextX, nextY, width, borderWidth - 2, SWP_NOZORDER);

	nextY += borderWidth;
	if (iCCtrl != nullptr)
		SetWindowPos(iCCtrl->GetHWND(), nullptr, nextX, nextY, width, rect.bottom - borderWidth - nextY, SWP_NOZORDER);
}

void ReactionDlg::SetupReactionList()
{
	HWND hList = GetDlgItem(hWnd, IDC_REACTION_LIST);
	int sel = ListView_GetNextItem(hList, -1, LVNI_SELECTED);

	ListView_SetExtendedListViewStyleEx(hList, LVS_EX_GRIDLINES, LVS_EX_GRIDLINES);

	ListView_SetImageList(hList, mIcons, LVSIL_STATE);

	LV_COLUMN column;
	column.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;

	column.fmt = LVCFMT_LEFT;
	column.pszText = GetString(IDS_AF_REACTIONS);
	column.cx = 340;
	ListView_InsertColumn(hList, kNameCol, &column);

	column.pszText = GetString(IDS_AF_FROM);
	column.cx = 60;
	ListView_InsertColumn(hList, kFromCol, &column);

	column.pszText = GetString(IDS_AF_TO);
	column.cx = 60;
	ListView_InsertColumn(hList, kToCol, &column);

	column.pszText = GetString(IDS_AF_CURVE);
	column.cx = 60;
	ListView_InsertColumn(hList, kCurveCol, &column);

	UpdateReactionList();
}

void ReactionDlg::SetupStateList()
{
	HWND hList = GetDlgItem(hWnd, IDC_STATE_LIST);
	int sel = ListView_GetNextItem(hList, -1, LVNI_SELECTED);

	ListView_SetExtendedListViewStyleEx(hList, LVS_EX_GRIDLINES, LVS_EX_GRIDLINES);
	ListView_SetImageList(hList, mIcons, LVSIL_STATE);

	LV_COLUMN column;
	column.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;

	column.fmt = LVCFMT_LEFT;
	column.pszText = GetString(IDS_AF_STATES);
	column.cx = 280;
	ListView_InsertColumn(hList, kNameCol, &column);

	column.pszText = GetString(IDS_AF_VALUE);
	column.cx = 60;
	ListView_InsertColumn(hList, kValueCol, &column);

	column.pszText = GetString(IDS_AF_STRENGTH);
	column.cx = 60;
	ListView_InsertColumn(hList, kStrengthCol, &column);

	column.pszText = GetString(IDS_AF_INFLUENCE);
	column.cx = 60;
	ListView_InsertColumn(hList, kInfluenceCol, &column);

	column.pszText = GetString(IDS_AF_FALLOFF);
	column.cx = 60;
	ListView_InsertColumn(hList, kFalloffCol, &column);

	UpdateStateList();
}

bool ReactionDlg::AddDrivenRow(HWND hList, ReactionListItem* listItem, int itemID)
{
	TSTR buf = listItem->GetName();
	if (buf.Length() > 0)
	{
		LVITEM drivenItem;
		drivenItem.mask = LVIF_TEXT | LVIF_PARAM; //|LVIF_INDENT; (doesn't work...)
		drivenItem.iItem = itemID;
		drivenItem.iSubItem = 0;
		// drivenItem.iIndent = 2;
		TSTR buf2 = _T("     ");
		buf2 += buf;
		drivenItem.pszText = const_cast<TCHAR*>(buf2.data());
		drivenItem.cchTextMax = buf2.length();
		drivenItem.lParam = (LPARAM)listItem;

		itemID = ListView_InsertItem(hList, &drivenItem) + 1;
		for (int col = 1; col < kNumReactionColumns; col++)
		{
			LV_ITEM lvitem;
			lvitem.mask = LVIF_PARAM;
			lvitem.iItem = itemID;
			lvitem.iSubItem = col;
			lvitem.lParam = (LPARAM)listItem;
			ListView_SetItem(hList, &lvitem);
		}
		return true;
	}
	return false;
}

bool ReactionDlg::AddDrivenStateRow(HWND hList, StateListItem* listItem, int itemID)
{
	TSTR buf = listItem->GetName();
	if (buf.Length() > 0)
	{
		LVITEM drivenItem;
		drivenItem.mask = LVIF_TEXT | LVIF_PARAM; // |LVIF_INDENT; (doesn't work...)
		drivenItem.iItem = itemID;
		drivenItem.iSubItem = 0;
		// drivenItem.iIndent = 2;
		TSTR buf2(_T("     "));
		buf2 += buf;
		drivenItem.pszText = const_cast<TCHAR*>(buf2.data());
		drivenItem.cchTextMax = buf2.length();
		drivenItem.lParam = (LPARAM)listItem;

		itemID = ListView_InsertItem(hList, &drivenItem) + 1;
		for (int col = 1; col < kNumStateColumns; col++)
		{
			LV_ITEM lvitem;
			lvitem.mask = LVIF_PARAM;
			lvitem.iItem = itemID;
			lvitem.iSubItem = col;
			lvitem.lParam = (LPARAM)listItem;
			ListView_SetItem(hList, &lvitem);
		}
		return true;
	}
	return false;
}

class GatherSelectedReactionComponents : public AnimEnum
{
	Tab<Reactor*> reactors;
	Tab<ReactionSet*> mDrivers;
	BitArray mSelDrivers;
	bool unselectedCleared = false;

public:
	GatherSelectedReactionComponents(Tab<ReactionSet*> m) : mDrivers(m)
	{
		reactors.SetCount(0);
		mSelDrivers.SetSize(mDrivers.Count());
		mSelDrivers.ClearAll();
		SetScope(SCOPE_ALL);
	}

	int proc(Animatable* anim, Animatable* client, int subNum) override
	{
		if (anim->GetInterface(REACTOR_INTERFACE))
		{
			auto* r = static_cast<Reactor*>(anim);
			reactors.Append(1, &r);
		}
		for (int i = 0; i < mDrivers.Count(); i++)
		{
			if (anim == mDrivers[i]->GetReactionDriver()->GetDriver())
			{
				mSelDrivers.Set(i, TRUE);
				break;
			}
		}
		return ANIM_ENUM_PROCEED;
	}

	Tab<Reactor*> SelectedReactors() { return reactors; }
	Tab<ReactionSet*> GetSelectedDrivers()
	{
		if (!unselectedCleared)
		{
			for (int i = mDrivers.Count() - 1; i >= 0; i--)
			{
				if (!mSelDrivers[i])
					mDrivers.Delete(i, 1);
			}
			unselectedCleared = true;
			mSelDrivers.SetSize(mDrivers.Count());
			mSelDrivers.SetAll();
		}
		return mDrivers;
	}
};

void ReactionDlg::UpdateReactionList()
{
	selectionValid = false;

	ICustButton* but = GetICustButton(GetDlgItem(hWnd, IDC_SHOW_SELECTED));
	BOOL showSelected = but->IsChecked();
	ReleaseICustButton(but);

	HWND hList = GetDlgItem(hWnd, IDC_REACTION_LIST);

	Tab<Reactor*> reactors;
	Tab<ReactionSet*> drivers;
	reactors.SetCount(0);
	drivers.SetCount(0);

	if (showSelected)
	{
		// collect all non-null drivers
		for (int i = 0; i < GetReactionManager()->ReactionCount(); i++)
		{
			ReactionSet* set = GetReactionManager()->GetReactionSet(i);
			if (ReactionDriver* driver = set->GetReactionDriver())
			{
				if (ReferenceTarget* owner = driver->Owner())
					drivers.Append(1, &set);
			}
		}
		if (drivers.Count() == 0)
		{
			int itemCt = ListView_GetItemCount(hList);
			for (int x = itemCt - 1; x >= 0; x--)
				ListView_DeleteItem(hList, x);

			reactionListValid = true;
			SelectionChanged();
			return;
		}

		// clear flags then enumerate nodes
		GatherSelectedReactionComponents ae(drivers);
		for (int x = 0; x < selectedNodes.Count(); x++)
		{
			selectedNodes[x]->EnumAnimTree(&ae, nullptr, 0);
		}
		reactors = ae.SelectedReactors();
		drivers = ae.GetSelectedDrivers();
	}

	int itemID = 0;
	for (int i = 0; i < GetReactionManager()->ReactionCount(); i++)
	{
		ReactionSet* set = GetReactionManager()->GetReactionSet(i);
		if (set == nullptr)
			continue;

		bool gotMatch = false;
		if (showSelected)
		{
			int x;
			for (x = 0; x < drivers.Count(); x++)
			{
				if (set == drivers[x])
				{
					gotMatch = true;
					break;
				}
			}
			if (!gotMatch)
			{
				bool gotIt = false;
				for (x = 0; x < set->DrivenCount(); x++)
				{
					for (int y = 0; y < reactors.Count(); y++)
					{
						if (set->Driven(x) == reactors[y])
						{
							gotIt = true;
							break;
						}
					}
					if (gotIt)
						break;
				}
				if (!gotIt)
					continue;
			}
		}

		auto* listItem = new ReactionListItem(ReactionListItem::kReactionSet, set, set, i);

		LV_ITEM mainItem;
		mainItem.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;

		mainItem.iItem = itemID;
		mainItem.iSubItem = 0;
		TSTR buf = listItem->GetName();
		mainItem.pszText = const_cast<TCHAR*>(buf.data());
		mainItem.cchTextMax = static_cast<int>(_tcslen(buf));
		mainItem.stateMask = LVIS_SELECTED;
		mainItem.state = (i == GetReactionManager()->GetSelected()) ? LVIS_SELECTED : 0;

		if (listItem->IsExpandable())
		{
			mainItem.stateMask |= LVIS_STATEIMAGEMASK;
			mainItem.state |= INDEXTOSTATEIMAGEMASK(9);
		}
		mainItem.lParam = (LPARAM)listItem;

		if (ListView_GetItemCount(hList) <= itemID)
			ListView_InsertItem(hList, &mainItem);
		else
			ListView_SetItem(hList, &mainItem);

		LVITEM item;
		for (int i = 1; i < kNumReactionColumns; i++)
		{
			item.mask = LVIF_PARAM;
			item.iSubItem = i;
			item.lParam = (LPARAM)listItem;
			ListView_SetItem(hList, &item);
		}

		itemID++;


		for (int x = 0; x < set->DrivenCount(); x++)
		{
			Reactor* r = set->Driven(x);
			if (showSelected && !gotMatch)
			{
				bool gotIt = false;
				for (int y = 0; y < reactors.Count(); y++)
				{
					if (r == reactors[y])
					{
						gotIt = true;
						break;
					}
				}
				if (!gotIt)
					continue;
			}
			listItem = new ReactionListItem(ReactionListItem::kDriven, r, set, x);
			if (AddDrivenRow(hList, listItem, itemID))
				itemID++;
		}
	}
	int itemCt = ListView_GetItemCount(hList);
	for (int x = itemCt; x > itemID; x--)
		ListView_DeleteItem(hList, x - 1);

	reactionListValid = true;
	SelectionChanged();
}

ReactionListItem* ReactionDlg::GetSelectedDriver()
{
	HWND hList = GetDlgItem(hWnd, IDC_REACTION_LIST);
	int count = ListView_GetItemCount(hList);
	if (count)
	{
		for (int item = 0; item < count; item++)
		{
			LV_ITEM lvItem;
			lvItem.iItem = item;
			lvItem.iSubItem = 0;
			lvItem.mask = LVIF_PARAM | LVIF_STATE;
			lvItem.stateMask = LVIS_SELECTED;
			BOOL bRet = ListView_GetItem(hList, &lvItem);
			if (lvItem.state & LVIS_SELECTED)
			{
				ReactionListItem* listItem = (ReactionListItem*)lvItem.lParam;
				if (listItem->GetReactionSet() != nullptr)
					return listItem;
			}
		}
	}
	return nullptr;
}

Tab<ReactionListItem*> ReactionDlg::GetSelectedDriven()
{
	Tab<ReactionListItem*> items;
	HWND hList = GetDlgItem(hWnd, IDC_REACTION_LIST);
	int count = ListView_GetItemCount(hList);
	if (count)
	{
		for (int item = 0; item < count; item++)
		{
			LV_ITEM lvItem;
			lvItem.iItem = item;
			lvItem.iSubItem = 0;
			lvItem.mask = LVIF_PARAM | LVIF_STATE;
			lvItem.stateMask = LVIS_SELECTED;
			BOOL bRet = ListView_GetItem(hList, &lvItem);
			if (lvItem.state & LVIS_SELECTED)
			{
				ReactionListItem* listItem = (ReactionListItem*)lvItem.lParam;
				if (listItem->Driven() != nullptr)
					items.Append(1, &listItem);
			}
		}
	}
	return items;
}

Tab<StateListItem*> ReactionDlg::GetSelectedStates()
{
	Tab<StateListItem*> states;
	HWND hList = GetDlgItem(hWnd, IDC_STATE_LIST);
	int count = ListView_GetItemCount(hList);
	if (count)
	{
		LV_ITEM lvItem;
		lvItem.iSubItem = 0;
		lvItem.mask = LVIF_PARAM | LVIF_STATE;
		lvItem.stateMask = LVIS_SELECTED;

		for (int item = 0; item < count; item++)
		{
			lvItem.iItem = item;
			BOOL bRet = ListView_GetItem(hList, &lvItem);
			if (lvItem.state & LVIS_SELECTED)
			{
				StateListItem* state = (StateListItem*)lvItem.lParam;
				states.Append(1, &state);
			}
		}
	}
	return states;
}

Tab<StateListItem*> ReactionDlg::GetSelectedDriverStates()
{
	Tab<StateListItem*> listItems = GetSelectedStates();
	Tab<StateListItem*> states;
	if (listItems.Count() > 0)
	{
		for (int i = 0; i < listItems.Count(); i++)
		{
			if (listItems[i]->GetDriverState() != nullptr)
				states.Append(1, &listItems[i]);
		}
	}
	return states;
}

Tab<StateListItem*> ReactionDlg::GetSelectedDrivenStates()
{
	Tab<StateListItem*> listItems = GetSelectedStates();
	Tab<StateListItem*> states;
	if (listItems.Count() > 0)
	{
		for (int i = 0; i < listItems.Count(); i++)
		{
			if (listItems[i]->GetDrivenState() != nullptr)
				states.Append(1, &listItems[i]);
		}
	}
	return states;
}

BOOL ReactionDlg::IsDriverStateSelected()
{
	HWND hList = GetDlgItem(hWnd, IDC_STATE_LIST);
	if (int count = ListView_GetItemCount(hList))
	{
		LV_ITEM lvItem;
		lvItem.iSubItem = 0;
		lvItem.mask = LVIF_PARAM | LVIF_STATE;
		lvItem.stateMask = LVIS_SELECTED;

		for (int item = 0; item < count; item++)
		{
			lvItem.iItem = item;
			BOOL bRet = ListView_GetItem(hList, &lvItem);
			if (lvItem.state & LVIS_SELECTED)
			{
				auto* state = reinterpret_cast<StateListItem*>(lvItem.lParam);
				if (state->GetDriverState() != nullptr)
					return TRUE;
			}
		}
	}
	return FALSE;
}

BOOL ReactionDlg::IsDrivenStateSelected()
{
	HWND hList = GetDlgItem(hWnd, IDC_STATE_LIST);
	if (int count = ListView_GetItemCount(hList))
	{
		LV_ITEM lvItem;
		lvItem.iSubItem = 0;
		lvItem.mask = LVIF_PARAM | LVIF_STATE;
		lvItem.stateMask = LVIS_SELECTED;

		for (int item = 0; item < count; item++)
		{
			lvItem.iItem = item;
			BOOL bRet = ListView_GetItem(hList, &lvItem);
			if (lvItem.state & LVIS_SELECTED)
			{
				StateListItem* state = (StateListItem*)lvItem.lParam;
				if (state->GetDrivenState() != nullptr)
					return TRUE;
			}
		}
	}
	return FALSE;
}


void ReactionDlg::SelectionChanged()
{
	blockInvalidation = true;
	selectionValid = true;
	UpdateStateList();
	UpdateButtons();
	UpdateStateButtons();
	UpdateCurveControl();
	UpdateCreateStates();
	blockInvalidation = false;
}

void ReactionDlg::StateSelectionChanged()
{
	blockInvalidation = true;
	stateSelectionValid = true;
	UpdateStateButtons();
	UpdateEditStates();
	blockInvalidation = false;
}

void ReactionDlg::RedrawControl()
{
	DbgAssert(iCCtrl != nullptr);
	if (iCCtrl != nullptr)
	{
		iCCtrl->Redraw();
	}
}
void ReactionDlg::EnableControlDraw(BOOL val)
{
	DbgAssert(iCCtrl != nullptr);
	if (iCCtrl != nullptr)
	{
		iCCtrl->EnableDraw(val);
	}
}

void ReactionDlg::SetControlXRange(float min, float max, BOOL rescaleKeys)
{
	DbgAssert(iCCtrl != nullptr);
	if (iCCtrl != nullptr)
	{
		iCCtrl->SetXRange(min, max, rescaleKeys);
	}
}
Point2 ReactionDlg::GetControlXRange()
{
	DbgAssert(iCCtrl != nullptr);
	return iCCtrl ? iCCtrl->GetXRange() : Point2::Origin;
}

void ReactionDlg::UpdateEditStates()
{
	bool changed = false;

	ICustButton* but = GetICustButton(GetDlgItem(hWnd, IDC_EDIT_STATE));
	BOOL isChecked = but->IsChecked();
	ReleaseICustButton(but);

	if (isChecked)
	{
		for (int i = 0; i < GetReactionManager()->ReactionCount(); i++)
		{
			ReactionSet* set = GetReactionManager()->GetReactionSet(i);
			for (int x = 0; x < set->DrivenCount(); x++)
			{
				if (Reactor* r = set->Driven(x))
				{
					r->setEditReactionMode(FALSE);
					changed = true;
				}
			}
		}
		Tab<StateListItem*> listItems = GetSelectedDrivenStates();
		if (listItems.Count() > 0)
		{
			for (int i = 0; i < listItems.Count(); i++)
			{
				Reactor* r = (Reactor*)listItems[i]->GetOwner();
				r->setEditReactionMode(TRUE);
				changed = true;
			}
		}
		else
		{
			listItems = GetSelectedDriverStates();
			if (listItems.Count() > 0)
			{
				ReactionSet* set = (ReactionSet*)listItems[0]->GetOwner();
				for (int i = 0; i < set->DrivenCount(); i++)
				{
					if (Reactor* r = set->Driven(i))
					{
						for (int x = 0; x < r->mDrivenStates.Count(); x++)
						{
							if (r->mDrivenStates[x].mDriverID == listItems[0]->GetIndex())
							{
								blockInvalidation = true;
								r->setSelected(x);
								r->setEditReactionMode(TRUE);
								blockInvalidation = false;
								changed = true;
							}
						}
					}
				}
			}
		}
	}

	if (changed)
		ip->RedrawViews(ip->GetTime());
}

void ReactionDlg::UpdateCreateStates()
{
	bool changed = false;
	ICustButton* but = GetICustButton(GetDlgItem(hWnd, IDC_CREATE_STATES));
	BOOL isChecked = but->IsChecked();
	ReleaseICustButton(but);

	if (isChecked)
	{
		for (int i = 0; i < GetReactionManager()->ReactionCount(); i++)
		{
			ReactionSet* set = GetReactionManager()->GetReactionSet(i);
			for (int x = 0; x < set->DrivenCount(); x++)
			{
				if (Reactor* r = set->Driven(x))
				{
					r->setCreateReactionMode(FALSE);
					changed = true;
				}
			}
		}
		Tab<ReactionListItem*> listItems = GetSelectedDriven();
		if (listItems.Count() > 0)
		{
			for (int i = 0; i < listItems.Count(); i++)
			{
				if (Reactor* r = listItems[i]->Driven())
				{
					r->setCreateReactionMode(TRUE);
					changed = true;
				}
			}
		}
		else
		{
			if (ReactionListItem* rListItem = GetSelectedDriver())
			{
				ReactionSet* set = rListItem->GetReactionSet();
				for (int i = 0; i < set->DrivenCount(); i++)
				{
					if (Reactor* r = set->Driven(i))
					{
						r->setCreateReactionMode(TRUE);
						changed = true;
					}
				}
			}
		}
	}
	if (changed)
		ip->RedrawViews(ip->GetTime());
}

void ReactionDlg::UpdateStateButtons()
{
	Tab<StateListItem*> listItems = GetSelectedStates();
	bool driverStateSelected = false;
	bool drivenStateSelected = false;

	ICustButton* setState = GetICustButton(GetDlgItem(hWnd, IDC_SET_STATE));
	ICustButton* deleteState = GetICustButton(GetDlgItem(hWnd, IDC_DELETE_STATE));
	ICustButton* editState = GetICustButton(GetDlgItem(hWnd, IDC_EDIT_STATE));
	ICustButton* appendState = GetICustButton(GetDlgItem(hWnd, IDC_APPEND_STATE));

	if (listItems.Count() > 0)
	{
		for (int i = 0; i < listItems.Count(); i++)
		{
			if (listItems[i]->GetDriverState() != nullptr)
				driverStateSelected = true;
			else
				drivenStateSelected = true;
		}
		deleteState->Enable(TRUE);
	}
	else
		deleteState->Enable(FALSE);


	if (driverStateSelected)
	{
		appendState->Enable(TRUE);
		setState->Enable(TRUE);
	}
	else
	{
		appendState->Enable(FALSE);
		setState->Enable(FALSE);
	}

	if (drivenStateSelected || driverStateSelected)
		editState->Enable(TRUE);
	else if (!editState->IsChecked())
		editState->Enable(FALSE);

	ReleaseICustButton(appendState);
	ReleaseICustButton(deleteState);
	ReleaseICustButton(setState);
	ReleaseICustButton(editState);
}

void ReactionDlg::UpdateStateList()
{
	HWND hList = GetDlgItem(hWnd, IDC_STATE_LIST);
	int itemID = 0;
	ReactionSet* set = nullptr;
	if (ReactionListItem* rListItem = GetSelectedDriver())
	{
		set = rListItem->GetReactionSet();
	}
	else
	{
		Tab<ReactionListItem*> listItems = GetSelectedDriven();
		if (listItems.Count() > 0)
		{
			set = (ReactionSet*)listItems[0]->GetOwner();
		}
	}
	if (set != nullptr)
	{
		if (ReactionDriver* driver = set->GetReactionDriver())
		{
			for (int i = 0; i < driver->states.Count(); i++)
			{
				DriverState* state = driver->states[i];
				auto* listItem = new StateListItem(StateListItem::kDriverState, state, set, i);

				LV_ITEM item;
				if (listItem->IsExpandable())
					item.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
				else
					item.mask = LVIF_TEXT | LVIF_PARAM;

				item.iItem = itemID;
				item.iSubItem = 0;
				item.pszText = const_cast<TCHAR*>(state->name.data());
				item.cchTextMax = static_cast<int>(_tcslen(state->name));
				if (listItem->IsExpandable())
				{
					item.stateMask = LVIS_STATEIMAGEMASK;
					item.state = INDEXTOSTATEIMAGEMASK(9);
				}
				item.lParam = (LPARAM)listItem;

				if (ListView_GetItemCount(hList) <= itemID)
					ListView_InsertItem(hList, &item);
				else
					ListView_SetItem(hList, &item);

				// LVITEM item;
				for (int k = 1; k < kNumStateColumns; k++)
				{
					item.mask = LVIF_PARAM;
					item.iSubItem = k;
					item.lParam = (LPARAM)listItem;
					ListView_SetItem(hList, &item);
				}

				itemID++;
				ReactionSet* set = (ReactionSet*)listItem->GetOwner();
				for (int k = 0; k < set->DrivenCount(); k++)
				{
					if (Reactor* r = set->Driven(k))
					{
						for (int x = 0; x < r->mDrivenStates.Count(); x++)
						{
							if (r->mDrivenStates[x].mDriverID == i)
							{
								listItem = new StateListItem(StateListItem::kDrivenState, nullptr, r, x);
								if (AddDrivenStateRow(hList, listItem, itemID))
								{
									itemID++;
									break;
								}
							}
						}
					}
				}
			}
		}
	}
	int itemCt = ListView_GetItemCount(hList);
	for (int x = itemCt; x > itemID; x--)
		ListView_DeleteItem(hList, x - 1);
}

void ReactionDlg::ClearCurves()
{
	iCCtrl->DeleteAllCurves();
}

void ReactionDlg::UpdateCurveControl()
{
	Tab<ReactionListItem*> drivenItems = GetSelectedDriven();
	Tab<Reactor*> reactors;

	// Get the Reactor pointers
	if (drivenItems.Count() > 0)
	{
		for (int x = 0; x < drivenItems.Count(); x++)
		{
			if (Reactor* r = drivenItems[x]->Driven())
				reactors.Append(1, &r);
		}
	}
	else
	{
		if (ReactionListItem* rListItem = GetSelectedDriver())
		{
			ReactionSet* set = rListItem->GetReactionSet();
			for (int x = 0; x < set->DrivenCount(); x++)
			{
				if (Reactor* r = set->Driven(x))
					reactors.Append(1, &r);
			}
		}
	}

	HoldSuspend hs;

	Tab<ICurve*> curves;
	float min = 0.0f;
	float max = 0.0f;
	bool initialized = false;
	// Get the curves from the Reactor pointers
	for (int x = 0; x < reactors.Count(); x++)
	{
		Reactor* r = reactors[x];
		if (r && (r->iCCtrl != nullptr && r->isCurveControlled))
		{
			r->UpdateCurves(false);
			int numCurves = r->iCCtrl->GetNumCurves();
			for (int i = 0; i < numCurves; i++)
			{
				ICurve* curve = r->iCCtrl->GetControlCurve(i);
				if (!initialized)
				{
					min = curve->GetPoint(0, 0).p.x;
					max = curve->GetPoint(0, curve->GetNumPts() - 1).p.x;
					initialized = true;
				}
				else
				{
					min = std::min(curve->GetPoint(0, 0).p.x, min);
					max = std::max(curve->GetPoint(0, curve->GetNumPts() - 1).p.x, max);
				}
				curves.Append(1, &curve);
			}
		}
	}

	int curveCount = curves.Count();
	// Here the Reaction Manager Dialog's UI CurveControl widget
	// gives up display responsibilities (i.e. ownership) of the Reactor's ICurves/Curves. It also
	// deletes any curves it may have previously owned.
	ClearCurves();
	iCCtrl->SetNumCurves(curveCount);

	BitArray ba;
	ba.SetSize(curveCount);
	ba.SetAll();
	iCCtrl->SetDisplayMode(ba);


	for (int i = 0; i < curveCount; i++)
	{
		// Here the Reaction Manager Dialog's UI CurveControl widget
		// takes display responsibilities (i.e. ownership) of the Reactor's ICurves/Curves.
		iCCtrl->ReplaceReference(i, curves[i]);
	}

	if (min == max)
		max = min + 1.0f;
	iCCtrl->SetXRange(min, max, FALSE);
	iCCtrl->ZoomExtents();
}

void ReactionDlg::UpdateButtons()
{
	ICustButton* addDriven = GetICustButton(GetDlgItem(hWnd, IDC_ADD_DRIVEN));
	ICustButton* addSel = GetICustButton(GetDlgItem(hWnd, IDC_ADD_SELECTED));
	ICustButton* deleteSel = GetICustButton(GetDlgItem(hWnd, IDC_DELETE_SELECTED));
	ICustButton* newState = GetICustButton(GetDlgItem(hWnd, IDC_NEW_STATE));
	ICustButton* createStates = GetICustButton(GetDlgItem(hWnd, IDC_CREATE_STATES));

	if (ReactionListItem* driverItem = GetSelectedDriver())
	{
		if (static_cast<ReactionSet*>(driverItem->GetOwner())->GetReactionDriver() != nullptr)
		{
			addDriven->Enable(TRUE);
			addSel->Enable(TRUE);
			newState->Enable(TRUE);
			createStates->Enable(TRUE);
		}
		else
		{
			addDriven->Enable(FALSE);
			addSel->Enable(FALSE);
			newState->Enable(FALSE);
			createStates->Enable(FALSE);
		}

		deleteSel->Enable(TRUE);
	}
	else
	{
		addDriven->Enable(FALSE);
		addSel->Enable(FALSE);
		Tab<ReactionListItem*> drivenItems = GetSelectedDriven();
		if (drivenItems.Count() > 0)
		{
			deleteSel->Enable(TRUE);
			bool aborted = false;
			for (int i = 0; i < drivenItems.Count(); i++)
			{
				if (drivenItems[i]->Driven() && drivenItems[i]->Driven()->GetReactionDriver() == nullptr)
				{
					createStates->Enable(FALSE);
					newState->Enable(FALSE);
					aborted = true;
					break;
				}
			}
			if (!aborted)
			{
				createStates->Enable(TRUE);
				newState->Enable(TRUE);
			}
		}
		else
		{
			deleteSel->Enable(FALSE);
			newState->Enable(FALSE);
			if (!createStates->IsChecked())
			{
				createStates->Disable();
			}
		}
	}

	ReleaseICustButton(addDriven);
	ReleaseICustButton(addSel);
	ReleaseICustButton(deleteSel);
	ReleaseICustButton(newState);
	ReleaseICustButton(createStates);
}

// provides hit testing into the grid system of the list control
POINT ReactionDlg::HitTest(POINT& p, UINT id)
{
	HWND hList = GetDlgItem(hWnd, id);

	// Check which cell was hit
	LV_HITTESTINFO hittestinfo;
	POINT ptGrid;
	ptGrid.x = -1;
	ptGrid.y = -1;
	hittestinfo.pt.x = p.x;
	hittestinfo.pt.y = p.y;
	int nRow = ListView_SubItemHitTest(hList, &hittestinfo);
	int nCol = 0;
	bool found = false;

	if (hittestinfo.flags & LVHT_ONITEMSTATEICON)
	{
		nCol = -1;
		nRow = hittestinfo.iItem;
		found = true;
	}
	else if (hittestinfo.flags & LVHT_ONITEM)
	{
		nCol = hittestinfo.iSubItem;
		nRow = hittestinfo.iItem;
		found = true;
	}

	if (found)
	{
		ptGrid.x = nRow;
		ptGrid.y = nCol;
	}

	return ptGrid;
}

ReactionListItem* ReactionDlg::GetReactionDataAt(int where)
{
	HWND hList = GetDlgItem(hWnd, IDC_REACTION_LIST);

	LV_ITEM lvItem;
	lvItem.iItem = where;
	lvItem.cchTextMax = kMaxTextLength;
	lvItem.iSubItem = 0;
	lvItem.mask = LVIF_PARAM;
	BOOL bRet = ListView_GetItem(hList, &lvItem);
	return bRet ? reinterpret_cast<ReactionListItem*>(lvItem.lParam) : nullptr;
}

StateListItem* ReactionDlg::GetStateDataAt(int where)
{
	HWND hList = GetDlgItem(hWnd, IDC_STATE_LIST);

	LV_ITEM lvItem;
	lvItem.iItem = where;
	lvItem.cchTextMax = kMaxTextLength;
	lvItem.iSubItem = 0;
	lvItem.mask = LVIF_PARAM;
	BOOL bRet = ListView_GetItem(hList, &lvItem);
	return bRet ? reinterpret_cast<StateListItem*>(lvItem.lParam) : nullptr;
}

bool ReactionDlg::InsertListItem(ReactionListItem* pItem, int after)
{
	HWND hList = GetDlgItem(hWnd, IDC_REACTION_LIST);

	// Get the next row number
	if (after == -1)
		after = ListView_GetItemCount(hList);

	return AddDrivenRow(hList, pItem, after);
}

void ReactionDlg::InsertDrivenInList(ReactionSet* set, int after)
{
	ReactionListItem* pItemInfo = nullptr;
	for (int i = 0; i < set->DrivenCount(); i++)
	{
		pItemInfo = new ReactionListItem(ReactionListItem::kDriven, set->Driven(i), set, i);
		if (InsertListItem(pItemInfo, after))
			after++;
	}
}

void ReactionDlg::RemoveDrivenFromList(ReactionSet* set, int after)
{
	HWND hList = GetDlgItem(hWnd, IDC_REACTION_LIST);

	int mark = -1;
	int count = ListView_GetItemCount(hList);
	LV_ITEM lvItem;

	if (after >= count)
		return;

	lvItem.iItem = after + 1;
	lvItem.cchTextMax = kMaxTextLength;
	lvItem.iSubItem = 0;
	lvItem.mask = LVIF_PARAM;
	BOOL bRet = ListView_GetItem(hList, &lvItem);
	assert(bRet);
	if (!bRet)
		return;
	ReactionListItem* pItemData = (ReactionListItem*)lvItem.lParam;
	while (pItemData->Driven() != nullptr)
	{
		ListView_DeleteItem(hList, after + 1);
		bRet = ListView_GetItem(hList, &lvItem);
		if (bRet)
			pItemData = (ReactionListItem*)lvItem.lParam;
		else
			break;
	}
}

bool ReactionDlg::ToggleReactionExpansionState(int nRow)
{
	HWND hList = GetDlgItem(hWnd, IDC_REACTION_LIST);

	RECT rectItem;
	LV_ITEM lvItem;
	lvItem.iItem = nRow;
	lvItem.iSubItem = 0;
	lvItem.mask = LVIF_PARAM;
	BOOL bRet = ListView_GetItem(hList, &lvItem);
	ReactionListItem* pItemInfo = (ReactionListItem*)lvItem.lParam;
	if (!pItemInfo->IsExpandable())
		return false;

	if (ReactionSet* driver = pItemInfo->GetReactionSet())
	{
		if (!pItemInfo->IsExpanded())
		{
			InsertDrivenInList(driver, nRow + 1);
			pItemInfo->Expand(true);
		}
		else
		{
			RemoveDrivenFromList(driver, nRow);
			pItemInfo->Expand(false);
		}
	}

	ListView_SetItemState(hList, nRow, INDEXTOSTATEIMAGEMASK(pItemInfo->IsExpanded() ? 9 : 8), LVIS_STATEIMAGEMASK);
	ListView_GetItemRect(hList, nRow, &rectItem, LVIR_SELECTBOUNDS);
	InvalidateRect(hList, &rectItem, TRUE);
	// UpdateWindow(hList);
	return true;
}

bool ReactionDlg::InsertListItem(StateListItem* pItem, int after)
{
	HWND hList = GetDlgItem(hWnd, IDC_STATE_LIST);

	// Get the next row number
	if (after == -1)
		after = ListView_GetItemCount(hList);

	return AddDrivenStateRow(hList, pItem, after);
}

void ReactionDlg::InsertStatesInList(DriverState* state, ReactionSet* set, int after)
{
	StateListItem* pItemInfo = nullptr;
	for (int k = 0; k < set->DrivenCount(); k++)
	{
		if (Reactor* r = set->Driven(k))
		{
			for (int x = 0; x < r->mDrivenStates.Count(); x++)
			{
				if (r->getDriverState(x) == state)
				{
					pItemInfo = new StateListItem(StateListItem::kDrivenState, &r->mDrivenStates[x], r, x);
					if (InsertListItem(pItemInfo, after))
						after++;
				}
			}
		}
	}
}

void ReactionDlg::RemoveStatesFromList(DriverState* state, int after)
{
	HWND hList = GetDlgItem(hWnd, IDC_STATE_LIST);

	int mark = -1;
	int count = ListView_GetItemCount(hList);
	LV_ITEM lvItem;

	if (after >= count)
		return;

	lvItem.iItem = after + 1;
	lvItem.cchTextMax = kMaxTextLength;
	lvItem.iSubItem = 0;
	lvItem.mask = LVIF_PARAM;
	BOOL bRet = ListView_GetItem(hList, &lvItem);
	assert(bRet);
	if (!bRet)
		return;

	auto* pItemData = reinterpret_cast<StateListItem*>(lvItem.lParam);
	while (pItemData->GetDrivenState() != nullptr)
	{
		ListView_DeleteItem(hList, after + 1);
		bRet = ListView_GetItem(hList, &lvItem);
		if (bRet)
			pItemData = reinterpret_cast<StateListItem*>(lvItem.lParam);
		else
			break;
	}
}

bool ReactionDlg::ToggleStateExpansionState(int nRow)
{
	HWND hList = GetDlgItem(hWnd, IDC_STATE_LIST);

	RECT rectItem;
	LV_ITEM lvItem;
	lvItem.iItem = nRow;
	lvItem.iSubItem = 0;
	lvItem.mask = LVIF_PARAM;
	BOOL bRet = ListView_GetItem(hList, &lvItem);
	StateListItem* pItemInfo = (StateListItem*)lvItem.lParam;
	if (!pItemInfo->IsExpandable())
		return false;

	if (DriverState* state = pItemInfo->GetDriverState())
	{
		if (!pItemInfo->IsExpanded())
		{
			auto* owner = static_cast<ReactionSet*>(pItemInfo->GetOwner());
			InsertStatesInList(state, owner, nRow + 1);
			pItemInfo->Expand(true);
		}
		else
		{
			RemoveStatesFromList(state, nRow);
			pItemInfo->Expand(false);
		}
	}

	ListView_SetItemState(hList, nRow, INDEXTOSTATEIMAGEMASK(pItemInfo->IsExpanded() ? 9 : 8), LVIS_STATEIMAGEMASK);
	ListView_GetItemRect(hList, nRow, &rectItem, LVIR_SELECTBOUNDS);
	InvalidateRect(hList, &rectItem, TRUE);
	// UpdateWindow(hList);
	return true;
}


void ReactionDlg::Update()
{
	valid = TRUE;
	// iCCtrl->Redraw();
}

bool ReactionListItem::GetRange(TimeValue& t, bool start)
{
	Interval iv;
	bool match = false;

	switch (mType)
	{
	case kReactionSet:
	{
		if (ReactionSet* set = GetReactionSet())
		{
			for (int i = 0; i < set->DrivenCount(); i++)
			{
				if (Reactor* reactor = set->Driven(i))
				{
					if (i == 0)
					{
						match = true;
						iv = reactor->GetTimeRange(TIMERANGE_ALL);
					}
					else
					{
						if (iv.Start() != reactor->GetTimeRange(TIMERANGE_ALL).Start())
						{
							match = false;
							break;
						}
					}
				}
			}
		}
	}
	break;
	case kDriven:
	{
		if (Reactor* reactor = Driven())
		{
			match = true;
			iv = reactor->GetTimeRange(TIMERANGE_ALL);
		}
	}
	break;
	default:
		assert(0);
	}
	if (match)
	{
		t = start ? iv.Start() : iv.End();
		return true;
	}
	return false;
}

void ReactionListItem::SetRange(TimeValue t, bool start)
{
	Interval iv;
	bool match = false;

	switch (mType)
	{
	case kReactionSet:
	{
		// if (ReactionSet* set = GetReactionSet())
		//{
		//	for(int i = 0; i < set->DrivenCount(); i++)
		//	{
		//		Reactor* reactor = set->Driven(i);
		//		if (i == 0)
		//		{
		//			match = true;
		//			iv = reactor->GetTimeRange(TIMERANGE_ALL);
		//		}
		//		else
		//		{
		//			if (iv.Start() != reactor->GetTimeRange(TIMERANGE_ALL).Start())
		//			{
		//				match = false;
		//				break;
		//			}
		//		}
		//	}
		//}
	}
	break;
	case kDriven:
	{
		if (Reactor* reactor = Driven())
		{
			iv = reactor->GetTimeRange(TIMERANGE_ALL);
			if (start)
				iv.SetStart(t);
			else
				iv.SetEnd(t);
			reactor->EditTimeRange(iv, TIMERANGE_ALL);
		}
	}
	break;
	default:
		assert(0);
	}
}

void ReactionListItem::GetValue(void* val)
{
	if (mType == kReactionSet)
	{
		ReactionDriver* driver = GetReactionSet()->GetReactionDriver();
		if (driver == nullptr)
			return;

		ReferenceTarget* owner = driver->Owner();
		int subnum = driver->SubIndex();
		if (subnum < 0)
		{
			if (subnum == IKTRACK)
			{
				Quat q;
				GetAbsoluteControlValue((INode*)owner, subnum, GetCOREInterface()->GetTime(), &q);
				*(Quat*)val = q;
			}
			else
			{
				Point3 p;
				GetAbsoluteControlValue((INode*)owner, subnum, GetCOREInterface()->GetTime(), &p);
				*(Point3*)val = p;
			}
		}
		else
		{
			if (Control* c = GetControlInterface(owner->SubAnim(subnum)))
			{
				switch (driver->GetType())
				{
				case FLOAT_VAR:
				{
					float f;
					c->GetValue(GetCOREInterface()->GetTime(), &f, FOREVER);
					*(float*)val = f;
				}
				break;
				case VECTOR_VAR:
				{
					Point3 p;
					c->GetValue(GetCOREInterface()->GetTime(), &p, FOREVER);
					*(Point3*)val = p;
				}
				break;
				case SCALE_VAR:
				{
					ScaleValue s;
					c->GetValue(GetCOREInterface()->GetTime(), &s, FOREVER);
					*(Point3*)val = s.s;
				}
				break;
				case QUAT_VAR:
				{
					Quat q;
					c->GetValue(GetCOREInterface()->GetTime(), &q, FOREVER);
					*(Quat*)val = q;
				}
				break;
				default:
					val = nullptr;
					break;
				}
			}
		}
	}
	else
	{
		if (Reactor* r = Driven())
		{
			switch (r->type)
			{
			case REACTORFLOAT:
				*(float*)val = r->curfval;
				break;
			case REACTORPOS:
			case REACTORP3:
			case REACTORSCALE:
				*(Point3*)val = r->curpval;
				break;
			case REACTORROT:
				*(Quat*)val = r->curqval;
				break;
			default:
				val = nullptr;
				break;
			}
		}
	}
}

static void RemoveDrivenFromScene(Reactor* r)
{
	if (theHold.Holding())
		theHold.Put(new DlgUpdateRestore());

	MyEnumProc dep(r);
	((ReferenceTarget*)r)->DoEnumDependents(&dep);

	for (int x = 0; x < dep.anims.Count(); x++)
	{
		for (int i = 0; i < dep.anims[x]->NumSubs(); i++)
		{
			Animatable* n = dep.anims[x]->SubAnim(i);
			if (n == r)
			{
				Animatable* parent = dep.anims[x];
				int subAnim = i;

				Control* cont = nullptr;
				switch (r->SuperClassID())
				{
				case CTRL_POSITION_CLASS_ID:
					cont = NewDefaultPositionController();
					break;
				case CTRL_POINT3_CLASS_ID:
					cont = NewDefaultPoint3Controller();
					break;
				case CTRL_ROTATION_CLASS_ID:
					cont = NewDefaultRotationController();
					break;
				case CTRL_SCALE_CLASS_ID:
					cont = NewDefaultScaleController();
					break;
				case CTRL_FLOAT_CLASS_ID:
					cont = NewDefaultFloatController();
					break;
				}

				if (cont != nullptr)
				{
					cont->Copy(r);
					parent->AssignController(cont, subAnim);
				}
			}
		}
	}
}

struct StateIndexPair
{
	DriverState* mState = nullptr;
	int index;
};

void ReactionDlg::WMCommand(int id, int notify, HWND hCtrl)
{
	switch (id)
	{
	case IDC_ADD_DRIVER:
	{
		ICustButton* but = GetICustButton(hCtrl);
		if (mCurPickMode)
		{
			GetCOREInterface()->ClearPickMode();
			delete mCurPickMode;
			mCurPickMode = nullptr;
		}
		if (but->IsChecked())
		{
			mCurPickMode = new ReactionDriverPickMode(but);
			GetCOREInterface()->SetPickMode(mCurPickMode);
			// button is released by the pick mode
		}
	}
	break;
	case IDC_REPLACE_DRIVER:
	{
		if (mCurPickMode)
		{
			GetCOREInterface()->ClearPickMode();
			delete mCurPickMode;
			mCurPickMode = nullptr;
		}
		if (ReactionListItem* listItem = GetSelectedDriver())
		{
			ICustButton* but = GetICustButton(GetDlgItem(hWnd, IDC_ADD_DRIVER));
			mCurPickMode = new ReplaceDriverPickMode(listItem->GetReactionSet(), but);
			GetCOREInterface()->SetPickMode(mCurPickMode);
			// button is released by the pick mode
		}
	}
	break;
	case IDC_ADD_DRIVEN:
	{
		if (mCurPickMode)
		{
			GetCOREInterface()->ClearPickMode();
			delete mCurPickMode;
			mCurPickMode = nullptr;
		}
		ICustButton* but = GetICustButton(hCtrl);
		if (but->IsChecked())
		{
			if (ReactionListItem* listItem = GetSelectedDriver())
			{
				Tab<int> indices;
				Tab<StateListItem*> sListItems = GetSelectedStates();
				for (int i = 0; i < sListItems.Count(); i++)
				{
					if (sListItems[i]->GetDriverState() != nullptr)
					{
						int index = sListItems[i]->GetIndex();
						indices.Append(1, &index);
					}
				}
				mCurPickMode = new ReactionDrivenPickMode(listItem->GetReactionSet(), indices, but);
				GetCOREInterface()->SetPickMode(mCurPickMode);
			}
			else
				but->SetCheck(FALSE);
		}
	}
	break;
	case IDC_ADD_SELECTED:
	{
		Tab<ReferenceTarget*> selectedNodes;
		for (int x = 0; x < GetCOREInterface()->GetSelNodeCount(); x++)
		{
			ReferenceTarget* targ = GetCOREInterface()->GetSelNode(x);
			selectedNodes.Append(1, &targ);
		}
		if (selectedNodes.Count() > 0)
		{
			if (ReactionListItem* listItem = GetSelectedDriver())
			{
				Tab<int> indices;
				Tab<StateListItem*> sListItems = GetSelectedStates();
				for (int i = 0; i < sListItems.Count(); i++)
				{
					if (sListItems[i]->GetDriverState() != nullptr)
					{
						int index = sListItems[i]->GetIndex();
						indices.Append(1, &index);
					}
				}

				ReactionDrivenPickMode cb(listItem->GetReactionSet(), indices, nullptr);
				theAnimPopupMenu.DoPopupMenu(&cb, selectedNodes);
			}
		}
	}
	break;
	case IDC_DELETE_SELECTED:
	{
		ReactionListItem* listItem = GetSelectedDriver();
		Tab<ReactionListItem*> drivenItems = GetSelectedDriven();

		if (drivenItems.Count() > 0 || listItem != nullptr)
		{
			theHold.Begin();

			for (int i = 0; i < drivenItems.Count(); i++)
			{
				RemoveDrivenFromScene(drivenItems[i]->Driven());
			}
			if (listItem != nullptr)
			{
				if (ReactionSet* set = listItem->GetReactionSet())
				{
					for (int i = set->DrivenCount() - 1; i >= 0; i--)
					{
						RemoveDrivenFromScene(set->Driven(i));
					}
					GetReactionManager()->RemoveTarget(set);
				}
			}
			theHold.Accept(GetString(IDS_DELETE_SELECTED));
			InvalidateReactionList();
		}
	}
	break;
	case IDC_SHOW_SELECTED:
	case IDC_REFRESH:
	{
		ICustButton* but = GetICustButton(GetDlgItem(hWnd, IDC_SHOW_SELECTED));
		BOOL isChecked = but->IsChecked();
		ReleaseICustButton(but);

		selectedNodes.SetCount(0);
		if (isChecked)
		{
			for (int x = 0; x < GetCOREInterface()->GetSelNodeCount(); x++)
			{
				INode* targ = GetCOREInterface()->GetSelNode(x);
				selectedNodes.Append(1, &targ);
			}
		}

		InvalidateReactionList();

		but = GetICustButton(GetDlgItem(hWnd, IDC_REFRESH));
		but->Disable();
		ReleaseICustButton(but);
	}
	break;
	case IDC_NEW_STATE:
	{
		ReactionListItem* listItem = GetSelectedDriver();
		Tab<ReactionListItem*> driven = GetSelectedDriven();
		theHold.Begin();
		ReactionSet* set = nullptr;
		if (listItem)
			set = listItem->GetReactionSet();
		else if (driven.Count() > 0)
			set = static_cast<ReactionSet*>(driven[0]->GetOwner());

		if (set != nullptr)
		{
			if (ReactionDriver* driver = set->GetReactionDriver())
			{
				int index;
				driver->AddState(nullptr, index, true, ip->GetTime());
				if (driven.Count() > 0)
				{
					for (int i = 0; i < driven.Count(); i++)
					{
						if (Reactor* r = driven[i]->Driven())
							r->CreateReactionAndReturn(TRUE, nullptr, ip->GetTime(), index);
					}
				}
				else
				{
					for (int i = 0; i < set->DrivenCount(); i++)
					{
						if (Reactor* r = set->Driven(i))
							r->CreateReactionAndReturn(TRUE, nullptr, ip->GetTime(), index);
					}
				}
				InvalidateSelection();
			}
		}
		theHold.Accept(GetString(IDS_NEW_STATE));
	}
	break;
	case IDC_APPEND_STATE:
	{
		Tab<StateListItem*> sListItems = GetSelectedStates();
		Tab<StateIndexPair> states;

		for (int k = 0; k < sListItems.Count(); k++)
		{
			if (DriverState* state = sListItems[k]->GetDriverState())
			{
				StateIndexPair pair;
				pair.mState = state;
				pair.index = sListItems[k]->GetIndex();
				states.Append(1, &pair);
			}
		}

		if (states.Count() > 0)
		{
			theHold.Begin();
			ReactionListItem* listItem = GetSelectedDriver();
			Tab<ReactionListItem*> driven = GetSelectedDriven();

			ReactionSet* set = nullptr;
			if (listItem)
				set = listItem->GetReactionSet();
			else if (driven.Count() > 0)
				set = static_cast<ReactionSet*>(driven[0]->GetOwner());

			if (set != nullptr)
			{
				if (ReactionDriver* driver = set->GetReactionDriver())
				{
					if (driven.Count() > 0)
					{
						for (int i = 0; i < driven.Count(); i++)
						{
							if (Reactor* r = driven[i]->Driven())
							{
								for (int j = 0; j < states.Count(); j++)
								{
									int x;
									for (x = 0; x < r->mDrivenStates.Count(); x++)
									{
										if (r->getDriverState(x) == states[j].mState)
										{
											break;
										}
									}
									if (x == r->mDrivenStates.Count())
									{
										this->selectionValid = false;
										r->CreateReactionAndReturn(TRUE, nullptr, ip->GetTime(), states[j].index);
									}
								}
							}
						}
					}
					else
					{
						for (int i = 0; i < set->DrivenCount(); i++)
						{
							if (Reactor* r = set->Driven(i))
							{
								for (int j = 0; j < states.Count(); j++)
								{
									int x;
									for (x = 0; x < r->mDrivenStates.Count(); x++)
									{
										if (r->getDriverState(x) == states[j].mState)
										{
											break;
										}
									}
									if (x == r->mDrivenStates.Count())
									{
										this->selectionValid = false;
										r->CreateReactionAndReturn(TRUE, nullptr, ip->GetTime(), states[j].index);
									}
								}
							}
						}
					}
					InvalidateSelection();
				}
			}
			theHold.Accept(GetString(IDS_NEW_STATE));
		}
	}
	break;
	case IDC_SET_STATE:
	{
		Tab<StateListItem*> sListItems = GetSelectedDriverStates();
		Tab<StateIndexPair> states;

		theHold.Begin();

		for (int k = 0; k < sListItems.Count(); k++)
		{
			DriverState* state = sListItems[k]->GetDriverState();
			StateIndexPair pair;
			pair.mState = state;
			pair.index = sListItems[k]->GetIndex();
			states.Append(1, &pair);
		}

		if (states.Count() > 0)
		{
			if (ReactionListItem* listItem = GetSelectedDriver())
			{
				DriverState* state = states[0].mState;
				ReactionSet* set = listItem->GetReactionSet();
				if (ReactionDriver* driver = set->GetReactionDriver())
				{
					switch (driver->GetType())
					{
					case FLOAT_VAR:
					{
						float f;
						listItem->GetValue(&f);
						driver->SetState(states[0].index, f);
					}
					break;
					case SCALE_VAR:
					case VECTOR_VAR:
					{
						Point3 p;
						listItem->GetValue(&p);
						driver->SetState(states[0].index, p);
					}
					break;
					case QUAT_VAR:
					{
						Quat q;
						listItem->GetValue(&q);
						driver->SetState(states[0].index, q);
					}
					break;
					}
					driver->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
				}
			}
			Tab<ReactionListItem*> driven = GetSelectedDriven();
			for (int j = 0; j < states.Count(); j++)
			{
				for (int i = 0; i < driven.Count(); i++)
				{
					if (Reactor* r = driven[i]->Driven())
					{
						int x;
						for (x = 0; x < r->mDrivenStates.Count(); x++)
						{
							if (r->getDriverState(x) == states[j].mState)
							{
								break;
							}
						}
						if (x == r->mDrivenStates.Count())
						{
							this->selectionValid = false;
							r->CreateReactionAndReturn(TRUE, nullptr, ip->GetTime(), states[j].index);
						}
					}
				}
			}
		}
		theHold.Accept(GetString(IDS_SET_STATE));
	}
	break;
	case IDC_DELETE_STATE:
	{
		theHold.Begin();
		Tab<StateListItem*> sListItems = GetSelectedDriverStates();
		for (int k = sListItems.Count() - 1; k >= 0; k--)
		{
			DriverState* state = sListItems[k]->GetDriverState();
			ReactionSet* set = (ReactionSet*)sListItems[k]->GetOwner();
			set->GetReactionDriver()->DeleteState(sListItems[k]->GetIndex());
		}

		sListItems = GetSelectedDrivenStates();
		for (int k = sListItems.Count() - 1; k >= 0; k--)
		{
			int id = sListItems[k]->GetDrivenState()->mDriverID;
			Reactor* r = (Reactor*)sListItems[k]->GetOwner();
			for (int x = r->mDrivenStates.Count() - 1; x >= 0; x--)
			{
				if (r->mDrivenStates[x].mDriverID == id)
				{
					r->DeleteReaction(x);
					break;
				}
			}
		}

		theHold.Accept(GetString(IDS_DELETE_STATE));
	}
	break;
	case IDC_EDIT_STATE:
	{
		ICustButton* thisBut = GetICustButton(GetDlgItem(hWnd, IDC_EDIT_STATE));
		ICustButton* but = GetICustButton(GetDlgItem(hWnd, IDC_CREATE_STATES));
		if (thisBut->IsChecked() && but->IsChecked())
		{
			but->SetCheck(FALSE);
			WMCommand(IDC_CREATE_STATES, 0, but->GetHwnd());
		}
		ReleaseICustButton(but);
		ReleaseICustButton(thisBut);

		but = GetICustButton(GetDlgItem(hWnd, IDC_EDIT_STATE));
		Tab<StateListItem*> listItems = GetSelectedDrivenStates();
		BOOL editing = but->IsChecked();
		bool changed = false;
		if (listItems.Count() > 0)
		{
			for (int i = 0; i < listItems.Count(); i++)
			{
				Reactor* r = (Reactor*)listItems[i]->GetOwner();
				r->setEditReactionMode(editing);
				changed = true;
				if (editing && i == 0)
				{
					switch (r->type)
					{
					case REACTORPOS:
					case REACTORP3:
						ip->SetStdCommandMode(CID_OBJMOVE);
						break;
					case REACTORROT:
						ip->SetStdCommandMode(CID_OBJROTATE);
						break;
					case REACTORSCALE:
						ip->SetStdCommandMode(CID_OBJSCALE);
						break;
					default:
						break;
					}
				}
			}
		}
		else
		{
			listItems = GetSelectedDriverStates();
			if (listItems.Count() > 0)
			{
				ReactionSet* set = (ReactionSet*)listItems[0]->GetOwner();
				for (int i = 0; i < set->DrivenCount(); i++)
				{
					if (Reactor* r = set->Driven(i))
					{
						for (int x = 0; x < r->mDrivenStates.Count(); x++)
						{
							if (r->mDrivenStates[x].mDriverID == listItems[0]->GetIndex())
							{
								blockInvalidation = true;
								r->setSelected(x);
								r->setEditReactionMode(editing);
								blockInvalidation = false;
								if (editing && !changed)
								{
									switch (r->type)
									{
									case REACTORPOS:
									case REACTORP3:
										ip->SetStdCommandMode(CID_OBJMOVE);
										break;
									case REACTORROT:
										ip->SetStdCommandMode(CID_OBJROTATE);
										break;
									case REACTORSCALE:
										ip->SetStdCommandMode(CID_OBJSCALE);
										break;
									default:
										break;
									}
								}
								changed = true;
							}
						}
					}
				}
			}
		}

		if (changed)
			ip->RedrawViews(ip->GetTime());
		else if (!editing)
			but->Disable();

		ReleaseICustButton(but);
	}
	break;
	case IDC_CREATE_STATES:
	{
		ICustButton* but = GetICustButton(GetDlgItem(hWnd, IDC_CREATE_STATES));
		Tab<ReactionListItem*> drivenItems = GetSelectedDriven();
		BOOL creating = but->IsChecked();
		if (!creating && drivenItems.Count() == 0)
			but->Disable();
		ReleaseICustButton(but);

		if (creating)
		{
			but = GetICustButton(GetDlgItem(hWnd, IDC_EDIT_STATE));
			if (but->IsChecked())
			{
				WMCommand(IDC_EDIT_STATE, 0, but->GetHwnd());
				but->SetCheck(FALSE);
			}
			ReleaseICustButton(but);
		}

		bool changed = false;
		if (drivenItems.Count() > 0)
		{
			for (int i = 0; i < drivenItems.Count(); i++)
			{
				if (Reactor* r = drivenItems[i]->Driven())
				{
					r->setCreateReactionMode(creating);
					if (creating && i == 0)
					{
						switch (r->type)
						{
						case REACTORPOS:
						case REACTORP3:
							ip->SetStdCommandMode(CID_OBJMOVE);
							break;
						case REACTORROT:
							ip->SetStdCommandMode(CID_OBJROTATE);
							break;
						case REACTORSCALE:
							ip->SetStdCommandMode(CID_OBJSCALE);
							break;
						default:
							break;
						}
					}
				}
			}
		}
		else
		{
			if (ReactionListItem* rListItem = GetSelectedDriver())
			{
				ReactionSet* set = rListItem->GetReactionSet();
				for (int i = 0; i < set->DrivenCount(); i++)
				{
					if (Reactor* r = set->Driven(i))
					{
						r->setCreateReactionMode(creating);
						if (creating && i == 0)
						{
							switch (r->type)
							{
							case REACTORPOS:
							case REACTORP3:
								ip->SetStdCommandMode(CID_OBJMOVE);
								break;
							case REACTORROT:
								ip->SetStdCommandMode(CID_OBJROTATE);
								break;
							case REACTORSCALE:
								ip->SetStdCommandMode(CID_OBJSCALE);
								break;
							default:
								break;
							}
						}
					}
				}
			}
		}
		if (!creating)
			ip->RedrawViews(ip->GetTime());
	}
	break;
	default:
		break;
	}
}

BOOL ReactionDlg::OnLMouseButtonDown(HWND hDlg, WPARAM wParam, POINT lParam, UINT id)
{
	int nRow;
	int nCol;
	POINT hit;

	POINT pt;
	pt.x = lParam.x; // horizontal position of cursor
	pt.y = lParam.y; // vertical position of cursor

	// Map mouse click to the controls grid
	hit = HitTest(pt, id);
	nRow = hit.x;
	nCol = hit.y;

	if (nRow == -1)
		return FALSE;

	BOOL ret = FALSE;
	if (id == IDC_REACTION_LIST)
	{
		switch (nCol)
		{
		case kExpandCol:
			if (ToggleReactionExpansionState(nRow))
			{
				ret = TRUE;
			}
			nCol = hit.y = kNameCol;
			// fall through
		case kNameCol: // name
		{
		}
		break;
		case kFromCol:
		{
			ReactionListItem* listItem = GetReactionListItem(GetDlgItem(hWnd, IDC_REACTION_LIST), nRow, nCol);
			if (listItem->GetRange(origTime, true))
			{
				ret = TRUE;
			}
		}
		break;
		case kToCol:
		{
			ReactionListItem* listItem = GetReactionListItem(GetDlgItem(hWnd, IDC_REACTION_LIST), nRow, nCol);
			if (listItem->GetRange(origTime, false))
			{
				ret = TRUE;
			}
			break;
		}
		case kCurveCol:
		{
			ReactionListItem* listItem = GetReactionListItem(GetDlgItem(hWnd, IDC_REACTION_LIST), nRow, nCol);
			listItem->ToggleUseCurve();
			ip->RedrawViews(ip->GetTime());
			ret = TRUE;
		}
		break;
		default:
			ret = TRUE;
		}
	}
	else if (id == IDC_STATE_LIST)
	{
		switch (nCol)
		{
		case kExpandCol:
			if (ToggleStateExpansionState(nRow))
			{
				ret = TRUE;
			}
			nCol = hit.y = kNameCol;
			break;
		case kNameCol:
		{
			StateListItem* listItem = GetStateListItem(GetDlgItem(hWnd, IDC_STATE_LIST), nRow, nCol);
			if (listItem->GetDrivenState() != nullptr)
			{
				blockInvalidation = true;
				if (auto* r = static_cast<Reactor*>(listItem->GetOwner()))
				{
					r->setSelected(listItem->GetIndex());
					blockInvalidation = false;
				}
			}
		}
		break;
		case kValueCol:
		{
			StateListItem* listItem = GetStateListItem(GetDlgItem(hWnd, IDC_STATE_LIST), nRow, nCol);
			if (listItem->GetType() == FLOAT_VAR)
			{
				listItem->GetValue(&origValue);
				ret = TRUE;
			}
		}
		break;
		case kStrengthCol:
		{
			StateListItem* listItem = GetStateListItem(GetDlgItem(hWnd, IDC_STATE_LIST), nRow, nCol);
			if (DrivenState* drivenState = listItem->GetDrivenState())
			{
				Reactor* owner = (Reactor*)listItem->GetOwner();
				if (!(owner->useCurve() && owner->getReactionType() == FLOAT_VAR))
				{
					origValue = drivenState->strength;
					ret = TRUE;
				}
			}
		}
		break;
		case kInfluenceCol:
		{
			StateListItem* listItem = GetStateListItem(GetDlgItem(hWnd, IDC_STATE_LIST), nRow, nCol);
			if (listItem->GetDrivenState())
			{
				Reactor* owner = (Reactor*)listItem->GetOwner();
				if (!(owner->useCurve() && owner->getReactionType() == FLOAT_VAR))
				{
					origValue = listItem->GetInfluence();
					ret = TRUE;
				}
			}
		}
		break;
		case kFalloffCol:
		{
			StateListItem* listItem = GetStateListItem(GetDlgItem(hWnd, IDC_STATE_LIST), nRow, nCol);
			DrivenState* drivenState = listItem->GetDrivenState();
			if (drivenState && !((Reactor*)listItem->GetOwner())->useCurve())
			{
				origValue = drivenState->falloff;
				ret = TRUE;
			}
		}
		break;
		default:
			ret = TRUE;
		}
	}
	NMHDR nh;
	nh.code = LVN_ITEMCHANGED;
	SendMessage(hWnd, WM_NOTIFY, id, (LPARAM)&nh);
	return ret;
}

void ReactionDlg::SpinnerChange(int id, POINT pt, int nRow, int nCol)
{
	if (nRow == -1)
		return;

	BOOL ret = FALSE;
	if (id == IDC_REACTION_LIST)
	{
		switch (nCol)
		{
		case kExpandCol:
			nCol = kNameCol;
			// fall through
		case kNameCol: // name
		{
		}
		break;
		case kFromCol:
		{
			ReactionListItem* listItem = GetReactionListItem(GetDlgItem(hWnd, IDC_REACTION_LIST), nRow, nCol);
			int frame = origTime / GetTicksPerFrame();
			frame += (origPt->y - pt.y) / 2;
			listItem->SetRange(TimeValue(frame * GetTicksPerFrame()), true);
			ret = TRUE;
			break;
		}
		case kToCol:
		{
			ReactionListItem* listItem = GetReactionListItem(GetDlgItem(hWnd, IDC_REACTION_LIST), nRow, nCol);
			int frame = origTime / GetTicksPerFrame();
			frame += (origPt->y - pt.y) / 2;
			listItem->SetRange(TimeValue(frame * GetTicksPerFrame()), false);
			ret = TRUE;
			break;
		}
		default:
			ret = TRUE;
		}
	}
	else if (id == IDC_STATE_LIST)
	{
		switch (nCol)
		{
		case kExpandCol:
			ret = TRUE;
			nCol = kNameCol;
			// fall through
		case kNameCol: // name
		{
		}
		break;
		case kValueCol:
		{
			StateListItem* listItem = GetStateListItem(GetDlgItem(hWnd, IDC_STATE_LIST), nRow, nCol);
			if (listItem->GetType() == FLOAT_VAR)
			{
				float multiplier = 10.0f;
				if ((GetKeyState(VK_CONTROL) & 0x8000))
					multiplier = 1.0f;
				float val = (origValue + (float)(origPt->y - pt.y) / multiplier);
				listItem->SetValue(val);
				iCCtrl->Redraw();
				ret = TRUE;
			}
			break;
		}
		case kStrengthCol:
		{
			StateListItem* listItem = GetStateListItem(GetDlgItem(hWnd, IDC_STATE_LIST), nRow, nCol);
			if (listItem->GetDrivenState())
			{
				Reactor* owner = (Reactor*)listItem->GetOwner();
				if (!(owner->useCurve() && owner->getReactionType() == FLOAT_VAR))
				{
					float multiplier = 1000.0f;
					if ((GetKeyState(VK_CONTROL) & 0x8000))
						multiplier = 100.0f;
					float strength = (origValue + (float)(origPt->y - pt.y) / multiplier);
					owner->setStrength(listItem->GetIndex(), strength);
				}
			}
		}
		break;
		case kInfluenceCol:
		{
			StateListItem* listItem = GetStateListItem(GetDlgItem(hWnd, IDC_STATE_LIST), nRow, nCol);
			if (listItem->GetDrivenState())
			{
				Reactor* owner = (Reactor*)listItem->GetOwner();
				if (!(owner->useCurve() && owner->getReactionType() == FLOAT_VAR))
				{
					float multiplier = 10.0f;
					if ((GetKeyState(VK_CONTROL) & 0x8000))
						multiplier = 1.0f;
					float influence = (origValue + (float)(origPt->y - pt.y) / multiplier);
					listItem->SetInfluence(influence);
				}
			}
		}
		break;
		case kFalloffCol:
		{
			StateListItem* listItem = GetStateListItem(GetDlgItem(hWnd, IDC_STATE_LIST), nRow, nCol);
			if (listItem->GetDrivenState())
			{
				Reactor* owner = (Reactor*)listItem->GetOwner();
				if (!owner->useCurve())
				{
					float multiplier = 100.0f;
					if ((GetKeyState(VK_CONTROL) & 0x8000))
						multiplier = 10.0f;
					float falloff = (origValue + (float)(origPt->y - pt.y) / multiplier);
					owner->setFalloff(listItem->GetIndex(), falloff);
				}
			}
		}
		break;
		default:
			ret = TRUE;
		}
	}
}

void ReactionDlg::SpinnerEnd(int id, BOOL cancel, int nRow, int nCol)
{
	if (cancel)
	{
		theHold.Cancel();
	}
	else if (id == IDC_REACTION_LIST)
	{
		switch (nCol)
		{
		case kToCol:
		{
			theHold.Accept(GetString(IDS_AF_RANGE_CHANGE));
			break;
		}
		case kFromCol:
		{
			theHold.Accept(GetString(IDS_AF_RANGE_CHANGE));
			break;
		}
		default:
			theHold.Accept(_T(""));
			break;
		}
	}
	else if (id == IDC_STATE_LIST)
	{
		switch (nCol)
		{
		case kValueCol:
		{
			theHold.Accept(GetString(IDS_AF_CHANGESTATE));
			break;
		}
		case kStrengthCol:
		{
			StateListItem* listItem = GetStateListItem(GetDlgItem(hWnd, IDC_STATE_LIST), nRow, nCol);
			if (listItem->GetDrivenState())
			{
				Reactor* owner = (Reactor*)listItem->GetOwner();
				if (!(owner->useCurve() && owner->getReactionType() == FLOAT_VAR))
				{
					theHold.Accept(GetString(IDS_AF_CHANGESTRENGTH));
				}
			}
			break;
		}
		case kInfluenceCol:
		{
			StateListItem* listItem = GetStateListItem(GetDlgItem(hWnd, IDC_STATE_LIST), nRow, nCol);
			if (listItem->GetDrivenState())
			{
				Reactor* owner = (Reactor*)listItem->GetOwner();
				if (!(owner->useCurve() && owner->getReactionType() == FLOAT_VAR))
				{
					theHold.Accept(GetString(IDS_AF_CHANGEINFLUENCE));
				}
			}
			break;
		}
		case kFalloffCol:
		{
			StateListItem* listItem = GetStateListItem(GetDlgItem(hWnd, IDC_STATE_LIST), nRow, nCol);
			if (listItem->GetDrivenState() && !((Reactor*)listItem->GetOwner())->useCurve())
			{
				theHold.Accept(GetString(IDS_AF_CHANGEFALLOFF));
			}
			break;
		}
		default:
			theHold.Accept(_T(""));
			break;
		}
	}
	ip->RedrawViews(ip->GetTime());
}

void ReactionDlg::SetMinInfluence()
{
	HWND list = GetDlgItem(hWnd, IDC_STATE_LIST);
	Tab<StateListItem*> listItems = GetSelectedDriverStates();
	for (int i = 0; i < listItems.Count(); i++)
	{
		ReactionSet* set = (ReactionSet*)listItems[i]->GetOwner();
		for (int x = 0; x < set->DrivenCount(); x++)
		{
			if (Reactor* r = set->Driven(x))
			{
				for (int y = 0; y < r->mDrivenStates.Count(); y++)
				{
					if (r->mDrivenStates[y].mDriverID == listItems[i]->GetIndex())
					{
						r->setMinInfluence(y);
						break;
					}
				}
			}
		}
	}
	listItems = GetSelectedDrivenStates();
	for (int i = 0; i < listItems.Count(); i++)
	{
		Reactor* r = (Reactor*)listItems[i]->GetOwner();
		r->setMinInfluence(listItems[i]->GetIndex());
	}
}

void ReactionDlg::SetMaxInfluence()
{
	HWND list = GetDlgItem(hWnd, IDC_STATE_LIST);
	Tab<StateListItem*> listItems = GetSelectedDriverStates();
	for (int i = 0; i < listItems.Count(); i++)
	{
		ReactionSet* set = (ReactionSet*)listItems[i]->GetOwner();
		for (int x = 0; x < set->DrivenCount(); x++)
		{
			if (Reactor* r = set->Driven(x))
			{
				for (int y = 0; y < r->mDrivenStates.Count(); y++)
				{
					if (r->mDrivenStates[y].mDriverID == listItems[i]->GetIndex())
					{
						r->setMaxInfluence(y);
						break;
					}
				}
			}
		}
	}
	listItems = GetSelectedDrivenStates();
	for (int i = 0; i < listItems.Count(); i++)
	{
		Reactor* r = (Reactor*)listItems[i]->GetOwner();
		r->setMaxInfluence(listItems[i]->GetIndex());
	}
}

void ReactionDlg::Change(BOOL redraw)
{
	GetReactionManager()->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	UpdateWindow(GetParent(hWnd));
	if (redraw)
		ip->RedrawViews(ip->GetTime());
}

RefResult ReactionDlg::NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate)
{
	switch (message)
	{
	case REFMSG_SUBANIM_STRUCTURE_CHANGED:
		InvalidateReactionList();
		break;
	case REFMSG_CHANGE:
		Invalidate();
		break;
	case REFMSG_REACTION_COUNT_CHANGED:
		InvalidateSelection();
		break;
	case REFMSG_REACTOR_SELECTION_CHANGED:
		Invalidate();
		break;
	case REFMSG_NODE_NAMECHANGE:
	case REFMSG_REACTTO_OBJ_NAME_CHANGED:
		// UpdateNodeName();
		break;
	case REFMSG_USE_CURVE_CHANGED:
		UpdateCurveControl();
		// SetupFalloffUI();
		break;
	case REFMSG_REACTION_NAME_CHANGED:
		// updateFlags |= REACTORDLG_LIST_CHANGED_SIZE;
		Invalidate();
		break;
	case REFMSG_REF_DELETED:
		// MaybeCloseWindow();
		break;
	case REFMSG_TARGET_DELETED:
		if (hTarget == reactionManager)
		{
			reactionManager = nullptr;
		}
		break;
	}
	return REF_SUCCEED;
}

int ReactionDlg::NumRefs()
{
	return 1;
}

RefTargetHandle ReactionDlg::GetReference(int i)
{
	return (i == 0) ? reactionManager : nullptr;
}

void ReactionDlg::SetReference(int i, RefTargetHandle rtarg)
{
	if (i == 0)
		reactionManager = rtarg;
}

BOOL ReactionDlg::IsRealDependency(ReferenceTarget* rtarg)
{
	return FALSE;
}

static int sRow = -1;
static int sCol = -1;
static int sID = -1;
static BOOL sHandled = FALSE;

static LRESULT CALLBACK ReactionListControlProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	ReactionDlg* dlg = DLGetWindowLongPtr<ReactionDlg*>(hWnd);
	BOOL bHandled = FALSE;
	BOOL result = FALSE;

	switch (message)
	{
	case WM_LBUTTONDBLCLK:
	{
		SetFocus(hWnd);
		ShowWindow(dlg->floatWindow->GetHwnd(), SW_HIDE);
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		POINT hit = dlg->HitTest(pt, IDC_REACTION_LIST);
		sRow = hit.x;
		sCol = hit.y;
		sID = IDC_REACTION_LIST;
		if (sCol > 0)
		{
			RECT rect, pRect;
			GetWindowRect(hWnd, &pRect);
			POINT pt = { pRect.left, pRect.top };
			ScreenToClient(GetParent(hWnd), &pt);

			ListView_GetSubItemRect(hWnd, sRow, sCol, LVIR_BOUNDS, &rect);

			int left = rect.left + pt.x + 2;
			int top = rect.top + pt.y + 2;
			SetWindowPos(dlg->floatWindow->GetHwnd(), hWnd, left, top, rect.right - rect.left, rect.bottom - rect.top, 0);
			ShowWindow(dlg->floatWindow->GetHwnd(), SW_SHOW);
			dlg->floatWindow->GiveFocus();
			ReactionListItem* listItem = dlg->GetReactionDataAt(sRow);
			switch (sCol)
			{
			case kFromCol:
			{
				TimeValue t;
				listItem->GetRange(t, true);
				dlg->floatWindow->SetText(t / GetTicksPerFrame(), 0);
			}
			break;
			case kToCol:
			{
				TimeValue t;
				listItem->GetRange(t, false);
				dlg->floatWindow->SetText(t / GetTicksPerFrame(), 0);
			}
			break;
			}
		}
	}
	break;

	case WM_LBUTTONDOWN:
	{
		if (GetFocus() != hWnd)
			SetFocus(hWnd);
		dlg->origPt = new POINT();
		dlg->origPt->x = GET_X_LPARAM(lParam);
		dlg->origPt->y = GET_Y_LPARAM(lParam);
		POINT hit = dlg->HitTest(*(dlg->origPt), IDC_REACTION_LIST);
		sRow = hit.x;
		sCol = hit.y;

		sHandled = bHandled = dlg->OnLMouseButtonDown(hWnd, wParam, *dlg->origPt, IDC_REACTION_LIST);
		if (bHandled)
		{
			SetCapture(hWnd);
			if (sCol > 0)
				theHold.Begin();
		}
		else
		{
			delete dlg->origPt;
			dlg->origPt = nullptr;
		}
		// dlg->selectionValid = false;
	}
	break;
	case WM_MOUSEMOVE:
	{
		POINT p = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		if (dlg->origPt != nullptr && sHandled && p.y != dlg->origPt->y)
		{
			dlg->SpinnerChange(IDC_REACTION_LIST, p, sRow, sCol);
			bHandled = sHandled;
		}
	}
	break;

	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
		if (dlg->origPt != nullptr)
		{
			delete dlg->origPt;
			dlg->origPt = nullptr;
			bHandled = sHandled;
			if (bHandled)
			{
				dlg->SpinnerEnd(IDC_REACTION_LIST, (message == WM_RBUTTONDOWN), sRow, sCol);
				ReleaseCapture();
			}
			dlg->selectionValid = false;
		}
		else if (message == WM_RBUTTONDOWN)
		{
			// pop up a right-click menu.
			dlg->InvokeRightClickMenu();
			bHandled = TRUE;
			result = TRUE;
		}
		break;
	case WM_PAINT:
		if (!dlg->blockInvalidation)
		{
			if (!dlg->reactionListValid)
				dlg->UpdateReactionList();
			else if (!dlg->selectionValid)
				dlg->SelectionChanged();
		}
		break;
	}

	if (bHandled)
		return result;

	return CallWindowProc(dlg->reactionListWndProc, hWnd, message, wParam, lParam);
}

void ReactionDlg::InvokeRightClickMenu()
{
	HMENU menu;
	if ((menu = CreatePopupMenu()) != nullptr)
	{
		ReactionListItem* selDriver = GetSelectedDriver();
		int anyDriverSelectionValid = (selDriver != nullptr) ? 0 : MF_GRAYED;
		int driverSelectionValid = (selDriver && static_cast<ReactionSet*>(selDriver->GetOwner())->GetReactionDriver()) ? 0 : MF_GRAYED;
		int drivenSelectionValid = (GetSelectedDriven().Count() > 0) ? 0 : MF_GRAYED;
		int driverStateSelectionValid = IsDriverStateSelected() ? 0 : MF_GRAYED;
		int drivenStateSelectionValid = IsDrivenStateSelected() ? 0 : MF_GRAYED;

		AppendMenu(menu, MF_STRING, IDC_ADD_DRIVER, GetString(IDS_ADD_DRIVER));
		AppendMenu(menu, (MF_STRING | anyDriverSelectionValid), IDC_REPLACE_DRIVER, GetString(IDS_REPLACE_DRIVER));
		AppendMenu(menu, (MF_STRING | driverSelectionValid), IDC_ADD_DRIVEN, GetString(IDS_ADD_DRIVEN));
		AppendMenu(menu, (MF_STRING | driverSelectionValid), IDC_ADD_SELECTED, GetString(IDS_ADD_SELECTED));
		AppendMenu(menu, (MF_STRING | (anyDriverSelectionValid & drivenSelectionValid)), IDC_DELETE_SELECTED, GetString(IDS_DELETE_SELECTED));

		AppendMenu(menu, MF_SEPARATOR, 0, 0);

		AppendMenu(menu, (MF_STRING | (driverSelectionValid & drivenSelectionValid)), IDC_CREATE_STATES, GetString(IDS_CREATE_STATES));
		AppendMenu(menu, (MF_STRING | (driverSelectionValid & drivenSelectionValid)), IDC_NEW_STATE, GetString(IDS_NEW_STATE));
		AppendMenu(menu, (MF_STRING | driverStateSelectionValid), IDC_APPEND_STATE, GetString(IDS_APPEND_STATE));
		AppendMenu(menu, (MF_STRING | (driverStateSelectionValid | driverSelectionValid)), IDC_SET_STATE, GetString(IDS_SET_STATE));
		AppendMenu(menu, (MF_STRING | (driverStateSelectionValid & drivenStateSelectionValid)), IDC_DELETE_STATE, GetString(IDS_DELETE_STATE));
		AppendMenu(menu, (MF_STRING | (driverStateSelectionValid & drivenStateSelectionValid)), IDC_EDIT_STATE, GetString(IDS_EDIT_STATE));

		// pop the menu
		POINT mp;
		GetCursorPos(&mp);
		int id = TrackPopupMenu(menu, TPM_CENTERALIGN + TPM_NONOTIFY + TPM_RETURNCMD, mp.x, mp.y, 0, hWnd, nullptr);
		DestroyMenu(menu);

		if (id != 0)
			ip->GetActionManager()->FindTable(kReactionMgrActions)->GetAction(id)->ExecuteAction();
	}
}

static LRESULT CALLBACK StateListControlProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	ReactionDlg* dlg = DLGetWindowLongPtr<ReactionDlg*>(hWnd);
	BOOL bHandled = FALSE;
	BOOL result = FALSE;

	switch (message)
	{
	case WM_LBUTTONDBLCLK:
	{
		SetFocus(hWnd);
		ShowWindow(dlg->floatWindow->GetHwnd(), SW_HIDE);
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		POINT hit = dlg->HitTest(pt, IDC_STATE_LIST);
		sRow = hit.x;
		sCol = hit.y;
		sID = IDC_STATE_LIST;
		if (sCol > 0)
		{
			RECT rect, pRect;
			GetWindowRect(hWnd, &pRect);
			POINT pt = { pRect.left, pRect.top };
			ScreenToClient(GetParent(hWnd), &pt);

			ListView_GetSubItemRect(hWnd, sRow, sCol, LVIR_BOUNDS, &rect);

			int left = rect.left + pt.x + 2;
			int top = rect.top + pt.y + 2;
			SetWindowPos(dlg->floatWindow->GetHwnd(), hWnd, left, top, rect.right - rect.left, rect.bottom - rect.top, 0);
			StateListItem* listItem = dlg->GetStateDataAt(sRow);
			switch (sCol)
			{
			case kValueCol:
				switch (listItem->GetType())
				{
				case FLOAT_VAR:
				{
					float f;
					listItem->GetValue(&f);
					dlg->floatWindow->SetText(f);
					ShowWindow(dlg->floatWindow->GetHwnd(), SW_SHOW);
					dlg->floatWindow->GiveFocus();
				}
				break;
				}
				break;
			case kStrengthCol:
			{
				if (DrivenState* state = listItem->GetDrivenState())
				{
					Reactor* owner = (Reactor*)listItem->GetOwner();
					if (!(owner->useCurve() && owner->getReactionType() == FLOAT_VAR))
					{
						dlg->floatWindow->SetText(state->strength * 100.0f);
						ShowWindow(dlg->floatWindow->GetHwnd(), SW_SHOW);
						dlg->floatWindow->GiveFocus();
					}
				}
			}
			break;
			case kInfluenceCol:
			{
				if (listItem->GetDrivenState())
				{
					Reactor* owner = (Reactor*)listItem->GetOwner();
					if (!(owner->useCurve() && owner->getReactionType() == FLOAT_VAR))
					{
						dlg->floatWindow->SetText(listItem->GetInfluence());
						ShowWindow(dlg->floatWindow->GetHwnd(), SW_SHOW);
						dlg->floatWindow->GiveFocus();
					}
				}
			}
			break;
			case kFalloffCol:
			{
				DrivenState* state = listItem->GetDrivenState();
				if (state != nullptr && !((Reactor*)listItem->GetOwner())->useCurve())
				{
					dlg->floatWindow->SetText(state->falloff);
					ShowWindow(dlg->floatWindow->GetHwnd(), SW_SHOW);
					dlg->floatWindow->GiveFocus();
				}
			}
			break;
			default:
				break;
			}
		}
	}
	break;
	case WM_LBUTTONDOWN:
	{
		if (GetFocus() != hWnd)
			SetFocus(hWnd);
		dlg->origPt = new POINT();
		dlg->origPt->x = GET_X_LPARAM(lParam);
		dlg->origPt->y = GET_Y_LPARAM(lParam);
		POINT hit = dlg->HitTest(*(dlg->origPt), IDC_STATE_LIST);
		sRow = hit.x;
		sCol = hit.y;

		sHandled = bHandled = dlg->OnLMouseButtonDown(hWnd, wParam, *dlg->origPt, IDC_STATE_LIST);
		if (bHandled)
		{
			SetCapture(hWnd);
			if (sCol > 0)
				theHold.Begin();
		}
		else
		{
			delete dlg->origPt;
			dlg->origPt = nullptr;
		}
		// dlg->stateSelectionValid = false;
	}
	break;
	case WM_MOUSEMOVE:
	{
		POINT p = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		if (dlg->origPt != nullptr && sHandled && p.y != dlg->origPt->y)
		{
			dlg->SpinnerChange(IDC_STATE_LIST, p, sRow, sCol);
			bHandled = sHandled;
		}
	}
	break;

	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
		if (dlg->origPt != nullptr)
		{
			delete dlg->origPt;
			dlg->origPt = nullptr;
			bHandled = sHandled;
			if (bHandled)
			{
				dlg->SpinnerEnd(IDC_STATE_LIST, (message == WM_RBUTTONDOWN), sRow, sCol);
				ReleaseCapture();
			}
			dlg->stateSelectionValid = false;
		}
		else if (message == WM_RBUTTONDOWN)
		{
			// pop up a right-click menu.
			dlg->InvokeRightClickMenu();
			bHandled = TRUE;
			result = TRUE;
		}
		break;
	case WM_PAINT:
		if (!dlg->blockInvalidation)
		{
			if (!dlg->reactionListValid)
				dlg->UpdateReactionList();
			else if (!dlg->selectionValid)
				dlg->SelectionChanged();
			else if (!dlg->stateSelectionValid)
				dlg->StateSelectionChanged();
		}
		break;
	}

	if (bHandled)
		return result;

	return CallWindowProc(dlg->stateListWndProc, hWnd, message, wParam, lParam);
}

static LRESULT CALLBACK SplitterControlProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	ReactionDlg* dlg = DLGetWindowLongPtr<ReactionDlg*>(GetParent(hWnd));
	BOOL bHandled = FALSE;
	BOOL result = FALSE;
	static int dragging = -1;
	static float total = 0.0f;

	switch (message)
	{
	case WM_LBUTTONDOWN:
	{
		total = dlg->rListPos + dlg->sListPos;

		RECT rect;
		GetWindowRect(hWnd, &rect);

		POINT curPos;
		curPos.x = GET_X_LPARAM(lParam);
		curPos.y = GET_Y_LPARAM(lParam);
		ClientToScreen(hWnd, &curPos);
		dragging = curPos.y - rect.top;

		SetCapture(hWnd);
	}
	break;
	case WM_MOUSEMOVE:
		SetCursor(LoadCursor(nullptr, IDC_SIZENS));
		if (dragging >= 0)
		{
			RECT pRect;
			HWND parent = GetParent(hWnd);
			GetClientRect(parent, &pRect);

			POINT curPos;
			curPos.x = GET_X_LPARAM(lParam);
			curPos.y = GET_Y_LPARAM(lParam) + dragging;
			ClientToScreen(hWnd, &curPos);
			ScreenToClient(parent, &curPos);

			float curY = ((float)curPos.y) / (float)pRect.bottom;
			if (hWnd == GetDlgItem(dlg->hWnd, IDC_SPLITTER1))
			{
				dlg->rListPos = std::min(std::max(curY, 0.05f), 0.95f);
				dlg->sListPos = total - dlg->rListPos;
			}
			else
			{
				dlg->sListPos = std::min(std::max(curY - dlg->rListPos, 0.05f), 0.95f);
			}
			dlg->PerformLayout();
			UpdateWindow(dlg->hWnd);
		}
		break;

	case WM_LBUTTONUP:
		dragging = -1;
		ReleaseCapture();
		break;
	case WM_PAINT:
		break;
	}

	if (bHandled)
		return result;

	return CallWindowProc(dlg->splitterWndProc, hWnd, message, wParam, lParam);
}

static INT_PTR CALLBACK ReactionDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ReactionDlg* dlg = DLGetWindowLongPtr<ReactionDlg*>(hWnd);

	switch (msg)
	{
	case WM_INITDIALOG:
	{
		dlg = (ReactionDlg*)lParam;
		DLSetWindowLongPtr(hWnd, lParam);

		HWND hList = GetDlgItem(hWnd, IDC_REACTION_LIST);
		DLSetWindowLongPtr(hList, lParam);
		dlg->reactionListWndProc = DLSetWindowProc(hList, ReactionListControlProc);

		hList = GetDlgItem(hWnd, IDC_STATE_LIST);
		DLSetWindowLongPtr(hList, lParam);
		dlg->stateListWndProc = DLSetWindowProc(hList, StateListControlProc);

		HWND hSplitter = GetDlgItem(hWnd, IDC_SPLITTER1);
		dlg->splitterWndProc = DLSetWindowLongPtr(hSplitter, SplitterControlProc);

		hSplitter = GetDlgItem(hWnd, IDC_SPLITTER2);
		DLSetWindowLongPtr(hSplitter, SplitterControlProc);

		dlg->SetupUI(hWnd);
	}
	break;
	case WM_NOTIFY:
	{
		LPNMHDR pNMHDR = (LPNMHDR)lParam;
		const UINT code = pNMHDR->code;

		switch (code)
		{
		case LVN_GETDISPINFO:
		{
			NMLVDISPINFO* pDispInfo = (NMLVDISPINFO*)lParam;
			if (DbgVerify(pDispInfo != nullptr) && (pDispInfo->item.mask | LVIF_TEXT) &&
					DbgVerify(pDispInfo->item.pszText != nullptr))
			{
				TSTR pSrcString;
				if (pDispInfo->hdr.idFrom == IDC_REACTION_LIST)
				{
					ReactionListItem* listItem = (ReactionListItem*)pDispInfo->item.lParam;
					switch (pDispInfo->item.iSubItem)
					{
					case kFromCol:
					{
						TimeValue t;
						if (listItem->GetRange(t, true))
							pSrcString.printf(_T("%i"), t / GetTicksPerFrame());
						break;
					}
					case kToCol:
					{
						TimeValue t;
						if (listItem->GetRange(t, false))
							pSrcString.printf(_T("%i"), t / GetTicksPerFrame());
						break;
					}
					case kCurveCol:
					{
						if (listItem->IsUsingCurve())
							pSrcString.printf(_T("X"));
						break;
					}
					}
				}
				else if (pDispInfo->hdr.idFrom == IDC_STATE_LIST)
				{
					StateListItem* listItem = (StateListItem*)pDispInfo->item.lParam;
					switch (pDispInfo->item.iSubItem)
					{
					case kValueCol:
					{
						int type = listItem->GetType();
						switch (type)
						{
						case FLOAT_VAR:
						{
							float f;
							listItem->GetValue(&f);
							pSrcString.printf(_T("%.3f"), f);
						}
						break;
						case SCALE_VAR:
						case VECTOR_VAR:
						{
							Point3 p;
							listItem->GetValue(&p);
							pSrcString.printf(_T("( %.3f; %.3f; %.3f )"), p[0], p[1], p[2]);
						}
						break;
						case QUAT_VAR:
						{
							Quat q;
							listItem->GetValue(&q);
							float x, y, z;
							q.GetEuler(&x, &y, &z);
							x = RadToDeg(x);
							y = RadToDeg(y);
							z = RadToDeg(z);
							pSrcString.printf(_T("( %.3f; %.3f; %.3f )"), x, y, z);
						}
						break;
						}
					}
					break;
					case kStrengthCol:
					{
						if (DrivenState* state = listItem->GetDrivenState())
						{
							Reactor* owner = (Reactor*)listItem->GetOwner();
							if (!(owner->useCurve() && owner->getReactionType() == FLOAT_VAR))
								pSrcString.printf(_T("%.1f"), state->strength * 100.0f);
						}
						break;
					}
					case kInfluenceCol:
					{
						if (DrivenState* state = listItem->GetDrivenState())
						{
							Reactor* owner = (Reactor*)listItem->GetOwner();
							if (!(owner->useCurve() && owner->getReactionType() == FLOAT_VAR))
								pSrcString.printf(_T("%.1f"), listItem->GetInfluence());
						}
						break;
					}
					case kFalloffCol:
					{
						DrivenState* state = listItem->GetDrivenState();
						if (state != nullptr && !((Reactor*)listItem->GetOwner())->useCurve())
							pSrcString.printf(_T("%.001f"), state->falloff);
						break;
					}
					}
				}
				_tcsncpy_s(pDispInfo->item.pszText, pDispInfo->item.cchTextMax, pSrcString, _TRUNCATE);
			}
		}

		// case LVN_ODSTATECHANGED:
		case LVN_ITEMCHANGED:
		{
			NMLISTVIEW* pListView = (NMLISTVIEW*)lParam;
			// NMLVODSTATECHANGE *pListView = (NMLVODSTATECHANGE *)lParam;
			// only handle change in selection state
			if (!(pListView->uChanged & LVIF_STATE))
				return TRUE;

			bool isSelected = (pListView->uNewState & LVIS_SELECTED) > 0;
			bool wasSelected = (pListView->uOldState & LVIS_SELECTED) > 0;
			// bool isSelected = (pListView->uNewState == LVIS_SELECTED);
			// bool wasSelected = (pListView->uOldState == LVIS_SELECTED);
			// I can't explain why we get messages with uOldState having
			// strange values (0x77d4xxxx) but if we ignore them we cut
			// down on the number of unnecessary reselections (#672926)
			// @todo: find out why
			if (isSelected != wasSelected && pListView->uOldState <= 0x77d40000)
			{
				if (pListView->hdr.idFrom == IDC_REACTION_LIST)
				{
					dlg->selectionValid = false;
				}
				else if (pListView->hdr.idFrom == IDC_STATE_LIST)
				{
					dlg->stateSelectionValid = false;
				}
			}
		}
		break;
		case LVN_BEGINLABELEDIT:
			if (((NMLVDISPINFO*)lParam)->hdr.idFrom == IDC_STATE_LIST)
			{
				LVITEM item = ((NMLVDISPINFO*)lParam)->item;
				StateListItem* listItem = (StateListItem*)item.lParam;
				if (listItem == nullptr || listItem->GetDrivenState() != nullptr)
					return TRUE;
				DisableAccelerators();
			}
			return FALSE;
		case LVN_ENDLABELEDIT:
			if (((NMLVDISPINFO*)lParam)->hdr.idFrom == IDC_STATE_LIST)
			{
				BOOL ret = FALSE;
				LVITEM item = ((NMLVDISPINFO*)lParam)->item;
				StateListItem* listItem = (StateListItem*)item.lParam;

				DriverState* state = listItem->GetDriverState();
				if (state != nullptr && item.pszText != nullptr)
				{
					state->name.printf(_T("%s"), item.pszText);
					dlg->selectionValid = false;
					listItem->GetOwner()->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
					ret = TRUE;
				}
				EnableAccelerators();
				return ret;
			}
			return FALSE;

			// case NM_CUSTOMDRAW:
			//	{

			//	LPNMLVCUSTOMDRAW  lplvcd = (LPNMLVCUSTOMDRAW)lParam;

			//	switch(lplvcd->nmcd.dwDrawStage) {

			//		case CDDS_PREPAINT :
			//			return CDRF_NOTIFYITEMDRAW;

			//		case CDDS_ITEMPREPAINT:
			//	        return CDRF_NEWFONT;
			//			//return CDRF_NOTIFYSUBITEMREDRAW;

			//		case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
			//			return CDRF_NEWFONT;
			//		}
			//	}
		}
		return MAKELRESULT(1, 0);
	}
	break;
	case CC_SPINNER_BUTTONDOWN:
		break;

	case CC_SPINNER_CHANGE:
		break;

	case WM_CUSTEDIT_ENTER:
		switch (LOWORD(wParam))
		{
		case IDC_EDIT_FLOAT:
			SetFocus(GetDlgItem(hWnd, sID));
			ShowWindow(dlg->floatWindow->GetHwnd(), SW_HIDE);
			switch (sID)
			{
			case IDC_REACTION_LIST:
			{
				ReactionListItem* listItem = dlg->GetReactionDataAt(sRow);
				switch (sCol)
				{
				case kFromCol:
					theHold.Begin();
					listItem->SetRange(dlg->floatWindow->GetInt() * GetTicksPerFrame(), true);
					theHold.Accept(GetString(IDS_AF_RANGE_CHANGE));
					break;
				case kToCol:
					theHold.Begin();
					listItem->SetRange(dlg->floatWindow->GetInt() * GetTicksPerFrame(), false);
					theHold.Accept(GetString(IDS_AF_RANGE_CHANGE));
					break;
				}
				sID = -1;
			}
			break;
			case IDC_STATE_LIST:
			{
				StateListItem* listItem = dlg->GetStateDataAt(sRow);
				switch (sCol)
				{
				case kValueCol:
					if (listItem->GetType() == FLOAT_VAR)
					{
						theHold.Begin();
						listItem->SetValue(dlg->floatWindow->GetFloat());
						dlg->RedrawControl();
						theHold.Accept(GetString(IDS_AF_CHANGESTATE));
					}
					break;
				case kStrengthCol:
				{
					if (listItem->GetDrivenState() != nullptr)
					{
						Reactor* owner = (Reactor*)listItem->GetOwner();
						if (!(owner->useCurve() && owner->getReactionType() == FLOAT_VAR))
						{
							theHold.Begin();
							owner->setStrength(listItem->GetIndex(), dlg->floatWindow->GetFloat() / 100.0f);
							theHold.Accept(GetString(IDS_AF_CHANGESTRENGTH));
						}
					}
				}
				break;
				case kInfluenceCol:
				{
					StateListItem* listItem = dlg->GetStateDataAt(sRow);
					if (listItem->GetDrivenState() != nullptr)
					{
						Reactor* owner = (Reactor*)listItem->GetOwner();
						if (!(owner->useCurve() && owner->getReactionType() == FLOAT_VAR))
						{
							theHold.Begin();
							listItem->SetInfluence(dlg->floatWindow->GetFloat());
							theHold.Accept(GetString(IDS_AF_CHANGEINFLUENCE));
						}
					}
				}
				break;
				case kFalloffCol:
				{
					StateListItem* listItem = dlg->GetStateDataAt(sRow);
					if (listItem->GetDrivenState() != nullptr && !((Reactor*)listItem->GetOwner())->useCurve())
					{
						theHold.Begin();
						Reactor* r = (Reactor*)listItem->GetOwner();
						r->setFalloff(listItem->GetIndex(), dlg->floatWindow->GetFloat());
						theHold.Accept(GetString(IDS_AF_CHANGEFALLOFF));
					}
					break;
				}
				}
			}
				sID = -1;
				break;
			}
			break;
		default:
			break;
		}
		break;

	case CC_SPINNER_BUTTONUP:
		break;

	case WM_MOUSEMOVE:
		SetCursor(LoadCursor(nullptr, IDC_ARROW));
		break;
	case WM_RBUTTONDOWN:
		// pop up a right-click menu.
		dlg->InvokeRightClickMenu();
		break;

	case WM_COMMAND:
		// dlg->ip->GetActionManager()->FindTable(kReactionMgrActions)->GetAction(LOWORD(wParam))->ExecuteAction();
		dlg->WMCommand(LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
		break;

	case WM_CC_CHANGE_CURVEPT:
	case WM_CC_CHANGE_CURVETANGENT:
	{
		ICurve* curve = ((ICurve*)lParam);
		Reactor* reactor = FindReactor(curve);
		if (reactor->flags & REACTOR_BLOCK_CURVE_UPDATE)
			break;
		reactor->flags |= REACTOR_BLOCK_CURVE_UPDATE;

		dlg->EnableControlDraw(FALSE);
		int ptNum = LOWORD(wParam);
		CurvePoint pt = curve->GetPoint(0, ptNum);

		theHold.Suspend();

		if (reactor->getReactionType() == FLOAT_VAR)
		{
			reactor->getDriverState(ptNum)->fvalue = pt.p.x;

			int curveNum;
			for (curveNum = 0; curveNum < reactor->iCCtrl->GetNumCurves(); curveNum++)
			{
				if (curve == reactor->iCCtrl->GetControlCurve(curveNum))
					break;
			}

			switch (reactor->type)
			{
			case REACTORFLOAT:
				reactor->mDrivenStates[ptNum].fstate = pt.p.y;
				reactor->UpdateCurves(false);
				break;
			case REACTORPOS:
			case REACTORP3:
			case REACTORSCALE:
				reactor->mDrivenStates[ptNum].pstate[curveNum] = pt.p.y;
				reactor->UpdateCurves(false);
				break;
			case REACTORROT:
			{
				float eulAng[3];
				QuatToEuler(reactor->mDrivenStates[ptNum].qstate, eulAng);
				eulAng[curveNum] = DegToRad(pt.p.y);
				EulerToQuat(eulAng, reactor->mDrivenStates[ptNum].qstate);
				reactor->UpdateCurves(false);
				break;
			}
			default:
				break;
			}

			float xMax = dlg->GetControlXRange().y;
			float xMin = dlg->GetControlXRange().x;
			float oldWidth = xMax - xMin;

			float oldMax = xMax;
			float oldMin = xMin;
			Point2 p, pin, pout;

			if (curve->GetNumPts() > ptNum + 1 &&
					pt.p.x >= curve->GetPoint(0, ptNum + 1).p.x - pt.out.x -
									curve->GetPoint(0, ptNum + 1).in.x) //{}
			{
				pt.p.x = curve->GetPoint(0, ptNum + 1).p.x - pt.out.x -
						curve->GetPoint(0, ptNum + 1).in.x - 0.001f;
			}
			else if (ptNum &&
					pt.p.x <= curve->GetPoint(0, ptNum - 1).p.x + pt.in.x +
									curve->GetPoint(0, ptNum + 1).out.x) //{}
			{
				pt.p.x = curve->GetPoint(0, ptNum - 1).p.x + pt.in.x +
						curve->GetPoint(0, ptNum + 1).out.x + 0.001f;
			}

			if (ptNum == 0)
			{
				if (pt.p.x > xMax)
					xMax = pt.p.x;
				xMin = pt.p.x;
				dlg->SetControlXRange(pt.p.x, xMax, FALSE);
			}
			else if (ptNum == curve->GetNumPts() - 1)
			{
				if (pt.p.x < xMin)
					xMin = pt.p.x;
				dlg->SetControlXRange(xMin, pt.p.x, FALSE);
				xMax = pt.p.x;
			}

			// keep the rest of them in sync
			for (int i = 0; i < reactor->iCCtrl->GetNumCurves(); i++)
			{
				ICurve* otherCurve = reactor->iCCtrl->GetControlCurve(i);
				if (curve != otherCurve)
				{
					pt = otherCurve->GetPoint(0, ptNum);
					pt.p.x = reactor->getDriverState(ptNum)->fvalue;
					otherCurve->SetPoint(0, ptNum, &pt, FALSE, FALSE);
				}
			}
		}
		theHold.Resume();
		dlg->EnableControlDraw(TRUE);
		reactor->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		Interface* ip = GetCOREInterface();
		ip->RedrawViews(ip->GetTime());

		reactor->flags &= ~REACTOR_BLOCK_CURVE_UPDATE;

		break;
	}
	case WM_CC_SEL_CURVEPT:
	{
		ICurve* curve = ((ICurve*)lParam);
		Reactor* reactor = FindReactor(curve);
		if (reactor->getReactionType() == FLOAT_VAR)
		{
			int selCount = (LOWORD(wParam));
			if (selCount == 1)
			{
				BitArray selPts = curve->GetSelectedPts();
				for (int i = 0; i < selPts.GetSize(); i++)
				{
					if (selPts[i])
					{
						// SendDlgItemMessage(hWnd, IDC_REACTION_LIST, LB_SETCURSEL, i, 0);
						// ListView_SetItemState(GetDlgItem(hWnd, IDC_STATE_LIST), i
						reactor->setSelected(i);
						break;
					}
				}
			}
		}
		break;
	}
	case WM_CC_DEL_CURVEPT:
	{
		ICurve* curve = ((ICurve*)lParam);
		Reactor* reactor = FindReactor(curve);
		if (reactor->getReactionType() == FLOAT_VAR)
		{
			int ptNum = LOWORD(wParam);
			reactor->flags |= REACTOR_BLOCK_CURVE_UPDATE;

			// delete the points on the other curves
			int curveNum;
			for (curveNum = 0; curveNum < reactor->iCCtrl->GetNumCurves(); curveNum++)
			{
				ICurve* otherCurve = reactor->iCCtrl->GetControlCurve(curveNum);
				if (curve != otherCurve)
				{
					otherCurve->Delete(ptNum);
				}
			}

			reactor->DeleteReaction(ptNum);
			reactor->flags &= ~REACTOR_BLOCK_CURVE_UPDATE;
			dlg->RedrawControl();
			dlg->selectionValid = false;
		}
	}
	break;
	case WM_CC_INSERT_CURVEPT:
	{
		ICurve* curve = ((ICurve*)lParam);
		Reactor* reactor = FindReactor(curve);
		if (reactor->getReactionType() == FLOAT_VAR && !theHold.RestoreOrRedoing())
		{
			if (curve->GetNumPts() <= reactor->mDrivenStates.Count())
				return 0;
			int ptNum = LOWORD(wParam);
			CurvePoint pt = curve->GetPoint(0, ptNum);
			// update the flags for the newly added point
			pt.flags |= CURVEP_NO_X_CONSTRAINT;
			curve->SetPoint(0, ptNum, &pt, FALSE, FALSE);

			int curNum;
			ICurve* tempCurve;

			reactor->flags |= REACTOR_BLOCK_CURVE_UPDATE;
			DrivenState* reaction = reactor->CreateReactionAndReturn(FALSE);
			DriverState* driverState = reactor->GetReactionDriver()->GetState(reaction->mDriverID);
			reactor->flags &= ~REACTOR_BLOCK_CURVE_UPDATE;

			if (reaction)
			{
				driverState->fvalue = pt.p.x;

				switch (reactor->type)
				{
				case REACTORFLOAT:
					reaction->fstate = pt.p.y;
					break;
				case REACTORROT:
					float ang[3];
					for (curNum = 0; curNum < reactor->iCCtrl->GetNumCurves(); curNum++)
					{
						tempCurve = reactor->iCCtrl->GetControlCurve(curNum);
						ang[curNum] = DegToRad(tempCurve->GetValue(0, driverState->fvalue, FOREVER, FALSE));
					}
					EulerToQuat(ang, reaction->qstate);
					break;
				case REACTORSCALE:
				case REACTORPOS:
				case REACTORP3:
					for (curNum = 0; curNum < reactor->iCCtrl->GetNumCurves(); curNum++)
					{
						tempCurve = reactor->iCCtrl->GetControlCurve(curNum);
						reaction->pstate[curNum] = tempCurve->GetValue(0, driverState->fvalue, FOREVER, FALSE);
					}
					break;
				}

				for (curNum = 0; curNum < reactor->iCCtrl->GetNumCurves(); curNum++)
				{
					tempCurve = reactor->iCCtrl->GetControlCurve(curNum);
					if (tempCurve != curve)
					{
						auto* newPt = new CurvePoint();
						newPt->p.x = driverState->fvalue;
						newPt->p.y = tempCurve->GetValue(0, driverState->fvalue);
						newPt->in = newPt->out = Point2(0.0f, 0.0f); // newPt->p;
						newPt->flags = pt.flags;
						tempCurve->Insert(ptNum, *newPt);
					}
				}

				if (reactor->mDrivenStates.Count() > 1) // if its not the first reaction
				{
					reactor->setMinInfluence(ptNum);
				}
				reactor->SortReactions();
			}
			dlg->selectionValid = false;
		}
	}
	break;
	case WM_PAINT:
		if (!dlg->valid && !dlg->blockInvalidation)
			dlg->Update();
		return 0;
	case WM_SIZE:
		dlg->PerformLayout();
		break;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		dlg = nullptr;
		break;

	case WM_DESTROY:
	{
		HWND hList = GetDlgItem(hWnd, IDC_REACTION_LIST);
		DLSetWindowLongPtr(hList, NULL);
		DLSetWindowLongPtr(hList, dlg->reactionListWndProc);

		hList = GetDlgItem(hWnd, IDC_STATE_LIST);
		DLSetWindowLongPtr(hList, NULL);
		DLSetWindowLongPtr(hList, dlg->stateListWndProc);

		hList = GetDlgItem(hWnd, IDC_SPLITTER1);
		DLSetWindowLongPtr(hList, dlg->splitterWndProc);

		hList = GetDlgItem(hWnd, IDC_SPLITTER2);
		DLSetWindowLongPtr(hList, dlg->splitterWndProc);

		HoldSuspend hs;
		dlg->DeleteMe();
	}
	break;

	default:
		return FALSE;
	}
	return TRUE;
}

int StateListItem::GetType()
{
	if (mType == kDriverState)
	{
		return GetDriverState()->type;
	}
	else
	{
		auto* r = static_cast<Reactor*>(GetOwner());
		switch (r->type)
		{
		case REACTORFLOAT:
			return FLOAT_VAR;
		case REACTORPOS:
		case REACTORP3:
		case REACTORSCALE:
			return VECTOR_VAR;
		case REACTORROT:
			return QUAT_VAR;
		default:
			return 0;
		}
	}
	return 0;
}

float StateListItem::GetInfluence()
{
	float infl = GetDrivenState()->influence;

	auto* r = static_cast<Reactor*>(GetOwner());
	ReactionDriver* driver = r->GetReactionDriver();
	if (driver && driver->Owner())
	{
		ParamDimension* dim = driver->Owner()->GetParamDimension(driver->SubIndex());
		infl = dim->Convert(infl);
	}
	return infl;
}

void StateListItem::SetInfluence(float infl)
{
	auto* r = static_cast<Reactor*>(GetOwner());
	ReactionDriver* driver = r->GetReactionDriver();
	if (driver && driver->Owner())
	{
		ParamDimension* dim = driver->Owner()->GetParamDimension(driver->SubIndex());
		r->setInfluence(GetIndex(), dim->UnConvert(infl));
	}
	else
		r->setInfluence(GetIndex(), infl);
}

void StateListItem::SetValue(float val)
{
	if (mType == kDriverState)
	{
		DriverState* state = GetDriverState();
		if (state->type == FLOAT_VAR)
		{
			ReactionDriver* driver = static_cast<ReactionSet*>(GetOwner())->GetReactionDriver();
			if (driver && driver->Owner())
			{
				ParamDimension* dim = driver->Owner()->GetParamDimension(driver->SubIndex());
				driver->SetState(this->GetIndex(), dim->UnConvert(val));
			}
			else
				driver->SetState(this->GetIndex(), val);
			GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
		}
	}
	else
	{
		auto* r = static_cast<Reactor*>(GetOwner());
		if (r->type == REACTORFLOAT)
		{
			ParamDimension* dim = nullptr;
			DependentIterator di(r);
			ReferenceMaker* maker = nullptr;
			while ((maker = di.Next()) != nullptr)
			{
				for (int i = 0; i < maker->NumSubs(); i++)
				{
					Animatable* n = maker->SubAnim(i);
					if (n == r)
					{
						dim = maker->GetParamDimension(i);
						val = dim->UnConvert(val);
						break;
					}
				}
				if (dim != nullptr)
					break;
			}

			r->setState(GetIndex(), val);
			GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
		}
	}
}

void StateListItem::GetValue(void* val)
{
	if (mType == kDriverState)
	{
		DriverState* state = GetDriverState();
		switch (state->type)
		{
		case FLOAT_VAR:
		{
			ReactionDriver* driver = static_cast<ReactionSet*>(GetOwner())->GetReactionDriver();
			if (driver && driver->Owner())
			{
				ParamDimension* dim = driver->Owner()->GetParamDimension(driver->SubIndex());
				*(float*)val = dim->Convert(state->fvalue);
			}
			else
				*(float*)val = state->fvalue;
		}
		break;
		case SCALE_VAR:
		case VECTOR_VAR:
			*(Point3*)val = state->pvalue;
			break;
		case QUAT_VAR:
			*(Quat*)val = state->qvalue;
			break;
		}
	}
	else
	{
		DrivenState* state = GetDrivenState();
		if (auto* r = static_cast<Reactor*>(GetOwner()))
		{
			switch (r->type)
			{
			case REACTORFLOAT:
			{
				MyEnumProc dep(r);
				r->DoEnumDependents(&dep);
				ParamDimension* dim = nullptr;

				for (int x = 0; x < dep.anims.Count(); x++)
				{
					for (int i = 0; i < dep.anims[x]->NumSubs(); i++)
					{
						Animatable* n = dep.anims[x]->SubAnim(i);
						if (n == r)
						{
							dim = dep.anims[x]->GetParamDimension(i);
							*(float*)val = dim->Convert(state->fstate);
							break;
						}
					}
					if (dim != nullptr)
						break;
				}

				if (dim == nullptr)
					*(float*)val = state->fstate;
			}
			break;
			case REACTORPOS:
			case REACTORP3:
			case REACTORSCALE:
				*(Point3*)val = state->pstate;
				break;
			case REACTORROT:
				*(Quat*)val = state->qstate;
				break;
			default:
				val = nullptr;
				break;
			}
		}
	}
}

TSTR StateListItem::GetName()
{
	if (mType == kDriverState)
	{
		return GetDriverState()->name;
	}
	else if (owner != nullptr)
	{
		TSTR buf = _T("");
		MyEnumProc dep(owner);
		owner->DoEnumDependents(&dep);

		for (int x = 0; x < dep.anims.Count(); x++)
		{
			for (int i = 0; i < dep.anims[x]->NumSubs(); i++)
			{
				ReferenceMaker* parent = dep.anims[x];
				Animatable* n = parent->SubAnim(i);
				if (n == owner)
				{
					TSTR name = _T("");
					TSTR subAnimName = parent->SubAnimName(i, true);
					TSTR modName = _T("");

					if (parent->SuperClassID() == PARAMETER_BLOCK2_CLASS_ID)
						parent = ((IParamBlock2*)parent)->GetOwner();
					if (parent != nullptr)
					{
						if (parent->SuperClassID() == OSM_CLASS_ID || parent->SuperClassID() == WSM_CLASS_ID)
							modName = ((BaseObject*)parent)->GetObjectName(true);

						if (modName.Length() > 0)
							subAnimName.printf(_T("%s / %s"), modName, subAnimName);

						parent->NotifyDependents(FOREVER, (PartID)&name, REFMSG_GET_NODE_NAME);
					}
					if (name.Length() > 0)
						buf.printf(_T("%s / %s"), name, subAnimName);
					else
						buf.printf(_T("%s"), subAnimName);

					// added (Locked) to the end of the name, like is done in trackview
					ILockedTracksMan* iltman =
							static_cast<ILockedTracksMan*>(GetCOREInterface(ILOCKEDTRACKSMAN_INTERFACE));
					ReferenceTarget* ranim = dynamic_cast<ReferenceTarget*>(n);
					ReferenceTarget* rparent = dynamic_cast<ReferenceTarget*>(parent);
					if (iltman && ranim && iltman->GetLocked(ranim, rparent, i))
					{
						buf += GetString(IDS_LOCKED);
					}

					return buf;
				}
			}
		}
		return buf;
	}
	return _T("");
}

TSTR ReactionListItem::GetName()
{
	TSTR buf = _T("");
	TSTR name;
	TSTR subAnimName;

	if (mType == kReactionSet)
	{
		ReactionDriver* driver = GetReactionSet()->GetReactionDriver();
		if (driver && driver->Owner())
		{
			ReferenceMaker* owner = driver->Owner();
			int subNum = driver->SubIndex();
			if (subNum < 0)
			{
				if (subNum == IKTRACK)
					subAnimName = GetString(IDS_AF_ROTATION);
				else
					subAnimName = GetString(IDS_AF_WS_POSITION);
			}
			else
				subAnimName = owner->SubAnimName(subNum, true);

			TSTR modName = _T("");

			if (owner->SuperClassID() == PARAMETER_BLOCK2_CLASS_ID)
				owner = ((IParamBlock2*)owner)->GetOwner();
			if (owner != nullptr)
			{
				if (owner->SuperClassID() == OSM_CLASS_ID || owner->SuperClassID() == WSM_CLASS_ID)
					modName = ((BaseObject*)owner)->GetObjectName(true);

				if (modName.Length() > 0)
					subAnimName.printf(_T("%s / %s"), modName, subAnimName);

				if (owner->SuperClassID() == BASENODE_CLASS_ID)
					name = ((INode*)owner)->GetName();
				else
					owner->NotifyDependents(FOREVER, (PartID)&name, REFMSG_GET_NODE_NAME);
			}
			if (name.Length() > 0)
				buf.printf(_T("%s / %s"), name, subAnimName);
			else
				buf.printf(_T("%s"), subAnimName);
		}
		else
			buf = GetString(IDS_UNASSIGNED);

		return buf;
	}
	else
	{
		if (Reactor* r = Driven())
		{
			MyEnumProc dep(r);
			r->DoEnumDependents(&dep);

			for (int x = 0; x < dep.anims.Count(); x++)
			{
				for (int i = 0; i < dep.anims[x]->NumSubs(); i++)
				{
					Animatable* parent = dep.anims[x];
					Animatable* n = parent->SubAnim(i);
					if (n == r)
					{
						subAnimName = parent->SubAnimName(i, true);
						TSTR modName = _T("");

						if (parent->SuperClassID() == PARAMETER_BLOCK2_CLASS_ID)
							parent = ((IParamBlock2*)parent)->GetOwner();
						if (parent != nullptr)
						{
							if (parent->SuperClassID() == OSM_CLASS_ID || parent->SuperClassID() == WSM_CLASS_ID)
								modName = ((BaseObject*)parent)->GetObjectName(true);
						}

						if (modName.Length() > 0)
							subAnimName.printf(_T("%s / %s"), modName, subAnimName);

						r->NotifyDependents(FOREVER, (PartID)&name, REFMSG_GET_NODE_NAME);
						if (name.Length() > 0)
							buf.printf(_T("%s / %s"), name, subAnimName);
						else
							buf.printf(_T("%s"), subAnimName);
						// added (Locked) to the end of the name, like is done in trackview
						if (r->GetLocked() == true)
						{
							buf += GetString(IDS_LOCKED);
						}
						return buf;
					}
				}
			}
		}
		else
			buf = GetString(IDS_UNASSIGNED);
	}
	return buf;
}


//-----------------------------------------------------------------------

// action table
static ActionDescription spActions[] = {

	ID_MIN_INFLUENCE, IDS_MIN_INFLUENCE, IDS_MIN_INFLUENCE, IDS_AF_REACTION_MANAGER,

	ID_MAX_INFLUENCE, IDS_MAX_INFLUENCE, IDS_MAX_INFLUENCE, IDS_AF_REACTION_MANAGER,

	IDC_ADD_DRIVER, IDS_ADD_DRIVER, IDS_ADD_DRIVER, IDS_AF_REACTION_MANAGER,

	IDC_REPLACE_DRIVER, IDS_REPLACE_DRIVER, IDS_REPLACE_DRIVER, IDS_AF_REACTION_MANAGER,

	IDC_ADD_DRIVEN, IDS_ADD_DRIVEN, IDS_ADD_DRIVEN, IDS_AF_REACTION_MANAGER,

	IDC_EDIT_STATE, IDS_EDIT_STATE, IDS_EDIT_STATE, IDS_AF_REACTION_MANAGER,

	IDC_ADD_SELECTED, IDS_ADD_SELECTED, IDS_ADD_SELECTED, IDS_AF_REACTION_MANAGER,

	IDC_DELETE_SELECTED, IDS_DELETE_SELECTED, IDS_DELETE_SELECTED, IDS_AF_REACTION_MANAGER,

	IDC_CREATE_STATES, IDS_CREATE_STATES, IDS_CREATE_STATES, IDS_AF_REACTION_MANAGER,

	IDC_NEW_STATE, IDS_NEW_STATE, IDS_NEW_STATE, IDS_AF_REACTION_MANAGER,

	IDC_APPEND_STATE, IDS_APPEND_STATE, IDS_APPEND_STATE, IDS_AF_REACTION_MANAGER,

	IDC_SET_STATE, IDS_SET_STATE, IDS_SET_STATE, IDS_AF_REACTION_MANAGER,

	IDC_DELETE_STATE, IDS_DELETE_STATE, IDS_DELETE_STATE, IDS_AF_REACTION_MANAGER
};

template <class T>
BOOL ReactionDlgActionCB<T>::ExecuteAction(int id)
{
	switch (id)
	{
	case ID_MIN_INFLUENCE:
		theHold.Begin();
		dlg->SetMinInfluence();
		theHold.Accept(GetString(IDS_MIN_INFLUENCE));
		break;
	case ID_MAX_INFLUENCE:
		theHold.Begin();
		dlg->SetMaxInfluence();
		theHold.Accept(GetString(IDS_MAX_INFLUENCE));
		break;
	case IDC_ADD_DRIVER:
	case IDC_REPLACE_DRIVER:
	case IDC_ADD_DRIVEN:
	case IDC_CREATE_STATES:
	case IDC_EDIT_STATE:
	{
		ICustButton* but = GetICustButton(GetDlgItem(dlg->hWnd, id));
		if (but)
			but->SetCheck(TRUE);
		ReleaseICustButton(but);
	}
		// fall through
	case IDC_ADD_SELECTED:
	case IDC_DELETE_SELECTED:
	case IDC_NEW_STATE:
	case IDC_APPEND_STATE:
	case IDC_SET_STATE:
	case IDC_DELETE_STATE:
		WPARAM wParam = MAKEWPARAM(id, 0);
		PostMessage(dlg->hWnd, WM_COMMAND, wParam, (LPARAM)GetDlgItem(dlg->hWnd, id));

		// dlg->WMCommand(id, 0, GetDlgItem(dlg->hWnd, id));
		break;
	}
	return TRUE;
}

ActionTable* GetActions()
{
	TSTR name = GetString(IDS_AF_REACTION_MANAGER);
	HACCEL hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_REACTIONMGR_SHORTCUTS));
	int numOps = _countof(spActions);
	ActionTable* pTab;
	pTab = new ActionTable(kReactionMgrActions, kReactionMgrContext, name, hAccel, numOps, spActions, hInstance);
	GetCOREInterface()->GetActionManager()->RegisterActionContext(kReactionMgrContext, name.data());
	return pTab;
}
