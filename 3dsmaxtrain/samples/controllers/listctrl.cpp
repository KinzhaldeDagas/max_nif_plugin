/**********************************************************************
 *<
	FILE: listctrl.cpp

	DESCRIPTION:  Implements built-in list controllers

	CREATED BY: Rolf Berteig

	HISTORY: created 9/16/95
	EDITED:  Moved out of Core and function published -- Adam Felt (08/30/00)
	EDITED:  Added weights and pose to pose blending  -- Adam Felt (02/21/02)
	EDITED:  Added transform list, UI redesign  -- Kelvin Zelt (11/11/2022)

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#include "ctrl.h"
#include "decomp.h"
#include "iparamb2.h"
#include "istdplug.h"
#include "notify.h"
#include <ILockedTracks.h>
#include <vector>
#include "3dsmaxport.h"
#include "MatrixMath.h"
#include "listControlUI.h"
#include <iomanip> // setprecision
#include <sstream> // stringstream
#include "maxicon.h"
#include "TrackView/AssignController.h"
#include <irefhierarchy.h> // detect instancing, IsRefTargetInstanced
#include "listctrl.h"

 // QT Support
#include "ui_ListControl.h"
#include <QtGui/QBrush>
#include <QtWidgets/QLayout>
#include <QtCore/QString>
#include <QtCore/QVariant>

#include "Qt/QmaxMainWindow.h"

extern HINSTANCE hInstance;

#define FLOATLIST_CNAME GetString(IDS_RB_FLOATLIST)
#define POINT3LIST_CNAME GetString(IDS_RB_POINT3LIST)
#define POSLIST_CNAME GetString(IDS_RB_POSITIONLIST)
#define ROTLIST_CNAME GetString(IDS_RB_ROTATIONLIST)
#define SCALELIST_CNAME GetString(IDS_RB_SCALELIST)
#define BLOCKCONTROL_CNAME GetString(IDS_PW_BLOCKCONTROL)
#define POINT4LIST_CNAME GetString(IDS_RB_POINT4LIST)
#define TMLIST_CNAME GetString(IDS_RB_TRANSFORMLIST)

#define PBLOCK_INIT_REF_ID 2

#define WEIGHT_MIN -10000.0f
#define WEIGHT_MAX 10000.0f


class ListControl;

typedef Tab<Control*> ControlTab;

class NameList : public Tab<TSTR*>
{
public:
	void Free()
	{
		for (int i = 0; i < Count(); i++)
		{
			delete (*this)[i];
			(*this)[i] = NULL;
		}
	}
	void Duplicate()
	{
		for (int i = 0; i < Count(); i++)
		{
			if ((*this)[i])
				(*this)[i] = new TSTR(*(*this)[i]);
		}
	}
};

class ListControl
		: public IListControl
		, public ILockedTrackImp
{
private:
	float _cached_total_weight{ 0.f };
	bool _cached_total_weight_is_valid{ false };

	QPointer<QDialog> mFloatingRollout = nullptr;
	int weightModeActive; // Holds the active controller only in weight mode.

public:
	ListDummyEntry dummy;
	Tab<Control*> conts; // ControlTab
	Control* clip; // ListControl
	NameList names;
	TSTR nameClip;
	float weightClip;

	//Control* indexControl = nullptr;
	IParamBlock2* pblock;
	IObjParam* storedObjParams = nullptr;
	ListControlRollup* mMotionRollout = nullptr; // Not null if we are currently editing in motion panel.
	DWORD paramFlags;

	// Returns if the sub controllers weight has a key at 'in_time'.
	bool GetSubCtrlWeightAtKey(int in_index, TimeValue in_time) const;
	// Set a sub controllers weight value.
	void SetSubCtrlWeight(int in_index, TimeValue in_time, float weight);

	// Floater dialog methods
	void CloseFloater(); //Tell the floater to close itself.
	void ClearFloaterPointer(); //tell teh controller to clear the pointer to the floating rollout so its not danging.
	// Make floating UI Rollout for this controller and remember is in 'floatingDialog';
	// Object params needed so that the list control dialog can begin and end the sub controllers edit mode.
	void MakeFloater(IObjParam* ip, std::wstring title);
	bool HasFloater() const;
	ListControlRollup* GetFloatingRollout() const;

	ListControl(BOOL loading = FALSE);
	ListControl(const ListControl& ctrl);
	virtual ~ListControl();

	ListControl& operator=(const ListControl& ctrl);
	void Resize(int c);
	float AverageWeight(float weight, TimeValue t);
	BOOL GetAverage() const;
	void SetIndexModeIndex(TimeValue t, int index);
	int GetIndexModeIndex(TimeValue t, Interval& valid) const;
	int GetIndexModeIndex(TimeValue t) const;

	// false = fan, true = chain
	BOOL GetSequential() const;
	// false = weight against identity, true = LERP from previous
	bool GetWeightAgainstMode() const;

	virtual ListControl* DerivedClone() = 0;
	virtual ClassDesc* GetClassDesc() = 0;
	
	// From IListControl
	int GetListCount() const override
	{
		return conts.Count();
	}

	// Get/Set active index.
	// Weight mode: weight mode active index.
	// Index mode: the index control in the pblock.
	void SetActive(int index) override;
	int GetActive() const override;

	// Get/Set tag.
	void SetTag(const MSTR& tag) override;
	MSTR GetTag() const override;

	// pureSet = true skips edit params step, mode check
	void SetWeightModeActive(int index, bool pureSet);
	void SetWeightModeActive(int index) {
		SetWeightModeActive(index, false);
	}
	int GetWeightModeActive() const;

	void AppendItem();
	void AssignItem(int index, QWidget* dialogParent);
	void DeleteItem(int index) override;
	void CutItem(int index) override;
	void PasteItem(int index) override;
	void SetName(int index, TSTR name) override;
	TSTR GetName(int index) override;

	virtual Control* GetSubCtrl(int in_index) const override;
	virtual float GetSubCtrlWeight(int in_index, TimeValue in_time) const override;

	// Populate the vector with all the subcontrollers weights. Returns the number of non zero weights.
	size_t GetWeights(TimeValue t, Interval& valid, std::vector<float>& weights) const;

	virtual bool SetIndexByName(MSTR name) override;
	virtual bool IsIndexMode() const override;

	// From FPMixinInterface
	BaseInterface* GetInterface(Interface_ID id)
	{
		if (id == LIST_CONTROLLER_INTERFACE)
			return (IListControl*)this;
		else
			return FPMixinInterface::GetInterface(id);
	}


	// From Control
	void Copy(Control* from) override;
	void CommitValue(TimeValue t) override;
	void RestoreValue(TimeValue t) override;
	BOOL IsLeaf() override
	{
		return FALSE;
	}
	int IsKeyable() override;
	void EnumIKParams(IKEnumCallback& callback) override;
	BOOL CompDeriv(TimeValue t, Matrix3& ptm, IKDeriv& derivs, DWORD flags) override;
	void MouseCycleCompleted(TimeValue t) override;
	BOOL InheritsParentTransform() override;
	BOOL IsReplaceable() override
	{
		return !GetLocked();
	}
	// Delegating ChangeParents to the active controller
	virtual BOOL ChangeParents(TimeValue t, const Matrix3& oldP, const Matrix3& newP, const Matrix3& tm) override;

	// From Animatable
	virtual int NumSubs() override
	{
		return conts.Count() + 2; // 3;
	} // numControllers+dummyController+pblock //+index
	virtual Animatable* SubAnim(int i) override;
	virtual TSTR SubAnimName(int i, bool localized) override;
	void DeleteThis() override
	{
		delete this;
	}
	void AddNewKey(TimeValue t, DWORD flags) override;
	void CloneSelectedKeys(BOOL offset) override;
	void DeleteKeys(DWORD flags) override;
	void SelectKeys(TrackHitTab& sel, DWORD flags) override;
	BOOL IsKeySelected(int index) override;
	void BeginEditParams(IObjParam* ip, ULONG flags, Animatable* prev) override;
	void EndEditParams(IObjParam* ip, ULONG flags, Animatable* next) override;
	ParamDimension* GetParamDimension(int i) override;
	BOOL AssignController(Animatable* control, int subAnim) override;
	void CopyKeysFromTime(TimeValue src, TimeValue dst, DWORD flags) override;
	void DeleteKeyAtTime(TimeValue t) override;
	BOOL IsKeyAtTime(TimeValue t, DWORD flags) override;
	BOOL GetNextKeyTime(TimeValue t, DWORD flags, TimeValue& nt) override;
	int GetKeyTimes(Tab<TimeValue>& times, Interval range, DWORD flags) override;
	int GetKeySelState(BitArray& sel, Interval range, DWORD flags) override;
	void EditTrackParams(
			TimeValue t, ParamDimensionBase* dim, const TCHAR* pname, HWND hParent, IObjParam* ip, DWORD flags) override;
	int TrackParamsType() override
	{
		if (GetLocked() == false)
			return TRACKPARAMS_WHOLE;
		else
			return TRACKPARAMS_NONE;
	}
	int SubNumToRefNum(int subNum) override;
	Interval GetTimeRange(DWORD flags) override;
	void* GetInterface(ULONG id) override
	{
		switch (id)
		{
		case I_LOCKED:
			return (ILockedTrackImp*)this;
		}
		return Control::GetInterface(id);
	}
	BOOL CanAssignController(int subAnim) override
	{
		return !GetLocked();
	}
	// From ReferenceTarget
	int NumParamBlocks() override
	{
		return 1;
	} // return number of ParamBlocks in this instance
	IParamBlock2* GetParamBlock(int i) override
	{
		return pblock;
	} // return i'th ParamBlock
	IParamBlock2* GetParamBlockByID(BlockID id) override
	{
		return (pblock->ID() == id) ? pblock : NULL;
	} // return id'd ParamBlock

	int NumRefs() override
	{
		return conts.Count() + 3;
	} // numControllers+dummyController+controllerInBuffer+pblock
	RefTargetHandle GetReference(int i) override;

private:
	virtual void SetReference(int i, RefTargetHandle rtarg) override;

public:
	RefTargetHandle Clone(RemapDir& remap) override;
	RefResult NotifyRefChanged(const Interval&, RefTargetHandle, PartID&, RefMessage, BOOL) override;
	void NotifyForeground(TimeValue t) override;
	IOResult Save(ISave* isave) override;
	IOResult Load(ILoad* iload) override;
};

ClassDesc* GetFloatListDesc();
// Control* ListControl::clip = NULL;

bool ActiveOnlyFilter(Animatable* anim, Animatable* parent, int subAnimIndex, Animatable* grandParent, INode* node)
{
	if (parent != NULL &&
			(parent->ClassID() == Class_ID(FLOATLIST_CONTROL_CLASS_ID, 0) ||
					parent->ClassID() == Class_ID(POINT3LIST_CONTROL_CLASS_ID, 0) ||
					parent->ClassID() == Class_ID(POSLIST_CONTROL_CLASS_ID, 0) ||
					parent->ClassID() == Class_ID(ROTLIST_CONTROL_CLASS_ID, 0) ||
					parent->ClassID() == Class_ID(SCALELIST_CONTROL_CLASS_ID, 0) ||
					parent->ClassID() == Class_ID(POINT4LIST_CONTROL_CLASS_ID, 0) ||
					parent->ClassID() == Class_ID(TMLIST_CONTROL_CLASS_ID, 0)))
	{
		if (subAnimIndex == ((ListControl*)parent)->GetActive() ||
				subAnimIndex > ((ListControl*)parent)->GetListCount())
			return true;
		return false;
	}
	return true;
}

bool NoWeightsFilter(Animatable* anim, Animatable* parent, int subAnimIndex, Animatable* grandParent, INode* node)
{
	if (parent != NULL &&
			(parent->ClassID() == Class_ID(FLOATLIST_CONTROL_CLASS_ID, 0) ||
					parent->ClassID() == Class_ID(POINT3LIST_CONTROL_CLASS_ID, 0) ||
					parent->ClassID() == Class_ID(POSLIST_CONTROL_CLASS_ID, 0) ||
					parent->ClassID() == Class_ID(ROTLIST_CONTROL_CLASS_ID, 0) ||
					parent->ClassID() == Class_ID(SCALELIST_CONTROL_CLASS_ID, 0) ||
					parent->ClassID() == Class_ID(POINT4LIST_CONTROL_CLASS_ID, 0) ||
					parent->ClassID() == Class_ID(TMLIST_CONTROL_CLASS_ID, 0)))
	{
		// This is as simple as filtering out the parameter block
		// at least for now...
		if (subAnimIndex == ((ListControl*)parent)->GetListCount() + 1)
			return false;
	}
	return true;
}

void ListCtrlNotifyStartup(void* param, NotifyInfo* info)
{
	ITrackBar* tb = GetCOREInterface()->GetTrackBar();
	if (tb != NULL)
	{
		ITrackBarFilterManager* filterManager =
				(ITrackBarFilterManager*)tb->GetInterface(TRACKBAR_FILTER_MANAGER_INTERFACE);
		if (filterManager != NULL)
		{
			filterManager->RegisterFilter(
					ActiveOnlyFilter, NULL, GetString(IDS_AF_ACTIVE_ONLY_FILTER), Class_ID(0x39822380, 0x69326448));
			filterManager->RegisterFilter(
					NoWeightsFilter, NULL, GetString(IDS_AF_NO_WEIGHTS_FILTER), Class_ID(0x5434724d, 0x10b22061));
		}
	}
}

class ListCtrlTrackBarFilterRegister
{
public:
	ListCtrlTrackBarFilterRegister()
	{
		RegisterNotification(ListCtrlNotifyStartup, NULL, NOTIFY_SYSTEM_STARTUP);
	}
};
ListCtrlTrackBarFilterRegister theListCtrlTrackBarFilterRegister;

#define LISTDLG_CONTREF 0

class ListControlRestore : public RestoreObj
{
public:
	ListControl* cont;
	ControlTab list, rlist;
	NameList unames, rnames;
	int active, ractive;

	ListControlRestore(ListControl* c)
		:RestoreObj()
	{
		cont = c;
		list = c->conts;
		active = c->GetWeightModeActive();
		unames = c->names;
		unames.Duplicate();
	}
	void Restore(int isUndo) override
	{
		rlist = cont->conts;
		ractive = cont->GetWeightModeActive();
		rnames = cont->names;
		rnames.Duplicate();

		cont->conts = list;
		cont->SetWeightModeActive(active, true/*pure set*/);
		cont->names.Free();
		cont->names = unames;
		cont->names.Duplicate();

		cont->NotifyDependents(FOREVER, 0, REFMSG_CHANGE);
		cont->NotifyDependents(FOREVER, 0, REFMSG_SUBANIM_STRUCTURE_CHANGED);
	}
	void Redo() override
	{
		cont->conts = rlist;
		cont->SetWeightModeActive(ractive, true/*pure set*/);
		cont->names.Free();
		cont->names = rnames;
		cont->names.Duplicate();

		cont->NotifyDependents(FOREVER, 0, REFMSG_CHANGE);
		cont->NotifyDependents(FOREVER, 0, REFMSG_SUBANIM_STRUCTURE_CHANGED);
	}
	TSTR Description() override
	{
		return _T("List Control");
	}
};

// The restore object handles stopping and starting editing
// durring an undo. One of these is placed before and after
// a cut copy/paste/delete operation.
class RemoveItemRestore : public RestoreObj
{
public:
	ListControl* cont;
	BOOL parity;

	RemoveItemRestore(ListControl* c, BOOL p)
	{
		cont = c;
		parity = p;
	}
	void Restore(int isUndo) override
	{
		if(cont->mMotionRollout)
		{
			if (parity) {
				cont->mMotionRollout->EndEdit(cont->GetActive());
			}
			else {
				cont->mMotionRollout->StartEdit(cont->GetActive());
			}
		}
	}
	void Redo() override
	{
		if (cont->mMotionRollout)
		{
			if (!parity)
				cont->mMotionRollout->EndEdit(cont->GetActive());
			else
				cont->mMotionRollout->StartEdit(cont->GetActive());
		}
	}
	TSTR Description() override
	{
		return _T("List control remove item");
	}
};

class ListSizeRestore : public RestoreObj
{
public:
	ListControl* cont;

	ListSizeRestore(ListControl* c)
	{
		cont = c;
	}
	void Restore(int isUndo) override
	{
		cont->conts.Resize(cont->conts.Count() - 1);
		cont->names.Resize(cont->names.Count() - 1);
		
		int activeIndex = cont->GetWeightModeActive();
		if (activeIndex >= cont->conts.Count())
			activeIndex = cont->conts.Count() - 1;
		if (activeIndex < 0)
			activeIndex = 0;
		cont->SetWeightModeActive(activeIndex, true/*pure set*/);
		
		cont->NotifyDependents(FOREVER, 0, REFMSG_SUBANIM_STRUCTURE_CHANGED);
	}
	void Redo() override
	{
		cont->conts.SetCount(cont->conts.Count() + 1);
		cont->conts[cont->conts.Count() - 1] = nullptr;
		cont->names.SetCount(cont->names.Count() + 1);
		cont->names[cont->names.Count() - 1] = nullptr;
		
		int activeIndex = cont->GetWeightModeActive();
		if (activeIndex >= cont->conts.Count())
			activeIndex = cont->conts.Count() - 1;
		if (activeIndex < 0)
			activeIndex = 0;
		cont->SetWeightModeActive(activeIndex, true/*pure set*/);

		cont->NotifyDependents(FOREVER, 0, REFMSG_SUBANIM_STRUCTURE_CHANGED);
	}
	TSTR Description() override
	{
		return _T("List size");
	}
};


// This restore object doesn't do anything except keep the clipboard
// item from being deleted.
class CutRestore
		: public RestoreObj
		, public ReferenceMaker
{
public:
	Control* cont;
	CutRestore(Control* c)
	{
		cont = NULL;
		HoldSuspend hs;
		ReplaceReference(0, c);
	}
	~CutRestore()
	{
		HoldSuspend hs;
		DeleteReference(0);
	}
	void Restore(int isUndo) override
	{
	}
	void Redo() override
	{
	}
	TSTR Description() override
	{
		return _T("List Control Cut");
	}
	Class_ID ClassID() override
	{
		return Class_ID(0x65b762b, 0xb335f61);
	}
	virtual void GetClassName(MSTR& s, bool localized) const override
	{
		s = _M("CutRestore");
	} // from Animatable
	int NumRefs() override
	{
		return 1;
	}
	RefTargetHandle GetReference(int i) override
	{
		return cont;
	}

private:
	virtual void SetReference(int i, RefTargetHandle rtarg) override
	{
		cont = (Control*)rtarg;
	}

public:
	RefResult NotifyRefChanged(
			const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate) override
	{
		if (message == REFMSG_TARGET_DELETED && hTarget == cont) {
			cont = nullptr;
		}
		return REF_SUCCEED;
	}
	BOOL CanTransferReference(int i) override
	{
		return FALSE;
	}
};

void ListDummyEntry::Init(Control* l)
{
	lc = l;
}
SClass_ID ListDummyEntry::SuperClassID()
{
	return lc->SuperClassID();
}

// added by AF -- some controllers don't initialize values before calling GetValue.
// This does the initializing for them
void ListDummyEntry::GetValue(TimeValue t, void* val, Interval& valid, GetSetMethod method)
{
	if (method == CTRL_ABSOLUTE)
	{
		switch (SuperClassID())
		{
		case CTRL_POSITION_CLASS_ID:
		case CTRL_POINT3_CLASS_ID:
			*((Point3*)val) = Point3(0, 0, 0);
			break;
		case CTRL_POINT4_CLASS_ID:
			*((Point4*)val) = Point4(0, 0, 0, 0);
			break;
		case CTRL_FLOAT_CLASS_ID:
			*((float*)val) = 0.0f;
			break;
		case CTRL_SCALE_CLASS_ID:
			*((ScaleValue*)val) = ScaleValue(Point3(1, 1, 1));
			break;
		case CTRL_ROTATION_CLASS_ID:
			*((Quat*)val) = Quat(0.0f, 0.0f, 0.0f, 1.0f);
			break;
		default:
			break;
		}
	}
}


ListControl::ListControl(BOOL loading)
	: IListControl(), ILockedTrackImp()
{
	// NULL out refs
	pblock = nullptr;
	clip = nullptr;
	mMotionRollout = nullptr;
	weightModeActive = 0;
	dummy.Init(this);
	paramFlags = 0;
	weightClip = 0.0f;
}

ListControl::ListControl(const ListControl& ctrl)
{
	//indexControl = nullptr;
	pblock = nullptr;
	clip = nullptr;
	DeleteAllRefsFromMe();
	Resize(ctrl.conts.Count());
	int i = 0;
	for (i = 0; i < ctrl.conts.Count(); i++)
	{
		ReplaceReference(i, ctrl.conts[i]);
	}
	ReplaceReference(i, nullptr);
	ReplaceReference(i + 1, ctrl.clip);
	ReplaceReference(i + 2, ctrl.pblock);
	names = ctrl.names;
	names.Duplicate();
	weightClip = ctrl.weightClip;
	weightModeActive = ctrl.weightModeActive;
	dummy.Init(this);
	paramFlags = 0;
	mMotionRollout = nullptr;
	mLocked = ctrl.mLocked;
}

ListControl::~ListControl()
{
	CloseFloater();
	DeleteAllRefsFromMe();
	// RK:12/15/00, for maxscript to get notified when the list controller is deleted
	dummy.NotifyDependents(FOREVER, 0, REFMSG_TARGET_DELETED, NOTIFY_ALL, TRUE, &dummy);
	names.Free();
}

void ListControl::CloseFloater()
{
	if(mFloatingRollout != nullptr)
	{
		// Destroy the PamamMap2 before the UI is destroyed.
		ListControlRollup* rr = GetFloatingRollout();
		if (rr != nullptr) {
			rr->DestroyParamMap();
		}
		mFloatingRollout->close();
	}
	mFloatingRollout = nullptr;
}

void ListControl::ClearFloaterPointer()
{
	mFloatingRollout = nullptr;
}

void ListControl::MakeFloater(IObjParam* ip, std::wstring title)
{
	if(storedObjParams == nullptr)
	{
		DbgAssert(false);
		return;
	}
	
	if (mFloatingRollout == nullptr)
	{
		QDialog* floater = new QDialog((QWidget*)GetCOREInterface()->GetQmaxMainWindow());
		floater->setAttribute(Qt::WA_DeleteOnClose);
		//Remove the "what's this?" button from floater title bar.
		floater->setWindowFlags(floater->windowFlags() & ~Qt::WindowContextHelpButtonHint);
		ListControlRollup* listControlRollout = new ListControlRollup(false, ip);
		listControlRollout->SetParamBlock(this, nullptr/*unused param*/);
		floater->setLayout(new QVBoxLayout());
		floater->layout()->addWidget(listControlRollout);

		// Make sure the dialog is big enough when using HDPI scaling.
		int dialogHeight = MaxSDK::UIScaled(300);
		if (SuperClassID() == CTRL_MATRIX3_CLASS_ID) {
			dialogHeight = MaxSDK::UIScaled(450); // Transform list has more UI so its bigger.
		}
		floater->setMinimumWidth(MaxSDK::UIScaled(200));
		floater->setMinimumHeight(dialogHeight);
		floater->resize(MaxSDK::UIScaled(300), dialogHeight);

		MSTR mstrTitle;
		GetClassName(mstrTitle);
		std::wstring floaterTitle = mstrTitle.data();
		floaterTitle += std::wstring(L" : ") + title;
		floater->setWindowTitle(QString::fromStdWString(floaterTitle));

		//remember the floater.
		mFloatingRollout = floater;
	}

	// Make sure we always pass on the object params
	ListControlRollup* rr = GetFloatingRollout();
	if(rr != nullptr) {
		rr->SetObjectParams(ip);
	}
	else {
		DbgAssert(false); // sanity check
	}
	mFloatingRollout->show();
}

bool ListControl::HasFloater() const
{
	if(mFloatingRollout != nullptr) {
		return true;
	}
	return false;
}

