/*****************************************************************************

	FILE: listctrl.h

	DESCRIPTION: Classes and functions pulled out of listctrl.cpp so the layer controller can also use them.

	CREATED BY:	Michael Zyracki

	HISTORY:	December 9th, 2005	Creation

	Copyright (c) 2005, All Rights Reserved.
 *****************************************************************************/

#pragma once

#include "istdplug.h"
#include "control.h"

class ListControl;
class ListDummyEntry : public Control
{
public:
	Control* lc; // chagned by mz in order to have layer controllers also use this class. no reason to be a ListControl
				 // class
	void Init(Control* l);
	Class_ID ClassID() override
	{
		return Class_ID(DUMMY_CONTROL_CLASS_ID, 0);
	}
	SClass_ID SuperClassID() override;
	void GetClassName(MSTR& s, bool localized) const override
	{
		s = _T("");
	}
	RefResult AutoDelete() override
	{
		return REF_SUCCEED;
	}
	RefResult NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message,
			BOOL propagate) override
	{
		return REF_SUCCEED;
	}
	RefTargetHandle Clone(RemapDir& remap) override
	{
		// This cannot exist as a reference because it has no ClassDesc to create it so do not
		// allow Cloning through the ref system; nullptr.
		DbgAssert(false); // Assert to be safe.
		return nullptr;
	}

	void Copy(Control* from) override
	{
	}
	void CommitValue(TimeValue t) override
	{
	}
	void RestoreValue(TimeValue t) override
	{
	}
	void GetValue(TimeValue t, void* val, Interval& valid, GetSetMethod method = CTRL_ABSOLUTE) override;
	void SetValue(TimeValue t, void* val, int commit = 1, GetSetMethod method = CTRL_ABSOLUTE) override
	{
	}

	BOOL CanCopyAnim() override
	{
		return FALSE;
	}
	int IsKeyable() override
	{
		return FALSE;
	}
};
