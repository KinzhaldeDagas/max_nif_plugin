#include "reactionMgr.h"
#include "custcont.h"

static ReactionManager theReactionMgr;
// Reaction Manager interface instance and descriptor
ReactionMgrImp theReactionMgrImp(REACTION_MANAGER_INTERFACE, _T("reactionMgr"), 0, nullptr, FP_CORE,
		IReactionManager::openEditor, _T("openEditor"), 0, TYPE_VOID, 0, 0, p_end);

ReactionManager* GetReactionManager() { return &theReactionMgr; }
void AddReactionMgrToScene();

//**********************************************************************
// ReactionManager implementation
//**********************************************************************
void PostReset(void* param, NotifyInfo* info) 
{
	if (info->intcode == NOTIFY_POST_SCENE_RESET)
	{
		ReactionManager* pReactionManager = GetReactionManager();
		if (pReactionManager)
		{
			DeleteCustAttribContainerAndNoteTracksProc deleteCustAttribContainerAndNoteTracksProc;
			pReactionManager->EnumRefHierarchy(deleteCustAttribContainerAndNoteTracksProc, false, false, false, false);
		}
		return;
	}
	AddReactionMgrToScene();
}

void PreReset(void* param, NotifyInfo* info)
{
	DbgAssert(info);

	// Do not empty the reaction manager if we're just loading a render preset file
	if (info != nullptr && info->intcode == NOTIFY_FILE_PRE_OPEN && info->callParam)
	{
		FileIOType type = *GetNotifyParam<NOTIFY_FILE_PRE_OPEN>(info);
		if (type == IOTYPE_RENDER_PRESETS)
			return;
	}

	for (int i = GetReactionManager()->NumRefs() - 1; i >= 0; i--)
		GetReactionManager()->RemoveReaction(i);
}

ReactionManager::ReactionManager()
{
	mSelected = -1;
	mSuspended = 0;
	RegisterNotification(PreReset, this, NOTIFY_SYSTEM_PRE_RESET);
	RegisterNotification(PreReset, this, NOTIFY_FILE_PRE_OPEN);
	RegisterNotification(PreReset, this, NOTIFY_SYSTEM_PRE_NEW);

	RegisterNotification(PreReset, this, NOTIFY_SYSTEM_SHUTDOWN);

	RegisterNotification(PostReset, this, NOTIFY_SYSTEM_POST_RESET);
	RegisterNotification(PostReset, this, NOTIFY_FILE_POST_OPEN);
	RegisterNotification(PostReset, this, NOTIFY_SYSTEM_POST_NEW);
	RegisterNotification(PostReset, this, NOTIFY_POST_SCENE_RESET);
}

ReactionManager::~ReactionManager() = default;

void ReactionManager::SetReference(int i, RefTargetHandle rtarg)
{
	if (i >= mReactionSets.Count())
	{
		if (rtarg != nullptr)
		{
			if (theHold.Holding())
			{
				theHold.Put(new MgrListSizeRestore(GetReactionManager(), true, i));
			}
			ReactionSet* m = (ReactionSet*)rtarg;
			mReactionSets.Append(1, &m);
		}
	}
	else
	{
		mReactionSets[i] = (ReactionSet*)rtarg;
	}
}

RefResult ReactionManager::NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate)
{
	switch (message)
	{
	case REFMSG_TARGET_DELETED:
		RemoveTarget(hTarget);
		break;
	}
	return REF_SUCCEED;
}

void ReactionManager::RemoveTarget(ReferenceTarget* target)
{
	for (int i = mReactionSets.Count() - 1; i >= 0; i--)
	{
		if (mReactionSets[i] == target)
		{
			RemoveReaction(i);
			break;
		}
	}
}

ReactionDriver* ReactionManager::AddReaction(ReferenceTarget* owner, int subAnimIndex, Reactor* driven)
{
	if (!IsSuspended())
	{
		int i;
		for (i = 0; i < mReactionSets.Count(); i++)
		{
			if (!mReactionSets[i])
				continue;
			ReactionDriver* driver = mReactionSets[i]->GetReactionDriver();
			if ((owner == nullptr && driver == nullptr) ||
					(driver != nullptr && driver->Owner() != nullptr && // don't add it to a set with a valid driver and a null client. This will soon be deleted.
						driver->Owner() == owner && driver->SubIndex() == subAnimIndex))
			{
				if (driven != nullptr)
				{
					mReactionSets[i]->AddDriven(driven);
					mSelected = i;
				}
				return driver;
			}
		}
		ReactionDriver* driver = owner ? new ReactionDriver(owner, subAnimIndex) : nullptr;

		if (theHold.Holding())
		{
			theHold.Put(new MgrListSizeRestore(GetReactionManager(), true, i));
		}
		ReactionSet* null_ptr = nullptr;
		mReactionSets.Append(1, &null_ptr);

		auto* rs = new ReactionSet(driver, driven);
		ReplaceReference(i, rs);
		mSelected = i;
		NotifyDependents(FOREVER, PART_ALL, REFMSG_SUBANIM_STRUCTURE_CHANGED);
		return driver;
	}
	return nullptr;
}