ListControlRollup* ListControl::GetFloatingRollout() const
{
	if (mFloatingRollout == nullptr) {
		return nullptr;
	}
	ListControlRollup* rr = mFloatingRollout->findChild<ListControlRollup*>();
	if (rr == nullptr) {
		return nullptr;
	}
	return rr;
}

void ListControl::Resize(int c)
{
	int pc = conts.Count();
	conts.SetCount(c);
	names.SetCount(c);
	pblock->EnableNotifications(FALSE);
	pblock->SetCount(kListCtrlWeight, c);
	// pblock->EnableNotifications(TRUE);		// CAL-10/18/2002: keep it disabled here (459225)
	for (int i = pc; i < c; i++)
	{
		conts[i] = NULL;
		names[i] = NULL;
		pblock->SetValue(kListCtrlWeight, 0, 1.0f, i);
	}
	pblock->EnableNotifications(TRUE); // CAL-10/18/2002: enable it here
}

ListControl& ListControl::operator=(const ListControl& ctrl)
{
	if (this == &ctrl) {
		return *this;
	}

	int i;
	RemapDir* remap = NewRemapDir();
	for (i = 0; i < ctrl.conts.Count(); i++)
	{
		ReplaceReference(i, remap->CloneRef(ctrl.conts[i]));
	}
	ReplaceReference(i + 2, remap->CloneRef(ctrl.pblock));
	remap->Backpatch();
	remap->DeleteThis();
	names = ctrl.names;
	names.Duplicate();
	weightClip = ctrl.weightClip;
	weightModeActive = ctrl.weightModeActive;
	mLocked = ctrl.mLocked;
	return *this;
}

Interval ListControl::GetTimeRange(DWORD flags)
{
	Interval range = NEVER;
	bool init = false;
	for (int i = 0; i < conts.Count(); i++)
	{
		if (conts[i])
		{
			if (!init)
			{
				range = conts[i]->GetTimeRange(flags);
				init = true;
			}
			else
			{
				Interval iv = conts[i]->GetTimeRange(flags);
				if (!iv.Empty())
				{
					if (!range.Empty())
					{
						range += iv.Start();
						range += iv.End();
					}
					else
					{
						range = iv;
					}
				}
			}
		}
	}
	return range;
}

RefTargetHandle ListControl::Clone(RemapDir& remap)
{
	ListControl* ctrl = DerivedClone();

	ctrl->Resize(conts.Count());
	int i;
	for (i = 0; i < conts.Count(); i++)
	{
		ctrl->ReplaceReference(i, remap.CloneRef(conts[i]));
	}
	ctrl->ReplaceReference(i + 1, remap.CloneRef(clip));
	ctrl->ReplaceReference(i + 2, remap.CloneRef(pblock));
	ctrl->weightModeActive = weightModeActive;
	ctrl->names = names;
	ctrl->names.Duplicate();
	ctrl->weightClip = weightClip;
	ctrl->mLocked = mLocked;
	BaseClone(this, ctrl, remap);
	return ctrl;
}

void ListControl::Copy(Control* from)
{
	if (GetLocked() == false)
	{
		if (from->ClassID() != Class_ID(DUMMY_CONTROL_CLASS_ID, 0))
		{
			if (theHold.Holding()) {
				theHold.Put(new ListSizeRestore(this));
			}
			Resize(conts.Count() + 1);
			ReplaceReference(conts.Count() - 1, from);
		}
	}
}

void ListControl::CommitValue(TimeValue t)
{
	const int index = GetActive();
	if (index < 0 || index >= conts.Count())
		return;
	if (conts[index] != nullptr)
		conts[index]->CommitValue(t);
}

void ListControl::RestoreValue(TimeValue t)
{
	const int index = GetActive();
	if (index >= 0 && index < conts.Count() && conts[index])
		conts[index]->RestoreValue(t);
}

int ListControl::IsKeyable()
{
	const int index = GetActive();
	if (index < 0 || index >= conts.Count() || conts[index] == nullptr)
		return 0;
	return conts[index]->IsKeyable();
}

void ListControl::EnumIKParams(IKEnumCallback& callback)
{
	const int index = GetActive();
	if (index < 0 || index >= conts.Count())
		return;
	if (conts[index])
		conts[index]->EnumIKParams(callback);
}

BOOL ListControl::CompDeriv(TimeValue t, Matrix3& ptm, IKDeriv& derivs, DWORD flags)
{
	const int index = GetActive();
	if (index < 0 || index >= conts.Count())
		return FALSE;
	
	// Note: ptm is not correct if there are controllers before
	if (conts[index])
		return conts[index]->CompDeriv(t, ptm, derivs, flags);
	return FALSE;
}

void ListControl::MouseCycleCompleted(TimeValue t)
{
	const int index = GetActive();
	if (index < 0 || index >= conts.Count())
		return;
	if (conts[index])
		conts[index]->MouseCycleCompleted(t);
}

// ambarish added this method on 11/28/2000
// the purpose is to ensure that the controller that doesn't want to inherit parent transform doesn't do so
BOOL ListControl::InheritsParentTransform()
{
	if (!conts.Count())
		return FALSE;
	//	int activeControlNumber =  GetActive();
	for (int i = 0; i <= GetActive(); ++i)
	{ // loop through each item in the list up to (and including) the active item
		if (conts[i] && conts[i]->InheritsParentTransform() == FALSE)
		{
			return FALSE;
		}
	}
	return TRUE;
}

BOOL ListControl::ChangeParents(TimeValue t, const Matrix3& oldP, const Matrix3& newP, const Matrix3& tm)
{
	const int index = GetActive();
	if (index < 0 || index >= conts.Count())
		return FALSE;

	if (conts[index])
		return conts[index]->ChangeParents(t, oldP, newP, tm);
	else
		return FALSE;
}


void ListControl::AddNewKey(TimeValue t, DWORD flags)
{
	if (GetLocked())
		return;
	for (int i = 0; i < conts.Count(); i++)
	{
		if (conts[i])
			conts[i]->AddNewKey(t, flags);
	}
}
void ListControl::CloneSelectedKeys(BOOL offset)
{
	if (GetLocked())
		return;
	const int index = GetActive();
	if (index < 0 || index >= conts.Count() || conts[index] == nullptr)
		return;
	conts[index]->CloneSelectedKeys(offset);
}
void ListControl::DeleteKeys(DWORD flags)
{
	if (GetLocked())
		return;
	const int index = GetActive();
	if (index < 0 || index >= conts.Count() || conts[index] == nullptr)
		return;
	conts[index]->DeleteKeys(flags);
}
void ListControl::SelectKeys(TrackHitTab& sel, DWORD flags)
{
	const int index = GetActive();
	if (index < 0 || index >= conts.Count() || conts[index] == nullptr)
		return;
	conts[index]->SelectKeys(sel, flags);
}

BOOL ListControl::IsKeySelected(int index)
{
	const int activeIndex = GetActive();
	if (activeIndex < 0 || index >= conts.Count() || conts[activeIndex] == nullptr)
		return FALSE;
	return conts[activeIndex]->IsKeySelected(index);
}

void ListControl::CopyKeysFromTime(TimeValue src, TimeValue dst, DWORD flags)
{
	if (GetLocked())
		return;
	const int index = GetActive();
	if (index < 0 || index >= conts.Count() || conts[index] == nullptr)
		return;
	conts[index]->CopyKeysFromTime(src, dst, flags);
}
void ListControl::DeleteKeyAtTime(TimeValue t)
{
	if (GetLocked())
		return;
	const int index = GetActive();
	if (index < 0 || index >= conts.Count() || conts[index] == nullptr)
		return;
	conts[index]->DeleteKeyAtTime(t);
}
BOOL ListControl::IsKeyAtTime(TimeValue t, DWORD flags)
{
	const int index = GetActive();
	if (index < 0 || index >= conts.Count() || conts[index] == nullptr)
		return FALSE;
	return conts[index]->IsKeyAtTime(t, flags);
}

BOOL ListControl::GetNextKeyTime(TimeValue t, DWORD flags, TimeValue& nt)
{
	const int index = GetActive();
	if (index < 0 || index >= conts.Count() || conts[index] == nullptr)
		return FALSE;
	return conts[index]->GetNextKeyTime(t, flags, nt);
}

int ListControl::GetKeyTimes(Tab<TimeValue>& times, Interval range, DWORD flags)
{
	const int index = GetActive();
	if (index < 0 || index >= conts.Count() || conts[index] == nullptr)
		return 0;
	return conts[index]->GetKeyTimes(times, range, flags);
}

int ListControl::GetKeySelState(BitArray& sel, Interval range, DWORD flags)
{
	const int index = GetActive();
	if (index < 0 || index >= conts.Count() || conts[index] == nullptr)
		return 0;
	return conts[index]->GetKeySelState(sel, range, flags);
}

RefResult ListControl::NotifyRefChanged(
		const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate)
{
	switch (message)
	{
	case REFMSG_WANT_SHOWPARAMLEVEL: // show the paramblock2 entry in trackview
	{
		BOOL* pb = (BOOL*)(partID);
		*pb = TRUE;
		return REF_HALT;
	}
	case REFMSG_SUBANIM_STRUCTURE_CHANGED:
	case REFMSG_CHANGE:
	{
		_cached_total_weight_is_valid = false;
	}
	break;
	}

	return REF_SUCCEED;
}

void ListControl::NotifyForeground(TimeValue t)
{
	const int index = GetActive();
	if (index < 0 || index >= conts.Count() || conts[index] == nullptr)
		return;
	conts[index]->NotifyForeground(t);
}

Animatable* ListControl::SubAnim(int i)
{
	if (i == conts.Count() + 1)
		return pblock;
	if (i == conts.Count()) {
		return &dummy;
	}
	if(i >= 0 && i < conts.Count()) {
		return conts[i];
	}
	return nullptr;
}

TSTR ListControl::SubAnimName(int i, bool localized)
{
	TSTR name;
	if (i == conts.Count() + 1) {
		return localized ? GetString(IDS_AF_LIST_WEIGHTS) : _T("Weights"); // the parameter block
	}
	if (i == conts.Count()) {
		return localized ? GetString(IDS_RB_AVAILABLE) : _T("Available");
	}
	if(i >= 0 && i < conts.Count())
	{
		if (names[i] && names[i]->length()) {
			name = *names[i];
		}
		else if (conts[i]) {
			conts[i]->GetClassName(name, localized);
		}
		else {
			name = localized ? GetString(IDS_RB_AVAILABLE) : _T("Available");
		}
	}
	return name;
}

int ListControl::SubNumToRefNum(int subNum)
{
	if (subNum <= conts.Count())
		return subNum;
	if (subNum == conts.Count() + 1)
		return subNum + 1; // the parameter block
	return -1;
}

RefTargetHandle ListControl::GetReference(int i)
{
	if (i < conts.Count())
		return conts[i]; // controllers in the list
	if (i == conts.Count())
		return NULL; // available dummy controller
	if (i == conts.Count() + 1)
		return clip; // controller in the copy buffer
	if (i == conts.Count() + 2)
		return pblock; // parameter block
	return NULL;
}

void ListControl::SetReference(int i, RefTargetHandle rtarg)
{
	if (i == conts.Count() + 2 || (rtarg && rtarg->ClassID() == Class_ID(PARAMETER_BLOCK2_CLASS_ID, 0)))
	{
		// set the parameter block
		pblock = (IParamBlock2*)rtarg;
		return;
	}

	if (i == conts.Count() + 1) // copying a controller to the buffer
	{
		clip = (Control*)rtarg;
		return;
	}

	if (i == conts.Count()) // pasting onto the available slot, grow the list by one
	{
		if (theHold.Holding())
		{
			theHold.Put(new ListSizeRestore(this));
		}
		Resize(conts.Count() + 1);
		conts[conts.Count() - 1] = (Control*)rtarg;
		if (!rtarg)
			names[conts.Count() - 1] = NULL;
		return;
	}

	if (i >= conts.Count())
		return; // if you make it here it is out of bounds...
	/*
		if (!rtarg) {
			conts.Delete(i,1);
			names.Delete(i,1);
			pblock->Delete(kListCtrlWeight, i, 1);
			}
		else
	*/
	conts[i] = (Control*)rtarg; // replacing a sub-controller
	if (!rtarg)
		names[i] = NULL;
}

ParamDimension* ListControl::GetParamDimension(int i)
{
	ParamDimension* dim = defaultDim;
	NotifyDependents(FOREVER, (PartID)&dim, REFMSG_GET_CONTROL_DIM);
	return dim;
}

BOOL ListControl::AssignController(Animatable* control, int subAnim)
{
	if (GetLocked())
		return FALSE;

	if (subAnim > conts.Count())
		subAnim = conts.Count();
	if (control == &dummy)
		return TRUE;
	if(control != nullptr)
	{
		// We are assigning a control (rather than assigning null to clear), make sure its a good super class.
		if(control->SuperClassID() != SuperClassID()) {
			return FALSE;
		}
	}

	if (subAnim == conts.Count())
	{
		if (theHold.Holding()) {
			theHold.Put(new ListSizeRestore(this));
		}
		Resize(conts.Count() + 1);
	}
	else
	{
		// Replacing an existing one, need to make sure it's not locked
		Control* cont = (Control*)SubAnim(subAnim);
		// Make sure its not locked and is replaceable.
		if(cont != nullptr &&
			((GetLockedTrackInterface(cont) && GetLockedTrackInterface(cont)->GetLocked() == true) ||
			!cont->IsReplaceable()))
		{
			return FALSE;
		}
	}

	ReplaceReference(SubNumToRefNum(subAnim), (RefTargetHandle)control);
	NotifyDependents(FOREVER, 0, REFMSG_CONTROLREF_CHANGE);
	NotifyDependents(FOREVER, 0, REFMSG_CHANGE);
	NotifyDependents(FOREVER, 0, REFMSG_SUBANIM_STRUCTURE_CHANGED);
	return TRUE;
}


#define LISTCOUNT_CHUNK 0x01010
#define LISTACTIVE_CHUNK 0x01020
#define ITEMNAME_CHUNK 0x01030
#define NONAME_CHUNK 0x01040
#define LOCK_CHUNK 0x2535 // the lock value
IOResult ListControl::Save(ISave* isave)
{
	ULONG nb;
	int count = conts.Count();
	isave->BeginChunk(LISTCOUNT_CHUNK);
	isave->Write(&count, sizeof(int), &nb);
	isave->EndChunk();

	isave->BeginChunk(LISTACTIVE_CHUNK);
	const int activeIndex = GetWeightModeActive();
	isave->Write(&activeIndex, sizeof(int), &nb);
	isave->EndChunk();

	int on = (mLocked == true) ? 1 : 0;
	isave->BeginChunk(LOCK_CHUNK);
	isave->Write(&on, sizeof(on), &nb);
	isave->EndChunk();

	for (int i = 0; i < count; i++)
	{
		if (names[i])
		{
			isave->BeginChunk(ITEMNAME_CHUNK);
			isave->WriteWString(*names[i]);
			isave->EndChunk();
		}
		else
		{
			isave->BeginChunk(NONAME_CHUNK);
			isave->EndChunk();
		}
	}

	return IO_OK;
}

class ListCtrlPostLoad : public PostLoadCallback
{
public:
	ListControl* listCtrl;

	ListCtrlPostLoad(ListControl* lc)
	{
		listCtrl = lc;
	}
	void proc(ILoad* iload)
	{
		// fill in the weight list when bringing in old files
		int listCount = listCtrl->pblock->Count(kListCtrlWeight);
		listCtrl->pblock->EnableNotifications(FALSE);
		listCtrl->pblock->SetCount(kListCtrlWeight, listCtrl->GetListCount());
		for (int i = listCount; i < listCtrl->GetListCount(); i++)
		{
			listCtrl->pblock->SetValue(kListCtrlWeight, 0, 1.0f, i);
		}
		listCtrl->pblock->EnableNotifications(TRUE);
		delete this;
	}
};


IOResult ListControl::Load(ILoad* iload)
{
	ULONG nb;
	IOResult res = IO_OK;
	int ix = 0;

	while (IO_OK == (res = iload->OpenChunk()))
	{
		switch (iload->CurChunkID())
		{
		case LISTCOUNT_CHUNK:
		{
			int count;
			res = iload->Read(&count, sizeof(count), &nb);
			Resize(count);
			break;
		}

		case LISTACTIVE_CHUNK:
			res = iload->Read(&weightModeActive, sizeof(int), &nb);
			break;

		case ITEMNAME_CHUNK:
		{
			TCHAR* buf;
			iload->ReadWStringChunk(&buf);
			names[ix++] = new TSTR(buf);
			break;
		}

		case NONAME_CHUNK:
			ix++;
			break;

		case LOCK_CHUNK:
		{
			int on;
			res = iload->Read(&on, sizeof(on), &nb);
			if (on)
				mLocked = true;
			else
				mLocked = false;
		}
		break;
		}
		iload->CloseChunk();
		if (res != IO_OK)
			return res;
	}
	iload->RegisterPostLoadCallback(new ListCtrlPostLoad(this));
	return IO_OK;
}

void ListControl::SetTag(const MSTR& newTag)
{
	pblock->SetValue(kListCtrlTag, 0, newTag);
}

MSTR ListControl::GetTag() const
{
	const TCHAR* tag = NULL;
	pblock->GetValue(kListCtrlTag, 0, tag);
	MSTR tagStr = tag;
	return tagStr;
}

// Return the active index depending on what the mode is.
// Returns -1 if no valid available index.
int ListControl::GetActive() const
{
	if (IsIndexMode() == true) {
		return GetIndexModeIndex(GetCOREInterface()->GetTime());
	}
	return GetWeightModeActive();
}

void ListControl::SetActive(int index)
{
	if (IsIndexMode() == true) {
		return SetIndexModeIndex(GetCOREInterface()->GetTime(), index);
	}
	return SetWeightModeActive(index);
}

void ListControl::SetWeightModeActive(int index, bool pureSet)
{
	const int count = GetListCount();
	if(index >= count) {
		index = count - 1;
	}
	if(index < 0) {
		index = 0;
	}
	
	const int weightModeActiveIndex = weightModeActive;
	if(weightModeActiveIndex == index) {
		return;
	}

	// In index mode, we do not need to begin/end edit params
	// because that is managed by index modes index.
	// We can still allow the weight mode active index to change, it just has no impact.
	const bool reactToIndexChange = IsIndexMode() == false && pureSet == false;
	if(reactToIndexChange)
	{
		if (mMotionRollout != nullptr)
		{
			mMotionRollout->EndEdit(weightModeActiveIndex, index);
			mMotionRollout->StartEdit(index, weightModeActiveIndex);
		}
	}
	
	weightModeActive = index;

	if(reactToIndexChange)
	{
		//the active control changed
		NotifyDependents(FOREVER, 0, REFMSG_CHANGE);
	}
}
int ListControl::GetWeightModeActive() const
{
	int active = weightModeActive;
	if (conts.Count() == 0)
		return 0;
	if (active < 0)
		active = 0;
	if (active >= conts.Count())
		active = conts.Count() - 1;
	return active;
}

void ListControl::AppendItem()
{
	ClassDesc* classDesc = GetDefaultController(SuperClassID());
	if (classDesc == nullptr) {
		DbgAssert(false);
		return;
	}
	Control* newControl = (Control*)(classDesc->Create());
	if (newControl == nullptr) {
		DbgAssert(false);
		return;
	}

	ListControlRestore* rest = new ListControlRestore(this);
	theHold.Begin();
	theHold.Put(new RemoveItemRestore(this, 0));

	// Assign a control to the end.
	AssignController(newControl, GetListCount());
	// Make the new control the active one
	SetActive(GetListCount() - 1);

	theHold.Put(rest);
	theHold.Put(new RemoveItemRestore(this, 1));
	theHold.Accept(GetString(IDS_RB_APPENDCONTROLLER));
}

void ListControl::AssignItem(int index, QWidget* dialogParent)
{
	if (index < 0 || index >= GetListCount()) {
		return;
	}

	// When in weight mode, if we are assigning to the active index, end / begin the sub rollouts.
	// Need to check for mMotionRollout as that being non-null signals that the list controllers UI is in the
	// command panel, even if the driving UI for this call is in a floating dialog.
	const int activeIndex = GetActive();
	bool beginNeeded = false;
	if(activeIndex == index && mMotionRollout != nullptr) {
		mMotionRollout->EndEdit(GetActive());
		beginNeeded = true;
	}
	
	theHold.Begin();
	
	if (dialogParent == nullptr) {
		dialogParent = GetCOREInterface()->GetQmaxMainWindow();
	}
	std::vector<MaxSDK::TreeEntry> entries;
	entries.push_back({ SubAnim(index), this, index });
	MaxSDK::Array<Control*> newControls;
	// Don't clear motion panel here because we manage our rollouts manually.
	if(!MaxSDK::PerformAssignController(dialogParent, entries, newControls, false))
	{
		theHold.Cancel(); // User canceled the operation.
		return;
	}
	
	theHold.Accept(GetString(IDS_RB_ASSIGNCONTROLLER));

	if(beginNeeded) {
		mMotionRollout->StartEdit(GetActive());
	}
}

void ListControl::DeleteItem(int a)
{
	if (a < 0 || a >= conts.Count()) {
		return;
	}

	if (mMotionRollout)
		mMotionRollout->EndEdit(GetActive());

	// Remember the numberings to broadcast the change.
	Tab<ULONG> renumberings;
	const int count = conts.Count();
	// Loop from deleted index down the list.
	ULONG renum = MAKELONG(a, -1); // Removed
	renumberings.Append(1, &renum);
	for (int i = a + 1; i < count; i++) {
		renum = MAKELONG(i, i - 1); // Moved up
		renumberings.Append(1, &renum);
	}

	ListControlRestore* rest = new ListControlRestore(this);
	theHold.Begin();
	theHold.Put(new RemoveItemRestore(this, 0));
	DeleteReference(a);
	conts.Delete(a, 1);
	names.Delete(a, 1);
	pblock->Delete(kListCtrlWeight, a, 1);

	// Adjust the weight mode active index.
	int weightIndex = weightModeActive;
	if (weightIndex > a)
	{
		weightIndex--;
		if (weightIndex < 0)
			weightIndex = 0;
		SetWeightModeActive(weightIndex, true);
	}
	// Adjust the index mode index to keep it the same
	const TimeValue currentTime = GetCOREInterface()->GetTime();
	int indexModeIndex = 0;
	pblock->GetValue(kListCtrlIndex, currentTime, indexModeIndex);
	if (indexModeIndex > a)
	{
		indexModeIndex--;
		if (indexModeIndex < 0)
			indexModeIndex = 0;
		SetIndexModeIndex(currentTime, indexModeIndex);
	}

	theHold.Put(rest);
	if (renumberings.Count() != 0) {
		NotifyDependents(FOREVER, (PartID)&renumberings, REFMSG_SUBANIM_NUMBER_CHANGED, NOTIFY_ALL, FALSE);
	}
	NotifyDependents(FOREVER, 0, REFMSG_CHANGE);
	NotifyDependents(FOREVER, 0, REFMSG_SUBANIM_STRUCTURE_CHANGED);
	theHold.Put(new RemoveItemRestore(this, 1));
	theHold.Accept(GetString(IDS_RB_DELETECONTROLLER));

	if (mMotionRollout)
		mMotionRollout->StartEdit(GetActive());
}

