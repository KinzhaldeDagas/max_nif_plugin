#include "reactionMgr.h"

/**
 * Checks whether a plugin class with a given Class_ID is considered to
 * be public or private.
 * @param aClass_ID The Class_ID of the class being checked.
 * @return 0 if the class is private, 1 if the class is public
 */
static int checkIfClassIsPublic(Class_ID aClass_Id)
{
	// currently, we don't distinguish between controllers, so all
	// are public, or all are private
	return TRUE;
}

//-----------------------------------------------------------------------------

class PositionReactor : public Reactor
{
public:
	PositionReactor(BOOL loading) : Reactor(REACTORPOS, loading) {}
	~PositionReactor() = default;

	int Elems() override { return 3; }
	Class_ID ClassID() override { return REACTORPOS_CLASS_ID; }
	SClass_ID SuperClassID() override { return CTRL_POSITION_CLASS_ID; }
	void GetClassName(MSTR& s, bool localized) const override
	{
		s = localized ? GetString(IDS_AF_REACTORPOS) : _T("Position Reaction");
	}

	// Control methods
	RefTargetHandle Clone(RemapDir& remap) override;
	void GetValue(TimeValue t, void* val, Interval& valid, GetSetMethod method) override;
};

class Point3Reactor : public Reactor
{
public:
	Point3Reactor(BOOL loading) : Reactor(REACTORP3, loading) {}
	~Point3Reactor() = default;

	int Elems() override { return 3; }
	Class_ID ClassID() override { return REACTORP3_CLASS_ID; }
	SClass_ID SuperClassID() override { return CTRL_POINT3_CLASS_ID; }
	void GetClassName(MSTR& s, bool localized) const override
	{
		s = localized ? GetString(IDS_AF_REACTORP3) : _T("Point3 Reaction");
	}

	// Control methods
	RefTargetHandle Clone(RemapDir& remap) override;
	void GetValue(TimeValue t, void* val, Interval& valid, GetSetMethod method) override;
};

class ScaleReactor : public Reactor
{
public:
	ScaleReactor(BOOL loading) : Reactor(REACTORSCALE, loading) {}
	~ScaleReactor() = default;

	int Elems() override { return 3; }
	Class_ID ClassID() override { return REACTORSCALE_CLASS_ID; }
	SClass_ID SuperClassID() override { return CTRL_SCALE_CLASS_ID; }
	void GetClassName(MSTR& s, bool localized) const override
	{
		s = localized ? GetString(IDS_AF_REACTORSCALE) : _T("Scale Reaction");
	}

	// Control methods
	RefTargetHandle Clone(RemapDir& remap) override;
	void GetValue(TimeValue t, void* val, Interval& valid, GetSetMethod method) override;
	void SetValue(TimeValue t, void* val, int commit, GetSetMethod method) override;
};

class RotationReactor : public Reactor
{
public:
	RotationReactor(BOOL loading) : Reactor(REACTORROT, loading) {}
	~RotationReactor() = default;

	int Elems() override { return 3; }
	Class_ID ClassID() override { return REACTORROT_CLASS_ID; }
	SClass_ID SuperClassID() override { return CTRL_ROTATION_CLASS_ID; }
	void GetClassName(MSTR& s, bool localized) const override
	{
		s = localized ? GetString(IDS_AF_REACTORROT) : _T("Rotation Reaction");
	}

	// Control methods
	RefTargetHandle Clone(RemapDir& remap) override;
	void GetValue(TimeValue t, void* val, Interval& valid, GetSetMethod method) override;
	void SetValue(TimeValue t, void* val, int commit, GetSetMethod method) override;
	void CommitValue(TimeValue t) override;
	void RestoreValue(TimeValue t) override;
};

class FloatReactor : public Reactor
{
public:
	FloatReactor(BOOL loading) : Reactor(REACTORFLOAT, loading) {}
	~FloatReactor() = default;

	int Elems() override { return 1; }
	Class_ID ClassID() override { return REACTORFLOAT_CLASS_ID; }
	SClass_ID SuperClassID() override { return CTRL_FLOAT_CLASS_ID; }
	void GetClassName(MSTR& s, bool localized) const override
	{
		s = localized ? GetString(IDS_AF_REACTORFLOAT) : _T("Float Reaction");
	}

