#include "reactor.h"
#include "tvnode.h"

#pragma once

#define REACTION_MGR_CLASSID Class_ID(0x294a389c, 0x87906d7)
#define REACTIONSET_CLASSID Class_ID(0x55293bb4, 0x4ad13dad)

class MgrListSizeRestore;
class DrivenListSizeRestore;

extern ClassDesc* GetReactionManagerDesc();
extern ClassDesc* GetReactionSetDesc();

class ReactionSet : public ReferenceTarget
{
	friend class DrivenListSizeRestore;
	friend class ReactionManager;

	ReactionDriver* mDriver = nullptr;
	Tab<Reactor*> mDriven;

public:
	ReactionSet() { mDriven.Init(); }
	ReactionSet(ReactionDriver* driver, Reactor* driven)
	{
		if (driver != nullptr)
			ReplaceReference(0, driver);

		mDriven.Init();
		if (driven != nullptr)
			AddDriven(driven);
	}
	~ReactionSet() = default;

	void DeleteThis() override { delete this; }
	Class_ID ClassID() override { return REACTIONSET_CLASSID; }
	SClass_ID SuperClassID() override { return REF_MAKER_CLASS_ID; }
	void GetClassName(MSTR& s, bool localized) const override
	{
		s = localized ? GetString(IDS_AF_REACTION_SET) : _T("Reaction Set");
	}

	IOResult Save(ISave* isave) override;
	IOResult Load(ILoad* iload) override;

	int NumRefs() override { return mDriven.Count() + 1; }
	ReferenceTarget* GetReference(int i) override;

private:
	virtual void SetReference(int i, RefTargetHandle rtarg) override;

public:
	RefResult NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate) override;
	BOOL IsRealDependency(ReferenceTarget* rtarg) override
	{
		return (rtarg == mDriver && mDriver->GetReference(0) != nullptr);
	}

	ReactionDriver* GetReactionDriver() { return mDriver; }
	int DrivenCount() const { return mDriven.Count(); }
	Reactor* Driven(int i) { return mDriven[i]; }
	void AddDriven(Reactor* r);
	void RemoveDriven(Reactor* r);
	void RemoveDriven(int i);

	void ReplaceDriver(ReferenceTarget* parent, int subAnim);

	int DrivenNumToRefNum(int i) const { return i + 1; }
};

// ---------  reaction manager  ---------

#define REACTION_MANAGER_INTERFACE Interface_ID(0x100940fa, 0x43aa3a02)

class IReactionManager : public FPStaticInterface
{
public:
	// function IDs
	enum { openEditor };

	virtual void OpenEditor() = 0; // open the reaction manager dialog
};

class ReactionMgrImp : public IReactionManager
{
	void OpenEditor() override;

	DECLARE_DESCRIPTOR(ReactionMgrImp)
	// dispatch map
	BEGIN_FUNCTION_MAP
	VFN_0(openEditor, OpenEditor);
	END_FUNCTION_MAP
};

class ReactionManager : public Control
{
	friend class MgrListSizeRestore;

	// Keep track of all drivers and a list of all driven assigned to that driver
	Tab<ReactionSet*> mReactionSets;
	int mSelected;
	int mSuspended;

public:
	ReactionManager();
	~ReactionManager();

	RefResult AutoDelete() override { return REF_FAIL; } // cannot be deleted.
	Class_ID ClassID() override { return REACTION_MGR_CLASSID; }
	SClass_ID SuperClassID() override { return REF_MAKER_CLASS_ID; }
	void GetClassName(MSTR& s, bool localized) const override
	{
		s = localized ? GetString(IDS_AF_REACTION_MANAGER) : _T("Reaction Manager");
	}

	IOResult Save(ISave* isave) override;
	IOResult Load(ILoad* iload) override;

