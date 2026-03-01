#include "reactionMgr.h"

#pragma once

class FullRestore : public RestoreObj
{
public:
	Reactor* sav = nullptr;
	Reactor* cur = nullptr;
	Reactor* redo = nullptr;

	FullRestore() = default;
	explicit FullRestore(Reactor* cont) : cur(cont)
	{
		sav = new Reactor();
		*sav = *cur;
	}
	~FullRestore()
	{
		if (sav) delete sav;
		if (redo) delete redo;
	}

	void Restore(int isUndo)  override
	{
		assert(cur);
		assert(sav);
		if (isUndo)
		{
			redo = new Reactor();
			*redo = *cur;
		}
		// cur->Copy(sav);
		*cur = *sav;
	}
	void Redo() override
	{
		assert(cur);
		if (redo)
			// cur->Copy(redo);
			*cur = *redo;
	}
	void EndHold() override {}
	TSTR Description() override { return _T("FullReactorRestore"); }
};


// A restore object to save the influence, strength, and falloff.
class SpinnerRestore : public RestoreObj
{
public:
	Reactor* cont = nullptr;
	Tab<DrivenState> ureaction, rreaction;
	int uselected, rselected;
	UINT flags;

	explicit SpinnerRestore(Reactor* c) : cont(c)
	{
		// ureaction = cont->reaction;
		ureaction.SetCount(cont->mDrivenStates.Count());
		for (int i = 0; i < cont->mDrivenStates.Count(); i++)
		{
			memset(&ureaction[i], 0, sizeof(DrivenState));
			ureaction[i] = cont->mDrivenStates[i];
		}
		uselected = cont->selected;
		flags = cont->flags;
	}

	void Restore(int isUndo) override
	{
		// if we're undoing, save a redo state
		if (isUndo)
		{
			// rreaction = cont->reaction;
			rreaction.SetCount(cont->mDrivenStates.Count());
			for (int i = 0; i < cont->mDrivenStates.Count(); i++)
			{
				memset(&rreaction[i], 0, sizeof(DrivenState));
				rreaction[i] = cont->mDrivenStates[i];
			}

			rselected = cont->selected;
		}
		// cont->reaction = ureaction;
		cont->mDrivenStates.SetCount(ureaction.Count());
		for (int i = 0; i < ureaction.Count(); i++)
		{
			memset(&cont->mDrivenStates[i], 0, sizeof(DrivenState));
			cont->mDrivenStates[i] = ureaction[i];
		}
		cont->selected = uselected;
		if (isUndo && !(flags & REACTOR_BLOCK_CURVE_UPDATE))
		{
			cont->RebuildFloatCurves();
		}
		cont->count = cont->mDrivenStates.Count();
		cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_REACTION_COUNT_CHANGED);
	}
	void Redo() override
	{
		// cont->reaction = rreaction;
		cont->mDrivenStates.SetCount(rreaction.Count());
		for (int i = 0; i < rreaction.Count(); i++)
		{
			memset(&cont->mDrivenStates[i], 0, sizeof(DrivenState));
			cont->mDrivenStates[i] = rreaction[i];
		}
		cont->selected = rselected;
		cont->count = cont->mDrivenStates.Count();
		if (!(flags & REACTOR_BLOCK_CURVE_UPDATE))
			cont->RebuildFloatCurves();
		cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_REACTION_COUNT_CHANGED);
	}
	void EndHold() override {}
	int Size() override { return sizeof(cont->mDrivenStates) + sizeof(float); }
};


class StateRestore : public RestoreObj
{
public:
	Reactor* cont = nullptr;
	Point3 ucurpval, rcurpval;
	float ucurfval, rcurfval;
	Quat ucurqval, rcurqval;
	Tab<DrivenState> ureaction, rreaction;

	explicit StateRestore(Reactor* c) : cont(c)
	{
		ucurpval = cont->curpval;
		ucurqval = cont->curqval;
		ucurfval = cont->curfval;
		// ureaction = cont->reaction;
		ureaction.SetCount(cont->mDrivenStates.Count());
		for (int i = 0; i < cont->mDrivenStates.Count(); i++)
		{
			memset(&ureaction[i], 0, sizeof(DrivenState));
			ureaction[i] = cont->mDrivenStates[i];
		}
	}