void ListControl::CutItem(int a)
{
	if (a < 0 || a >= conts.Count()) {
		return;
	}

	if (mMotionRollout)
		mMotionRollout->EndEdit(GetActive());

	// Remember the numberings to broadcast the change.
	Tab<ULONG> renumberings;
	const int count = conts.Count();
	// Loop from deleted index down the list.
	ULONG renum = MAKELONG(a, -1); // Removed
	renumberings.Append(1, &renum);
	for (int i = a + 1; i < count; i++) {
		renum = MAKELONG(i, i - 1); // Moved up
		renumberings.Append(1, &renum);
	}

	ListControlRestore* rest = new ListControlRestore(this);
	ReplaceReference(conts.Count() + 1, conts[a]);
	if (names[a])
		nameClip = *names[a];
	else
		nameClip = _T("");
	pblock->GetValue(kListCtrlWeight, GetCOREInterface()->GetTime(), weightClip, a);
	theHold.Begin();
	theHold.Put(new RemoveItemRestore(this, 0));
	theHold.Put(new CutRestore(conts[a]));
	DeleteReference(a);
	conts.Delete(a, 1);
	names.Delete(a, 1);
	pblock->Delete(kListCtrlWeight, a, 1);

	// Adjust the weight mode active index.
	int weightIndex = weightModeActive;
	if (weightIndex > a)
	{
		weightIndex--;
		if (weightIndex < 0)
			weightIndex = 0;
		SetWeightModeActive(weightIndex, true);
	}
	// Adjust the index mode index to keep it the same
	const TimeValue currentTime = GetCOREInterface()->GetTime();
	int indexModeIndex = 0;
	pblock->GetValue(kListCtrlIndex, currentTime, indexModeIndex);
	if (indexModeIndex > a)
	{
		indexModeIndex--;
		if (indexModeIndex < 0)
			indexModeIndex = 0;
		SetIndexModeIndex(currentTime, indexModeIndex);
	}

	theHold.Put(rest);
	if (renumberings.Count() != 0) {
		NotifyDependents(FOREVER, (PartID)&renumberings, REFMSG_SUBANIM_NUMBER_CHANGED, NOTIFY_ALL, FALSE);
	}
	NotifyDependents(FOREVER, 0, REFMSG_CONTROLREF_CHANGE);
	NotifyDependents(FOREVER, 0, REFMSG_CHANGE);
	NotifyDependents(FOREVER, 0, REFMSG_SUBANIM_STRUCTURE_CHANGED);
	theHold.Put(new RemoveItemRestore(this, 1));
	theHold.Accept(GetString(IDS_RB_CUTCONTROLLER));

	if (mMotionRollout)
		mMotionRollout->StartEdit(GetActive());
}

void ListControl::PasteItem(int a)
{
	if (a < 0 || a > conts.Count()) {
		return;
	}

	if (!clip)
		return;

	if (mMotionRollout)
		mMotionRollout->EndEdit(GetActive());

	// Remember the numberings to broadcast the change.
	Tab<ULONG> renumberings;
	const int count = conts.Count();
	// Loop from added index down the list.
	ULONG renum = MAKELONG(-1, a); // Added
	renumberings.Append(1, &renum);
	for (int i = a; i < count; i++) {
		renum = MAKELONG(i, i + 1); // Moved down
		renumberings.Append(1, &renum);
	}
	
	theHold.Begin();
	theHold.Put(new RemoveItemRestore(this, 0));
	theHold.Put(new ListControlRestore(this));

	Control* null_ptr = NULL;
	if (a == conts.Count())
		conts.Append(1, &null_ptr);
	else
		conts.Insert(a, 1, &null_ptr);

	TSTR* ptr = nullptr;
	if(nameClip.length() > 0) {
		ptr = new TSTR(nameClip); //Only set the name if we had one so its not empty.
	}
	names.Insert(a, 1, &ptr);
	pblock->Insert(kListCtrlWeight, a, 1, &weightClip);
	ReplaceReference(a, clip);

	// Adjust the weight mode index to keep it the same
	int weightIndex = weightModeActive;
	if (weightIndex > a)
	{
		weightIndex++;
		SetWeightModeActive(weightIndex, true);
	}
	// Adjust the index mode index to keep it the same
	const TimeValue currentTime = GetCOREInterface()->GetTime();
	int indexModeIndex = 0;
	pblock->GetValue(kListCtrlIndex, currentTime, indexModeIndex);
	if (indexModeIndex > a)
	{
		indexModeIndex++;
		SetIndexModeIndex(currentTime, indexModeIndex);
	}

	// Notify about the reference change
	if (renumberings.Count() != 0) {
		NotifyDependents(FOREVER, (PartID)&renumberings, REFMSG_SUBANIM_NUMBER_CHANGED, NOTIFY_ALL, FALSE);
	}
	NotifyDependents(FOREVER, 0, REFMSG_CONTROLREF_CHANGE);
	NotifyDependents(FOREVER, 0, REFMSG_CHANGE);
	NotifyDependents(FOREVER, 0, REFMSG_SUBANIM_STRUCTURE_CHANGED);
	theHold.Put(new RemoveItemRestore(this, 1));
	theHold.Accept(GetString(IDS_RB_PASTECONTROLLER));
	DeleteReference(conts.Count() + 1);
	clip = nullptr;
	// Extra one to update the UI buttons because the clip is done.
	NotifyDependents(FOREVER, 0, REFMSG_CHANGE);

	if (mMotionRollout)
		mMotionRollout->StartEdit(GetActive());
}

void ListControl::SetName(int index, TSTR name)
{
	if (index < 0 || index >= names.Count() || name.length() == 0) {
		return;
	}
	if (!(names[index])) {
		names[index] = new TSTR;
	}
	*(names[index]) = name;
	NotifyDependents(FOREVER, 0, REFMSG_NODE_NAMECHANGE);
}


TSTR ListControl::GetName(int index)
{
	TSTR name;
	if (index < 0 || index >= conts.Count()) {
		return _T("");
	}
	if (names[index] != nullptr && names[index]->length() > 0) {
		return *(names[index]);
	}
	else
	{
		name = SubAnimName(index, true);
		return name;
	}
}


Control* ListControl::GetSubCtrl(int in_index) const
{
	if (in_index < 0 || in_index >= names.Count())
		return nullptr;

	return conts[in_index];
}

float ListControl::GetSubCtrlWeight(int in_index, TimeValue in_time) const
{
	float val = 0.0f;
	if (in_index >= 0 && in_index < conts.Count() && pblock &&
			in_index < pblock->Count(kListCtrlWeight))
	{
		pblock->GetValue(kListCtrlWeight, in_time, val, in_index);
	}
	return val;
}

size_t ListControl::GetWeights(TimeValue t, Interval& valid, std::vector<float>& weights) const
{
	// Grab all the weights to decide if we can optimize for the case where there is only 1.
	weights.resize(conts.Count());
	size_t numNonZeroWeights = 0;
	for (int i = 0; i < conts.Count(); ++i)
	{
		float weight = 0.0f;
		pblock->GetValue(kListCtrlWeight, t, weight, valid, i);
		if (fabs(weight) < std::numeric_limits<float>::epsilon()) {
			weights[i] = 0.0f;
			continue; //skip if no weight
		}
		weights[i] = weight;

		// Add up total non-zero weights to see how much work we have
		numNonZeroWeights++;
	}
	return numNonZeroWeights;
}

bool ListControl::SetIndexByName(MSTR entryNameMatch)
{
	if(entryNameMatch.length() == 0) {
		return false;
	}
	
	// Case insensitive matching.
	auto stdStringToLower = [](std::wstring& str)
	{
		std::transform(str.begin(), str.end(), str.begin(),
			[](unsigned char c) { return std::tolower(c); });
	};

	std::wstring searchText = entryNameMatch.data();
	stdStringToLower(searchText);

	// Find the first entry that matches the best
	const int contCount = GetListCount();
	int matchingIndex = -1;
	size_t closestMatch = std::wstring::npos;
	for (int i = 0; i < contCount; i++)
	{
		std::wstring name = GetName(i).data();
		stdStringToLower(name);
		const size_t matchingPosition = name.find(searchText);
		if (matchingPosition != std::wstring::npos && matchingPosition < closestMatch)
		{
			// This match is better than the previous one
			closestMatch = matchingPosition;
			matchingIndex = i;
			if (closestMatch == 0) {
				// Can't get a better match than this, break out
				break;
			}
		}
	}

	if (matchingIndex >= 0)
	{
		//Found something, set that index.
		SetIndexModeIndex(GetCOREInterface()->GetTime(), matchingIndex);
		return true;
	}
	return false;
}

bool ListControl::GetSubCtrlWeightAtKey(int in_index, TimeValue in_time) const
{
	if(pblock == nullptr) {
		return false;
	}
	return pblock->KeyFrameAtTimeByID(kListCtrlWeight, GetCOREInterface()->GetTime(), in_index);
}

void ListControl::SetSubCtrlWeight(int in_index, TimeValue in_time, float weight)
{
	if (in_index >= 0 && in_index < names.Count() && pblock &&
		in_index < pblock->Count(kListCtrlWeight))
	{
		pblock->SetValue(kListCtrlWeight, in_time, weight, in_index);
	}
}

//***************************************************************

void ListControl::EditTrackParams(
		TimeValue t, ParamDimensionBase* dim, const TCHAR* pname, HWND hParent, IObjParam* ip, DWORD flags)
{
	if (GetLocked()) {
		return;
	}

	storedObjParams = ip;
	MakeFloater(ip, pname);
}

void ListControl::BeginEditParams(IObjParam* ip, ULONG flags, Animatable* prev)
{
	if (GetLocked() == false)
	{
		paramFlags = flags;

		mMotionRollout = new ListControlRollup(true/*comman panel display mode*/, ip);
		mMotionRollout->SetParamBlock(this, GetParamBlock(0));
		const MSTR rollupTitle = ClassName();
		// Manually add the rollout because we have ROLLUP_DONT_ADD_TO_CP set.
		ip->AddRollupPage(*mMotionRollout, rollupTitle, 0);

		// Popup the rollouts for the active controller
		mMotionRollout->StartEdit(GetActive());
		
		ClassDesc* cd = GetClassDesc();
		if(cd != nullptr) {
			cd->BeginEditParams(ip, this, flags, prev);
		}
	}
}

void ListControl::EndEditParams(IObjParam* ip, ULONG flags, Animatable* next)
{
	if(mMotionRollout != nullptr)
	{
		// Destroy the PamamMap2 before the UI is destroyed.
		mMotionRollout->DestroyParamMap();

		// Manually delete the rollout because we have ROLLUP_DONT_ADD_TO_CP set.
		if (mMotionRollout->GetObjectParams() != nullptr)
			mMotionRollout->GetObjectParams()->DeleteRollupPage(*mMotionRollout);
		mMotionRollout = nullptr;
	}
	paramFlags = 0;
}

// This function is mis-named, it normalizes the weight.
// Note that the incoming weight is at time t and totals are computed in time 0.
// FIXME : Compute the total weight using the TimeValue argument.
// TODO: KZ, to make this total the weights at the current frame would break backwards compatibility.
float ListControl::AverageWeight(float weight, TimeValue)
{
	const BOOL average = GetAverage();
	if (!average)
		return weight;

	if (_cached_total_weight_is_valid)
	{
		return _cached_total_weight != 0.0f ? (weight / _cached_total_weight) : 0.f;
	}

	_cached_total_weight = 0.0f;

	for (int i = 0; i < conts.Count(); i++)
	{
		float tempWeight = 0.0f;
		pblock->GetValue(kListCtrlWeight, 0, tempWeight, i);
		_cached_total_weight += tempWeight;
	}
	_cached_total_weight_is_valid = true;
	return _cached_total_weight != 0.0f ? (weight / _cached_total_weight) : 0.f;
}

BOOL ListControl::GetAverage() const
{
	BOOL average = FALSE;
	// Time of zero is fine because its not animatable.
	pblock->GetValue(kListCtrlAverage, 0, average);
	return average;
}

bool ListControl::IsIndexMode() const
{
	BOOL indexMode = FALSE;
	// Time of zero is fine because its not animatable.
	pblock->GetValue(kListCtrlIndexMode, 0, indexMode);
	return indexMode != 0;
}

void ListControl::SetIndexModeIndex(TimeValue t, int index)
{
	const int count = GetListCount();
	if (index >= count) {
		index = count - 1;
	}
	if (index < 0) {
		index = 0;
	}

	int indexModeIndex = 0;
	pblock->GetValue(kListCtrlIndex, t, indexModeIndex);
	if (indexModeIndex == index) {
		return;
	}

	// Index is different, set it as is, the accessor will clamp it to a valid index.
	theHold.Begin();
	pblock->SetValue(kListCtrlIndex, t, index);
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
	theHold.Accept(GetString(IDS_AF_LIST_PARAM_CHANGED_INDEX));
}

int ListControl::GetIndexModeIndex(TimeValue t, Interval& valid) const
{
	const int listCount = GetListCount();
	if (listCount == 0) {
		return 0; // The list can be empty sometimes
	}

	int index = 0;
	pblock->GetValue(kListCtrlIndex, t, index, valid);
	if(index >= listCount) {
		index = listCount - 1;
	}
	if(index < 0) {
		index = 0;
	}
	return index;
}

int ListControl::GetIndexModeIndex(TimeValue t) const
{
	Interval valid = FOREVER;
	return GetIndexModeIndex(t, valid);
}

//-------------------------------------------------------------------------
// List Control Qt UI
//

ListControlRollup::ListControlRollup(bool inCommandPanel, IObjParam* ip) :
	QMaxParamBlockWidget(),
	ReferenceMaker(),
	TimeChangeCallback(),
	mIObjParams(ip),
	mInCommandPanel(inCommandPanel),
	m_UI(new Ui::ListControlRollup())
{
	cont = nullptr;
	m_UI->setupUi(this);

	// Setup the UI labels
	m_UI->btnSetActiveBelow->setText(QString::fromStdWString(GetString(IDS_AF_LIST_SETACTIVE)));
	m_UI->btnCutBelow->setText(QString::fromStdWString(GetString(IDS_AF_LIST_CUT)));
	m_UI->btnDeleteBelow->setText(QString::fromStdWString(GetString(IDS_AF_LIST_DELETE)));
	m_UI->btnPasteBelow->setText(QString::fromStdWString(GetString(IDS_AF_LIST_PASTE)));
	m_UI->btnAppendBelow->setText(QString::fromStdWString(GetString(IDS_AF_LIST_APPEND)));
	m_UI->btnAssignBelow->setText(QString::fromStdWString(GetString(IDS_AF_LIST_ASSIGN)));
	m_UI->radioWeightMode->setText(QString::fromStdWString(GetString(IDS_AF_LIST_WEIGHT)));
	m_UI->radioIndexMode->setText(QString::fromStdWString(GetString(IDS_AF_LIST_INDEX)));
	m_UI->groupBoxWeight->setTitle(QString::fromStdWString(GetString(IDS_AF_LIST_WEIGHT)));
	m_UI->groupBoxIndex->setTitle(QString::fromStdWString(GetString(IDS_AF_LIST_INDEX)));
	m_UI->average->setText(QString::fromStdWString(GetString(IDS_AF_LIST_AVERAGE)));
	m_UI->labelIndex->setText(QString::fromStdWString(GetString(IDS_AF_LIST_INDEX)));
	m_UI->labelByName->setText(QString::fromStdWString(GetString(IDS_AF_LIST_INDEXBYNAME)));

	m_UI->groupBoxDependency->setTitle(QString::fromStdWString(GetString(IDS_AF_LIST_SEQUENTIAL_MODE)));
	m_UI->radioFan->setText(QString::fromStdWString(GetString(IDS_AF_LIST_FAN)));
	m_UI->radioChain->setText(QString::fromStdWString(GetString(IDS_AF_LIST_CHAIN)));

	m_UI->groupBoxWeightMethod->setTitle(QString::fromStdWString(GetString(IDS_AF_LIST_WEIGHT_METHOD)));
	m_UI->radioAgainstIdentity->setText(QString::fromStdWString(GetString(IDS_AF_LIST_AGAINST_IDENTITY)));
	m_UI->radioLerpPrevious->setText(QString::fromStdWString(GetString(IDS_AF_LIST_LERP_PREVIOUS)));

	// Setup Tooltips
	m_UI->tableView->setToolTip(QString::fromStdWString(GetString(IDS_AF_LIST_TT_TABLE)));
	m_UI->btnSetActiveBelow->setToolTip(QString::fromStdWString(GetString(IDS_AF_LIST_TT_SETACTIVE)));
	m_UI->btnDeleteBelow->setToolTip(QString::fromStdWString(GetString(IDS_AF_LIST_TT_DELETE)));
	m_UI->btnAppendBelow->setToolTip(QString::fromStdWString(GetString(IDS_AF_LIST_TT_APPEND)));
	m_UI->btnAssignBelow->setToolTip(QString::fromStdWString(GetString(IDS_AF_LIST_TT_ASSIGN)));
	m_UI->btnCutBelow->setToolTip(QString::fromStdWString(GetString(IDS_AF_LIST_TT_CUT)));
	m_UI->btnPasteBelow->setToolTip(QString::fromStdWString(GetString(IDS_AF_LIST_TT_PASTE)));
	m_UI->radioWeightMode->setToolTip(QString::fromStdWString(GetString(IDS_AF_LIST_TT_WEIGHTMODE)));
	m_UI->radioIndexMode->setToolTip(QString::fromStdWString(GetString(IDS_AF_LIST_TT_INDEXMODE)));
	//IDS_AF_LIST_TT_WEIGHTSPINNER
	m_UI->index->setToolTip(QString::fromStdWString(GetString(IDS_AF_LIST_TT_INDEXSPINNER)));
	m_UI->labelIndex->setToolTip(QString::fromStdWString(GetString(IDS_AF_LIST_TT_INDEXSPINNER)));
	m_UI->lineEditByName->setToolTip(QString::fromStdWString(GetString(IDS_AF_LIST_TT_BYNAME)));
	m_UI->labelByName->setToolTip(QString::fromStdWString(GetString(IDS_AF_LIST_TT_BYNAME)));
	//IDS_AF_LIST_TT_AVERAGE
	//IDS_AF_LIST_TT_AVERAGE_TM
	//IDS_AF_LIST_TT_AVERAGE_ROT
	m_UI->groupBoxDependency->setToolTip(QString::fromStdWString(GetString(IDS_AF_LIST_TT_SEQUENTIAL)));
	m_UI->radioFan->setToolTip(QString::fromStdWString(GetString(IDS_AF_LIST_TT_FAN)));
	m_UI->radioChain->setToolTip(QString::fromStdWString(GetString(IDS_AF_LIST_TT_CHAIN)));
	m_UI->groupBoxWeightMethod->setToolTip(QString::fromStdWString(GetString(IDS_AF_LIST_TT_WEIGHTMETHOD)));
	m_UI->radioAgainstIdentity->setToolTip(QString::fromStdWString(GetString(IDS_AF_LIST_TT_WEIGHTIDENTITY)));
	m_UI->radioLerpPrevious->setToolTip(QString::fromStdWString(GetString(IDS_AF_LIST_TT_WEIGHTLERP)));

	// Connections
	// Check boxes
	DbgVerify(QObject::connect(m_UI->average, &QCheckBox::stateChanged, this, &ListControlRollup::AverageStateChanged));

	// Set dependency mode radio buttons
	DbgVerify(QObject::connect(m_UI->radioFan, &QRadioButton::clicked, this, &ListControlRollup::RadioFanClicked));
	DbgVerify(QObject::connect(m_UI->radioChain, &QRadioButton::clicked, this, &ListControlRollup::RadioChainClicked));

	// Set Weighting Method
	DbgVerify(QObject::connect(m_UI->radioAgainstIdentity, &QRadioButton::clicked, this, &ListControlRollup::RadioAgainstIdentityClicked));
	DbgVerify(QObject::connect(m_UI->radioLerpPrevious, &QRadioButton::clicked, this, &ListControlRollup::RadioLerpPreviousClicked));

	// Set mode radio buttons
	DbgVerify(QObject::connect(m_UI->radioWeightMode, &QRadioButton::clicked, this, &ListControlRollup::RadioWeightModeClicked));
	DbgVerify(QObject::connect(m_UI->radioIndexMode, &QRadioButton::clicked, this, &ListControlRollup::RadioIndexModeClicked));

	// Double click to set active
	DbgVerify(QObject::connect(m_UI->tableView, &QTableView::doubleClicked, this, &ListControlRollup::TableDoubleClicked));
	// Table context menu request
	DbgVerify(QObject::connect(m_UI->tableView, &ListControlTableView::customContextMenuRequested, this, &ListControlRollup::TableContextMenu));
	
	// 4 buttons
	DbgVerify(QObject::connect(m_UI->btnSetActiveBelow, &QToolButton::clicked, this, &ListControlRollup::ButtonSetActiveClicked));
	DbgVerify(QObject::connect(m_UI->btnCutBelow, &QToolButton::clicked, this, &ListControlRollup::ButtonCutClicked));
	DbgVerify(QObject::connect(m_UI->btnPasteBelow, &QToolButton::clicked, this, &ListControlRollup::ButtonPasteClicked));
	DbgVerify(QObject::connect(m_UI->btnDeleteBelow, &QToolButton::clicked, this, &ListControlRollup::ButtonDeleteClicked));
	DbgVerify(QObject::connect(m_UI->btnAppendBelow, &QToolButton::clicked, this, &ListControlRollup::ButtonAppendClicked));
	DbgVerify(QObject::connect(m_UI->btnAssignBelow, &QToolButton::clicked, this, &ListControlRollup::ButtonAssignClicked));

	//Index spinner
	DbgVerify(QObject::connect(m_UI->index, qOverload<int>(&MaxSDK::QmaxSpinBox::valueChanged),
		this, &ListControlRollup::IndexSpinnerValueChanged));
	
	// Set index by name
	DbgVerify(QObject::connect(m_UI->lineEditByName, &QLineEdit::editingFinished, this, &ListControlRollup::ByNameIndexEditingFinished));

	GetCOREInterface()->RegisterTimeChangeCallback(this);
}

ListControlRollup::~ListControlRollup()
{
	// ParamMap2 should already be destroyed, just in case call again.
	DestroyParamMap();

	if(cont != nullptr)
	{
		if (mInCommandPanel == true)
		{
			const int activeIndex = cont->GetActive();
			EndEdit(activeIndex);
		}
		else
		{
			// Get the controller to clear its pointer to this floating dialog
			cont->ClearFloaterPointer();
		}
	}

	{
		HoldSuspend hs;
		ReplaceReference(LISTDLG_CONTREF, nullptr);
		GetCOREInterface()->UnRegisterTimeChangeCallback(this);
	}

	delete m_UI;
}

void ListControlRollup::DestroyParamMap()
{
	if (mParamMap != nullptr)
	{
		DestroyCPParamMap2(mParamMap);
		mParamMap = nullptr;
	}
}

void ListControlRollup::StartEdit(int startIndex, int prevIndex /*= -1*/) const
{
	// Make sure we have a motion panel rollout
	if (cont->mMotionRollout == nullptr) {
		return;
	}

	// Find the previous animatable if any
	Animatable* prev = nullptr;
	if(prevIndex >= 0 && prevIndex < cont->conts.Count() && cont->conts[prevIndex]) {
		prev = cont->conts[prevIndex];
	}

	if (startIndex >= 0 && startIndex < cont->conts.Count() && cont->conts[startIndex]) {
		cont->conts[startIndex]->BeginEditParams(GetObjectParams(), cont->paramFlags, prev);
	}
}

void ListControlRollup::EndEdit(int endIndex, int nextIndex /*= -1*/) const
{
	// Make sure we have a motion panel rollout
	if (cont->mMotionRollout == nullptr) {
		return;
	}

	// Find the next animatable if any
	Animatable* next = nullptr;
	if(nextIndex >= 0 && nextIndex < cont->conts.Count() && cont->conts[nextIndex]) {
		next = cont->conts[nextIndex];
	}

	if (endIndex >= 0 && endIndex < cont->conts.Count() && cont->conts[endIndex]) {
		cont->conts[endIndex]->EndEditParams(GetObjectParams(), END_EDIT_REMOVEUI, next);
	}
}