void ReactionManager::AddReactionSet(ReactionSet* set)
{
	int count = mReactionSets.Count();
	for (int i = 0; i < count; i++)
	{
		if (mReactionSets[i] == set)
			return;
	}

	if (theHold.Holding())
	{
		theHold.Put(new MgrListSizeRestore(GetReactionManager(), true, count));
	}
	ReactionSet* null_ptr = nullptr;
	mReactionSets.Append(1, &null_ptr);

	ReplaceReference(count, set);
	mSelected = count;
	NotifyDependents(FOREVER, PART_ALL, REFMSG_SUBANIM_STRUCTURE_CHANGED);
}

void ReactionManager::MaybeAddReactor(Reactor* driven)
{
	for (int i = 0; i < mReactionSets.Count(); i++)
	{
		ReactionSet* set = GetReactionSet(i);
		if (set->GetReactionDriver() == driven->GetReactionDriver())
		{
			for (int x = 0; x < set->DrivenCount(); x++)
			{
				if (set->Driven(x) == driven)
				{
					mSelected = i;
					return; // already added
				}
			}
			// if a set has this driver, but not this driven add the driven. this can happen with R7 beta files.
			set->AddDriven(driven);
			return;
		}
	}

	// if you made it this far you are probably loading an PreR7 or R7 beta file.
	if (ReactionDriver* driver = driven->GetReactionDriver())
	{
		if (auto* rs = new ReactionSet(driver, driven))
			AddReactionSet(rs);
	}
	else
	{
		GetReactionManager()->AddReaction(nullptr, 0, driven);
	}
}

void ReactionManager::RemoveDriven(ReferenceTarget* owner, int subAnimIndex, Reactor* driven)
{
	for (int i = 0; i < mReactionSets.Count(); i++)
	{
		ReactionDriver* driver = mReactionSets[i]->GetReactionDriver();
		if ((driver == nullptr && owner == nullptr) ||
				(driver != nullptr && driver->Owner() == owner && driver->SubIndex() == subAnimIndex))
		{
			if (driven != nullptr)
				mReactionSets[i]->RemoveDriven(driven);
			return;
		}
	}
}

void ReactionManager::RemoveReaction(ReferenceTarget* owner, int subAnimIndex, Reactor* driven)
{
	for (int i = 0; i < mReactionSets.Count(); i++)
	{
		ReactionDriver* driver = mReactionSets[i]->GetReactionDriver();
		if ((driver == nullptr && owner == nullptr) ||
				(driver != nullptr && driver->Owner() == owner && driver->SubIndex() == subAnimIndex))
		{
			if (driven != nullptr)
				mReactionSets[i]->RemoveDriven(driven);
			if (mReactionSets[i]->DrivenCount() == 0)
			{
				RemoveReaction(i);
			}
			return;
		}
	}
}

void ReactionManager::RemoveReaction(int i)
{
	if (i >= mReactionSets.Count())
		return;

	Suspend();
	DeleteReference(i);
	if (theHold.Holding())
		theHold.Put(new MgrListSizeRestore(GetReactionManager(), false, i));
	// Deleting the reference above may have triggered other reaction set deletions
	if (i < mReactionSets.Count())
		mReactionSets.Delete(i, 1);
	if (mSelected >= i)
		mSelected--;
	NotifyDependents(FOREVER, PART_ALL, REFMSG_SUBANIM_STRUCTURE_CHANGED);
	Resume();
}

IOResult ReactionManager::Save(ISave* isave) { return IO_OK; }
IOResult ReactionManager::Load(ILoad* iload) { return IO_OK; }

void ReactionMgrImp::OpenEditor()
{
	// open reaction manager dialog on selected objects
	if (GetReactionDlg() == nullptr)
	{
		CreateReactionDlg();
		ShowWindow(GetReactionDlg()->hWnd, SW_SHOWNORMAL);
	}
	else if (IsIconic(GetReactionDlg()->hWnd))
		ShowWindow(GetReactionDlg()->hWnd, SW_SHOWNORMAL);
	else
		DestroyWindow(GetReactionDlg()->hWnd);
}