	int NumRefs() override { return mReactionSets.Count(); }
	ReferenceTarget* GetReference(int i) override { return (ReferenceTarget*)mReactionSets[i]; }

private:
	virtual void SetReference(int i, RefTargetHandle rtarg) override;

public:
	RefResult NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate) override;
	BOOL IsRealDependency(ReferenceTarget* rtarg)
	{
		auto* set = static_cast<ReactionSet*>(rtarg);
		return (set->GetReactionDriver() != nullptr && set->GetReactionDriver()->Owner() != nullptr);
	}
	BOOL BypassTreeView() override { return TRUE; }

	void Copy(Control*) override {}
	BOOL IsLeaf() override { return FALSE; }
	void GetValue(TimeValue, void*, Interval&, GetSetMethod) override {}
	void SetValue(TimeValue, void*, int, GetSetMethod) {}

	ReactionDriver* AddReaction(ReferenceTarget* owner, int subAnimIndex, Reactor* driven = nullptr);
	void RemoveDriven(ReferenceTarget* owner, int subAnimIndex, Reactor* driven);
	void RemoveReaction(ReferenceTarget* owner, int subAnimIndex, Reactor* driven = nullptr);
	int ReactionCount() { return mReactionSets.Count(); }
	ReactionSet* GetReactionSet(int i) { return mReactionSets[i]; }
	void AddReactionSet(ReactionSet* set);
	void MaybeAddReactor(Reactor* driven);
	void RemoveTarget(ReferenceTarget* target);
	void RemoveReaction(int i);

	int GetSelected() const { return mSelected; }
	void SetSelected(int index) { mSelected = index; }

private:
	void Suspend() { mSuspended++; }
	void Resume() { mSuspended--; }
	bool IsSuspended() const { return mSuspended > 0; }
};

ReactionManager* GetReactionManager();

#define CHILDINDENT 2 // the amount to indent the name of a driven

class ReactionListItem;
class StateListItem;

enum
{
	kExpandCol = -1, // pseudo column to contain the expand/collapse icon
	kNameCol = 0,

	kFromCol = 1,
	kToCol = 2,
	kCurveCol = 3,

	kValueCol = 1,
	kStrengthCol = 2,
	kInfluenceCol = 3,
	kFalloffCol = 4,
};

class ReactionDlg;

template <class T>
class ReactionDlgActionCB : public ActionCallback
{
public:
	T* dlg = nullptr;
	explicit ReactionDlgActionCB(T* d) : dlg(d) {}
	BOOL ExecuteAction(int id) override;
};

ReactionDlg* GetReactionDlg();
void CreateReactionDlg();

class ReactionDlg : public ReferenceTarget, public TimeChangeCallback
{
public:
	IObjParam* ip = nullptr;
	HWND hWnd;
	bool valid, selectionValid, stateSelectionValid, reactionListValid;
	POINT* origPt = nullptr;
	ICustEdit* floatWindow = nullptr;
	bool blockInvalidation;
	float rListPos, sListPos;

	WNDPROC reactionListWndProc;
	WNDPROC stateListWndProc;
	WNDPROC splitterWndProc;

	ReactionDlg(IObjParam* ip, HWND hParent);
	~ReactionDlg();

	Class_ID ClassID() override { return REACTIONDLG_CLASS_ID; }
	SClass_ID SuperClassID() override { return REF_MAKER_CLASS_ID; }
	virtual void GetClassName(MSTR& s, bool localized) const override { s = _M("ReactionDlg"); }
	RefResult AutoDelete() override { return REF_SUCCEED; }

	void TimeChanged(TimeValue t) override { /*Invalidate();*/ }
	static void PreSceneNodeDeleted(void* param, NotifyInfo* info);
	static void SelectionSetChanged(void* param, NotifyInfo* info);
	static void PreReset(void* param, NotifyInfo* info);
	static void PreOpen(void* param, NotifyInfo* info);
	static void PostOpen(void* param, NotifyInfo* info);


	void SetupUI(HWND hWnd);
	void PerformLayout();
	void SetupReactionList();
	void SetupStateList();
	void FillButton(HWND hWnd, int butID, int iconIndex, int disabledIconIndex, TSTR toolText, CustButType type);
	void BuildCurveCtrl();

	void Invalidate();
	void InvalidateReactionList()
	{
		reactionListValid = false;
		selectionValid = false;
		InvalidateRect(GetDlgItem(hWnd, IDC_REACTION_LIST), nullptr, FALSE);
	}
	void InvalidateSelection()
	{
		selectionValid = false;
		InvalidateRect(hWnd, nullptr, FALSE);
	}

	void InvalidateStateSelection()
	{
		stateSelectionValid = false;
		InvalidateRect(hWnd, nullptr, FALSE);
	}

	void ClearCurves();

