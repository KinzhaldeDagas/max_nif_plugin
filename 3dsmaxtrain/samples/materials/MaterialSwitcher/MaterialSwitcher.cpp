/**********************************************************************
 *<
   FILE: MaterialSwitcher.cpp

   DESCRIPTION:  Material Switcher

   CREATED BY: Mathew Kaustinen

   HISTORY: Created 4 August 2022

 *>   Copyright (c) 2022 Autodesk, Inc.
 **********************************************************************/

#include "MaterialSwitcher.h"
#include <Materials/MaterialSwitcherInterface.h>
#include <maxicon.h>
#include <winutil.h>

// Compatibility and Wrapper methods
#include "IRefTargWrappingRefTarg.h"
#include "IMtlRender_Compatibility.h"

// Build options for R&D
// Turned ON in current design
#define HIDE_SCROLL_WHEN_NOT_NEEDED
// Turned OFF in current design
//#define ANIMATIBLE_SWITCHER_ID // For initial version, this is disabled due to Renderer and Viewport issues
//#define ALLOW_NAME_SEARCH	 // Name is read only for now
//#define TEXMAP_INDEX

#define TIMENOW GetCOREInterface()->GetTime()
#define REDRAWVIEWS() GetCOREInterface()->RedrawViews(TIMENOW)
#define SUBHILIGHT_COLOR (RGB(80, 132, 178))

#define MTL_SWITCHER_MXS_INTERFACE Interface_ID(0x4ecd74a6, 0x1)

extern HINSTANCE hInstance;

const int PBLOCK_REF = 0;
const int SWITCHER_VERSION = 100;

const TSTR UpTriangleSymbolWithLeadingSpace(TSTR::FromUTF8(" \xe2\x96\xB2")); // Black Up-Pointing Triangle
const TSTR DownTriangleSymbolWithLeadingSpace(TSTR::FromUTF8(" \xe2\x96\xBC")); // Black Down-Pointing Triangle


#define MAX_NAME_STRING 256	// We need to use TCHARs with our sort routine this is how many Characters we use
#define MAX_MATERIALS 9999	// Max material slots we support
#define UI_MATERIAL_COUNT 30 // How Many Material Slots Shown in UI
#define INITIAL_MATERIAL_COUNT 10 // How Many Materials to Start With
#define	ACTIVE_STAMP 0XFFFF // ACTIVE STAMP ID, this is the ID we give the stamp in the upper right corner


class MtlSwitcher;

/*===========================================================================*\
 |	Dialog Processor
\*===========================================================================*/

class MtlSwitcherDlgProc
		: public ParamMap2UserDlgProc
		, public ParamDlg
{
public:

	// Our Active Sorting Type
	enum
	{
		SORT_INDEX_ASCENDING,
		SORT_INDEX_DESCENDING,
		SORT_NAME_ASCENDING,
		SORT_NAME_DESCENDING,
		SORT_MATERIAL_ASCENDING,
		SORT_MATERIAL_DESCENDING
	};

	HWND hwmedit; // window handle of the materials editor dialog
	IMtlParams* ip;
	MtlSwitcher* theMtl; // current mtl being edited.
	HWND hMaterialPanel; // Rollup pane
	HWND hScroll;
	TimeValue curTime;
	BOOL valid;
	ICustButton* iMtlButton[UI_MATERIAL_COUNT];
	ICustEdit* iName[UI_MATERIAL_COUNT];
	ICustStatus* iIndex[UI_MATERIAL_COUNT];
#ifdef ALLOW_NAME_SEARCH
	ICustEdit* iNameSearch;
#endif

	MtlDADMgr dadMgr;
	int rollupHeight;
	int frameHeight;
	int scrollHeight;

	int subSelection;	// this is the selection when UI is locked

	int CurrentSortingType = SORT_INDEX_ASCENDING;

	ToolTipExtender tooltips;

	MtlSwitcherDlgProc(MtlSwitcher* m, HWND hwMtlEdit, IMtlParams* imp);

	~MtlSwitcherDlgProc();

	INT_PTR DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	HIMAGELIST GetLockIcons()
	{
		// Icons for Lock/Unlock
		static HIMAGELIST hLockIcons = nullptr;
		if (hLockIcons == nullptr)
		{
			hLockIcons = ImageList_Create(MaxSDK::UIScaled(16), MaxSDK::UIScaled(15), ILC_MASK, 2, 0);
			HBITMAP hBitmap = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_NEW_LOCK));
			HBITMAP hMask = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_NEW_LOCK_ALPHA));
			HBITMAP scaledBitmap = MaxSDK::GetUIScaledBitmap(hBitmap);
			if (scaledBitmap)
			{
				DeleteObject(hBitmap);
				hBitmap = scaledBitmap;
			}
			HBITMAP scaledMask = MaxSDK::GetUIScaledBitmap(hMask);
			if (scaledMask)
			{
				DeleteObject(hMask);
				hMask = scaledMask;
			}
			ImageList_Add(hLockIcons, hBitmap, hMask);
			DeleteObject(hBitmap);
			DeleteObject(hMask);

			hBitmap = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_NEW_OPEN));
			hMask = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_NEW_OPEN_ALPHA));
			scaledBitmap = MaxSDK::GetUIScaledBitmap(hBitmap);
			if (scaledBitmap)
			{
				DeleteObject(hBitmap);
				hBitmap = scaledBitmap;
			}
			scaledMask = MaxSDK::GetUIScaledBitmap(hMask);
			if (scaledMask)
			{
				DeleteObject(hMask);
				hMask = scaledMask;
			}
			ImageList_Add(hLockIcons, hBitmap, hMask);
			DeleteObject(hBitmap);
			DeleteObject(hMask);
		}
		return hLockIcons;
	}

	void InitDialog(HWND hDlg);
	void SetStaticTooltips();
	void SetDynamicTooltips();
	void InitSortHeaders();
	void SetSortIndexLabel(bool bClear);
	void SetSortNameLabel(bool bClear);
	void SetSortMaterialLabel(bool bClear);
	void UpdateSortLabels(int newType);
	void OnSortIndex();
	void OnSortName();
	void OnSortMaterial();
	void OnInsert();
	void OnDelete();
	void OnLock();
	void OnNameSearch(const TCHAR* name);
	void ScrollToActiveMaterial();
	void UpdateSelectedLabel();
	bool MayUpdateSubSelectionOnDelete(int index)
	{
		return (index && index < subSelection);
	}

	void SetUpStamp(HWND hWnd, HWND hStamp, HWND hIndex);
	void SetUpActiveStamp(HWND hWnd, HWND hStamp);

	void VScroll(int code, short int cpos);
	BOOL WheelScroll(short int position);
	bool ScrollingNeeded();
	void UpdateScrollBar();

#ifdef TEXMAP_INDEX
	void UpdateSelectionUI();
#endif
	void UpdateNumMatSpinner();

	void DrawPStampBlackBorder(HDC hdc, BOOL onSub, Rect& rect);
	void DrawPStampHilite(int i, BOOL on, BOOL onSub);
	void DrawPStampHilite(HDC hdc, BOOL on, BOOL onSub, Rect & rect);
	void DrawPStamp(HDC hdc, Rect& rect, int i);
	void DrawPStamp(HDC hdc, Rect& rect, Mtl* m, bool bHighlight, bool bSubHighlight, bool bIsActiveStamp);
	void RemovePStampHilite();
	void HighlightActiveStamp();
	void SelectMtl(int i);
	void UpdateSubMtlNames();
	void LoadDialog();
	void RefreshActiveStamp()
	{
		if (hMaterialPanel)
			InvalidateRect(GetDlgItem(hMaterialPanel, IDC_PSTAMP_ACTIVE), NULL, TRUE);
	}

	void UpdateMtlDisplay()
	{
		if (ip)
			ip->MtlChanged();
	}

	int SubMtlNumFromNameID(int id);

	// ******************* ParamDlg Methods *************************
	// methods inherited from ParamDlg:
	Class_ID ClassID() override
	{
		return MATERIAL_SWITCHER_CLASS_ID;
	}

	void SetThing(ReferenceTarget* m) override;

	ReferenceTarget* GetThing() override
	{
		return (ReferenceTarget*)theMtl;
	}

	void DeleteThis() override
	{
		delete this;
	}

	void ReloadDialog() override
	{
		LoadDialog();
	}

	void ActivateDlg(BOOL /*onOff*/) override
	{
		// do nothing for now
	}

	int FindSubMtlFromHWND(HWND hWnd) override;

	void SetTime(TimeValue t) override
	{
		if (t != curTime)
		{
			curTime = t;
			HighlightActiveStamp();
			UpdateSelectedLabel();
		}
	}
};


//-----------------------------------------------------------------------------
//  Material Switcher
//-----------------------------------------------------------------------------

// Parameter and ParamBlock IDs
enum
{
	switcher_params,
}; // pblock ID

enum // switcher_params param IDs
{
	pb_materials,
	pb_mtl_labels,
	pb_switcher_selection,
	pb_num_mats,		// UI and Read Only
	pb_locked,			// Is our selection locked in the UI
#ifdef TEXMAP_INDEX
	pb_index_map,
#else
	pb_index_map_obs,
#endif
};