void ListControlRollup::SetParamBlock(ReferenceMaker* owner, IParamBlock2* const /*param_block*/)
{
	{
		// Floater does not get any magic and needs manual binding, subscribe to the messages
		HoldSuspend hs;
		ReplaceReference(LISTDLG_CONTREF, (ListControl*)owner);
		if (cont == nullptr) {
			return;
		}
	}

	// Hook up a parameter map so UI will update
	const int flags = ROLLUP_DONT_ADD_TO_CP;
	const MSTR title;
	int paramMapID = 0; // Floater param map id
	if (mInCommandPanel) {
		paramMapID = 1; // command panel param map id
	}
	// Should have index and average auto connected.
	IParamMap2* paramMap = CreateCPParamMap2(paramMapID, cont->GetParamBlock(0), GetCOREInterface(), *this, title, flags);

	// This widget owns param map and deletes it
	mParamMap = paramMap;	

	tableModel.reset(new ListControlTableModel(cont, m_UI->tableView));
	m_UI->tableView->setModel(tableModel.get());
	m_UI->tableView->setItemDelegate(new ListTableEntryEditDelegate(this, cont, m_UI->tableView));

	// Must be after the 'setModel' because the selection model gets reset internally.
	QItemSelectionModel* selMod = m_UI->tableView->selectionModel();
	DbgVerify(QObject::connect(selMod, &QItemSelectionModel::currentRowChanged,
		this, &ListControlRollup::TableRowChanged));
	m_UI->tableView->resizeColumnsToContents();

	// Make it so that the columns auto fit to the space. No more manually adjusting them.
	m_UI->tableView->horizontalHeader()->setSectionResizeMode(ListTableColumns::eSetActiveCol, QHeaderView::ResizeToContents);
	m_UI->tableView->horizontalHeader()->setSectionResizeMode(ListTableColumns::eNameCol, QHeaderView::Stretch);
	m_UI->tableView->horizontalHeader()->setSectionResizeMode(ListTableColumns::eWeightCol, QHeaderView::Fixed);
	

	// Setup the UI dependent on the type of list controller.
	if (cont->SuperClassID() != CTRL_MATRIX3_CLASS_ID) {
		// Remove tm list only parts
		m_UI->groupBoxDependency->setVisible(false);
		m_UI->radioFan->setVisible(false);
		m_UI->radioChain->setVisible(false);
		m_UI->groupBoxWeightMethod->setVisible(false);
		m_UI->radioAgainstIdentity->setVisible(false);
		m_UI->radioLerpPrevious->setVisible(false);

		//Not TM, tooltips
		m_UI->average->setToolTip(QString::fromStdWString(GetString(IDS_AF_LIST_TT_AVERAGE)));
	}
	else
	{
		//TM specific tooltips
		m_UI->average->setToolTip(QString::fromStdWString(GetString(IDS_AF_LIST_TT_AVERAGE_TM)));
	}

	if (cont->SuperClassID() == CTRL_ROTATION_CLASS_ID) {
		// Rotation list has a different label for the normalize button
		m_UI->average->setText(QString::fromStdWString(GetString(IDS_AF_LIST_POSE_TO_POSE)));
		m_UI->average->setToolTip(QString::fromStdWString(GetString(IDS_AF_LIST_TT_AVERAGE_ROT)));
	}

	// Sync UI controls right away from ParamBlock data, so that other BTT map option UI code 
	// like the NormalMap UI handling in BTTMainDialog works on the proper state. 
	paramMap->UpdateUI(GetCOREInterface()->GetTime());
}

void ListControlRollup::UpdateUI(const TimeValue t)
{
	if (cont == nullptr) {
		return;
	}
	
	// Manually update everything because there is no magic Qt binding to the paramblock for the floating dialog.
	UpdateUIPart(t, kListCtrlWeight);
	UpdateUIPart(t, kListCtrlAverage);
	UpdateUIPart(t, kListCtrlSequential);
	UpdateUIPart(t, kListCtrlIndexMode);
	UpdateUIPart(t, kListCtrlIndex);
	UpdateUIPart(t, kListCtrlWeightAgainstMode);
}

void ListControlRollup::UpdateUIPart(const TimeValue /*t*/, const ParamID param_id)
{
	if (cont == nullptr) {
		return;
	}

	const bool indexMode = cont->IsIndexMode() != 0; //Update the UI depending on the mode.

	//The floater UI doesn't connect the same way the rollout one does, manually update
	if (param_id == kListCtrlWeight)
	{
		InvalidateTableData();
	}
	else if (param_id == kListCtrlSequential)
	{
		if (indexMode == false) {
			const BOOL sequentialMode = cont->GetSequential();
			if(sequentialMode == false)
			{
				// fan mode
				m_UI->radioFan->setChecked(true);
				m_UI->radioChain->setChecked(false);
			}
			else
			{
				// chain mode
				m_UI->radioChain->setChecked(true);
				m_UI->radioFan->setChecked(false);
			}
		}
	}
	else if(param_id == kListCtrlWeightAgainstMode)
	{
		if (indexMode == false) {
			const bool weightAgainstMode = cont->GetWeightAgainstMode();
			if(weightAgainstMode == false)
			{
				// Against identity
				m_UI->radioAgainstIdentity->setChecked(true);
				m_UI->radioLerpPrevious->setChecked(false);
			}
			else
			{
				// LERP from previous
				m_UI->radioLerpPrevious->setChecked(true);
				m_UI->radioAgainstIdentity->setChecked(false);
			}
		}
	}
	else if (param_id == kListCtrlIndexMode)
	{
		const bool oldModelMode = tableModel->GetMode();

		// Detect when the mode changed and update the selected row to show the persistent spinner.
		if (oldModelMode != indexMode && indexMode)
		{
			// Going to index mode: before hiding the weight column below, remove persistent spinner
			const QModelIndex index = m_UI->tableView->selectionModel()->currentIndex();
			TableRowChanged(QModelIndex(), index);
		}

		// Always do this to help initialize the UI
		tableModel->SetMode(indexMode);
		
		// Table already invalidate above.
		if (indexMode == false) {
			m_UI->radioWeightMode->setChecked(true);
			m_UI->radioIndexMode->setChecked(false);

			m_UI->groupBoxIndex->setVisible(false);
			m_UI->tableView->verticalHeader()->setVisible(false);

			m_UI->groupBoxWeight->setVisible(true);
			m_UI->average->setVisible(true);
			if (cont->SuperClassID() == CTRL_MATRIX3_CLASS_ID)
			{
				// TM list has extra stuff in the UI
				m_UI->groupBoxDependency->setVisible(true);
				m_UI->radioFan->setVisible(true);
				m_UI->radioChain->setVisible(true);
				m_UI->groupBoxWeightMethod->setVisible(true);
				m_UI->radioAgainstIdentity->setVisible(true);
				m_UI->radioLerpPrevious->setVisible(true);
			}
		}
		else {
			m_UI->radioIndexMode->setChecked(true);
			m_UI->radioWeightMode->setChecked(false);

			m_UI->groupBoxIndex->setVisible(true);
			m_UI->tableView->verticalHeader()->setVisible(true);

			m_UI->groupBoxWeight->setVisible(false);
			m_UI->average->setVisible(false);
			if (cont->SuperClassID() == CTRL_MATRIX3_CLASS_ID)
			{
				// TM list has extra stuff in the UI
				m_UI->groupBoxDependency->setVisible(false);
				m_UI->radioFan->setVisible(false);
				m_UI->radioChain->setVisible(false);
				m_UI->groupBoxWeightMethod->setVisible(false);
				m_UI->radioAgainstIdentity->setVisible(false);
				m_UI->radioLerpPrevious->setVisible(false);
			}
		}

		// Detect when the mode changed and update the selected row to show the persistent spinner.
		if (oldModelMode != indexMode && !indexMode)
		{
			// Going to weight mode: after showing the weight column above, update the persistent spinner
			const QModelIndex index = m_UI->tableView->selectionModel()->currentIndex();
			TableRowChanged(index, QModelIndex());
		}
	}
	else if(param_id == kListCtrlIndex)
	{
		if (indexMode == true) {
			Interval valid = FOREVER;
			const int index = cont->GetIndexModeIndex(GetCOREInterface()->GetTime(), valid);
			
			// Make sure the index spinner shows the correct clamped value.
			// Needed because there is a custom accessor that doesn't reflect in the UI automatically.
			const int currentUiIndex = m_UI->index->value();
			// +1 because controller is 0-based, UI is 1-based.
			if(currentUiIndex != index + 1) {
				m_UI->index->setValue(index + 1);
			}
			
			// Populate the ByName line edit with the current active name
			const WStr indexName = cont->GetName(index);
			if(indexName.length() > 0) {
				m_UI->lineEditByName->setText(indexName);
			}
		}
	}
}

void ListControlRollup::InvalidateTableData() const
{
	// Force view to pull all new data from the model
	ListControlTableModel* model = dynamic_cast<ListControlTableModel*>(m_UI->tableView->model());
	if (model == nullptr)
	{
		DbgAssert(false);
		return;
	}

	model->InvalidateTableData();
	UpdateTableButtons();
}

void ListControlRollup::UpdateTableButtons() const
{
	// Don't use selectedRows() here because sometimes the entire row isn't selected even though that's our selection type.
	QModelIndex index = m_UI->tableView->selectionModel()->currentIndex();
	bool enabled = true;
	if (!index.isValid()) {
		enabled = false;
	}
	m_UI->btnCutBelow->setEnabled(enabled);
	m_UI->btnSetActiveBelow->setEnabled(enabled);
	m_UI->btnDeleteBelow->setEnabled(enabled);
	m_UI->btnAssignBelow->setEnabled(enabled);

	//Only enable the paste button if there is something to paste
	m_UI->btnPasteBelow->setEnabled(cont->clip != nullptr);
}

void ListControlRollup::UpdateParameterUI(const TimeValue t, const ParamID param_id, const int tab_index)
{
	if(cont == nullptr) {
		return;
	}
	UpdateUIPart(t, param_id);
}

RefResult ListControlRollup::NotifyRefChanged(
	const Interval& changeInt,
	RefTargetHandle hTarget,
	PartID& partID,
	RefMessage message,
	BOOL propagate)
{
	switch (message)
	{
	case REFMSG_CHANGE:
		UpdateUI(GetCOREInterface()->GetTime());
		break;

	case REFMSG_REF_DELETED:
	{
		// TODO: when the controller is removed from the scene, close the dialog. This is an existing bug.
		break;
	}
	case REFMSG_TARGET_DELETED:
		cont->CloseFloater();
		break;
	case REFMSG_NODE_NAMECHANGE:
		UpdateUIPart(GetCOREInterface()->GetTime(), kListCtrlWeight);
		break;
	case REFMSG_SUBANIM_STRUCTURE_CHANGED:
	case REFMSG_CONTROLREF_CHANGE:
		//Maybe the number of rows in the table changed
		if(tableModel != nullptr) {
			tableModel->InvalidateTableSize();
		}
		UpdateUIPart(GetCOREInterface()->GetTime(), kListCtrlWeight);

		// Refresh the weight spinner editor if there is one.
		const QModelIndex index = m_UI->tableView->selectionModel()->currentIndex();
		TableRowChanged(index, index);
		break;
	}
	return REF_SUCCEED;
}

int ListControlRollup::NumRefs()
{
	return 1; //reference to the owning controller.
}

RefTargetHandle ListControlRollup::GetReference(int i)
{
	switch (i)
	{
	case LISTDLG_CONTREF:
		return cont;
	default:
		return nullptr;
	}
}
void ListControlRollup::SetReference(int i, RefTargetHandle rtarg)
{
	switch (i)
	{
	case LISTDLG_CONTREF:
		cont = (ListControl*)rtarg;
		break;
	}
}

void ListControlRollup::TimeChanged(TimeValue t)
{
	//Update the animatable parts of the UI because the time changed.
	UpdateUIPart(t, kListCtrlWeight);
}

void ListControlRollup::AverageStateChanged(int /*state*/)
{
	if (cont == nullptr) {
		DbgAssert(false);
		return;
	}

	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
}

void ListControlRollup::RadioFanClicked(bool checked)
{
	// Sequential parameter
	if (checked == false || cont == nullptr)
		return;

	const BOOL currentSequential = cont->GetSequential();
	if (currentSequential == TRUE/*chain mode*/)
	{
		//switch to fan mode
		const BOOL newSequential = FALSE;
		theHold.Begin();
		cont->pblock->SetValue(kListCtrlSequential, 0, newSequential);
		cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
		theHold.Accept(GetString(IDS_AF_LIST_PARAM_CHANGED_SEQUENTIAL));
	}
}

void ListControlRollup::RadioChainClicked(bool checked)
{
	// Sequential parameter
	if (checked == false || cont == nullptr)
		return;

	const BOOL currentSequential = cont->GetSequential();
	if (currentSequential == FALSE/*fan mode*/)
	{
		//switch to chain mode
		const BOOL newSequential = TRUE;
		theHold.Begin();
		cont->pblock->SetValue(kListCtrlSequential, 0, newSequential);
		cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
		theHold.Accept(GetString(IDS_AF_LIST_PARAM_CHANGED_SEQUENTIAL));
	}
}

void ListControlRollup::RadioAgainstIdentityClicked(bool checked)
{
	// weight against method parameter
	if (checked == false || cont == nullptr)
		return;

	const bool currentWeightMode = cont->GetWeightAgainstMode();
	if (currentWeightMode == true/*LERP previous mode*/)
	{
		//switch to against identity mode
		const BOOL newWeightMode = FALSE;
		theHold.Begin();
		cont->pblock->SetValue(kListCtrlWeightAgainstMode, 0, newWeightMode);
		cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
		theHold.Accept(GetString(IDS_AF_LIST_PARAM_CHANGED_WEIGHT_MODE));
	}
}

void ListControlRollup::RadioLerpPreviousClicked(bool checked)
{
	// weight against method parameter
	if (checked == false || cont == nullptr)
		return;

	const bool currentWeightMode = cont->GetWeightAgainstMode();
	if (currentWeightMode == false/*against identity mode*/)
	{
		//switch to LERP previous mode
		const BOOL newWeightMode = TRUE;
		theHold.Begin();
		cont->pblock->SetValue(kListCtrlWeightAgainstMode, 0, newWeightMode);
		cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
		theHold.Accept(GetString(IDS_AF_LIST_PARAM_CHANGED_WEIGHT_MODE));
	}
}

void ListControlRollup::RadioWeightModeClicked(bool checked)
{
	if (checked == false || cont == nullptr)
		return;

	if(cont->IsIndexMode())
	{
		//switch to weight mode
		BOOL indexMode = FALSE;
		theHold.Begin();
		cont->pblock->SetValue(kListCtrlIndexMode, 0, indexMode);
		cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
		theHold.Accept(GetString(IDS_AF_LIST_PARAM_CHANGED_MODE));
	}
}

void ListControlRollup::RadioIndexModeClicked(bool checked)
{
	if (checked == false || cont == nullptr)
		return;

	if (!cont->IsIndexMode())
	{
		//switch to index mode
		BOOL indexMode = TRUE;
		theHold.Begin();
		cont->pblock->SetValue(kListCtrlIndexMode, 0, indexMode);
		cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
		theHold.Accept(GetString(IDS_AF_LIST_PARAM_CHANGED_MODE));
	}
}

void ListControlRollup::IndexSpinnerValueChanged(int index)
{
	if (cont == nullptr)
	{
		DbgAssert(false);
		return;
	}

	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
}

void ListControlRollup::TableDoubleClicked(const QModelIndex& index)
{
	if(cont == nullptr) {
		return;
	}
	// Set the row to the active control to the double clicked row
	const int row = index.row();
	if(row < 0 || row >= cont->GetListCount()) {
		return;
	}
	// Only take the double click on the active col.
	const int col = index.column();
	if(col != ListTableColumns::eSetActiveCol) {
		return;
	}
	
	if(cont->GetActive() == row)
	{
		// Open the track properties
		IObjParam* ip = GetObjectParams();
		if (ip == nullptr) {
			return;
		}
		const DWORD flags = EDITTRACK_BUTTON;
		// Do not use winid() here. It does not always return a handle that controllers can use as a parent. Using main window.
		HWND parentHandle = GetCOREInterface()->GetMAXHWnd();
		MaxSDK::OpenEditTrackParams({ cont->SubAnim(row), cont, row },
			GetCOREInterface()->GetTime(), parentHandle, ip, flags);
		return;
	}

	// Switching the active control
	if(cont->IsIndexMode()) {
		// Index mode, set the index
		cont->SetIndexModeIndex(GetCOREInterface()->GetTime(), row);
	}
	else {
		// Weight the weight mode index
		cont->SetWeightModeActive(row);
	}
}

void ListControlRollup::TableRowChanged(const QModelIndex& current, const QModelIndex& previous)
{
	UpdateTableButtons();

	if (previous.isValid())
	{
		QModelIndex editorIndex = tableModel->index(previous.row(), ListTableColumns::eWeightCol); // check the proper cell.
		if (m_UI->tableView->isPersistentEditorOpen(editorIndex))
		{
			m_UI->tableView->closePersistentEditor(editorIndex);
		}
	}

	if (current.isValid())
	{
		const int row = current.row();
		QModelIndex editorIndex = tableModel->index(row, ListTableColumns::eWeightCol);
		m_UI->tableView->openPersistentEditor(editorIndex);
	}
}

// Helper method to avoid copy paste.
int ListControlRollup::GetIndexFromSelectionModel() const
{
	if (cont == nullptr)
		return -1;

	// Selected row in the table.
	const QModelIndex index = m_UI->tableView->selectionModel()->currentIndex();
	if (!index.isValid()) {
		return -1;
	}
	const int row = index.row();
	if (row < 0 || row >= cont->GetListCount()) {
		return -1;
	}
	return row;
}

void ListControlRollup::ButtonSetActiveClicked(bool)
{
	const int index = GetIndexFromSelectionModel();
	if (index < 0)
		return;
	
	if(cont->IsIndexMode()) {
		// Index mode, set the index
		cont->SetIndexModeIndex(GetCOREInterface()->GetTime(), index);
	}
	else {
		// Weight mode, set the weight mode index
		cont->SetWeightModeActive(index);
	}
}

void ListControlRollup::ButtonCutClicked(bool)
{
	const int index = GetIndexFromSelectionModel();
	if (index < 0)
		return;
	
	cont->CutItem(index);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
}

void ListControlRollup::ButtonPasteClicked(bool)
{
	if (cont == nullptr)
		return;

	// Which row are we pasting into
	int index = index = cont->GetListCount(); //paste at the end of the list.;
	QModelIndex selectedIndex = m_UI->tableView->selectionModel()->currentIndex();
	if (selectedIndex.isValid()) {
		const int row = selectedIndex.row();
		if (row >= 0 || row <= cont->GetListCount()) {
			index = row; //got a good selected row.
		}
	}
	if (index < 0)
		return;
	
	cont->PasteItem(index);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
}

void ListControlRollup::ButtonAppendClicked(bool)
{
	cont->AppendItem();
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
}

void ListControlRollup::ButtonAssignClicked(bool)
{
	const int index = GetIndexFromSelectionModel();
	if (index < 0)
		return;
	cont->AssignItem(index, this);
}

void ListControlRollup::ButtonDeleteClicked(bool)
{
	const int index = GetIndexFromSelectionModel();
	if (index < 0)
		return;
	cont->DeleteItem(index);
	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
}

void ListControlRollup::ByNameIndexEditingFinished()
{
	if (cont == nullptr) {
		DbgAssert(false);
		return;
	}

	const std::wstring searchText = m_UI->lineEditByName->text().toStdWString();
	if (searchText.length() == 0) {
		return;
	}

	if(cont->SetIndexByName(searchText.data()))
	{
		// Clear the test to signify that a match was found and used
		m_UI->lineEditByName->setText(QString());
	}
}

void ListControlRollup::TableContextMenu(const QPoint& pos)
{
	if (cont == nullptr) {
		DbgAssert(false);
		return;
	}
	// Selected row in the table.
	const int index = GetIndexFromSelectionModel();
	if (index < 0 || index >= cont->GetListCount()) {
		return;
	}

	// When in weight mode, if we've right clicked the active item, close the sub rollouts just in case.
	// Need to check for mMotionRollout as that being non-null signals that the list controllers UI is in the
	// command panel, even if the driving UI for this call is in a floating dialog.
	const int activeIndex = cont->GetActive();
	bool beginNeeded = false;
	if (activeIndex == index && cont->mMotionRollout != nullptr) {
		cont->mMotionRollout->EndEdit(cont->GetActive());
		beginNeeded = true;
	}
	
	std::vector<MaxSDK::TreeEntry> entries{ {cont->conts[index], cont , index} };
	QMenu* menu = new QMenu(GetCOREInterface()->GetQmaxMainWindow());
	MaxSDK::PopulateControllerContextMenu(menu, this, entries, false, true);
	if (!menu->isEmpty()) {
		menu->setAttribute(Qt::WA_DeleteOnClose);
		// Use exec here so its a blocking call.
		menu->exec(m_UI->tableView->viewport()->mapToGlobal(pos));
	}
	else {
		delete menu;
	}

	if(beginNeeded) {
		cont->mMotionRollout->StartEdit(cont->GetActive());
	}
}

ListControlTableModel::ListControlTableModel(ListControl* ownerListControl, QObject* parent)
	: QAbstractTableModel(parent)
{
	DbgAssert(ownerListControl != nullptr);
	cont = ownerListControl;
}

int ListControlTableModel::rowCount(const QModelIndex& parent) const
{
	if (cont == nullptr) {
		DbgAssert(false);
		return 0;
	}
	return cont->conts.Count();
}
int ListControlTableModel::columnCount(const QModelIndex& parent) const
{
	if (cont == nullptr) {
		DbgAssert(false);
		return 0;
	}
	if(mIndexMode)
	{
		return ListTableColumns::eNameCol + 1;
	}
	return ListTableColumns::eWeightCol + 1;
}
QVariant ListControlTableModel::data(const QModelIndex& index, int role) const
{
	if (cont == nullptr) {
		DbgAssert(false);
		return QVariant();
	}

	QVariant v;
	if (role == Qt::DisplayRole)
	{
		if (index.column() == ListTableColumns::eNameCol) // Name column, return control name
		{
			std::wstring name = cont->GetName(index.row()).data();
			v.setValue(QString::fromStdWString(name));
		}
		else if (index.column() == ListTableColumns::eWeightCol)
		{
			// Weight column
			float weight = cont->GetSubCtrlWeight(index.row(), GetCOREInterface()->GetTime());
			//apply the pblock entry dim
			weight = stdPercentDim->Convert(weight);

			//formal the output
			std::wstringstream wstrStream;
			wstrStream << std::fixed << std::setprecision(3) << weight;
			v.setValue(QString::fromStdWString(wstrStream.str()));
		}
	}
	else if (role == Qt::ForegroundRole)
	{
		if (index.column() == ListTableColumns::eWeightCol)
		{
			const bool brackets = cont->GetSubCtrlWeightAtKey(index.row(), GetCOREInterface()->GetTime());
			if (brackets)
			{
				// TODO: KZ, red brackets when we can get access to that, for now just red text.
				// Text from color manager is too dark.
				QColor color(255, 0, 0);
				/*IColorManager* colorManager = GetColorManager();
				if(colorManager != nullptr)
				{
					const COLORREF animC = colorManager->GetColor(kAnimationKeyBrackets);
					color = QColor(GetRValue(animC), GetGValue(animC), GetBValue(animC));
				}*/
				v.setValue(QBrush(color));
			}
		}
	}
	else if(role == Qt::ToolTipRole)
	{
		if (index.column() == ListTableColumns::eSetActiveCol)
		{
			// Tooltip to help users understand what this col is all about
			v.setValue(QString::fromStdWString(GetString(IDS_AF_LIST_TT_ACTIVE_COL)));
		}
		if (index.column() == ListTableColumns::eNameCol)
		{
			// Display the name of the controller as the tooltip because its likely truncated.
			const int currentRow = index.row();
			std::wstring controlName = cont->GetName(index.row()).data();
			Control* subControl = cont->GetSubCtrl(currentRow);
			if(subControl != nullptr)
			{
				MSTR str;
				subControl->GetClassName(str);
				const std::wstring subControlName = str.data();
				if(subControlName.length() > 0 && controlName != subControlName) {
					controlName += L" : " + subControlName;
				}
			}

			std::wstring ttLabel = GetString(IDS_AF_LIST_TT_NAME_COL);
			std::wstring tooltip;
			tooltip += controlName + L"<br/>" + ttLabel;
			v.setValue(QString::fromStdWString(tooltip));
		}
		if (index.column() == ListTableColumns::eWeightCol)
		{
			// Tooltip to help users understand what this col is all about
			v.setValue(QString::fromStdWString(GetString(IDS_AF_LIST_TT_WEIGHTSPINNER)));
		}
	}
	else if (role == Qt::DecorationRole)
	{
		if (index.column() == ListTableColumns::eSetActiveCol) // Active column, return indicator
		{
			if(cont->GetActive() == index.row())
			{
				const QIcon icon = MaxSDK::LoadMaxMultiResIcon("MainUI/ListActiveSub.svg");
				v.setValue(icon);
			}
		}
	}
	else if (role == Qt::FontRole)
	{
		if (index.column() == ListTableColumns::eNameCol)
		{
			const int currentRow = index.row();
			Control* subControl = cont->GetSubCtrl(currentRow);
			if (subControl != nullptr && GetRefHierarchyInterface()->IsRefTargetInstanced(subControl))
			{
				QFont font;
				font.setBold(true); // This entry is instanced, make it bold
				v.setValue(font);
			}
		}
	}
	return v;
}

