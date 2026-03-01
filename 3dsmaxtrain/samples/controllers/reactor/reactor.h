/**********************************************************************
 *<
	FILE: reactor.h

	DESCRIPTION: Header file for Reactor Controller

	CREATED BY: Adam Felt

	HISTORY:

 *>	Copyright (c) 1998-1999 Adam Felt, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "Max.h"
#include "resource.h"
#include "ActionTable.h"
#include <maxscript/maxscript.h>

#include "iparamm.h"
#include "ReactAPI.h"
#include "macrorec.h"
#include "notify.h"
#include <ILockedTracks.h>

extern ClassDesc* GetFloatReactorDesc();
extern ClassDesc* GetPositionReactorDesc();
extern ClassDesc* GetPoint3ReactorDesc();
extern ClassDesc* GetRotationReactorDesc();
extern ClassDesc* GetScaleReactorDesc();
extern ClassDesc* GetReactionDriverDesc();

extern HINSTANCE hInstance;

//--------------------------------------------------------------------------

// Keyboard Shortcuts stuff
const ActionTableId kReactionMgrActions = 0x6bd55e20;
const ActionContextId kReactionMgrContext = 0x6bd55e20;

TCHAR* GetString(int id);
ActionTable* GetActions();
void GetAbsoluteControlValue(INode* node, int subNum, TimeValue t, void* pt, Interval& iv);
inline void GetAbsoluteControlValue(INode* node, int subNum, TimeValue t, void* pt, Interval&& valid = FOREVER)
{
	GetAbsoluteControlValue(node, subNum, t, pt, valid);
}

//-----------------------------------------------------------------------

#define REACTOR_DONT_SHOW_CREATE_MSG	(1 << 1) // don't display the Create reaction warning message
#define REACTOR_DONT_CREATE_SIMILAR		(1 << 2) // don't create a reaction if it has the same value as an existing reaction
#define REACTOR_BLOCK_CURVE_UPDATE		(1 << 3) // don't update the Curves
#define REACTOR_BLOCK_REACTION_DELETE	(1 << 4) // only delete one reaction if multiple keys are selected

#define REACTORDLG_LIST_CHANGED_SIZE		(1 << 5) // refresh the listbox and everything depending on selection
#define REACTORDLG_LIST_SELECTION_CHANGED	(1 << 6) // update the fields depending on the list selection
#define REACTORDLG_REACTTO_NAME_CHANGED		(1 << 7)
//#define REACTORDLG_REACTION_NAME_CHANGED	(1<<8)

#define REFMSG_REACTOR_SELECTION_CHANGED	0x00022000
#define REFMSG_REACTION_COUNT_CHANGED		0x00023000
#define REFMSG_REACTTO_OBJ_NAME_CHANGED		0x00024000
#define REFMSG_USE_CURVE_CHANGED			0x00025000
#define REFMSG_REACTION_NAME_CHANGED		0x00026000

#define REACTION_DRIVER_CLASSID Class_ID(0x29651db6, 0x2c8515c2)

template <class T>
class ReactorActionCB;
class Reactor;

// Pick Mode Stuff
//******************************************

class PickObjectCallback
{
public:
	virtual TSTR GetName() const = 0;
	virtual bool LoopTest(ReferenceTarget* anim) = 0;
	virtual void NodePicked(ReferenceTarget* anim) = 0;
	virtual bool IncludeRoots() = 0;
	virtual bool MultipleChoice() = 0;
	virtual void PostPick() = 0;
};

class ReactionPickMode : public PickModeCallback, public PickNodeCallback, public PickObjectCallback
{
	Tab<ReferenceTarget*> nodes;

public:
	ReactionPickMode() { nodes.SetCount(0); }

	BOOL HitTest(IObjParam* ip, HWND hWnd, ViewExp* vpt, IPoint2 m, int flags) override;
	BOOL Pick(IObjParam* ip, ViewExp* vpt) override;
	BOOL PickAnimatable(Animatable* anim) override;
	BOOL RightClick(IObjParam* ip, ViewExp* vpt) override { return TRUE; }
	BOOL Filter(INode* node) override;
	PickNodeCallback* GetFilter() override { return this; }
	BOOL FilterAnimatable(Animatable* anim);
	BOOL AllowMultiSelect() override { return MultipleChoice(); }
	bool MultipleChoice() override { return false; }
	void ExitMode(IObjParam* ip) override;
	void PostPick() override {};
};

//-----------------------------------------------

// Storage class for the animatable right-click style menu
//*******************************************************
struct SubAnimPair
{
	ReferenceTarget* parent = nullptr;
	int id = -1;
	SubAnimPair() = default;
};

class AnimEntry
{
public:
	const TCHAR* pname = nullptr; // param name
	int level; // used during pop-up building, 0->same, 1->new level, -1->back level
	// ReferenceTarget* root;		// display root
	Tab<SubAnimPair*> pair; // anim/subanim num pair

	AnimEntry() { pair.SetCount(0); }
	AnimEntry(const TCHAR* n, int l, Tab<SubAnimPair*> p) : pname(n ? save_string(n) : n), level(l)
	{
		pair.SetCount(0);
		for (int i = 0; i < p.Count(); i++)
			pair.Append(1, &p[i]);
	}

	~AnimEntry()
	{
		for (int i = 0; i < pair.Count(); i++)
			delete pair[i];
		free((void*)pname);
	}

	AnimEntry& operator=(const AnimEntry& from)
	{
		Clear();
		pname = from.pname ? save_string(from.pname) : nullptr;
		level = from.level;
		pair.SetCount(0);
		for (int i = 0; i < from.pair.Count(); i++)
		{
			SubAnimPair* p = new SubAnimPair();
			p->id = from.pair[i]->id;
			p->parent = from.pair[i]->parent;
			pair.Append(1, &p);
		}
		return *this;
	}

	void Clear()
	{
		if (pname)
			free((void*)pname);
		pname = nullptr;
	}

	Animatable* Anim(int i)
	{
		return pair[i]->parent ? pair[i]->parent->SubAnim(pair[i]->id) : nullptr;
	}
};

/* -----  viewport right-click menu ----------------- */

