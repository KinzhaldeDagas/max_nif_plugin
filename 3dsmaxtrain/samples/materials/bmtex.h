//*********************************************************************/
// Copyright (c) 1998-2020 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//*********************************************************************/

#ifndef _BMTEX_H_
#define _BMTEX_H_

#include <max.h>
#include <imtl.h>
#include <bmmlib.h>
#include <bitmap.h>
#include <iparamm2.h>
#include <stdmat.h>
#include <AssetManagement\IAssetAccessor.h>
#include <containers/array.h>
#include <IMaterialBrowserEntryInstanceCallback.h>

class BMTex;
class BMTexDlg;
struct NotifyInfo;

// JBW: IDs for ParamBlock2 blocks and parameters
// Parameter and ParamBlock IDs, bmtex_time
enum { bmtex_params,bmtex_time};  // pblock ID

enum 
{ 
	bmtex_clipu,bmtex_clipv,bmtex_clipw,bmtex_cliph,
	bmtex_jitter,bmtex_usejitter,
	bmtex_apply,bmtex_crop_place,
	bmtex_filtering,
	bmtex_monooutput,
	bmtex_rgboutput,
	bmtex_alphasource,
	bmtex_premultalpha,
	bmtex_bitmap,
	bmtex_coords,	 // access for UVW mapping
	bmtex_output,	 //output window
	bmtex_filename   // bitmap filename virtual parameter, JBW 2/23/99
};


enum 
{ 
	bmtex_start,bmtex_playbackrate, bmtex_endcondition,
	bmtex_matidtime
};

//------------------------------------------------------------------------
// BMSampler
//------------------------------------------------------------------------

class BMSampler: public MapSampler {
public:
	BMSampler();
	void Init(BMTex* bmt, Bitmap* bm = nullptr);
	int PlaceUV(ShadeContext& sc, float &u, float &v, int iu, int iv);
	int PlaceUVFilter(ShadeContext& sc, float &u, float &v, int iu, int iv);

	// -- from MapSampler
	AColor Sample(ShadeContext& sc, float u,float v) override;
	AColor SampleFilter(ShadeContext& sc, float u,float v, float du, float dv) override;

protected:
	Bitmap *bm;
	BMTex *tex;
	int alphaSource;
	float u0,v0,u1,v1,ufac,vfac,ujit,vjit;
	int bmw,bmh,clipx, clipy, cliph;
	float fclipw,fcliph, fbmh, fbmw;
} ;

//------------------------------------------------------------------------
// BMAlphaSampler
//------------------------------------------------------------------------
class BMAlphaSampler: public BMSampler {
public:
	// -- from MapSampler
	AColor Sample(ShadeContext& sc, float u,float v) override { return AColor(0,0,0,0);}
	AColor SampleFilter(ShadeContext& sc, float u,float v, float du, float dv) override { return AColor(0,0,0,0);}
	float SampleMono(ShadeContext& sc, float u,float v) override;
	float SampleMonoFilter(ShadeContext& sc, float u,float v, float du, float dv) override;
} ;



//------------------------------------------------------------------------
// BMCropper
//------------------------------------------------------------------------
class BMCropper:public CropCallback {
	BMTexDlg *dlg;
	BMTex *tex;
	BOOL mode;
	float u0,v0,w0,h0;
public:
	float GetInitU() override { return u0; }
	float GetInitV() override { return v0; }
	float GetInitW() override { return w0; }
	float GetInitH() override { return h0; }
	BOOL GetInitMode() override { return mode; }
	void SetValues(float u, float v, float w, float h, BOOL md) override;
	void OnClose() override;
	void Init(BMTexDlg *tx, TimeValue t);
};

//------------------------------------------------------------------------
// BMTexDlg
//------------------------------------------------------------------------

//class BMTexDlg: public ParamDlg , public DADMgr{
class BMTexDlg: public ParamMap2UserDlgProc, protected BitmapNotify {
public:
	HWND hwmedit;	 // window handle of the materials editor dialog
	IMtlParams *ip;
	BMTex *theBMTex;	 // current BMTex being edited.
	HWND hPanel; // Rollup pane
	//		HWND hTime; // Time Rollup pane
	TimeValue curTime; 
	//		ICustButton *iName;
	BOOL valid;
private:
	BOOL isActive;
public:
	BOOL cropping;
	BMCropper cropper;