Qt::ItemFlags ListControlTableModel::flags(const QModelIndex& index) const
{
	if (cont == nullptr) {
		return Qt::ItemFlag::NoItemFlags;
	}
	// Editable flag allows for the edit delegate
	Qt::ItemFlags flags = Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEditable;
	return flags;
}

bool ListControlTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if (cont == nullptr) {
		return false;
	}
	// Wet active col not editable.
	if (index.column() == ListTableColumns::eNameCol)
	{
		std::wstring name = value.toString().toStdWString();
		cont->SetName(index.row(), name.data());
		return true;
	}
	// Weight col dealt with my the spinner bound to the paramblock.
	return false;
}

QVariant ListControlTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	QVariant v;
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		std::wstring headerName;
		if (section == ListTableColumns::eSetActiveCol) {
			headerName = GetString(IDS_AF_LIST_ACTIVE_COL);
		}
		if(section == ListTableColumns::eNameCol) {
			headerName = GetString(IDS_AF_LIST_ENTRY);
		}
		else if (section == ListTableColumns::eWeightCol) {
			headerName = GetString(IDS_AF_LIST_WEIGHT);
		}
		v.setValue(QString::fromStdWString(headerName));
	}
	else if(orientation == Qt::Vertical && role == Qt::DisplayRole)
	{
		// Display line numbers that correspond with the indexes
		// Make the indexes start at 1 in the UI, +1;
		v.setValue(QString::number(section + 1));
	}
	return v;
}

void ListControlTableModel::InvalidateTableData()
{
	// Invalidate the table data, resize done in using "InvalidateTableSize()"
	const int listRowCount = cont->GetListCount();

	const QModelIndex index = this->index(0, 0);
	const QModelIndex index2 = this->index(listRowCount - 1, ListTableColumns::eWeightCol);
	QVector<int> roles{ Qt::DisplayRole, Qt::ForegroundRole, Qt::DecorationRole };
	Q_EMIT dataChanged(index, index2, roles);
}

void ListControlTableModel::InvalidateTableSize()
{
	Q_EMIT layoutChanged();
}

void ListControlTableModel::SetMode(bool indexMode)
{
	if(mIndexMode == indexMode) {
		return;
	}
	// We are switching modes
	mIndexMode = indexMode;

	// Update the entire table
	Q_EMIT layoutChanged();
}

ListControlTableView::ListControlTableView(QWidget* parent)
	:QTableView(parent)
{
}

//Override this to help it fit into the command panel rollout
QSize ListControlTableView::minimumSizeHint() const
{
	return sizeHint();
}
QSize ListControlTableView::sizeHint() const
{
	return QSize(MaxSDK::UIScaled(100), MaxSDK::UIScaled(100));
}

void ListControlTableView::resizeEvent(QResizeEvent* event)
{
	QAbstractItemModel* tableModel = model();
	if(tableModel != nullptr && tableModel->columnCount() >= ListTableColumns::eWeightCol)
	{
		// make the weight column stay a certain % of the width
		setColumnWidth(ListTableColumns::eWeightCol, this->width() * 30 / 100);
	}

	__super::resizeEvent(event);
}

ListTableEntryEditDelegate::ListTableEntryEditDelegate(ListControlRollup* rollout, ListControl* owner, QObject* parent)
	: QStyledItemDelegate(parent), cont(owner), mRollout(rollout)
{
	DbgAssert(cont);
	DbgAssert(mRollout);
}

QWidget* ListTableEntryEditDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	if (index.column() == ListTableColumns::eNameCol)
	{
		// Editing the controller name
		QLineEdit* editor = new QLineEdit();
		editor->setParent(parent);
		editor->setFrame(false);
		return editor;
	}
	if(index.column() == ListTableColumns::eWeightCol)
	{
		//Editing a weight
		mRollout->weightSpinner = new MaxSDK::QmaxDoubleSpinBox(parent);
		mRollout->weightSpinner->setFrame(false);
		mRollout->weightSpinner->setToolTip(QString::fromStdWString(GetString(IDS_AF_LIST_TT_WEIGHTSPINNER)));

		IParamMap2* pm = mRollout->GetParamMap();
		if(pm != nullptr) {
			pm->ConnectUI(kListCtrlWeight, mRollout->weightSpinner, index.row());
		}
		else {
			DbgAssert(false);
		}

		//Connect to the control
		DbgVerify(QObject::connect(mRollout->weightSpinner, qOverload<double>(&MaxSDK::QmaxDoubleSpinBox::valueChanged),
			this, &ListTableEntryEditDelegate::WeightSpinnerValueChanged));

		return mRollout->weightSpinner;
	}
	return nullptr;
}

void ListTableEntryEditDelegate::destroyEditor(QWidget* editor, const QModelIndex& index) const
{
	if(index.column() == ListTableColumns::eWeightCol && mRollout->weightSpinner == editor)
	{
		IParamMap2* pm = mRollout->GetParamMap();
		if (pm != nullptr) {
			pm->DisconnectUI(mRollout->weightSpinner);
		}
		else {
			DbgAssert(false);
		}
		DbgVerify(QObject::disconnect(mRollout->weightSpinner, qOverload<double>(&MaxSDK::QmaxDoubleSpinBox::valueChanged),
			this, &ListTableEntryEditDelegate::WeightSpinnerValueChanged));
		mRollout->weightSpinner = nullptr;
	}
	__super::destroyEditor(editor, index);
}

//set the value if the editor delegate
void ListTableEntryEditDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	if (index.column() == ListTableColumns::eNameCol)
	{
		// Editing the controller name
		QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(editor);
		if (lineEdit != nullptr)
		{
			const WStr name = cont->GetName(index.row());
			lineEdit->setText(QString::fromStdWString(name.data()));
		}
	}
	// No need to set data for col weight col because its connected to the pblock.
}

//set data from editor into the model
void ListTableEntryEditDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
	//disconnect
	if (index.column() == ListTableColumns::eNameCol)
	{
		QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(editor);
		const WStr name = lineEdit->text().toStdWString().data();
		cont->SetName(index.row(), name);
	}
	// No need to set data for weight col because its connected to the pblock.
}

void ListTableEntryEditDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	editor->setGeometry(option.rect);
}

void ListTableEntryEditDelegate::WeightSpinnerValueChanged(double value) const
{
	if(cont == nullptr || mRollout == nullptr)
	{
		DbgAssert(false);
		return;
	}

	GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
}

//-------------------------------------------------------------------------
// List control Class Desc base class
//

class ListUIClassDesc : public ClassDesc2
{
public:
	virtual int IsPublic() override { return 1; }
	virtual const TCHAR* Category() final { return _T(""); }
	virtual HINSTANCE HInstance() final {
		return hInstance; // returns owning module handle
	}
};


//-------------------------------------------------------------------------
// Float list control
//

class FloatListControl : public ListControl
{
public:
	FloatListControl(BOOL loading)
			: ListControl(loading)
	{
		Init();
	}
	FloatListControl()
	{
		Init();
	}

	void GetValue(TimeValue t, void* val, Interval& valid, GetSetMethod method = CTRL_ABSOLUTE) override;
	void SetValue(TimeValue t, void* val, int commit, GetSetMethod method) override;
	ListControl* DerivedClone() override;
	virtual ClassDesc* GetClassDesc() override;
	void Init();

	Class_ID ClassID() override
	{
		return Class_ID(FLOATLIST_CONTROL_CLASS_ID, 0);
	}
	SClass_ID SuperClassID() override
	{
		return CTRL_FLOAT_CLASS_ID;
	}
	void GetClassName(MSTR& s, bool localized) const override
	{
		s = localized ? FLOATLIST_CNAME : _T("Float List");
	}
};

void FloatListControl::GetValue(TimeValue t, void* val, Interval& valid, GetSetMethod method)
{
	float* v = (float*)val;
	float weight;

	if (method == CTRL_ABSOLUTE)
	{
		*v = 0.0f;
	}

	// CAL-10/18/2002: conts.Count() and pblock->Count(kListCtrlWeight) might go out of sync while
	// undo. (459225)
	if (conts.Count() != pblock->Count(kListCtrlWeight))
		return;

	const BOOL indexMode = IsIndexMode();
	if(indexMode)
	{
		// In this mode we return only the selected index
		const int index = GetIndexModeIndex(t, valid);
		if(index >= 0 && index < conts.Count() && conts[index])
			conts[index]->GetValue(t, v, valid, CTRL_RELATIVE);
	}
	else
	{
		float prevVal;
		for (int i = 0; i < conts.Count(); i++)
		{
			pblock->GetValue(kListCtrlWeight, t, weight, valid, i);
			if (weight != 0.0f)
			{
				prevVal = *v;
				if (conts[i])
					conts[i]->GetValue(t, v, valid, CTRL_RELATIVE);
				*v = prevVal + ((*v) - prevVal) * AverageWeight(weight, t);
			}
		}
	}
}

void FloatListControl::SetValue(TimeValue t, void* val, int commit, GetSetMethod method)
{
	if (GetLocked())
		return;
	if (!conts.Count())
		return;
	
	const BOOL indexMode = IsIndexMode();
	if(indexMode)
	{
		// Do nothing but pass on the set value
		const int index = GetIndexModeIndex(t);
		if (index >= 0 && index < conts.Count() && conts[index]) {
			conts[index]->SetValue(t, val, commit, method);
		}
		return;
	}

	float weight;
	const int activeIndex = GetActive();
	if(activeIndex < 0 || activeIndex >= conts.Count()) {
		return;
	}
	if (method == CTRL_ABSOLUTE)
	{
		float v = *((float*)val);
		pblock->GetValue(kListCtrlWeight, t, weight, activeIndex);
		v *= weight;
		float before = 0.0f, after = 0.0f;
		Interval valid;
		float prevVal;
		for (int i = 0; i < activeIndex; i++)
		{
			prevVal = before;
			if (conts[i])
				conts[i]->GetValue(t, &before, valid, CTRL_RELATIVE);
			pblock->GetValue(kListCtrlWeight, t, weight, i);
			before = prevVal + (before - prevVal) * AverageWeight(weight, t);
			//	conts[i]->GetValue(t,&before,valid,CTRL_RELATIVE);
		}
		for (int i = activeIndex + 1; i < conts.Count(); i++)
		{
			prevVal = after;
			if (conts[i])
				conts[i]->GetValue(t, &after, valid, CTRL_RELATIVE);
			pblock->GetValue(kListCtrlWeight, t, weight, i);
			after = prevVal + (after - prevVal) * AverageWeight(weight, t);
			//	conts[i]->GetValue(t,&after,valid,CTRL_RELATIVE);
		}
		v = -before + v + -after;
		assert(activeIndex >= 0);
		if (conts[activeIndex])
			conts[activeIndex]->SetValue(t, &v, commit, method);
	}
	else
	{
		assert(activeIndex >= 0);
		pblock->GetValue(kListCtrlWeight, t, weight, activeIndex);
		*((float*)val) *= weight;
		if (conts[activeIndex])
			conts[activeIndex]->SetValue(t, val, commit, method);
	}
}

ListControl* FloatListControl::DerivedClone()
{
	return new FloatListControl;
}

class FloatListClassDesc : public ListUIClassDesc
{
public:
	const TCHAR* ClassName() override
	{
		return FLOATLIST_CNAME;
	}
	const TCHAR* NonLocalizedClassName() override
	{ 
		return _T("Float List"); 
	}
	SClass_ID SuperClassID() override
	{
		return CTRL_FLOAT_CLASS_ID;
	}
	Class_ID ClassID() override
	{
		return Class_ID(FLOATLIST_CONTROL_CLASS_ID, 0);
	}
	void* Create(BOOL loading) override
	{
		return new FloatListControl(loading);
	}
	const TCHAR* InternalName() override
	{
		return _T("FloatList");
	} // returns fixed parsable name (scripter-visible name)
};
static FloatListClassDesc floatListCD;
ClassDesc* GetFloatListDesc()
{
	return &floatListCD;
}

void FloatListControl::Init()
{
	// make the paramblock
	floatListCD.MakeAutoParamBlocks(this);
}

ClassDesc* FloatListControl::GetClassDesc()
{
	return GetFloatListDesc();
}

// CAL-10/7/2002: add a PBAccessor for List Controller to return a local name for the weight tracks
// list controller PBAccessor
class ListCtrlPBAccessor : public PBAccessor
{
public:
	TSTR GetLocalName(ReferenceMaker* owner, ParamID id, int tabIndex, bool /*localized*/)
	{
		ListControl* ctrl = (ListControl*)owner;
		TSTR name;
		if (id == kListCtrlWeight)
		{
			// CAL-11/12/2002: Check if the ctrl(owner) is NULL. This is possible for scenes created
			//		before A019, because of the problem reported in defect #463222. (467256)
			if (ctrl)
				name.printf(_T("%s: %s"), GetString(IDS_AF_LIST_WEIGHT), ctrl->SubAnimName(tabIndex, true));
			else
				name.printf(_T("%s"), GetString(IDS_AF_LIST_WEIGHT));
		}
		return name;
	}
};
static ListCtrlPBAccessor theListCtrlPBAccessor;

class IndexDimension : public ParamDimension {
public:
	DimType DimensionType() { return DIM_CUSTOM; }
	float Convert(float value)  override
	{
		return value + 1.f;
	}
	float UnConvert(float value) override
	{
		return value - 1.f;
	}
};
static IndexDimension theIndexDim;

// Purpose of this is to react when the index changes
class ListCtrlIndexPBAccessor : public PBAccessor
{
	// Make sure the index is clampped to a valid range in the list of controllers
	// Before the value is set (so we can catch the case) update the edit params rollouts if it changed.
	void PreSet(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t) override
	{
		ListControl* cont = (ListControl*)owner;
		if (cont == nullptr)
		{
			DbgAssert(false);
			return;
		}
		if (id != kListCtrlIndex) {
			return;
		}

		// Limit the index to a valid entry
		int newIndex = v.i;
		const int subControlCount = cont->GetListCount();
		if (subControlCount == 0) {
			v.i = 0; // We have no entries, use default value
		}
		if (newIndex >= subControlCount) {
			newIndex = subControlCount - 1;
		}
		if (newIndex < 0) {
			newIndex = 0; //default value
		}
		v.i = newIndex;

		// Did the index change?
		// If yes and
		// we are in motion panel and
		// we are in index mode,
		// Then BeginEditParams on that one.
		if (cont->mMotionRollout)
		{
			// We are editing in the motion panel
			const int currentActiveIndex = cont->GetActive();
			// Have we got a different indexes and are we in index mode
			if (currentActiveIndex != newIndex && cont->IsIndexMode())
			{
				// This means we need to switch which controller is getting edited in the motion panel
				cont->mMotionRollout->EndEdit(currentActiveIndex, newIndex);
				cont->mMotionRollout->StartEdit(newIndex, currentActiveIndex);
			}
		}
	}
};
static ListCtrlIndexPBAccessor theListCtrlIndexPBAccessor;

// Purpose of this accessor is to react when the mode is changing
class theListCtrlIndexModePBAccessor : public PBAccessor
{
	//Before the value is set, using the old and new mode, handle the edit params
	void PreSet(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t) override
	{
		ListControl* cont = (ListControl*)owner;
		if (cont == nullptr)
		{
			DbgAssert(false);
			return;
		}
		if (id != kListCtrlIndexMode) {
			return;
		}

		const bool newMode = v.i;

		// Did the index mode change?
		// If yes and
		// we are in motion panel and
		// the active index is different between weight mode and index mode,
		// Then BeginEditParams on that one.
		if (cont->mMotionRollout)
		{
			// We are editing in the motion panel
			const bool oldMode = cont->IsIndexMode();
			if(oldMode != newMode)
			{
				//The modes changed
				int oldActiveIndex;
				int newActiveIndex;
				if(oldMode == true)
				{
					// We were in index mode
					oldActiveIndex = cont->GetIndexModeIndex(t);
					// New index will come from weight modes active index
					newActiveIndex = cont->GetWeightModeActive();
				}
				else
				{
					// We were in weight mode
					oldActiveIndex = cont->GetWeightModeActive();
					// New index will come from index modes index
					newActiveIndex = cont->GetIndexModeIndex(t);
				}

				// Are they different?
				if(oldActiveIndex != newActiveIndex)
				{
					// This means we need to switch which controller is getting edited in the motion panel
					cont->mMotionRollout->EndEdit(oldActiveIndex, newActiveIndex);
					cont->mMotionRollout->StartEdit(newActiveIndex, oldActiveIndex);
				}
			}
		}
	}
};
static theListCtrlIndexModePBAccessor thetheListCtrlIndexModePBAccessor;


// clang-format off
// per instance list controller block
static ParamBlockDesc2 list_paramblk (kListCtrlWeightParams, _T("Parameters"),  0, &floatListCD, P_AUTO_CONSTRUCT, PBLOCK_INIT_REF_ID,
	// params
	kListCtrlWeight, _T("weight"), TYPE_FLOAT_TAB, 0, P_ANIMATABLE+P_VARIABLE_SIZE+P_TV_SHOW_ALL+P_COMPUTED_NAME,  IDS_AF_LIST_WEIGHT, 
		p_default, 		1.0, 
		p_range,		WEIGHT_MIN, WEIGHT_MAX,
		p_dim,			stdPercentDim,
		p_accessor,		&theListCtrlPBAccessor,
		p_end,
		
	kListCtrlAverage, _T("average"), TYPE_BOOL, 0, IDS_AF_LIST_AVERAGE, 
		p_default, 		FALSE, 
		p_end,

	kListCtrlIndex, _T("index"), TYPE_INT, P_RESET_DEFAULT, IDS_AF_LIST_INDEX,
		p_default, 0,
		p_range, 0, 1000,
		p_dim, &theIndexDim,
		p_accessor, &theListCtrlIndexPBAccessor, //use accessor to limit to weight table size.
	p_end,

	kListCtrlIndexMode, _T("indexMode"), TYPE_BOOL, 0, IDS_AF_LIST_INDEX_MODE,
		p_default,	FALSE,
		p_accessor,	& thetheListCtrlIndexModePBAccessor, //use accessor to manage edit params if that changes.
		p_end,
	kListCtrlTag, _T("tag"), TYPE_STRING, 0, IDS_AF_LIST_TAG,
		p_default, _T(""),
		p_end,
	p_end
	);

static FPInterfaceDesc listControlInterface ( LIST_CONTROLLER_INTERFACE, _T("list"), 0, &floatListCD, FP_MIXIN,
		IListControl::list_getNumItems,		_T("getCount"),		0, TYPE_INT,	0,	0,
		IListControl::list_setActive,		_T("setActive"),	0, TYPE_VOID,	0,	1,
			_T("listIndex"), 0, TYPE_INDEX,
		IListControl::list_getActive,		_T("getActive"),	0, TYPE_INT,	0,	0,
		IListControl::list_deleteItem,		_T("delete"),		0, TYPE_VOID,	0,	1,
			_T("listIndex"), 0, TYPE_INDEX,
		IListControl::list_cutItem,			_T("cut"),			0, TYPE_VOID,	0,	1,
			_T("listIndex"), 0, TYPE_INDEX,
		IListControl::list_pasteItem,		_T("paste"),		0, TYPE_VOID,	0,	1,
			_T("listIndex"), 0, TYPE_INDEX,
		IListControl::list_getName,			_T("getName"),		0, TYPE_TSTR_BV, 0,  1,
			_T("listIndex"), 0, TYPE_INDEX,
		IListControl::list_setName,			_T("setName"),		0, TYPE_VOID,   0,  2,
			_T("listIndex"), 0, TYPE_INDEX,
			_T("name"),		 0, TYPE_STRING,

	    IListControl::list_getSubCtrl,			_T("getSubCtrl"),		0, TYPE_CONTROL,	0,	1,
			_T("index"), 0, TYPE_INDEX,
		IListControl::list_getSubCtrlWeight,	_T("getSubCtrlWeight"),	0, TYPE_FLOAT,	0,	1,
			_T("index"), 0, TYPE_INDEX,

		IListControl::list_setIndexByName, _T("setIndexByName"), 0, TYPE_BOOL, 0, 1,
		_T("entryNameMatch"), 0, TYPE_STRING,

		properties,
		IListControl::list_count, FP_NO_FUNCTION, _T("count"), 0, TYPE_INT,
		IListControl::list_getActive_prop, IListControl::list_setActive_prop, _T("active"), 0, TYPE_INDEX,
		p_end
);
// clang-format on


FPInterfaceDesc* IListControl::GetDesc()
{
	return &listControlInterface;
	// return NULL;
}


//-------------------------------------------------------------------------
// Point3 list control
//

class Point3ListControl : public ListControl
{
public:
	Point3ListControl(BOOL loading)
			: ListControl(loading)
	{
		Init();
	}
	Point3ListControl()
	{
		Init();
	}

	void GetValue(TimeValue t, void* val, Interval& valid, GetSetMethod method = CTRL_ABSOLUTE) override;
	void SetValue(TimeValue t, void* val, int commit, GetSetMethod method) override;
	ListControl* DerivedClone() override;
	virtual ClassDesc* GetClassDesc() override;
	void Init();

	Class_ID ClassID() override
	{
		return Class_ID(POINT3LIST_CONTROL_CLASS_ID, 0);
	}
	SClass_ID SuperClassID() override
	{
		return CTRL_POINT3_CLASS_ID;
	}
	void GetClassName(MSTR& s, bool localized) const override
	{
		s = localized ? POINT3LIST_CNAME : _T("Point3 List");
	}
};

void Point3ListControl::GetValue(TimeValue t, void* val, Interval& valid, GetSetMethod method)
{
	Point3* v = (Point3*)val;
	float weight;

	if (method == CTRL_ABSOLUTE)
	{
		*v = Point3(0, 0, 0);
	}

	const BOOL indexMode = IsIndexMode();
	if (indexMode)
	{
		// In this mode we return only the selected index
		const int index = GetIndexModeIndex(t, valid);
		if(index >= 0 && index < conts.Count() && conts[index])
			conts[index]->GetValue(t, v, valid, CTRL_RELATIVE);
	}
	else
	{
		Point3 prevVal;
		for (int i = 0; i < conts.Count(); i++)
		{
			pblock->GetValue(kListCtrlWeight, t, weight, valid, i);
			if (weight != 0.0f)
			{
				prevVal = *v;
				if (conts[i])
					conts[i]->GetValue(t, v, valid, CTRL_RELATIVE);
				*v = prevVal + ((*v) - prevVal) * AverageWeight(weight, t);
			}
		}
	}
}