class AnimPopupMenu
{
private:
	//	INode*		node;				// source node & param...
	//	AnimEntry	entry;
	Tab<HMENU> menus;

	void add_hmenu_items(Tab<AnimEntry*>& wparams, HMENU menu, int& i);
	bool wireable(Animatable* parent, int subnum);
	void build_params(PickObjectCallback* cb, Tab<AnimEntry*>& wparams, Tab<ReferenceTarget*> root);
	bool add_subanim_params(PickObjectCallback* cb, Tab<AnimEntry*>& wparams, const TCHAR* name,
			Tab<ReferenceTarget*>& parents, int level, Tab<ReferenceTarget*>& roots);
	AnimEntry* find_param(Tab<AnimEntry>& params, Animatable* anim, Animatable* parent, int subNum);
	bool Filter(PickObjectCallback* cb, Animatable* anim, Animatable* child, bool includeChildren = false);

public:
	AnimPopupMenu() = default;
	void DoPopupMenu(PickObjectCallback* cb, Tab<ReferenceTarget*> node);
	Control* AssignDefaultController(Animatable* parent, int subnum);
};

static AnimPopupMenu theAnimPopupMenu;

class DriverState
{
public:
	TSTR name;
	int type;

	// The type used here is the same as the client track
	Point3 pvalue; // current value if it is a point3
	float fvalue; // current value if it's a float
	Quat qvalue; // current value if it's a quat

	explicit DriverState(const DriverState* state) { *this = *state; }
	DriverState(TSTR n, int t, void* val) : name(n), type(t)
	{
		if (val != nullptr)
		{
			switch (type)
			{
			case FLOAT_VAR:
				fvalue = *(float*)val;
				break;
			case SCALE_VAR:
			case VECTOR_VAR:
				pvalue = *(Point3*)val;
				break;
			case QUAT_VAR:
				qvalue = *(Quat*)val;
				break;
			}
		}
	}

	DriverState& operator=(const DriverState& from) = default;

	DriverState* Clone() { return new DriverState(this); }
};