	//-----------------------------
	BMTexDlg(HWND hwMtlEdit, IMtlParams *imp, BMTex *m); 
	~BMTexDlg();

	// inherited from ParamMap2UserDlgProc
	void Update(TimeValue t) override;
	INT_PTR DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;    

	//    INT_PTR PanelProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
	INT_PTR TimeProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
	void LoadDialog(BOOL draw);  // stuff params into dialog
	void ReloadDialog();
	void UpdateMtlDisplay() { ip->MtlChanged(); }
	void ActivateDlg(BOOL onOff) {}
	void StuffBMNameField(HWND hwndDlg);
	void EnableAlphaButtons(BOOL isNew=FALSE);
	void EnableViewImage();
	void Invalidate() { valid = FALSE; InvalidateRect(hPanel, nullptr, 0); }
	BOOL KeyAtCurTime(int id);
	void ShowCropImage();
	void RemoveCropImage();

	// methods inherited from ParamDlg:
	Class_ID ClassID();
	void SetThing(ReferenceTarget *m) override;
	ReferenceTarget* GetThing() { return (ReferenceTarget *)theBMTex; }
	void DeleteThis() override { delete this;  }	

protected:

	// from BitmapNotify
	int Changed(ULONG flags) override;
	void VFBClosed() override;

private:

	// The temporary bitmap instance used to display in the viewer.
	Bitmap* m_displayBitmap;
};

class BMTexNotify: public BitmapNotify {
public:
	BMTex *tex;
	void SetTex(BMTex *tx) { tex  = tx; }
	int Changed(ULONG flags) override;
};

class BMTexPostLoad;

//--------------------------------------------------------------
// BMTex: A 2D texture map
//--------------------------------------------------------------