	// Control methods
	RefTargetHandle Clone(RemapDir& remap) override;
	void GetValue(TimeValue t, void* val, Interval& valid, GetSetMethod method) override;
	void SetValue(TimeValue t, void* val, int commit, GetSetMethod method) override;
	void CommitValue(TimeValue t) override;
	void RestoreValue(TimeValue t) override;
};

class PostPatchReactionDriver : public PostPatchProc
{
	Reactor* cont = nullptr;
	ReactionDriver* mOldDriver = nullptr;

public:
	PostPatchReactionDriver(Reactor* c, ReactionDriver* d) : cont(c), mOldDriver(d) {}
	virtual int Proc(RemapDir& remap) override
	{
		ReferenceTarget* client = remap.FindMapping(mOldDriver->client);
		if (client == nullptr)
			client = mOldDriver->client;
		ReactionDriver* newDriver = GetReactionManager()->AddReaction(client, mOldDriver->subnum, cont);
		if (newDriver != mOldDriver)
			*newDriver = *mOldDriver;
		cont->ReplaceReference(0, newDriver);
		return TRUE;
	}
};


//*****************************************************
// Function Publishing descriptor for Mixin interface
//*****************************************************

static FPInterfaceDesc reactor_interface(REACTOR_INTERFACE, _T("reactions"), 0, nullptr, FP_MIXIN, IReactor::react_to,
		_T("reactTo"), 0, TYPE_BOOL, 0, 1, _T("object"), 0, TYPE_REFTARG, IReactor::create_reaction, _T("create"), 0,
		TYPE_BOOL, 0, 1, _T("name"), 0, TYPE_STRING, f_keyArgDefault, _T("scriptCreated"), // uses keyword arguments
		IReactor::delete_reaction, _T("delete"), 0, TYPE_BOOL, 0, 1, _T("index"), 0, TYPE_INDEX,
		IReactor::get_reaction_count, _T("getCount"), 0, TYPE_INT, 0, 0, IReactor::select_reaction, _T("select"), 0,
		TYPE_VOID, 0, 1, _T("index"), 0, TYPE_INDEX, IReactor::get_selected_reaction, _T("getSelected"), 0, TYPE_INDEX,
		0, 0,

		IReactor::set_reaction_state, _T("setStateToCurrent"), 0, TYPE_BOOL, 0, 1, _T("index"), 0, TYPE_INDEX,
		IReactor::set_float_reaction_state, _T("setFloatState"), 0, TYPE_BOOL, 0, 2, _T("index"), 0, TYPE_INDEX,
		_T("state"), 0, TYPE_FLOAT, IReactor::set_p3_reaction_state, _T("setVectorState"), 0, TYPE_BOOL, 0, 2,
		_T("index"), 0, TYPE_INDEX, _T("state"), 0, TYPE_POINT3, IReactor::set_quat_reaction_state,
		_T("setRotationState"), 0, TYPE_BOOL, 0, 2, _T("index"), 0, TYPE_INDEX, _T("state"), 0, TYPE_QUAT,

		IReactor::set_reaction_value, _T("setValueToCurrent"), 0, TYPE_BOOL, 0, 1, _T("index"), 0, TYPE_INDEX,
		IReactor::set_float_reaction_value, _T("setValueAsFloat"), 0, TYPE_BOOL, 0, 2, _T("index"), 0, TYPE_INDEX,
		_T("value"), 0, TYPE_FLOAT, IReactor::set_p3_reaction_value, _T("setValueAsVector"), 0, TYPE_BOOL, 0, 2,
		_T("index"), 0, TYPE_INDEX, _T("value"), 0, TYPE_POINT3, IReactor::set_quat_reaction_value,
		_T("setValueAsQuat"), 0, TYPE_BOOL, 0, 2, _T("index"), 0, TYPE_INDEX, _T("value"), 0, TYPE_QUAT,

		IReactor::set_reaction_influence, _T("setInfluence"), 0, TYPE_BOOL, 0, 2, _T("index"), 0, TYPE_INDEX,
		_T("influence"), 0, TYPE_FLOAT, IReactor::set_reaction_strength, _T("setStrength"), 0, TYPE_BOOL, 0, 2,
		_T("index"), 0, TYPE_INDEX, _T("strength"), 0, TYPE_FLOAT, IReactor::set_reaction_falloff, _T("setFalloff"), 0,
		TYPE_BOOL, 0, 2, _T("index"), 0, TYPE_INDEX, _T("influence"), 0, TYPE_FLOAT, IReactor::set_reaction_name,
		_T("setName"), 0, TYPE_VOID, 0, 2, _T("index"), 0, TYPE_INDEX, _T("name"), 0, TYPE_STRING,

		IReactor::get_reaction_name, _T("getName"), 0, TYPE_STRING, 0, 1, _T("index"), 0, TYPE_INDEX,
		IReactor::get_reaction_influence, _T("getInfluence"), 0, TYPE_FLOAT, 0, 1, _T("index"), 0, TYPE_INDEX,
		IReactor::get_reaction_strength, _T("getStrength"), 0, TYPE_FLOAT, 0, 1, _T("index"), 0, TYPE_INDEX,
		IReactor::get_reaction_falloff, _T("getFalloff"), 0, TYPE_FLOAT, 0, 1, _T("index"), 0, TYPE_INDEX,

		IReactor::get_reaction_type, _T("getType"), 0, TYPE_ENUM, IReactor::reaction_type, 0, 0,

		IReactor::get_reaction_state, _T("getState"), 0, TYPE_FPVALUE_BV, 0, 1, _T("index"), 0, TYPE_INDEX,
		IReactor::get_reaction_value, _T("getValue"), 0, TYPE_FPVALUE_BV, 0, 1, _T("index"), 0, TYPE_INDEX,
		// IReactor::get_curve, _T("getCurveControl"), 0, TYPE_REFTARG, 0, 0,

		properties, IReactor::get_editing_state, IReactor::set_editing_state, _T("editStateMode"), 0, TYPE_BOOL,
		IReactor::get_create_mode, IReactor::set_create_mode, _T("createMode"), 0, TYPE_BOOL, IReactor::get_use_curve,
		IReactor::set_use_curve, _T("useCurve"), 0, TYPE_BOOL,

		enums, IReactor::reaction_type, 4, _T("floatReaction"), FLOAT_VAR, _T("positionalReaction"), VECTOR_VAR,
		_T("rotationalReaction"), QUAT_VAR, _T("scaleReaction"), SCALE_VAR, p_end);