//**********************************************************************
// ReactionSet implementation
//**********************************************************************

ReferenceTarget* ReactionSet::GetReference(int i)
{
	if (i == 0)
		return mDriver;
	else
		return mDriven[i - 1];
}

void ReactionSet::SetReference(int i, RefTargetHandle rtarg)
{
	if (i == 0)
	{
		mDriver = static_cast<ReactionDriver*>(rtarg);
	}
	else
	{
		i--;
		if (i >= mDriven.Count())
		{
			if (rtarg != nullptr)
			{
				if (theHold.Holding())
				{
					theHold.Put(new DrivenListSizeRestore(this, true, i));
				}
				Reactor* r = (Reactor*)rtarg;
				mDriven.Append(1, &r);
			}
		}
		else
		{
			mDriven[i] = (Reactor*)rtarg;
		}
	}
}

RefResult ReactionSet::NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate)
{
	switch (message)
	{
	case REFMSG_TARGET_DELETED:
	{
		if (hTarget == mDriver)
		{
			mDriver = nullptr;
			return REF_AUTO_DELETE;
		}
		else
		{
			for (int i = mDriven.Count() - 1; i >= 0; i--)
			{
				if (hTarget == mDriven[i])
				{
					if (theHold.Holding())
						theHold.Put(new DrivenListSizeRestore(this, false, i));
					mDriven.Delete(i, 1);
					NotifyDependents(FOREVER, PART_ALL, REFMSG_SUBANIM_STRUCTURE_CHANGED);
					break;
				}
			}
		}
	}

	break;
	}
	return REF_SUCCEED;
}

void ReactionSet::AddDriven(Reactor* r)
{
	int i;
	for (i = 0; i < mDriven.Count(); i++)
	{
		if (mDriven[i] == r)
			return;
	}

	if (theHold.Holding())
	{
		theHold.Put(new DrivenListSizeRestore(this, true, i));
	}
	Reactor* null_ptr = nullptr;
	mDriven.Append(1, &null_ptr);

	ReplaceReference(DrivenNumToRefNum(i), r);
	NotifyDependents(FOREVER, PART_ALL, REFMSG_SUBANIM_STRUCTURE_CHANGED);
}

void ReactionSet::RemoveDriven(Reactor* r)
{
	for (int i = 0; i < mDriven.Count(); i++)
	{
		if (mDriven[i] == r)
		{
			RemoveDriven(i);
			return;
		}
	}
}

void ReactionSet::RemoveDriven(int i)
{
	DeleteReference(DrivenNumToRefNum(i));
	mDriven.Delete(i, 1);
	if (theHold.Holding())
	{
		theHold.Put(new DrivenListSizeRestore(this, false, i));
	}
	NotifyDependents(FOREVER, PART_ALL, REFMSG_SUBANIM_STRUCTURE_CHANGED);
}

void ReactionSet::ReplaceDriver(ReferenceTarget* parent, int subAnim)
{
	int defaultID = -1;
	bool aborted = false;

	for (int i = 0; i < GetReactionManager()->ReactionCount(); i++)
	{
		ReactionSet* set = GetReactionManager()->GetReactionSet(i);
		if (set != this)
		{
			ReactionDriver* driver = set->GetReactionDriver();
			if (driver != nullptr && driver->Owner() == parent && driver->subnum == subAnim)
			{
				aborted = true;
				break;
			}
		}
	}

	if (!aborted)
	{
		auto* driver = new ReactionDriver(parent, subAnim);
		// driver->AddState(nullptr, defaultID, true, GetCOREInterface()->GetTime());
		ReplaceReference(0, driver);
	}

	if (DrivenCount())
	{
		for (int i = DrivenCount() - 1; i >= 0; i--)
		{
			BOOL succeed = false;
			if (Reactor* r = Driven(i))
			{
				if (parent->SuperClassID() == BASENODE_CLASS_ID)
					succeed = r->reactTo(parent, GetCOREInterface()->GetTime(), false);
				else
					succeed = r->reactTo(static_cast<ReferenceTarget*>(parent->SubAnim(subAnim)), GetCOREInterface()->GetTime(), false);

				if (succeed)
					r->CreateReactionAndReturn(true, nullptr, GetCOREInterface()->GetTime(), defaultID);
				else
					return;
			}
		}
		NotifyDependents(FOREVER, PART_ALL, REFMSG_SUBANIM_STRUCTURE_CHANGED);
	}
	else
	{
		NotifyDependents(FOREVER, PART_ALL, REFMSG_SUBANIM_STRUCTURE_CHANGED);
		MaybeAutoDelete();
	}
}

