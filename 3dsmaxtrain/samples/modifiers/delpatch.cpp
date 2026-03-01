/**********************************************************************
 *<
	FILE: delpatch.cpp

	DESCRIPTION:  A deletion modifier for patches

	CREATED BY: Tom Hudson

	HISTORY: 4/10/2000

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#include "mods.h"
#include "iparamm.h"


#define DELETE_PATCH_CLASS_ID 0x7F34cc2a

class DeletePatchMod : public Modifier {	
	public:		
		DeletePatchMod();

		// From Animatable
		void DeleteThis() { delete this; }
		void GetClassName(MSTR& s, bool localized) const override {s = localized ? GetString(IDS_TH_DELETEPATCHMOD) : _T("DeletePatch");}  
		virtual Class_ID ClassID() { return Class_ID(DELETE_PATCH_CLASS_ID,0);}		
		RefTargetHandle Clone(RemapDir& remap);
		const TCHAR *GetObjectName(bool localized) const override {return localized ? GetString(IDS_TH_DELETEPATCHMOD) : _T("DeletePatch");}

		// From modifier
		ChannelMask ChannelsUsed()  {return GEOM_CHANNEL|TOPO_CHANNEL|SELECT_CHANNEL|SUBSEL_TYPE_CHANNEL|VERTCOLOR_CHANNEL|TEXMAP_CHANNEL;}
		ChannelMask ChannelsChanged() {return GEOM_CHANNEL|TOPO_CHANNEL|SELECT_CHANNEL|VERTCOLOR_CHANNEL|TEXMAP_CHANNEL;}
		Class_ID InputType() { return Class_ID(PATCHOBJ_CLASS_ID,0); }
		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
		Interval LocalValidity(TimeValue t) {return FOREVER;}

		// From BaseObject
		CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;} 
		
		int NumRefs() {return 0;}
		RefTargetHandle GetReference(int i) {return NULL;}
private:
		virtual void SetReference(int i, RefTargetHandle rtarg) {}
public:

		int NumSubs() {return 0;}
		Animatable* SubAnim(int i) {return NULL;}
		TSTR SubAnimName(int i, bool localized) override {return _T("");}

		RefResult NotifyRefChanged( const Interval& changeInt,RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message, BOOL propagate) {return REF_SUCCEED;}
	};


//--- ClassDescriptor and class vars ---------------------------------

class DeletePatchClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void*			Create(BOOL loading = FALSE) { return new DeletePatchMod; }
	const TCHAR*	ClassName() { return GetString(IDS_TH_DELETEPATCHMOD); }
	const TCHAR*	NonLocalizedClassName() { return _T("DeletePatch"); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(DELETE_PATCH_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetString(IDS_RB_DEFDEFORMATIONS);}
	};

static DeletePatchClassDesc deleteDesc;
ClassDesc* GetDeletePatchModDesc() {return &deleteDesc;}


//--- Delete patch mod methods -------------------------------

DeletePatchMod::DeletePatchMod()
	{

	}

RefTargetHandle DeletePatchMod::Clone(RemapDir& remap)
	{
	DeletePatchMod *mod = new DeletePatchMod;
	BaseClone(this, mod, remap);
	return mod;
	}

void DeletePatchMod::ModifyObject(
		TimeValue t, ModContext &mc, ObjectState *os, INode *node)
	{
	if(os->obj->ClassID() == Class_ID(PATCHOBJ_CLASS_ID,0))
		((PatchObject *)os->obj)->DoDeleteSelected(FALSE);
	}