// Storage class for the track being referenced
//*********************************************
class ReactionDriver : public ReferenceTarget
{
public:
	ReferenceTarget* client = nullptr;
	int subnum;
	Tab<DriverState*> states;
	int type = 0;
	NameMaker* nmaker = nullptr;

	ReactionDriver() { states.Init(); }
	ReactionDriver(ReferenceTarget* c, int id);
	~ReactionDriver()
	{
		for (int i = states.Count() - 1; i >= 0; i--)
			delete states[i];
		if (nmaker)
			delete nmaker;
		DbgAssert(client == nullptr);
	}

	ReactionDriver& operator=(const ReactionDriver& from)
	{
		subnum = from.subnum;
		type = from.type;
		for (int i = states.Count() - 1; i >= 0; i--)
			delete states[i];
		states.SetCount(0);

		if (nmaker)
			delete nmaker;
		nmaker = GetCOREInterface()->NewNameMaker(FALSE);
		for (int i = 0; i < from.states.Count(); i++)
		{
			nmaker->AddName(from.states[i]->name);
			auto* state = new DriverState(from.states[i]);
			states.Append(1, &state);
		}
		return *this;
	}

	RefTargetHandle Clone(RemapDir& remap) override;

	void DeleteThis() override { delete this; }
	Class_ID ClassID() override { return REACTION_DRIVER_CLASSID; }
	SClass_ID SuperClassID() override { return REF_MAKER_CLASS_ID; }
	void GetClassName(MSTR& s, bool localized) const override
	{
		s = localized ? GetString(IDS_AF_REACTION_DRIVER) : _T("Reaction Driver");
	}

	IOResult Save(ISave* isave) override;
	IOResult Load(ILoad* iload) override;

	int NumRefs() override { return 1; }
	ReferenceTarget* GetReference(int i) override { return client; }

private:
	virtual void SetReference(int i, RefTargetHandle rtarg) override { client = rtarg; }

public:
	RefResult NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate) override;
	BOOL IsRealDependency(ReferenceTarget* rtarg) override { return FALSE; }

	ReferenceTarget* GetDriver()
	{
		return (subnum < 0) ? client : static_cast<ReferenceTarget*>(client->SubAnim(subnum));
	}

	ReferenceTarget* Owner() { return client; }
	int SubIndex() const { return subnum; }

	DriverState* GetState(int i) { return (i >= states.Count()) ? nullptr : states[i]; }
	DriverState* AddState(const TCHAR* name, int& id, bool setDefaults, TimeValue t);
	void DeleteState(int i);
	int GetType() const { return type; }
	void GetValue(TimeValue t, void* val);
	void SetState(int i, float val);
	void SetState(int i, Point3 val);
	void SetState(int i, Quat val);

	void CopyFrom(Reactor* r);
};

// ReactionSet variables
//*****************************************
class DrivenState
{
public:
	int mDriverID;
	float influence;
	float multiplier;
	float strength;
	float falloff;

	// The type used here is the same as the controller type
	float fstate; // reaction state if it's a float
	Quat qstate; // reaction state if it's a quat
	Point3 pstate; // reaction state if it's a point3

	DrivenState& operator=(const DrivenState& from) = default;
};

class Reactor : public IReactor , public ResourceMakerCallback , public ILockedTrackImp
{
	friend class TransferReactorReferencePLCB;
	ReactionDriver* mDriver = nullptr;
	ReferenceTarget* mClient = nullptr; // required for loading pre-R7 files

	int mNumRefs; // # of references made by this reactor - depends on max file version
	static constexpr int kREFID_DRIVER = 0;
	static constexpr int kREFID_CURVE_CTL = 1;
	static constexpr int kREFID_CLIENT_PRE_R7 = 0; // the old refid to client, used by pre-R7 version of reactor
	static constexpr int kREFID_CLIENT_REMAP_PRE_R7 = 2; // used to remap client refs loaded from pre-R7 max files
	static constexpr int kNUM_REFS = 2;
	static constexpr int kNUM_REFS_PRE_R7 = 3;
	void SetNumRefs(int newNumRefs);

public:
	int type, selected, count;
	int isCurveControlled;
	BOOL editing; // editing the reaction state
	BOOL createMode;
	Tab<DrivenState> mDrivenStates;
	Interval ivalid;
	Interval range;
	ICurveCtl* iCCtrl = nullptr;
	UINT flags;
	bool curvesValid;