	void Update();
	void UpdateReactionList();
	void UpdateStateList();
	void UpdateCurveControl();
	void UpdateButtons();
	void UpdateStateButtons();
	void UpdateEditStates();
	void UpdateCreateStates();
	void SelectionChanged();
	void StateSelectionChanged();

	void RedrawControl();
	void EnableControlDraw(BOOL);
	void SetControlXRange(float min, float max, BOOL rescaleKeys = TRUE);
	Point2 GetControlXRange();

	void Change(BOOL redraw = FALSE);

	bool ToggleReactionExpansionState(int rowNum);
	bool AddDrivenRow(HWND hList, ReactionListItem* listItem, int itemID);
	bool InsertListItem(ReactionListItem* pItem, int after);
	void InsertDrivenInList(ReactionSet* set, int after);
	void RemoveDrivenFromList(ReactionSet* set, int after);

	bool ToggleStateExpansionState(int rowNum);
	bool AddDrivenStateRow(HWND hList, StateListItem* listItem, int itemID);
	bool InsertListItem(StateListItem* pItem, int after);
	void InsertStatesInList(DriverState* state, ReactionSet* owner, int after);
	void RemoveStatesFromList(DriverState* state, int after);

	ReactionListItem* GetReactionDataAt(int where);
	StateListItem* GetStateDataAt(int where);
	ReactionListItem* GetSelectedDriver();
	Tab<ReactionListItem*> GetSelectedDriven();
	Tab<StateListItem*> GetSelectedStates();
	Tab<StateListItem*> GetSelectedDriverStates();
	Tab<StateListItem*> GetSelectedDrivenStates();

	BOOL IsDriverStateSelected();
	BOOL IsDrivenStateSelected();

	POINT HitTest(POINT& p, UINT id);
	BOOL OnLMouseButtonDown(HWND hDlg, WPARAM wParam, POINT lParam, UINT id);
	void WMCommand(int id, int notify, HWND hCtrl);
	void SpinnerChange(int id, POINT p, int nRow, int nCol);
	void SpinnerEnd(int id, BOOL cancel, int nRow, int nCol);
	void SetMinInfluence();
	void SetMaxInfluence();

	void InvokeRightClickMenu();

	RefResult NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate) override;
	int NumRefs() override;
	RefTargetHandle GetReference(int i) override;

private:
	virtual void SetReference(int i, RefTargetHandle rtarg) override;

public:
	BOOL IsRealDependency(ReferenceTarget* rtarg) override;

private:
	ICurveCtl* iCCtrl = nullptr;
	INodeTab selectedNodes;
	RefTargetHandle reactionManager;
	ReactionPickMode* mCurPickMode = nullptr;
	ReactionDlgActionCB<ReactionDlg>* reactionDlgActionCB; // Actions handler
	static const int kMaxTextLength;
	static const int kNumReactionColumns;
	static const int kNumStateColumns;
	float origValue;
	static HIMAGELIST mIcons;
	TimeValue origTime;
	void GiveUpDisplay(ICurve* curve);
};

class ReactionListItem
{
public:
	enum ItemType { kReactionSet, kDriven };

private:
	void* mItem = nullptr;
	ItemType mType;
	bool mIsExpanded = true;
	ReferenceTarget* owner = nullptr;
	int index;

public:
	ReactionListItem(ItemType type, void* obj, ReferenceTarget* o, int id)
		: mItem(obj), mType(type), owner(o), index(id) {}

	ReactionSet* GetReactionSet() { return (mType == kReactionSet) ? static_cast<ReactionSet*>(mItem) : nullptr; }
	Reactor* Driven() { return (mType == kDriven) ? static_cast<Reactor*>(mItem) : nullptr; }

	TSTR GetName();

	ReferenceTarget* GetOwner() { return owner; }
	int GetIndex() const { return index; }

	bool GetRange(TimeValue& t, bool start);
	void SetRange(TimeValue t, bool start);
	void GetValue(void* val);

	void ToggleUseCurve()
	{
		if (Reactor* driven = Driven())
		{
			driven->useCurve(!driven->useCurve());
		}
	}

	BOOL IsUsingCurve()
	{
		Reactor* driven = Driven();
		return driven ? driven->useCurve() : false;
	}