class MtlSwitcher
		: public Mtl
		, public IReshading
		, public MaxSDK::MtlSwitcherInterface
		, public IRefTargWrappingRefTarg
		, public FPMixinInterface
		, public TimeChangeCallback
{
	friend class MtlSwitcherDlgProc;
	friend class DelSubRestore;
	friend class AddMtlsRestore;
	friend class DeleteMtlsRestore;
	friend class InsertMtlRestore;

	// Animatable parameters
	ReshadeRequirements mReshadeRQ;

	// Dialog Items
	MtlSwitcherDlgProc* paramDlg;

public:

	IntTab sortedIndexes;	// The Indexes in order of the current sort

	int mPStampSize = PS_TINY;
	int mActivePStampSize = PS_SMALL;
	int offset;
	int lastActiveIndex;

	BOOL ignoreNotify;
	BOOL isBatchChange;
	INodeTab nodeTab;

	IParamBlock2* pblock; // ref #0

	// FPInterface
	enum // MXS commands
	{
		fnIdGetActiveMaterial,
		fnIdSelectMaterialByID,
		fnIdSelectMaterialByName,
		fnIdSelectedName,
	};

	static FPInterfaceDesc	mFPInterfaceDesc;
	FPInterfaceDesc* GetDesc() override { return &mFPInterfaceDesc; }
	FPInterfaceDesc* GetDescByID(Interface_ID id) override {
		if (MTL_SWITCHER_MXS_INTERFACE == id)
			return &mFPInterfaceDesc;

		return FPMixinInterface::GetDescByID(id);
	}

	//DECLARE_DESCRIPTOR - Needed for static interfaces
	BEGIN_FUNCTION_MAP
	FN_0(fnIdGetActiveMaterial, TYPE_MTL, GetActiveMtl);
	FN_1(fnIdSelectMaterialByID, TYPE_MTL, SelectMaterialByID, TYPE_INT);
	FN_1(fnIdSelectMaterialByName, TYPE_MTL, SelectMaterialByName, TYPE_STRING);
	RO_PROP_FN(fnIdSelectedName, GetSelectedName, TYPE_TSTR_BV);
	END_FUNCTION_MAP

	// 

	void TimeChanged(TimeValue t) override
	{
#ifdef TEXMAP_INDEX
		Texmap* tm = pblock->GetTexmap(pb_index_map, 0);
		if (tm)
		{
			int current = pblock->GetInt(pb_switcher_selection) - 1;
			int newValue = GetActiveMtlIndex();
			if (newValue != current)
			{
				macroRec->Disable();
				SetActiveMtlIndex(newValue, FALSE);
				macroRec->Enable();
			}
		}
#endif

		int newActiveIndex = GetActiveMtlIndex();
		if (lastActiveIndex != newActiveIndex)
		{
			lastActiveIndex = newActiveIndex;
			if (paramDlg)
			{
				paramDlg->HighlightActiveStamp();
				paramDlg->RefreshActiveStamp();
			}
		}
	}


	// IRefTargWrappingRefTarg
	static ReferenceTarget* GetWrappedObject(ReferenceTarget* wrappingObject, bool recurse)
	{
		if (wrappingObject)
		{
			if ((wrappingObject->SuperClassID() == MATERIAL_CLASS_ID) &&
					(wrappingObject->ClassID() == MATERIAL_SWITCHER_CLASS_ID))
			{
				MtlSwitcher* thisMtl = (MtlSwitcher*)wrappingObject;
				if (thisMtl)
				{
					Mtl* activeMtl = thisMtl->GetActiveMtl();
					if (recurse && activeMtl)
					{
						return GetWrappedObject(activeMtl, true);
					}
					return activeMtl;
				}
			}
		}
		return nullptr;
	}

	virtual ReferenceTarget* GetWrappedObject(bool recurse) const override
	{
		Interval iv;
		Mtl* activeMtl = GetActiveMtlConst(TIMENOW, iv);
		if (recurse && activeMtl && (activeMtl->ClassID() == MATERIAL_SWITCHER_CLASS_ID))
		{
			return GetWrappedObject(activeMtl, true);
		}
		return activeMtl;
	}

	void AddMtl(ReferenceTarget* rt = nullptr, int id = 0, const TCHAR* name = nullptr);
	void InsertMtl(int index);
	void DeleteLastMtl();
	void DeleteMtl(int index);
#ifdef TEXMAP_INDEX
	bool HasIndexMap()
	{
		return (pblock->GetTexmap(pb_index_map) != nullptr);
	}
#endif

	void SortMtls(bool bUseCompareFunc, bool bDescending, CompareFnc cmp);
	void SortMtlsByName(bool bDescending);
	void SortMtlsByID(bool bDesending);
	void SortMtlsBySlotName(bool bDescending);
	Mtl* SelectMaterialByName(TSTR name);
	Mtl* SelectMaterialByID(int index);
	TSTR GetSelectedName();
	void SetNumSubMtls(int n);
	void BulkSetNumSubMtls(int n);
	void SetSubMtlAndName(int mtlid, Mtl* m, TSTR& subMtlName);
	void ClampOffset();
	void ReloadDialog()
	{
		if (paramDlg)
			paramDlg->ReloadDialog();
	}

	// Routines to manage sorted indexes
	int SortedOrderCount()
	{
		return sortedIndexes.Count();
	}

	void InsertSortedIndex(int materialindex)
	{
		 int insertion_location = 0;
		int num = sortedIndexes.Count();

		 for (int i = 0; i < num; i++)
		{
			int val = sortedIndexes[i];
			if (val == materialindex)
				insertion_location = i;
			if (val >= materialindex)
			{
				val++;
				sortedIndexes[i] = val;
			}
		}
		sortedIndexes.Insert(insertion_location, 1, &materialindex);
	}

	void InsertSortedIndex(int location, int materialindex)
	{
		// Careful, we need to update the following indexes
		sortedIndexes.Insert(location, 1, &materialindex);
		int count = sortedIndexes.Count();
		for (int i = 0; i < count; i++)
		{
			if (i != location)
			{
				int currentValue = sortedIndexes[i];
				if (currentValue >= materialindex)
				{
					currentValue++;
					sortedIndexes[i] = currentValue;
				}
			}
		}
	}

	void PopulateSortedIndex(int count = -1)
	{
		if (count < 1)
			count = pblock->Count(pb_materials);

		sortedIndexes.SetCount(count);
		for (int i = 0; i < count; i++)
		{
			sortedIndexes[i] = i+1;
		}
	}

	void ResizeSortedIndex(int newSize)
	{
		int origSize = sortedIndexes.Count();
		if (newSize == origSize)
			return;

		if (newSize > origSize)
		{
			sortedIndexes.SetCount(newSize);
			for (int i = origSize; i < newSize; i++)
			{
				sortedIndexes[i] = i + 1;
			}
		}
		else
		{
			// Possible optimization opportunity here
			for (int i = origSize - 1; i >= newSize; i--)
			{
				DeleteLastSortedIndex();
			}
		}
	}

	// Delete the entry that matches the order index given, return the actual index
	int FindAndDeleteSortedIndex(int orderIndex)
	{
		int count = sortedIndexes.Count();
		int deleteIndex = 0;

		// Find the one to delete, update the values for the others
		for (int i = 0; i < count; i++)
		{
			int val = sortedIndexes[i];
			if (val == orderIndex)
				deleteIndex = i;
			if (val > orderIndex)
			{
				val--;
				sortedIndexes[i] = val;
			}
		}

		sortedIndexes.Delete(deleteIndex, 1);

		return deleteIndex;
	}

	// Delete the last index, reset remaining values
	void DeleteLastSortedIndex()
	{
		int lastEntry = sortedIndexes.Count() - 1;
		if (lastEntry >= 0)
		{
			int materialIndex = sortedIndexes[lastEntry];
			sortedIndexes.Delete(lastEntry, 1);
			for (int i = 0; i < lastEntry; i++)
			{
				int thisIndex = sortedIndexes[i];
				if (thisIndex >= materialIndex)
					sortedIndexes[i] = thisIndex - 1;
			}
		}
	}

	// Delete the index, reset remaining values
	void DeleteSortedIndex(int i)
	{
		if ((i >= 0) && (i < sortedIndexes.Count()))
		{
			int materialIndex = sortedIndexes[i];
			sortedIndexes.Delete(i, 1);

			int count = sortedIndexes.Count();
			for (int i = 0; i < count; i++)
			{
				int thisIndex = sortedIndexes[i];
				if (thisIndex >= materialIndex)
					sortedIndexes[i] = thisIndex - 1;
			}
		}
	}

	int GetSortedEntry(int i)
	{
		if ((i >= 0) && (i < sortedIndexes.Count()))
			return sortedIndexes[i];

		return 0;
	}

	void SetSortedEntry(int i, int value)
	{
		if ((i >= 0) && (i < sortedIndexes.Count()))
			sortedIndexes[i] = value;
	}

	int GetSortOrderFromMtlIndex(int index)
	{
		int iSortedIndex = 0;
		int count = sortedIndexes.Count();
		int index1 = index + 1;
		for (int i = 0; i < count; i++)
		{
			int thisIndex = sortedIndexes[i];
			if (thisIndex == index1)
			{
				iSortedIndex = i;
				break;
			}
		}
		return iSortedIndex;
	}

	int GetMtlIndexFromSortOrder(int index)
	{
		int mtlIndex = 0;
		if (index >= 0 && index < sortedIndexes.Count())
		{
			mtlIndex = sortedIndexes[index] - 1;
		}
		return mtlIndex;
	}

	Sampler* GetPixelSampler(int mtlNum, BOOL backFace) override;

	void SetAmbient(Color c, TimeValue t) override
	{
	}
	void SetDiffuse(Color c, TimeValue t) override
	{
	}
	void SetSpecular(Color c, TimeValue t) override
	{
	}
	void SetShininess(float v, TimeValue t) override
	{
	}

	Color GetAmbient(int mtlNum = 0, BOOL backFace = FALSE) override;
	Color GetDiffuse(int mtlNum = 0, BOOL backFace = FALSE) override;
	Color GetSpecular(int mtlNum = 0, BOOL backFace = FALSE) override;
	float GetXParency(int mtlNum = 0, BOOL backFace = FALSE) override;
	float GetShininess(int mtlNum = 0, BOOL backFace = FALSE) override;
	float GetShinStr(int mtlNum = 0, BOOL backFace = FALSE) override;
	float WireSize(int mtlNum = 0, BOOL backFace = FALSE) override;

	MtlSwitcher(BOOL loading);
	~MtlSwitcher();
	void SetParamDlg(MtlSwitcherDlgProc* msdp)
	{
		paramDlg = msdp;
	}
	ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams* imp) override;
	void Shade(ShadeContext& sc) override;
	float EvalDisplacement(ShadeContext& sc) override;
	Interval DisplacementValidity(TimeValue t) override;
	void Update(TimeValue t, Interval& valid) override;
	void Init();
	void Reset() override;
	Interval Validity(TimeValue t) override;
	void NotifyChanged();

	Class_ID ClassID() override
	{
		return MATERIAL_SWITCHER_CLASS_ID;
	}
	SClass_ID SuperClassID() override
	{
		return MATERIAL_CLASS_ID;
	}
	void GetClassName(MSTR& s, bool localized) const override
	{
		s = localized ? GetString(IDS_MATERIAL_SWITCHER_CLASSNAME) : _T("Material Switcher");
	}

	void DeleteThis() override
	{
		delete this;
	}

#ifdef TEXMAP_INDEX
	int NumSubTexmaps() override
	{
		return 1;
	}

	Texmap* GetSubTexmap(int i) override
	{
		if (i == 0)
			return pblock->GetTexmap(pb_index_map);

		return nullptr;
	}

	void SetSubTexmap(int i, Texmap* m) override
	{
		if (i == 0)
			pblock->SetValue(pb_index_map, 0, m);
	}

	TSTR GetSubTexmapSlotName(int i, bool localized) override
	{
		if (i==0)
			return _T("Index Map");

		return _T("");
	}
#endif

	int NumSubMtls() override
	{
		if (isBatchChange)
			return 0;

		return pblock->Count(pb_materials);
	}

	// From MtlSwitcherInterface
	Mtl* GetActiveMtl() override 
	{
		Interval iv;
		return GetActiveMtlConst(TIMENOW, iv);
	}

	Mtl* GetActiveMtl(TimeValue t, Interval& valid) override
	{
		return GetActiveMtlConst(t, valid);
	}

	Mtl* GetActiveMtlConst(TimeValue t, Interval& valid) const
	{
		int index = GetActiveMtlIndex(t, valid);
		if ((index >= 0) && (index < pblock->Count(pb_materials)))
			return pblock->GetMtl(pb_materials, 0, index);

		return nullptr;
	}

	void SetActiveMtlIndex(int i, BOOL bRedraw = TRUE)
	{
		if (i < 0 || i >= NumSubMtls())
			return;

		pblock->SetValue(pb_switcher_selection, TIMENOW, i+1);
		
		if (paramDlg)
			paramDlg->UpdateSelectedLabel();

		if (bRedraw)
			REDRAWVIEWS();
	}

	int GetActiveMtlIndex() const
	{
		Interval iv;
		return GetActiveMtlIndex(TIMENOW, iv);
	}

	int GetActiveMtlIndex(TimeValue t, Interval &iv) const
	{
		int index = 0;
#ifdef TEXMAP_INDEX
		if (GetTexMapIndex(t, index, iv))
			return index-1;
#endif
		pblock->GetValue(pb_switcher_selection, t, index, iv);
		if (index > 0)
		{
			if (index >= pblock->Count(pb_materials))
				return pblock->Count(pb_materials) - 1;

			return index - 1;
		}

		return 0;
	}

	bool IsUILocked()
	{
		return (pblock->GetInt(pb_locked) != 0);
	}

#ifdef TEXMAP_INDEX
	bool GetTexMapIndex(TimeValue t, int &index, Interval &iv) const
	{
		Texmap* tm = pblock->GetTexmap(pb_index_map, t);
		if (!tm)
			return false;

		SCTex sc;
		sc.curTime = t;

		index = (int)(tm->EvalMono(sc) + 0.5f);
		if (index < 1)
			index = 1;
		if (index > pblock->Count(pb_materials))
			index = pblock->Count(pb_materials) - 1;

		iv = tm->Validity(t);

		return true;
	}
#endif

	Mtl* GetSubMtl(int i) override
	{
		if (i < 0 || (i >= NumSubMtls()))
			return nullptr;

		return pblock->GetMtl(pb_materials, 0, i);
	}


	void SetSubMtl(int i, Mtl* m) override;
	TSTR GetSubMtlSlotName(int i, bool localized) override;

	BOOL IsMultiMtl() override
	{
		return FALSE;
	}

	BOOL SupportsMultiMapsInViewport() override
	{
		return TRUE;
	}

	int NumSubs() override
	{
#ifdef TEXMAP_INDEX
		// If we're batch changing, just return these two subs to avoid constant material checks
		if (isBatchChange)
			return 2;

		return 2 + NumSubMtls();
#else
		if (isBatchChange)
			return 1;

		return 1 + NumSubMtls();

#endif
	}
	Animatable* SubAnim(int i) override;
	TSTR SubAnimName(int i, bool localized) override;
	int SubNumToRefNum(int subNum) override
	{
		return subNum;
	}

	// From ref
	int NumRefs() override
	{
		return 1;
	}
	RefTargetHandle GetReference(int i) override;

private:
	void SetReference(int i, RefTargetHandle rtarg) override;

public:

	RefTargetHandle Clone(RemapDir& remap) override;
	RefResult NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message,
			BOOL propagate) override;

	// IO
	IOResult Save(ISave* isave) override;
	IOResult Load(ILoad* iload) override;

	int NumParamBlocks() override
	{
		return 1;
	}

	IParamBlock2* GetParamBlock(int i) override
	{
		return pblock;
	}

	IParamBlock2* GetParamBlockByID(BlockID id) override
	{
		return (pblock->ID() == id) ? pblock : nullptr;
	}

	BOOL SupportsRenderElements() override
	{
		return TRUE;
	}
	ReshadeRequirements GetReshadeRequirements() override
	{
		return mReshadeRQ;
	}
	void PreShade(ShadeContext& sc, IReshadeFragment* pFrag) override;
	void PostShade(ShadeContext& sc, IReshadeFragment* pFrag, int& nextTexIndex, IllumParams* ip) override;

	// Map appearance in viewport, use this for black when empty
	int VPDisplaySubMtl() override
	{
		if (GetActiveMtl())
			return GetActiveMtlIndex();
		
		return -1;
	}

	void MappingsRequired(int subMtlNum, BitArray& mapreq, BitArray& bumpreq) override
	{
		if (subMtlNum >= 0 && subMtlNum < pblock->Count(pb_materials))
		{
			Mtl* m = pblock->GetMtl(pb_materials, subMtlNum);
			if (m)
				m->MappingsRequired(subMtlNum, mapreq, bumpreq);
		}
	}

	// From Mtl
	bool IsOutputConst(ShadeContext& sc, int stdID) override;
	bool EvalColorStdChannel(ShadeContext& sc, int stdID, Color& outClr) override;
	bool EvalMonoStdChannel(ShadeContext& sc, int stdID, float& outVal) override;

	void* GetInterface(ULONG id) override;

	BaseInterface* GetInterface(Interface_ID id) override
	{
		if (MTL_SWITCHER_ACCESS_INTERFACE == id)
			return (MtlSwitcherInterface*)(this);

		if (MTL_SWITCHER_MXS_INTERFACE == id)
			return (FPInterface*)(this);
		
		return Mtl::GetInterface(id);
	}

	BOOL SupportTexDisplay() override
	{
		Mtl* m = GetActiveMtl();
		if (m)
			return (m->SupportTexDisplay());

		return FALSE;
	}
};

//
/*===========================================================================*\
 |	Class Descriptor
\*===========================================================================*/

class MtlSwitcherClassDesc
		: public ClassDesc2
		, public IMtlRender_Compatibility_MtlBase
{
public:
	int 			IsPublic() override { return TRUE; }
	void* Create(BOOL loading) override { return new MtlSwitcher(loading); }
	const TCHAR* ClassName() override { return GetString(IDS_MATERIAL_SWITCHER_CLASSNAME); }
	SClass_ID		SuperClassID() override { return MATERIAL_CLASS_ID; }
	Class_ID ClassID() override
	{
		return MATERIAL_SWITCHER_CLASS_ID;
	}
	const TCHAR* Category() override { return _T("") ; }

	// Hardwired name, used by MAX Script as unique identifier
	const TCHAR* InternalName() override { return _T("Material Switcher"); }
	HINSTANCE		HInstance() override { return hInstance; }

	virtual const TCHAR* NonLocalizedClassName() override { return _T("Material Switcher"); }

	// IMtlRender_Compatibility_MtlBase
	virtual bool IsCompatibleWithRenderer(ClassDesc& rendererClassDesc) override
	{
		// return true as we don't know about the submaterial
		return true;
	}
};