	Point3 curpval;
	Point3 upVector;
	float curfval;
	Quat curqval;
	BOOL blockGetNodeName; // RB 3/23/99: See imp of getNodeName()

	ReactorActionCB<Reactor>* reactorActionCB = nullptr; // Actions handler

	ReactionPickMode* pickReactToMode = nullptr;

	virtual int Elems() { return 0; }

	Reactor();
	Reactor(int t, BOOL loading);
	Reactor& operator=(const Reactor& from);
	~Reactor();
	void Init(BOOL loading = FALSE);

	BOOL assignReactObj(ReferenceTarget* client, int subnum);
	BOOL reactTo(ReferenceTarget* anim, TimeValue t = GetCOREInterface()->GetTime(), bool createReaction = true);
	ReactionDriver* GetReactionDriver() { return mDriver; }
	void updReactionCt(int val, BOOL sort = true);
	DrivenState* CreateReactionAndReturn(BOOL setDefaults = TRUE, const TCHAR* buf = nullptr, TimeValue t = GetCOREInterface()->GetTime(), int driverID = -1);
	BOOL CreateReaction(const TCHAR* buf = nullptr, TimeValue t = GetCOREInterface()->GetTime());
	BOOL DeleteReaction(int i = -1);
	int getReactionCount() override { return mDrivenStates.Count(); }
	void deleteAllVars();
	int getSelected() override { return selected; }
	void setSelected(int i) override;
	int getType() override { return type; }
	int getReactionType() override { return (mDriver == nullptr) ? 0 : mDriver->GetType(); }
	const TCHAR* getReactionName(int i) override;
	void setReactionName(int i, TSTR name) override;
	void* getReactionValue(int i) override;
	BOOL setReactionValue(int i = -1, TimeValue t = NULL) override;
	BOOL setReactionValue(int i, float val) override;
	BOOL setReactionValue(int i, Point3 val) override;
	BOOL setReactionValue(int i, Quat val) override;
	float getCurFloatValue(TimeValue t);
	Point3 getCurPoint3Value(TimeValue t);
	ScaleValue getCurScaleValue(TimeValue t);
	Quat getCurQuatValue(TimeValue t);
	BOOL setInfluence(int num, float inf) override;
	float getInfluence(int num) override;
	void setMinInfluence(int x = -1);
	void setMaxInfluence(int x = -1);
	BOOL setStrength(int num, float inf) override;
	float getStrength(int num) override;
	BOOL setFalloff(int num, float inf) override;
	float getFalloff(int num) override;
	void setEditReactionMode(BOOL edit) override;
	BOOL getEditReactionMode() override { return editing; }
	void setCreateReactionMode(BOOL edit) override;
	BOOL getCreateReactionMode() override;

	void* getState(int num) override;
	BOOL setState(int num = -1, TimeValue t = NULL) override;
	BOOL setState(int num, float val) override;
	BOOL setState(int num, Point3 val) override;
	BOOL setState(int num, Quat val) override;
	void getNodeName(ReferenceTarget* client, TSTR& name);
	Point3 getUpVector() override { return upVector; }
	void setUpVector(Point3 up) override { upVector = up; }
	BOOL useCurve() override { return isCurveControlled; }
	void useCurve(BOOL use) override
	{
		isCurveControlled = use;
		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	}
	ICurveCtl* getCurveControl() override { return iCCtrl; }
	void SortReactions();

	void Update(TimeValue t);
	void ComputeMultiplier(TimeValue t);
	BOOL ChangeParents(TimeValue t, const Matrix3& oldP, const Matrix3& newP, const Matrix3& tm) override;
	void ShowCurveControl();
	void BuildCurves();
	void RebuildFloatCurves();
	void UpdateCurves(bool doZoom = true);
	void GetCurveValue(TimeValue t, float* val, Interval& valid);