	bool IsExpandable()
	{
		if (ReactionSet* set = GetReactionSet())
		{
			for (int j = 0; j < set->DrivenCount(); j++)
			{
				Reactor* r = set->Driven(j);
				if (!r)
					continue;
				MyEnumProc dep(r);
				r->DoEnumDependents(&dep);

				for (int x = 0; x < dep.anims.Count(); x++)
				{
					for (int i = 0; i < dep.anims[x]->NumSubs(); i++)
					{
						Animatable* n = dep.anims[x]->SubAnim(i);
						if (n == r)
						{
							return true; // found one in the scene
						}
					}
				}
			}
		}
		return false;
	}

	bool IsExpanded() const { return mIsExpanded; }
	void Expand(bool expanded) { mIsExpanded = expanded; }
};

class StateListItem
{
public:
	enum ItemType { kDriverState, kDrivenState, };

private:
	void* mItem = nullptr;
	ReferenceTarget* owner = nullptr;
	ItemType mType;
	bool mIsExpanded = true;
	int index;

public:
	StateListItem(ItemType type, void* obj, ReferenceTarget* o, int id)
		: mItem(obj), mType(type), owner(o), index(id) {}

	DriverState* GetDriverState() { return (mType == kDriverState) ? static_cast<DriverState*>(mItem) : nullptr; }

	DrivenState* GetDrivenState()
	{
		if (mType == kDrivenState && GetOwner())
		{
			if (auto* r = static_cast<Reactor*>(GetOwner()))
			{
				DbgAssert(GetIndex() < r->mDrivenStates.Count());
				if (GetIndex() < r->mDrivenStates.Count())
					return r->mDrivenStates.Addr(GetIndex());
			}
		}
		return nullptr;
	}

	ReferenceTarget* GetOwner() { return owner; }
	int GetIndex() const { return index; }

	TSTR GetName();
	int GetType();
	void GetValue(void* val);
	void SetValue(float val);
	float GetInfluence();
	void SetInfluence(float val);

	bool IsExpandable()
	{
		DriverState* state = GetDriverState();
		// find any driven that use this driver state
		for (int j = 0; j < GetReactionManager()->ReactionCount(); j++)
		{
			ReactionSet* set = GetReactionManager()->GetReactionSet(j);
			for (int k = 0; k < set->DrivenCount(); k++)
			{
				if (Reactor* r = set->Driven(k))
				{
					for (int x = 0; x < r->mDrivenStates.Count(); x++)
					{
						if (r->getDriverState(x) == state)
						{
							return true;
						}
					}
				}
			}
		}
		return false;
	}

	bool IsExpanded() const { return mIsExpanded; }
	void Expand(bool expanded) { mIsExpanded = expanded; }
};

class ReactionDriverPickMode : public ReactionPickMode
{
	ICustButton* but = nullptr;

public:
	explicit ReactionDriverPickMode(ICustButton* b) : but(b) {}
	~ReactionDriverPickMode()
	{
		if (but)
			ReleaseICustButton(but);
	}
	TSTR GetName() const override { return GetString(IDS_ADD_DRIVER); }
	bool LoopTest(ReferenceTarget* anim) override { return true; }
	void NodePicked(ReferenceTarget* anim) override;
	bool IncludeRoots() override { return true; }
	bool MultipleChoice() override { return true; }
	void PostPick() override
	{
		// if(GetReactionDlg() != nullptr)
		//	GetReactionDlg()->ip->GetActionManager()->FindTable(kReactionMgrActions)->GetAction(IDC_ADD_DRIVEN)->ExecuteAction();
	}

	void EnterMode(IObjParam* ip) override
	{
		ReactionPickMode::EnterMode(ip);
		if (but)
			but->SetCheck(TRUE);
	}
	void ExitMode(IObjParam* ip) override
	{
		ReactionPickMode::ExitMode(ip);
		if (but)
			but->SetCheck(FALSE);
	}
};

class ReplaceDriverPickMode : public ReactionPickMode
{
	ICustButton* but = nullptr;
	ReactionSet* mSet = nullptr;

public:
	ReplaceDriverPickMode(ReactionSet* set, ICustButton* b) : mSet(set), but(b) {}
	~ReplaceDriverPickMode()
	{
		if (but)
			ReleaseICustButton(but);
	}
	TSTR GetName() const override { return GetString(IDS_REPLACE_DRIVER); }
	bool LoopTest(ReferenceTarget* anim) override;
	void NodePicked(ReferenceTarget* anim) override;
	bool IncludeRoots() override { return true; }

