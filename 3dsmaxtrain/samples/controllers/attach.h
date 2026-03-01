/**********************************************************************
*<
FILE: attach.h

DESCRIPTION: Attachment controller

CREATED BY: Rolf Berteig

HISTORY: 11/18/96

*>	Copyright (c) 1996, All Rights Reserved.
**********************************************************************/
// NOTE: included in maxsdk\samples\mxs\agni\lam_ctrl.cpp

#ifndef __ATTACHCTRL__H
#define __ATTACHCTRL__H

#include "ctrl.h"
#include <ILockedTracks.h>
#include <unordered_map >

class AKey
{
public:
	TimeValue time;
	DWORD flags;
	DWORD face;
	float u0, u1;
	float tens, cont, bias, easeIn, easeOut;

	Point3 pos, norm, din, dout, dir;
	Quat quat, qa, qb;

	AKey()
			: flags(0)
			, face(0)
			, u0(0.f)
			, u1(0.f)
			, tens(0.f)
			, cont(0.f)
			, bias(0.f)
			, easeIn(0.f)
			, easeOut(0.f)
			, pos(Point3(0, 0, 0))
			, din(Point3(0, 0, 0))
			, dout(Point3(0, 0, 0))
			, norm(Point3(1, 0, 0))
			, dir(Point3(1, 0, 0))
			, quat(IdentQuat())
			, qa(IdentQuat())
			, qb(IdentQuat())
	{
	}

	AKey(const AKey&) = default;
	AKey(AKey&&) = default;
	AKey& operator=(const AKey&) = default;
	AKey& operator=(AKey&&) = default;
};

class PickObjectMode;
class SetPosCMode;

class AttachCtrl : public StdControl, public TimeChangeCallback, public IAttachCtrl,
public ILockedTrackImp
{
public:
	// Controller data
	// these are the keys if the user animates the object across the surface
	// if the user only has one key then key[0] is continually updated and regardless of
	// time is stored in the first key.
	MaxSDK::Array<AKey> keys;  
	//whole frame time cache of the result since this can be a heavy weight controller
	std::unordered_map <TimeValue, Matrix3> mTimeCache;
	INode* node;
	BOOL rangeLinked, align, manUpdate, setPosButton;

	// Current value cache
	Interval range, mValid;
	Point3 val;
	Quat qval;
	BOOL trackValid, doManUpdate;

	static HWND hWnd;
	static IObjParam *ip;
	static AttachCtrl *editCont;
	static BOOL uiValid;
	static ISpinnerControl *iTime, *iFace, *iA, *iB, *iTens, *iCont, *iBias, *iEaseTo, *iEaseFrom;
	static ICustButton *iPickOb, *iSetPos, *iPrev, *iNext, *iUpdate;
	static ICustStatus *iStat;
	//this is the index of the active Key in keys ie the one up in the UI
	static int mActiveUIKeyIndex;
	static PickObjectMode *pickObMode;
	static SetPosCMode *setPosMode;

	AttachCtrl();

	// Animatable methods
	void DeleteThis() {delete this;}		
	int IsKeyable() {return 1;}
	BOOL IsAnimated() {return (keys.length() > 0); }
	Class_ID ClassID() {return ATTACH_CONTROL_CLASS_ID;}
	SClass_ID SuperClassID() {return CTRL_POSITION_CLASS_ID;}
	void GetClassName(MSTR& s, bool localized) const override;
	void BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev); 
	void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next); 

	// Animatable's Schematic View methods
	SvGraphNodeReference SvTraverseAnimGraph(IGraphObjectManager *gom, Animatable *owner, int id, DWORD flags);
	TSTR SvGetRelTip(IGraphObjectManager *gom, IGraphNode *gNodeTarger, int id, IGraphNode *gNodeMaker);
	bool SvCanDetachRel(IGraphObjectManager *gom, IGraphNode *gNodeTarget, int id, IGraphNode *gNodeMaker);
	bool SvDetachRel(IGraphObjectManager *gom, IGraphNode *gNodeTarget, int id, IGraphNode *gNodeMaker);
	bool SvLinkChild(IGraphObjectManager *gom, IGraphNode *gNodeThis, IGraphNode *gNodeChild);
	bool SvCanConcludeLink(IGraphObjectManager *gom, IGraphNode *gNode, IGraphNode *gNodeChild);

	// Reference methods
	RefResult NotifyRefChanged(const Interval&, RefTargetHandle, PartID&, RefMessage, BOOL);
	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);
	RefTargetHandle Clone(RemapDir &remap);
	int NumRefs() {return Control::NumRefs()+1;};	
	RefTargetHandle GetReference(int i);
private:
	virtual void SetReference(int i, RefTargetHandle rtarg);
	//just a usefule methods that prints out the state of the controller variables
	void DumpState();
