/**********************************************************************
 *<
	FILE: CELLTEX.CPP

	DESCRIPTION: A Cellular texture

	CREATED BY: Rolf Berteig

	HISTORY: created 3/22/97

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#include "util.h"
#include "stdmat.h"
#include "iparamm2.h"
#include "texutil.h"

extern HINSTANCE hInstance;

#define SHOW_3DMAPS_WITH_2D

#define CELLTEX_NAME		GetString(IDS_RB_CELLULAR)

//class CellTexParamDlg;

#define NSUBTEX	3

class CellTex : public Texmap { 
	public:

		static ParamDlg* xyzGenDlg;	
		static ParamDlg* texoutDlg;

		IParamBlock2 *pblock;		// ref 0
		XYZGen *xyzGen;				// ref 1
		TextureOutput *texout;		// ref 2
		Texmap* subTex[NSUBTEX];	// ref 3-5

//		CellTexParamDlg *paramDlg;

		// Caches
		Interval ivalid;
		Interval mapValid;
		CRITICAL_SECTION csect;
		Color cellCol, divCol1, divCol2;
		float size, spread, low, high, mid, var, blend, varOff;
		float highMinuslow, midMinuslow, highMinusmid, iterations;
		float rough, smooth;
		int type, fract, useCellMap, useDiv1Map, useDiv2Map, adapt;

#ifdef SHOW_3DMAPS_WITH_2D
		TexHandle *texHandle;
		Interval texHandleValid;
#endif

		CellTex();
		~CellTex() {
			DeleteCriticalSection(&csect);
#ifdef SHOW_3DMAPS_WITH_2D
			DiscardTexHandle();
#endif
			}

		ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);
		//ULONG Requirements(int subMtlNum);
		ULONG LocalRequirements(int subMtlNum);
		void LocalMappingsRequired(int subMtlNum, BitArray & mapreq, BitArray &bumpreq);  
		void Update(TimeValue t, Interval& valid);
		void Init();
		void Reset();
		Interval Validity(TimeValue t) {Interval v = FOREVER; Update(t, v); return v;}		
		XYZGen* GetTheXYZGen() { return xyzGen; }

		// Evaluation
		AColor EvalColor(ShadeContext& sc);
		float EvalMono(ShadeContext& sc);
		AColor EvalFunction(ShadeContext& sc, float u, float v, float du, float dv);		
		Point3 EvalNormalPerturb(ShadeContext& sc);
		float CellFunc(Point3 pt,float dpsq,Point3 &np,BOOL noAdapt);

		// Methods to access texture maps of material
		int NumSubTexmaps() {return NSUBTEX;}
		Texmap* GetSubTexmap(int i) {return subTex[i];}
		void SetSubTexmap(int i, Texmap *m);
		TSTR GetSubTexmapSlotName(int i, bool localized) override;

		Class_ID ClassID() {return CELLTEX_CLASSID;}
		SClass_ID SuperClassID() {return TEXMAP_CLASS_ID;}
		void GetClassName(MSTR& s, bool localized) const override { s = localized ? GetString(IDS_RB_CELLULAR) : _T("Cellular"); } // mjm - 2.3.99
		void DeleteThis() {delete this;}	

		int NumSubs() {return 6;}
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i, bool localized) override;
		int SubNumToRefNum(int subNum) {return subNum;}

 		int NumRefs() {return 6;}
		RefTargetHandle GetReference(int i);
private:
		virtual void SetReference(int i, RefTargetHandle rtarg);
public:

		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);
		RefTargetHandle Clone(RemapDir &remap);
		RefResult NotifyRefChanged( const Interval& changeInt, RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message, BOOL propagate);

// JBW: direct ParamBlock access is added
		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return pblock; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; } // return id'd ParamBlock
		BOOL SetDlgThing(ParamDlg* dlg);

		// Same as Marble
		bool IsLocalOutputMeaningful( ShadeContext& sc ) { return true; }

#ifdef SHOW_3DMAPS_WITH_2D
		void DiscardTexHandle() {
			if (texHandle) {
				texHandle->DeleteThis();
				texHandle = NULL;
				}
			}
		BOOL SupportTexDisplay() { return TRUE; }
		void ActivateTexDisplay(BOOL onoff) {
			if (!onoff) DiscardTexHandle();
			}
		DWORD_PTR GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker);
#endif SHOW_3DMAPS_WITH_2D

	};

class CellTexClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() {return TRUE;}
	void*			Create(BOOL loading) {return new CellTex;}
	const TCHAR*	ClassName() { return GetString(IDS_RB_CELLULAR_CDESC); }
	const TCHAR*	NonLocalizedClassName() { return _T("Cellular"); }
	SClass_ID		SuperClassID() {return TEXMAP_CLASS_ID;}
	Class_ID 		ClassID() {return CELLTEX_CLASSID;}
	const TCHAR* 	Category() {return TEXMAP_CAT_3D;}
// JBW: new descriptor data accessors added.  Note that the 
//      internal name is hardwired since it must not be localized.
	const TCHAR*	InternalName() { return _T("cellularTex"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle
	};

static CellTexClassDesc cellTexCD;
ClassDesc* GetCellTexDesc() {return &cellTexCD;}
ParamDlg* CellTex::xyzGenDlg;	
ParamDlg* CellTex::texoutDlg;

#define MAX_ITERATIONS	20.0f


//--- Parameter Map/Parameter block IDs ------------------------------
enum { cellular_params };  // pblock ID
// grad_params param IDs


enum 
{ 
	cellular_celcolor, cellular_divcol1, cellular_divcol2,
	cellular_celmap, cellular_divmap1, cellular_divmap2,
	cellular_map1_on, cellular_map2_on, cellular_map3_on, 
	cellular_variation,cellular_size,cellular_spread,
	cellular_lowthresh,cellular_midthresh,cellular_highthresh,
	cellular_type, cellular_fractal,cellular_iteration,
	cellular_rough, cellular_smooth,cellular_adaptive,// main grad params 

	cellular_coords, cellular_output	  // access for UVW mapping
};

static ParamBlockDesc2 cellular_param_blk ( cellular_params, _T("parameters"),  0, &cellTexCD, P_AUTO_CONSTRUCT + P_AUTO_UI, 0, 
	//rollout
	IDD_CELLTEX_PARAMS, IDS_RB_CELLPARAMS, 0, 0, NULL, 
	// params
	cellular_celcolor,	 _T("cellColor"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_RB_CELLCOLOR,	
		p_default,		Color(1.0,1.0,1.0), 
		p_ui,			TYPE_COLORSWATCH, IDC_CELLTEX_CELLCOLOR, 
		p_nonLocalizedName, _T("Cell Color"),
		p_end,
	cellular_divcol1,	 _T("divColor1"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_RB_DIVCOLOR1,	
		p_default,		Color(0.5,0.5,0.5), 
		p_ui,			TYPE_COLORSWATCH, IDC_CELLTEX_DIVCOL1, 
		p_nonLocalizedName, _T("Division Color1"),
		p_end,
	cellular_divcol2,	 _T("divColor2"),	TYPE_RGBA,				P_ANIMATABLE,	IDS_RB_DIVCOLOR2,	
		p_default,		Color(0,0,0), 
		p_ui,			TYPE_COLORSWATCH, IDC_CELLTEX_DIVCOL2, 
		p_nonLocalizedName, _T("Division Color2"),
		p_end,
	cellular_celmap,		_T("cellMap"),		TYPE_TEXMAP,			P_OWNERS_REF,	IDS_RB_CELLMAP,
		p_refno,		3,
		p_subtexno,		0,		
		p_ui,			TYPE_TEXMAPBUTTON, IDC_CELLTEX_CELLCOLOR_MAP,
		p_end,
	cellular_divmap1,		_T("divMap1"),		TYPE_TEXMAP,			P_OWNERS_REF,	IDS_RB_DIVMAP1,
		p_refno,		4,
		p_subtexno,		1,		
		p_ui,			TYPE_TEXMAPBUTTON, IDC_CELLTEX_DIVCOL1_MAP,
		p_nonLocalizedName, _T("DivisionMap1"),
		p_end,
	cellular_divmap2,		_T("divMap2"),		TYPE_TEXMAP,			P_OWNERS_REF,	IDS_RB_DIVMAP2,
		p_refno,		5,
		p_subtexno,		2,		
		p_ui,			TYPE_TEXMAPBUTTON, IDC_CELLTEX_DIVCOL2_MAP,
		p_nonLocalizedName, _T("DivisionMap2"),
		p_end,
	cellular_map1_on,	_T("map1Enabled"), TYPE_BOOL,			0,				IDS_PW_MAP1_ON,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHECKBOX, IDC_CELLTEX_CELLCOLOR_USEMAP,
		p_end,
	cellular_map2_on,	_T("map2Enabled"), TYPE_BOOL,			0,				IDS_PW_MAP2_ON,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHECKBOX, IDC_CELLTEX_DIVCOL1_USEMAP,
		p_end,
	cellular_map3_on,	_T("map3Enabled"), TYPE_BOOL,			0,				IDS_PW_MAP3_ON,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHECKBOX, IDC_CELLTEX_DIVCOL2_USEMAP,
		p_end,

	cellular_variation,	_T("variation"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_RB_VARIATION,
		p_default,		0.f,
		p_range,		0.0, 100.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_CELLTEX_VAR, IDC_CELLTEX_VARSPIN, 0.1f, 
		p_end,
	cellular_size,	_T("size"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_RB_SIZE,
		p_default,		5.f,
		p_range,		0.001f,999999999.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_CELLTEX_SIZE, IDC_CELLTEX_SIZESPIN, 0.1f, 
		p_end,
	cellular_spread,	_T("spread"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_RB_SPREAD,
		p_default,		0.5f,
		p_range,		0.001f,999999999.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_CELLTEX_SPREAD,IDC_CELLTEX_SPREADSPIN, 0.01f, 
		p_end,

	cellular_lowthresh,	_T("lowThresh"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_RB_LOW,
		p_default,		0.0f,
		p_range,		0.0f,1.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_CELLTEX_LOW,IDC_CELLTEX_LOWSPIN, 0.01f, 
		p_nonLocalizedName, _T("Low"),
		p_end,
	cellular_midthresh,	_T("midThresh"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_RB_MID,
		p_default,		0.5f,
		p_range,		0.0f,1.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_CELLTEX_MID,IDC_CELLTEX_MIDSPIN, 0.01f, 
		p_nonLocalizedName, _T("Mid"),
		p_end,
	cellular_highthresh,	_T("highThresh"),   TYPE_FLOAT,			P_ANIMATABLE,	IDS_RB_HIGH,
		p_default,		1.0f,
		p_range,		0.0f,1.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_CELLTEX_HIGH,IDC_CELLTEX_HIGHSPIN, 0.01f, 
		p_nonLocalizedName, _T("High"),
		p_end,

	cellular_type, _T("type"), TYPE_INT,				0,				IDS_RB_TYPE,
		p_default,		0,
		p_range,		0,	2,
		p_ui,			TYPE_RADIO, 2, IDC_CELLTEX_CIRCULAR, IDC_CELLTEX_IRREGULAR,
		p_end,

	cellular_fractal, _T("fractal"), TYPE_BOOL,				0,		IDS_RB_FRACTAL,
		p_default,		0,
		p_ui,			TYPE_SINGLECHECKBOX,  IDC_CELLTEX_FRACTAL,
		p_enable_ctrls,	3, cellular_iteration,cellular_rough,cellular_adaptive,
		p_end,

	cellular_iteration,		_T("iteration"),	TYPE_FLOAT,	P_ANIMATABLE,	IDS_RB_ITERATIONS,
		p_default,		3.f,
		p_range,		1.0, MAX_ITERATIONS,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_CELLTEX_ITER,IDC_CELLTEX_ITERSPIN, 0.01f, 
		p_nonLocalizedName, _T("Iterations"),
		p_enabled,		FALSE,
		p_end,

	cellular_rough,		_T("roughness"),	TYPE_FLOAT,	P_ANIMATABLE,	IDS_RB_ROUGHNESS,
		p_default,		0.f,
		p_range,		0.0, 1.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_CELLTEX_ROUGH,IDC_CELLTEX_ROUGHSPIN, 0.01f, 
		p_enabled,		FALSE,
		p_end,

	cellular_smooth,	_T("smooth"),	TYPE_FLOAT,	P_ANIMATABLE,	IDS_RB_BUMPSMOOTHING,
		p_default,		0.1f,
		p_range,		0.0, 1.0f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_CELLTEX_BUMPSMOOTH,IDC_CELLTEX_BUMPSMOOTHSPIN, 0.01f, 
		p_nonLocalizedName, _T("Bump smoothing"),
		p_end,

	cellular_adaptive, _T("adaptive"), TYPE_BOOL,				0,		IDS_PW_ADAPTIVE,
		p_default,		1,
		p_ui,			TYPE_SINGLECHECKBOX,  IDC_CELLTEX_ADAPTIVE,
		p_enabled,		FALSE,
		p_end,
	cellular_coords,		_T("coords"),	TYPE_REFTARG,		P_OWNERS_REF,	IDS_PW_COORDS,
		p_refno,		1, 
		p_end,
	cellular_output,		_T("output"),	TYPE_REFTARG,		P_OWNERS_REF,	IDS_PW_OUTPUT,
		p_refno,		2, 
		p_end,


	p_end
);


#define PARAMDESC_LENGH 18

static ParamBlockDescID descVer0[] = {
	{ TYPE_POINT3, NULL, TRUE, cellular_celcolor }, // Cell color
	{ TYPE_POINT3, NULL, TRUE, cellular_divcol1 }, // Div col 1
	{ TYPE_POINT3, NULL, TRUE, cellular_divcol2 }, // Div col 2
	{ TYPE_FLOAT,  NULL, TRUE, cellular_variation },	// variation
	{ TYPE_FLOAT,  NULL, TRUE, cellular_size },	// size
	{ TYPE_FLOAT,  NULL, TRUE, cellular_spread },	// spread
	{ TYPE_FLOAT,  NULL, TRUE, cellular_lowthresh },	// low thresh
	{ TYPE_FLOAT,  NULL, TRUE, cellular_midthresh },	// mid thresh
	{ TYPE_FLOAT,  NULL, TRUE, cellular_highthresh },	// high thresh
	{ TYPE_INT,  NULL, FALSE, cellular_type },	// type
	{ TYPE_INT,  NULL, FALSE, cellular_fractal },	// fractal
	{ TYPE_FLOAT,  NULL, TRUE, cellular_iteration },// iterations	
	};

static ParamBlockDescID descVer1[] = {
	{ TYPE_POINT3, NULL, TRUE, cellular_celcolor }, // Cell color
	{ TYPE_POINT3, NULL, TRUE, cellular_divcol1 }, // Div col 1
	{ TYPE_POINT3, NULL, TRUE, cellular_divcol2 }, // Div col 2
	{ TYPE_FLOAT,  NULL, TRUE, cellular_variation },	// variation
	{ TYPE_FLOAT,  NULL, TRUE, cellular_size },	// size
	{ TYPE_FLOAT,  NULL, TRUE, cellular_spread },	// spread
	{ TYPE_FLOAT,  NULL, TRUE, cellular_lowthresh },	// low thresh
	{ TYPE_FLOAT,  NULL, TRUE, cellular_midthresh },	// mid thresh
	{ TYPE_FLOAT,  NULL, TRUE, cellular_highthresh },	// high thresh
	{ TYPE_INT,  NULL, FALSE, cellular_type },	// type
	{ TYPE_INT,  NULL, FALSE, cellular_fractal },	// fractal
	{ TYPE_FLOAT,  NULL, TRUE, cellular_iteration },// iterations
	{ TYPE_INT,  NULL, FALSE, cellular_map1_on },	// use cell col map
	{ TYPE_INT,  NULL, FALSE, cellular_map2_on },	// use div1 col map
	{ TYPE_INT,  NULL, FALSE, cellular_map3_on },	// use div2 col map
	};

static ParamBlockDescID descVer2[] = {
	{ TYPE_RGBA, NULL, TRUE, cellular_celcolor }, // Cell color
	{ TYPE_RGBA, NULL, TRUE, cellular_divcol1 }, // Div col 1
	{ TYPE_RGBA, NULL, TRUE, cellular_divcol2 }, // Div col 2
	{ TYPE_FLOAT,  NULL, TRUE, cellular_variation },	// variation
	{ TYPE_FLOAT,  NULL, TRUE, cellular_size },	// size
	{ TYPE_FLOAT,  NULL, TRUE, cellular_spread },	// spread
	{ TYPE_FLOAT,  NULL, TRUE, cellular_lowthresh },	// low thresh
	{ TYPE_FLOAT,  NULL, TRUE, cellular_midthresh },	// mid thresh
	{ TYPE_FLOAT,  NULL, TRUE, cellular_highthresh },	// high thresh
	{ TYPE_INT,  NULL, FALSE, cellular_type },	// type
	{ TYPE_INT,  NULL, FALSE, cellular_fractal },	// fractal
	{ TYPE_FLOAT,  NULL, TRUE, cellular_iteration },// iterations
	{ TYPE_INT,  NULL, FALSE, cellular_map1_on },	// use cell col map
	{ TYPE_INT,  NULL, FALSE, cellular_map2_on },	// use div1 col map
	{ TYPE_INT,  NULL, FALSE, cellular_map3_on },	// use div2 col map
	{ TYPE_FLOAT,  NULL, TRUE, cellular_rough },// rough
	{ TYPE_FLOAT,  NULL, TRUE, cellular_smooth },// smooth
	{ TYPE_INT,  NULL, FALSE, cellular_adaptive },	// adaptive
	};

#define PBLOCK_LENGTH	18

static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer0,12,0),	
	ParamVersionDesc(descVer1,15,1),
	ParamVersionDesc(descVer2,18,2)
	};
#define NUM_OLDVERSIONS	3

//--- CellTex Methods -----------------------------------------------

ParamDlg* CellTex::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp)
	{
//	paramDlg = new CellTexParamDlg(this,imp,hwMtlEdit);	
//	return paramDlg;
	// create the rollout dialogs
	xyzGenDlg = xyzGen->CreateParamDlg(hwMtlEdit, imp);	
	IAutoMParamDlg* dlg = cellTexCD.CreateParamDlgs(hwMtlEdit, imp, this);
	texoutDlg = texout->CreateParamDlg(hwMtlEdit, imp);
	// add the secondary dialogs to the main
	dlg->AddDlg(xyzGenDlg);
	dlg->AddDlg(texoutDlg);
//	celTex_param_blk.SetUserDlgProc(new NoiseDlgProc(this));
	return dlg;

	}

void CellTex::Update(TimeValue t, Interval& valid)
	{
	EnterCriticalSection(&csect);
	if (!ivalid.InInterval(t)) {
		ivalid = FOREVER;		
		xyzGen->Update(t,ivalid);
		texout->Update(t,ivalid);

		pblock->GetValue(cellular_celcolor,t,cellCol,ivalid);
		pblock->GetValue(cellular_divcol1,t,divCol1,ivalid);
		pblock->GetValue(cellular_divcol2,t,divCol2,ivalid);
		pblock->GetValue(cellular_variation,t,var,ivalid);
		pblock->GetValue(cellular_size,t,size,ivalid);
		pblock->GetValue(cellular_spread,t,spread,ivalid);
		pblock->GetValue(cellular_lowthresh,t,low,ivalid);
		pblock->GetValue(cellular_midthresh,t,mid,ivalid);
		pblock->GetValue(cellular_highthresh,t,high,ivalid);		
		pblock->GetValue(cellular_type,t,type,ivalid);
		pblock->GetValue(cellular_fractal,t,fract,ivalid);
		pblock->GetValue(cellular_iteration,t,iterations,ivalid);		
		pblock->GetValue(cellular_map1_on,t,useCellMap,ivalid);
		pblock->GetValue(cellular_map2_on,t,useDiv1Map,ivalid);
		pblock->GetValue(cellular_map3_on,t,useDiv2Map,ivalid);		
		pblock->GetValue(cellular_rough,t,rough,ivalid);
		pblock->GetValue(cellular_smooth,t,smooth,ivalid);
		pblock->GetValue(cellular_adaptive,t,adapt,ivalid);
		
		
		smooth *= 0.7f;
		rough = 2.0f-rough;

		highMinuslow = high-low;		
		midMinuslow = mid - low;
		highMinusmid = high - mid;		
		if (type) {
			spread = spread/2.0f;
			}		
		var /= 50.0f;
		varOff = 1.0f-var * 0.5f;	

		
		NotifyDependents(FOREVER, PART_TEXMAP, REFMSG_DISPLAY_MATERIAL_CHANGE);
		}
	if (!mapValid.InInterval(t))
	{
		mapValid.SetInfinite();
		for (int i=0; i<NSUBTEX; i++) {
			if (subTex[i]) 
				subTex[i]->Update(t,mapValid);
		}
	}
	valid &= mapValid;
	valid &= ivalid;
	LeaveCriticalSection(&csect);
	}

#define IN_TO_M(x) (x / 39.370079f)

void CellTex::Init()
	{
	if (xyzGen) xyzGen->Reset();
	else ReplaceReference(1, GetNewDefaultXYZGen());

	if (texout) texout->Reset();
	else ReplaceReference(2, GetNewDefaultTextureOutput());

    RegisterDistanceDefault(_T("Cellular Params"), _T("Size"), 5.0f, IN_TO_M(5.0f));
    float size = GetDistanceDefault(_T("Cellular Params"), _T("Size"));
	pblock->SetValue(cellular_size,0,size);

	fract = 0;
	ivalid.SetEmpty();
	mapValid.SetEmpty();
	}

void CellTex::Reset(){
	cellTexCD.Reset(this, TRUE);	// reset all pb2's
	DeleteReference(3);
	DeleteReference(4);
	DeleteReference(5);
	Init();
	}

CellTex::CellTex()
	{
//	paramDlg = NULL;
#ifdef SHOW_3DMAPS_WITH_2D
	texHandle = NULL;
#endif
	pblock   = NULL;
	xyzGen   = NULL;
	texout   = NULL;
	subTex[0] = subTex[1] = subTex[2] = NULL;
	cellTexCD.MakeAutoParamBlocks(this);	// make and intialize paramblock2
	Init();
	InitializeCriticalSection(&csect);
	ivalid.SetEmpty();
	mapValid.SetEmpty();
	}


BOOL CellTex::SetDlgThing(ParamDlg* dlg)
{
	// JBW: set the appropriate 'thing' sub-object for each
	// secondary dialog
	if ((xyzGenDlg!= NULL) && (dlg == xyzGenDlg))
		xyzGenDlg->SetThing(xyzGen);
	else if ((texoutDlg!= NULL) && (dlg == texoutDlg))
		texoutDlg->SetThing(texout);
	else 
		return FALSE;
	return TRUE;
}

#ifdef SHOW_3DMAPS_WITH_2D
DWORD_PTR CellTex::GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker) {
	if (texHandle) {
		if (texHandleValid.InInterval(t))
			return texHandle->GetHandle();
		else DiscardTexHandle();
		}
	texHandle = thmaker.MakeHandle(GetVPDisplayDIB(t,thmaker,texHandleValid));
	return texHandle->GetHandle();
	}
#endif

#define ITER	3.0f
static Point3 ptOffset(1000.0f,1000.0f,1000.0f);

AColor CellTex::EvalColor(ShadeContext& sc)
	{	
	// Get object point
	Point3 p,dp;
	if (gbufID) sc.SetGBufferID(gbufID);
	xyzGen->GetXYZ(sc,p,dp);
	p += ptOffset;
	p = p/size;
	
	// Eval maps
	Color cellC, div1C, div2C;
	if (useCellMap && subTex[0]) cellC = subTex[0]->EvalColor(sc);
	else cellC = cellCol;
	if (useDiv1Map && subTex[1]) div1C = subTex[1]->EvalColor(sc);
	else div1C = divCol1;
	if (useDiv2Map && subTex[2]) div2C = subTex[2]->EvalColor(sc);
	else div2C = divCol2;

	// Evaluate cell function
	float dist[2];
	int ids[2];
	float u;
	if (type) {
		if (fract) FractalCellFunction(p,iterations,rough,2,dist,ids);
		else CellFunction(p,2,dist,ids);
		u = 1.0f - (dist[1]-dist[0])/spread;		
	} else {
		if (fract) FractalCellFunction(p,iterations,rough,1,dist,ids);
		else CellFunction(p,1,dist,ids);
		u = dist[0]/spread;
		}

	// Vari cell color
	if (var>0.0f) {
		float vr = RandFromCellID(ids[0])*var + varOff;
		cellC.r = cellC.r*vr;
		cellC.g = cellC.g*vr;
		cellC.b = cellC.b*vr;
		cellC.ClampMinMax();		
		}

	if (u<low) return texout->Filter(RGBA(cellC));
	if (u>high) return texout->Filter(RGBA(div2C));
	if (u<mid) {
		u = (u-low)/(midMinuslow);
		return texout->Filter(RGBA(div1C*u + (1.0f-u)*cellC));
	} else {
		u = (u-mid)/(highMinusmid);
		return texout->Filter(RGBA(div2C*u + (1.0f-u)*div1C));
		}
	}

float CellTex::EvalMono(ShadeContext& sc)
	{
	return Intens(EvalColor(sc));
	}

#define SMOOTH	0.2f

float CellTex::CellFunc(Point3 pt,float dpsq,Point3 &np,BOOL noAdapt)
	{
	float dist[3];
	Point3 grad[3];
	float u, iter = 0.0f;
	
	if (fract) {
		if (adapt) {
			iter = iterations/dpsq;
			if (iter>MAX_ITERATIONS) iter = MAX_ITERATIONS; // RB 2/19/99: We run into some sort of floating point limitation (I think) when this gets over about 20 or so.
			if (iter<1.0f)  iter = 1.0f;
		} else {
			iter = iterations;
			}
		}

	if (type) {
		if (fract) FractalCellFunction(
			 pt/size,iter,rough,3,dist,NULL,grad,smooth);
		else CellFunction(pt/size,3,dist,NULL,grad,smooth);
		u  = (dist[1]-dist[0])/spread;
		np = (grad[1]-grad[0])/spread;
	} else {
		if (fract) FractalCellFunction(
			 pt/size,iter,rough,2,dist,NULL,grad,smooth);
		else CellFunction(pt/size,2,dist,NULL,grad,smooth);
		u  = dist[0]/spread;
		np = grad[0]/spread;
		}
	
#if 1
	if (u<low+SMOOTH) {
		if (u<low) {
			np = Point3(0,0,0);
		} else {
			float s = (u-low)/SMOOTH;
			np = np*s;
			}
		return 0.0f;
		}
	if (u>high) {
		if (u>high+SMOOTH) {
			np = Point3(0,0,0);
		} else {
			float s = 1.0f-(u-high)/SMOOTH;
			np = np*s;
			}
		return 1.0f;
		}
#else
	if (u<0.0f) {
		np = Point3(0,0,0);
		return 0.0f;
		}
	if (u>1.0f) {
		np = Point3(0,0,0);
		return 1.0f;
		}
#endif
	return u;
	}

Point3 CellTex::EvalNormalPerturb(ShadeContext& sc)
	{
	Point3 p,dp;
	xyzGen->GetXYZ(sc,p,dp);	
	p += ptOffset;
	Point3 np(0.0f,0.0f,0.0f);
	float dpsq = DotProd(dp,dp);		
	float d = CellFunc(p,dpsq,np,sc.InMtlEditor());

	Texmap* sub0 = (useCellMap && subTex[0])?subTex[0]:NULL; 
	Texmap* sub1 = (useDiv1Map && subTex[1])?subTex[1]:NULL; 
	Texmap* sub2 = (useDiv2Map && subTex[2])?subTex[2]:NULL; 
	if (d<low) {
		if (sub0) 
			np  = sub0->EvalNormalPerturb(sc);
		}
	else 
	if (d>high) {
		if (sub2) 
			np  = sub2->EvalNormalPerturb(sc);
		}
	else {
		Point3 M[3];
		xyzGen->GetBumpDP(sc,M);
		np = Point3( DotProd(np,M[0]),DotProd(np,M[1]),DotProd(np,M[2]));
		if (d<mid) {
			if (sub0||sub1) {
				float a,b;
				Point3 da,db;
				// d((1-k)*a + k*b ) = dk*(b-a) + k*(db-da) + da
				d = (d-low)/(midMinuslow);

				// div1C*u + (1.0f-u)*cellC) ;
				if (sub0) {
					a = sub0->EvalMono(sc);
					da = sub0->EvalNormalPerturb(sc);
					}
				else {
					 a = 1.0f;
					 da = Point3(0.0f,0.0f,0.0f);
					 }
				if (sub1) {
					b = sub1->EvalMono(sc);
					db = sub1->EvalNormalPerturb(sc);
					}
				else {
					 b = 1.0f;
					 db = Point3(0.0f,0.0f,0.0f);
					 }
				np = (b-a)*np + d*(db-da) + da;
				}
			} 
		else {
			if (sub1 || sub2) {
				float a,b;
				Point3 da,db;
				// div2C*u + (1.0f-u)*div1C);
				d = (d-mid)/(highMinusmid);
				if (sub1) {
					a = sub1->EvalMono(sc);
					da = sub1->EvalNormalPerturb(sc);
					}
				else {
					 a = 1.0f;
					 da = Point3(0.0f,0.0f,0.0f);
					 }
				if (sub2) {
					b = sub2->EvalMono(sc);
					db = sub2->EvalNormalPerturb(sc);
					}
				else {
					 b = 1.0f;
					 db = Point3(0.0f,0.0f,0.0f);
					 }
				np = (b-a)*np + d*(db-da)+ da;
				}
			}
		}

//	float d = CellFunc(p,dpsq,np,sc.InMtlEditor());
//	Point3 tmp;
//	float div = type ? -0.1875f : 0.0375f;
//	Point3 DP[3];
//	xyzGen->GetBumpDP(sc,DP);
//	np.x = (CellFunc(p+DP[0],dpsq,tmp,sc.InMtlEditor()) - d)/div;
//	np.y = (CellFunc(p+DP[1],dpsq,tmp,sc.InMtlEditor()) - d)/div;
//	np.z = (CellFunc(p+DP[2],dpsq,tmp,sc.InMtlEditor()) - d)/div;

	if (type) np = np * -0.5f;	
	

	return texout->Filter(sc.VectorFromNoScale(np,REF_OBJECT));
	}

void CellTex::SetSubTexmap(int i, Texmap *m)
	{
	ReplaceReference(i+3,m);
	if (i==0)
		{
		cellular_param_blk.InvalidateUI(cellular_celmap);
		mapValid.SetEmpty();
		}
	else if (i==1)
		{
		cellular_param_blk.InvalidateUI(cellular_divmap1);
		mapValid.SetEmpty();
		}
	else if (i==2)
		{
		cellular_param_blk.InvalidateUI(cellular_divmap2);
		mapValid.SetEmpty();
		}

//	if (paramDlg) paramDlg->UpdateSubTexNames();
	}


TSTR CellTex::GetSubTexmapSlotName(int i, bool localized)
{
	switch (i)
	{
	case 0: return localized ? GetString(IDS_RB_CELLCOLOR) : _T("Cell Color");
	case 1: return localized ? GetString(IDS_RB_DIVCOLOR1) : _T("Division Color1");
	case 2: return localized ? GetString(IDS_RB_DIVCOLOR2) : _T("Division Color2");
	default: return _T("");
	}
}

Animatable* CellTex::SubAnim(int i)
	{
	return GetReference(i);
	}

TSTR CellTex::SubAnimName(int i, bool localized)
{
	switch (i)
	{
	default:
	case 0: return localized ? GetString(IDS_RB_PARAMETERS) : _T("Parameters");
	case 1: return localized ? GetString(IDS_RB_COORDINATES) : _T("Coordinates");
	case 2: return localized ? GetString(IDS_RB_OUTPUT) : _T("Output");
	case 3: return localized ? GetString(IDS_RB_CELLMAP) : _T("CellMap");
	case 4: return localized ? GetString(IDS_RB_DIVMAP1) : _T("DivisionMap1");
	case 5: return localized ? GetString(IDS_RB_DIVMAP2) : _T("DivisionMap2");
	}
}

RefTargetHandle CellTex::GetReference(int i)
	{
	switch (i) {
		case 0:  return pblock;
		case 1:  return xyzGen;
		case 2:  return texout;
		default: return subTex[i-3];
		}
	}

void CellTex::SetReference(int i, RefTargetHandle rtarg)
	{
	switch (i) {
		case 0:  pblock = (IParamBlock2*)rtarg; break;
		case 1:  xyzGen = (XYZGen *)rtarg; break;
		case 2:  texout = (TextureOutput *)rtarg; break;
		default: subTex[i-3] = (Texmap *)rtarg; break;
		}
	}

#define MTL_HDR_CHUNK 0x4000
IOResult CellTex::Save(ISave *isave) { 
	IOResult res;
	// Save common stuff
	isave->BeginChunk(MTL_HDR_CHUNK);
	res = MtlBase::Save(isave);
	isave->EndChunk();
	if (res!=IO_OK) return res;
	return IO_OK;
	}

IOResult CellTex::Load(ILoad *iload)
	{
	IOResult res;
	int id;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(id=iload->CurChunkID())  {
			case MTL_HDR_CHUNK:
				res = MtlBase::Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}

	ParamBlock2PLCB* plcb = new ParamBlock2PLCB(versions, NUM_OLDVERSIONS, &cellular_param_blk, this, 0);
	iload->RegisterPostLoadCallback(plcb);
	return IO_OK;
	}

RefTargetHandle CellTex::Clone(RemapDir &remap)
	{
	CellTex *map = new CellTex;
	*((MtlBase*)map) = *((MtlBase*)this);  // copy superclass stuff
	map->ReplaceReference(0,remap.CloneRef(pblock));	
	map->ReplaceReference(1,remap.CloneRef(xyzGen));
	map->ReplaceReference(2,remap.CloneRef(texout));
	for (int i=0; i<NSUBTEX; i++) {
		if (subTex[i]) map->ReplaceReference(3+i,remap.CloneRef(subTex[i]));
		}
	BaseClone(this, map, remap);
	return map;
	}

ULONG CellTex::LocalRequirements(int subMtlNum)
	{
	return xyzGen->Requirements(subMtlNum);
	}

void CellTex::LocalMappingsRequired(int subMtlNum, BitArray & mapreq, BitArray &bumpreq) {
	xyzGen->MappingsRequired(subMtlNum,mapreq,bumpreq);
	}

RefResult CellTex::NotifyRefChanged(
		const Interval& changeInt,
		RefTargetHandle hTarget,
		PartID& partID,
		RefMessage message,
		BOOL propagate)
	{
	switch (message) {
		case REFMSG_CHANGE:
			ivalid.SetEmpty();
			mapValid.SetEmpty();
			if (hTarget == pblock)
			{
				ParamID changing_param = pblock->LastNotifyParamID();
				cellular_param_blk.InvalidateUI(changing_param);
#ifdef SHOW_3DMAPS_WITH_2D
				if (changing_param != -1)
					DiscardTexHandle();
#endif
			}
#ifdef SHOW_3DMAPS_WITH_2D
			else if (hTarget == xyzGen)
			{
				DiscardTexHandle();
			}
#endif
			break;
		}
	return REF_SUCCEED;
	}