	void Restore(int isUndo) override
	{
		if (isUndo)
		{
			rcurpval = cont->curpval;
			rcurqval = cont->curqval;
			rcurfval = cont->curfval;
			// rreaction = cont->reaction;
			rreaction.SetCount(cont->mDrivenStates.Count());
			for (int i = 0; i < cont->mDrivenStates.Count(); i++)
			{
				memset(&rreaction[i], 0, sizeof(DrivenState));
				rreaction[i] = cont->mDrivenStates[i];
			}
		}
		cont->curpval = ucurpval;
		cont->curqval = ucurqval;
		cont->curfval = ucurfval;
		// cont->reaction = ureaction;
		cont->mDrivenStates.SetCount(ureaction.Count());
		for (int i = 0; i < ureaction.Count(); i++)
		{
			memset(&cont->mDrivenStates[i], 0, sizeof(DrivenState));
			cont->mDrivenStates[i] = ureaction[i];
		}
		if (isUndo)
			cont->UpdateCurves();
		cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}
	void Redo() override
	{
		cont->curpval = rcurpval;
		cont->curqval = rcurqval;
		cont->curfval = rcurfval;
		// cont->reaction = rreaction;
		cont->mDrivenStates.SetCount(rreaction.Count());
		for (int i = 0; i < rreaction.Count(); i++)
		{
			memset(&cont->mDrivenStates[i], 0, sizeof(DrivenState));
			cont->mDrivenStates[i] = rreaction[i];
		}

		cont->UpdateCurves();
		cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}
	void EndHold() override { cont->ClearAFlag(A_HELD); }
	TSTR Description() override { return _T("Reactor State"); }
};

class RangeRestore : public RestoreObj
{
public:
	Reactor* cont = nullptr;
	Interval ur, rr;
	RangeRestore(Reactor* c) : cont(c), ur(cont->range) {}

	void Restore(int isUndo) override
	{
		rr = cont->range;
		cont->range = ur;
		cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}
	void Redo() override
	{
		cont->range = rr;
		cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}
	void EndHold() override { cont->ClearAFlag(A_HELD); }
	TSTR Description() override { return _T("Reactor control range"); }
};

class DriverStateRestore : public RestoreObj
{
public:
	ReactionDriver* mDriver = nullptr;
	float fVal;
	Quat qVal;
	Point3 pVal;
	int index;

	DriverStateRestore(ReactionDriver* m, int i) : mDriver(m), index(i),
		fVal(m->GetState(i)->fvalue),
		pVal(m->GetState(i)->pvalue),
		qVal(m->GetState(i)->qvalue) {}
	
	void Restore(int isUndo) override
	{
		float tempFVal = 0;
		Point3 tempPVal;
		Quat tempQVal;
		DriverState* state = mDriver->GetState(index);
		if (isUndo)
		{
			tempFVal = state->fvalue;
			tempPVal = state->pvalue;
			tempQVal = state->qvalue;
		}

		state->fvalue = fVal;
		state->pvalue = pVal;
		state->qvalue = qVal;
		if (isUndo)
		{
			fVal = tempFVal;
			pVal = tempPVal;
			qVal = tempQVal;
		}
		mDriver->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}
	void Redo() override { Restore(TRUE); }
	TSTR Description() override { return _T("Driver State Restore"); }
};

class DriverStateListRestore : public RestoreObj
{
public:
	ReactionDriver* mDriver = nullptr;
	Tab<DriverState*> states;

	explicit DriverStateListRestore(ReactionDriver* m) : mDriver(m)
	{
		for (int i = 0; i < mDriver->states.Count(); i++)
		{
			DriverState* newState = mDriver->states[i]->Clone();
			states.Append(1, &newState);
		}
	}
	~DriverStateListRestore()
	{
		for (int i = 0; i < states.Count(); i++)
		{
			delete states[i];
		}
	}

	void Restore(int isUndo) override
	{
		Tab<DriverState*> tempStates;
		if (isUndo)
		{
			for (int i = 0; i < mDriver->states.Count(); i++)
			{
				DriverState* newState = mDriver->states[i]->Clone();
				tempStates.Append(1, &newState);
				delete mDriver->states[i];
			}
		}

		mDriver->states = states;

		if (isUndo)
			states = tempStates;

		mDriver->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		mDriver->NotifyDependents(FOREVER, PART_ALL, REFMSG_REACTION_COUNT_CHANGED);
	}
	void Redo() override { Restore(TRUE); }
	TSTR Description() { return _T("Driver State List Restore"); }
};


class DriverIDRestore : public RestoreObj
{
public:
	Reactor* mReactor = nullptr;
	int mID;

	DriverIDRestore(Reactor* r, int id) : mReactor(r), mID(id) {}
	~DriverIDRestore() = default;

	void Restore(int isUndo) override
	{
		for (int x = mReactor->mDrivenStates.Count() - 1; x >= 0; x--)
		{
			if (mReactor->mDrivenStates[x].mDriverID >= mID)
				mReactor->mDrivenStates[x].mDriverID++;
		}
	}
	void Redo() override
	{
		for (int x = mReactor->mDrivenStates.Count() - 1; x >= 0; x--)
		{
			if (mReactor->mDrivenStates[x].mDriverID > mID)
				mReactor->mDrivenStates[x].mDriverID--;
		}
	}
	TSTR Description() override { return _T("Driver IDs Restore"); }
};

class DlgUpdateRestore : public RestoreObj
{
public:
	DlgUpdateRestore() = default;
	~DlgUpdateRestore() = default;

	void Restore(int isUndo) override
	{
		if (GetReactionDlg())
			GetReactionDlg()->InvalidateReactionList();
	}
	void Redo() override { Restore(FALSE); }
	TSTR Description() override { return _T("Update Reaction Dialog"); }
};