public:

	// Control methods				
	void Copy(Control *from);
	BOOL IsLeaf() {return TRUE;}
	BOOL CanApplyEaseMultCurves(){return !GetLocked();}
	void CommitValue(TimeValue t) {}
	void RestoreValue(TimeValue t) {}
	BOOL IsReplaceable() {return !GetLocked();}  
	// Animatable methods
	Interval GetTimeRange(DWORD flags);
	void EditTimeRange(Interval range,DWORD flags);
	void MapKeys(TimeMap *map,DWORD flags );		
	int NumKeys() {return (int)keys.length();}
	TimeValue GetKeyTime(int index) {return keys[index].time;}
	int GetKeyIndex(TimeValue t);		
	void DeleteKeyAtTime(TimeValue t);
	BOOL IsKeyAtTime(TimeValue t,DWORD flags);				
	int GetKeyTimes(Tab<TimeValue> &times,Interval range,DWORD flags);
	int GetKeySelState(BitArray &sel,Interval range,DWORD flags);
	BOOL GetNextKeyTime(TimeValue t,DWORD flags,TimeValue &nt);
	void DeleteTime(Interval iv, DWORD flags);
	void ReverseTime(Interval iv, DWORD flags);
	void ScaleTime(Interval iv, float s);
	void InsertTime(TimeValue ins, TimeValue amount);
	BOOL SupportTimeOperations() {return TRUE;}
	void DeleteKeys(DWORD flags);
	void DeleteKeyByIndex(int index);
	void SelectKeys(TrackHitTab& sel, DWORD flags);
	void SelectKeyByIndex(int i,BOOL sel);
	void FlagKey(TrackHitRecord hit);
	int GetFlagKeyIndex();
	int NumSelKeys();
	void CloneSelectedKeys(BOOL offset=FALSE);
	void AddNewKey(TimeValue t,DWORD flags);		
	BOOL IsKeySelected(int index);
	BOOL CanCopyTrack(Interval iv, DWORD flags) {return TRUE;}
	BOOL CanPasteTrack(TrackClipObject *cobj,Interval iv, DWORD flags) {return cobj->ClassID()==ClassID();}
	TrackClipObject *CopyTrack(Interval iv, DWORD flags);
	void PasteTrack(TrackClipObject *cobj,Interval iv, DWORD flags);
	int HitTestTrack(			
		TrackHitTab& hits,
		Rect& rcHit,
		Rect& rcTrack,			
		float zoom,
		int scroll,
		DWORD flags);
	int PaintTrack(
		ParamDimensionBase *dim,
		HDC hdc,
		Rect& rcTrack,
		Rect& rcPaint,
		float zoom,
		int scroll,
		DWORD flags);
	void EditTrackParams(
		TimeValue t,
		ParamDimensionBase *dim,
		const TCHAR *pname,
		HWND hParent,
		IObjParam *ip,
		DWORD flags);
	int TrackParamsType() {if(GetLocked()==false) return TRACKPARAMS_KEY; else return TRACKPARAMS_NONE;}

	// StdControl methods
	void GetValueLocalTime(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE) {}
	void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);
	void SetValueLocalTime(TimeValue t, void *val, int commit, GetSetMethod method) {}
	void Extrapolate(Interval range,TimeValue t,void *val,Interval &valid,int type) {}
	void* CreateTempValue() {return new Point3;}
	void DeleteTempValue(void *val) {delete (Point3*)val;}
	void ApplyValue(void *val, void *delta) {((Matrix3*)val)->PreTranslate(*((Point3*)delta));}
	void MultiplyValue(void *val, float m) {*((Point3*)val) *= m;}

	// TimeChangedCallback
	void TimeChanged(TimeValue t) {InvalidateUI();}

	// IAttachCtrl
	BOOL SetObject(INode *node);
	INode* GetObject();
	void SetKeyPos(TimeValue t, DWORD fi, Point3 bary);
	AKey* GetKey(int index);
	BOOL GetAlign();
	void SetAlign(BOOL align);
	BOOL GetManualUpdate();
	void SetManualUpdate(BOOL manUpdate);
	void Invalidate(BOOL forceUpdate = FALSE);

	// BaseInterface
	Interface_ID GetID() { return I_ATTACHCTRL; }

	// Local methods
	//sorts the keys 
	void SortKeys();
	//sorts the keys and removes duplicates
	//we can get duplicate 2 keys 2 ways.  First by selecting and dragging a key over the top of another one
	//Second by setting the active keys time to the value of an existing key.
	// fromUISpinner used to determine which key of the duplicates to keep.  If set true it uses the mActiveUIKeyIndex
	// to determine which key to delete  ( this should be called when the Time UI spinner is changed, 
	// otherwise it usese the flag on the keys KEY_SELECTED to determine
	void SortAndDeleteDuplicateKeys(bool fromUISpinner = false);
	void HoldTrack();
	void PrepareTrack(TimeValue t);
	float GetInterpVal(TimeValue t,size_t& n0, size_t& n1);
	Point3 PointOnPath(TimeValue t);
	Quat QuatOnPath(TimeValue t);		

	Interval CompValidity(TimeValue t);

	void CompFirstDeriv();
	void CompLastDeriv();
	void Comp2KeyDeriv();
	void CompMiddleDeriv(int i);
	void CompAB(int i);

	void InvalidateUI();
	void UpdateUI();
	void UpdateTCBGraph();
	void UpdateBaryGraph();
	void SetupWindow(HWND hWnd);
	void DestroyWindow();
	void SpinnerChange(int id);
	void Command(int id, LPARAM lParam);

	BaseInterface* GetInterface(Interface_ID id)
	{
		if (id==I_ATTACHCTRL)
			return static_cast<IAttachCtrl*>(this);
		else 
			return StdControl::GetInterface(id);
	}

	void* GetInterface(ULONG id)
	{
		switch (id) {
			case I_LOCKED:
					return (ILockedTrackImp*) this;
			}
		return StdControl::GetInterface(id);
	}


};


#endif // __ATTACHCTRL__H