///*****************************************
/// Get Descriptor method for Mixin Interface
///*****************************************
FPInterfaceDesc* IReactor::GetDesc() { return &reactor_interface; }

//*******************************************************************
// Class Descriptors
//*******************************************************************

static bool posReactorInterfaceAdded = false;
class PositionReactorClassDesc : public ClassDesc2
{
public:
	int IsPublic() override { return checkIfClassIsPublic(ClassID()); }
	void* Create(BOOL loading) override
	{
		if (!posReactorInterfaceAdded)
		{
			AddInterface(&reactor_interface);
			posReactorInterfaceAdded = true;
		}
		return new PositionReactor(loading);
	}
	const TCHAR* ClassName() override { return GetString(IDS_AF_REACTORPOS); }
	const TCHAR* NonLocalizedClassName() override { return _T("Position Reaction"); }
	SClass_ID SuperClassID() override { return CTRL_POSITION_CLASS_ID; }
	Class_ID ClassID() override { return REACTORPOS_CLASS_ID; }
	const TCHAR* Category() override { return _T(""); }

	const TCHAR* InternalName() override { return _T("PositionReactor"); } // returns fixed parsable name (scripter-visible name)
	HINSTANCE HInstance() override { return hInstance; } // returns owning module handle
};

//-----------------------------------------------------------------------------

static bool point3ReactorInterfaceAdded = false;
class Point3ReactorClassDesc : public ClassDesc2
{
public:
	int IsPublic() override { return checkIfClassIsPublic(ClassID()); }
	void* Create(BOOL loading) override
	{
		if (!point3ReactorInterfaceAdded)
		{
			AddInterface(&reactor_interface);
			point3ReactorInterfaceAdded = true;
		}
		return new Point3Reactor(loading);
	}