static MtlSwitcherClassDesc MtlSwitcherCD;
ClassDesc2* GetMtlSwitcherDesc()
{
	return &MtlSwitcherCD;
}

// FPInterface
FPInterfaceDesc MtlSwitcher::mFPInterfaceDesc(MTL_SWITCHER_MXS_INTERFACE,
	_T("Material Switcher"), // Interface name used by maxscript - don't localize it!
		IDS_LIBDESCRIPTION, // Res ID of description string
		&MtlSwitcherCD, // Class descriptor
		FP_MIXIN,
		// - Methods -
		fnIdGetActiveMaterial, _T("GetActiveMaterial"), 0, TYPE_MTL, 0, 0,
		fnIdSelectMaterialByID, _T("SelectMaterialByID"), 0, TYPE_MTL, 0, 1, _T("Index"), 0, TYPE_INT,
		fnIdSelectMaterialByName, _T("SelectMaterialByName"), 0, TYPE_MTL, 0, 1, _T("Name"), 0, TYPE_STRING,
		// Properties
		fnIdSelectedName, _T("selectedname"), 0, TYPE_TSTR_BV, 0, 0,
	p_end);


// We want the number of sub materials accessible, but not stored
class TransientPBAccessor : public PBAccessor
{
public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t) // set from v
	{
		if (id == pb_num_mats)
		{
			MtlSwitcher* const mtl = dynamic_cast<MtlSwitcher*>(owner);
			if (mtl)
			{
				mtl->BulkSetNumSubMtls(v.i);
			}
		}
	}

	void Get(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t, Interval& valid) // get into v
	{
		MtlSwitcher* const mtl = dynamic_cast<MtlSwitcher*>(owner);
		if (id == pb_num_mats)
		{
			
			if (mtl)
				v.i = mtl->NumSubMtls();
		}
	}
};

static TransientPBAccessor transientAccessor;

// Clamp selection
class SelectionPBAccessor : public PBAccessor
{
public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t) // set from v
	{
		if (id == pb_switcher_selection)
		{
			MtlSwitcher* const mtl = dynamic_cast<MtlSwitcher*>(owner);
			if (mtl)
			{
				if (v.i > mtl->NumSubMtls())
					mtl->pblock->SetValue(pb_switcher_selection, t, mtl->NumSubMtls());
				else if (v.i < 1)
					mtl->pblock->SetValue(pb_switcher_selection, t, 1);
			}
		}
	}
};

static SelectionPBAccessor selection_accessor;

// per instance param block
static ParamBlockDesc2 switcher_param_blk(switcher_params, _T("parameters"), 0, &MtlSwitcherCD, P_AUTO_CONSTRUCT + P_AUTO_UI,
	PBLOCK_REF,
	// rollout
	IDD_MATERIAL_SWITCHER, IDS_MATERIAL_PARAMS, 0, 0, nullptr,

	// params
	pb_materials, _T("materialList"), TYPE_MTL_TAB, INITIAL_MATERIAL_COUNT, P_VARIABLE_SIZE, IDS_MATERIAL,
	p_nonLocalizedName, _T("Material"),
	p_end,

#ifdef TEXMAP_INDEX
	pb_index_map,_T("indexMap"), TYPE_TEXMAP, 0, IDS_INDEX_MAP,
	p_ui, TYPE_TEXMAPBUTTON, IDC_INDEX_MAP,
	p_subtexno, 0,
	p_end,
#endif
	
	pb_mtl_labels, _T("names"), TYPE_STRING_TAB, INITIAL_MATERIAL_COUNT, P_VARIABLE_SIZE, IDS_MAP,
	p_end,
	
#ifdef ANIMATIBLE_SWITCHER_ID
	pb_switcher_selection, _T("selection"), TYPE_INT, P_ANIMATABLE, IDS_SELECTION,
#else
	pb_switcher_selection, _T("selection"), TYPE_INT, 0, IDS_SELECTION,
#endif
	p_default, 1,
	p_range, 1, MAX_MATERIALS,
	p_ui, TYPE_SPINNER, EDITTYPE_INT, IDC_SELECTION_EDIT, IDC_SELECTION_SPIN, 1.0f,
	p_accessor,	&selection_accessor,
	p_end,

	pb_num_mats, _T("nummats"), TYPE_INT, P_TRANSIENT, IDS_NUM_MAPS,
	p_range, 1, MAX_MATERIALS,
	p_ui, TYPE_SPINNER, EDITTYPE_INT, IDC_NUMMATS_EDIT, IDC_NUMMATS_SPIN, 1.0f,
	p_accessor, &transientAccessor,
	p_end,

	pb_locked,	_T("UILocked"), TYPE_BOOL, 0, IDS_UILOCKED,
	p_ui, TYPE_CHECKBUTTON, IDC_LOCK,
	p_end,
	
	p_end
);

static int subMtlId[UI_MATERIAL_COUNT] = { IDC_SWITCHER_MTL0, IDC_SWITCHER_MTL1, IDC_SWITCHER_MTL2, IDC_SWITCHER_MTL3, IDC_SWITCHER_MTL4,
	IDC_SWITCHER_MTL5, IDC_SWITCHER_MTL6, IDC_SWITCHER_MTL7, IDC_SWITCHER_MTL8, IDC_SWITCHER_MTL9,
	IDC_SWITCHER_MTL10, IDC_SWITCHER_MTL11, IDC_SWITCHER_MTL12, IDC_SWITCHER_MTL13, IDC_SWITCHER_MTL14, IDC_SWITCHER_MTL15,
	IDC_SWITCHER_MTL16, IDC_SWITCHER_MTL17, IDC_SWITCHER_MTL18, IDC_SWITCHER_MTL19, IDC_SWITCHER_MTL20, IDC_SWITCHER_MTL21,
	IDC_SWITCHER_MTL22, IDC_SWITCHER_MTL23, IDC_SWITCHER_MTL24, IDC_SWITCHER_MTL25, IDC_SWITCHER_MTL26, IDC_SWITCHER_MTL27,
	IDC_SWITCHER_MTL28, IDC_SWITCHER_MTL29 };

static int subNameId[UI_MATERIAL_COUNT] = { IDC_MTL_NAME0, IDC_MTL_NAME1, IDC_MTL_NAME2, IDC_MTL_NAME3, IDC_MTL_NAME4,
	IDC_MTL_NAME5, IDC_MTL_NAME6, IDC_MTL_NAME7, IDC_MTL_NAME8, IDC_MTL_NAME9,
	IDC_MTL_NAME10, IDC_MTL_NAME11, IDC_MTL_NAME12, IDC_MTL_NAME13, IDC_MTL_NAME14, IDC_MTL_NAME15, IDC_MTL_NAME16,
	IDC_MTL_NAME17, IDC_MTL_NAME18, IDC_MTL_NAME19, IDC_MTL_NAME20, IDC_MTL_NAME21, IDC_MTL_NAME22, IDC_MTL_NAME23,
	IDC_MTL_NAME24, IDC_MTL_NAME25, IDC_MTL_NAME26, IDC_MTL_NAME27, IDC_MTL_NAME28, IDC_MTL_NAME29 };

static int subIndex[UI_MATERIAL_COUNT] = { IDC_MTL_ID0, IDC_MTL_ID1, IDC_MTL_ID2, IDC_MTL_ID3, IDC_MTL_ID4, IDC_MTL_ID5,
	IDC_MTL_ID6, IDC_MTL_ID7, IDC_MTL_ID8, IDC_MTL_ID9,
	IDC_MTL_ID10, IDC_MTL_ID11, IDC_MTL_ID12, IDC_MTL_ID13, IDC_MTL_ID14, IDC_MTL_ID15, IDC_MTL_ID16, IDC_MTL_ID17,
	IDC_MTL_ID18, IDC_MTL_ID19, IDC_MTL_ID20, IDC_MTL_ID21,	IDC_MTL_ID22, IDC_MTL_ID23, IDC_MTL_ID24, IDC_MTL_ID25,
	IDC_MTL_ID26, IDC_MTL_ID27, IDC_MTL_ID28, IDC_MTL_ID29 };

static int mtlPStampIDs[] = { IDC_PSTAMP1, IDC_PSTAMP2,	IDC_PSTAMP3, IDC_PSTAMP4, IDC_PSTAMP5, IDC_PSTAMP6,	IDC_PSTAMP7,
	IDC_PSTAMP8, IDC_PSTAMP9, IDC_PSTAMP10,
	IDC_PSTAMP11, IDC_PSTAMP12, IDC_PSTAMP13, IDC_PSTAMP14, IDC_PSTAMP15, IDC_PSTAMP16, IDC_PSTAMP17, IDC_PSTAMP18,
	IDC_PSTAMP19, IDC_PSTAMP20, IDC_PSTAMP21, IDC_PSTAMP22, IDC_PSTAMP23, IDC_PSTAMP24, IDC_PSTAMP25, IDC_PSTAMP26,
	IDC_PSTAMP27, IDC_PSTAMP28, IDC_PSTAMP29, IDC_PSTAMP30 };


static int SubMtlNumFromPStampID(int id)
{
	if (id == IDC_PSTAMP_ACTIVE)
		return ACTIVE_STAMP;

	for (int i = 0; i < UI_MATERIAL_COUNT; i++)
	{
		if (mtlPStampIDs[i] == id)
			return i;
	}
	return 0;
}

static int SubMtlNumFromMaterialID(int id)
{
	for (int i = 0; i < UI_MATERIAL_COUNT; i++)
	{
		if (subMtlId[i] == id)
			return i;
	}
	return 0;
}

void MtlSwitcherDlgProc::RemovePStampHilite()
{
	for (int i = 0; i < UI_MATERIAL_COUNT; i++)
	{
		DrawPStampHilite(i, FALSE, FALSE);
	}
}

#define DORECT(x) Rectangle(hdc, rect.left - (x), rect.top - (x), rect.right + (x), rect.bottom + (x))

BOOL MtlSwitcherDlgProc::WheelScroll(short int position)
{
	// If we're not handling, let the parent do so
	if (!ScrollingNeeded())
		return FALSE;

	if (position > 0)
		VScroll(SB_LINEUP, 0);
	else if (position < 0)
		VScroll(SB_LINEDOWN, 0);

	return TRUE;
}

void MtlSwitcherDlgProc::VScroll(int code, short int cpos)
{
	for (int i = 0; i < UI_MATERIAL_COUNT; i++)
	{
		if (iName[i]->HasFocus())
			SetFocus(nullptr);
	}

	switch (code)
	{
	case SB_LINEUP:
		theMtl->offset--;
		break;

	case SB_LINEDOWN:
		theMtl->offset++;
		break;

	case SB_PAGEUP:
		theMtl->offset -= UI_MATERIAL_COUNT;
		break;

	case SB_PAGEDOWN:
		theMtl->offset += UI_MATERIAL_COUNT;
		break;

	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		theMtl->offset = cpos;
		break;

	case SB_BOTTOM: {
		theMtl->offset = theMtl->NumSubMtls() - UI_MATERIAL_COUNT;
	}
	break;

	}

	theMtl->ClampOffset();
	UpdateSubMtlNames();
}

void MtlSwitcherDlgProc::DrawPStampBlackBorder(HDC hdc, BOOL onSub, Rect& rect)
{
	SelectObject(hdc, GetStockObject(NULL_BRUSH));
	SelectObject(hdc, GetStockObject(BLACK_PEN));
	DORECT(0);
}

void MtlSwitcherDlgProc::DrawPStampHilite(HDC hdc, BOOL on, BOOL onSub, Rect& rect)
{
	SelectObject(hdc, GetStockObject(NULL_BRUSH));
	HPEN hGray = CreatePen(PS_SOLID, 0, ColorMan()->GetColor(kBackgroundEven));

	auto old_pen = SelectObject(hdc, hGray);

	if (onSub && !on)
	{
		HPEN hSubHighlight = CreatePen(PS_SOLID, 0, SUBHILIGHT_COLOR);
		SelectObject(hdc, hSubHighlight);
		DORECT(1);
		DeleteObject(hSubHighlight);
		SelectObject(hdc, hGray);
	} else {
		if (on)
			SelectObject(hdc, GetStockObject(WHITE_PEN));

		DORECT(1);
	}

	DeleteObject(hGray);
	SelectObject(hdc, old_pen);
}

void MtlSwitcherDlgProc::DrawPStampHilite(int i, BOOL on, BOOL onSub)
{
	if ((i < 0) || (i >= UI_MATERIAL_COUNT))
		return;

	HWND hwStamp = GetDlgItem(hMaterialPanel, mtlPStampIDs[i]);
	HDC hdc = GetDC(hwStamp);
	Rect rect;
	GetClientRect(hwStamp, &rect);
	DrawPStampHilite(hdc, on, onSub, rect);
	ReleaseDC(hwStamp, hdc);
}

void MtlSwitcherDlgProc::HighlightActiveStamp()
{
	// Clear All
	RemovePStampHilite();
	int iActive = theMtl->GetActiveMtlIndex();
	if (iActive < 0)
		return;

	int iSortedIndex = theMtl->GetSortOrderFromMtlIndex(iActive);

	int iIndex = iSortedIndex - theMtl->offset;
	if (iIndex >= 0 && iIndex < UI_MATERIAL_COUNT)
	{
		DrawPStampHilite(iIndex, TRUE, FALSE);
	}

	if (theMtl->IsUILocked())
	{
		int SubIndex = theMtl->GetSortOrderFromMtlIndex(subSelection) - theMtl->offset;
		if (SubIndex != iIndex && SubIndex >= 0 && SubIndex < UI_MATERIAL_COUNT)
		{
			DrawPStampHilite(SubIndex, FALSE, TRUE);
		}
	} else
		UpdateSelectedLabel();
}