void Point3ListControl::SetValue(TimeValue t, void* val, int commit, GetSetMethod method)
{
	if (GetLocked())
		return;
	if (!conts.Count())
		return;

	const BOOL indexMode = IsIndexMode();
	if(indexMode)
	{
		const int index = GetIndexModeIndex(t);
		if (index >= 0 && index < conts.Count() && conts[index]) {
			conts[index]->SetValue(t, val, commit, method);
		}
		return;
	}

	float weight;
	const int activeIndex = GetWeightModeActive();
	if (activeIndex < 0 || activeIndex >= conts.Count())
		return;
	if (method == CTRL_ABSOLUTE)
	{
		Point3 v = *((Point3*)val);
		Point3 before(0, 0, 0), after(0, 0, 0);
		Interval valid;
		Point3 prevVal;
		for (int i = 0; i < activeIndex; i++)
		{
			prevVal = before;
			if (conts[i])
				conts[i]->GetValue(t, &before, valid, CTRL_RELATIVE);
			pblock->GetValue(kListCtrlWeight, t, weight, i);
			before = prevVal + (before - prevVal) * AverageWeight(weight, t);
			// conts[i]->GetValue(t,&before,valid,CTRL_RELATIVE);
		}
		for (int i = activeIndex + 1; i < conts.Count(); i++)
		{
			prevVal = after;
			if (conts[i])
				conts[i]->GetValue(t, &after, valid, CTRL_RELATIVE);
			pblock->GetValue(kListCtrlWeight, t, weight, i);
			after = prevVal + (after - prevVal) * AverageWeight(weight, t);
			// conts[i]->GetValue(t,&after,valid,CTRL_RELATIVE);
		}
		v = -before + v + -after;
		if (conts[activeIndex])
			conts[activeIndex]->SetValue(t, &v, commit, method);
	}
	else
	{
		pblock->GetValue(kListCtrlWeight, t, weight, activeIndex);
		*((Point3*)val) = *((Point3*)val) * AverageWeight(weight, t);
		if (conts[activeIndex])
			conts[activeIndex]->SetValue(t, val, commit, method);
	}
}

ListControl* Point3ListControl::DerivedClone()
{
	return new Point3ListControl;
}

// keeps track of whether an FP interface desc has been added to the ClassDesc
static bool p3ListInterfaceLoaded = false;

class Point3ListClassDesc : public ListUIClassDesc
{
public:
	const TCHAR* ClassName() override
	{
		return POINT3LIST_CNAME;
	}
	const TCHAR* NonLocalizedClassName() override
	{
		return _T("Point3 List");
	}
	SClass_ID SuperClassID() override
	{
		return CTRL_POINT3_CLASS_ID;
	}
	Class_ID ClassID() override
	{
		return Class_ID(POINT3LIST_CONTROL_CLASS_ID, 0);
	}
	void* Create(BOOL loading) override
	{
		if (!p3ListInterfaceLoaded)
		{
			AddInterface(&listControlInterface);
			p3ListInterfaceLoaded = true;
		}
		return new Point3ListControl(loading);
	}

	const TCHAR* InternalName() override
	{
		return _T("Point3List");
	} // returns fixed parsable name (scripter-visible name)
};
static Point3ListClassDesc point3ListCD;
ClassDesc* GetPoint3ListDesc()
{
	return &point3ListCD;
}

void Point3ListControl::Init()
{
	// make the paramblock
	point3ListCD.MakeAutoParamBlocks(this);
}

ClassDesc* Point3ListControl::GetClassDesc()
{
	return GetPoint3ListDesc();
}

// CAL-10/30/2002: Add individual ParamBlockDesc2 for each list controller (463222)
//		When the pblock is cloned in ListControl::Clone(), the owner of the new pblock isn't set to the
//		new list controller after ctrl->ReplaceReference() is called.
//		Normally when ReplaceReference() is called with pblock, the owner of the pblock will be set
//		by ParamBlock2::RefAdded() if the rmaker's ClassID and the ParamBlockDesc2->ClassDescriptor's
//		ClassID are the same. However, all list controllers use one ParamBlockDesc2, list_paramblk,
//		which has a class descriptor, floatListCD. Therefore, the owner of the new pblock will remain
//		NULL if 'ctrl' is not a FloatListControl. In max, it is assumed that a ParamBlockDesc2 belongs
//		to a single ClassDesc. The list controllers share one ParamBlockDesc2 in many Classes and
//		unfortunately max's architecture isn't quite ready for that yet.
//		Fix it by adding individual ParamBlockDesc2 for each list controller and remove the call of
//		AddParamBlockDesc() from the ClassDesc2::Create() method of these classes.

// clang-format off
// per instance list controller block
static ParamBlockDesc2 point3_list_paramblk (kListCtrlWeightParams, _T("Parameters"),  0, &point3ListCD, P_AUTO_CONSTRUCT, PBLOCK_INIT_REF_ID,
	// params
	kListCtrlWeight, _T("weight"), TYPE_FLOAT_TAB, 0, P_ANIMATABLE+P_VARIABLE_SIZE+P_TV_SHOW_ALL+P_COMPUTED_NAME,  IDS_AF_LIST_WEIGHT, 
		p_default, 		1.0, 
		p_range,		WEIGHT_MIN, WEIGHT_MAX,
		p_dim,			stdPercentDim,
		p_accessor,		&theListCtrlPBAccessor,
		p_end,
		
	kListCtrlAverage, _T("average"), TYPE_BOOL, 0, IDS_AF_LIST_AVERAGE, 
		p_default, 		FALSE, 
		p_end,

	kListCtrlIndex, _T("index"), TYPE_INT, P_RESET_DEFAULT, IDS_AF_LIST_INDEX,
		p_default, 0,
		p_range, 0, 1000, //use accessor to limit to weight table size.
		p_dim, &theIndexDim,
		p_accessor, & theListCtrlIndexPBAccessor,
		p_end,

	kListCtrlIndexMode, _T("indexMode"), TYPE_BOOL, 0, IDS_AF_LIST_INDEX_MODE,
		p_default,		FALSE,
		p_accessor, & thetheListCtrlIndexModePBAccessor, //use accessor to manage edit params if that changes.
		p_end,
	kListCtrlTag, _T("tag"), TYPE_STRING, 0, IDS_AF_LIST_TAG,
		p_default, _T(""),
		p_end,
	p_end
);

// clang-format on

//-------------------------------------------------------------------------
// Point4 list control
//

class Point4ListControl : public ListControl
{
public:
	Point4ListControl(BOOL loading)
			: ListControl(loading)
	{
		Init();
	}
	Point4ListControl()
	{
		Init();
	}

	void GetValue(TimeValue t, void* val, Interval& valid, GetSetMethod method = CTRL_ABSOLUTE) override;
	void SetValue(TimeValue t, void* val, int commit, GetSetMethod method) override;
	ListControl* DerivedClone() override;
	virtual ClassDesc* GetClassDesc() override;
	void Init();

	Class_ID ClassID() override
	{
		return Class_ID(POINT4LIST_CONTROL_CLASS_ID, 0);
	}
	SClass_ID SuperClassID() override
	{
		return CTRL_POINT4_CLASS_ID;
	}
	void GetClassName(MSTR& s, bool localized) const override
	{
		s = localized ? POINT4LIST_CNAME : _T("Point4 List");
	}
};

void Point4ListControl::GetValue(TimeValue t, void* val, Interval& valid, GetSetMethod method)
{
	Point4* v = (Point4*)val;
	float weight;

	if (method == CTRL_ABSOLUTE)
	{
		*v = Point4(0, 0, 0, 0);
	}

	const BOOL indexMode = IsIndexMode();
	if (indexMode)
	{
		// In this mode we return only the selected index
		const int index = GetIndexModeIndex(t, valid);
		if(index >= 0 && index < conts.Count() && conts[index])
			conts[index]->GetValue(t, v, valid, CTRL_RELATIVE);
	}
	else
	{
		Point4 prevVal;
		for (int i = 0; i < conts.Count(); i++)
		{
			pblock->GetValue(kListCtrlWeight, t, weight, valid, i);
			if (weight != 0.0f)
			{
				prevVal = *v;
				if (conts[i])
					conts[i]->GetValue(t, v, valid, CTRL_RELATIVE);
				*v = prevVal + ((*v) - prevVal) * AverageWeight(weight, t);
			}
		}
	}
}

void Point4ListControl::SetValue(TimeValue t, void* val, int commit, GetSetMethod method)
{
	if (GetLocked())
		return;
	if (!conts.Count())
		return;
		
	const BOOL indexMode = IsIndexMode();
	if (indexMode)
	{
		const int index = GetIndexModeIndex(t);
		if (index >= 0 && index < conts.Count() && conts[index]) {
			conts[index]->SetValue(t, val, commit, method);
		}
		return;
	}

	float weight;
	const int activeIndex = GetWeightModeActive();
	if (activeIndex < 0 || activeIndex >= conts.Count())
		return;
	if (method == CTRL_ABSOLUTE)
	{
		Point4 v = *((Point4*)val);
		Point4 before(0, 0, 0, 0), after(0, 0, 0, 0);
		Interval valid;
		Point4 prevVal;
		for (int i = 0; i < activeIndex; i++)
		{
			prevVal = before;
			if (conts[i])
				conts[i]->GetValue(t, &before, valid, CTRL_RELATIVE);
			pblock->GetValue(kListCtrlWeight, t, weight, i);
			before = prevVal + (before - prevVal) * AverageWeight(weight, t);
			// conts[i]->GetValue(t,&before,valid,CTRL_RELATIVE);
		}
		for (int i = activeIndex + 1; i < conts.Count(); i++)
		{
			prevVal = after;
			if (conts[i])
				conts[i]->GetValue(t, &after, valid, CTRL_RELATIVE);
			pblock->GetValue(kListCtrlWeight, t, weight, i);
			after = prevVal + (after - prevVal) * AverageWeight(weight, t);
			// conts[i]->GetValue(t,&after,valid,CTRL_RELATIVE);
		}
		v = -before + v + -after;
		if (conts[activeIndex])
			conts[activeIndex]->SetValue(t, &v, commit, method);
	}
	else
	{
		pblock->GetValue(kListCtrlWeight, t, weight, activeIndex);
		*((Point4*)val) = *((Point4*)val) * AverageWeight(weight, t);
		if (conts[activeIndex])
			conts[activeIndex]->SetValue(t, val, commit, method);
	}
}
ListControl* Point4ListControl::DerivedClone()
{
	return new Point4ListControl;
}

// keeps track of whether an FP interface desc has been added to the ClassDesc
static bool p4ListInterfaceLoaded = false;

class Point4ListClassDesc : public ListUIClassDesc
{
public:
	const TCHAR* ClassName() override
	{
		return POINT4LIST_CNAME;
	}
	const TCHAR* NonLocalizedClassName() override
	{
		return _T("Point4 List");
	}
	SClass_ID SuperClassID() override
	{
		return CTRL_POINT4_CLASS_ID;
	}
	Class_ID ClassID() override
	{
		return Class_ID(POINT4LIST_CONTROL_CLASS_ID, 0);
	}
	void* Create(BOOL loading) override
	{
		if (!p4ListInterfaceLoaded)
		{
			AddInterface(&listControlInterface);
			// AddParamBlockDesc(&list_paramblk);
			p4ListInterfaceLoaded = true;
		}
		return new Point4ListControl(loading);
	}

	const TCHAR* InternalName() override
	{
		return _T("Point4List");
	} // returns fixed parsable name (scripter-visible name)
};
static Point4ListClassDesc point4ListCD;
ClassDesc* GetPoint4ListDesc()
{
	return &point4ListCD;
}

void Point4ListControl::Init()
{
	// make the paramblock
	point4ListCD.MakeAutoParamBlocks(this);
}

ClassDesc* Point4ListControl::GetClassDesc()
{
	return  GetPoint4ListDesc();
}

// clang-format off
// per instance list controller block
static ParamBlockDesc2 point4_list_paramblk (kListCtrlWeightParams, _T("Parameters"),  0, &point4ListCD, P_AUTO_CONSTRUCT, PBLOCK_INIT_REF_ID,
	// params
	kListCtrlWeight, _T("weight"), TYPE_FLOAT_TAB, 0, P_ANIMATABLE+P_VARIABLE_SIZE+P_TV_SHOW_ALL+P_COMPUTED_NAME,  IDS_AF_LIST_WEIGHT, 
		p_default, 		1.0, 
		p_range,		WEIGHT_MIN, WEIGHT_MAX,
		p_dim,			stdPercentDim,
		p_accessor,		&theListCtrlPBAccessor,
		p_end,
		
	kListCtrlAverage, _T("average"), TYPE_BOOL, 0, IDS_AF_LIST_AVERAGE, 
		p_default, 		FALSE, 
		p_end,

	kListCtrlIndex, _T("index"), TYPE_INT, P_RESET_DEFAULT, IDS_AF_LIST_INDEX,
		p_default, 0,
		p_range, 0, 1000, //use accessor to limit to weight table size.
		p_dim, &theIndexDim,
		p_accessor, & theListCtrlIndexPBAccessor,
		p_end,

	kListCtrlIndexMode, _T("indexMode"), TYPE_BOOL, 0, IDS_AF_LIST_INDEX_MODE,
		p_default,		FALSE,
		p_accessor, & thetheListCtrlIndexModePBAccessor, //use accessor to manage edit params if that changes.
		p_end,
	kListCtrlTag, _T("tag"), TYPE_STRING, 0, IDS_AF_LIST_TAG,
		p_default, _T(""),
		p_end,
	p_end
);

// clang-format on

//-------------------------------------------------------------------------
// Position list control
//

class PositionListControl : public ListControl
{
public:
	PositionListControl(BOOL loading)
			: ListControl(loading)
	{
		Init();
	}
	PositionListControl()
	{
		Init();
	}

	void GetValue(TimeValue t, void* val, Interval& valid, GetSetMethod method = CTRL_ABSOLUTE) override;
	void SetValue(TimeValue t, void* val, int commit, GetSetMethod method) override;
	ListControl* DerivedClone() override;
	virtual ClassDesc* GetClassDesc() override;
	void Init();

	Class_ID ClassID() override
	{
		return Class_ID(POSLIST_CONTROL_CLASS_ID, 0);
	}
	SClass_ID SuperClassID() override
	{
		return CTRL_POSITION_CLASS_ID;
	}
	void GetClassName(MSTR& s, bool localized) const override
	{
		s = localized ? POSLIST_CNAME : _T("Position List");
	}
};


void PositionListControl::GetValue(TimeValue t, void* val, Interval& valid, GetSetMethod method)
{
	float weight;

	const BOOL indexMode = IsIndexMode();
	int index = 0;
	if (indexMode)
	{
		// In this mode we return only the selected index
		index = GetIndexModeIndex(t, valid);
	}

	if (method == CTRL_ABSOLUTE)
	{
		Point3 prevPos;
		Matrix3 tm(1);
		if (indexMode)
		{
			// Just this index, no weighting
			if(index >= 0 && index < conts.Count() && conts[index])
				conts[index]->GetValue(t, &tm, valid, CTRL_RELATIVE);
		}
		else
		{
			// Weight and blend the list
			for (int i = 0; i < conts.Count(); i++)
			{
				pblock->GetValue(kListCtrlWeight, t, weight, valid, i);
				if (weight != 0.0f)
				{
					prevPos = tm.GetTrans();
					if (conts[i])
						conts[i]->GetValue(t, &tm, valid, CTRL_RELATIVE);
					tm.SetTrans(prevPos + (tm.GetTrans() - prevPos) * AverageWeight(weight, t));
				}
			}
		}
		*(Point3*)val = tm.GetTrans();
	}
	else
	{
		Matrix3* tm = (Matrix3*)val;
		if (indexMode)
		{
			// Just this index, no weighting
			if(index >= 0 && index < conts.Count() && conts[index])
				conts[index]->GetValue(t, tm, valid, CTRL_RELATIVE);
		}
		else
		{
			Point3 prevPos;
			for (int i = 0; i < conts.Count(); i++)
			{
				if (conts[i])
				{
					pblock->GetValue(kListCtrlWeight, t, weight, valid, i);
					if (weight != 0.0f)
					{
						prevPos = tm->GetTrans();
						conts[i]->GetValue(t, tm, valid, CTRL_RELATIVE);
						tm->SetTrans(prevPos + (tm->GetTrans() - prevPos) * AverageWeight(weight, t));
					}
				}
			}
		}
	}
}

void PositionListControl::SetValue(TimeValue t, void* val, int commit, GetSetMethod method)
{
	if (GetLocked())
		return;
	if (!conts.Count())
		return;

	const BOOL indexMode = IsIndexMode();
	if (indexMode)
	{
		const int index = GetIndexModeIndex(t);
		if (index >= 0 && index < conts.Count() && conts[index]) {
			conts[index]->SetValue(t, val, commit, method);
		}
		return;
	}

	const int activeIndex = GetWeightModeActive();
	if (activeIndex < 0 || activeIndex >= conts.Count())
		return;

	std::vector<float> weights;
	Interval dummyValidity = FOREVER;
	GetWeights(t, dummyValidity, weights);

	Point3 v = *((Point3*)val);

	Matrix3 before, after;
	Point3 prevPos;
	for (int i = 0; i < activeIndex; i++)
	{
		prevPos = before.GetTrans();
		if (conts[i])
			conts[i]->GetValue(t, &before, CTRL_RELATIVE);
		before.SetTrans(prevPos + (before.GetTrans() - prevPos) * AverageWeight(weights[i], t));
		// conts[i]->GetValue(t,&before,valid,CTRL_RELATIVE);
	}
	for (int i = activeIndex + 1; i < conts.Count(); i++)
	{
		prevPos = after.GetTrans();
		if (conts[i])
			conts[i]->GetValue(t, &after, CTRL_RELATIVE);
		after.SetTrans(prevPos + (after.GetTrans() - prevPos) * AverageWeight(weights[i], t));
		// conts[i]->GetValue(t,&after,valid,CTRL_RELATIVE);
	}

	if (method == CTRL_ABSOLUTE)
	{
		// v = -before.GetTrans() + v + -after.GetTrans();
		// RB 11/28/2000:
		// This was incorrect. It should be:
		// Inverse(afterControllers) * val * Inverse(beforeControllers)

		// v = Inverse(before) * v * Inverse(after);
		v = Inverse(after) * v * Inverse(before);

		if (conts[activeIndex])
			conts[activeIndex]->SetValue(t, &v, commit, method);
	}
	else
	{
		v = VectorTransform(Inverse(before), v);
		v = VectorTransform(Inverse(after), v);

		if (conts[activeIndex])
			conts[activeIndex]->SetValue(t, &v, commit, method);
	}
}

ListControl* PositionListControl::DerivedClone()
{
	return new PositionListControl;
}

// keeps track of whether an FP interface desc has been added to the ClassDesc
static bool posListInterfaceLoaded = false;

class PositionListClassDesc : public ListUIClassDesc
{
public:
	const TCHAR* ClassName() override
	{
		return POSLIST_CNAME;
	}
	const TCHAR* NonLocalizedClassName() override
	{
		return _T("Position List");
	}
	SClass_ID SuperClassID() override
	{
		return CTRL_POSITION_CLASS_ID;
	}
	Class_ID ClassID() override
	{
		return Class_ID(POSLIST_CONTROL_CLASS_ID, 0);
	}
	void* Create(BOOL loading) override
	{
		if (!posListInterfaceLoaded)
		{
			AddInterface(&listControlInterface);
			// AddParamBlockDesc(&list_paramblk);
			posListInterfaceLoaded = true;
		}
		return new PositionListControl(loading);
	}

	const TCHAR* InternalName() override
	{
		return _T("PositionList");
	} // returns fixed parsable name (scripter-visible name)
};
static PositionListClassDesc posListCD;
ClassDesc* GetPositionListDesc()
{
	return &posListCD;
}

void PositionListControl::Init()
{
	// make the paramblock
	posListCD.MakeAutoParamBlocks(this);
}

ClassDesc* PositionListControl::GetClassDesc()
{
	return GetPositionListDesc();
}

// clang-format off
// CAL-10/30/2002: Add individual ParamBlockDesc2 for each list controller - see point3_list_paramblk.
// per instance list controller block
static ParamBlockDesc2 pos_list_paramblk (kListCtrlWeightParams, _T("Parameters"),  0, &posListCD, P_AUTO_CONSTRUCT, PBLOCK_INIT_REF_ID,
	// params
	kListCtrlWeight, _T("weight"), TYPE_FLOAT_TAB, 0, P_ANIMATABLE+P_VARIABLE_SIZE+P_TV_SHOW_ALL+P_COMPUTED_NAME,  IDS_AF_LIST_WEIGHT, 
		p_default, 		1.0, 
		p_range,		WEIGHT_MIN, WEIGHT_MAX,
		p_dim,			stdPercentDim,
		p_accessor,		&theListCtrlPBAccessor,
		p_end,
		
	kListCtrlAverage, _T("average"), TYPE_BOOL, 0, IDS_AF_LIST_AVERAGE, 
		p_default, 		FALSE, 
		p_end,

	kListCtrlIndex, _T("index"), TYPE_INT, P_RESET_DEFAULT, IDS_AF_LIST_INDEX,
		p_default, 0,
		p_range, 0, 1000, //use accessor to limit to weight table size.
		p_dim, &theIndexDim,
		p_accessor, & theListCtrlIndexPBAccessor,
		p_end,

	kListCtrlIndexMode, _T("indexMode"), TYPE_BOOL, 0, IDS_AF_LIST_INDEX_MODE,
		p_default,		FALSE,
		p_accessor, & thetheListCtrlIndexModePBAccessor, //use accessor to manage edit params if that changes.
		p_end,
	kListCtrlTag, _T("tag"), TYPE_STRING, 0, IDS_AF_LIST_TAG,
		p_default, _T(""),
		p_end,
	p_end
);

// clang-format on

//-------------------------------------------------------------------------
// Rotation list control
//

class RotationListControl : public ListControl
{
public:
	RotationListControl(BOOL loading)
			: ListControl(loading)
	{
		Init();
	}
	RotationListControl()
	{
		Init();
	}

	void GetValue(TimeValue t, void* val, Interval& valid, GetSetMethod method = CTRL_ABSOLUTE) override;
	void SetValue(TimeValue t, void* val, int commit, GetSetMethod method) override;
	ListControl* DerivedClone() override;
	virtual ClassDesc* GetClassDesc() override;
	void Init();

	Class_ID ClassID() override
	{
		return Class_ID(ROTLIST_CONTROL_CLASS_ID, 0);
	}
	SClass_ID SuperClassID() override
	{
		return CTRL_ROTATION_CLASS_ID;
	}
	void GetClassName(MSTR& s, bool localized) const override
	{
		s = localized ? ROTLIST_CNAME : _T("Rotation List");
	}
};

void RotationListControl::GetValue(TimeValue t, void* val, Interval& valid, GetSetMethod method)
{
	const BOOL indexMode = IsIndexMode();
	Matrix3 tm, localTM;
	if (method == CTRL_ABSOLUTE) {
		tm.IdentityMatrix();
	}
	else {
		tm = *((Matrix3*)val);
	}

	if(indexMode)
	{
		// In this mode we return only the selected index
		const int index = GetIndexModeIndex(t, valid);
		
		// Just the index sub control, no weighting
		if(index >= 0 && index < conts.Count() && conts[index])
			conts[index]->GetValue(t, &tm, valid, CTRL_RELATIVE);
	}
	else
	{
		BOOL average = FALSE;
		pblock->GetValue(kListCtrlAverage, 0, average);
		std::vector<float> weights;
		GetWeights(t, valid, weights);
		
		// Weight the list and blend
		if (!average)
		{ // Addative method
			for (int i = 0; i < conts.Count(); i++)
			{
				if (weights[i] != 0.0f && conts[i])
				{
					localTM = tm;
					conts[i]->GetValue(t, &localTM, valid, CTRL_RELATIVE);
					localTM = localTM * Inverse(tm);

					Quat weightedRot = Quat(localTM);
					weightedRot.Normalize();
					weightedRot.MakeClosest(IdentQuat()); // CAL-10/15/2002: find the smallest rotation
					weightedRot = Slerp(IdentQuat(), weightedRot, std::min(std::max(weights[i], 0.0f), 1.0f));
					weightedRot.Normalize();
					PreRotateMatrix(tm, weightedRot);
				}
			}
		}
		else
		{ // pose to pose blending
			Matrix3 initTM = tm;
			Quat lastRot = IdentQuat();
			for (int i = 0; i < conts.Count(); i++)
			{
				if (weights[i] != 0.0f && conts[i])
				{
					localTM = tm;
					conts[i]->GetValue(t, &localTM, valid, CTRL_RELATIVE);
					localTM = localTM * Inverse(tm);

					Quat weightedRot = Quat(localTM);
					weightedRot.Normalize();
					weightedRot.MakeClosest(lastRot);
					weightedRot = Slerp(lastRot, weightedRot, std::min(std::max(weights[i], 0.0f), 1.0f));
					weightedRot.Normalize();

					lastRot = weightedRot;
					weightedRot = weightedRot * Quat(initTM);

					Point3 trans = tm.GetTrans();
					tm.SetRotate(weightedRot);
					tm.SetTrans(trans);
				}
			}
		}
	}

	if (method == CTRL_ABSOLUTE)
	{
		*((Quat*)val) = Quat(tm);
	}
	else
	{
		*((Matrix3*)val) = tm;
	}
}