	const TCHAR* ClassName() override { return GetString(IDS_AF_REACTORP3); }
	const TCHAR* NonLocalizedClassName() override { return _T("Point3 Reaction"); }
	SClass_ID SuperClassID() override { return CTRL_POINT3_CLASS_ID; }
	Class_ID ClassID() override { return REACTORP3_CLASS_ID; }
	const TCHAR* Category() override { return _T(""); }

	const TCHAR* InternalName() override { return _T("Point3Reactor"); } // returns fixed parsable name (scripter-visible name)
	HINSTANCE HInstance() override { return hInstance; } // returns owning module handle
};

//-----------------------------------------------------------------------------

static bool scaleReactorInterfaceAdded = false;
class ScaleReactorClassDesc : public ClassDesc2
{
public:
	int IsPublic() override { return checkIfClassIsPublic(ClassID()); }
	void* Create(BOOL loading) override
	{
		if (!scaleReactorInterfaceAdded)
		{
			AddInterface(&reactor_interface);
			scaleReactorInterfaceAdded = true;
		}
		return new ScaleReactor(loading);
	}
	const TCHAR* ClassName() override { return GetString(IDS_AF_REACTORSCALE); }
	const TCHAR* NonLocalizedClassName() override { return _T("Scale Reaction"); }
	SClass_ID SuperClassID() override { return CTRL_SCALE_CLASS_ID; }
	Class_ID ClassID() override { return REACTORSCALE_CLASS_ID; }
	const TCHAR* Category() override { return _T(""); }

	const TCHAR* InternalName() override { return _T("ScaleReactor"); } // returns fixed parsable name (scripter-visible name)
	HINSTANCE HInstance() override { return hInstance; } // returns owning module handle
};

//-------------------------------------------------------------------

static bool rotationReactorInterfaceAdded = false;
class RotationReactorClassDesc : public ClassDesc2
{
public:
	int IsPublic() override { return checkIfClassIsPublic(ClassID()); }
	void* Create(BOOL loading) override
	{
		if (!rotationReactorInterfaceAdded)
		{
			AddInterface(&reactor_interface);
			rotationReactorInterfaceAdded = true;
		}
		return new RotationReactor(loading);
	}
	const TCHAR* ClassName() override { return GetString(IDS_AF_REACTORROT); }
	const TCHAR* NonLocalizedClassName() override { return _T("Rotation Reaction"); }
	SClass_ID SuperClassID() override { return CTRL_ROTATION_CLASS_ID; }
	Class_ID ClassID() override { return REACTORROT_CLASS_ID; }
	const TCHAR* Category() override { return _T(""); }

	const TCHAR* InternalName() override { return _T("RotationReactor"); } // returns fixed parsable name (scripter-visible name)
	HINSTANCE HInstance() { return hInstance; } // returns owning module handle
};

//-----------------------------------------------------------------------------

static bool floatReactorInterfaceAdded = false;
class FloatReactorClassDesc : public ClassDesc2
{
public:
	int IsPublic() override { return checkIfClassIsPublic(ClassID()); }
	void* Create(BOOL loading) override
	{
		if (!floatReactorInterfaceAdded)
		{
			AddInterface(&reactor_interface);
			floatReactorInterfaceAdded = true;
		}
		return new FloatReactor(loading);
	}

	const TCHAR* ClassName() override { return GetString(IDS_AF_REACTORFLOAT); }
	const TCHAR* NonLocalizedClassName() override { return _T("Float Reaction"); }
	SClass_ID SuperClassID() override { return CTRL_FLOAT_CLASS_ID; }
	Class_ID ClassID() override { return REACTORFLOAT_CLASS_ID; }
	const TCHAR* Category() override { return _T(""); }

	const TCHAR* InternalName() override { return _T("FloatReactor"); } // returns fixed parsable name (scripter-visible name)
	HINSTANCE HInstance() override { return hInstance; } // returns owning module handle
};

///*****************************************************************
/// Static descriptors
///*****************************************************************

static PositionReactorClassDesc positionReactorCD;
static Point3ReactorClassDesc point3ReactorCD;
static ScaleReactorClassDesc scaleReactorCD;
static RotationReactorClassDesc rotationReactorCD;
static FloatReactorClassDesc floatReactorCD;