	FPValue fpGetReactionValue(int index) override;
	FPValue fpGetState(int index) override;

	DriverState* getDriverState(int i);

	// Animatable methods
	void DeleteThis() override { delete this; }
	int IsKeyable() override { return (!createMode && !editing) ? 0 : 1; }
	BOOL IsAnimated() override { return (mDrivenStates.Count() > 1 && !createMode && !editing); }
	Interval GetTimeRange(DWORD flags) override { return range; }

	void EditTimeRange(Interval range, DWORD flags) override;
	void MapKeys(TimeMap* map, DWORD flags) override;

	void HoldTrack();
	void HoldAll();
	void HoldParams();
	void HoldRange();

	int NumSubs() override;
	BOOL AssignController(Animatable* control, int subAnim) override { return false; }
	Animatable* SubAnim(int i) override { return nullptr; }
	TSTR SubAnimName(int i, bool localized) override { return _T(""); }
	virtual void GetClassName(MSTR& s, bool localized) const override
	{
		s = _M("Reactor");
	}

	// Animatable's Schematic View methods
	SvGraphNodeReference SvTraverseAnimGraph(IGraphObjectManager* gom, Animatable* owner, int id, DWORD flags) override;
	TSTR SvGetRelTip(IGraphObjectManager* gom, IGraphNode* gNodeTarger, int id, IGraphNode* gNodeMaker) override;
	bool SvHandleRelDoubleClick(IGraphObjectManager* gom, IGraphNode* gNodeTarget, int id, IGraphNode* gNodeMaker) override;

	void EditTrackParams(TimeValue t, ParamDimensionBase* dim, const TCHAR* pname, HWND hParent, IObjParam* ip, DWORD flags) override;
	int TrackParamsType() override { return TRACKPARAMS_WHOLE; } // always show UI, it contains multiple reactions

	// Reference methods
	int NumRefs() override;
	RefTargetHandle GetReference(int i) override;

private:
	void SetReference(int i, RefTargetHandle rtarg) override;

public:
	// The reaction driver is a weak reference
	BOOL IsRealDependency(ReferenceTarget* rtarg) override { return TRUE; }
	RefResult NotifyRefChanged(const Interval&, RefTargetHandle, PartID&, RefMessage, BOOL) override;
	IOResult Save(ISave* isave) override;
	IOResult Load(ILoad* iload) override;
	int RemapRefOnLoad(int iref) override;

	// Control methods
	void Copy(Control* from) override;
	BOOL IsLeaf() override { return FALSE; }
	BOOL IsReplaceable() override { return !GetLocked(); }

	// These three default implementation are shared by Position, Point3 and Scale controllers
	void SetValue(TimeValue t, void* val, int commit, GetSetMethod method) override;
	void CommitValue(TimeValue t) override;
	void RestoreValue(TimeValue t) override;

	void GetValue(TimeValue t, void* val, Interval& valid, GetSetMethod method) override {} // overwritten by everyone

	// From ResourceMakerCallback
	void ResetCallback(int curvenum, ICurveCtl* pCCtl) override;
	void NewCurveCreatedCallback(int curvenum, ICurveCtl* pCCtl) override;
	void* GetInterface(ULONG id) override
	{
		if (id == I_RESMAKER_INTERFACE)
			return (void*)(ResourceMakerCallback*)this;
		else if (id == I_LOCKED)
			return (ILockedTrackImp*)this;
		else
			return (void*)Control::GetInterface(id);
	}

	// Function Publishing method (Mixin Interface)
	//******************************
	BaseInterface* GetInterface(Interface_ID id) override
	{
		return (id == REACTOR_INTERFACE) ? static_cast<Reactor*>(this) : FPMixinInterface::GetInterface(id);
	}
	//******************************
};

// Enumeration Proc
// Used to determine the proper reference object....
//******************************************
class MyEnumProc : public DependentEnumProc
{
public:
	Tab<ReferenceMaker*> anims;
	ReferenceTarget* me = nullptr;

	explicit MyEnumProc(ReferenceTarget* m) : me(m) {}
	virtual int proc(ReferenceMaker* rmaker) override;
};