void MtlSwitcherDlgProc::UpdateSelectedLabel()
{
	int iActive = theMtl->GetActiveMtlIndex();

	const TCHAR* name = nullptr;
	if (iActive < theMtl->pblock->Count(pb_mtl_labels))
	{
		name = theMtl->pblock->GetStr(pb_mtl_labels, 0, iActive);
		if (name)
		{
#ifdef ALLOW_NAME_SEARCH
			iNameSearch->SetText(name);
#else
			// Reformat so there is a leading space if not blank
			if (_tcslen(name))
			{
				TSTR formattedName;
				formattedName.printf(_T(" %s"), name);
				SetWindowText(GetDlgItem(hMaterialPanel, IDC_NAME_STATIC), formattedName);
			} else
				SetWindowText(GetDlgItem(hMaterialPanel, IDC_NAME_STATIC), _T(""));
#endif
		}
	}
	if (!name)
	{
#ifdef ALLOW_NAME_SEARCH
		iNameSearch->SetText(_T(""));
#else
		SetWindowText(GetDlgItem(hMaterialPanel, IDC_NAME_STATIC), _T(""));
#endif
	}
}

void MtlSwitcherDlgProc::DrawPStamp(HDC hdc, Rect& rect, Mtl* m, bool bHighlight, bool bSubHighlight, bool bIsActiveStamp)
{
	if (m == nullptr)
	{
		auto old_brush = (HBRUSH)SelectObject(hdc, ColorMan()->GetBrush(kBackgroundEven));
		DORECT(0);
		SelectObject(hdc, old_brush);
	}
	else
	{
		int stampSize = theMtl->mPStampSize;
		if (bIsActiveStamp)
			stampSize = theMtl->mActivePStampSize;

		PStamp* ps = m->GetPStamp(stampSize);
		if (!ps)
		{
			ps = m->CreatePStamp(stampSize, TRUE);
		}
		int w = ps->Width();
		int h = ps->Height();
		int scanw = ByteWidth(w);
		int nb = scanw * h;
		UBYTE* workImg = new UBYTE[nb];
		ps->GetImage(workImg);

		// Resize if ActiveStamp if necessary
		if (false)
		{
			Rect stamp_rect;
			int dimension = PSDIM(stampSize);
			stamp_rect.left = 0;
			stamp_rect.top = 0;
			stamp_rect.right = dimension-1;
			stamp_rect.bottom = dimension-1;

			GetGPort()->DisplayMap(hdc, rect, stamp_rect, workImg, scanw);
		}
		else
		{
			Rect stamp_rect;
			stamp_rect.left = 1;
			stamp_rect.top = 1;
			stamp_rect.right = w + 1;
			stamp_rect.bottom = h + 1;

			GetGPort()->DisplayMap(hdc, stamp_rect, 0, 0, workImg, scanw);
		}

		delete[] workImg;
		workImg = nullptr;
	}

	if (!bIsActiveStamp)
	{
		DrawPStampBlackBorder(hdc, bSubHighlight, rect);

		if (bHighlight || bSubHighlight)
			DrawPStampHilite(hdc, bHighlight, bSubHighlight, rect);
	}
}

void MtlSwitcherDlgProc::DrawPStamp(HDC hdc, Rect& rect, int i)
{
	if (!theMtl)
		return;

	if (i == ACTIVE_STAMP)
	{
		DrawPStamp(hdc, rect, theMtl->GetActiveMtl(), false, false, true);
	}
	else
	{
		int index = i + theMtl->offset;
		int mtlIndex = theMtl->GetMtlIndexFromSortOrder(index);
		if (mtlIndex >= 0 && mtlIndex < theMtl->NumSubMtls())
		{
			Mtl* m = theMtl->GetSubMtl(mtlIndex);
			DrawPStamp(hdc, rect, m, (mtlIndex == theMtl->GetActiveMtlIndex()), theMtl->IsUILocked() && (mtlIndex == subSelection), false);
		}
	}
}

void MtlSwitcherDlgProc::SelectMtl(int i)
{
	if (i == ACTIVE_STAMP)
	{
		ScrollToActiveMaterial();
		return;
	}

	if (!theMtl ||  i< 0 || i>= UI_MATERIAL_COUNT)
		return;

	int index = i + theMtl->offset;
	int mtlIndex = theMtl->GetMtlIndexFromSortOrder(index);

	int priorStamp = -1;

	if (!theMtl->IsUILocked())
	{

#ifdef TEXMAP_INDEX
		// Do nothing if a texmap exists
		if (theMtl->HasIndexMap())
			return;
#endif

		priorStamp = theMtl->GetSortOrderFromMtlIndex(theMtl->GetActiveMtlIndex()) - theMtl->offset;
		theMtl->SetActiveMtlIndex(mtlIndex);
	}
	else
	{
		bool bNeedMainHighlight = false;
		if ((subSelection >= 0) && (subSelection != theMtl->GetActiveMtlIndex()))
		{
			priorStamp = theMtl->GetSortOrderFromMtlIndex(subSelection) - theMtl->offset;
		}
		else
			bNeedMainHighlight = true;

		subSelection = mtlIndex;

		// Highlight main
		int mainHighlight = theMtl->GetActiveMtlIndex() - theMtl->offset;
		if (mainHighlight >= 0 && mainHighlight < UI_MATERIAL_COUNT)
		{
			HWND hwps = GetDlgItem(hMaterialPanel, mtlPStampIDs[mainHighlight]);
			InvalidateRect(hwps, nullptr, FALSE);
		}
	}

	// Clear the prior entry
	if (priorStamp >= 0 && priorStamp < UI_MATERIAL_COUNT)
		DrawPStampHilite(priorStamp, FALSE, FALSE);

	HWND hwps = GetDlgItem(hMaterialPanel, mtlPStampIDs[i]);
	InvalidateRect(hwps, nullptr, FALSE);

}

int MtlSwitcherDlgProc::SubMtlNumFromNameID(int id)
{
	for (int i = 0; i < UI_MATERIAL_COUNT; i++)
	{
		if (subNameId[i] == id)
			return i;
	}
	return 0;
}

int MtlSwitcherDlgProc::FindSubMtlFromHWND(HWND hWnd)
{
	for (int i = 0; i < UI_MATERIAL_COUNT; i++)
	{
		if (hWnd == iMtlButton[i]->GetHwnd())
		{
			int index = i + theMtl->offset;
			return (theMtl->GetMtlIndexFromSortOrder(index));
		}
	}
	return -1;
}