ClassDesc* GetPositionReactorDesc() { return &positionReactorCD; }
ClassDesc* GetPoint3ReactorDesc() { return &point3ReactorCD; }
ClassDesc* GetScaleReactorDesc() { return &scaleReactorCD; }
ClassDesc* GetRotationReactorDesc() { return &rotationReactorCD; }
ClassDesc* GetFloatReactorDesc() { return &floatReactorCD; }


///*****************************************************************
/// Implementation code
///*****************************************************************
RefTargetHandle FloatReactor::Clone(RemapDir& remap)
{
	// make a new reactor controller and give it our param values.
	auto* cont = new FloatReactor(FALSE);
	if (ReactionDriver* thisDriver = GetReactionDriver())
		remap.AddPostPatchProc(new PostPatchReactionDriver(cont, thisDriver), true);
	else
		GetReactionManager()->AddReaction(nullptr, 0, cont);

	*cont = *this;
	cont->mLocked = mLocked;
	if (!iCCtrl)
		cont->BuildCurves();
	else
		cont->ReplaceReference(1, remap.CloneRef(iCCtrl));
	BaseClone(this, cont, remap);
	return cont;
}

void FloatReactor::GetValue(TimeValue t, void* val, Interval& valid, GetSetMethod method)
{
	float f[3];
	Update(t);
	valid &= ivalid;
	if (!editing && !createMode && count)
	{
		if (getReactionType() == FLOAT_VAR && isCurveControlled)
		{
			if (mDrivenStates.Count() > 1)
			{
				GetCurveValue(t, f, valid);
				curfval = f[0];
			}
		}
		else
		{
			ComputeMultiplier(t);
			// sum up all the weighted states
			float ray = 0.0f;
			for (int i = 0; i < count; i++)
				ray += ((mDrivenStates[i].fstate) * mDrivenStates[i].multiplier);
			if (count)
				curfval = ray;
		}
	}

	if (method == CTRL_RELATIVE)
	{
		*((float*)val) += curfval;
	}
	else
	{
		*((float*)val) = curfval;
	}
}

void FloatReactor::SetValue(TimeValue t, void* val, int commit, GetSetMethod method)
{
	if (!GetLocked())
	{
		if ((editing || createMode) && count)
		{
			if (!TestAFlag(A_SET))
			{
				HoldTrack();
				tmpStore.PutBytes(sizeof(float), &curfval, this);
				SetAFlag(A_SET);
			}

			if (method == CTRL_RELATIVE)
				curfval += *((float*)val);
			else
				curfval = *((float*)val);

			ivalid.SetInstant(t);
			if (commit)
				CommitValue(t);
			if (!commit)
				NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		}
	}
}


void FloatReactor::CommitValue(TimeValue t)
{
	if (TestAFlag(A_SET))
	{
		if (ivalid.InInterval(t))
		{
			if (editing && count)
				mDrivenStates[selected].fstate = curfval;
			if (getReactionType() == FLOAT_VAR)
				UpdateCurves();

			tmpStore.Clear(this);
			ivalid.SetEmpty();
			NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		}
		ClearAFlag(A_SET);
	}
}

void FloatReactor::RestoreValue(TimeValue t)
{
	if (TestAFlag(A_SET))
	{
		tmpStore.GetBytes(sizeof(float), &curfval, this);
		if (editing && count)
			mDrivenStates[selected].fstate = curfval;
		tmpStore.Clear(this);
		ivalid.SetInstant(t);
		ClearAFlag(A_SET);
	}
}


//--------------------------------------------------------------------------

RefTargetHandle PositionReactor::Clone(RemapDir& remap)
{
	// make a new reactor controller and give it our param values.
	auto* cont = new PositionReactor(FALSE);
	if (ReactionDriver* thisDriver = GetReactionDriver())
		remap.AddPostPatchProc(new PostPatchReactionDriver(cont, thisDriver), true);
	else
		GetReactionManager()->AddReaction(nullptr, 0, cont);

	*cont = *this;
	cont->mLocked = mLocked;

	if (!iCCtrl)
		cont->BuildCurves();
	else
		cont->ReplaceReference(1, remap.CloneRef(iCCtrl));
	BaseClone(this, cont, remap);
	return cont;
}