static bool IsGimbalAxis(const Point3& a)
{
	int i = a.MaxComponent();
	if (a[i] != 1.0f)
		return false;
	int j = (i + 1) % 3, k = (i + 2) % 3;
	if (a[j] != 0.0f || a[k] != FLT_MIN)
		return false;
	return true;
}
void RotationListControl::SetValue(TimeValue t, void* val, int commit, GetSetMethod method)
{
	if (GetLocked())
		return;
	if (!conts.Count())
		return;

	const BOOL indexMode = IsIndexMode();
	if (indexMode)
	{
		//Do nothing but pass on the set value
		const int index = GetIndexModeIndex(t);
		if (index >= 0 && index < conts.Count() && conts[index]) {
			conts[index]->SetValue(t, val, commit, method);
		}
		return;
	}

	const int activeIndex = GetWeightModeActive();
	if (activeIndex < 0 || activeIndex >= conts.Count())
		return;
	if (conts[activeIndex] == nullptr)
		return;
	
	AngAxis* aa = (AngAxis*)val;

	bool gimbal_axis = false;
	if (GetCOREInterface()->GetRefCoordSys() == COORDS_GIMBAL && method == CTRL_RELATIVE && IsGimbalAxis(aa->axis))
		gimbal_axis = true;

	std::vector<float> weights;
	Interval dummyValidity = FOREVER;
	GetWeights(t, dummyValidity, weights);

	Matrix3 before(1);
	Matrix3 localTM;
	BOOL average = FALSE;
	pblock->GetValue(kListCtrlAverage, 0, average);

	if (!average)
	{ // Addative method
		if (!gimbal_axis)
			for (int i = 0; i < activeIndex; i++)
			{
				localTM = before;
				if (conts[i])
					conts[i]->GetValue(t, &localTM, CTRL_RELATIVE);
				localTM = localTM * Inverse(before);

				Quat weightedRot = Slerp(IdentQuat(), Quat(localTM), std::min(std::max(weights[i], 0.0f), 1.0f));
				PreRotateMatrix(before, weightedRot);
			}

		if (method == CTRL_ABSOLUTE)
		{
			Quat v = *((Quat*)val);
			Matrix3 after(1);
			for (int i = activeIndex + 1; i < conts.Count(); i++)
			{
				localTM = after;
				if (conts[i])
					conts[i]->GetValue(t, &localTM, CTRL_RELATIVE);
				localTM = localTM * Inverse(after);

				Quat weightedRot = Slerp(IdentQuat(), Quat(localTM), std::min(std::max(weights[i], 0.0f), 1.0f));
				PreRotateMatrix(after, weightedRot);
			}

			v = Inverse(Quat(after)) * v * Inverse(Quat(before));
			conts[activeIndex]->SetValue(t, &v, commit, method);
		}
		else
		{
			AngAxis na = *aa;
			if (!(conts[activeIndex]->ClassID() == Class_ID(LOOKAT_CONSTRAINT_CLASS_ID, 0)) && !gimbal_axis)
				na.axis = VectorTransform(Inverse(before), na.axis);
			conts[activeIndex]->SetValue(t, &na, commit, method);
		}
	}
	else
	{ // pose to pose blending
		Matrix3 initTM = before;
		Quat lastRot = IdentQuat();
		if (!gimbal_axis)
			for (int i = 0; i < activeIndex; i++)
			{
				localTM = before;
				if (conts[i])
					conts[i]->GetValue(t, &localTM, CTRL_RELATIVE);
				localTM = localTM * Inverse(before);
				Quat weightedRot = Slerp(lastRot, Quat(localTM), std::min(std::max(weights[i], 0.0f), 1.0f));
				lastRot = weightedRot;
				weightedRot = weightedRot * Quat(initTM);
				Point3 trans = before.GetTrans();
				before.SetRotate(weightedRot);
				before.SetTrans(trans);
			}

		if (method == CTRL_ABSOLUTE)
		{
			Quat v = *((Quat*)val);
			Matrix3 after(1);
			initTM = after;
			lastRot = IdentQuat();
			for (int i = activeIndex + 1; i < conts.Count(); i++)
			{
				localTM = after;
				if (conts[i])
					conts[i]->GetValue(t, &localTM, CTRL_RELATIVE);
				localTM = localTM * Inverse(after);
				Quat weightedRot = Slerp(lastRot, Quat(localTM), std::min(std::max(weights[i], 0.0f), 1.0f));
				lastRot = weightedRot;
				weightedRot = weightedRot * Quat(initTM);
				Point3 trans = after.GetTrans();
				after.SetRotate(weightedRot);
				after.SetTrans(trans);
			}

			v = Inverse(Quat(after)) * v * Inverse(Quat(before));
			conts[activeIndex]->SetValue(t, &v, commit, method);
		}
		else
		{
			AngAxis na = *aa;
			if (!(conts[activeIndex]->ClassID() == Class_ID(LOOKAT_CONSTRAINT_CLASS_ID, 0)) && !gimbal_axis)
				na.axis = VectorTransform(Inverse(before), na.axis);

			conts[activeIndex]->SetValue(t, &na, commit, method);
		}
	}
}

ListControl* RotationListControl::DerivedClone()
{
	return new RotationListControl;
}

// keeps track of whether an FP interface desc has been added to the ClassDesc
static bool rotListInterfaceLoaded = false;

class RotationListClassDesc : public ListUIClassDesc
{
public:
	const TCHAR* ClassName() override
	{
		return ROTLIST_CNAME;
	}
	const TCHAR* NonLocalizedClassName() override
	{
		return _T("Rotation List");
	}
	SClass_ID SuperClassID() override
	{
		return CTRL_ROTATION_CLASS_ID;
	}
	Class_ID ClassID() override
	{
		return Class_ID(ROTLIST_CONTROL_CLASS_ID, 0);
	}
	void* Create(BOOL loading) override
	{
		if (!rotListInterfaceLoaded)
		{
			AddInterface(&listControlInterface);
			// AddParamBlockDesc(&list_paramblk);
			rotListInterfaceLoaded = true;
		}
		return new RotationListControl(loading);
	}

	const TCHAR* InternalName() override
	{
		return _T("RotationList");
	} // returns fixed parsable name (scripter-visible name)
};
static RotationListClassDesc rotListCD;
ClassDesc* GetRotationListDesc()
{
	return &rotListCD;
}

void RotationListControl::Init()
{
	// make the paramblock
	rotListCD.MakeAutoParamBlocks(this);
}

ClassDesc* RotationListControl::GetClassDesc()
{
	return GetRotationListDesc();
}

// clang-format off
// CAL-10/30/2002: Add individual ParamBlockDesc2 for each list controller - see point3_list_paramblk.
// per instance list controller block
static ParamBlockDesc2 rot_list_paramblk (kListCtrlWeightParams, _T("Parameters"),  0, &rotListCD, P_AUTO_CONSTRUCT, PBLOCK_INIT_REF_ID,
	// params
	kListCtrlWeight, _T("weight"), TYPE_FLOAT_TAB, 0, P_ANIMATABLE+P_VARIABLE_SIZE+P_TV_SHOW_ALL+P_COMPUTED_NAME,  IDS_AF_LIST_WEIGHT, 
		p_default, 		1.0, 
		p_range,		WEIGHT_MIN, WEIGHT_MAX,
		p_dim,			stdPercentDim,
		p_accessor,		&theListCtrlPBAccessor,
		p_end,
		
	kListCtrlAverage, _T("average"), TYPE_BOOL, 0, IDS_AF_LIST_AVERAGE, 
		p_default, 		FALSE, 
		p_end,

	kListCtrlIndex, _T("index"), TYPE_INT, P_RESET_DEFAULT, IDS_AF_LIST_INDEX,
		p_default, 0,
		p_range, 0, 1000, //use accessor to limit to weight table size.
		p_dim, &theIndexDim,
		p_accessor, & theListCtrlIndexPBAccessor,
		p_end,

	kListCtrlIndexMode, _T("indexMode"), TYPE_BOOL, 0, IDS_AF_LIST_INDEX_MODE,
		p_default,		FALSE,
		p_accessor, & thetheListCtrlIndexModePBAccessor, //use accessor to manage edit params if that changes.
		p_end,
	kListCtrlTag, _T("tag"), TYPE_STRING, 0, IDS_AF_LIST_TAG,
		p_default, _T(""),
		p_end,
	p_end
);

// clang-format on

//-------------------------------------------------------------------------
// Scale list control
//

class ScaleListControl : public ListControl
{
public:
	ScaleListControl(BOOL loading)
			: ListControl(loading)
	{
		Init();
	}
	ScaleListControl()
	{
		Init();
	}

	void GetValue(TimeValue t, void* val, Interval& valid, GetSetMethod method = CTRL_ABSOLUTE) override;
	void SetValue(TimeValue t, void* val, int commit, GetSetMethod method) override;
	ListControl* DerivedClone() override;
	virtual ClassDesc* GetClassDesc() override;
	void Init();

	Class_ID ClassID() override
	{
		return Class_ID(SCALELIST_CONTROL_CLASS_ID, 0);
	}
	SClass_ID SuperClassID() override
	{
		return CTRL_SCALE_CLASS_ID;
	}
	void GetClassName(MSTR& s, bool localized) const override
	{
		s = localized ? SCALELIST_CNAME : _T("Scale List");
	}
};

void ScaleListControl::GetValue(TimeValue t, void* val, Interval& valid, GetSetMethod method)
{
	const BOOL indexMode = IsIndexMode();
	int index = -1;
	if (indexMode)
	{
		// In this mode we return only the selected index
		index = GetIndexModeIndex(t, valid);
	}

	std::vector<float> weights;
	GetWeights(t, valid, weights);

	if (method == CTRL_ABSOLUTE)
	{
		ScaleValue totalScale(Point3(1, 1, 1));
		if (indexMode)
		{
			// Just the index, no weighting
			if (index >= 0 && index < conts.Count() && conts[index])
				conts[index]->GetValue(t, &totalScale, valid, CTRL_ABSOLUTE);
		}
		else
		{
			ScaleValue tempScale(Point3(0, 0, 0));
			for (int i = 0; i < conts.Count(); i++)
			{
				if (weights[i] != 0.0f)
				{
					// CAL-06/06/02: why do we need to zero the scale here? doesn't make sense to me??
					// if (i==0) totalScale = Point3(0,0,0);
					if (conts[i])
						conts[i]->GetValue(t, &tempScale, valid, CTRL_ABSOLUTE);
					totalScale = totalScale + tempScale * AverageWeight(weights[i], t);
				}
			}
		}
		(*(ScaleValue*)val) = totalScale;
	}
	else
	{
		Matrix3* tm = (Matrix3*)val;
		if (indexMode)
		{
			// Just the index, no weighting
			if(index >= 0 && index < conts.Count() && conts[index])
				conts[index]->GetValue(t, tm, valid, CTRL_RELATIVE);
		}
		else
		{
			AffineParts parts;
			decomp_affine(*tm, &parts);
			Point3 tempVal = parts.k;
			for (int i = 0; i < conts.Count(); i++)
			{
				if (weights[i] != 0.0f && conts[i])
				{
					conts[i]->GetValue(t, tm, valid, CTRL_RELATIVE);
					decomp_affine(*tm, &parts);
					tempVal = tempVal + (parts.k - tempVal) * AverageWeight(weights[i], t);
					// CAL-06/06/02: Use comp_affine to reconstruct the matrix better.
					//		NOTE: SetRotate() will erase the scale set by SetScale().
					parts.k = tempVal;
					comp_affine(parts, *tm);
					// Quat rot = Quat(*tm);
					// Point3 trans =tm->GetTrans();
					// tm->SetScale(tempVal);
					// tm->SetRotate(rot);
					// tm->SetTrans(trans);
				}
			}
		}
	}
}


void ScaleListControl::SetValue(TimeValue t, void* val, int commit, GetSetMethod method)
{
	if (GetLocked())
		return;
	if (!conts.Count())
		return;

	const BOOL indexMode = IsIndexMode();
	if (indexMode)
	{
		//Do nothing but pass on the set value
		const int index = GetIndexModeIndex(t);
		if (index >= 0 && index < conts.Count() && conts[index]) {
			conts[index]->SetValue(t, val, commit, method);
		}
		return;
	}

	const int activeIndex = GetWeightModeActive();
	if (activeIndex < 0 || activeIndex >= conts.Count())
		return;
	
	if (method == CTRL_ABSOLUTE)
	{
		ScaleValue v = *((Point3*)val);
		Matrix3 before(1), after(1);
		Interval valid;
		for (int i = 0; i < activeIndex; i++)
		{
			if (conts[i])
				conts[i]->GetValue(t, &before, valid, CTRL_RELATIVE);
		}
		for (int i = activeIndex + 1; i < conts.Count(); i++)
		{
			if (conts[i])
				conts[i]->GetValue(t, &after, valid, CTRL_RELATIVE);
		}

		AffineParts bparts, aparts;
		decomp_affine(Inverse(before), &bparts);
		decomp_affine(Inverse(after), &aparts);
		v.q = Inverse(bparts.u) * v.q * Inverse(aparts.u);
		// CAL-06/06/02: scale should be computed by scaling (not subtracting) out others scales.
		//		NOTE: probably need to check divide-by-zero exception.
		// v.s = -bparts.k + v.s + -aparts.k;
		DbgAssert((bparts.k * aparts.k) != Point3::Origin);
		v.s = v.s / (bparts.k * aparts.k);
		if (conts[activeIndex])
			conts[activeIndex]->SetValue(t, &v, commit, method);
	}
	else
	{
		if (conts[activeIndex])
			conts[activeIndex]->SetValue(t, val, commit, method);
	}
}

ListControl* ScaleListControl::DerivedClone()
{
	return new ScaleListControl;
}

// keeps track of whether an FP interface desc has been added to the ClassDesc
static bool scaleListInterfaceLoaded = false;

class ScaleListClassDesc : public ListUIClassDesc
{
public:
	const TCHAR* ClassName() override
	{
		return SCALELIST_CNAME;
	}
	const TCHAR* NonLocalizedClassName() override
	{
		return _T("Scale List");
	}
	SClass_ID SuperClassID() override
	{
		return CTRL_SCALE_CLASS_ID;
	}
	Class_ID ClassID() override
	{
		return Class_ID(SCALELIST_CONTROL_CLASS_ID, 0);
	}
	void* Create(BOOL loading) override
	{
		if (!scaleListInterfaceLoaded)
		{
			AddInterface(&listControlInterface);
			// AddParamBlockDesc(&list_paramblk);
			scaleListInterfaceLoaded = true;
		}
		return new ScaleListControl(loading);
	}

	const TCHAR* InternalName() override
	{
		return _T("ScaleList");
	} // returns fixed parsable name (scripter-visible name)
};
static ScaleListClassDesc scaleListCD;
ClassDesc* GetScaleListDesc()
{
	return &scaleListCD;
}

void ScaleListControl::Init()
{
	// make the paramblock
	scaleListCD.MakeAutoParamBlocks(this);
}

ClassDesc* ScaleListControl::GetClassDesc()
{
	return GetScaleListDesc();
}

// clang-format off
// CAL-10/30/2002: Add individual ParamBlockDesc2 for each list controller - see point3_list_paramblk.
// per instance list controller block
static ParamBlockDesc2 scale_list_paramblk (kListCtrlWeightParams, _T("Parameters"),  0, &scaleListCD, P_AUTO_CONSTRUCT, PBLOCK_INIT_REF_ID,
	// params
	kListCtrlWeight, _T("weight"), TYPE_FLOAT_TAB, 0, P_ANIMATABLE+P_VARIABLE_SIZE+P_TV_SHOW_ALL+P_COMPUTED_NAME,  IDS_AF_LIST_WEIGHT, 
		p_default, 		1.0, 
		p_range,		WEIGHT_MIN, WEIGHT_MAX,
		p_dim,			stdPercentDim,
		p_accessor,		&theListCtrlPBAccessor,
		p_end,
		
	kListCtrlAverage, _T("average"), TYPE_BOOL, 0, IDS_AF_LIST_AVERAGE, 
		p_default, 		FALSE, 
		p_end,

	kListCtrlIndex, _T("index"), TYPE_INT, P_RESET_DEFAULT, IDS_AF_LIST_INDEX,
		p_default, 0,
		p_range, 0, 1000, //use accessor to limit to weight table size.
		p_dim, &theIndexDim,
		p_accessor, & theListCtrlIndexPBAccessor,
		p_end,

	kListCtrlIndexMode, _T("indexMode"), TYPE_BOOL, 0, IDS_AF_LIST_INDEX_MODE,
		p_default,		FALSE,
		p_accessor, & thetheListCtrlIndexModePBAccessor, //use accessor to manage edit params if that changes.
		p_end,
	kListCtrlTag, _T("tag"), TYPE_STRING, 0, IDS_AF_LIST_TAG,
		p_default, _T(""),
		p_end,
	p_end
);

// clang-format on

//-------------------------------------------------------------------------
// Block Control Watje

class BlockControl : public ListControl
{
public:
	BlockControl(BOOL loading)
			: ListControl(loading)
	{
		Init();
	}
	BlockControl()
	{
		Init();
	}

	void GetValue(TimeValue t, void* val, Interval& valid, GetSetMethod method = CTRL_ABSOLUTE) override;
	void SetValue(TimeValue t, void* val, int commit, GetSetMethod method) override;
	ListControl* DerivedClone() override;
	virtual ClassDesc* GetClassDesc() override { return nullptr; }
	void Init();

	Class_ID ClassID() override
	{
		return Class_ID(DRIVERBLOCKLIST_CONTROL_CLASS_ID, 0);
	}
	SClass_ID SuperClassID() override
	{
		return DRIVERBLOCK_SUPER_CLASS_ID;
	}
	void GetClassName(MSTR& s, bool localized) const override
	{
		s = localized ? BLOCKCONTROL_CNAME : _T("Block Control");
	}
	BOOL IsReplaceable() override
	{
		return FALSE;
	}

	BOOL CanCopyAnim() override
	{
		return FALSE;
	}
	BOOL CanApplyEaseMultCurves() override
	{
		return FALSE;
	}

	int PaintTrack(ParamDimensionBase* dim, HDC hdc, Rect& rcTrack, Rect& rcPaint, float zoom, int scroll, DWORD flags) override;

	int NumSubs() override
	{
		return conts.Count() + 1;
	} // numControllers+dummyController (no pblock)
};

void BlockControl::GetValue(TimeValue t, void* val, Interval& valid, GetSetMethod method)
{
}

void BlockControl::SetValue(TimeValue t, void* val, int commit, GetSetMethod method)
{
}

ListControl* BlockControl::DerivedClone()
{
	return new BlockControl;
}

int BlockControl::PaintTrack(
		ParamDimensionBase* dim, HDC hdc, Rect& rcTrack, Rect& rcPaint, float zoom, int scroll, DWORD flags)
{
	if (flags & PAINTTRACK_SUBTREEMODE)
		return TRACK_DONE; // don't paint subtrack keys for this controlller
	return TRACK_DORANGE;
}

// keeps track of whether an FP interface desc has been added to the ClassDesc
static bool blockControlInterfaceLoaded = false;

class BlockControlClassDesc : public ListUIClassDesc
{
public:
	int IsPublic() override
	{
		return 0;
	}
	const TCHAR* ClassName() override
	{
		return BLOCKCONTROL_CNAME;
	}
	const TCHAR* NonLocalizedClassName() override
	{
		return _T("Block Control");
	}
	SClass_ID SuperClassID() override
	{
		return DRIVERBLOCK_SUPER_CLASS_ID;
	}
	Class_ID ClassID() override
	{
		return Class_ID(DRIVERBLOCKLIST_CONTROL_CLASS_ID, 0);
	}
	void* Create(BOOL loading) override
	{
		if (!blockControlInterfaceLoaded)
		{
			AddInterface(&listControlInterface);
			// AddParamBlockDesc(&list_paramblk);
			blockControlInterfaceLoaded = true;
		}
		return new BlockControl(loading);
	}

	const TCHAR* InternalName() override
	{
		return _T("BlockControl"); // Note: As other ClassDesc here, we return the same string as NonLocalizedClassName(), but without spaces.
	} // returns fixed parsable name (scripter-visible name)
};

static BlockControlClassDesc BlockControlCD;

ClassDesc* GetBlockControlDesc()
{
	return &BlockControlCD;
}

void BlockControl::Init()
{
	// make the paramblock
	BlockControlCD.MakeAutoParamBlocks(this);
}

// clang-format off
// CAL-10/30/2002: Add individual ParamBlockDesc2 for each list controller - see point3_list_paramblk.
// per instance list controller block
static ParamBlockDesc2 block_control_paramblk (kListCtrlWeightParams, _T("Parameters"),  0, &BlockControlCD, P_AUTO_CONSTRUCT, PBLOCK_INIT_REF_ID,
	// params
	kListCtrlWeight, _T("weight"), TYPE_FLOAT_TAB, 0, P_ANIMATABLE+P_VARIABLE_SIZE+P_TV_SHOW_ALL+P_COMPUTED_NAME,  IDS_AF_LIST_WEIGHT, 
		p_default, 		1.0, 
		p_range,		WEIGHT_MIN, WEIGHT_MAX,
		p_dim,			stdPercentDim,
		p_accessor,		&theListCtrlPBAccessor,
		p_end,
		
	kListCtrlAverage, _T("average"), TYPE_BOOL, 0, IDS_AF_LIST_AVERAGE, 
		p_default, 		FALSE, 
		p_end,

	kListCtrlIndex, _T("index"), TYPE_INT, P_RESET_DEFAULT, IDS_AF_LIST_INDEX,
		p_default, 0,
		p_range, 0, 1000, //use accessor to limit to weight table size.
		p_dim, &theIndexDim,
		p_accessor, & theListCtrlIndexPBAccessor,
		p_end,

	kListCtrlIndexMode, _T("indexMode"), TYPE_BOOL, 0, IDS_AF_LIST_INDEX_MODE,
		p_default,		FALSE,
		p_accessor, & thetheListCtrlIndexModePBAccessor, //use accessor to manage edit params if that changes.
		p_end,
	kListCtrlTag, _T("tag"), TYPE_STRING, 0, IDS_AF_LIST_TAG,
		p_default, _T(""),
		p_end,
	p_end
);

//-------------------------------------------------------------------------
// Transform list control
//

class TransformListControl : public ListControl {
public:
	TransformListControl(BOOL loading) : ListControl(loading) { Init(); }
	TransformListControl() { Init(); }

	Class_ID ClassID() override { return Class_ID(TMLIST_CONTROL_CLASS_ID, 0); }
	SClass_ID SuperClassID() override { return CTRL_MATRIX3_CLASS_ID; }
	void GetClassName(MSTR& s, bool localized) const override
	{
		s = localized ? TMLIST_CNAME : _T("Transform List");
	}

	Matrix3 PerformWeightedGetValue(TimeValue t, const Matrix3& parentTm, Interval& valid, std::vector<Matrix3>* results);
	