class ReactionSetPostLoadCallback : public PostLoadCallback
{
	ReactionSet* mSet = nullptr;

public:
	explicit ReactionSetPostLoadCallback(ReactionSet* s) : mSet(s) {}

	int Priority() override { return 3; }
	// CleanUpReactionSet removes null driven from the driven list
	int CleanUpReactionSet(ReactionSet* in_set)
	{
		int nbCleaned = 0;
		if (in_set)
		{
			for (int i = in_set->DrivenCount() - 1; i >= 0; i--)
			{
				if (!in_set->Driven(i))
				{
					in_set->RemoveDriven(i);
					nbCleaned++;
				}
			}
		}
		return nbCleaned;
	}
	void proc(ILoad* iload) override
	{
		CleanUpReactionSet(mSet);
		GetReactionManager()->AddReactionSet(mSet);
	}
};

#define DRIVEN_COUNT_CHUNK 0x5001

IOResult ReactionSet::Save(ISave* isave)
{
	ULONG nb;

	isave->BeginChunk(DRIVEN_COUNT_CHUNK);
	int count = mDriven.Count();
	isave->Write(&count, sizeof(int), &nb);
	isave->EndChunk();

	return IO_OK;
}

IOResult ReactionSet::Load(ILoad* iload)
{
	ULONG nb;
	int id;

	while (iload->OpenChunk() == IO_OK)
	{
		switch (id = iload->CurChunkID())
		{
		case DRIVEN_COUNT_CHUNK:
		{
			int count = 0;
			iload->Read(&count, sizeof(int), &nb);
			mDriven.SetCount(count);
			for (int i = 0; i < count; i++)
				mDriven[i] = nullptr;

			break;
		}
		}
		iload->CloseChunk();
	}

	auto* plc = new ReactionSetPostLoadCallback(this);
	iload->RegisterPostLoadCallback(plc);

	return IO_OK;
}

void AddReactionMgrToScene()
{
	Interface* ip = GetCOREInterface();
	if (ip == nullptr)
		return;

	ITrackViewNode* tvRoot = ip->GetTrackViewRootNode();
	if (tvRoot->FindItem(REACTION_MGR_CLASSID) < 0)
	{
		tvRoot->AddController(GetReactionManager(), _T("Reaction Manager"), REACTION_MGR_CLASSID);
	}
}

///*****************************************************************
/// Class descriptors
///*****************************************************************

class ReactionManagerClassDesc : public ClassDesc2
{
public:
	ReactionManagerClassDesc() { AddReactionMgrToScene(); }

	int IsPublic() override { return FALSE; }
	void* Create(BOOL loading) override { return GetReactionManager(); }
	const TCHAR* ClassName() override { return GetString(IDS_AF_REACTION_MANAGER); }
	const TCHAR* NonLocalizedClassName() override { return _T("Reaction Manager"); }
	SClass_ID SuperClassID() override { return REF_MAKER_CLASS_ID; }
	Class_ID ClassID() override { return REACTION_MGR_CLASSID; }
	const TCHAR* Category() override { return _T(""); }

	const TCHAR* InternalName() override { return _T("ReactionManager"); } // returns fixed parsable name (scripter-visible name)
	HINSTANCE HInstance() override { return hInstance; } // returns owning module handle
	void ResetClassParams(BOOL fileReset) override {}

	int NumActionTables() override { return 1; }
	ActionTable* GetActionTable(int i) override { return GetActions(); }
};

class ReactionSetClassDesc : public ClassDesc2
{
public:
	int IsPublic() override { return FALSE; }
	void* Create(BOOL loading) override { return new ReactionSet(); }
	const TCHAR* ClassName() override { return GetString(IDS_AF_REACTION_SET); }
	const TCHAR* NonLocalizedClassName() override { return _T("Reaction Set"); }
	SClass_ID SuperClassID() override { return REF_MAKER_CLASS_ID; }
	Class_ID ClassID() override { return REACTIONSET_CLASSID; }
	const TCHAR* Category() override { return _T(""); }

	const TCHAR* InternalName() override { return _T("ReactionSet"); } // returns fixed parsable name (scripter-visible name)
	HINSTANCE HInstance() override { return hInstance; } // returns owning module handle
};

static ReactionManagerClassDesc reactionMgrCD;
ClassDesc* GetReactionManagerDesc() { return &reactionMgrCD; }

static ReactionSetClassDesc reactionSetCD;
ClassDesc* GetReactionSetDesc() { return &reactionSetCD; }