LRESULT CALLBACK PStampWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LONG_PTR id = SubMtlNumFromPStampID(GetWindowLongPtr(hwnd, GWLP_ID));
	MtlSwitcherDlgProc* theDlg = DLGetWindowLongPtr<MtlSwitcherDlgProc*>(hwnd);
	if (theDlg == nullptr)
		return FALSE;

	switch (msg)
	{
	case WM_LBUTTONUP:
		theDlg->SelectMtl(id);
		break;

	case WM_PAINT: {
		PAINTSTRUCT ps;
		Rect rect;
		HDC hdc = BeginPaint(hwnd, &ps);
		if (!IsRectEmpty(&ps.rcPaint))
		{
			GetClientRect(hwnd, &rect);
			theDlg->DrawPStamp(hdc, rect, id);
		}
		EndPaint(hwnd, &ps);
	}
	break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool MtlSwitcherDlgProc::ScrollingNeeded()
{
	if (theMtl)
		return theMtl->NumSubMtls() > UI_MATERIAL_COUNT;

	return false;
}

void MtlSwitcherDlgProc::UpdateScrollBar()
	{
	if (!hScroll || !theMtl)
		return;

	bool bEnabled = ScrollingNeeded();
#ifdef HIDE_SCROLL_WHEN_NOT_NEEDED
	if (bEnabled)
	{
		SetScrollRange(hScroll, SB_CTL, 0, theMtl->NumSubMtls() - UI_MATERIAL_COUNT, FALSE);
		SetScrollPos(hScroll, SB_CTL, theMtl->offset, TRUE);
	}
	ShowWindow(hScroll, bEnabled);
#else
	SetScrollRange(hScroll, SB_CTL, 0, theMtl->NumSubMtls() - UI_MATERIAL_COUNT, FALSE);
	SetScrollPos(hScroll, SB_CTL, theMtl->offset, TRUE);
	EnableWindow(hScroll, theMtl->NumSubMtls() > UI_MATERIAL_COUNT);
#endif
}

void MtlSwitcherDlgProc::UpdateNumMatSpinner()
{
	ISpinnerControl* isc = GetISpinner(GetDlgItem(hMaterialPanel, IDC_NUMMATS_SPIN));
	if (isc)
	{
		int val = isc->GetIVal();
		int num = theMtl->NumSubMtls();
		if (val != num)
			isc->SetValue(num, FALSE);
	}
}

#ifdef TEXMAP_INDEX
void MtlSwitcherDlgProc::UpdateSelectionUI()
{
	if (!theMtl)
		return;

	ISpinnerControl* isc = GetISpinner(GetDlgItem(hMaterialPanel, IDC_SELECTION_SPIN));
	if (isc)
	{
		isc->Enable(!theMtl->HasIndexMap());
		ReleaseISpinner(isc);
	}

	if (iNameSearch)
	{
		iNameSearch->Enable(!theMtl->HasIndexMap());
	}
}
#endif

void MtlSwitcherDlgProc::UpdateSubMtlNames()
{
	int ct = theMtl->pblock->Count(pb_mtl_labels);

	RemovePStampHilite();

	int i;

	int numSubs = theMtl->NumSubMtls();
	if (numSubs > UI_MATERIAL_COUNT)
		numSubs = UI_MATERIAL_COUNT;

	theMtl->ClampOffset();

	TSTR textBuffer;

	for (i = 0; i < numSubs; i++)
	{
		int index = i + theMtl->offset;
		int mtlIndex = theMtl->GetMtlIndexFromSortOrder(index);
		if (mtlIndex >= 0)
		{
			Mtl* m = theMtl->GetSubMtl(mtlIndex);

			ShowWindow(GetDlgItem(hMaterialPanel, subMtlId[i]), SW_SHOW);
			ShowWindow(GetDlgItem(hMaterialPanel, subIndex[i]), SW_SHOW);
			ShowWindow(GetDlgItem(hMaterialPanel, subNameId[i]), SW_SHOW);
			ShowWindow(GetDlgItem(hMaterialPanel, mtlPStampIDs[i]), SW_SHOW);

			iMtlButton[i]->SetText(m?m->GetName().data() :GetString(IDS_NONE));

			const TCHAR* name = nullptr;

			if (mtlIndex < theMtl->pblock->Count(pb_mtl_labels))
				name = theMtl->pblock->GetStr(pb_mtl_labels, 0, mtlIndex);

			if (name)
			{
				iName[i]->GetText(textBuffer);
				if (_tcscmp(name, textBuffer))
					iName[i]->SetText(name);
			}
			else
				iName[i]->SetText(_T(""));

			textBuffer.printf(_T("%d"), mtlIndex + 1);
			iIndex[i]->SetText(textBuffer);

		}
		HWND hwps = GetDlgItem(hMaterialPanel, mtlPStampIDs[i]);
		InvalidateRect(hwps, nullptr, FALSE);
	}

	// Hide Remaining
	if (i < UI_MATERIAL_COUNT)
	{
		for (int j = i; j < UI_MATERIAL_COUNT; j++)
		{
			ShowWindow(GetDlgItem(hMaterialPanel, subMtlId[j]), SW_HIDE);
			ShowWindow(GetDlgItem(hMaterialPanel, subIndex[j]), SW_HIDE);
			ShowWindow(GetDlgItem(hMaterialPanel, subNameId[j]), SW_HIDE);
			ShowWindow(GetDlgItem(hMaterialPanel, mtlPStampIDs[j]), SW_HIDE);
		}
	}

	// See if we need to resize the rollup and material list
	IRollupWindow* irw = ip->GetMtlEditorRollup();
	int delta = 0;
	if (irw)
	{
		int index = irw->GetPanelIndex(hMaterialPanel);
		constexpr float ButtonSize = 27.5f;	
		
		if (i != UI_MATERIAL_COUNT)
			delta = (int)((UI_MATERIAL_COUNT - i) * ButtonSize);

		int newRollupHeight = rollupHeight - delta;

		if (newRollupHeight != irw->GetPageDlgHeight(index))
		{
			// Resize everything else
			Rect rect;

			HWND thisWindow = GetDlgItem(hMaterialPanel, IDC_MATERIALS_FRAME);
			GetClientRectP(thisWindow, &rect);
			int newHeight = frameHeight - delta;
			SetWindowPos(thisWindow, NULL, 0, 0, rect.w() - 1, newHeight,
					SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW);

			if (ScrollingNeeded())
			{
				thisWindow = GetDlgItem(hMaterialPanel, IDC_MULTI_SCROLL);
				GetClientRectP(thisWindow, &rect);
				newHeight = scrollHeight - delta;
				SetWindowPos(thisWindow, NULL, 0, 0, rect.w() - 1, newHeight,
						SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW);
			}

			irw->SetPageDlgHeight(index, newRollupHeight);
		}
	}

	SetDynamicTooltips();
	UpdateScrollBar();
#ifdef TEXMAP_INDEX
	UpdateSelectionUI();
#endif
}

void MtlSwitcherDlgProc::LoadDialog()
{
	if (theMtl)
	{
		Interval valid;
		theMtl->Update(TIMENOW, valid);
		UpdateSubMtlNames();
		// Update the Active Stamp too
		RefreshActiveStamp();
	}
}

void MtlSwitcherDlgProc::SetThing(ReferenceTarget* m)
{
	assert(m->ClassID() == MATERIAL_SWITCHER_CLASS_ID);
	assert(m->SuperClassID() == MATERIAL_CLASS_ID);
	RemovePStampHilite();
	if (theMtl)
		theMtl->SetParamDlg(nullptr);

	theMtl = (MtlSwitcher*)m;
	if (theMtl)
		theMtl->SetParamDlg(this);
	LoadDialog();
}

void MtlSwitcherDlgProc::SetUpStamp(HWND hWnd, HWND hStamp, HWND hIndex)
{
	if (!theMtl)
		return;

	RECT index_rect = {};
	GetWindowRect(hIndex, &index_rect);
	RECT stamp_rect = {};
	GetWindowRect(hStamp, &stamp_rect);
	POINT pt{ stamp_rect.left + ((stamp_rect.right - stamp_rect.left) / 2),
		index_rect.top + ((index_rect.bottom - index_rect.top) / 2) };
	ScreenToClient(hWnd, &pt);
	int stamp_size = PS_TINY_SIZE + 2;
	if (MaxSDK::UIScaled(PS_TINY_SIZE) >= PS_SMALL_SIZE)
	{
		theMtl->mPStampSize = PS_SMALL;
		stamp_size = PS_SMALL_SIZE + 2;
	}
	else
	{
		theMtl->mPStampSize = PS_TINY;
	}

	MoveWindow(hStamp, pt.x - (stamp_size / 2), pt.y - (stamp_size / 2), stamp_size, stamp_size, FALSE);
	DLSetWindowProc(hStamp, PStampWndProc);
	DLSetWindowLongPtr(hStamp, this);
}

void MtlSwitcherDlgProc::SetUpActiveStamp(HWND hWnd, HWND hStamp)
{
	if (!theMtl)
		return;

	// Force stampe size to small as in 4K, Large (88x88) is too big for the window size
	int stamp_size = PS_SMALL_SIZE + 4;
	theMtl->mActivePStampSize = PS_SMALL;

	// Align to the bottom of the name control and centered horizontally
	RECT name_rect = {};
	GetWindowRect(GetDlgItem(hWnd, IDC_NAME_STATIC), &name_rect);
	RECT stamp_rect = {};
	GetWindowRect(hStamp, &stamp_rect);
	POINT pt{ stamp_rect.left + ((stamp_rect.right - stamp_rect.left) / 2),
		(name_rect.bottom - stamp_size) };
	ScreenToClient(hWnd, &pt);

	MoveWindow(hStamp, pt.x - (stamp_size / 2), pt.y, stamp_size, stamp_size, FALSE);
	DLSetWindowProc(hStamp, PStampWndProc);
	DLSetWindowLongPtr(hStamp, this);
}

void MtlSwitcherDlgProc::SetStaticTooltips()
{
	tooltips.SetToolTip(GetDlgItem(hMaterialPanel, IDC_SELECTION_SPIN), GetString(IDS_TT_ID));
	tooltips.SetToolTip(GetDlgItem(hMaterialPanel, IDC_NUMMATS_SPIN), GetString(IDS_TT_NUMATS));

	tooltips.SetToolTip(GetDlgItem(hMaterialPanel, IDC_SWITCHER_INSERT), GetString(IDS_TT_INSERT));
	tooltips.SetToolTip(GetDlgItem(hMaterialPanel, IDC_SWITCHER_DELETE), GetString(IDS_TT_DELETE));

	tooltips.SetToolTip(GetDlgItem(hMaterialPanel, IDC_LOCK), GetString(IDS_TT_LOCK_HEADER));
	tooltips.SetToolTip(GetDlgItem(hMaterialPanel, IDC_INDEX_SORT), GetString(IDS_TT_ID_HEADER));
	tooltips.SetToolTip(GetDlgItem(hMaterialPanel, IDC_NAME_SORT), GetString(IDS_TT_NAME_HEADER));
	tooltips.SetToolTip(GetDlgItem(hMaterialPanel, IDC_MATERIAL_SORT), GetString(IDS_TT_MATERIAL_HEADER));
}	

void MtlSwitcherDlgProc::SetDynamicTooltips()
{
	int count = std::min(theMtl->NumSubMtls(), UI_MATERIAL_COUNT);

	for (int i = 0; i < count; i++)
	{
		Mtl* m = nullptr;
		HWND hwSub = GetDlgItem(hMaterialPanel, subMtlId[i]);
		int subMtl = FindSubMtlFromHWND(hwSub);

		if (subMtl >= 0)
			m = theMtl->GetSubMtl(subMtl);

		if (m)
		{
			tooltips.SetToolTip(hwSub, m->GetFullName(true));
		}
		else
		{
			tooltips.SetToolTip(hwSub, GetString(IDS_TT_MATERIAL_NONE));	
		}
	}
}

void MtlSwitcherDlgProc::InitDialog(HWND hDlg)
{
	hMaterialPanel = hDlg;
	if (!hMaterialPanel)
		return;

	IRollupWindow* irw = ip->GetMtlEditorRollup();
	int index = irw->GetPanelIndex(hMaterialPanel);
	rollupHeight = irw->GetPageDlgHeight(index);

	Rect rect;
	GetClientRectP(GetDlgItem(hMaterialPanel, IDC_MATERIALS_FRAME), &rect);
	frameHeight = rect.h();

	GetClientRectP(GetDlgItem(hMaterialPanel, IDC_MULTI_SCROLL), &rect);
	scrollHeight = rect.h();

	OnLock();

	ICustButton* icbLock = GetICustButton(GetDlgItem(hDlg, IDC_LOCK));
	if (icbLock)
	{
		icbLock->SetType(CBT_CHECK);
		icbLock->SetHighlightColor(RGB(64, 64, 64));
		icbLock->SetImage(GetLockIcons(), 1, 0, 1, 0, MaxSDK::UIScaled(16), MaxSDK::UIScaled(15));
	}

	hScroll = GetDlgItem(hMaterialPanel, IDC_SWITCHER_SCROLL);
	UpdateScrollBar();

	InitSortHeaders();

#ifdef ALLOW_NAME_SEARCH
	iNameSearch = GetICustEdit(GetDlgItem(hMaterialPanel, IDC_NAME_SELECTION_EDIT));
	iNameSearch->WantReturn(TRUE);
	ShowWindow(GetDlgItem(hMaterialPanel, IDC_NAME_STATIC), FALSE);
#else
	ShowWindow(GetDlgItem(hMaterialPanel, IDC_NAME_SELECTION_EDIT), FALSE);
#endif

	for (int i = 0; i < UI_MATERIAL_COUNT; i++)
	{
		iMtlButton[i] = GetICustButton(GetDlgItem(hMaterialPanel, subMtlId[i]));
		iMtlButton[i]->SetDADMgr(&dadMgr);

		iName[i] = GetICustEdit(GetDlgItem(hMaterialPanel, subNameId[i]));
		iName[i]->SetLeading(2);

		iIndex[i] = GetICustStatus(GetDlgItem(hMaterialPanel, subIndex[i]));
		iIndex[i]->SetTextFormat(STATUSTEXT_CENTERED);
		if (i + theMtl->offset < theMtl->NumSubMtls())
		{
			const TCHAR* name = theMtl->pblock->GetStr(pb_mtl_labels, 0, i + theMtl->offset);
			iName[i]->SetText(name);
			TSTR idTex;
			idTex.printf(_T("%d"), i + theMtl->offset + 1);
			iIndex[i]->SetText(idTex);
		}

		SetUpStamp(hMaterialPanel, GetDlgItem(hMaterialPanel, mtlPStampIDs[i]), iIndex[i]->GetHwnd());
	}

	SetUpActiveStamp(hMaterialPanel, GetDlgItem(hMaterialPanel, IDC_PSTAMP_ACTIVE));
	HighlightActiveStamp();

	SetStaticTooltips();

}

void MtlSwitcherDlgProc::SetSortIndexLabel(bool bClear)
{
	HWND hIndex = GetDlgItem(hMaterialPanel, IDC_INDEX_SORT);
	if (hIndex)
	{
		TSTR label(GetString(IDS_INDEX));
		if (!bClear)
		{
			if (CurrentSortingType == SORT_INDEX_ASCENDING)
			{
				label.Append(DownTriangleSymbolWithLeadingSpace);
			}
			else if (CurrentSortingType == SORT_INDEX_DESCENDING)
			{
				label.Append(UpTriangleSymbolWithLeadingSpace);
			}
		}
		SetWindowText(hIndex, label);
	}
}

void MtlSwitcherDlgProc::SetSortNameLabel(bool bClear)
{
	HWND hName = GetDlgItem(hMaterialPanel, IDC_NAME_SORT);
	if (hName)
	{
		TSTR label(GetString(IDS_NAME));
		if (!bClear)
		{
			if (CurrentSortingType == SORT_NAME_ASCENDING)
			{
				label.Append(DownTriangleSymbolWithLeadingSpace);
			}
			else if (CurrentSortingType == SORT_NAME_DESCENDING)
			{
				label.Append(UpTriangleSymbolWithLeadingSpace);
			}
		}
		SetWindowText(hName, label);
	}

}

void MtlSwitcherDlgProc::SetSortMaterialLabel(bool bClear)
{
	HWND hMaterial = GetDlgItem(hMaterialPanel, IDC_MATERIAL_SORT);
	if (hMaterial)
	{
		TSTR label(GetString(IDS_MATERIAL));
		if (!bClear)
		{
			if (CurrentSortingType == SORT_MATERIAL_ASCENDING)
			{
				label.Append(DownTriangleSymbolWithLeadingSpace);
			}
			else if (CurrentSortingType == SORT_MATERIAL_DESCENDING)
			{
				label.Append(UpTriangleSymbolWithLeadingSpace);
			}
		}
		SetWindowText(hMaterial, label);
	}
}

void MtlSwitcherDlgProc::UpdateSortLabels(int newType)
{
	switch (CurrentSortingType)
	{
	case SORT_INDEX_ASCENDING:
	case SORT_INDEX_DESCENDING:
		if ((newType != SORT_INDEX_ASCENDING) && (newType != SORT_INDEX_DESCENDING))
			SetSortIndexLabel(true);
		break;

	case SORT_NAME_ASCENDING:
	case SORT_NAME_DESCENDING:
		if ((newType != SORT_NAME_ASCENDING) && (newType != SORT_NAME_DESCENDING))
			SetSortNameLabel(true);
		break;

	case SORT_MATERIAL_ASCENDING:
	case SORT_MATERIAL_DESCENDING:
		if ((newType != SORT_MATERIAL_ASCENDING) && (newType != SORT_MATERIAL_DESCENDING))
			SetSortMaterialLabel(true);
		break;
	}

	CurrentSortingType = newType;
	if ((newType == SORT_INDEX_ASCENDING) || (newType == SORT_INDEX_DESCENDING))
		SetSortIndexLabel(false);
	else if ((newType == SORT_NAME_ASCENDING) || (newType == SORT_NAME_DESCENDING))
		SetSortNameLabel(false);
	else if ((newType == SORT_MATERIAL_ASCENDING) || (newType == SORT_MATERIAL_DESCENDING))
		SetSortMaterialLabel(false);
}

void MtlSwitcherDlgProc::InitSortHeaders()
{
	if (!hMaterialPanel)
		return;

	SetSortIndexLabel(false);
	SetSortNameLabel(false);
	SetSortMaterialLabel(false);
}

void MtlSwitcherDlgProc::OnSortIndex()
{
	if (!theMtl)
		return;

	bool bDescending = false;
	if (CurrentSortingType != SORT_INDEX_ASCENDING)
		UpdateSortLabels(SORT_INDEX_ASCENDING);
	else
	{
		bDescending = true;
		UpdateSortLabels(SORT_INDEX_DESCENDING);
	}

	RemovePStampHilite();
	theMtl->SortMtlsByID(bDescending);
	UpdateSubMtlNames();
	ScrollToActiveMaterial();
}

void MtlSwitcherDlgProc::OnSortName()
{
	if (!theMtl)
		return;

	bool bDescending = false;
	if (CurrentSortingType != SORT_NAME_ASCENDING)
		UpdateSortLabels(SORT_NAME_ASCENDING);
	else
	{
		bDescending = true;
		UpdateSortLabels(SORT_NAME_DESCENDING);
	}

	RemovePStampHilite();
	theMtl->SortMtlsBySlotName(bDescending);
	UpdateSubMtlNames();
	ScrollToActiveMaterial();
}

void MtlSwitcherDlgProc::OnSortMaterial()
{
	if (!theMtl)
		return;

	bool bDescending = false;
	if (CurrentSortingType != SORT_MATERIAL_ASCENDING)
		UpdateSortLabels(SORT_MATERIAL_ASCENDING);
	else
	{
		bDescending = true;
		UpdateSortLabels(SORT_MATERIAL_DESCENDING);
	}

	RemovePStampHilite();
	theMtl->SortMtlsByName(bDescending);
	UpdateSubMtlNames();
	ScrollToActiveMaterial();
}

void MtlSwitcherDlgProc::OnInsert()
{
	if (theMtl->NumSubMtls() >= MAX_MATERIALS) // don't allow more than max materials
		return;

	int index = -1;
	if (theMtl->IsUILocked())
	{
		index = subSelection;
	}
	else
	{
		index = theMtl->GetActiveMtlIndex();
	}

	if (index >= 0)
	{
		RemovePStampHilite();
		theHold.Begin();
		theMtl->InsertMtl(index);
		theHold.Accept(GetString(IDS_INSERT_SUBMTL));
		UpdateSubMtlNames();
		UpdateMtlDisplay();
	}
}

void MtlSwitcherDlgProc::OnDelete()
{
	if (theMtl->NumSubMtls() == 1) // Don't let the user delete the last material, should disable button
		return;

	int index = -1;
	if (theMtl->IsUILocked())
	{
		index = subSelection;

		// We have to decrement our main selection
		int mainSelection = theMtl->GetActiveMtlIndex();
	}
	else
	{
		index = theMtl->GetActiveMtlIndex();
	}

	if (index >= 0)
	{
		RemovePStampHilite();
		theHold.Begin();
		theMtl->DeleteMtl(index);
		theHold.Accept(GetString(IDS_DEL_SUBMTL));
		UpdateSubMtlNames();
		UpdateMtlDisplay();
	}
}

void MtlSwitcherDlgProc::OnLock()
{
	if (theMtl)
		subSelection = theMtl->GetActiveMtlIndex();
}

void MtlSwitcherDlgProc::ScrollToActiveMaterial()
{
	int index = theMtl->GetActiveMtlIndex();
	int scrollpos = theMtl->GetSortOrderFromMtlIndex(index);

	if (scrollpos >= 0)
	{
		int delta = scrollpos - theMtl->offset;
		// See if already visible
		if ((delta >= 0) && (delta < UI_MATERIAL_COUNT))
			return;

		theMtl->offset = scrollpos;
		theMtl->ClampOffset();

		UpdateSubMtlNames();
	}
}

void MtlSwitcherDlgProc::OnNameSearch(const TCHAR* name)
{
	if (theMtl->SelectMaterialByName(name))
		ScrollToActiveMaterial();
}

INT_PTR MtlSwitcherDlgProc::DlgProc(TimeValue t, IParamMap2* map, HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int id = LOWORD(wParam);
	int code = HIWORD(wParam);
	switch (msg)
	{
	case WM_INITDIALOG: 
		InitDialog(hwndDlg);
		return TRUE;

	case WM_VSCROLL:
		VScroll(LOWORD(wParam), (short int)HIWORD(wParam));
		break;

	case WM_MOUSEWHEEL:
		return WheelScroll((short int)HIWORD(wParam));

	case WM_COMMAND:
		switch (id)
		{
		case IDC_SWITCHER_MTL0:
		case IDC_SWITCHER_MTL1:
		case IDC_SWITCHER_MTL2:
		case IDC_SWITCHER_MTL3:
		case IDC_SWITCHER_MTL4:
		case IDC_SWITCHER_MTL5:
		case IDC_SWITCHER_MTL6:
		case IDC_SWITCHER_MTL7:
		case IDC_SWITCHER_MTL8:
		case IDC_SWITCHER_MTL9:
		case IDC_SWITCHER_MTL10:
		case IDC_SWITCHER_MTL11:
		case IDC_SWITCHER_MTL12:
		case IDC_SWITCHER_MTL13:
		case IDC_SWITCHER_MTL14:
		case IDC_SWITCHER_MTL15:
		case IDC_SWITCHER_MTL16:
		case IDC_SWITCHER_MTL17:
		case IDC_SWITCHER_MTL18:
		case IDC_SWITCHER_MTL19:
		case IDC_SWITCHER_MTL20:
		case IDC_SWITCHER_MTL21:
		case IDC_SWITCHER_MTL22:
		case IDC_SWITCHER_MTL23:
		case IDC_SWITCHER_MTL24:
		case IDC_SWITCHER_MTL25:
		case IDC_SWITCHER_MTL26:
		case IDC_SWITCHER_MTL27:
		case IDC_SWITCHER_MTL28:
		case IDC_SWITCHER_MTL29:
		{
			int index = SubMtlNumFromMaterialID(id) + theMtl->offset;
			int materialIndex = theMtl->GetSortedEntry(index) - 1; 
			if (materialIndex >= 0 && materialIndex < theMtl->NumSubMtls())
			{
				PostMessage(hwmedit, WM_SUB_MTL_BUTTON, materialIndex, (LPARAM)theMtl);
			}
		}
		break;

		case IDC_SWITCHER_INSERT:
			OnInsert();
			break;

		case IDC_SWITCHER_DELETE:
			OnDelete();
			break;

		case IDC_INDEX_SORT:
			OnSortIndex();
			break;

		case IDC_MATERIAL_SORT:
			OnSortMaterial();
			break;

		case IDC_NAME_SORT:
			OnSortName();
			break;

#ifdef ALLOW_NAME_SEARCH
		case IDC_NAME_SELECTION_EDIT:
			if ((HIWORD(wParam) == EN_CHANGE) && iNameSearch)
			{
				if (iNameSearch->GotReturn())
				{
					TSTR buf;
					iNameSearch->GetText(buf);
					OnNameSearch(buf);
				}
			}
			break;
#endif

		case IDC_MTL_NAME0:
		case IDC_MTL_NAME1:
		case IDC_MTL_NAME2:
		case IDC_MTL_NAME3:
		case IDC_MTL_NAME4:
		case IDC_MTL_NAME5:
		case IDC_MTL_NAME6:
		case IDC_MTL_NAME7:
		case IDC_MTL_NAME8:
		case IDC_MTL_NAME9:
		case IDC_MTL_NAME10:
		case IDC_MTL_NAME11:
		case IDC_MTL_NAME12:
		case IDC_MTL_NAME13:
		case IDC_MTL_NAME14:
		case IDC_MTL_NAME15:
		case IDC_MTL_NAME16:
		case IDC_MTL_NAME17:
		case IDC_MTL_NAME18:
		case IDC_MTL_NAME19:
		case IDC_MTL_NAME20:
		case IDC_MTL_NAME21:
		case IDC_MTL_NAME22:
		case IDC_MTL_NAME23:
		case IDC_MTL_NAME24:
		case IDC_MTL_NAME25:
		case IDC_MTL_NAME26:
		case IDC_MTL_NAME27:
		case IDC_MTL_NAME28:
		case IDC_MTL_NAME29:
			if (HIWORD(wParam) == EN_CHANGE)
			{
				int index = SubMtlNumFromNameID(id);
				int indexWithOffset = index + theMtl->offset;
				if (indexWithOffset >= 0 && index < UI_MATERIAL_COUNT)
				{
					int nameIndex = theMtl->GetSortedEntry(indexWithOffset) - 1;
					if (nameIndex >= 0 && nameIndex < theMtl->NumSubMtls())
					{
						TSTR buf;
						iName[index]->GetText(buf);
						theMtl->pblock->SetValue(pb_mtl_labels, 0, buf, nameIndex);
					}
				}
			}
			break;

		}
		break;

	 case WM_PAINT:
		if (!valid)
		{
			valid = TRUE;
			LoadDialog();
		}
		return FALSE;
	}

	return FALSE;
}

MtlSwitcherDlgProc::MtlSwitcherDlgProc(MtlSwitcher* m, HWND hwMtlEdit, IMtlParams* imp)
{
	dadMgr.Init(this);
	hwmedit = hwMtlEdit;
	hScroll = nullptr;

	frameHeight = -1;
	rollupHeight = -1;
	scrollHeight = -1;
	subSelection = -1;
	
	ip = imp;
	hMaterialPanel = nullptr;
	theMtl = m;
	theMtl->SetParamDlg(this);

	valid = FALSE;
	theMtl->ClampOffset();
#ifdef ALLOW_NAME_SEARCH
	iNameSearch = nullptr;
#endif

	for (int i = 0; i < UI_MATERIAL_COUNT; i++)
	{
		iMtlButton[i] = nullptr;
		iName[i] = nullptr;
		iIndex[i] = nullptr;
	}
	curTime = imp->GetTime();
}

MtlSwitcherDlgProc::~MtlSwitcherDlgProc()
{
	if (theMtl)
		theMtl->SetParamDlg(nullptr);
#ifdef ALLOW_NAME_SEARCH
	ReleaseICustEdit(iNameSearch);
#endif

	for (int i = 0; i < UI_MATERIAL_COUNT; i++)
	{
		ReleaseICustButton(iMtlButton[i]);
		ReleaseICustEdit(iName[i]);
		ReleaseICustStatus(iIndex[i]);
	}
}

//-----------------------------------------------------------------------------
//  MtlSwitcher
//-----------------------------------------------------------------------------


MtlSwitcher::MtlSwitcher(BOOL loading)
		: mReshadeRQ(RR_None)
{
	paramDlg = nullptr;
	ignoreNotify = FALSE;
	isBatchChange = FALSE;
	pblock = nullptr;
	lastActiveIndex = -1;

	MtlSwitcherCD.MakeAutoParamBlocks(this); // make and intialize paramblock2

	Init();
	if (!loading)
		PopulateSortedIndex();

	pblock->DefineParamAlias(_T("material1"), pb_materials, 0); // Alias for base material to support macroRecording

	GetCOREInterface()->RegisterTimeChangeCallback(this);
}

MtlSwitcher::~MtlSwitcher()
{
	GetCOREInterface()->UnRegisterTimeChangeCallback(this);
}

void MtlSwitcher::Init()
{
	offset = 0;
}

void MtlSwitcher::Reset()
{
	Init();
	BulkSetNumSubMtls(INITIAL_MATERIAL_COUNT);
}

void* MtlSwitcher::GetInterface(ULONG id)
{
	if (id == IID_IReshading)
		return (IReshading*)(this);

	if (I_REFTARGWRAPPINGREFTARG == id)
		return (IRefTargWrappingRefTarg*)(this);
	
	return Mtl::GetInterface(id);
}

void MtlSwitcher::NotifyChanged()
{
	if (!isBatchChange)
		NotifyDependents(FOREVER, PART_MTL, REFMSG_CHANGE);
}

void MtlSwitcher::ClampOffset()
{
	int count = pblock->Count(pb_materials);
	if (offset > count - UI_MATERIAL_COUNT)
		offset = count - UI_MATERIAL_COUNT;

	if (offset < 0)
		offset = 0;
}

// These allow the real-time renderer to display a material appearance.
Color MtlSwitcher::GetAmbient(int mtlNum, BOOL backFace)
{
	Mtl* m = GetActiveMtl();
	if (m)
		return m->GetAmbient(mtlNum, backFace);

	return Color(0, 0, 0);
}

Color MtlSwitcher::GetDiffuse(int mtlNum, BOOL backFace)
{
	Mtl* m = GetActiveMtl();
	if (m)
		return m->GetDiffuse(mtlNum, backFace);

	return Color(0, 0, 0);
}

Color MtlSwitcher::GetSpecular(int mtlNum, BOOL backFace)
{
	Mtl* m = GetActiveMtl();
	if (m)
		return m->GetSpecular(mtlNum, backFace);

	return Color(0, 0, 0);
}

float MtlSwitcher::GetXParency(int mtlNum, BOOL backFace)
{
	Mtl* m = GetActiveMtl();
	if (m)
		return m->GetXParency(mtlNum, backFace);

	return 0.0f;
}

float MtlSwitcher::GetShininess(int mtlNum, BOOL backFace)
{
	Mtl* m = GetActiveMtl();
	if (m)
		return m->GetShininess(mtlNum, backFace);

	return 0.0f;
}

float MtlSwitcher::GetShinStr(int mtlNum, BOOL backFace)
{
	Mtl* m = GetActiveMtl();
	if (m)
		return m->GetShinStr(mtlNum, backFace);

	return 0.0f;
}

float MtlSwitcher::WireSize(int mtlNum, BOOL backFace)
{
	Mtl* m = GetActiveMtl();
	if (m)
		return m->WireSize(mtlNum, backFace);

	return 0.0f;
}

RefTargetHandle MtlSwitcher::Clone(RemapDir& remap)
{
	MtlSwitcher* mnew = new MtlSwitcher(TRUE);
	*((MtlBase*)mnew) = *((MtlBase*)this);

	mnew->offset = offset;

	mnew->ReplaceReference(PBLOCK_REF, remap.CloneRef(pblock));

	mnew->PopulateSortedIndex();

	BaseClone(this, mnew, remap);
	return (RefTargetHandle)mnew;
}

ParamDlg* MtlSwitcher::CreateParamDlg(HWND hwMtlEdit, IMtlParams* imp)
{
	IAutoMParamDlg* dlg = MtlSwitcherCD.CreateParamDlgs(hwMtlEdit, imp, this);
	switcher_param_blk.SetUserDlgProc(new MtlSwitcherDlgProc(this, hwMtlEdit, imp));
	return dlg;
}

void MtlSwitcher::Update(TimeValue t, Interval& valid)
{
#ifdef TEXMAP_INDEX
		Texmap* tm = pblock->GetTexmap(pb_index_map);
		if (tm)
			tm->Update(t, valid);
#endif

	valid &= Validity(t);
	// loop through and update all sub materials
	int count = pblock->Count(pb_materials);
	for (int i = 0; i < count; i++)
	{
		// tell sub material to update
		Mtl* m = pblock->GetMtl(pb_materials, 0, i);
		if (m)
			m->Update(t, valid);
	}
}

Interval MtlSwitcher::Validity(TimeValue t)
{
	Interval valid = FOREVER;

	// We only worry about the validity interval of the selection ID and its respective material
	Mtl* m = GetActiveMtl(t, valid);
	if (m)
		valid &= m->Validity(t);

	return valid;
}

struct sortEl
{
	Mtl* mtl;
	int index;
	TCHAR name[MAX_NAME_STRING];
	bool bDescending;
};

 static int __cdecl dummyComp(const void* arg1, const void* arg2)
{
	 return 0;
 }

static int __cdecl cmpNames(const void* arg1, const void* arg2)
{
	sortEl* s1 = (sortEl*)arg1;
	sortEl* s2 = (sortEl*)arg2;

	int res = 0;

	Mtl* m1 = s1->mtl;
	Mtl* m2 = s2->mtl;
	if (m1 && m2)
	{
		res = _tcsicmp(m1->GetFullName(true).data(), m2->GetFullName(true).data());
	}
	else
	{
		if (m1)
			res = -1;
		if (m2)
			res = 1;
	}

	if (res == 0)
	{
		int id1 = s1->index;
		int id2 = s2->index;
		res = id2 < id1 ? 1 : id1 < id2 ? -1 : 0;
	}
	if (!s1->bDescending || res == 0)
		return res;

	return -res;
}

static int __cdecl cmpSlotNames(const void* arg1, const void* arg2)
{
	sortEl* s1 = (sortEl*)arg1;
	sortEl* s2 = (sortEl*)arg2;

	size_t len1 = _tcslen(s1->name);
	size_t len2 = _tcslen(s2->name);

	int res = 0;

	if (len1 && len2)
	{
		res = _tcsicmp(s1->name, s2->name);
	}
	else
	{
		if (len1)
			res = -1;
		if (len2)
			res = 1;
	}
	if (res == 0)
	{
		// If names are equal, use index
		int id1 = s1->index;
		int id2 = s2->index;
		res = id2 < id1 ? 1 : id1 < id2 ? -1 : 0;
	}
	if (!s1->bDescending || res == 0)
		return res;

	return -res;
}

void MtlSwitcher::SortMtlsByName(bool bDescending)
{
	SortMtls(true, bDescending, cmpNames);
}

void MtlSwitcher::SortMtlsByID(bool bDescending)
{
	SortMtls(false, bDescending, dummyComp);
}

void MtlSwitcher::SortMtlsBySlotName(bool bDescending)
{
	SortMtls(true, bDescending, cmpSlotNames);
}

void MtlSwitcher::SortMtls(bool bUseCompareFunc, bool bDescending, CompareFnc cmp)
{
	int n = pblock->Count(pb_materials);
	if (n <= 1)
		return;

	macroRec->Disable();

	// Check if we're using a compare function, otherwise order based on index
	if (bUseCompareFunc)
	{
		Tab<sortEl> sortlist;
		sortlist.SetCount(n);
		for (int i = 0; i < n; i++)
		{
			sortEl s;

			s.mtl = pblock->GetMtl(pb_materials, 0, i);
			const TCHAR* name = pblock->GetStr(pb_mtl_labels, 0, i);
			if (name)
				_tcsncpy(s.name, name, MAX_NAME_STRING - 1);
			else
				_tcscpy(s.name, _T(""));

			s.index = i+1;
			s.bDescending = bDescending;

			sortlist[i] = s;
		}
		sortlist.Sort(cmp);

		// All we actually do is update the sort order list
		for (int i = 0; i < n; i++)
		{
			sortEl& s = sortlist[i];
			SetSortedEntry(i, s.index);
		}
	} else 
	{
		// Just to make sure we're aligned
		ResizeSortedIndex(n);
		for (int i = 0; i < n; i++)
		{
			if (bDescending)
				SetSortedEntry(i, n - i);
			else
				SetSortedEntry(i, i + 1);
		}
	}
	//	NotifyChanged();
	macroRec->Enable();
}

Mtl* MtlSwitcher::SelectMaterialByID(int index)
{
	if ((index < 1) || index > NumSubMtls())
		return nullptr;

	pblock->SetValue(pb_switcher_selection, TIMENOW, index);
	return GetActiveMtl();
}

Mtl* MtlSwitcher::SelectMaterialByName(TSTR name)
{
	if (_tcslen(name) == 0)
		return nullptr;

	// Search the names, but by current sort order
	DbgAssert(SortedOrderCount() == pblock->Count(pb_mtl_labels));

	int count = SortedOrderCount();
	int selection = -1;
	for (int i=0; i<count; i++)
	{
		int nameIndex = GetSortedEntry(i); 
		if ((nameIndex > 0) && (nameIndex <= pblock->Count(pb_mtl_labels)))
		{
			nameIndex--;
			const TCHAR* thisName = pblock->GetStr(pb_mtl_labels, 0, nameIndex);
			if (thisName && (_tcsicmp(thisName, name) == 0))
			{
				selection = nameIndex;
				break;
			}
		}
	}

	if (selection >= 0)
	{
		selection++;
		pblock->SetValue(pb_switcher_selection, TIMENOW, selection);
		return GetActiveMtl();
	}

	return nullptr;
}

TSTR MtlSwitcher::GetSelectedName()
{
	TSTR name;

	int id = GetActiveMtlIndex();
	if ((id >= 0) && (id < pblock->Count(pb_mtl_labels)))
	{
		name = pblock->GetStr(pb_mtl_labels, TIMENOW, id);
	}

	return name;
}

class DelSubRestore : public RestoreObj
{
	MtlSwitcher* mtl;
	SingleRefMaker subm;
	int nsub;
	int priorActive;
	TSTR name;
	int sortedId;
	int offset;
	bool isSubOnly;

public:
	DelSubRestore(MtlSwitcher* m, int i, bool subOnly);
	~DelSubRestore()
	{
	}
	void Restore(int isUndo) override;
	void Redo() override;
	TSTR Description() override
	{
		return _T("DelSubMtlRestore");
	}
};

DelSubRestore::DelSubRestore(MtlSwitcher* m, int i, bool subOnly)
{
	theHold.Suspend();

	nsub = i;
	mtl = m;
	isSubOnly = subOnly;
	priorActive = mtl->pblock->GetInt(pb_switcher_selection);

	subm.SetRef(m->pblock->GetMtl(pb_materials, 0, i));
	name = mtl->pblock->GetStr(pb_mtl_labels, 0, i);
	sortedId = mtl->GetSortedEntry(i);
	offset = mtl->offset;  // so we can correct any scrolling

	theHold.Resume();
}

void DelSubRestore::Restore(int isUndo)
{
	if (isUndo)
	{
		Mtl* foo = nullptr;
		if (mtl->paramDlg)
			mtl->paramDlg->RemovePStampHilite();

		int priorIgnore = mtl->ignoreNotify;
		mtl->ignoreNotify = true;

		Mtl* sm = (Mtl*)subm.GetRef();
		mtl->pblock->Insert(pb_materials, nsub, 1, &sm);
		const TCHAR* pname = name.data();
		mtl->pblock->Insert(pb_mtl_labels, nsub, 1, &pname);
		mtl->pblock->SetValue(pb_num_mats, 0, mtl->pblock->Count(pb_mtl_labels));
		mtl->InsertSortedIndex(nsub, sortedId);
		mtl->offset = offset;

		mtl->ignoreNotify = priorIgnore;

		if (!isSubOnly && priorActive != mtl->pblock->GetInt(pb_switcher_selection))
			mtl->pblock->SetValue(pb_switcher_selection, TIMENOW, priorActive);
		
		if (mtl->paramDlg)
			mtl->paramDlg->UpdateSubMtlNames();
	}
}

void DelSubRestore::Redo()
{
	mtl->DeleteMtl(nsub);
}

class InsertMtlRestore : public RestoreObj
{
public:
	MtlSwitcher* m;
	int index;

	InsertMtlRestore(MtlSwitcher* mul, int i)
	{
		m = mul;
		index = i;
	}

	~InsertMtlRestore()
	{
	}

	void Restore(int isUndo) override
	{
		m->DeleteMtl(index);
	}

	void Redo() override
	{
		m->InsertMtl(index);
	}

	TSTR Description() override
	{
		return _T("InsertMtlRestore");
	}
};

class AddMtlsRestore : public RestoreObj
{
public:
	MtlSwitcher* m;
	int num;
	SingleRefMaker* subm;
	TSTR* _name;
	int* sorted_id;	// Rename this
	int selectedMaterialIndex;

	AddMtlsRestore(MtlSwitcher* mul, int n)
	{
		m = mul;
		num = n;
		selectedMaterialIndex = mul->GetActiveMtlIndex();
		subm = new SingleRefMaker[n];
		_name = new TSTR[n];
		sorted_id = new int[num];
	}

	~AddMtlsRestore()
	{
		delete[] subm;
		delete[] _name;
		delete[] sorted_id;
	}

	// Used to store all the missing data for all the materials being added
	// At creation time, only n and num can be set.
	void SetMissingFields(MtlSwitcher* mul, int n)
	{
		theHold.Suspend();

		for (int i = 0; i < num; i++)
		{
			subm[i].SetRef(mul->pblock->GetMtl(pb_materials, 0, n + i));
			_name[i] = mul->pblock->GetStr(pb_mtl_labels, 0, n + i);
			sorted_id[i] = n + i;
		}
		theHold.Resume();
	}

	void Restore(int isUndo) override
	{
		for (int i=0; i<num; i++)
			m->DeleteLastMtl();
	}

	void Redo() override
	{
		int newCount = m->NumSubMtls() + num;

		m->pblock->SetValue(pb_num_mats, 0, newCount);
	}

	TSTR Description() override
	{
		return _T("AddMtlsRestore");
	}
};

class DeleteMtlsRestore : public RestoreObj
{
public:
	MtlSwitcher* m;
	int num;
	SingleRefMaker* subm;
	TSTR* _name;
	int* sorted_id; // Rename this
	int selectedMaterialIndex;

	DeleteMtlsRestore(MtlSwitcher* mul, int n)
	{
		m = mul;
		num = n;
		selectedMaterialIndex = mul->GetActiveMtlIndex();
		subm = new SingleRefMaker[n];
		_name = new TSTR[n];
		sorted_id = new int[num];

		int fromIndex = m->NumSubMtls() - n  -1;
		SetMissingFields(m, fromIndex);
	}

	~DeleteMtlsRestore()
	{
		delete[] subm;
		delete[] _name;
		delete[] sorted_id;
	}

	// Used to store all the missing data for all the materials being delete
	// At creation time, only n and num can be set.
	void SetMissingFields(MtlSwitcher* mul, int n)
	{
		theHold.Suspend();

		for (int i = 0; i < num; i++)
		{
			subm[i].SetRef(mul->pblock->GetMtl(pb_materials, 0, n + i));
			_name[i] = mul->pblock->GetStr(pb_mtl_labels, 0, n + i);
			sorted_id[i] = n + i;
		}
		theHold.Resume();
	}

	void Restore(int isUndo) override
	{
		int newCount = m->NumSubMtls() + num;
		m->pblock->SetValue(pb_num_mats, 0, newCount);
		m->NotifyChanged();
	}

	void Redo() override
	{
		int newCount = m->NumSubMtls() - num;
		if (newCount > 0)
		{
			m->pblock->SetValue(pb_num_mats, 0, newCount);
		}
	}

	TSTR Description() override
	{
		return _T("DeleteMtlsRestore");
	}
};

void MtlSwitcher::InsertMtl(int index)
{
	SuspendSetKeyMode();

	InsertMtlRestore* undoData = nullptr;

	if (theHold.Holding())
		theHold.Put(undoData = new InsertMtlRestore(this, index));
	theHold.Suspend();

	// Should create one if necessary
	Mtl* newMtl = nullptr;

	int priorNotify = ignoreNotify;
	ignoreNotify = TRUE;

	macroRec->Disable();

	TCHAR dummyLabel[] = _T("");
	const TCHAR* pname = dummyLabel;//label.data();
	pblock->Insert(pb_mtl_labels, index, 1, &pname);

	int currentActiveIndex = GetActiveMtlIndex();

	// Update all of our indexes
	InsertSortedIndex(index + 1);

	pblock->Insert(pb_materials, index, 1, &newMtl);

	if (IsUILocked() && index <= currentActiveIndex)
		SetActiveMtlIndex(currentActiveIndex + 1);

	ignoreNotify = priorNotify;
	macroRec->Enable();

	pblock->SetValue(pb_num_mats, 0, pblock->Count(pb_materials));

	NotifyChanged();

	theHold.Resume();
	ResumeSetKeyMode();
}

void MtlSwitcher::AddMtl(ReferenceTarget* rt, int id, const TCHAR* name)
{
	int matCount = pblock->Count(pb_materials);
	if (matCount >= MAX_MATERIALS)
		return;

	SuspendSetKeyMode();

	AddMtlsRestore* undoData = nullptr;

	if (theHold.Holding())
		theHold.Put(undoData = new AddMtlsRestore(this, 1));

	theHold.Suspend();
	
	macroRec->Disable();

	int ignorePrevious = ignoreNotify;
	ignoreNotify = TRUE;

	//pblock->SetValue(pb_num_mats, 0, pblock->Count(pb_materials));
	pblock->SetCount(pb_mtl_labels, matCount+1);
	if (rt)
		pblock->SetValue(pb_mtl_labels, 0, name, matCount);

	ResizeSortedIndex(matCount+1);
	
	pblock->SetCount(pb_materials, matCount + 1);
	
	macroRec->Enable();
	pblock->SetValue(pb_num_mats, 0, matCount + 1);

	if (rt)
		pblock->SetValue(pb_materials, 0, rt, matCount);
	
	if (undoData)
		undoData->SetMissingFields(this, matCount);

	ignoreNotify = ignorePrevious;

	NotifyChanged();

	theHold.Resume();
	ResumeSetKeyMode();
}

void MtlSwitcher::BulkSetNumSubMtls(int num)
{
	int currentCount = pblock->Count(pb_materials);

	if (num == currentCount)
		return;

	isBatchChange = TRUE;
	
	macroRec->Disable();

	int ignorePrevious = ignoreNotify;
	ignoreNotify = TRUE;

	if (num < currentCount)
	{
		int deleteCount = currentCount - num;
		pblock->Delete(pb_mtl_labels, num, deleteCount);
		pblock->Delete(pb_materials, num, deleteCount);
		int selection = GetActiveMtlIndex();
		if (selection > num)
			pblock->SetValue(pb_switcher_selection, TIMENOW, num);
	}
	else
	{
		pblock->SetCount(pb_materials, num);
		pblock->SetCount(pb_mtl_labels, num);
	}

	ResizeSortedIndex(num);

	ignoreNotify = ignorePrevious;
	macroRec->Enable();

	isBatchChange = FALSE;

	NotifyDependents(FOREVER, PART_MTL, REFMSG_CHANGE);
	
	if (num > currentCount)
	{
		if (paramDlg)
			paramDlg->UpdateSubMtlNames();
	}
	else
		ReloadDialog();
}

void MtlSwitcher::SetNumSubMtls(int num)
{
	if (num == pblock->Count(pb_materials))
		return;

	if (num < 1)
		num = 1;
	if (num > MAX_MATERIALS)
		num = MAX_MATERIALS;

	isBatchChange = TRUE; //REFMSG_CHANGE once at the end.

	int n = num - pblock->Count(pb_materials);
	if (n > 0)
	{
		int startOffset = pblock->Count(pb_materials);

		AddMtlsRestore* undoData = nullptr;
		if (theHold.Holding())
			theHold.Put(undoData = new AddMtlsRestore(this, n));

		theHold.Suspend();

		int ignorePrevious = ignoreNotify;
		ignoreNotify = TRUE;

		for (int i = 0; i < n; i++)
			AddMtl();

		ignoreNotify = ignorePrevious;
		
		if (undoData)
			undoData->SetMissingFields(this, startOffset);

		theHold.Resume();
	}
	else if (n <= 0)
	{
		n = -n;

		DeleteMtlsRestore* undoData = nullptr;
		if (theHold.Holding())
			theHold.Put(undoData = new DeleteMtlsRestore(this, n));

		theHold.Suspend();

		int ignorePrevious = ignoreNotify;
		ignoreNotify = TRUE;

		for (int i = 0; i < n; i++)
		{
			if (pblock->Count(pb_materials) == 1) // Always have 1
				break;

			DeleteLastMtl();
		}
		if (GetActiveMtlIndex() > pblock->Count(pb_materials))
			SetActiveMtlIndex(pblock->Count(pb_materials) - 1);

		ignoreNotify = ignorePrevious;
		theHold.Resume();
	}

	ClampOffset();

	isBatchChange = FALSE; // send REFMSG_CHANGE once at the end

	if (!theHold.RestoreOrRedoing())
		NotifyChanged();
}

void MtlSwitcher::DeleteLastMtl()
{
	int matCount = pblock->Count(pb_materials);

	// Don't delete if there is only 1
	if (matCount == 1)
		return;

	int lastEntry = matCount - 1;

	if (theHold.Holding())
		theHold.Put(new DelSubRestore(
				this, lastEntry, IsUILocked() && paramDlg && paramDlg->MayUpdateSubSelectionOnDelete(lastEntry)));
	theHold.Suspend();

	int ignorePrior = ignoreNotify;
	ignoreNotify = TRUE;

	pblock->Delete(pb_materials, lastEntry, 1);

	macroRec->Disable();
	pblock->Delete(pb_mtl_labels, lastEntry, 1);
	DeleteLastSortedIndex();
	offset--;

	int ui_selected = pblock->GetInt(pb_switcher_selection, TIMENOW);
	if (ui_selected == lastEntry)
		pblock->SetValue(pb_switcher_selection, TIMENOW, ui_selected - 1);

	pblock->SetValue(pb_num_mats, 0, pblock->Count(pb_materials));
	macroRec->Enable();

	ignoreNotify = ignorePrior;

	ClampOffset();
	NotifyChanged();
	theHold.Resume();
}

void MtlSwitcher::DeleteMtl(int index)
{
	int matCount = pblock->Count(pb_materials);

	// Don't delete if there is only 1
	if (matCount == 1)
	{
		return;
	}

	if (index >= 0 && index < matCount)
	{
		int ignorePrior = ignoreNotify;
		ignoreNotify = TRUE;

		macroRec->Disable();

		int deleteIndex = FindAndDeleteSortedIndex(index + 1);

		// Test labels as materials haven't been removed yet
		if (deleteIndex == pblock->Count(pb_mtl_labels))
		{
			offset--;
		}

		int newSelected = GetSortedEntry(index);

		if (theHold.Holding())
			theHold.Put(new DelSubRestore(this, index, IsUILocked() && index > GetActiveMtlIndex()));
		theHold.Suspend();

		pblock->Delete(pb_mtl_labels, index, 1);

		// See if we need to decrement the selection
		if (IsUILocked() && index < GetActiveMtlIndex())
		{
			pblock->SetValue(pb_switcher_selection, TIMENOW, GetActiveMtlIndex());
		}

		macroRec->Enable();

		pblock->SetValue(pb_num_mats, 0, pblock->Count(pb_materials));

		ignoreNotify = ignorePrior;

		pblock->Delete(pb_materials, index, 1);

		ClampOffset();
		NotifyChanged();
		theHold.Resume();
	}
}

RefTargetHandle MtlSwitcher::GetReference(int i)
{
	if (i == PBLOCK_REF)
		return pblock;

	return nullptr;
}

void MtlSwitcher::SetReference(int i, RefTargetHandle rtarg)
{
	if (i == PBLOCK_REF)
		pblock = (IParamBlock2*)rtarg;
}

void MtlSwitcher::SetSubMtlAndName(int mtlid, Mtl* m, TSTR& nm)
{
	if (mtlid >= 0)
	{
		pblock->SetValue(pb_materials, 0, m, mtlid);
		const TCHAR* pname = nm.data();
		pblock->SetValue(pb_mtl_labels, 0, pname, mtlid);
	}
	else
	{
		int n = pblock->Count(pb_materials);
		SetNumSubMtls(n + 1);
		// set the n-th mtl to have mtlid for its mtl ID.
		pblock->SetValue(pb_materials, 0, m, n);
		pblock->SetValue(pb_mtl_labels, 0, nm.data(), n);
	}
	if (paramDlg)
		paramDlg->UpdateSubMtlNames();
}

void MtlSwitcher::SetSubMtl(int i, Mtl* m)
{
	if (i >= 0)
	{
		pblock->SetValue(pb_materials, 0, m, i);
	}
	else
	{
		int n = pblock->Count(pb_materials);
		SetNumSubMtls(n + 1);
		pblock->SetValue(pb_materials, 0, m, n);
	}
}

TSTR MtlSwitcher::GetSubMtlSlotName(int i, bool localized)
{
	TSTR s;
	if ((i >= 0) && (i < pblock->Count(pb_mtl_labels)))
	{
		const TCHAR * name = pblock->GetStr(pb_mtl_labels, 0, i);
		if (name)
			s.printf(_T("(%d) %s"), i + 1, name);
		else
			s.printf(_T("(%d)"), i + 1);
	}

	else
		s.printf(_T("(%d)"), i + 1);
	return s;
}

Animatable* MtlSwitcher::SubAnim(int i)
{
	if (i == 0)
		return pblock;

#ifdef TEXMAP_INDEX
	if (i == 1)
		return pblock->GetTexmap(pb_index_map, 0);

	i -= 2;
#else
	i--;
#endif
	return pblock->GetMtl(pb_materials, 0, i);
}

TSTR MtlSwitcher::SubAnimName(int i, bool localized)
{
	if (i == 0)
		return (localized ? GetString(IDS_MATERIAL_PARAMS) : _T("Switcher Material"));

#ifdef TEXMAP_INDEX		
	if (i == 1)
	{
		Texmap* t = pblock->GetTexmap(pb_index_map, 0);
		if (t)
			return t->GetFullName(localized);

		return _T("None");
	}

	int id = i - 2;
#else
	int id = i - 1;
#endif

	const TCHAR* name = nullptr;
	TSTR s;
	if (id >= 0 && id < pblock->Count(pb_mtl_labels))
	{
		name = pblock->GetStr(pb_mtl_labels, 0, id);
		if (name)
			s.printf(_T("(%d) %s"), id + 1, name);
		else
			s.printf(_T("(%d)"), id + 1);
	}

	TSTR nm;
	Mtl* m = nullptr;

	if (id >= 0 && id < pblock->Count(pb_materials))
	{
		m = pblock->GetMtl(pb_materials, 0, id);
	}
	if (m)
		nm.printf(_T("%s: %s"), s, m->GetFullName(localized));
	else
		nm.printf(_T("%s: None"), s);

	return nm;
}


RefResult MtlSwitcher::NotifyRefChanged(
		const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate)
{
	switch (message)
	{
	case REFMSG_CHANGE:
		if (ignoreNotify)
			return REF_SUCCEED; 

		if (hTarget == pblock)
		{
			int tabIndex = 0;
			ParamID changing_param = pblock->LastNotifyParamID(tabIndex);
			MtlSwitcherDlgProc* ms = paramDlg;

			if ((changing_param == pb_switcher_selection))
			{
				mReshadeRQ = RR_NeedReshade;
				if (ms)
				{
					ms->RemovePStampHilite();
					ms->ScrollToActiveMaterial();
					ms->UpdateSelectedLabel();
				}
			}
			else
			{
#ifdef TEXMAP_INDEX
				if (changing_param == pb_index_map)
				{
					if (ms)
						ms->UpdateSelectionUI();

					TimeChanged(TIMENOW);
				}
#endif

				if (changing_param == pb_materials) 
				{
					// If not one of the entries, then it is count was changed (ie. MXS)
					if (tabIndex == -1)
					{
						pblock->SetCount(pb_mtl_labels, pblock->Count(pb_materials));
						PopulateSortedIndex();
						if (ms)
							ms->InitSortHeaders();						
					}
					
					Mtl* m = GetSubMtl(tabIndex);
					if (m)
					{
						m->DiscardPStamp(mPStampSize);
						if (mActivePStampSize != mPStampSize)
							m->DiscardPStamp(mActivePStampSize);
						IReshading* r = static_cast<IReshading*>(m->GetInterface(IID_IReshading));
						mReshadeRQ = r == nullptr ? RR_None : r->GetReshadeRequirements();
					}

					if (ms)
					{
						ms->LoadDialog();
						ms->UpdateNumMatSpinner();
					}
				} else if (changing_param == pb_mtl_labels)
				{
					// First let's see if the user changed the count (via maxscript)
					if (pblock->Count(pb_mtl_labels) != pblock->GetInt(pb_num_mats))
					{
						pblock->SetValue(pb_num_mats, 0, pblock->Count(pb_mtl_labels));
					}
					else if (ms)
					{
						if (tabIndex == GetActiveMtlIndex())
							ms->UpdateSelectedLabel();
					}
				}
				else if (ms)
				{
					if (changing_param == pb_num_mats)
						ms->LoadDialog();

					if (changing_param == pb_locked)
					{
						ms->OnLock();
						ms->HighlightActiveStamp();
					}
				}
			}
			switcher_param_blk.InvalidateUI(changing_param);
		}

		if (hTarget != nullptr)
		{
			switch (hTarget->SuperClassID())
			{
			case MATERIAL_CLASS_ID: {
				IReshading* r = static_cast<IReshading*>(hTarget->GetInterface(IID_IReshading));
				mReshadeRQ = r == nullptr ? RR_None : r->GetReshadeRequirements();
			}
			break;
			}
		}
		break;

	case REFMSG_SUBANIM_STRUCTURE_CHANGED:
		if ((hTarget != this) && (hTarget != pblock))
			return REF_SUCCEED;

		mReshadeRQ = RR_NeedPreshade;
		NotifyChanged();
		break;
	}

	if (isBatchChange && (message == REFMSG_CHANGE && (hTarget == pblock)))
	{
		return REF_STOP;
	}

	return (REF_SUCCEED);
}


Sampler* MtlSwitcher::GetPixelSampler(int mtlNum, BOOL backFace)
{
	Mtl* m = GetActiveMtl();
	if (m)
		return m->GetPixelSampler(mtlNum, backFace);

	return nullptr;
}

void MtlSwitcher::PreShade(ShadeContext& sc, IReshadeFragment* pFrag)
{
	IReshading* pReshading = nullptr;
	int mtlnum = sc.mtlNum;

	Mtl* m = GetActiveMtl();
	if (m)
	{
		// store sub-material number and preshade it
		pFrag->AddIntChannel(mtlnum);
		pReshading = (IReshading*)(m->GetInterface(IID_IReshading));
		if (pReshading)
			pReshading->PreShade(sc, pFrag);
	}
	else
	{
		// -1 indicates no submaterial used
		pFrag->AddIntChannel(-1);
	}
}

void MtlSwitcher::PostShade(ShadeContext& sc, IReshadeFragment* pFrag, int& nextTexIndex, IllumParams*)
{
	int mtlnum = pFrag->GetIntChannel(nextTexIndex++);
	if (mtlnum == -1)
	{
		// no submaterial was used
		sc.out.c.Black();
		sc.out.t.Black();
		return;
	}

	Mtl* m = GetActiveMtl();
	if (m)
	{
		IReshading*  pReshading = (IReshading*)(m->GetInterface(IID_IReshading));
		if (pReshading)
			pReshading->PostShade(sc, pFrag, nextTexIndex);
	} else
	{
		sc.out.c.Black();
		sc.out.t.Black();
	}
}

void MtlSwitcher::Shade(ShadeContext& sc)
{
	if (gbufID)
		sc.SetGBufferID(gbufID);

	Mtl* m = GetActiveMtl();
	if (m)
		return m->Shade(sc);
}

float MtlSwitcher::EvalDisplacement(ShadeContext& sc)
{
	Mtl* m = GetActiveMtl();
	if (m)
		return m->EvalDisplacement(sc);

	return 0.0f;
}

Interval MtlSwitcher::DisplacementValidity(TimeValue t)
{
	Interval valid = FOREVER;

	int id;
	pblock->GetValue(pb_switcher_selection, t, id, valid);

	Mtl* m = GetActiveMtl();
	if (m)
		valid &= m->DisplacementValidity(t);

	return valid;
}

#define MTL_HDR_CHUNK 0x4000
#define SWITCHER_VERSION_CHUNK 0x4010
#define NUM_MATERIALS_CHUNK 0x4020

IOResult MtlSwitcher::Save(ISave* isave)
{
	IOResult res;
	ULONG nb;

	isave->BeginChunk(MTL_HDR_CHUNK);
	res = MtlBase::Save(isave);
	isave->EndChunk();
	if (res != IO_OK)
		return res;

	isave->BeginChunk(SWITCHER_VERSION_CHUNK);
	isave->Write(&SWITCHER_VERSION, sizeof(SWITCHER_VERSION), &nb);
	isave->EndChunk();

	int numSubs = NumSubMtls();
	isave->BeginChunk(NUM_MATERIALS_CHUNK);
	isave->Write(&numSubs, sizeof(numSubs), &nb);
	isave->EndChunk();

	return res;
}

 IOResult MtlSwitcher::Load(ILoad* iload) {
	IOResult res;
	int id;
	ULONG nb;

	while (IO_OK == (res = iload->OpenChunk()))
	{
		switch (id = iload->CurChunkID())
		{
			case MTL_HDR_CHUNK:
				res = MtlBase::Load(iload);
				break;

			case NUM_MATERIALS_CHUNK: {
				int numSubs;
				res = iload->Read(&numSubs, sizeof(numSubs), &nb);
				PopulateSortedIndex(numSubs);
			}
		}
		iload->CloseChunk();
		if (res != IO_OK)
			return res;
	}

	return IO_OK;
}

 
bool MtlSwitcher::IsOutputConst(ShadeContext& sc, int stdID)
{
	Mtl* m = GetActiveMtl();
	if (m)
		return m->IsOutputConst(sc, stdID);

	return true;
}

bool MtlSwitcher::EvalColorStdChannel(ShadeContext& sc, int stdID, Color& outClr)
{
	Mtl* m = GetActiveMtl();
	if (m)
		return m->EvalColorStdChannel(sc, stdID, outClr);

	return false;
}

bool MtlSwitcher::EvalMonoStdChannel(ShadeContext& sc, int stdID, float& outVal)
{
	Mtl* m = GetActiveMtl();
	if (m)
		return m->EvalMonoStdChannel(sc, stdID, outVal);

	return false;
}