	void EnterMode(IObjParam* ip) override
	{
		ReactionPickMode::EnterMode(ip);
		if (but)
			but->SetCheck(TRUE);
	}
	void ExitMode(IObjParam* ip) override
	{
		ReactionPickMode::ExitMode(ip);
		if (but)
			but->SetCheck(FALSE);
	}
};


class ReactionDrivenPickMode : public ReactionPickMode
{
	ReactionSet* mDriver = nullptr;
	Tab<int> mDriverIDs;
	ICustButton* but = nullptr;

public:
	ReactionDrivenPickMode(ReactionSet* m, Tab<int> driverIDs, ICustButton* b)
		: mDriver(m), mDriverIDs(driverIDs), but(b) {}
	~ReactionDrivenPickMode()
	{
		if (but)
			ReleaseICustButton(but);
	}

	TSTR GetName() const override { return GetString(IDS_ADD_DRIVEN); }
	bool LoopTest(ReferenceTarget* anim) override;
	void NodePicked(ReferenceTarget* anim) override;
	bool IncludeRoots() override { return false; }
	bool MultipleChoice() override { return true; }
	bool FilterAnimatable(Animatable* anim);

	void EnterMode(IObjParam* ip) override
	{
		ReactionPickMode::EnterMode(ip);
		if (but)
			but->SetCheck(TRUE);
	}
	void ExitMode(IObjParam* ip) override
	{
		ReactionPickMode::ExitMode(ip);
		if (but)
			but->SetCheck(FALSE);
	}
};

class DrivenListSizeRestore : public RestoreObj
{
public:
	ReactionSet* mDriver = nullptr;
	bool mIncrease;
	int mIndex;

	DrivenListSizeRestore(ReactionSet* m, bool increase, int index)
		: mDriver(m), mIncrease(increase), mIndex(index) {}
	
	void Restore(int isUndo) override
	{
		if (mIncrease)
			mDriver->mDriven.Resize(mDriver->mDriven.Count() - 1);
		else
		{
			Reactor* dummyEntry = nullptr;
			mDriver->mDriven.Insert(mIndex, 1, &dummyEntry);
		}
		mDriver->NotifyDependents(FOREVER, 0, REFMSG_SUBANIM_STRUCTURE_CHANGED);
	}
	void Redo() override
	{
		if (mIncrease)
		{
			mDriver->mDriven.SetCount(mDriver->mDriven.Count() + 1);
			mDriver->mDriven[mDriver->mDriven.Count() - 1] = nullptr;
		}
		else
			mDriver->mDriven.Delete(mIndex, 1);
		mDriver->NotifyDependents(FOREVER, 0, REFMSG_SUBANIM_STRUCTURE_CHANGED);
	}
	TSTR Description() override { return _T("Driven Count"); }
};


class MgrListSizeRestore : public RestoreObj
{
public:
	ReactionManager* mMgr = nullptr;
	bool mIncrease;
	int mIndex;

	MgrListSizeRestore(ReactionManager* m, bool increase, int index)
		: mMgr(m), mIncrease(increase), mIndex(index) {}
	
	void Restore(int isUndo) override
	{
		if (mIncrease)
			mMgr->mReactionSets.SetCount(mMgr->mReactionSets.Count() - 1);
		else
		{
			ReactionSet* dummyEntry = nullptr;
			mMgr->mReactionSets.Insert(mIndex, 1, &dummyEntry);
		}
		mMgr->NotifyDependents(FOREVER, 0, REFMSG_SUBANIM_STRUCTURE_CHANGED);
	}

	void Redo() override
	{
		if (mIncrease)
		{
			mMgr->mReactionSets.SetCount(mMgr->mReactionSets.Count() + 1);
			mMgr->mReactionSets[mMgr->mReactionSets.Count() - 1] = nullptr;
		}
		else
			mMgr->mReactionSets.Delete(mIndex, 1);
		mMgr->NotifyDependents(FOREVER, 0, REFMSG_SUBANIM_STRUCTURE_CHANGED);
	}

	TSTR Description() override { return _T("Manager Size"); }
};