class BMTex
		: public BitmapTex
		, public MaxSDK::IMaterialBrowserEntryInstanceCallback
{
	friend class BMTexPostLoad;
	friend class BMTexDlg;
	friend class BMTexDlgProc;
	friend class BMSampler;
	friend class BMAlphaSampler;
	friend class BMTexPBAccessor;
	friend class BMTex2PostLoad;

	static ParamDlg* uvGenDlg;	
	static ParamDlg* texoutDlg;

	UVGen *uvGen;		   // ref #0


	TextureOutput *texout; // ref #2

	BMTexNotify bmNotify;
	TexHandle *texHandle;
	Interval ivalid;
	BMSampler mysamp;
	BMAlphaSampler alphasamp;

	Tab<BMSampler*> mysampList;
	Tab<BMAlphaSampler*> alphasampList;

public:

	class BMMSilentModeGuard;
	class ProxyModeDisableGuard;

	BOOL isParm2;

	float pbRate;
	TimeValue startTime;
	BOOL applyCrop;
	BOOL loading;
	BOOL loadingOld;
	BOOL placeImage;
	BOOL randPlace;
	BOOL fileNotFound; //Never checked anywhere. This flag is set to true on any error even when the file is found. Looks stale. /coz 20200508
	int filterType;
	int alphaSource;
	int rollScroll;
	int endCond;
	int alphaAsMono;
	int alphaAsRGB;
	float clipu, clipv, clipw, cliph, jitter;
	BOOL premultAlpha;
	BOOL isNew;
	BOOL loadFailed;  //Never checked, never set to true: looks stale /coz 20200508
	BOOL bmWasSet;
	int texTime;
	Interval texValid;
	Interval clipValid;
	float rumax,rumin,rvmax,rvmin;


	IParamBlock2 *pblock;   // ref #1
	IParamBlock2 *pblockTime;   // ref #3
	static BMTexDlg *paramDlg;

	// Returns the bitmap to be used for display, render, etc. If
	// "tie time to material ID" is enabled, this returns the LAST bitmap in the list.
	// Why does it return the "last" bitmap in the list? That's for historical reasons;
	// the old code used to behave like this.
	Bitmap* GetActiveBitmap() const;

	// This callback gets immediately invoked, when a user interactively creates
	// this texture in SME. We use this to show the file chooser to allow the
	// user to directly choose a file.
	void OnInteractiveCreation() override;

private:

	//==========================================================================
	// class BitmapHolder
	//
	// Encapsulates the handling of proxy vs. non-proxy bitmaps,
	// to force all BMTex methods to pass through this class and the appropriate
	// methods rather than accessing the bitmap directly.
	//==========================================================================
	class BitmapHolder {
	public:

		BitmapHolder();
		~BitmapHolder();

		// Copy constructor is1 meant to be used by STL only. It is
		// unsafe to be used in any other way as having multiple pointers to the same bitmap
		// is simply unsafe.
		BitmapHolder(const BitmapHolder& other);
		BitmapHolder& operator=(const BitmapHolder& other);

		// Enables/disables proxy mode, deleting the unused bitmap if necessary
		void EnableProxyMode(bool deleteNonProxyBitmap);
		void DisableProxyMode(bool deleteProxyBitmap);

		// Loads the bitmap described by the bitmap info; may load the proxy bitmap,
		// depending on the proxy mode.
		bool LoadBitmap(BMMRES& status, bool silenceBitmapManager);

		// Loads the bitmap in-place, re-using the same Bitmap. May load the proxy bitmap,
		// depending on the proxy mode. Does nothing if a bitmap was not already created
		// through LoadBitmap().
		void LoadInto(bool forceReload);

		// Frees both proxy and non-proxy bitmaps.
		void FreeBitmap();

		// Calls Bitmap::GoTo(), on either the proxy or non-proxy bitmap, depending on the proxy mode.
		BMMRES GotoFrame(bool silenceBitmapManager);

		// Returns the bitmap info of the non-proxy image.
		// Even when the proxy image is loaded while the high-res image is not loaded,
		// the non-proxy bitmap info will be up-to-date.
		BitmapInfo& GetBitmapInfo();
		const BitmapInfo& GetBitmapInfo() const;

		// Returns the bitmap to be used for getting pixel values; will return either
		// the proxy or non-proxy bitmap, depending on the proxy mode.
		Bitmap* GetBitmap() const;

		// Called to override the bitmap being used; overrides the proxy bitmap, if
		// and only if the input bitmap has the MAP_PROXY flag.
		void SetBitmap(Bitmap* bitmap);

		// The following methods apply the desired settings to both proxy and non-proxy bitmaps.
		int SetFilter(UINT filterType);
		void SetCroppingValues(float u, float v, float w, float h, BOOL placeImage);
		void SetNotify(BitmapNotify *bmnot);

	private:

		void FreeProxyBitmap();
		void FreeNonProxyBitmap();

		bool m_isProxyMode;

		// The non-proxy bitmap is loaded without the MAP_PROXYREQUEST flag.
		// It is therefore guaranteed not to be a reduced-size image.
		BitmapInfo m_nonProxyBitmapInfo;
		Bitmap* m_nonProxyBitmap;

		// The proxy bitmap is loaded WITH the MAP_PROXYREQUEST flag. Depending, on the proxy
		// manager settings, it may or may not be a reduced-size image.
		BitmapInfo m_proxyBitmapInfo;
		Bitmap* m_proxyBitmap;
	};

	// Indicates whether proxies have been disabled for rendering
	bool m_proxiesWereDisabledForRendering;
	// The number of times this bitmap has been temporarily disabled
	// (this value enables us to avoid multiple disables)
	int m_numTemporaryDisables;
	// The bitmap currently in use.
	BitmapHolder m_theBitmap;
	// A shortcut to the bitmap info stored in m_theBitmap.
	BitmapInfo& m_bi;
	// The list of bitmaps use with the "match time to material ID" feature.
	MaxSDK::Array<BitmapHolder> m_theBitmapList;

public:

	BMTex();
	~BMTex();
	IMtlParams *ip;
	ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) override;
	void Update(TimeValue t, Interval& valid) override;
	void Init();
	void Reset() override;
	Interval Validity(TimeValue t) override { Interval v = FOREVER; Update(t,v); return v; }
	TSTR GetFullName(bool localized) override;

	// Inherited from Texmap. Bitmap texture output is always meaningful
	bool IsLocalOutputMeaningful( ShadeContext& sc ) override { return true; }

	void SetOutputLevel(TimeValue t, float v) override {texout->SetOutputLevel(t,v); }
	void SetFilterType(int ft) override;
	void SetAlphaSource(int as) override;
	void SetEndCondition(int endcond) override;
	//		void SetEndCondition(int endcond) { 
	//			pblockTime->SetValue( bmtex_endcondition, 0, endcond);		
	//			endCond = endcond; 
	//			ParamID changedParam = pblock->LastNotifyParamID();
	//			bmtex_time_param_blk.InvalidateUI(changedParam);
	//			}
	void SetAlphaAsMono(BOOL onoff) override { 
		onoff = onoff != 0;
		BOOL oldOnOff = pblock->GetInt( bmtex_monooutput, 0 ) != 0;
		if ( onoff != oldOnOff )
			pblock->SetValue( bmtex_monooutput, 0, onoff);		
		alphaAsMono = onoff; 
	}
	void SetAlphaAsRGB(BOOL onoff) override { 
		onoff = onoff != 0;
		BOOL oldOnOff = pblock->GetInt( bmtex_rgboutput, 0 ) != 0;
		if ( onoff != oldOnOff )
			pblock->SetValue( bmtex_rgboutput, 0, onoff);		
		alphaAsRGB = onoff; 
	}
	void SetPremultAlpha(BOOL onoff) override { 
		onoff = onoff != 0;
		BOOL oldOnOff = pblock->GetInt( bmtex_premultalpha, 0 ) != 0;
		if ( onoff != oldOnOff )
			pblock->SetValue( bmtex_premultalpha, 0, onoff);		
		premultAlpha = onoff; 
	}
	void SetMapName(const TCHAR *name, bool isUIAction = false) override;
	void SetMap(const MaxSDK::AssetManagement::AssetUser& asset, bool isUIAction=false) override;
	void SetPlaybackRate(float r) override; 
	void SetStartTime(TimeValue t) override;

	/****
	void SetStartTime(TimeValue t) {
	pblockTime->SetValue( bmtex_start, 0, t);		
	startTime = t; 
	}
	void SetPlaybackRate(float r) { 
	pblockTime->SetValue( bmtex_playbackrate, 0, r);		
	pbRate = r; 
	}
	****/
	void SetClipU(TimeValue t, float f);
	void SetClipV(TimeValue t, float f) ;
	void SetClipW(TimeValue t, float f) ;
	void SetClipH(TimeValue t, float f) ;
	void SetJitter(TimeValue t, float f);

	int GetFilterType() override { return filterType; }
	int GetAlphaSource() override { return alphaSource; }
	int GetEndCondition() override { return endCond; }
	BOOL GetAlphaAsMono(BOOL onoff) override { return alphaAsMono; }
	BOOL GetAlphaAsRGB(BOOL onoff) override { return alphaAsRGB; }
	BOOL GetPremultAlpha(BOOL onoff) override { return premultAlpha; }
	const TCHAR *GetMapName() override {
		UpdateBIName(); // CCJ 4/27/99
		return m_bi.Name();
	}
	const MaxSDK::AssetManagement::AssetUser& GetMap() override ;
	TimeValue GetStartTime() override { return startTime; }
	float GetPlaybackRate() override { return pbRate; }
	StdUVGen* GetUVGen() override { return (StdUVGen*)uvGen; }
	TextureOutput* GetTexout() override { return texout; }
	Bitmap *GetBitmap(TimeValue t) override;
	float GetClipU(TimeValue t) { 	
		return pblock->GetFloat( bmtex_clipu, t); 	
	}
	float GetClipV(TimeValue t) { 							
		return pblock->GetFloat( bmtex_clipv, t); 
	}
	float GetClipW(TimeValue t) { 
		return pblock->GetFloat( bmtex_clipw, t); 
	}
	float GetClipH(TimeValue t) { 
		return pblock->GetFloat( bmtex_cliph, t); 
	}
	float GetJitter(TimeValue t) { 
		return pblock->GetFloat( bmtex_jitter, t); 
	}
	void StuffCropValues(); // stuff new values into the cropping VFB
	void EnableStuff();
	void UpdateBIName() {
		PBBitmap* bitmapPB = nullptr;
		pblock->GetValue( bmtex_bitmap, 0,bitmapPB, clipValid );
		if (bitmapPB != nullptr)   
			m_bi = bitmapPB->bi;	  //DS 2/24/99
		else m_bi.SetName(_T(""));   // DS 3/23/99
	}

	void UpdtSampler() {
		BOOL tieTimeToMatID;

		pblockTime->GetValue( bmtex_matidtime, 0, tieTimeToMatID,FOREVER);

		if (!tieTimeToMatID)
		{
			mysamp.Init(this);
			alphasamp.Init(this);
		}
		else
		{
			for (int i = 0; i < mysampList.Count(); i++)
			{
				mysampList[i]->Init(this,m_theBitmapList[i].GetBitmap());
				alphasampList[i]->Init(this,m_theBitmapList[i].GetBitmap());
			}
		}
	}

	void NotifyChanged();
	void FreeBitmap();
	BMMRES LoadBitmap(TimeValue t, bool quiet=true, bool reload=false);
	int CalcFrame(TimeValue t);
	// 'isUIAction' should be set to true if the reloading occurs as the result
	// of a "Reload" button being pressed in the UI.
	void ReloadBitmap(bool isUIAction);

	// Evaluate the color of map for the context.
	RGBA EvalColor(ShadeContext& sc) override;
	float EvalMono(ShadeContext& sc) override;
	Point3 EvalNormalPerturb(ShadeContext& sc) override;

	void DiscardTexHandle();

	BOOL SupportTexDisplay() override { return TRUE; }
	void ActivateTexDisplay(BOOL onoff) override;
	DWORD_PTR GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker) override;
	BITMAPINFO* GetVPDisplayDIB(TimeValue t, TexHandleMaker& thmaker, Interval &valid, BOOL mono=FALSE, BOOL forceW=0, BOOL forceH=0) override;

	void GetUVTransform(Matrix3 &uvtrans) override { uvGen->GetUVTransform(uvtrans); }
	int GetTextureTiling() override { return  uvGen->GetTextureTiling(); }
	int GetUVWSource() override { return uvGen->GetUVWSource(); }
	int GetMapChannel () override { return uvGen->GetMapChannel(); }
	UVGen *GetTheUVGen() override { return uvGen; }

	int RenderBegin(TimeValue t, ULONG flags) override;
	int RenderEnd(TimeValue t) override;
	int LoadMapFiles(TimeValue t) override;
	void RenderBitmap(TimeValue t, Bitmap *bm, float scale3D, BOOL filter) override;

	Class_ID ClassID() override;
	SClass_ID SuperClassID() override;
	void GetClassName(MSTR& s, bool localized) const override;
	void DeleteThis() override;

	// Requirements
	ULONG LocalRequirements(int subMtlNum) override
	{
		return uvGen->Requirements(subMtlNum);
	}

	void LocalMappingsRequired(int subMtlNum, BitArray& mapreq, BitArray& bumpreq) override
	{
		uvGen->MappingsRequired(subMtlNum, mapreq, bumpreq);
	}

	int NumSubs() override { return 4; }
	Animatable* SubAnim(int i) override;
	TSTR SubAnimName(int i, bool localized) override;
	int SubNumToRefNum(int subNum) override { return subNum; }
	void InitSlotType(int sType) override { if (uvGen) uvGen->InitSlotType(sType); }

	// From ref
	int NumRefs() override { return 4; }
	RefTargetHandle GetReference(int i) override;
	void SetReference(int i, RefTargetHandle rtarg) override;
	int RemapRefOnLoad(int iref) override; 

	RefTargetHandle Clone(RemapDir& remap) override;
	RefResult NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message,
			BOOL propagate) override;

	// From Animatable
	void EnumAuxFiles(AssetEnumCallback& nameEnum, DWORD flags) override;
	int SetProperty(ULONG id, void* data) override;
	void SetBitmap(Bitmap* bm) override;
	void FreeAllBitmaps() override
	{
		FreeBitmap();
	}

	// IO
	IOResult Save(ISave *isave) override;
	IOResult Load(ILoad *iload) override;

	// JBW: direct ParamBlock access is added
	int	NumParamBlocks() override { return 2; } // return number of ParamBlocks in this instance

	IParamBlock2* GetParamBlock(int i) override
	{
		if (i == 0)
			return pblock;
		else if (i == 1)
			return pblockTime;
		else
			return nullptr;
	}

	IParamBlock2* GetParamBlockByID(BlockID id) override
	{
		if (pblock->ID() == id)
			return pblock;
		else if (pblockTime->ID() == id)
			return pblockTime;
		else
			return nullptr;
	}

	BOOL SetDlgThing(ParamDlg* dlg) override;

	// pops up a bitmap loader dlg
	void BitmapLoadDlg() override;

	// forces the bitmap to reload and view to be redrawn
	void ReloadBitmapAndUpdate() override;

	BaseInterface* GetInterface(Interface_ID id) override;
	void fnReload() override;
	void fnViewImage() override;

	int ComputeParticleFrame(ShadeContext& sc);
	BOOL mTieTimeToMatID;
	int IsHighDynamicRange() const override;

	// -- from Animatable
	void* GetInterface(ULONG id) override;
	// lrr 06/04 
	static TCHAR	s_lastName[MAX_PATH];		// last loaded complete ( with path ) file name 

	void SetBitmapInfo(const BitmapInfo& bi) override;