	void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method = CTRL_ABSOLUTE) override;
	void SetValue(TimeValue t, void *val, int commit, GetSetMethod method) override;

	ListControl *DerivedClone() override;
	virtual ClassDesc* GetClassDesc() override;
	void Init();

	// Behave like a TM controller, Try to be a pass-through for the active active sub control.
	// Overriding this form parent ListControl, all sub TM's need to know about the parent change.
	virtual BOOL ChangeParents(TimeValue t, const Matrix3& oldP, const Matrix3& newP, const Matrix3& tm) override;
	BOOL CreateLockKey(TimeValue t, int which) override;
	DWORD GetInheritanceFlags() override;
	BOOL SetInheritanceFlags(DWORD f, BOOL keepPos) override;
	//KZ, skip this tm control method, best to always inherit, can't conditionally do this per sub tm anyway.
	//BOOL InheritsParentTransform() override; 

	// IK stuff
	BOOL CanCopyIKParams(int which) override;
	IKClipObject* CopyIKParams(int which) override;
	BOOL CanPasteIKParams(IKClipObject* co, int which) override;
	void PasteIKParams(IKClipObject* co, int which) override;
	void InitIKJoints(InitJointData* posData, InitJointData* rotData) override;
	BOOL GetIKJoints(InitJointData* posData, InitJointData* rotData) override;
	void InitIKJoints2(InitJointData2* posData, InitJointData2* rotData) override;
	BOOL GetIKJoints2(InitJointData2* posData, InitJointData2* rotData) override;
	void MirrorIKConstraints(int axis, int which, BOOL pasteMirror = FALSE) override;

	// Try to pass these calls throgh to the active controller
	Control* GetPositionController() override;
	Control* GetRotationController() override;
	Control* GetScaleController() override;
	BOOL SetPositionController(Control* c) override;
	BOOL SetRotationController(Control* c) override;
	BOOL SetScaleController(Control* c) override;
};

BOOL ListControl::GetSequential() const
{
	//false = fan, true = chain
	BOOL sequential = FALSE;
	if (dynamic_cast<TransformListControl const*>(this) != nullptr)
	{
		// Time of zero is fine because its not animatable.
		pblock->GetValue(kListCtrlSequential, 0, sequential);
	}
	return sequential;
}

bool ListControl::GetWeightAgainstMode() const
{
	//false = weight against identity, true = LERP from previous
	BOOL weightMode = FALSE;
	if (dynamic_cast<TransformListControl const*>(this) != nullptr)
	{
		// Time of zero is fine because its not animatable.
		pblock->GetValue(kListCtrlWeightAgainstMode, 0, weightMode);
	}
	return weightMode != 0;
}

Quat BlendBetweenQuats(const Quat& basisQuat, const Quat& effectingQuat, float weightTowardsEffecting, bool relativeRotations)
{
	// Clamp between [-1.0, 1.0] for rotations
	if (weightTowardsEffecting < -1.0f)
		weightTowardsEffecting = -1.0f;
	if (weightTowardsEffecting > 1.0f)
		weightTowardsEffecting = 1.0f;

	Quat weightedRot = effectingQuat;
	weightedRot.Normalize();

	float slerpWeight = weightTowardsEffecting;
	// SLERP only works with positive weights so flip the rotation.
	if (weightTowardsEffecting < 0.f) {
		slerpWeight = -slerpWeight;
		weightedRot.Invert();
	}

	if (relativeRotations)
	{
		// find the smallest rotation to previous rotation
		weightedRot.MakeClosest(basisQuat);

		// rotations are all relative to each other.
		weightedRot = Slerp(basisQuat, weightedRot, slerpWeight);
	}
	else
	{
		// find the smallest rotation to identity quat
		weightedRot.MakeClosest(IdentQuat());

		// rotations are all realative to identity quat, not each other
		weightedRot = Slerp(IdentQuat(), weightedRot, slerpWeight);
		weightedRot.Normalize();

		// Combining the rotations in reverse order produces the correct result. This is what rotation_list does.
		weightedRot = weightedRot * basisQuat;
	}

	weightedRot.Normalize();
	return weightedRot;
};

AffineParts MatrixMath::BlendAffineParts(const AffineParts& basisParts, const AffineParts& effectingParts,
	float effectingPosScaleWeight, float effectingRotationWeight,
	bool weightIsAgainstBasis /*false=weight against identity. true=for rotation and scale, this behaves like a lerp from basis to target*/)
{
	AffineParts finalParts;
	finalParts.t = basisParts.t + (effectingParts.t * effectingPosScaleWeight);
	
	// Slerp the rotations
	finalParts.q = BlendBetweenQuats(basisParts.q, effectingParts.q, effectingRotationWeight, weightIsAgainstBasis/*, legacyRotationCompatibility*/);

	// Combine the scale rotations
	finalParts.u = BlendBetweenQuats(basisParts.u, effectingParts.u, effectingRotationWeight, weightIsAgainstBasis/*, false*/);

	// Handle scale factors.
	if(weightIsAgainstBasis)
	{
		// treat the effecting scale as having been scaled by basis then weight the different. (Copying what scale list does).
		const Point3 affectedScale = (basisParts.k * basisParts.f) * (effectingParts.k * effectingParts.f);
		finalParts.k = (basisParts.k * basisParts.f) + ((affectedScale - (basisParts.k * basisParts.f)) * effectingPosScaleWeight);
	}
	else
	{
		// weighting is between 1 and effecting scale.
		finalParts.k = (basisParts.k * basisParts.f) + ((effectingParts.k * effectingParts.f) - Point3(1, 1, 1)) * effectingPosScaleWeight;
	}
	finalParts.f = 1.0f;
	
	return finalParts;
}

Matrix3 TransformListControl::PerformWeightedGetValue(TimeValue t, const Matrix3& parentTm, Interval& valid, std::vector<Matrix3>* results)
{
	// sequentialMode: false = fan, true = chain
	const bool sequentialMode = GetSequential() != 0;

	// WeightAgainstMode: false = weight against identity, true = LERP from previous
	const bool weightAgainstMode = GetWeightAgainstMode();

	// Grab all the weights to decide if we can optimize for the case where there is only 1.
	std::vector<float> weights;
	const size_t numNonZeroWeights = GetWeights(t, valid, weights);

	Matrix3 tm = parentTm; //start by being relative to the incoming transform

	if(results != nullptr) {
		results->resize(conts.Count());
	}

	bool performNormalBlending = true;
	if (numNonZeroWeights == 0)
	{
		//No tm expressed, use tm as is (which is the parent TM)
		performNormalBlending = false;
	}
	else if (numNonZeroWeights == 1)
	{
		// Try to catch the case where we only have 1 controller and we can avoid blending because weight might be == 100%
		// Find the 1 sub controller we a weight for.
		int index = -1;
		for (int i = 0; i < weights.size(); ++i)
		{
			if (weights[i] > 0.0f) {
				index = i;
				break;
			}
		}
		// Lets see if we can avoid any blending
		// -1 check for sanity
		if (index != -1)
		{
			// Grab the normalized weight and check if we are at 100% for the shortcut.
			const float averageWeight = AverageWeight(weights[index], t);
			if(fabs(averageWeight - 1.0f) < std::numeric_limits<float>::epsilon())
			{
				// Shortcut detected: We have 1 weighted sub control and it has a weight of 100%.
				// Simply get the value and skip the decomposition and blending.
				conts[index]->GetValue(t, &tm, valid, CTRL_RELATIVE);
				performNormalBlending = false;
			}
		}
		else {
			performNormalBlending = true;
		}
	}

	if (performNormalBlending)
	{
		// Go over each controller and accumulate the weighted parts applied in top of this running total.
		// Setup initial parts as parent.
		AffineParts currentCumulativeParts;
		currentCumulativeParts.k.Set(1, 1, 1); //Default scale of 1;
		currentCumulativeParts.f = 1.0f;
		for (int i = 0; i < conts.Count(); i++)
		{
			if (conts[i] == nullptr ||
				fabs(weights[i]) < std::numeric_limits<float>::epsilon())
			{
				//skip if we have a bad controller
				//skip if we can consider this weight zero.
				continue;
			}

			// Pass in the current tm.
			Matrix3 localTm;
			if (sequentialMode == true)
			{
				// Chain mode, pass previous result in
				localTm = tm; //use the previous result as input to the next controller
				conts[i]->GetValue(t, &localTm, valid, CTRL_RELATIVE);
				localTm = localTm * Inverse(tm); //Remove the current cumulative running tm
			}
			else
			{
				// Fan mode, pass parent in
				localTm = parentTm; // use the parent as input to the next controller
				conts[i]->GetValue(t, &localTm, valid, CTRL_RELATIVE);
				localTm = localTm * Inverse(parentTm); //Remove the parent TM
			}

			// If there is scaling and rotation present, this decomposition can produce an incorrect rotation blending result.
			AffineParts localTmParts;
			decomp_affine(localTm, &localTmParts);
			localTmParts.q.Normalize();

			// Scale the affine parts to the attributed weight.
			const float averageWeight = AverageWeight(weights[i], t);

			Matrix3 prevTM;
			if(results != nullptr)
			{
				// Remember the previous tm so we can compensate for it and record the results.
				comp_affine(currentCumulativeParts, prevTM);
			}

			// sequentialMode: false = fan, true = chain
			//    False -> fan; weight localTmParts against identity then combine with previous results.
			//    True -> chain; move from previous cumilative TM towards current controllers tm.
			currentCumulativeParts = MatrixMath::BlendAffineParts(currentCumulativeParts, localTmParts,
				averageWeight/*weight for pos and scale*/,
				weights[i]/*legacy compat:weight for rotation always un-normalized*/,
				weightAgainstMode/*false=weight against identity. true=for rotation and scale, this behaves like a lerp from basis to target*/);

			if (sequentialMode == true || results != nullptr)
			{
				Matrix3 currentTM;
				comp_affine(currentCumulativeParts, currentTM);

				if (sequentialMode == true)
				{
					// Rebuild tm to update our current position to pass into the next controller.
					// reapply the parent to the current cumulative TM.
					tm = currentTM * parentTm;
				}
				if(results != nullptr)
				{
					// Record the intermediate results.
					(*results)[i] = currentTM * Inverse(prevTM);
				}
			}
		} //for

		// For non-sequentialMode (fan), (when not recording results) do this once at the end for a little more perf.
		if (sequentialMode == false)
		{
			// Rebuild tm to update our current position to pass into the next controller.
			Matrix3 cumulativeTm;
			comp_affine(currentCumulativeParts, cumulativeTm);

			// reapply the parent to the current cumulative TM.
			tm = cumulativeTm * parentTm;
		}
	}
	return tm;
}

void TransformListControl::GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method)
{
	if (method == CTRL_ABSOLUTE)
	{
		// KZ 20/09/2022: Other TM controllers do not handle CTRL_ABSOLUTE at all and return garbage data.
		// See PRSControl, it passes a Matrix3 into its component controls who expect respectively a Point3, Quat, ScaleValue.
		// Due to this oversight, we should bail. 
		DbgAssert(false);
		return;
	}

	Matrix3* val_tm = static_cast<Matrix3*>(val); //parent tm

	const BOOL indexMode = IsIndexMode();
	if (indexMode)
	{
		// In this mode we return only the selected index
		const int index = GetIndexModeIndex(t, valid);

		// Just the index, no weighting
		if(index >= 0 && index < conts.Count() && conts[index])
			conts[index]->GetValue(t, val_tm, valid, CTRL_RELATIVE);
		return; //We are done, treat tm_list like a switcher.
	}

	// Perform the weighting
	Matrix3 tm = PerformWeightedGetValue(t, *val_tm, valid, nullptr);

	// assign the value back
	*val_tm = tm;
}

void TransformListControl::SetValue(TimeValue t, void *val, int commit, GetSetMethod method)
{
	if (GetLocked() ||
		conts.Count() == 0) {
		return;
	}

	SetXFormPacket* packet = static_cast<SetXFormPacket*>(val);
	if(packet == nullptr) {
		return;
	}

	const BOOL indexMode = IsIndexMode();
	if (indexMode)
	{
		// Do nothing but pass on the set value
		const int index = GetIndexModeIndex(t);
		if (index >= 0 && index < conts.Count() && conts[index]) {
			conts[index]->SetValue(t, packet, commit, method);
		}
		return; // Easy done.
	}

	// In Weight mode, use that active index.
	const int activeIndex = GetWeightModeActive();
	if (activeIndex < 0 || activeIndex >= conts.Count())
		return;

	const Control* activeControl = GetSubCtrl(activeIndex);
	if (activeControl == nullptr) {
		DbgAssert(false);
		return;
	}
	
	switch (packet->command)
	{
	case XFORM_MOVE:
	{
		if (method == CTRL_ABSOLUTE)
		{
			// TODO: KZ, Seems unused by tm controllers, need to verify
			DbgAssert(false);
		}
		else
		{
			// KZ, Try this, not what the pos list does but math makes sense and feels right in vp.
			conts[activeIndex]->SetValue(t, packet, commit, method);
		}
		break;
	}
	case XFORM_ROTATE:
	{
		// Trying to copy what the rotation list controllers behavior
		if (method == CTRL_ABSOLUTE)
		{
			// TODO: KZ, Seems unused by tm controllers, need to verify
			DbgAssert(false);
		}
		else
		{
			// Perform a GetValue but accumulate the results so we can rebuild the space the user is manipulating in.
			// Copying what the rotation_list is doing, feels good in the viewport.
			std::vector<Matrix3> results;
			Interval dummyValid = FOREVER;
			Matrix3 tm = PerformWeightedGetValue(t, packet->tmParent, dummyValid, &results);
			DbgAssert(results.size() == conts.Count());
			Matrix3 beforeTm;
			for (int i = 0; i < activeIndex; i++) {
				beforeTm *= results[i];
			}
			
			AngAxis na = packet->aa;
			na.axis = VectorTransform(Inverse(beforeTm), na.axis);
			SetXFormPacket newPacket(na, packet->localOrigin, packet->tmParent,packet->tmAxis);
			conts[activeIndex]->SetValue(t, &newPacket, commit, method);
		}
		
		break;
	}
	case XFORM_SCALE:
	{
		if (method == CTRL_ABSOLUTE)
		{
			// TODO: KZ, Seems unused by tm controllers, need to verify
			DbgAssert(false);
		}
		else
		{
			//Just use the existing packet like the scale list does, feels ok in vp.
			conts[activeIndex]->SetValue(t, packet, commit, method);
		}
		break;
	}
	case XFORM_SET:
	{
		// Trying to copy what the rotation list controllers behavior
		if (method == CTRL_ABSOLUTE)
		{
			// Perform a GetValue but accumulate the results so we can rebuild the space the user is manipulating in.
			std::vector<Matrix3> results;
			Interval dummyValid = FOREVER;
			PerformWeightedGetValue(t, packet->tmParent, dummyValid, &results);
			DbgAssert(results.size() == conts.Count());
			Matrix3 beforeTm;
			for (int i = 0; i < activeIndex; i++) {
				beforeTm *= results[i];
			}
			Matrix3 afterTm;
			for (int i = activeIndex + 1; i < conts.Count(); i++) {
				afterTm *= results[i];
			}

			Matrix3 setTM = packet->tmAxis;
			setTM = Inverse(afterTm) * setTM * Inverse(beforeTm);
			SetXFormPacket newPacket(setTM, packet->tmParent);
			conts[activeIndex]->SetValue(t, &newPacket, commit, method);
		}
		else
		{
			// TODO: KZ, Seems unused by tm controllers, need to verify
			DbgAssert(false);
		}
		break;
	}
	}
}

ListControl* TransformListControl::DerivedClone()
{
	return new TransformListControl;
}

BOOL TransformListControl::ChangeParents(TimeValue t, const Matrix3& oldP, const Matrix3& newP, const Matrix3& tm)
{
	const int index = GetActive();
	if (index < 0 || index >= conts.Count() || conts[index] == nullptr)
		return FALSE;

	// The 'tm' argument is the current world TM.
	// We need to determine this without any blending so don't use the one provided.
	Matrix3 currentWorldTM = oldP;
	conts[index]->GetValue(t, &currentWorldTM, CTRL_RELATIVE);
	return conts[index]->ChangeParents(t, oldP, newP, currentWorldTM);
}

// Copy what the PRS control is doing, this should behave like a proxy for the sub tm control
BOOL TransformListControl::CreateLockKey(TimeValue t, int which)
{
	const int activeIndex = GetActive();
	if (!conts.Count() || conts[activeIndex] == nullptr)
		return FALSE;
	return conts[activeIndex]->CreateLockKey(t, which);
}

DWORD TransformListControl::GetInheritanceFlags()
{
	const int activeIndex = GetActive();
	if (!conts.Count() || conts[activeIndex] == nullptr)
		return FALSE;
	return conts[activeIndex]->GetInheritanceFlags();
}

BOOL TransformListControl::SetInheritanceFlags(DWORD f, BOOL keepPos)
{
	const int activeIndex = GetActive();
	if (!conts.Count() || conts[activeIndex] == nullptr)
		return FALSE;
	return conts[activeIndex]->SetInheritanceFlags(f, keepPos);
}

BOOL TransformListControl::CanCopyIKParams(int which)
{
	const int activeIndex = GetActive();
	if (!conts.Count() || conts[activeIndex] == nullptr)
		return FALSE;
	return conts[activeIndex]->CanCopyIKParams(which);
}

IKClipObject* TransformListControl::CopyIKParams(int which)
{
	const int activeIndex = GetActive();
	if (!conts.Count() || conts[activeIndex] == nullptr)
		return nullptr;
	return conts[activeIndex]->CopyIKParams(which);
}

BOOL TransformListControl::CanPasteIKParams(IKClipObject* co, int which)
{
	const int activeIndex = GetActive();
	if (!conts.Count() || conts[activeIndex] == nullptr)
		return FALSE;
	return conts[activeIndex]->CanPasteIKParams(co, which);
}

void TransformListControl::PasteIKParams(IKClipObject* co, int which)
{
	const int activeIndex = GetActive();
	if (conts.Count() != 0 && conts[activeIndex] != nullptr) {
		conts[activeIndex]->PasteIKParams(co, which);
	}
}

void TransformListControl::InitIKJoints(InitJointData* posData, InitJointData* rotData)
{
	const int activeIndex = GetActive();
	if (conts.Count() != 0 && conts[activeIndex] != nullptr) {
		conts[activeIndex]->InitIKJoints(posData, rotData);
	}
}

BOOL TransformListControl::GetIKJoints(InitJointData* posData, InitJointData* rotData)
{
	const int activeIndex = GetActive();
	if (conts.Count() != 0 && conts[activeIndex] != nullptr) {
		conts[activeIndex]->GetIKJoints(posData, rotData);
	}
	return FALSE;
}

void TransformListControl::InitIKJoints2(InitJointData2* posData, InitJointData2* rotData)
{
	const int activeIndex = GetActive();
	if (conts.Count() != 0 && conts[activeIndex] != nullptr) {
		conts[activeIndex]->InitIKJoints2(posData, rotData);
	}
}

BOOL TransformListControl::GetIKJoints2(InitJointData2* posData, InitJointData2* rotData)
{
	const int activeIndex = GetActive();
	if (conts.Count() != 0 && conts[activeIndex] != nullptr) {
		return conts[activeIndex]->GetIKJoints2(posData, rotData);
	}
	return FALSE;
}

void TransformListControl::MirrorIKConstraints(int axis, int which, BOOL pasteMirror)
{
	const int activeIndex = GetActive();
	if (conts.Count() != 0 && conts[activeIndex] != nullptr) {
		return conts[activeIndex]->MirrorIKConstraints(axis, which, pasteMirror);
	}
}

Control* TransformListControl::GetPositionController()
{
	const int activeIndex = GetActive();
	if (conts.Count() != 0 && conts[activeIndex] != nullptr) {
		return conts[activeIndex]->GetPositionController();
	}
	return nullptr;
}

Control* TransformListControl::GetRotationController()
{
	const int activeIndex = GetActive();
	if (conts.Count() != 0 && conts[activeIndex] != nullptr) {
		return conts[activeIndex]->GetRotationController();
	}
	return nullptr;
}

Control* TransformListControl::GetScaleController()
{
	const int activeIndex = GetActive();
	if (conts.Count() != 0 && conts[activeIndex] != nullptr) {
		return conts[activeIndex]->GetScaleController();
	}
	return nullptr;
}

BOOL TransformListControl::SetPositionController(Control* c)
{
	const int activeIndex = GetActive();
	if (conts.Count() != 0 && conts[activeIndex] != nullptr) {
		return conts[activeIndex]->SetPositionController(c);
	}
	return FALSE;
}

BOOL TransformListControl::SetRotationController(Control* c)
{
	const int activeIndex = GetActive();
	if (conts.Count() != 0 && conts[activeIndex] != nullptr) {
		return conts[activeIndex]->SetRotationController(c);
	}
	return FALSE;
}

BOOL TransformListControl::SetScaleController(Control* c)
{
	const int activeIndex = GetActive();
	if (conts.Count() != 0 && conts[activeIndex] != nullptr) {
		return conts[activeIndex]->SetScaleController(c);
	}
	return FALSE;
}

//keeps track of whether an FP interface desc has been added to the ClassDesc
static bool tmListInterfaceLoaded = false;

class TransformListClassDesc : public ListUIClassDesc
{
public:
	const TCHAR *	ClassName() override { return TMLIST_CNAME; }
	const TCHAR*	NonLocalizedClassName() override { return _T("Transform List"); }
	SClass_ID		SuperClassID() override { return CTRL_MATRIX3_CLASS_ID; }
	Class_ID		ClassID() override { return Class_ID(TMLIST_CONTROL_CLASS_ID, 0); }
	void *	Create(BOOL loading) override
	{
		if (!tmListInterfaceLoaded)
		{
			AddInterface(&listControlInterface);
			tmListInterfaceLoaded = true;
		}
		return new TransformListControl(loading);
	}
	const TCHAR* InternalName() override { return _T("TransformList"); } // returns fixed parsable name (script-visible name)
};
static TransformListClassDesc tmListCD;
ClassDesc* GetTransformListDesc() { return &tmListCD; }

void TransformListControl::Init()
{
	// make the paramblock
	tmListCD.MakeAutoParamBlocks(this);
}

ClassDesc* TransformListControl::GetClassDesc()
{
	return GetTransformListDesc();
}

// per instance list controller block
static ParamBlockDesc2 tm_list_paramblk(kListCtrlWeightParams, _T("Parameters"), 0, &tmListCD, P_AUTO_CONSTRUCT, PBLOCK_INIT_REF_ID,
	// params
	kListCtrlWeight, _T("weight"), TYPE_FLOAT_TAB, 0, P_ANIMATABLE + P_VARIABLE_SIZE + P_TV_SHOW_ALL + P_COMPUTED_NAME, IDS_AF_LIST_WEIGHT,
		p_default, 1.0,
		p_range, WEIGHT_MIN, WEIGHT_MAX,
		p_dim, stdPercentDim,
		p_accessor, &theListCtrlPBAccessor,
		p_end,

	kListCtrlAverage, _T("average"), TYPE_BOOL, 0, IDS_AF_LIST_AVERAGE,
		p_default, FALSE,
		p_end,

	kListCtrlIndex, _T("index"), TYPE_INT, P_RESET_DEFAULT, IDS_AF_LIST_INDEX,
		p_default, 0,
		p_range, 0, 1000, //use accessor to limit to weight table size.
		p_dim, &theIndexDim,
		p_accessor, & theListCtrlIndexPBAccessor,
		p_end,

	kListCtrlIndexMode, _T("indexMode"), TYPE_BOOL, 0, IDS_AF_LIST_INDEX_MODE,
		p_default, FALSE,
		p_accessor, & thetheListCtrlIndexModePBAccessor, //use accessor to manage edit params if that changes.
		p_end,

	// false = treat list as a fan, true = treat list as a chain
	kListCtrlSequential, _T("sequential"), TYPE_BOOL, 0, IDS_AF_LIST_SEQUENTIAL,
		p_default, FALSE,
		p_end,

	// false = weight against identity, true = LERP from previous
	kListCtrlWeightAgainstMode, _T("weightAgainstPrevious"), TYPE_BOOL, 0, IDS_AF_LIST_WEIGHT_AGAINST_PREVIOUS,
		p_default, FALSE,
		p_end,
	kListCtrlTag, _T("tag"), TYPE_STRING, 0, IDS_AF_LIST_TAG,
		p_default, _T(""),
		p_end,
	p_end
);

// clang-format on

//-------------------------------------------------------------------------