void PositionReactor::GetValue(TimeValue t, void* val, Interval& valid, GetSetMethod method)
{
	Point3 ray = Point3(0, 0, 0);
	float f[3];
	Update(t);
	valid &= ivalid;
	if (!editing && !createMode && count)
	{
		if (getReactionType() == FLOAT_VAR && isCurveControlled)
		{
			GetCurveValue(t, f, valid);
			for (int x = 0; x < 3; x++)
			{
				curpval[x] = f[x];
			}
		}
		else
		{
			ComputeMultiplier(t);
			// sum up all the weighted states
			for (int i = 0; i < count; i++)
				ray += ((mDrivenStates[i].pstate) * mDrivenStates[i].multiplier);
			curpval = ray;
		}
	}

	if (method == CTRL_RELATIVE)
	{
		Matrix3* mat = (Matrix3*)val;
		mat->PreTranslate(curpval);
	}
	else
	{
		*((Point3*)val) = curpval;
	}
}
//---------------------------------------------------------

RefTargetHandle Point3Reactor::Clone(RemapDir& remap)
{
	// make a new reactor controller and give it our param values.
	auto* cont = new Point3Reactor(FALSE);
	if (ReactionDriver* thisDriver = GetReactionDriver())
		remap.AddPostPatchProc(new PostPatchReactionDriver(cont, thisDriver), true);
	else
		GetReactionManager()->AddReaction(nullptr, 0, cont);

	*cont = *this;
	cont->mLocked = mLocked;
	if (!iCCtrl)
		cont->BuildCurves();
	else
		cont->ReplaceReference(1, remap.CloneRef(iCCtrl));
	BaseClone(this, cont, remap);
	return cont;
}

void Point3Reactor::GetValue(TimeValue t, void* val, Interval& valid, GetSetMethod method)
{
	float f[3];
	Update(t);
	valid &= ivalid;
	if (!editing && !createMode && count)
	{
		if (getReactionType() == FLOAT_VAR && isCurveControlled)
		{
			GetCurveValue(t, f, valid);
			for (int x = 0; x < 3; x++)
			{
				curpval[x] = f[x];
			}
		}
		else
		{
			ComputeMultiplier(t);
			// sum up all the weighted states
			Point3 ray = Point3(0, 0, 0);
			for (int i = 0; i < count; i++)
				ray += ((mDrivenStates[i].pstate) * mDrivenStates[i].multiplier);
			curpval = ray;
		}
	}

	if (method == CTRL_RELATIVE)
	{
		*((Point3*)val) += curpval;
	}
	else
	{
		*((Point3*)val) = curpval;
	}
}

//--------------------------------------------------------------------------

RefTargetHandle ScaleReactor::Clone(RemapDir& remap)
{
	// make a new reactor controller and give it our param values.
	auto* cont = new ScaleReactor(FALSE);
	if (ReactionDriver* thisDriver = GetReactionDriver())
		remap.AddPostPatchProc(new PostPatchReactionDriver(cont, thisDriver), true);
	else
		GetReactionManager()->AddReaction(nullptr, 0, cont);

	*cont = *this;
	cont->mLocked = mLocked;
	if (!iCCtrl)
		cont->BuildCurves();
	else
		cont->ReplaceReference(1, remap.CloneRef(iCCtrl));
	BaseClone(this, cont, remap);
	return cont;
}

void ScaleReactor::GetValue(TimeValue t, void* val, Interval& valid, GetSetMethod method)
{
	float f[3];
	Update(t);
	valid &= ivalid;
	if (!editing && !createMode && count)
	{
		if (getReactionType() == FLOAT_VAR && isCurveControlled)
		{
			GetCurveValue(t, f, valid);
			for (int x = 0; x < 3; x++)
			{
				curpval[x] = f[x];
			}
		}
		else
		{
			ComputeMultiplier(t);
			// sum up all the weighted states
			Point3 ray = Point3(0, 0, 0);
			for (int i = 0; i < count; i++)
				ray += ((mDrivenStates[i].pstate) * mDrivenStates[i].multiplier);
			curpval = ray;
		}
	}

	if (method == CTRL_RELATIVE)
	{
		Matrix3* mat = (Matrix3*)val;
		ApplyScaling(*mat, curpval);
	}
	else
	{
		*((Point3*)val) = curpval;
	}
}