private:

	static void BMTexNotify(void *param, NotifyInfo *info);
	static bool ShouldDisableProxiesForRendering(bool isMtlEditor);
	static bool KeepNonProxyInMemoryAfterRendering();

	void Notify(NotifyInfo* info);
	void EnableProxyMode(bool deleteNonProxyBitmap);
	void DisableProxyMode(bool deleteProxyBitmap);
	void ShowBitmapProxyPrecacheDialog();

	// Initializes the bitmap samplers. To be called when the active bitmap might have changed.
	void InitBitmapSamplers();

};

class BMTexPathAccessor : public IAssetAccessor	{
public:

	BMTexPathAccessor(BMTex* aTex);
	virtual ~BMTexPathAccessor();

	// path accessor functions
	MaxSDK::AssetManagement::AssetUser GetAsset() const override;
	bool SetAsset(const MaxSDK::AssetManagement::AssetUser& aNewAsset) override;

	bool IsInputAsset() const override ;

	// asset client information
	MaxSDK::AssetManagement::AssetType GetAssetType() const override ;
	const TCHAR* GetAssetDesc() const override;
	const TCHAR* GetAssetClientDesc() const override ;
	bool IsAssetPathWritable() const override;

protected:
	BMTex* mTex;
};

//==============================================================================
// class BMTex::BMMSilentModeGuard
//
// Guard class used to silence the bitmap manager temporarily.
//==============================================================================
class BMTex::BMMSilentModeGuard {
public:

	BMMSilentModeGuard(bool setSilentMode);
	~BMMSilentModeGuard();

private:

	bool m_setSilentMode;
	BOOL m_previousSilentMode;
};

//==============================================================================
// class BMTex::ProxyModeDisableGuard
//
// Guard class used to disable the proxy mode on a BMTex, temporarily, but only
// on the main bitmap (i.e. the bitmap list is not affected).
//==============================================================================
class BMTex::ProxyModeDisableGuard {
public:

	// Set 'doDisable' to false basically makes this guard do nothing;
	// it's useful to turn the guard OFF when a certain condition holds.
	ProxyModeDisableGuard(BMTex& bitmapTex, bool doDisable = true);
	~ProxyModeDisableGuard();

private:

	bool m_doDisable;
	BMTex& m_bitmapTex;
};

#endif