void ScaleReactor::SetValue(TimeValue t, void* val, int commit, GetSetMethod method)
{
	if (!GetLocked())
	{
		if ((editing || createMode) && count)
		{
			if (!TestAFlag(A_SET))
			{
				HoldTrack();
				tmpStore.PutBytes(sizeof(Point3), &curpval, this);
				SetAFlag(A_SET);
			}
			if (method == CTRL_RELATIVE)
				curpval *= *((Point3*)val);
			else
				curpval = *((Point3*)val);

			ivalid.SetInstant(t);
			if (commit)
				CommitValue(t);
			if (!commit)
				NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		}
	}
}


//------------------------------------------------------------------

RefTargetHandle RotationReactor::Clone(RemapDir& remap)
{
	// make a new reactor controller and give it our param values.
	auto* cont = new RotationReactor(FALSE);
	if (ReactionDriver* thisDriver = GetReactionDriver())
		remap.AddPostPatchProc(new PostPatchReactionDriver(cont, thisDriver), true);
	else
		GetReactionManager()->AddReaction(nullptr, 0, cont);

	*cont = *this;
	cont->mLocked = mLocked;
	if (!iCCtrl)
		cont->BuildCurves();
	else
		cont->ReplaceReference(1, remap.CloneRef(iCCtrl));
	BaseClone(this, cont, remap);
	return cont;
}

void RotationReactor::GetValue(TimeValue t, void* val, Interval& valid, GetSetMethod method)
{
	float eulr[3];
	Quat ray;
	Update(t);
	valid &= ivalid;
	if (!editing && !createMode && count)
	{
		if (getReactionType() == FLOAT_VAR && isCurveControlled)
		{
			GetCurveValue(t, eulr, valid);
			for (int p = 0; p < 3; p++)
			{
				eulr[p] = DegToRad(eulr[p]);
			}
			EulerToQuat(eulr, curqval);
		}
		else
		{
			ComputeMultiplier(t);
			// sum up all the weighted states
			curqval.Identity();

			for (int i = 0; i < count; i++)
			{
				ray = mDrivenStates[i].qstate;
				QuatToEuler(ray, eulr);
				eulr[0] *= mDrivenStates[i].multiplier;
				eulr[1] *= mDrivenStates[i].multiplier;
				eulr[2] *= mDrivenStates[i].multiplier;
				EulerToQuat(eulr, ray);

				curqval += ray;
			}
		}
	}

	if (method == CTRL_RELATIVE)
	{
		Matrix3* mat = (Matrix3*)val;
		PreRotateMatrix(*mat, curqval);
	}
	else
	{
		*((Quat*)val) = curqval;
	}
}

void RotationReactor::SetValue(TimeValue t, void* val, int commit, GetSetMethod method)
{
	if (!GetLocked())
	{
		if ((editing || createMode) && count)
		{
			if (!TestAFlag(A_SET))
			{
				HoldTrack();
				tmpStore.PutBytes(sizeof(Quat), &curqval, this);
				SetAFlag(A_SET);
			}

			if (method == CTRL_RELATIVE)
				curqval *= Quat(*((AngAxis*)val));
			else
				curqval = Quat(*((AngAxis*)val));

			ivalid.SetInstant(t);
			if (commit)
				CommitValue(t);
			if (!commit)
				NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		}
	}
}


void RotationReactor::CommitValue(TimeValue t)
{
	if (TestAFlag(A_SET))
	{
		if (ivalid.InInterval(t))
		{

			if (editing && count)
				mDrivenStates[selected].qstate = curqval;
			if (getReactionType() == FLOAT_VAR)
				UpdateCurves();

			tmpStore.Clear(this);
			ivalid.SetEmpty();
			NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		}
		ClearAFlag(A_SET);
	}
}

void RotationReactor::RestoreValue(TimeValue t)
{
	if (TestAFlag(A_SET))
	{
		if (count)
		{
			tmpStore.GetBytes(sizeof(Quat), &curqval, this);
			if (editing && count)
				mDrivenStates[selected].qstate = curqval;
			tmpStore.Clear(this);
			ivalid.SetInstant(t);
		}
		ClearAFlag(A_SET);
	}
}
