/**********************************************************************
 *<
	FILE: selmod.cpp

	DESCRIPTION:  A volumetric selection modifier

	CREATED BY: Rolf Berteig

	HISTORY: 10/21/95
	06/30/2023 - Performance Improvements. M. Kaustinen


 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "mods.h"

#include "buildver.h"
#include "iparamm2.h"
#include "sctex.h"
#include "shape.h"
#include "simpobj.h"
#include "tvnode.h"
#include "iColorMan.h"
#include "MaxIcon.h"
#include "modstack.h"
#include "IParticleObjectExt.h"
#include "modsres.h"
#include "resourceOverride.h"
#include "meshadj.h"

#include "IActionItemOverrideManager.h"
#include "EditSoftSelectionMode.h"
#include "PerformanceTools.h"
#include <MeshNormalSpec.h>

#include <tbb/blocked_range.h>
#include <tbb/partitioner.h>
#include <tbb/parallel_for.h>


#define MAX_MATID	0xffff

#define REALLYBIGFLOAT	float( 1.0e+37F )

#define TIMENOW GetCOREInterface()->GetTime()

#define MCONTAINER_TVNODE_CLASS_ID Class_ID(0xe27e0f2b, 0x74fad492)
#define CONTAINER_TVNODE_CLASS_ID Class_ID(0xe27e0f2b, 0x74fad481)
#define SELNODE_TVNODE_CLASS_ID Class_ID(0xe27e0f2b, 0x74fad482)

static GenSubObjType SOT_Apparatus(14);
static GenSubObjType SOT_Center(15);
static GenSubObjType SOT_Vertex(1);
static GenSubObjType SOT_Polygon(4);

class SelModData : public LocalModData {
public:
	int id;
	INode *selfNode;
	SelModData()
	{
		selfNode = NULL;
		id = -1;
	}
	SelModData(int i)
	{
		id = i;
		selfNode = NULL;
	}
	~SelModData()
	{
	}	
	LocalModData*	Clone()
	{
		SelModData* d = new SelModData();
		d->id = -1;
		d->selfNode = NULL;

		return d;

	}	
};

class SelNodeNotify;


class SelMod;

class SelModValidatorClass : public PBValidator
{
public:
	class SelMod *mod;
private:
	BOOL Validate(PB2Value &v) 
	{
		INode *node = (INode*) v.r;

		if (node->TestForLoop(FOREVER,(ReferenceMaker *) mod)!=REF_SUCCEED) return FALSE;

		return TRUE;

	};
};


// These classes contain our "hard" and "soft" bounding boxes for each face on the
// volume selecting mesh.  They are arranged in "X" order to speed up skip ahead and
// early termination of box tests
class Box3DEntry
{
public:
	Box3 b3DHard;
	Box3 b3DSoft;
	int faceIndex;

	bool StopHardTesting(float fX)
	{
		return (fX < b3DHard.pmin.x);
	}

	bool HardContains(float fY, float fZ)
	{
		return ((b3DHard.pmin.y <= fY) && (b3DHard.pmax.y >= fY) && (b3DHard.pmin.z <= fZ) &&
				(b3DHard.pmax.z >= fZ));
	}

	bool SoftContains(float fX, float fY, float fZ)
	{
		return ((b3DSoft.pmax.x >= fX) && (b3DSoft.pmin.y <= fY) && (b3DSoft.pmax.y >= fY) &&
				(b3DSoft.pmin.z <= fZ) && (b3DSoft.pmax.z >= fZ));
	}

	bool StopSoftTesting(float fX)
	{
		return (fX < b3DSoft.pmin.x);
	}

	void SetEmpty()
	{
		b3DHard.Init();
	}

	void AssignSoftSelection(float f)
	{
		b3DSoft = b3DHard;
		b3DSoft.EnlargeBy(f);
	}

};

static int Box3DSort(const void* elem1, const void* elem2)
{
	auto* a = (Box3DEntry*)elem1;
	auto* b = (Box3DEntry*)elem2;
	if (a->b3DHard.pmin.x < b->b3DHard.pmin.x)
		return -1;

	if (a->b3DHard.pmin.x > b->b3DHard.pmin.x)
		return 1;

	return 0;
}

class Box3DArray
{
public:
	const int Count()
	{
		return boxTab.Count();
	}

	void SetCount(int count)
	{
		boxTab.SetCount(count);
	}

	void ZeroCount()
	{
		boxTab.ZeroCount();
	}

	void Init(int index, bool bSoft)
	{
		bSupportSoftSelection = bSoft;
		boxTab[index].SetEmpty();
		boxTab[index].faceIndex = index;
	}

	int GetFace(int index)
	{
		return boxTab[index].faceIndex;
	}

	void AddPoint(int index, const Point3& p)
	{
		boxTab[index].b3DHard += p;
	}

	void Expand(int index, float fSoft)
	{
		if (bSupportSoftSelection)
		{
			boxTab[index].AssignSoftSelection(fSoft);
		}
	}

	Box3DEntry* GetHardArray(float f, int &numEntries)
	{
		numEntries = Count();
		if (numEntries > 0)
		{
			if (f <= fMidHardFaceValue)
				return boxTab.Addr(0);
			
			numEntries -= midHardFace;
			return boxTab.Addr(midHardFace);
		}
		return nullptr;
	}

	Box3DEntry* GetSoftArray(float f, int& numEntries)
	{
		DbgAssert(bSupportSoftSelection);
		numEntries = Count();
		if (numEntries > 0)
		{
			if (f <= fMidSoftFaceValue)
				return boxTab.Addr(0);

			numEntries -= midSoftFace;
			return boxTab.Addr(midSoftFace);
		}
		return nullptr;
	}

	int GetFirstHardOccurance(float& f)
	{
		int count = Count();
		if (!count)
			return 0;

		for (int i=0; i<count; i++)
		{
			if (f >= boxTab[i].b3DHard.pmin.x)
			{
				f = boxTab[i].b3DHard.pmin.x;
				return i;
			}
		}
		return count -1;
	}

	int GetFirstSoftOccurance(float& f)
	{
		int count = Count();
		if (!count)
			return 0;

		for (int i = 0; i < count; i++)
		{
			if (f >= boxTab[i].b3DSoft.pmin.x)
			{
				f = boxTab[i].b3DSoft.pmin.x;
				return i;
			}
		}
		return count - 1;
	}

	void PrepForTesting(const Box3& hardBox, const Box3& softBox)
	{
		int count = Count();
		if (count)
		{
			boxTab.Sort(Box3DSort);

			// Find the "mid" face, the first face that contains the middle x value
			// this allows us to "skip ahead" for certain distance calculations
			int halfCount = (count + 1) / 2;

			fMidHardFaceValue = boxTab[halfCount].b3DHard.pmin.x;
			midHardFace = GetFirstHardOccurance(fMidHardFaceValue);
			if (bSupportSoftSelection)
			{
				fMidSoftFaceValue = boxTab[halfCount].b3DSoft.pmin.x;
				midSoftFace = GetFirstSoftOccurance(fMidSoftFaceValue);
			}
		} else
		{
			midHardFace = 0;
			midSoftFace = 0;
		}
	}

private:
	Tab<Box3DEntry> boxTab;
	bool bSupportSoftSelection = false;

	int midHardFace = 0;
	float fMidHardFaceValue = 0.0f;

	int midSoftFace = 0;
	float fMidSoftFaceValue = 0.0f;
};

// Version info added for MAXr4, where we can now be applied to a PatchMesh and retain identity!
#define SELMOD_VER1 1
#define SELMOD_VER4 4

#define SELMOD_CURRENT_VERSION SELMOD_VER4

class SMActionCB;

class SelMod : public Modifier, public EditSSCB {	
public:

	SelModValidatorClass validator;

	IParamBlock2 *pblock2,*pblock2_afr;		
	Control *tmControl;
	Control *posControl;
	DWORD flags;		

	Box3 mcBox;
	SelNodeNotify *notify;
	Matrix3 ntm;
	Matrix3 otm;
	TimeValue rt;
	int map,channel;
	int matID;
	int smG;
	BOOL autoFit;

	int useAR, level, method,vol,selType,invert;
	float pinch, falloff, bubble;
	int edgeLimit;
	Texmap *tmap;

	// These are values for the target object
	Box3 bbox;
	Box3 bboxFalloff;
	Box3 bboxTargetLocal;	// the target's bounding box expressed in local coords for early rejection
	ObjectState sos;
	ShapeObject *pathOb;
	PolyShape workShape;

	SimpleParticle *pobj;
	IParticleObjectExt* epobj;

	Mesh *msh;
	SCTex shadeContext;
	INode *targnode;

	Box3DArray box3DArray;
	Tab<Point3> normList;
	Tab<Point3> uvwList;
	
	// Our selection values
	Tab<float> fTable;

	ITrackViewNode *container;

	int version;

	static IObjParam *ip; static SelMod* curMod;
	static MoveModBoxCMode *moveMode;
	static RotateModBoxCMode *rotMode;
	static UScaleModBoxCMode *uscaleMode;
	static NUScaleModBoxCMode *nuscaleMode;
	static SquashModBoxCMode *squashMode;		
	static EditSSMode*	editSSMode;

	static Tab<ActionItem*> mOverrides;
	static SMActionCB*	mpActions;

	SelMod(BOOL create);
	~SelMod();
	void InitControl(ModContext &mc,Object *obj, TimeValue t);
	Matrix3 CompMatrix(TimeValue t,ModContext *mc, Matrix3 *ntm, BOOL scale=TRUE, BOOL offset=TRUE);
	void DoIcon(PolyLineProc& lp,BOOL sel);
	float PointInVolumeSingleThread(TimeValue t, Point3 pt, float u, float v, float w, const Matrix3& tm, const Box3& box);
	float PointInVolumeThreadSafe(TimeValue t, Point3 pt, const Matrix3 &tm, const Box3& box);
	float DistFromVolume(const Point3 pt, const Matrix3 &tm, const Matrix3 &ctm);

	bool BuildSelectionArray(Mesh& mesh, const Matrix3& tm, const Box3& box, TimeValue t);
	bool BuildSelectionArray(MNMesh& mesh, const Matrix3& tm, const Box3& box, TimeValue t);
	bool BuildSelectionArray(PatchMesh& mesh, const Matrix3& tm, const Box3& box, TimeValue t);
	
	void SelectVertices (Mesh &mesh, Matrix3 &tm, Matrix3 &ctm, Box3 &box, TimeValue t);
	void SelectVertices (MNMesh &mesh, Matrix3 &tm, Matrix3 &ctm, Box3 &box, TimeValue t);
	void SelectVertices (PatchMesh &mesh, Matrix3 &tm, Matrix3 &ctm, Box3 &box, TimeValue t);
	
	void SelectFaces (TimeValue t, Mesh &mesh, Matrix3 &tm,Box3 &box);
	void SelectFaces (TimeValue t, MNMesh &mesh, Matrix3 &tm,Box3 &box);
	void SelectPatches (TimeValue t, PatchMesh &mesh, Matrix3 &tm,Box3 &box);

	// From Animatable
	void DeleteThis() { delete this; }
	void GetClassName(MSTR& s, bool localized) const override { s= localized ? GetString(IDS_RB_VOLSELECT_CLASS) : _T("Vol. Select"); }  
	virtual Class_ID ClassID() { return Class_ID(SELECTOSM_CLASS_ID,0);}
	void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
	void EndEditParams( IObjParam *ip,ULONG flags,Animatable *next);
	const TCHAR *GetObjectName(bool localized) const override {return localized ? GetString(IDS_RB_VOLSELECT) : _T("Vol. Select");}
	CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;} 
	int SubNumToRefNum(int subNum);
	BOOL AssignController(Animatable *control,int subAnim);

	ChannelMask ChannelsUsed()  {return OBJ_CHANNELS;}		
	ChannelMask ChannelsChanged()
	{
		if (ChangesTopology())
			return TOPO_CHANNEL | GEOM_CHANNEL | SELECT_CHANNEL | SUBSEL_TYPE_CHANNEL;

		return GEOM_CHANNEL | SELECT_CHANNEL | SUBSEL_TYPE_CHANNEL;
	} 
	Class_ID InputType() {return defObjectClassID;}
	void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
	Interval LocalValidity(TimeValue t);
	Interval GetValidity(TimeValue t);

	int NumRefs() {return 4;}
	RefTargetHandle GetReference(int i);
private:
	virtual void SetReference(int i, RefTargetHandle rtarg);
public:

	int NumSubs() {return 4;}
	Animatable* SubAnim(int i);
	TSTR SubAnimName(int i, bool localized) override;

	// NS: New SubObjType API
	int NumSubObjTypes();
	ISubObjType *GetSubObjType(int i);

	// JBW: direct ParamBlock access is added
	int	NumParamBlocks() { return 2; }					// return number of ParamBlocks in this instance
	IParamBlock2* GetParamBlock(int i) { if (i == 0) return pblock2; 
	else if (i == 1) return pblock2_afr; 
	else return NULL;
	} // return i'th ParamBlock
	IParamBlock2* GetParamBlockByID(BlockID id) {if (pblock2->ID() == id) return pblock2 ;
	else if (pblock2_afr->ID() == id) return pblock2_afr ;
	else return  NULL; } // return id'd ParamBlock

	RefTargetHandle Clone(RemapDir& remap);
	RefResult NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate);

	IOResult Load(ILoad *iload);
	IOResult Save(ISave *isave);

	IOResult SaveLocalData(ISave *isave, LocalModData *pld);
	IOResult LoadLocalData(ILoad *iload, LocalModData **pld);


	int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc);
	int Display(TimeValue t, INode* inode, ViewExp *vpt, int flagst, ModContext *mc);
	void GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc);			

	void Move(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE);
	void Rotate(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Quat& val, BOOL localOrigin=FALSE);
	void Scale(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE);
	void GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
	void GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
	void ActivateSubobjSel(int level, XFormModes& modes);

	void EnableAffectRegion (TimeValue t);
	void DisableAffectRegion (TimeValue t);
	float GetPixel(TimeValue t, Point3 pt,float u, float v,float w);

	void RecurseDepth(float u1, float u2, float &fu,  ShapeObject *s,int Curve,int Piece, int &depth, Point3 fp);
	void PointToPiece(float &tempu,ShapeObject *s,int Curve,int Piece, int depth, Point3 fp);
	float SplineToPoint(Point3 p1, ShapeObject *s, TimeValue t);
	float DistSquareToFace(const float distCheck, const Point3& pa, const Point3& p1, const Point3& p2, const Point3& p3, const int faceIndex);
	float LineToPointSquared(const Point3& p1, const Point3& l1, const Point3& l2);
	void RecurseDepthB(float u1, float u2, float &fu,  BezierShape *s,int Curve,int Piece, int &depth, Point3 fp);
	void PointToPieceB(float &tempu,BezierShape *s,int Curve,int Piece, int depth, Point3 fp);

	INode* GetNodeFromModContext(ModContext *mc, int &which);

	// EditSSCB methods:
	void DoAccept(TimeValue t);
	void SetPinch(TimeValue t, float pinch);
	void SetBubble(TimeValue t, float bubble);
	float GetPinch(TimeValue t);
	float GetBubble(TimeValue t);
	void SetFalloff(TimeValue t,float falloff);
	float GetFalloff(TimeValue t);

	void EditSoftSelectionMode();

	bool ChangesTopology();

	BOOL badObject;

private:
	void AddTVNode();
	void BuildGrid();
	void FreeGrid()
	{
		grid.FreeGrid();
	}

	UniformGrid grid;
	float worldRadius;

	bool IsSelectionThreadSafe();

	Box3 TransformBox(const Box3 boxIn, const Matrix3 &m)
	{
		return boxIn * m;
	}
};

static void DoBoxIcon(BOOL sel,float length, PolyLineProc& lp);
static void DoCylinderIcon(BOOL sel,float radius, float height, PolyLineProc& lp);
static void DoSphereIcon(BOOL sel,float radius, PolyLineProc& lp);

enum { vsel_params, vsel_afr };

enum { sel_level, sel_method, sel_type, sel_volume, sel_invert, sel_node, sel_texture, sel_map, sel_map_channel, sel_matid, sel_smGroup, sel_autofit };
// vsel_afr IDs
enum { sel_use_ar, sel_falloff, sel_pinch, sel_bubble, sel_use_edge_limit,sel_edge_limit };


class SelNodeNotify  : public TVNodeNotify 
{
public:
	SelMod *s;
	SelNodeNotify(SelMod *smod)
	{
		s = smod;
	}
	RefResult NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate)
	{
		if(( (message == REFMSG_CHANGE) || 
			(message == REFMSG_MOUSE_CYCLE_COMPLETED)
			) && (!s->badObject))
		{
			s->pblock2->SetValue(sel_method,0,s->method);
			return REF_STOP; 
		}
		return REF_SUCCEED ;
	}
};

//--- Action table and overrides for SelMod
const ActionTableId kActionID = 0x48ce6227;

class SMActionCB : public ActionCallback {
	SelMod *mpMod;
public:
	SMActionCB (SelMod *m) : mpMod(m) { }
	BOOL ExecuteAction(int id);
};

BOOL SMActionCB::ExecuteAction (int id) {
	switch (id) {
		case ID_EDIT_SS:
			mpMod->EditSoftSelectionMode();
			return true;
	}
	return false;
}

const int kNumActions = 1;

static ActionDescription actions[] = {
	ID_EDIT_SS,
	IDS_EDIT_SS,
	IDS_EDIT_SS,
	IDS_RB_VOLSELECT

};

ActionTable* GetSMActions() {
	TSTR name = GetString (IDS_RB_VOLSELECT);
	HACCEL hAccel = LoadAccelerators (hInstance, MAKEINTRESOURCE (IDR_ACCEL));
	ActionTable* pTab = NULL;
	pTab = new ActionTable (kActionID, kActionID,
		name, hAccel, kNumActions, actions, hInstance);
	GetCOREInterface()->GetActionManager()->RegisterActionContext(kActionID, name.data());

	IActionItemOverrideManager *actionOverrideMan = static_cast<IActionItemOverrideManager*>(GetCOREInterface(IACTIONITEMOVERRIDEMANAGER_INTERFACE ));
	if(actionOverrideMan)
	{
		//register the action items
		TSTR desc;
		ActionItem *item = pTab->GetAction(ID_EDIT_SS);
		if(item)
		{
			item->GetDescriptionText(desc);
			actionOverrideMan->RegisterActionItemOverride(item,desc); 
		}
	}

	return pTab;
}

//override for all of the command modes
class SMEditSSOverride : public IActionItemOverride 
{
protected:
	BOOL mActiveOverride;
	SelMod *mpObj;
public:
	SMEditSSOverride(SelMod *m):mActiveOverride(FALSE),mpObj(m){}
	BOOL IsOverrideActive();
	BOOL StartOverride();
	BOOL EndOverride();
};

BOOL SMEditSSOverride::IsOverrideActive()
{
	return mActiveOverride;
}


BOOL SMEditSSOverride::StartOverride()
{
	if(mActiveOverride==TRUE) //already TRUE, exit
		return FALSE;
	if (mpObj == nullptr || mpObj->ip == nullptr)
		return FALSE;
	if(mpObj->ip->GetCommandMode()!=mpObj->editSSMode)
	{
		mpObj->ip->PushCommandMode(mpObj->editSSMode);
	}
	else
		return FALSE;

	mActiveOverride = TRUE;
	return TRUE;
}

BOOL SMEditSSOverride::EndOverride()
{
	if(mActiveOverride==FALSE) //wasn't active had a problem
		return FALSE;
	mActiveOverride = FALSE;
	if (mpObj == nullptr || mpObj->ip == nullptr)
		return FALSE;
	mpObj->ip->DeleteMode(mpObj->editSSMode);

	return TRUE;
}
//--- ClassDescriptor and class vars ---------------------------------

IObjParam*          SelMod::ip          = NULL;
SelMod*				SelMod::curMod      = NULL;
MoveModBoxCMode*    SelMod::moveMode    = NULL;
RotateModBoxCMode*  SelMod::rotMode     = NULL;
UScaleModBoxCMode*  SelMod::uscaleMode  = NULL;
NUScaleModBoxCMode* SelMod::nuscaleMode = NULL;
SquashModBoxCMode*  SelMod::squashMode  = NULL;
EditSSMode*			SelMod::editSSMode	= NULL;

SMActionCB*			SelMod::mpActions	= NULL;
Tab<ActionItem*>	SelMod::mOverrides;

class SelClassDesc:public ClassDesc2 {
public:
	int 			IsPublic() { return 1; }
	void*			Create(BOOL loading = FALSE) { return new SelMod(!loading);}
	const TCHAR*	ClassName() { return GetString(IDS_RB_VOLSELECT_CLASS); }
	const TCHAR*	NonLocalizedClassName() { return _T("Vol. Select"); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(SELECTOSM_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetString(IDS_RB_DEFDEFORMATIONS);}
	// JBW: new descriptor data accessors added.  Note that the 
	//      internal name is hardwired since it must not be localized.
	const TCHAR*	InternalName() { return _T("VolumeSelect"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }			// returns owning module handle
	int             NumActionTables(){return 1;}
	ActionTable *GetActionTable(int i) {return GetSMActions ();}
};

static SelClassDesc selDesc;
extern ClassDesc* GetSelModDesc() {return &selDesc;}

// JBW: Here follows the new parameter block descriptors.  There are now 3, 
//      two new STATIC ones to hold the old class var parameters, one for the main
//		per-instance parameters.  Apart from all the extra 
//      metadata you see in the definitions, one important new idea is the
//      folding of ParamMap description info into the parameter descriptor and
//      providing a semi-automatic rollout desipaly mechanism.
//      

// Parameter Block definitions

// JBW: First come the position and version independent IDs for each
//      of the blocks and the parameters in each block.  These IDs are used
//	    in subsequent Get/SetValue() parameter access, etc. and for version-independent
//      load and save

// vsel_params param IDs
#define PBLOCK_REF	0
#define TM_REF		1
#define POS_REF		2
#define PBLOCK_AFR_REF	3

// JBW: this descriptor defines the main per-instance parameter block.  It is flagged as AUTO_CONSTRUCT which
//      means that the CreateInstance() will automatically create one of these blocks and set it to the reference
//      number given (0 in this case, as seen at the end of the line).

// per instance geosphere block
static ParamBlockDesc2 sel_param_blk ( vsel_params, _T("Parameters"),  0, &selDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_REF, 
									  //rollout
									  IDD_SELECTPARAM, IDS_RB_PARAMETERS, 0, 0, NULL,
									  // params
									  sel_level,  _T("level"), TYPE_INT, 	P_RESET_DEFAULT, 	IDS_PW_LEVEL, 
									  p_default, 		0,	
									  p_range, 		0, 2, 
									  p_ui, 			TYPE_RADIO, 3,IDC_SEL_OBJECT,IDC_SEL_VERTEX,IDC_SEL_FACE,
									  p_end, 
									  sel_method,  _T("method"), TYPE_INT, 	P_RESET_DEFAULT, 	IDS_PW_METHOD, 
									  p_default, 		0,	
									  p_range, 		0, 2, 
									  p_ui, 			TYPE_RADIO, 3,IDC_SEL_REPLACE,IDC_SEL_ADD,IDC_SEL_SUBTRACT,
									  p_end, 
									  sel_type,  _T("type"), TYPE_INT, 	P_RESET_DEFAULT, 	IDS_PW_TYPE, 
									  p_default, 		0,	
									  p_range, 		0, 1, 
									  p_ui, 			TYPE_RADIO, 2,IDC_SEL_WINDOW,IDC_SEL_CROSSING,
									  p_end, 


									  sel_volume,  _T("volume"), TYPE_INT, 	P_RESET_DEFAULT, 	IDS_PW_VOLUME, 
									  p_default, 		0,	
									  p_range, 		0, 7, 
									  p_ui, 			TYPE_RADIO,  8, IDC_SEL_BOXB, IDC_SEL_SPHEREB, IDC_SEL_CYLINDERB, IDC_SEL_MESH_OBJECTB, IDC_SEL_TEXTURE_MAPB,
																		IDC_SEL_MATB, IDC_SEL_SMGROUPB , IDC_SEL_STACK,
									  p_end, 

									  sel_invert, 	_T("invert"),		TYPE_BOOL, 		P_RESET_DEFAULT,				IDS_PW_INVERT,
									  p_default, 		FALSE, 
									  p_ui, 			TYPE_SINGLECHECKBOX, 	IDC_SEL_INVERT, 
									  p_end, 

									  sel_node, 	_T("node"),		TYPE_INODE, 		0,				IDS_PW_NODE,
									  p_ui, 			TYPE_PICKNODEBUTTON, 	IDC_SEL_OBJECT_BUTTON, 
									  p_end, 
									  sel_texture, 	_T("texture"),		TYPE_TEXMAP, 		0,				IDS_PW_TEXMAP,
									  p_ui, 			TYPE_TEXMAPBUTTON, 	IDC_SEL_TEXTURE_BUTTON,
									  p_nonLocalizedName, _T("TextureMap"),
									  p_end,
									  sel_map,  _T("map"), TYPE_INT, 	P_RESET_DEFAULT, 	IDS_PW_MAP, 
									  p_default, 		0,	
									  p_range, 		0, 1, 
									  p_ui, 			TYPE_RADIO,  2,IDC_MAP_CHAN1,IDC_MAP_CHAN2,
									  p_end, 

									  sel_map_channel,  _T("mapChannel"),	TYPE_INT, 	P_RESET_DEFAULT, 	IDS_PW_CHANNEL, 
									  p_default, 		1,	
									  p_range, 		1, 99, 
									  p_ui, 			TYPE_SPINNER, EDITTYPE_INT, IDC_MAP_CHAN,IDC_MAP_CHAN_SPIN,  SPIN_AUTOSCALE,
									  p_end, 

									  sel_matid, _T("matID"), TYPE_INT, P_ANIMATABLE | P_RESET_DEFAULT, IDS_VS_MATID,
									  p_default, 1,
									  p_range, 1, MAX_MATID,
									  p_ui, TYPE_SPINNER, EDITTYPE_INT, IDC_VS_MATID, IDC_VS_MATIDSPIN, 1.0,
									  p_end,

									  sel_smGroup, _T("smGroup"), TYPE_INT, P_ANIMATABLE | P_RESET_DEFAULT, IDS_VS_SMGROUP,
									  p_default, 1,
									  p_range, 1, 32,
									  p_ui, TYPE_SPINNER, EDITTYPE_INT, IDC_VS_SMG, IDC_VS_SMGSPIN, 1.0,
									  p_end,

									  sel_autofit, 	_T("autofit"),		TYPE_BOOL, 		P_RESET_DEFAULT,				IDS_PW_AUTOFIT,
									  p_default, 		TRUE, 
									  p_ui, 			TYPE_SINGLECHECKBOX, 	IDC_SEL_AUTOFIT, 
									  p_end, 


									  p_end
									  );


static ParamBlockDesc2 sel_afr_blk ( vsel_afr, _T("AffectRegion"),  0, &selDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCK_AFR_REF, 

									//rollout
									IDD_SELMOD_AFFECTREGION, IDS_MS_SOFTSEL, 0, 0, NULL,
									// params
									sel_use_ar, 	_T("UseAffectRegion"),		TYPE_BOOL, 		P_RESET_DEFAULT,		IDS_PW_USE_EDGE_LIMIT,
									p_default, 		FALSE, 
									p_ui, 			TYPE_SINGLECHECKBOX, 	IDC_MS_USE_SS, 
									p_enabled,		FALSE,
									p_enable_ctrls,	4, sel_falloff, sel_pinch, sel_bubble,sel_use_edge_limit,
									p_end, 

									sel_falloff,  _T("falloff"),	TYPE_FLOAT, 	P_ANIMATABLE|P_RESET_DEFAULT, 	IDS_PW_FALLOFF2, 
									p_default, 		20.0f,	
									p_range, 		0.0f, 9999999999.0f, 
									p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_FALLOFF,IDC_FALLOFFSPIN, SPIN_AUTOSCALE, 
									p_enabled,		FALSE,
									p_end, 

									sel_pinch,  _T("pinch"),	TYPE_FLOAT, 	P_ANIMATABLE|P_RESET_DEFAULT, 	IDS_PW_PINCH, 
									p_default, 		0.0f,	
									p_range, 		-10.0f, 10.0f, 
									p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_PINCH,IDC_PINCHSPIN, 0.01f, 
									p_enabled,		FALSE,
									p_end, 

									sel_bubble,  _T("bubble"),	TYPE_FLOAT, 	P_ANIMATABLE|P_RESET_DEFAULT, 	IDS_PW_BUBBLE, 
									p_default, 		0.0f,	
									p_range, 		-10.0f, 10.0f, 
									p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_BUBBLE,IDC_BUBBLESPIN, 0.01f, 
									p_enabled,		FALSE,
									p_end, 

									sel_use_edge_limit, 	_T("useEdgeLimit"),		TYPE_INT, 		P_RESET_DEFAULT,		IDS_PW_USE_EDGE_LIMIT,
									p_default, 		FALSE, 
									p_ui, 			TYPE_SINGLECHECKBOX, 	IDC_USE_EDGE_LIMIT, 
									p_enabled,		FALSE,
									p_enable_ctrls,	1, sel_edge_limit,
									p_end, 

									 sel_edge_limit, _T("edgeLimit"), TYPE_INT, P_ANIMATABLE | P_RESET_DEFAULT, IDS_PW_EDGE_LIMIT_VS,
									 p_default, 10,
									 p_range, 1, 999999,
									 p_ui, TYPE_SPINNER, EDITTYPE_INT, IDC_EDGELIMIT, IDC_EDGELIMIT_SPIN, 1.0,
									 p_end,

									p_end
									);

//--- Parameter map/block descriptors -------------------------------


// Levels
#define SEL_OBJECT		0
#define SEL_VERTEX		1
#define SEL_FACE		2

// Volumes
#define SEL_BOX			0
#define SEL_SPHERE		1
#define SEL_CYLINDER	2
#define SEL_MESH_OBJECT	3
#define SEL_TEXTURE		4
#define SEL_MATID 5
#define SEL_SMG 6
#define SEL_USESTACK 7

// Methods
#define SEL_REPLACE		0
#define SEL_ADD			1
#define SEL_SUBTRACT	2

// Types
#define SEL_WINDOW		0
#define SEL_CROSSING	1

// Flags
#define CONTROL_FIT		(1<<0)
#define CONTROL_CENTER	(1<<1)
#define CONTROL_UNIFORM	(1<<3)
#define CONTROL_HOLD	(1<<4)
#define CONTROL_INIT	(1<<5)
#define CONTROL_OP		(CONTROL_FIT|CONTROL_CENTER|CONTROL_UNIFORM|CONTROL_CHANGEFROMBOX)

#define CONTROL_USEBOX	(1<<6) // new for version 2. Means that gizmo space is not 0-1 but instead is based on the dimensions of the mod context box
#define CONTROL_CHANGEFROMBOX	(1<<7) // removes the above problem 


//
//
// Parameters

static int levelIDs[] = {IDC_SEL_OBJECT,IDC_SEL_VERTEX,IDC_SEL_FACE};
static int methodIDs[] = {IDC_SEL_REPLACE,IDC_SEL_ADD,IDC_SEL_SUBTRACT};
static int typeIDs[] = {IDC_SEL_WINDOW,IDC_SEL_CROSSING};

static ParamBlockDescID descVer0[] = {
	{ TYPE_INT, NULL, FALSE, sel_level },
	{ TYPE_INT, NULL, FALSE, sel_method },
	{ TYPE_INT, NULL, FALSE, sel_type },
	{ TYPE_INT, NULL, FALSE, sel_volume },
	{ TYPE_INT, NULL, FALSE, sel_invert }
};
static ParamBlockDescID descVer1[] = {
	{ TYPE_INT, NULL, FALSE, sel_level },
	{ TYPE_INT, NULL, FALSE, sel_method },
	{ TYPE_INT, NULL, FALSE, sel_type },
	{ TYPE_INT, NULL, FALSE, sel_volume },
	{ TYPE_INT, NULL, FALSE, sel_invert },
	{ TYPE_INT, NULL, TRUE, sel_use_ar },
	{ TYPE_FLOAT, NULL, TRUE, sel_falloff },
	{ TYPE_FLOAT, NULL, TRUE, sel_pinch },
	{ TYPE_FLOAT, NULL, TRUE, sel_bubble },
};

#define PBLOCK_LENGTH 9

static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer0,5,0)
};
#define NUM_OLDVERSIONS	1

#define CURRENT_VERSION 1

//--- SelDlgProc -----------------------------------
 
class SelDlgProc : public ParamMap2UserDlgProc {
public:
	SelMod *mod;		
	SelDlgProc(SelMod *m) {mod = m;}		
	INT_PTR DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);		
	void DeleteThis() {delete this;}		
};

INT_PTR SelDlgProc::DlgProc (TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch (msg) {		
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_SEL_FIT:
					mod->flags |= CONTROL_FIT|CONTROL_HOLD|CONTROL_INIT;
					mod->NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
					break;

				case IDC_SEL_CENTER:
					mod->flags |= CONTROL_CENTER|CONTROL_HOLD;
					mod->NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
					break;
				
				case IDC_SEL_RESET:
					theHold.Begin();
					mod->ReplaceReference(TM_REF,NULL);
					mod->flags |= CONTROL_FIT|CONTROL_CENTER|CONTROL_INIT;
					theHold.Accept(GetString(IDS_PW_UNDO_RESET));
					mod->NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
					break;
				case IDC_SEL_OBJECT:
				case IDC_SEL_FACE:
				case IDC_SEL_SMGROUPB:
				case IDC_SEL_MATB:
//watje 5-26-99
				case IDC_SEL_TEXTURE_MAPB:
				{
					mod->DisableAffectRegion(t);
					break;
				}

				case IDC_SEL_VERTEX:
				case IDC_SEL_BOXB:
				case IDC_SEL_SPHEREB:
				case IDC_SEL_CYLINDERB:
				case IDC_SEL_MESH_OBJECTB:
				case IDC_SEL_STACK:
				{
					mod->EnableAffectRegion(t);
					break;
				}
		
			}
			break;
	}
	return FALSE;
}

class AffectRegProc : public ParamMap2UserDlgProc 
{
public:
	SelMod *em;
	HWND hWnd;
	AffectRegProc () { em = NULL; }
	INT_PTR DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
	void DeleteThis() { }
	void Update(TimeValue t)
	{
		Rect rect;
		GetClientRectP(GetDlgItem(hWnd,IDC_AR_GRAPH),&rect);
		InvalidateRect(hWnd,&rect,FALSE);
	};
};

static AffectRegProc theAffectRegProc;

#define GRAPHSTEPS 20

float AffectRegFunctA(float dist,float falloff,float pinch,float bubble) {
	if (falloff<dist) return 0.0f;
	if (falloff == 0.0f)
		return 0.0f;
	float u = ((falloff - dist)/falloff);
	float u2 = u*u, s = 1.0f-u;	
	return (3*u*bubble*s + 3*u2*(1.0f-pinch))*s + u*u2;
}

static void DrawCurve (HWND hWnd,HDC hdc) {
	float pinch, falloff, bubble;
	ISpinnerControl *spin = GetISpinner(GetDlgItem(hWnd,IDC_FALLOFFSPIN));
	falloff = spin->GetFVal();
	ReleaseISpinner(spin);	

	spin = GetISpinner(GetDlgItem(hWnd,IDC_PINCHSPIN));
	pinch = spin->GetFVal();
	ReleaseISpinner(spin);

	spin = GetISpinner(GetDlgItem(hWnd,IDC_BUBBLESPIN));
	bubble = spin->GetFVal();
	ReleaseISpinner(spin);	

	TSTR label = FormatUniverseValue(falloff);
	SetWindowText(GetDlgItem(hWnd,IDC_FARLEFTLABEL),label);
	SetWindowText(GetDlgItem(hWnd,IDC_FARRIGHTLABEL),label);

	Rect rect, orect;
	GetClientRectP(GetDlgItem(hWnd,IDC_AR_GRAPH),&rect);
	orect = rect;

	SelectObject(hdc,GetStockObject(NULL_PEN));
	SelectObject(hdc,GetStockObject(WHITE_BRUSH));
	Rectangle(hdc,rect.left,rect.top,rect.right,rect.bottom);	
	SelectObject(hdc,GetStockObject(NULL_BRUSH));

	rect.left   += 3;
	rect.right  -= 3;
	rect.top    += 20;
	rect.bottom -= 20;

	SelectObject(hdc,CreatePen(PS_DOT,0,GetCustSysColor(COLOR_BTNFACE)));
	MoveToEx(hdc,orect.left,rect.top,NULL);
	LineTo(hdc,orect.right,rect.top);
	MoveToEx(hdc,orect.left,rect.bottom,NULL);
	LineTo(hdc,orect.right,rect.bottom);
	MoveToEx(hdc,(rect.left+rect.right)/2,orect.top,NULL);
	LineTo(hdc,(rect.left+rect.right)/2,orect.bottom);
	DeleteObject(SelectObject(hdc,GetStockObject(BLACK_PEN)));

	MoveToEx(hdc,rect.left,rect.bottom,NULL);
	for (int i=0; i<=GRAPHSTEPS; i++) {
		float dist = falloff * float(abs(i-GRAPHSTEPS/2))/float(GRAPHSTEPS/2);		
		float y = AffectRegFunctA(dist,falloff,pinch,bubble);
		int ix = rect.left + int(float(rect.w()-1) * float(i)/float(GRAPHSTEPS));
		int	iy = rect.bottom - int(y*float(rect.h()-2)) - 1;
		if (iy<orect.top) iy = orect.top;
		if (iy>orect.bottom-1) iy = orect.bottom-1;
		LineTo(hdc, ix, iy);
	}

	WhiteRect3D(hdc,orect,TRUE);
}

INT_PTR AffectRegProc::DlgProc (TimeValue t, IParamMap2 *map,
								HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	if (!em) 
		return FALSE;

	Rect rect;
	TSTR zero;

	switch (msg) {
		case WM_INITDIALOG:
			zero = FormatUniverseValue(0.0f);
			SetWindowText(GetDlgItem(hWnd,IDC_NEARLABEL),zero);
			ShowWindow(GetDlgItem(hWnd,IDC_AR_GRAPH),SW_HIDE);
			this->hWnd = hWnd;
			em->EnableAffectRegion (t);
			break;
			
		case WM_PAINT: 
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd,&ps);
			DrawCurve(hWnd,hdc);
			EndPaint(hWnd,&ps);
			return FALSE;
		}

		case CC_SPINNER_CHANGE:
			GetClientRectP(GetDlgItem(hWnd,IDC_AR_GRAPH),&rect);
			InvalidateRect(hWnd,&rect,FALSE);
			return FALSE;
			break;

		default:
			return FALSE;
	}
	return TRUE;
}


//--- SelMod methods -------------------------------

SelMod::SelMod(BOOL create) 
{	
	badObject = FALSE;
	validator.mod = this;

	if (create) 
		flags = CONTROL_CENTER|CONTROL_FIT|CONTROL_INIT|CONTROL_USEBOX;
	else 
		flags = 0;

	tmControl  = NULL;
	posControl = NULL;
	pblock2 = pblock2_afr = NULL;

	GetSelModDesc()->MakeAutoParamBlocks(this);

	container = NULL;
	notify = new SelNodeNotify(this);
	version = SELMOD_CURRENT_VERSION;
	pobj = NULL;
	epobj = NULL;
}


SelMod::~SelMod()
{
	// mjm - begin - 5.10.99
	if (container && notify)
	{
		ITrackViewNode *tvr = GetCOREInterface()->GetTrackViewRootNode();
		ITrackViewNode *global = tvr->GetNode(GLOBAL_VAR_TVNODE_CLASS_ID);
		ITrackViewNode *tvroot = global->GetNode(MCONTAINER_TVNODE_CLASS_ID);
		if (!tvroot)
			tvroot = tvr;
		if (tvroot) 
		{
			int ct = tvroot->NumItems();
			for (int i = 0; i < ct; i++)
			{
				ITrackViewNode *n = tvroot->GetNode(i);
				if (container == n) {
					container->UnRegisterTVNodeNotify(notify);
					tvroot->RemoveItem(i);
					// [YF] break the loop on found
					break;
				}
			}
		}
	}

	if (notify)
		delete notify;
	// mjm - end
}


bool SelMod::ChangesTopology()
{
	return (pblock2->GetInt(sel_level) == SEL_FACE);
}


#define USEBOX_CHUNK	0x0100
#define BOX_CHUNK		0x0110
#define BACKPATCH_CHUNK		0x0120
#define VERSION_CHUNK		0x0130


class SelModPostLoad : public PostLoadCallback {
public:
	SelMod *n;
	BOOL param2;
	SelModPostLoad(SelMod *ns, BOOL p) {n = ns;param2 = p;}
	void proc(ILoad *iload) {  
		if (n->container != NULL)
		{
			n->container->RegisterTVNodeNotify(n->notify);
			n->container->HideChildren(TRUE);
		}
		if (!param2)
			n->pblock2->SetValue(sel_autofit,0,1);

		delete this; 
	} 
};



IOResult SelMod::Load(ILoad *iload)
{
	IOResult res;	
	Modifier::Load(iload);	
	ULONG nb;

	flags &= ~CONTROL_USEBOX;
	BOOL param2 = FALSE;

	// Default for older files
	version = SELMOD_VER1;

	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case USEBOX_CHUNK:
				flags |= CONTROL_USEBOX;
				flags |= CONTROL_CHANGEFROMBOX;
				flags |= CONTROL_INIT;
				break; 
			case BOX_CHUNK:
				iload->Read(&mcBox,sizeof(mcBox),&nb);
				break; 
			case BACKPATCH_CHUNK:
				ULONG id;
				iload->Read(&id,sizeof(ULONG), &nb);
				if (id!=0xffffffff)
					{
					iload->RecordBackpatch(id,(void**)&container);
					}
				param2 = TRUE;

				break;
			case VERSION_CHUNK:
				res = iload->Read(&version,sizeof(int),&nb);
				break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
	}

	if (param2)
		flags &= ~CONTROL_CHANGEFROMBOX;

	ParamBlock2PLCB* plcb = new ParamBlock2PLCB(versions, NUM_OLDVERSIONS, &sel_param_blk, this, PBLOCK_REF);
	iload->RegisterPostLoadCallback(plcb);

	ParamBlock2PLCB* plcb2 = new ParamBlock2PLCB(versions, NUM_OLDVERSIONS, &sel_afr_blk, this, PBLOCK_AFR_REF);
	iload->RegisterPostLoadCallback(plcb2);
	iload->RegisterPostLoadCallback(new SelModPostLoad(this,param2));

	return IO_OK;
}

IOResult SelMod::Save(ISave *isave)
{
	Modifier::Save(isave);
	ULONG nb;

	isave->BeginChunk(BOX_CHUNK);
	isave->Write(&mcBox,sizeof(mcBox),&nb);

	isave->EndChunk();

	ULONG id = isave->GetRefID(container);

	isave->BeginChunk(BACKPATCH_CHUNK);
	isave->Write(&id,sizeof(ULONG),&nb);
	isave->EndChunk();
	isave->BeginChunk (VERSION_CHUNK);
	isave->Write (&version, sizeof(int), &nb);
	isave->EndChunk();

	if (flags&CONTROL_USEBOX) {
		isave->BeginChunk(USEBOX_CHUNK);
		isave->EndChunk();
	}

	return IO_OK;
}


void SelMod::BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev) {
	this->ip = ip; curMod = this;

	// Create sub object editing modes.
	moveMode    = new MoveModBoxCMode(this,ip);
	rotMode     = new RotateModBoxCMode(this,ip);
	uscaleMode  = new UScaleModBoxCMode(this,ip);
	nuscaleMode = new NUScaleModBoxCMode(this,ip);
	squashMode  = new SquashModBoxCMode(this,ip);	
	editSSMode = new EditSSMode(this,this,ip);

	// Add our accelerator table (keyboard shortcuts)
	mpActions = new SMActionCB (this);
	ip->GetActionManager()->ActivateActionTable (mpActions, kActionID);

	//register all of the overrides with this modifier, first though unregister any 	//still around for wthatever reason,though EndEditParams should handlethis
	IActionItemOverrideManager *actionOverrideMan = static_cast<IActionItemOverrideManager*>(GetCOREInterface(IACTIONITEMOVERRIDEMANAGER_INTERFACE ));

	if(actionOverrideMan)
	{
		for(int i=0;i<mOverrides.Count();++i)
		{
			actionOverrideMan->DeactivateActionItemOverride(mOverrides[i]);		
		}
	}
	mOverrides.ZeroCount();

	ActionTable *pTable = mpActions->GetTable();
	ActionItem *item = pTable->GetAction(ID_EDIT_SS);
	mOverrides.Append(1,&item);
	SMEditSSOverride *commandModeOverride = new SMEditSSOverride(this);
	actionOverrideMan->ActivateActionItemOverride(item,commandModeOverride);

	selDesc.BeginEditParams(ip, this, flags, prev);
	sel_param_blk.SetUserDlgProc(new SelDlgProc(this));
	theAffectRegProc.em = this;
	sel_afr_blk.SetUserDlgProc(&theAffectRegProc);

	sel_param_blk.ParamOption(sel_node,p_validator,&validator);

	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);
}

void SelMod::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
{	
	this->ip = NULL; curMod = NULL;

	TimeValue t = ip->GetTime();
	ClearAFlag(A_MOD_BEING_EDITED);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);

	ip->DeleteMode(moveMode);
	ip->DeleteMode(rotMode);
	ip->DeleteMode(uscaleMode);
	ip->DeleteMode(nuscaleMode);
	ip->DeleteMode(squashMode);	
	delete moveMode; moveMode = NULL;
	delete rotMode; rotMode = NULL;
	delete uscaleMode; uscaleMode = NULL;
	delete nuscaleMode; nuscaleMode = NULL;
	delete squashMode; squashMode = NULL;

	//unregister all of the overrides.  Need to do this before deactiviting the keyboard shortcuts.
	IActionItemOverrideManager *actionOverrideMan = static_cast<IActionItemOverrideManager*>(GetCOREInterface(IACTIONITEMOVERRIDEMANAGER_INTERFACE ));
	if(actionOverrideMan)
	{
		for(int i=0;i<mOverrides.Count();++i)
		{
			actionOverrideMan->DeactivateActionItemOverride(mOverrides[i]);		
		}
	}
	mOverrides.ZeroCount();

	// Deactivate our keyboard shortcuts
	if (mpActions) {
		ip->GetActionManager()->DeactivateActionTable (mpActions, kActionID);
		delete mpActions;
		mpActions = NULL;
	}
	ip->DeleteMode(editSSMode);
	if(editSSMode)
	{
		delete editSSMode;
		editSSMode = NULL;
	}

	selDesc.EndEditParams(ip, this, flags, next);
	theAffectRegProc.em = NULL;
}


// private namespace
namespace
{
	class sMyEnumProc : public DependentEnumProc 
	{
	public :
		virtual int proc(ReferenceMaker *rmaker); 
		INodeTab Nodes;              
	};

	int sMyEnumProc::proc(ReferenceMaker *rmaker) 
	{ 
		if (rmaker->SuperClassID()==BASENODE_CLASS_ID)    
		{
			Nodes.Append(1, (INode **)&rmaker);                 
			return DEP_ENUM_SKIP;
		}
		return DEP_ENUM_CONTINUE;
	}
}

Interval SelMod::LocalValidity(TimeValue t)
{	
	// aszabo|feb.05.02 If we are being edited, return NEVER 
	// to forces a cache to be built after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;
	return GetValidity(t);
}

//aszabo|feb.06.02 - When LocalValidity is called by ModifyObject,
// it returns NEVER and thus the object channels are marked non valid
// As a result, the mod stack enters and infinite evaluation of the modifier
// ModifyObject now calls GetValidity and CORE calls LocalValidity to
// allow for building a cache on the input of this modifier when it's 
// being edited 
Interval SelMod::GetValidity(TimeValue t)
{
	Interval valid = FOREVER;
	if (tmControl) {
		Matrix3 tm(1);
		tmControl->GetValue(t,&tm,valid,CTRL_RELATIVE);		
		if (posControl) posControl->GetValue(t,&tm,valid,CTRL_RELATIVE);
	}
	int localUseAR=FALSE;
	pblock2->GetValue (sel_level, t, level, FOREVER);
	pblock2->GetValue (sel_method, t, method, FOREVER);
	if ((level == SEL_VERTEX) && (method == 0))
	{
		pblock2_afr->GetValue (sel_use_ar, t, useAR, valid);
		localUseAR = useAR;
	}
	if (localUseAR) {
		float f;
		pblock2_afr->GetValue (sel_falloff, t, f, valid);
		pblock2_afr->GetValue (sel_pinch, t, f, valid);
		pblock2_afr->GetValue (sel_bubble, t, f, valid);
		int ei;
		pblock2_afr->GetValue(sel_edge_limit, t, ei, valid);
	}

	pblock2->GetValue(sel_volume,t,vol,FOREVER);

	if (vol == SEL_TEXTURE) {
		Texmap *ttmap = NULL;
		pblock2->GetValue(sel_texture,t,ttmap,FOREVER);
		if (ttmap != NULL)
			valid &= ttmap->Validity(t);
	}
	else if (vol == SEL_MESH_OBJECT) {
		INode *tnode;
		pblock2->GetValue(sel_node,t,tnode,FOREVER);
		if (tnode != NULL) {
			Matrix3 tm = tnode->GetObjectTM(t,&valid);
			ObjectState nos = tnode->EvalWorldState(t);
			if (nos.obj->IsShapeObject()) {
				ShapeObject *pathOb = (ShapeObject*)nos.obj;
				if (!pathOb->NumberOfCurves(t)) {
					pathOb = NULL;
				}

			}

			valid &= nos.obj->ObjectValidity(t);
			valid &= nos.Validity(t);
			sMyEnumProc dep;              
			DoEnumDependents(&dep);
			for (int i = 0; i < dep.Nodes.Count(); i++)
			{
				dep.Nodes[i]->GetObjectTM(t,&valid);
			}
			if (nos.obj->IsParticleSystem() )
				valid.Set(t,t);

		}
	}
	else if (vol == SEL_MATID)
	{
		int matid;
		pblock2->GetValue(sel_matid, t, matid, valid);

	}
	else if (vol == SEL_SMG)
	{
		int smGrp;
		pblock2->GetValue(sel_smGroup, t, smGrp, valid);
		
	}


	if (!(valid == FOREVER))
		valid.Set(t,t);  //<- THIS IS A HACK TO FORCE AN UPDATE SINCE THERE IS NO WAY THAT I KNOW OF TO GET THE BASE OBJECT VALIDIDTY

	return valid;
}

RefTargetHandle SelMod::Clone(RemapDir& remap) 
{
	SelMod* newmod = new SelMod(FALSE);	
	newmod->ReplaceReference(PBLOCK_REF,remap.CloneRef(pblock2));
	newmod->ReplaceReference(PBLOCK_AFR_REF,remap.CloneRef(pblock2_afr));
	newmod->ReplaceReference(TM_REF,remap.CloneRef(tmControl));
	if (posControl)
		newmod->ReplaceReference(POS_REF,remap.CloneRef(posControl));
	newmod->flags = CONTROL_USEBOX;
	newmod->container = NULL;
	//watje bug fix 5/24/00 196569
	newmod->mcBox=mcBox;
	newmod->version = version;

	BaseClone(this, newmod, remap);
	return newmod;
}

static void FixupBox(Box3 &box)
{
	if (box.IsEmpty()) box.MakeCube(Point3(0,0,0),10.0f);
	for (int i=0; i<3; i++) {
		if (fabs(box.pmax[i]-box.pmin[i])<0.001) {
			float cent = (box.pmax[i]-box.pmin[i])/2.0f;
			box.pmax[i] = cent + 0.0005f;
			box.pmin[i] = cent - 0.0005f;
		}
	}
}

bool SelMod::IsSelectionThreadSafe()
{
	// These the non thread safe operations because they either evaluate textures or call IntersectRay
	bool bNotThreadSafe = ((vol == SEL_TEXTURE) ||
			((vol == SEL_MESH_OBJECT) && (msh == nullptr) && (sos.obj->SuperClassID() == GEOMOBJECT_CLASS_ID)));

	return !bNotThreadSafe;
}

void SelMod::InitControl(ModContext &mc, Object *obj, TimeValue t)
{
	Box3 box;
	Matrix3 tm;

	Box3 mcbox = *mc.box;
	FixupBox(mcbox);	

	if (tmControl==NULL) {
		ReplaceReference(TM_REF,NewDefaultMatrix3Controller()); 
		NotifyDependents(FOREVER,0,REFMSG_CONTROLREF_CHANGE);
	}
	if (posControl==NULL) {
		ReplaceReference(POS_REF,NewDefaultPositionController()); 
		NotifyDependents(FOREVER,0,REFMSG_CONTROLREF_CHANGE);
	}

	if (flags&(CONTROL_FIT|CONTROL_CENTER)) {
		Point3 zero(0,0,0);
		posControl->SetValue(t,&zero);
	}

	tm = Inverse(CompMatrix(t,&mc,NULL,FALSE));	
	if (mc.box->IsEmpty()) 
	{
		box.MakeCube(Point3(0,0,0),10.0f);
		box = box * (*(mc.tm)) * tm;
	}
	else box = *(mc.box) * (*(mc.tm)) * tm;

	FixupBox(box);
	box.Scale(1.0000005f);	
	BOOL n3 = theHold.IsSuspended();

	BOOL isSuspended = FALSE;
	if (flags&CONTROL_HOLD) 
	{
		if (theHold.IsSuspended())
		{
			theHold.Resume();
			isSuspended = TRUE;
		}
		theHold.Begin();
	}

	if (flags&CONTROL_INIT) {
		SuspendAnimate();
		AnimateOff();
	}

	if (flags&CONTROL_UNIFORM) {
		Matrix3 tm(1), id(1);
		Interval valid;
		tmControl->GetValue(t,&tm,valid,CTRL_RELATIVE);
		float av = 0.0f;
		Point3 s;
		av += Length(tm.GetRow(0));
		av += Length(tm.GetRow(1));
		av += Length(tm.GetRow(2));
		av /= 3.0f;
		s.x = av/Length(tm.GetRow(0));
		s.y = av/Length(tm.GetRow(1));
		s.z = av/Length(tm.GetRow(2));

		SetXFormPacket pckt(s,TRUE,id,tm);
		tmControl->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);
	}

	if (flags&CONTROL_FIT) {
		Point3 s, w  = box.Width();
		Matrix3 tm(1), id(1);
		Interval valid;
		tmControl->GetValue(t,&tm,valid,CTRL_RELATIVE);

		int axis = -1;
		if (vol==SEL_BOX) {
			s.x = w.x==0.0f ? 1.0f : w.x/2.0f;
			s.y = w.y==0.0f ? 1.0f : w.y/2.0f;
			s.z = w.z==0.0f ? 1.0f : w.z/2.0f;
		} 
		else 
			if (vol==SEL_SPHERE) {
				float max = w.x;
				axis = 0;
				if (w.y>max) 
				{
					max = w.y;
					axis = 1;
				}
				if (w.z>max) 
				{
					max = w.z;
					axis = 2;
				}
				if (max==0.0f) max = 1.0f;
				s.x = s.y = s.z = max/2.0f;
			} 
			else {
				if (w.x>w.y) s.x = s.y = w.x/2.0f;
				else s.x = s.y = w.y/2.0f;
				s.z = w.z/2.0f;
				if (s.x==0.0f) s.x = 1.0f;
				if (s.y==0.0f) s.y = 1.0f;
				if (s.z==0.0f) s.z = 1.0f;
			}

			if (flags&CONTROL_USEBOX) {
				if (vol == SEL_CYLINDER)
				{
					if (w.x>w.y) 
					{
						s.x /= mcbox.Width().x*0.5f;
						s.y /= mcbox.Width().x*0.5f;
					}
					else{
						s.x /= mcbox.Width().y*0.5f;
						s.y /= mcbox.Width().y*0.5f;
					}
					s.z /= mcbox.Width().z*0.5f;

				}
				else if (vol == SEL_SPHERE)
				{
					if (axis == 0)
					{
						s.x /= mcbox.Width().x*0.5f;
						s.y /= mcbox.Width().x*0.5f;
						s.z /= mcbox.Width().x*0.5f;
					}
					else if (axis == 1)
					{
						s.x /= mcbox.Width().y*0.5f;
						s.y /= mcbox.Width().y*0.5f;
						s.z /= mcbox.Width().y*0.5f;
					}
					else 
					{
						s.x /= mcbox.Width().z*0.5f;
						s.y /= mcbox.Width().z*0.5f;
						s.z /= mcbox.Width().z*0.5f;
					}


				}

				else
				{
					s.x /= mcbox.Width().x*0.5f;
					s.y /= mcbox.Width().y*0.5f;
					s.z /= mcbox.Width().z*0.5f;
				}
			}

			SetXFormPacket pckt(s,TRUE,id,tm);
			tmControl->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);

			// redo the box so our center op works
			tm = Inverse(CompMatrix(t,&mc,NULL,FALSE));	
			obj->GetDeformBBox(t,box,&tm,TRUE);
			FixupBox(box);
	}		

	if (flags&(CONTROL_CENTER|CONTROL_FIT)) {
		Matrix3 tm(1);
		Interval valid;
		tmControl->GetValue(t,&tm,valid,CTRL_RELATIVE);		
		if (!(flags&CONTROL_USEBOX)) {
			SetXFormPacket pckt(VectorTransform(tm,box.Center()));
			tmControl->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);
		} 
		else {			
			SetXFormPacket pckt(mcbox.Center()-(mcbox.Center()*tm));
			tmControl->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);
		}
	}

	if (flags&CONTROL_HOLD) 
	{
		if (flags&CONTROL_FIT)
			theHold.Accept(GetString(IDS_PW_UNDO_FIT));	
		else 
			theHold.Accept(GetString(IDS_RB_CENTER));	


		if (isSuspended)
		{
			theHold.Suspend();
		}
	}

	if ( (flags&CONTROL_INIT) ||(flags&CONTROL_CHANGEFROMBOX) ) {
		mcBox = mcbox;
		flags &= ~CONTROL_CHANGEFROMBOX;
		ResumeAnimate();
	}

	// Turn off everything except the use box flag
	flags &= CONTROL_USEBOX;
}


float SelMod::GetPixel(TimeValue t, Point3 pt,float u, float v, float w)
{
	float f = 0.0f;
	// Note: EvalColor isn't thread safe
	if (tmap) {
		shadeContext.scrPos.x = int(u);
		shadeContext.scrPos.y = int(v);
		shadeContext.uvw.x = u;
		shadeContext.uvw.y = v;
		shadeContext.uvw.z = w;
		shadeContext.pt = pt;
		shadeContext.curTime = t;
		AColor c;
		c  = tmap->EvalColor(shadeContext);
		f += c.r * 0.299f;
		f += c.g * 0.587f;
		f += c.b * 0.114f;		
	}
	return f;
}

#define EPSILON 0.000001f


void SelMod::RecurseDepth(float u1, float u2, float &fu,  ShapeObject *s,int Curve,int Piece, int &depth, Point3 fp)
{
	for (int i = 0; i < depth; i++)
	{
		float u = (u1+u2)*.5f;
		float midu = (u2-u1)*.25f;
		float tu1 = u - midu; 
		float tu2 = u + midu;
		Point3 p1, p2;
		p1 = s->InterpPiece3D(rt, Curve, Piece, tu1);
		p2 = s->InterpPiece3D(rt, Curve, Piece, tu2);

		if ( LengthSquared(fp-p1) < LengthSquared(fp-p2) )
		{
			u1 = u1;
			u2 = u;
		}
		else
		{
			u1 = u;
			u2 = u2;
		}

	}
	fu = (u2+u1)*0.5f;
}

void SelMod::PointToPiece(float &tempu,ShapeObject *s,int Curve,int Piece, int depth, Point3 fp)
{
	float tu;
	float su,eu;
	int depth1;

	depth1 = depth;

	su = 0.0f;
	eu = 0.25f;

	float fdist = REALLYBIGFLOAT;
	float fu = 0.0f;

	for (int i = 0; i < 4; i++)
	{
		tu = 0.0f;
		depth = depth1;
		RecurseDepth(su,eu,tu,s,Curve,Piece,depth,fp);
		su += 0.25f;
		eu += 0.25f;
		Point3 dp = s->InterpPiece3D(rt, Curve, Piece, tu);
		float dist = LengthSquared(fp-dp);
		if (dist<fdist)
		{
			fdist = dist;
			fu = tu;
		}
	}

	tempu = fu;
}

void SelMod::RecurseDepthB(float u1, float u2, float &fu,  BezierShape *s,int Curve,int Piece, int &depth, Point3 fp)
{
	for (int i = 0; i < depth; i++)
	{
		float u = (u1+u2)*.5f;
		float midu = (u2-u1)*.25f;
		float tu1 = u - midu; 
		float tu2 = u + midu;
		Point3 p1 = s->splines[Curve]->InterpBezier3D(Piece, tu1);
		Point3 p2 = s->splines[Curve]->InterpBezier3D(Piece, tu2);

		if ( LengthSquared(fp-p1) < LengthSquared(fp-p2) )
		{
			u1 = u1;
			u2 = u;
		}
		else
		{
			u1 = u;
			u2 = u2;
		}

	}
	fu = (u2+u1)*0.5f;
}


void SelMod::PointToPieceB(float &tempu,BezierShape *s,int Curve,int Piece, int depth, Point3 fp)
{
	float tu;
	float su,eu;
	int depth1;

	depth1 = depth;

	su = 0.0f;
	eu = 0.25f;

	float fdist = REALLYBIGFLOAT;
	float fu = 0.0f;

	for (int i = 0; i < 4; i++)
	{
		tu = 0.0f;
		depth = depth1;
		RecurseDepthB(su,eu,tu,s,Curve,Piece,depth,fp);
		su += 0.25f;
		eu += 0.25f;
		Point3 dp = s->splines[Curve]->InterpBezier3D(Piece, tu);
		float dist = LengthSquared(fp-dp);
		if (dist<fdist)
		{
			fdist = dist;
			fu = tu;
		}
	}

	tempu = fu;
}


float SelMod::SplineToPoint(Point3 p1, ShapeObject *s, TimeValue t)
{
	int rec_depth = 5;
	int piece_count = 0;
	float fdist = REALLYBIGFLOAT;

	BezierShape bez;

	if (!s->NumberOfCurves (t)) return 0.0f;
	if (s->CanMakeBezier()) 
	{
		s->MakeBezier (rt, bez);
		if (bez.splineCount < 1) return 0.0f;
	} 
	else 
	{	// Do dumb conversion to bezier.
		PolyShape pshp;
		s->MakePolyShape (rt, pshp);
		if (pshp.numLines < 1) return 0.0f;
		bez = pshp;	// Basic conversion implemented in operator=.
	}

	for (int i = 0; i < bez.SplineCount(); i++)
	{
		for (int j = 0; j < bez.splines[i]->Segments(); j++)
		{
			float u;
			PointToPieceB(u,&bez,i,j,rec_depth,p1);
			Point3 dp = bez.splines[i]->InterpBezier3D(j,u);
			float dist = LengthSquared(p1-dp);
			if (dist<fdist)
			{
				fdist = dist;
			}
		}
	}

	return (float)sqrt(fdist);
}


float SelMod::LineToPointSquared(const Point3& p1, const Point3&  l1, const Point3& l2)
{
	float dist = 0.0f;
	const Point3 VectorA = Normalize(l2-l1);
	Point3 VectorB = p1-l1;
	float Angle = (float) acos(DotProd(VectorA,Normalize(VectorB)));
	if (Angle > (3.14f/2.0f))
	{
		dist = LengthSquared(VectorB); //p1-l1);
	}
	else
	{
		VectorB = p1-l2;
		dist = LengthSquared(VectorB);

		Angle = (float)acos(-DotProd(VectorA, Normalize(VectorB)));
		if (Angle <= (3.14f / 2.0f))
		{
			float sinA = (float)sin(Angle);
			dist *= sinA * sinA;
		}
	}

	return dist;
}


float SelMod::DistSquareToFace(const float distCheck, const Point3& pa, const Point3& p1, const Point3& p2,
		const Point3& p3, const int faceIndex)
{
	//if inside the face,  take distance from plane
	//check if intersects triangle

	const Point3& n = normList[faceIndex];

	// See if the ray intersects the plane (backfaced)
	float rn = LengthSquared(n);

	// Use a point on the plane to find d
	float d = DotProd(p1,n);

	// Find the point on the ray that intersects the plane
	float a = (DotProd(pa, n) - d) / rn;

	// Vector to the plane.
	Point3 hp = a * n;

	// If we are already too far away, don't bother checking further
	float fDist = LengthSquared(hp);
	if (fDist < distCheck)
	{
		// Compute barycentric coords.
		const Point3 bry = msh->BaryCoords(faceIndex, pa - hp);

		// barycentric coordinates must be in the range 0-1
		if (bry.z < 0.0f || bry.z > 1.0f ||bry.x < 0.0f || bry.x > 1.0f || bry.y < 0.0f || bry.y > 1.0f)
		{
			// else take distance from closest edge
			// find 2 closest points and use that edge
			float closest = LineToPointSquared(pa, p1, p2);
			float d = LineToPointSquared(pa, p2, p3);
			if (d < closest)
				closest = d;
			d = LineToPointSquared(pa, p3, p1);
			if (d < closest)
				closest = d;
			return closest;
		}
		else
		{
			return fDist;
		}
	}

	return REALLYBIGFLOAT;
}

float SelMod::PointInVolumeSingleThread(TimeValue t, Point3 pt, float u, float v, float w, const Matrix3& tm, const Box3& box)
{
	DbgAssert((vol == SEL_MESH_OBJECT) || (vol == SEL_TEXTURE));
	
	if (vol == SEL_TEXTURE)
	{
		float f = GetPixel(t, pt, u, v, w);
		if (f > 0.99f)
			f = 1.0f; // this has to be done since MS VC++ optimizer does not evaluate the same for debug and release
		return f;
	}
	else if (vol == SEL_MESH_OBJECT)
	{
		if (targnode != NULL)
		{
			DbgAssert(sos.obj->SuperClassID() == GEOMOBJECT_CLASS_ID);
			DbgAssert(msh == nullptr);
			// is geom object
			{
				pt = pt * otm;
				if (bbox.Contains(pt))
				{
					int ct = 0;

					Point3 dir(1.0f, 0.0f, 0.0f);
					Ray ray;
					ray.p = pt;
					ray.p.x -= 9999.9f;
					ray.dir = dir;
					Point3 norm;
					float l = 0;
					float at = 0.0f;

					while ((sos.obj->IntersectRay(rt, ray, at, norm)) && (l < 9999.9f))
					{
						Point3 tp = ray.dir * (at + 0.1f);
						ray.p = ray.p + tp;
						l += at;
						if (l < 9999.9f)
							ct++;
					}

					if (sos.obj->ClassID() != Class_ID(SPHERE_CLASS_ID, 0))
					{
						ray.p = pt;
						ray.dir = Point3(-1.0f, 0.0f, 0.0f);
						while ((sos.obj->IntersectRay(rt, ray, at, norm)))
						{
							Point3 tp = ray.dir * (at + 0.1f);
							ray.p = ray.p + tp;
							ct++;
						}
					}
					if ((ct % 2) == 1)
						return 1.0f;
				}
				return REALLYBIGFLOAT;
			}
		}
	}

	return 0.0f;
}

float SelMod::PointInVolumeThreadSafe(TimeValue t, Point3 pt, const Matrix3& tm, const Box3& box)
{
	switch (vol)
	{
	case SEL_BOX:
		pt = pt * tm;
		for (int i = 0; i < 3; i++)
		{
			if (fabs(pt[i]) > 1.0f)
				return 0.0f;
		}
		return 1.0f;

	case SEL_SPHERE:
		pt = pt*tm;
		if (LengthSquared(pt) <= 1.0f)
			return 1.0f;
		return 0.0f;

	case SEL_CYLINDER:
		pt = pt*tm;
		if ((pt.x * pt.x + pt.y * pt.y) > 1.0f)
			return 0.0f;
		if (fabs(pt.z) > 1.0f)
			return 0.0f;

		return 1.0f;

	case SEL_MESH_OBJECT:
	{
		if (targnode == nullptr)
			return 0.0f;

		if (sos.obj->SuperClassID()==SHAPE_CLASS_ID)
		{
			//is spline
			pt = pt*otm;
			if (sos.obj->SuperClassID()==SHAPE_CLASS_ID)
			{
				if (useAR)
				{
					rt = t;
					if (bboxFalloff.Contains(pt))
					{
						float d =  SplineToPoint(pt, pathOb, t);
						return d;
					}
				}
				return 999999999999.0f;
			}
		}
		else if (sos.obj->IsParticleSystem() )
		{
			pt = pt * ntm;
			int pindex = -1;
			float pdist;
			grid.ClosestPoint(pt,worldRadius,pindex,pdist);
			if (pindex == -1) 
				pdist = REALLYBIGFLOAT;
			return pdist;
		}

		else if (sos.obj->SuperClassID()==GEOMOBJECT_CLASS_ID)
		//is geom object
		{
			pt = pt*otm;
			if (bbox.Contains(pt))
			{
				int ct = 0;
				DbgAssert(msh != nullptr);

				int boxCount = box3DArray.Count();
				if (boxCount)
				{
					const Point3 dir(-1.0f, 0.0f, 0.0f);
					const float fX = pt.x;
					const float fY = pt.y;
					const float fZ = pt.z;

					Box3DEntry* boxEntryPtr = box3DArray.GetHardArray(fX, boxCount);
					DbgAssert(boxEntryPtr);
					for (int k = 0; k < boxCount; k++)
					{					
						if (boxEntryPtr->StopHardTesting(fX))
							break;
											
						if (boxEntryPtr->HardContains(fY, fZ))
						{
							const int faceIndex = boxEntryPtr->faceIndex;
							const int vert0 = msh->faces[faceIndex].v[0];						
							const Point3& p3Vert = msh->verts[vert0];

							// check if intersects triangle
							const Point3& n = normList[faceIndex];

							// See if the ray intersects the plane (backfaced)
							const float rn = DotProd(dir, n);

							// Use a point on the plane to find d
							//float d = DotProd(plist[2], n);
							const float d = DotProd(p3Vert, n);

							// Find the point on the ray that intersects the plane
							const float a = (d - DotProd(pt, n)) / rn;

							// The point on the ray and in the plane.
							Point3 hp = pt + a * dir;

							if (hp.x < fX)
							{
								if (p3Vert.z == hp.z)
									hp.z += 0.001f;
								else
								{
									const int vert1 = msh->faces[faceIndex].v[1];
									if (msh->verts[vert1].z == hp.z)
										hp.z += 0.001f;
									else
									{
										const int vert2 = msh->faces[faceIndex].v[2];
										if (msh->verts[vert2].z == hp.z)
											hp.z += 0.001f;
									}
								}

								// Compute barycentric coords.
								Point3 bry = msh->BaryCoords(faceIndex, hp);

								// barycentric coordinates must be in the range 0-1
								if (bry.z > 0.0f && bry.z < 1.0f && bry.x > 0.0f && bry.x < 1.0f && bry.y > 0.0f &&	bry.y < 1.0f)
									ct++;
							}
						}
						boxEntryPtr++;
					}
				}
				// We are inside, no further testing needed
				if ((ct % 2) == 1)
					return 1.0f;
			}

			if (useAR && bboxFalloff.Contains(pt))
			{
				const float fX = pt.x;
				const float fY = pt.y;
				const float fZ = pt.z;
				Point3 hp;

				float closest = REALLYBIGFLOAT;
				int boxCount = box3DArray.Count();
				if (boxCount > 0)
				{
					Box3DEntry* boxEntryPtr = box3DArray.GetSoftArray(fX, boxCount);
					DbgAssert(boxEntryPtr);
					for (int k = 0; k < boxCount; k++)
					{
						if (boxEntryPtr->StopSoftTesting(fX))
							break;

						if (boxEntryPtr->SoftContains(fX, fY, fZ))
						{
							const int faceIndex = boxEntryPtr->faceIndex;

							// get distance from that face
							const int a = msh->faces[faceIndex].v[0];
							const int b = msh->faces[faceIndex].v[1];
							const int c = msh->faces[faceIndex].v[2];
							const float d = DistSquareToFace(closest, pt, msh->verts[a], msh->verts[b], msh->verts[c], faceIndex);
							if (d < closest)
							{
								closest = d;
							}
						}

						boxEntryPtr++;
					}
				}
				if (closest != REALLYBIGFLOAT)
					return Sqrt(closest);
			}
			return REALLYBIGFLOAT;
		}

		return 0.0f;
	}
	default:
		DbgAssert(0);
	}

	return 0.0f;
}

// Returns negative amount if point inside volume, 0 on surface, positive outside.
float SelMod::DistFromVolume(const Point3 pt, const Matrix3 &tm, const Matrix3 & ctm)
{
	Point3 p = tm*pt;
	float max = -REALLYBIGFLOAT;
	Point3 diff, scale, center;
	for (int i=0; i<3; i++) 
		scale[i] = Length (ctm.GetRow (i));

	switch (vol) {
	case SEL_BOX:
		for (int i=0; i<3; i++) {
			diff[i] = (float(fabs(p[i])) - 1.0f)*scale[i];
			if (!i || (diff[i]>max)) max = diff[i];
			if (diff[i]<0) diff[i] = 0.0f;
		}
		if (max<0) return max;	// this far inside box.
		return Length(diff);

	case SEL_SPHERE: // Not easy!  Distance to ellipsoid...
		return Length(((p)-Normalize(p))*(scale));	// Is this right though?

	case SEL_CYLINDER:
		if ((p.z < 0.0) || (p.z > 1.0)) 
			return Length(((p)-Normalize(p))*(scale));	// see note above
		else
		{
			p.z = 0;
			return Length(((p)-Normalize(p))*(scale));	// see note above
		}
	}

	return 0.0f;
}


void SelMod::BuildGrid()
{
	if (targnode && (vol == SEL_MESH_OBJECT) && sos.obj && sos.obj->IsParticleSystem() )
	{
		float scale = Length(ntm.GetRow(0));
		if (Length(ntm.GetRow(1)) > scale) scale = Length(ntm.GetRow(1));
		if (Length(ntm.GetRow(2)) > scale) scale = Length(ntm.GetRow(2));
		worldRadius = falloff*scale;
		Tab<Point3> pointList;
		if (pobj)
		{
			int count = pobj->parts.Count();
			float closest=999999999.9f;
			for (int pid = 0; pid < count; pid++)
			{
				TimeValue age  = pobj->ParticleAge(rt,pid);
				TimeValue life = pobj->ParticleLife(rt,pid);
				if (age!=-1)
				{
					Point3 curval = pobj->parts.points[pid];
					pointList.Append(1,&curval,1000);
				}
			}	
			grid.InitializeGrid(50);
			if (pointList.Count()> 0)
				grid.LoadPoints(pointList.Addr(0),pointList.Count());
		}
		else if (epobj)
		{
			int count = epobj->NumParticles();
			float closest=999999999.9f;
			for (int pid = 0; pid < count; pid++)
			{
				TimeValue age  = epobj->GetParticleAgeByIndex(pid);
				if (age!=-1)
				{
					Point3 *curval = epobj->GetParticlePositionByIndex(pid);

					if (curval)
					{
						pointList.Append(1,curval,1000);

					}
				}
			}	
			grid.InitializeGrid(50);
			if (pointList.Count()> 0)
				grid.LoadPoints(pointList.Addr(0),pointList.Count());					
		}

	}
}

bool SelMod::BuildSelectionArray(Mesh& mesh, const Matrix3& tm, const Box3& box, TimeValue t)
{
	fTable.SetCount(mesh.getNumVerts());

	// Early rejection
	if ((vol == SEL_MESH_OBJECT) && !box.Intersects(bboxTargetLocal))
	{
		for (auto i = 0; i < fTable.Count(); i++)
			fTable[i] = BIGFLOAT;

		return false;
	}

	BuildGrid();

	if (IsSelectionThreadSafe())
	{
		DbgAssert(vol != SEL_TEXTURE);
#ifdef NDEBUG
		tbb::parallel_for(tbb::blocked_range<int>(0, mesh.getNumVerts()), [&](const auto& range) {
			for (int i = range.begin(); i < range.end(); i++)
				fTable[i] = PointInVolumeThreadSafe(t, mesh.verts[i], tm, box);
		});
#else
		for (int i = 0; i < mesh.getNumVerts(); i++)
			fTable[i] = PointInVolumeThreadSafe(t, mesh.verts[i], tm, box);
#endif
	}
	else
	{
		Point3* uvwPtr = nullptr;
		TVFace* tvFace = nullptr;
		const int uvwListCount = uvwList.Count();
		if ((vol == SEL_TEXTURE) && (uvwListCount > 0))
		{
			uvwPtr = uvwList.Addr(0);
			int currentChannel = map ? 0 : channel;
			tvFace = mesh.mapFaces(currentChannel);
		}
		Point3 tv;

		for (int i = 0; i < mesh.getNumVerts(); i++)
		{
			if (tvFace != nullptr)
			{
				if (i < uvwListCount)
					tv = uvwPtr[i];
				else
					tv = Point3(0.5f, 0.5f, 0.0f);
			}
			fTable[i] = PointInVolumeSingleThread(t, mesh.verts[i], tv.x, tv.y, tv.z, tm, box);
		}
	}
	FreeGrid();
	return true;
}

void SelMod::SelectVertices(Mesh& mesh, Matrix3& tm, Matrix3& ctm, Box3& box, TimeValue t)
{
	if ((method == SEL_REPLACE) && (vol != SEL_USESTACK))
	{
		mesh.VertSel().ClearAll();
		mesh.ClearVSelectionWeights();
	}

	if ((vol == SEL_MATID) || (vol == SEL_SMG))
	{
		BitArray andVerts(mesh.numVerts);
		andVerts.SetAll();
		BitArray orVerts(mesh.numVerts);
		DWORD realSmG = (1 << (smG - 1));
		for (int i = 0; i < mesh.numFaces; i++)
		{
			Face& fac = mesh.faces[i];
			BOOL hot = (vol == SEL_MATID) ? (fac.getMatID() == matID) : (fac.smGroup & realSmG);
			if (hot)
			{
				for (int j = 0; j < 3; j++)
					orVerts.Set(fac.v[j]);
			}
			else
			{
				for (int j = 0; j < 3; j++)
					andVerts.Clear(fac.v[j]);
			}
		}
		andVerts &= orVerts; // So that verts with no faces won't be in andVerts.
		// If in crossing mode, select verts that touch this material.  Otherwise, select only verts
		// surrounded by this material.
		BitArray* selVerts = (selType == SEL_CROSSING) ? &orVerts : &andVerts;

		float* vsw = mesh.getVSelectionWeights(); // NULL if none.
		switch (method)
		{
		case SEL_REPLACE:
		case SEL_ADD:
			mesh.VertSel() |= *selVerts;
			if (vsw)
			{
				for (int i = 0; i < mesh.numVerts; i++)
					if ((*selVerts)[i])
						vsw[i] = 1.0f;
			}
			break;
		case SEL_SUBTRACT:
			mesh.VertSel() &= ~(*selVerts);
			if (vsw)
			{
				for (int i = 0; i < mesh.numVerts; i++)
					if ((*selVerts)[i])
						vsw[i] = 0.0f;
			}
			break;
		}
		if (invert)
		{
			mesh.VertSel() = ~mesh.VertSel();
			if (vsw)
			{
				for (int i = 0; i < mesh.numVerts; i++)
					vsw[i] = 1.0f - vsw[i];
			}
		}
		return;
	}

	if ((useAR) || (vol == SEL_TEXTURE))
		mesh.SupportVSelectionWeights();
	float* vsw = mesh.getVSelectionWeights(); // NULL if none.
	BitArray& vertSel = mesh.VertSel();
	// bit selection may not be in weights if source not using soft sel
	//so add them to vsw
	if (vsw)
	{
		for (int i = 0; i < mesh.numVerts; ++i)
		{
			if (vertSel[i])
			{
				vsw[i] = 1.0f;
			}
		}
	}

	if ((vol == SEL_USESTACK) && (useAR))
	{
		MeshTempData* temp = new MeshTempData;
		temp->SetMesh (&mesh);
		temp->InvalidateAffectRegion ();	// have to do, or it might remember last time's falloff, etc.

		temp->InvalidateDistances ();

		BOOL useEdgeLimit = pblock2_afr->GetInt(sel_use_edge_limit);
		int edist = pblock2_afr->GetInt(sel_edge_limit); 
		if (!useEdgeLimit)
			edist = 999999;

		Tab<float> *vwTable = temp->VSWeight (useEdgeLimit, edist, FALSE,
			falloff, pinch, bubble);
		int nv = mesh.numVerts;
		int min = (nv<vwTable->Count()) ? nv : vwTable->Count();
		if (min) memcpy (vsw, vwTable->Addr(0), min*sizeof(float));

		// zero out any accidental leftovers:
		for (int i=min; i<nv; i++) vsw[i] = 0.0f;
		delete temp;
	}
	else if ((vol == SEL_USESTACK) && (!useAR))
	{
		int nv = mesh.numVerts;
		//no soft sel so zero out an less than 1 values
		if (vsw)
		{
			for (int i = 0; i < nv; i++)
			{
				if (vsw[i] < 1.0f)
				{
					vsw[i] = 0.0f;
				}
			}
		}
	}
	else
	{
	const bool bUseTable = BuildSelectionArray(mesh, tm, box, t);

	float f = 0.0f;
	for (int i = 0; i < mesh.getNumVerts(); i++)
	{
		f = 0.0f;
		if (bUseTable)
			f = fTable[i];

		if (f == 1.0f)
		{
			if (method == SEL_SUBTRACT)
			{
				vertSel.Clear(i);
				if (vsw)
					vsw[i] = 0.0f;
			}
			else
			{
				vertSel.Set(i);
				if (vsw)
					vsw[i] = 1.0f;
			}
		}
		else
		{
			if (vsw)
			{
				if (bUseTable && (vol != SEL_TEXTURE))
				{
					if (vol != SEL_MESH_OBJECT)
						f = AffectRegFunctA(DistFromVolume(mesh.verts[i], tm, ctm), falloff, pinch, bubble);
					else
						f = AffectRegFunctA(f, falloff, pinch, bubble);
				}
				else if (bUseTable)
				{
					if (vol == SEL_MESH_OBJECT) // f contains the distance from surface
					{
						f = AffectRegFunctA(f, 0, 0, 0);
					}
				}

				switch (method)
				{
				case SEL_SUBTRACT:
					vsw[i] = vsw[i] - f;
					if (vsw[i] < 0)
						vsw[i] = 0.0f;
					break;
				case SEL_ADD:
					vsw[i] += f;
					if (vsw[i] >= 1)
						vsw[i] = 1.0f;
					break;
				default:
					vsw[i] = f;
					break;
				}

				vertSel.Set(i, vsw[i] == 1.0f);
			}
		}

		if (invert)
		{
			if (vsw)
			{
				vsw[i] = 1.0f - vsw[i];
				vertSel.Set(i, vsw[i] == 1.0f);
			}
			else
				vertSel.Set(i, !vertSel[i]);
		}
	}
	}
}

bool SelMod::BuildSelectionArray(MNMesh& mesh, const Matrix3& tm, const Box3& box, TimeValue t)
{
	fTable.SetCount(mesh.numv);

	// Early rejection
	if ((vol == SEL_MESH_OBJECT) && !box.Intersects(bboxTargetLocal))
	{
		for (auto i = 0; i < fTable.Count(); i++)
			fTable[i] = BIGFLOAT;

		return false;
	}

	BuildGrid();

	if (IsSelectionThreadSafe())
	{
		DbgAssert(vol != SEL_TEXTURE);
#ifdef NDEBUG
		tbb::parallel_for(tbb::blocked_range<int>(0, mesh.numv), [&](const auto& range) {
			for (int i = range.begin(); i < range.end(); i++)
				fTable[i] = PointInVolumeThreadSafe(t, mesh.v[i].p, tm, box);
		});
#else
		for (int i = 0; i < mesh.numv; i++)
				fTable[i] = PointInVolumeThreadSafe(t, mesh.v[i].p, tm, box);
#endif
	}
	else
	{
		const int uvwListCount = uvwList.Count();
		int currentChannel = map ? 0 : channel;
		Point3* uvwPtr = nullptr;
		bool texmap = false;
		if ((vol == SEL_TEXTURE) && mesh.M(currentChannel))
		{
			texmap = !mesh.M(currentChannel)->GetFlag(MN_DEAD);
			if (texmap)
			{
				if (uvwListCount > 0)
					uvwPtr = uvwList.Addr(0);
				else
					texmap = false;
			}
		}

		Point3 tv;

		for (int i = 0; i < mesh.numv; i++)
		{
			if (texmap)
			{
				if (i < uvwListCount)
					tv = uvwPtr[i];
				else
					tv = Point3(0.5f, 0.5f, 0.0f);
			}
			fTable[i] = PointInVolumeSingleThread(t, mesh.v[i].p, tv.x, tv.y, tv.z, tm, box);
		}
	}

	FreeGrid();
	return true;
}

void SelMod::SelectVertices (MNMesh &mesh, Matrix3 &tm, Matrix3 & ctm, Box3 &box, TimeValue t)
{
	if ((method==SEL_REPLACE) && (vol != SEL_USESTACK)){
		mesh.ClearVFlags (MN_SEL);
		mesh.freeVSelectionWeights ();
	}

	if ((vol == SEL_MATID) || (vol == SEL_SMG)) {
		BitArray andVerts(mesh.numv);
		andVerts.SetAll();
		BitArray orVerts(mesh.numv);

		DWORD realSmG = (1<<(smG-1));
		for (int i=0; i<mesh.numf; i++) {
			MNFace & fac = mesh.f[i];
			BOOL hot = (vol == SEL_MATID) ? (fac.material == matID) : (fac.smGroup & realSmG);
			if (hot) {
				for (int j=0; j<fac.deg; j++) orVerts.Set (fac.vtx[j]);
			} else {
				for (int j=0; j<fac.deg; j++) andVerts.Clear (fac.vtx[j]);
			}
		}
	 	andVerts &= orVerts;	// So that verts with no faces won't be in andVerts.
		// If in crossing mode, select verts that touch this material.  Otherwise, select only verts
		// surrounded by this material.
		BitArray *selVerts = (selType==SEL_CROSSING) ? &orVerts : &andVerts;

		float *vsw = mesh.getVSelectionWeights ();	// NULL if none.
		switch (method) {
		case SEL_REPLACE:
		case SEL_ADD:
			for (int i=0; i<mesh.numv; i++) {
				if (!(*selVerts)[i]) continue;
				mesh.v[i].SetFlag (MN_SEL);
				if (vsw) vsw[i] = 1.0f;
			}
			break;
		case SEL_SUBTRACT:
			for (int i=0; i<mesh.numv; i++) {
				if (!(*selVerts)[i]) continue;
				mesh.v[i].ClearFlag (MN_SEL);
				if (vsw) vsw[i] = 0.0f;
			}
			break;
		}
		if (invert) {
			for (int i=0; i<mesh.numv; i++) {
				mesh.v[i].SetFlag (MN_SEL, !mesh.v[i].GetFlag (MN_SEL));
				if (vsw) vsw[i] = 1.0f - vsw[i];
			}
		}
		return;
	}

	if ((useAR) || (vol == SEL_TEXTURE))
		mesh.SupportVSelectionWeights();
	float* vsw = mesh.getVSelectionWeights(); // NULL if none.
	// bit selection may not be in weights if source not using soft sel
	// so add them to vsw
	if (vsw)
	{
		for (int i = 0; i < mesh.numv; ++i)
		{
			if (mesh.v[i].GetFlag(MN_SEL))
			{
				vsw[i] = 1.0f;
			}
		}
	}


	if ((vol == SEL_USESTACK) &&  (useAR))
	{
		MNTempData* temp = new MNTempData;
		temp->SetMesh(&mesh);

		 // soft selection is NOT locked
		temp->InvalidateSoftSelection(); // have to do, or it might remember last time's falloff, etc.

		temp->InvalidateDistances();
		
		BOOL useEdgeLimit = pblock2_afr->GetInt(sel_use_edge_limit);
		int edist = pblock2_afr->GetInt(sel_edge_limit); 
		if (!useEdgeLimit)
			edist = 999999;
		Tab<float>* vwTable = temp->VSWeight(useEdgeLimit, edist, FALSE, falloff, pinch, bubble, MN_SEL);

		if (vwTable == NULL)
			return;

		int min = (mesh.numv < vwTable->Count()) ? mesh.numv : vwTable->Count();
		if (min)
			memcpy(vsw, vwTable->Addr(0), min * sizeof(float));
		// zero out any accidental leftovers:
		for (int i = min; i < mesh.numv; i++)
			vsw[i] = 0.0f;
		
	}
	else if ((vol == SEL_USESTACK) && (!useAR))
	{
		int nv = mesh.numv;
		//no soft sel so zero out an less than 1 values
		if (vsw)
		{
			for (int i = 0; i < nv; i++)
			{
				if (vsw[i] < 1.0f)
				{
					vsw[i] = 0.0f;
				}
			}
		}
	}
	else
	{
	const bool bUseTable = BuildSelectionArray(mesh, tm, box, t);

	float f = 0.0f;
	for (int i = 0; i < mesh.numv; i++)
	{
		if (bUseTable)
			f = fTable[i];

		if (f == 1.0f)
		{
			if (method == SEL_SUBTRACT)
			{
				mesh.v[i].ClearFlag(MN_SEL);
				if (vsw)
					vsw[i] = 0.0f;
			}
			else
			{
				mesh.v[i].SetFlag(MN_SEL);
				if (vsw)
					vsw[i] = 1.0f;
			}
		}
		else
		{
			if (vsw)
			{
				if ((bUseTable) && (vol != SEL_TEXTURE))
				{
					if (useAR)
					{
						if (vol != SEL_MESH_OBJECT)
							f = AffectRegFunctA(DistFromVolume(mesh.v[i].p, tm, ctm), falloff, pinch, bubble);
						else
							f = AffectRegFunctA(f, falloff, pinch, bubble);
					}
					else
					{
						if (vol == SEL_MESH_OBJECT) // f contains the distance from surface
						{
							f = AffectRegFunctA(f, 0, 0, 0);
						}
					}
				}

				switch (method)
				{
				case SEL_SUBTRACT:
					vsw[i] = vsw[i] - f;
					if (vsw[i] < 0)
						vsw[i] = 0.0f;
					break;
				case SEL_ADD:
					vsw[i] += f;
					if (vsw[i] >= 1)
						vsw[i] = 1.0f;
					break;
				default:
					vsw[i] = f;
					break;
				}
				mesh.v[i].SetFlag(MN_SEL, vsw[i] == 1.0f);
			}
		}

		if (invert)
		{
			if (vsw)
			{
				vsw[i] = 1.0f - vsw[i];
				mesh.v[i].SetFlag(MN_SEL, vsw[i] == 1.0f);
			}
			else
				mesh.v[i].SetFlag(MN_SEL, !mesh.v[i].GetFlag(MN_SEL));
		}
	}
	}
}

bool SelMod::BuildSelectionArray(PatchMesh& pmesh, const Matrix3& tm, const Box3& box, TimeValue t)
{
	fTable.SetCount(pmesh.getNumVerts());

	// Early rejection
	if ((vol == SEL_MESH_OBJECT) && !box.Intersects(bboxTargetLocal))
	{
		for (auto i = 0; i < fTable.Count(); i++)
			fTable[i] = BIGFLOAT;

		return false;
	}

	BuildGrid();

	if (IsSelectionThreadSafe())
	{
		DbgAssert(vol != SEL_TEXTURE);
#ifdef NDEBUG
		tbb::parallel_for(tbb::blocked_range<int>(0, pmesh.getNumVerts()), [&](const auto& range) {
			for (int i = range.begin(); i < range.end(); i++)
				fTable[i] = PointInVolumeThreadSafe(t, pmesh.verts[i].p, tm, box);
		});
#else
		for (int i = 0; i < pmesh.getNumVerts(); i++)
			fTable[i] = PointInVolumeThreadSafe(t, pmesh.verts[i].p, tm, box);
#endif
	}
	else
	{
		TVPatch* tvPatch = nullptr;
		const int uvwListCount = uvwList.Count();
		if (vol == SEL_TEXTURE)
		{
			int currentChannel = map ? 0 : channel;
			tvPatch = pmesh.mapPatches(currentChannel);
		}

		Point3 tv;
		for (int i = 0; i < pmesh.getNumVerts(); i++)
		{
			if (tvPatch != nullptr)
			{
				if (i < uvwListCount)
					tv = uvwList[i];
				else
					tv = Point3(0.5f, 0.5f, 0.0f);
			}
			fTable[i] = PointInVolumeSingleThread(t, pmesh.verts[i].p, tv.x, tv.y, tv.z, tm, box);
		}
	}
	FreeGrid();
	
	return true;
}

void SelMod::SelectVertices (PatchMesh &pmesh, Matrix3 &tm, Matrix3 & ctm, Box3 &box, TimeValue t)
{
	if ((method == SEL_REPLACE) && (vol != SEL_USESTACK))
	{
		pmesh.vertSel.ClearAll();
		pmesh.InvalidateVertexWeights();
	}

	if ((vol == SEL_MATID) || (vol == SEL_SMG)) {
		BitArray andVerts;
		BitArray orVerts;
		andVerts.SetSize (pmesh.numVerts);
		andVerts.SetAll ();
		orVerts.SetSize (pmesh.numVerts);
		orVerts.ClearAll ();
		DWORD realSmG = (1<<(smG-1));
		for (int i=0; i<pmesh.numPatches; i++) {
			Patch &p = pmesh.patches[i];
			BOOL hot = (vol == SEL_MATID) ? (p.getMatID() == matID) : (p.smGroup & realSmG);
			if (hot) {
				for (int j=0; j<p.type; j++) orVerts.Set (p.v[j]);
			} else {
				for (int j=0; j<p.type; j++) andVerts.Clear (p.v[j]);
			}
		}
		andVerts &= orVerts;	// So that verts with no faces won't be in andVerts.
		// If in crossing mode, select verts that touch this material.  Otherwise, select only verts
		// surrounded by this material.
		BitArray *selVerts = (selType==SEL_CROSSING) ? &orVerts : &andVerts;

		float *vsw = pmesh.GetVSelectionWeights ();	// NULL if none.
		switch (method) {
		case SEL_REPLACE:
		case SEL_ADD:
			pmesh.vertSel |= *selVerts;
			if (vsw) {
				for (int i=0; i<pmesh.numVerts; i++)
					if ((*selVerts)[i])
						vsw[i] = 1.0f;
			}
			break;
		case SEL_SUBTRACT:
			pmesh.vertSel &= ~(*selVerts);
			if (vsw) {
				for (int i=0; i<pmesh.numVerts; i++)
					if ((*selVerts)[i])
						vsw[i] = 0.0f;
			}
			break;
		}
		if (invert) {
			pmesh.vertSel = ~pmesh.vertSel;
			if (vsw) {
				for (int i=0; i<pmesh.numVerts; i++)
					vsw[i] = 1.0f-vsw[i];
			}
		}
		return;
	}

	const bool bUseTable = BuildSelectionArray(pmesh, tm, box, t);

	if ((useAR) || (vol == SEL_TEXTURE))
		pmesh.SupportVSelectionWeights();
	float* vsw = pmesh.GetVSelectionWeights();

	if ((vol == SEL_USESTACK) &&  (useAR))
	{
		bool calcVertexWeights = false;


		if ( falloff != pmesh.Falloff() ) {
			pmesh.SetFalloff( falloff );
			calcVertexWeights = true;	
		}

		if ( bubble != pmesh.Bubble() ) {
			pmesh.SetBubble( bubble );
			calcVertexWeights = true;	
		}

		if ( pinch != pmesh.Pinch() ) {
			pmesh.SetPinch( pinch );
			calcVertexWeights = true;	
		}
		
		if ( pblock2_afr->GetInt(sel_use_edge_limit) != pmesh.UseEdgeDists() ) {
			pmesh.SetUseEdgeDists( pblock2_afr->GetInt(sel_use_edge_limit) );
			calcVertexWeights = true;	
		}

		if ( edgeLimit != pmesh.EdgeDist() ) {
			pmesh.SetEdgeDist( edgeLimit );
			calcVertexWeights = true;	
		}


		if ( useAR != pmesh.UseSoftSelections() ) {
			pmesh.SetUseSoftSelections( useAR );
			// calcVertexWeights = true; /* not necessary since SetUseSoftSelections(..) will calc the vertex weights for us */	
		}

		if ( calcVertexWeights ) {
			pmesh.UpdateVertexDists();
			pmesh.UpdateEdgeDists();
			pmesh.UpdateVertexWeights();
		}		
	}
	else
	{
	float f = 0.0f;

		for (int i = 0; i < pmesh.getNumVerts(); i++)
	{
		if (bUseTable)
			f = fTable[i];

			if (f == 1.0f)
			{
				if (method == SEL_SUBTRACT)
				{
				pmesh.vertSel.Clear(i);
				if (vsw)
					vsw[i] = 0.0f;
				}
				else
				{
				pmesh.vertSel.Set(i);
				if (vsw)
					vsw[i] = 1.0f;
			}
			}
			else
			{
				if (vsw)
				{
				if (bUseTable && (vol != SEL_TEXTURE))
				{
					if (vol != SEL_MESH_OBJECT)
							f = AffectRegFunctA(DistFromVolume(pmesh.verts[i].p, tm, ctm), falloff, pinch, bubble);
					else
						f = AffectRegFunctA(f, falloff, pinch, bubble);
				}

					switch (method)
					{
				case SEL_SUBTRACT:
					vsw[i] = vsw[i] - f;
					if (vsw[i] < 0)
						vsw[i] = 0.0f;
					break;
				case SEL_ADD:
					vsw[i] += f;
					if (vsw[i] >= 1)
						vsw[i] = 1.0f;
					break;
				default:
					vsw[i] = f;
					break;
				}

					pmesh.vertSel.Set(i, vsw[i] == 1.0f);
			}
		}

			if (invert)
			{
				if (vsw)
				{
					vsw[i] = 1.0f - vsw[i];
				pmesh.vertSel.Set(i, vsw[i] == 1.0f);
				}
				else
					pmesh.vertSel.Set(i, !pmesh.vertSel[i]);
			}
		}
	}
}

void SelMod::SelectFaces (TimeValue t, Mesh &mesh, Matrix3 &tm, Box3 &box)
{
	const int numFaces = mesh.getNumFaces();
	if (numFaces <= 0)
		return;

	if ((method == SEL_REPLACE) && (vol != SEL_USESTACK))
		mesh.FaceSel().ClearAll();

	if ((vol == SEL_MATID) || (vol == SEL_SMG)) {
		DWORD realSmG = (1<<(smG-1));
		BitArray& faceSel = mesh.FaceSel();
		for (int i = 0; i < numFaces; i++)
		{
			if (vol==SEL_MATID){
				if (mesh.faces[i].getMatID () != matID) continue;
			} else {
				if (mesh.faces[i].getSmGroup () != realSmG) continue;
			}
			if (method == SEL_SUBTRACT)
				faceSel.Clear (i);
			else
				faceSel.Set (i);
		}
		if (invert)
			mesh.FaceSel() = ~mesh.FaceSel();

		return;
	}

	const bool bUseTable = BuildSelectionArray(mesh, tm, box, t);
	if (bUseTable && fTable.Count())
	{
		BitArray& faceSel = mesh.FaceSel();
		float* f = fTable.Addr(0);
		for (int i = 0; i < numFaces; i++)
		{
			int in = 0;
			for (int k = 0; k < 3; k++)
			{
				int thisVert = mesh.faces[i].v[k];
				if (f[thisVert] == 1.0f)
				{
					in++;
					if (selType == SEL_CROSSING)
						break;
				}
				else
				{
					if (selType == SEL_WINDOW)
						goto nextFace;
				}
			}

			if (in)
			{
				if (method == SEL_SUBTRACT)
				{
					faceSel.Clear(i);
				}
				else
				{
					faceSel.Set(i);
				}
			}

		nextFace:;

			if (invert)
				faceSel.Set(i, !faceSel[i]);
		}
	} else
	{
		// Nothing Selected, so only invert selection if requested
		if (invert)
		{
			mesh.FaceSel() = ~mesh.FaceSel();
		}
	}
}

void SelMod::SelectFaces (TimeValue t, MNMesh &mesh, Matrix3 &tm, Box3 &box)
{
	if ((method == SEL_REPLACE) && (vol != SEL_USESTACK))
		mesh.ClearFFlags (MN_SEL);

	if ((vol == SEL_MATID) || (vol == SEL_SMG)) {
		DWORD realSmG = (1<<(smG-1));
		for (int i=0; i<mesh.numf; i++) {
			if (vol==SEL_MATID) {
				if (mesh.f[i].material != matID) continue;
			} else {
				if (mesh.f[i].smGroup != realSmG) continue;
			}
			if (method == SEL_SUBTRACT) mesh.f[i].ClearFlag (MN_SEL);
			else mesh.f[i].SetFlag (MN_SEL);
		}
		if (invert) {
			for (int i=0; i<mesh.numf; i++) mesh.f[i].SetFlag (MN_SEL, !mesh.f[i].GetFlag (MN_SEL));
		}
		return;
	}

	const bool bUseTable = BuildSelectionArray(mesh, tm, box, t);

	if (bUseTable)
	{

		for (int i = 0; i < mesh.numf; i++)
		{
			int in = 0;
			bool outside = false;
			if (mesh.f[i].GetFlag(MN_DEAD))
				continue;

			for (int k = 0; k < mesh.f[i].deg; k++)
			{

				int vertIndex = mesh.f[i].vtx[k];
				if (fTable[vertIndex] == 1.0f)
				{
					in++;
					if (selType == SEL_CROSSING)
						break;
				}
				else
				{
					if (selType == SEL_WINDOW)
					{
						outside = true;
						break;
					}
				}
			}
			if (in && !outside)
			{
				if (method == SEL_SUBTRACT)
					mesh.f[i].ClearFlag(MN_SEL);
				else
					mesh.f[i].SetFlag(MN_SEL);
			}
			if (invert)
				mesh.f[i].SetFlag(MN_SEL, !mesh.f[i].GetFlag(MN_SEL));
		}
	}
	else
	{
		// We completely missed, so just invert if required
		if (invert)
		{
			for (int i = 0; i < mesh.numf; i++)
			{
				if (!mesh.f[i].GetFlag(MN_DEAD))
					mesh.f[i].SetFlag(MN_SEL, !mesh.f[i].GetFlag(MN_SEL));
			}
		}
	}
}

void SelMod::SelectPatches (TimeValue t, PatchMesh &pmesh, Matrix3 &tm, Box3 &box) {
	if ((method == SEL_REPLACE) && (vol != SEL_USESTACK))
		pmesh.patchSel.ClearAll();

	if ((vol == SEL_MATID) || (vol == SEL_SMG)) {
		DWORD realSmG = (1<<(smG-1));
		for (int i=0; i<pmesh.getNumPatches(); i++) {
			if (vol==SEL_MATID){
				if (pmesh.patches[i].getMatID () != matID) continue;
			} else {
				if (pmesh.patches[i].smGroup != realSmG) continue;
			}
			if (method == SEL_SUBTRACT)
				pmesh.patchSel.Clear (i);
			else
				pmesh.patchSel.Set (i);
		}
		if (invert)
			pmesh.patchSel = ~pmesh.patchSel;

		return;
	}

	const bool bUseTable = BuildSelectionArray(pmesh, tm, box, t);
	if (bUseTable)
	{
		for (int i = 0; i < pmesh.getNumPatches(); i++)
		{
			int in = 0;
			Patch& p = pmesh.patches[i];
			for (int k = 0; k < p.type; k++)
			{
				int index = pmesh.patches[i].v[k];
				if (fTable[index] == 1.0f)
				{
					in++;
					if (selType == SEL_CROSSING)
						break;
				}
				else
				{
					if (selType == SEL_WINDOW)
						goto nextFace;
				}
			}

			if (in)
			{
				if (method == SEL_SUBTRACT)
				{
					pmesh.patchSel.Clear(i);
				}
				else
				{
					pmesh.patchSel.Set(i);
				}
			}

		nextFace:;

			if (invert)
				pmesh.patchSel.Set(i, !pmesh.patchSel[i]);
		}
		
	} else
	{
		// Nothing selected, just invert if requested
		if (invert)
		{
			for (int i = 0; i < pmesh.getNumPatches(); i++)
				pmesh.patchSel.Set(i, !pmesh.patchSel[i]);
		}
	}
}


class SelModGeomPipelineEnumProc: public  GeomPipelineEnumProc
{
public:	
	SelModGeomPipelineEnumProc(ModContext& in_mc) : mFound(false), mModContext(in_mc) { }
	PipeEnumResult proc(ReferenceTarget *object, IDerivedObject *derObj, int index);
	bool mFound;
	ModContext& mModContext;

protected:
	SelModGeomPipelineEnumProc(); // disallowed
	SelModGeomPipelineEnumProc(SelModGeomPipelineEnumProc& rhs); // disallowed
	SelModGeomPipelineEnumProc& operator=(const SelModGeomPipelineEnumProc& rhs); // disallowed
};

PipeEnumResult SelModGeomPipelineEnumProc::proc(
	ReferenceTarget *object, 
	IDerivedObject *derObj, 
	int index)
{
	ModContext* curModCtx = NULL;
	if ((derObj != NULL) && (curModCtx = derObj->GetModContext(index)) == &mModContext) 
	{
		DbgAssert(object != NULL);
		DbgAssert(object->ClassID() == Class_ID(SELECTOSM_CLASS_ID,0));
		mFound = true;
		return PIPE_ENUM_STOP;
	}
	return PIPE_ENUM_CONTINUE;
}


INode* SelMod::GetNodeFromModContext(ModContext *in_modContext, int &which)
{

	sMyEnumProc dep;              
	DoEnumDependents(&dep);
	for ( int i = 0; i < dep.Nodes.Count(); i++)
	{
		INode *node = dep.Nodes[i];
		BOOL found = FALSE;

		if (node)
		{
			Object* obj = node->GetObjectRef();
			SelModGeomPipelineEnumProc pipeEnumProc(*in_modContext);
			EnumGeomPipeline(&pipeEnumProc, obj);
			if (pipeEnumProc.mFound)
			{
				which = i;
				return node;
			}
		}
	}
	return NULL;
}

void SelMod::AddTVNode()
{
	ITrackViewNode *tvr = GetCOREInterface()->GetTrackViewRootNode();
	if (tvr) {
		ITrackViewNode *global = tvr->GetNode(GLOBAL_VAR_TVNODE_CLASS_ID);
		if (global) {
			// If needed, add a new track view node under the "global tracks" node, to which all 
			// SelMod modifiers will hang their track view nodes.
			ITrackViewNode *selModRoot = global->GetNode(MCONTAINER_TVNODE_CLASS_ID);
			if (!selModRoot) 
			{
				ITrackViewNode *mcontainer = CreateITrackViewNode(TRUE);
				global->AddNode(mcontainer, GetString(IDS_PW_VOLDATA), MCONTAINER_TVNODE_CLASS_ID);
				selModRoot = mcontainer;
			}

			// If needed, create a new track view node for this modifier under the SelMod root tv node
			if (container == NULL)
			{
				container = CreateITrackViewNode(TRUE);
				container->HideChildren(TRUE);
				selModRoot->AddNode(container, GetString(IDS_PW_VOLDATA), CONTAINER_TVNODE_CLASS_ID);
				container->RegisterTVNodeNotify(notify);
			}
		}
	}
}

void SelMod::ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node)
{
	if(version >= SELMOD_VER4 && os->obj->IsSubClassOf(patchObjectClassID)) {
	}
	else	// If it's a TriObject, process it
	if(os->obj->IsSubClassOf(triObjectClassID)) {
		//need to update the normal spec that it has been modified since 
		//the modifier now tags topo has being changed in its requirements
		//only applies to tri objects
		TriObject* tobj = (TriObject*)os->obj;
		Mesh& mesh = tobj->GetMesh();
		MeshNormalSpec* spec = mesh.GetSpecifiedNormals();
		if (spec)
		{
			spec->SetFlag(MESH_NORMAL_MODIFIER_SUPPORT);
		}
	}
	else if (os->obj->IsSubClassOf (polyObjectClassID)) {
	}
	else	// If it can convert to a TriObject, do it
		if(os->obj->CanConvertToType(triObjectClassID)) {
		}
		else
		{
			badObject = TRUE;
		}

	rt = t;
	pblock2->GetValue (sel_volume,t,vol,FOREVER);

	if (mc.localData == NULL)
	{
		// Add a new trackview node 
		AddTVNode();

		SelModData *d  = new SelModData(0);
		mc.localData = d;

		INode *localnode;
		int id;
		localnode = GetNodeFromModContext(&mc,id);
		d->id = id;


		if (localnode)
		{
			container->AddController(localnode->GetTMController(), localnode->GetName(), SELNODE_TVNODE_CLASS_ID);
		}
		else
		{
			d->id = -1;
		}

		//create a back pointer to the container entry				
		//return and call notify again to force another update
		NotifyDependents(FOREVER, OBJ_CHANNELS, REFMSG_CHANGE);

		Interval valid;
		valid.SetEmpty();
		os->obj->UpdateValidity(GEOM_CHAN_NUM,valid); // Have to do this to get it to evaluate
		//		os->obj->UpdateValidity(TOPO_CHAN_NUM,valid); // Have to do this to get it to evaluate
		return;
	}
	else if (((SelModData *)mc.localData)->id == -1)
	{
		SelModData *d  = (SelModData *)mc.localData;

		AddTVNode();

		INode *localnode;
		int id;
		localnode = GetNodeFromModContext(&mc,id);

		if (localnode)
		{
			container->AddController(localnode->GetTMController(), localnode->GetName(), SELNODE_TVNODE_CLASS_ID);
			d->id = id;
		}
		else{
			d->id = -1;
		}
		//create a back pointer to the container entry				
		//return and call notify again to force another update
		NotifyDependents(FOREVER, OBJ_CHANNELS, REFMSG_CHANGE);

		Interval valid;
		valid.SetEmpty();
		os->obj->UpdateValidity(GEOM_CHAN_NUM,valid); // Have to do this to get it to evaluate
		if (vol == SEL_MESH_OBJECT)
			return;
	}

	if ((ip && curMod == this) && (mc.localData == NULL))
	{
		NotifyDependents(FOREVER, OBJ_CHANNELS, REFMSG_CHANGE);
		Interval valid;
		valid.SetEmpty();
		os->obj->UpdateValidity(GEOM_CHAN_NUM,valid); // Have to do this to get it to evaluate
		return;
	}


	if (container == NULL)
	{
		if (vol == SEL_MESH_OBJECT) 
		{
			SelModData *d  = (SelModData *) mc.localData;
			if ((d!= NULL) && (d->selfNode !=NULL))
			{
				AddTVNode();

				container->AddController(d->selfNode->GetTMController(), d->selfNode->GetName(), SELNODE_TVNODE_CLASS_ID);
				d->id = 0;
			}
			else
				return;
		}
	}

	Interval valid = GetValidity(t);
	//build local data now
	pblock2->GetValue (sel_level,t,level,FOREVER);
	pblock2->GetValue (sel_method,t,method,FOREVER);
	pblock2->GetValue (sel_type,t,selType,FOREVER);
	pblock2->GetValue (sel_invert,t,invert,FOREVER);
	pblock2->GetValue (sel_map,t,map,FOREVER);
	pblock2->GetValue (sel_matid, t, matID, FOREVER);
	matID--;
	pblock2->GetValue (sel_smGroup, t, smG, FOREVER);
	pblock2->GetValue (sel_map_channel,t,channel,FOREVER);

	pblock2->GetValue (sel_autofit,t,autoFit,FOREVER);
	if (level == SEL_VERTEX)
	{
		pblock2_afr->GetValue(sel_use_ar, t, useAR, FOREVER);
		pblock2_afr->GetValue(sel_falloff, t, falloff, FOREVER);
		pblock2_afr->GetValue(sel_pinch, t, pinch, FOREVER);
		pblock2_afr->GetValue(sel_bubble, t, bubble, FOREVER);
		pblock2_afr->GetValue(sel_edge_limit, t, edgeLimit, FOREVER);
	}
	else
		useAR = FALSE;

	targnode = NULL;
	tmap = NULL;

	TriObject *triObj = NULL;
	PatchObject *patchObj = NULL;
	PolyObject *polyObj = NULL;
	BOOL converted = FALSE;

	// For version 4 and later, we process patch & poly meshes as they are and pass them on.  Earlier
	// versions converted to TriMeshes (done below).  For adding other new types of objects, add
	// them here!
	if(version >= SELMOD_VER4 && os->obj->IsSubClassOf(patchObjectClassID)) {
		patchObj = (PatchObject *)os->obj;
	}
	else
	{
		// If it's a TriObject, process it
		if (os->obj->IsSubClassOf(triObjectClassID))
		{
			triObj = (TriObject*)os->obj;
		}
		else if (os->obj->IsSubClassOf(polyObjectClassID))
		{
			polyObj = (PolyObject*)os->obj;
		}
		else // If it can convert to a TriObject, do it
				if (os->obj->CanConvertToType(triObjectClassID))
		{
			triObj = (TriObject*)os->obj->ConvertToType(t, triObjectClassID);
			converted = TRUE;
		}
		else
		{
			badObject = TRUE;
			if (!tmControl || (flags & CONTROL_OP))
				InitControl(mc, os->obj, t);

			return; // We can't deal with it!
		}

		Mesh& mesh = triObj ? triObj->GetMesh() : *((Mesh*)NULL);

		PatchMesh& pmesh = patchObj ? patchObj->GetPatchMesh(t) : *((PatchMesh*)NULL);

		TriObject* collapsedtobj = NULL;

		// is texture selection
		if (vol == SEL_TEXTURE)
		{
			pblock2->GetValue(sel_texture, t, tmap, FOREVER);
			if (tmap)
			{
				tmap->LoadMapFiles(t);
				tmap->Update(t, FOREVER);
			}
			shadeContext.scale = 1.0f;
			shadeContext.duvw = Point3(0.001f, 0.001f, 0.0f);
			shadeContext.dpt = Point3(0.001f, 0.001f, 0.001f);
			shadeContext.curTime = t;
			if (uvwList.Count() != os->obj->NumPoints())
				uvwList.SetCount(os->obj->NumPoints());
			int currentChannel;
			currentChannel = map ? 0 : channel;

			if (triObj)
			{
				if (uvwList.Count() != mesh.numVerts)
					uvwList.SetCount(mesh.numVerts);

				TVFace* tvFace = mesh.mapFaces(currentChannel);
				Point3* tVerts = NULL;
				if (tvFace != NULL)
					tVerts = mesh.mapVerts(currentChannel);
				Tab<int> ctList;
				ctList.SetCount(mesh.numVerts);

				const Point3 tv(0.0f, 0.0f, 0.0f);
				for (int i = 0; i < mesh.getNumVerts(); i++)
				{				
					ctList[i] = 0;
					uvwList[i] = tv;
				}
				if (tvFace != NULL)
				{
					for (int j = 0; j < mesh.numFaces; j++)
					{
						for (int k = 0; k < 3; k++)
						{
							int index = mesh.faces[j].v[k];
							int tindex = tvFace[j].t[k];
							if (ctList[index] == 0)
								uvwList[index] = tVerts[tindex];
							ctList[index] += 1;
						}
					}
				}
			}
			else if (polyObj)
			{
				MNMesh& mm = polyObj->GetMesh();
				MNMapFace* tvFace = NULL;
				UVVert* tVerts = NULL;
				if (mm.M(currentChannel) && !mm.M(currentChannel)->GetFlag(MN_DEAD))
				{
					tvFace = mm.M(currentChannel)->f;
					tVerts = mm.M(currentChannel)->v;
				}
				Tab<int> ctList;
				ctList.SetCount(os->obj->NumPoints());

				const Point3 tv(0.0f, 0.0f, 0.0f);
				for (int i = 0; i < mm.numv; i++)
				{
					ctList[i] = 0;
					uvwList[i] = tv;
				}
				if (tvFace != NULL)
				{
					for (int j = 0; j < mm.numf; j++)
					{
						for (int k = 0; k < mm.f[j].deg; k++)
						{
							int index = mm.f[j].vtx[k];
							int tindex = tvFace[j].tv[k];
							if (ctList[index] == 0)
								uvwList[index] = tVerts[tindex];
							ctList[index] += 1;
						}
					}
				}
			}
			else if (patchObj)
			{
				TVPatch* tvPatch = pmesh.mapPatches(currentChannel);
				PatchTVert* tVerts = NULL;
				if (tvPatch != NULL)
					tVerts = pmesh.mapVerts(currentChannel);
				Tab<int> ctList;
				ctList.SetCount(os->obj->NumPoints());

				const Point3 tv(0.0f, 0.0f, 0.0f);
				for (int i = 0; i < pmesh.getNumVerts(); i++)
				{
					ctList[i] = 0;
					uvwList[i] = tv;
				}
				if (tvPatch != NULL)
				{
					for (int j = 0; j < pmesh.numPatches; j++)
					{
						Patch& p = pmesh.patches[j];
						for (int k = 0; k < p.type; k++)
						{
							int index = pmesh.patches[j].v[k];
							int tindex = tvPatch[j].tv[k];
							if (ctList[index] == 0)
								uvwList[index] = tVerts[tindex].p;
							ctList[index] += 1;
						}
					}
				}
			}
		}
		else if ((vol == SEL_MESH_OBJECT) && (level != SEL_OBJECT))
		{
			// is object
			pblock2->GetValue(sel_node, t, targnode, FOREVER);
			msh = NULL;
			if (targnode != NULL)
			{
				sos = targnode->EvalWorldState(t);
				sos.obj->GetDeformBBox(rt, bbox);
				bboxFalloff = bbox;
				if (useAR)
					bboxFalloff.EnlargeBy(falloff);

				// is spline
				if (sos.obj->SuperClassID() == SHAPE_CLASS_ID)
				{
					pathOb = (ShapeObject*)sos.obj;
					pathOb->MakePolyShape(t, workShape);
				}
				else if (sos.obj->IsParticleSystem())
				{
					// is particle
					epobj = NULL;
					pobj = (SimpleParticle*)sos.obj->GetInterface(I_SIMPLEPARTICLEOBJ);
					if (pobj)
						pobj->UpdateParticles(t, targnode);
					else
					{
						epobj = (IParticleObjectExt*)sos.obj->GetInterface(PARTICLEOBJECTEXT_INTERFACE);

						if (epobj)
							epobj->UpdateParticles(targnode, t);
					}
				}
				else
				{
					// is object
					if (sos.obj->IsSubClassOf(triObjectClassID))
					{
						TriObject* tobj = (TriObject*)sos.obj;
						msh = &tobj->GetMesh();
					}
					// collapse it to a mesh
					else
					{
						if (sos.obj->CanConvertToType(triObjectClassID))
						{
							collapsedtobj = (TriObject*)sos.obj->ConvertToType(t, triObjectClassID);
							msh = &collapsedtobj->GetMesh();
						}
					}
				}
			}
		}

		if ((vol == SEL_MESH_OBJECT) && (level != SEL_OBJECT))
		{
			int id;
			INode* SelfNode = GetNodeFromModContext(&mc, id);

			if (SelfNode)
			{
				Interval iv;
				// check if local data present if so use it
				ntm = SelfNode->GetObjectTM(t, &iv);

				// build box hit list
				if (msh != NULL)
				{
					if (box3DArray.Count() != msh->numFaces)
					{
						box3DArray.SetCount(msh->numFaces);
						normList.SetCount(msh->numFaces);
					}

					Point3 pnorm[3];
					// Point2 p;
					for (int i = 0; i < msh->numFaces; i++)
					{
						box3DArray.Init(i, useAR);

						for (int j = 0; j < 3; j++)
						{
							int index = msh->faces[i].v[j];
							Point3& p = msh->verts[index];

							pnorm[j] = p;
							box3DArray.AddPoint(i, p);
						}
						box3DArray.Expand(i, falloff);
						normList[i] = Normalize(pnorm[1] - pnorm[0]) ^ (pnorm[2] - pnorm[1]);
					}
					box3DArray.PrepForTesting(bbox, bboxFalloff);
				}

				rt = t;

				if (mc.localData != NULL)
				{
					SelModData* d = (SelModData*)mc.localData;
					if (d->selfNode)
						ntm = d->selfNode->GetObjectTM(t, &iv);
					else
					{
						d->selfNode = SelfNode;
						ntm = d->selfNode->GetObjectTM(t, &iv);
					}
					if (targnode != NULL)
					{
						otm = targnode->GetObjectTM(t, &iv);
						otm = Inverse(otm);
						otm = ntm * otm;
						bboxTargetLocal = TransformBox(bboxFalloff, Inverse(otm));
					}
				}
			}
		}
		else
		{
			box3DArray.ZeroCount();
			normList.ZeroCount();
		}
		// copied the mc box into our box so we cause problems later up in the stack.
		if ((mc.box != NULL) && (flags & CONTROL_CHANGEFROMBOX))
		{
			mcBox.pmin = mc.box->pmin;
			mcBox.pmax = mc.box->pmax;
			flags &= ~CONTROL_CHANGEFROMBOX;
		}

		// Determine which bounding box to use (stack or gizmo)
		// Issues were occuring as the gizmo was being used for
		// early mesh rejection.
		Box3 rejectionBox;
		if (vol == SEL_MESH_OBJECT)
		{
			if (mc.box)
				rejectionBox = *mc.box;
		}
		else
			rejectionBox = mcBox;
		

		// Prepare the controller and set up mats
		if (!tmControl || (flags & CONTROL_OP))
			InitControl(mc, os->obj, t);

		Matrix3 ctm = CompMatrix(t, &mc, NULL, TRUE, FALSE);
		Matrix3 tm = Inverse(ctm);
		FixupBox(rejectionBox);

		if (triObj)
		{
			if (mesh.VertSel().GetSize() != mesh.getNumVerts())
				mesh.VertSel().SetSize(triObj->GetMesh().getNumVerts(), 1);
			if (mesh.FaceSel().GetSize() != mesh.getNumFaces())
				mesh.FaceSel().SetSize(mesh.getNumFaces(), 1);

			switch (level)
			{
			case SEL_OBJECT:
				mesh.selLevel = MESH_OBJECT;
				mesh.ClearDispFlag(DISP_VERTTICKS | DISP_SELVERTS | DISP_SELFACES);
				break;

			case SEL_VERTEX:
				mesh.selLevel = MESH_VERTEX;
				mesh.SetDispFlag(DISP_VERTTICKS | DISP_SELVERTS);
				SelectVertices(triObj->GetMesh(), tm, ctm, rejectionBox, t);
				break;

			case SEL_FACE:
				mesh.selLevel = MESH_FACE;
				mesh.SetDispFlag(DISP_SELFACES);
				SelectFaces(t, triObj->GetMesh(), tm, rejectionBox);
				break;
			}
		}
		else if (polyObj)
		{
			MNMesh& mm = polyObj->GetMesh();
			switch (level)
			{
			case SEL_OBJECT:
				mm.selLevel = MNM_SL_OBJECT;
				mm.ClearDispFlag(MNDISP_VERTTICKS | MNDISP_SELVERTS | MNDISP_SELFACES | MNDISP_SELEDGES);
				break;

			case SEL_VERTEX:
				mm.selLevel = MNM_SL_VERTEX;
				mm.SetDispFlag(MNDISP_VERTTICKS | MNDISP_SELVERTS);
				mm.ClearDispFlag(MNDISP_SELFACES | MNDISP_SELEDGES);
				SelectVertices(mm, tm, ctm, rejectionBox, t);
				break;

			case SEL_FACE:
				mm.selLevel = MNM_SL_FACE;
				mm.SetDispFlag(MNDISP_SELFACES);
				mm.ClearDispFlag(MNDISP_VERTTICKS | MNDISP_SELVERTS | MNDISP_SELEDGES);
				SelectFaces(t, mm, tm, rejectionBox);
				break;
			}
		}
		else if (patchObj)
		{
			if (pmesh.vertSel.GetSize() != pmesh.getNumVerts())
				pmesh.vertSel.SetSize(pmesh.getNumVerts(), 1);
			if (pmesh.patchSel.GetSize() != pmesh.getNumPatches())
				pmesh.patchSel.SetSize(pmesh.getNumPatches(), 1);

			switch (level)
			{
			case SEL_OBJECT:
				pmesh.selLevel = PATCH_OBJECT;
				pmesh.ClearDispFlag(DISP_VERTTICKS | DISP_SELVERTS | DISP_SELPATCHES);
				break;

			case SEL_VERTEX:
				pmesh.selLevel = PATCH_VERTEX;
				pmesh.SetDispFlag(DISP_VERTTICKS | DISP_SELVERTS);
				SelectVertices(pmesh, tm, ctm, rejectionBox, t);
				break;

			case SEL_FACE:
				pmesh.selLevel = PATCH_PATCH;
				pmesh.SetDispFlag(DISP_SELPATCHES);
				SelectPatches(t, pmesh, tm, rejectionBox);
				break;
			}
		}

		if (collapsedtobj)
			collapsedtobj->DeleteThis();

		if (!converted)
		{
			os->obj->UpdateValidity(SELECT_CHAN_NUM, valid);
			os->obj->UpdateValidity(GEOM_CHAN_NUM, valid); // Have to do this to get it to evaluate
			os->obj->UpdateValidity(SUBSEL_TYPE_CHAN_NUM, FOREVER);
		}
		else
		{
			// Stuff converted object into the pipeline!
			triObj->SetChannelValidity(GEOM_CHAN_NUM, valid);
			triObj->SetChannelValidity(TEXMAP_CHAN_NUM, valid);
			triObj->SetChannelValidity(MTL_CHAN_NUM, valid);
			triObj->SetChannelValidity(SELECT_CHAN_NUM, valid);
			triObj->SetChannelValidity(SUBSEL_TYPE_CHAN_NUM, valid);
			triObj->SetChannelValidity(DISP_ATTRIB_CHAN_NUM, valid);

			os->obj = triObj;
		}
	}
}

int SelMod::SubNumToRefNum(int subNum)
{
	return subNum;
}

BOOL SelMod::AssignController(Animatable *control,int subAnim)
{
	ReplaceReference(subAnim,(Control*)control);
	NotifyDependents(FOREVER,0,REFMSG_CONTROLREF_CHANGE);
	NotifyDependents(FOREVER,0,REFMSG_CHANGE);
	return TRUE;
}

RefTargetHandle SelMod::GetReference(int i)
{
	switch (i) {
		case PBLOCK_REF: return pblock2;
		case TM_REF: return tmControl;
		case POS_REF: return posControl;
		case PBLOCK_AFR_REF: return pblock2_afr;
		default: return NULL;
	}
}

void SelMod::SetReference(int i, RefTargetHandle rtarg)
{
	switch (i) {
		case PBLOCK_REF: pblock2 = (IParamBlock2*)rtarg; break;
		case TM_REF: tmControl = (Control*)rtarg; break;
		case POS_REF: posControl = (Control*)rtarg; break;
		case PBLOCK_AFR_REF: pblock2_afr = (IParamBlock2*)rtarg; break;
	}
}

Animatable* SelMod::SubAnim(int i)
{
	switch (i) {
		case PBLOCK_REF: return pblock2;
		case TM_REF: return tmControl;
		case POS_REF: return posControl;
		case PBLOCK_AFR_REF: return pblock2_afr;
		default: return NULL;
	}
}

TSTR SelMod::SubAnimName(int i, bool localized)
{
	switch (i)
	{
	case PBLOCK_REF: return localized ? GetString(IDS_RB_PARAMETERS) : _T("Parameters");
	case TM_REF: return localized ? GetString(IDS_RB_APPARATUS) : _T("Gizmo");
	case POS_REF: return localized ? GetString(IDS_RB_CENTER) : _T("Center");
	case PBLOCK_AFR_REF: return localized ? GetString(IDS_MS_SOFTSEL) : _T("Soft Selection");
	default: return TSTR(_T(""));
	}
}

RefResult SelMod::NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, 
		RefMessage message, BOOL propagate) 
{
	switch (message) {
		case REFMSG_CHANGE:
			// see if this message came from a changing parameter in the pblock,
			// if so, limit rollout update to the changing item and update any active viewport texture
			if (hTarget == pblock2)
			{
				ParamID changing_param = pblock2->LastNotifyParamID();

				sel_param_blk.InvalidateUI(changing_param);

				// NS: 5/15/00 Update the StackView
				if(changing_param == sel_level)
				{
					// First pass on the REFMSG_CHANGED, so the modstack gets invalidated
					// and then pass on the REFMSG_NUM_SUBOBJECTTYPES_CHANGED, so that the 
					// StackView gets updated.

					NotifyDependents(changeInt, partID, message);
					NotifyDependents(FOREVER, 0, REFMSG_NUM_SUBOBJECTTYPES_CHANGED);
					return REF_STOP;
				}
				else if (changing_param == sel_texture)
				{

					int m;
					//hack alert for some reason materials dep are not updating the stack this forces an update
					pblock2->GetValue (sel_method, GetCOREInterface()->GetTime(), m, FOREVER);
					pblock2->SetValue (sel_method, GetCOREInterface()->GetTime(), m);
					GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
				}
			}
			if (hTarget == pblock2_afr) 
			{
				ParamID changing_param = pblock2_afr->LastNotifyParamID();

				sel_afr_blk.InvalidateUI(changing_param);
			}

			break;
	}
	return REF_SUCCEED;
}


// --- Gizmo transformations ------------------------------------------

void SelMod::Move(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin) 
{	
	if (tmControl == NULL) return;
	if (ip && ip->GetSubObjectLevel()==1) {		
		SetXFormPacket pckt(val,partm,tmAxis);
		tmControl->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);
	} else {
		Matrix3 ptm = partm;				
		tmControl->GetValue(t,&ptm,FOREVER,CTRL_RELATIVE);
		posControl->SetValue(t,
			VectorTransform(tmAxis*Inverse(ptm),val),TRUE,CTRL_RELATIVE);
	}
}

void SelMod::Rotate(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Quat& val, BOOL localOrigin) 
{
	if (tmControl == NULL) return;
	SetXFormPacket pckt(val,localOrigin,partm,tmAxis);
	tmControl->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);		
}

void SelMod::Scale(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin) 
{
	if (tmControl == NULL) return;
	SetXFormPacket pckt(val,localOrigin,partm,tmAxis);
	tmControl->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);		
}

Matrix3 SelMod::CompMatrix(TimeValue t,ModContext *mc, Matrix3 *ntm, BOOL scale, BOOL offset)
{
	Matrix3 tm(1);
	Interval valid;
	if (autoFit)
	{
		Box3 mcbox;
		if (mc && mc->box) mcbox = *mc->box;
		FixupBox(mcbox);

		if (mc && scale && (flags&CONTROL_USEBOX)) {
			tm.Scale(mcbox.Width()/2.0f);
			tm.Translate(mcbox.Center());
		}
	}
	else
	{
		FixupBox(mcBox);

		if (mc && scale && (flags&CONTROL_USEBOX)) {
			tm.Scale(mcBox.Width()/2.0f);
			tm.Translate(mcBox.Center());
		}
	}

	if (posControl && offset) {
		Matrix3 tmc(1);
		posControl->GetValue(t,&tmc,valid,CTRL_RELATIVE);		
		tm = tm * tmc;
	}
	if (tmControl) {
		Matrix3 tmc(1);
		tmControl->GetValue(t,&tmc,valid,CTRL_RELATIVE);		
		tm = tm * tmc;
	}

	if (mc && mc->tm) {
		tm = tm * Inverse(*mc->tm);
	}
	if (ntm) {
		tm = tm * *ntm;
	}
	return tm;
}


void SelMod::DoIcon(PolyLineProc& lp,BOOL sel) 
{
	switch (vol) {
		case SEL_BOX: DoBoxIcon(sel,2.0f,lp); break;
		case SEL_SPHERE: DoSphereIcon(sel,1.0f,lp); break;
		case SEL_CYLINDER: DoCylinderIcon(sel,1.0f,2.0f,lp); break;
	}
}

static Box3 unitBox(Point3(-0.5f,-0.5f,-0.5f),Point3(0.5f,0.5f,0.5f));
 
int SelMod::HitTest (TimeValue t, INode* inode, int type, int crossing, 
		int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc) 
{
	if ( ! vpt || ! vpt->IsAlive() )
	{
		// why are we here
		DbgAssert(!_T("Invalid viewport!"));
		return FALSE;
	}

	int savedLimits;	
	GraphicsWindow *gw = vpt->getGW();
	HitRegion hr;
	MakeHitRegion(hr,type, crossing,4,p);
	gw->setHitRegion(&hr);
	Matrix3 modmat, ntm = inode->GetObjectTM(t);
	DrawLineProc lp(gw);

	gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);
	gw->clearHitCode();

	if (ip && ip->GetSubObjectLevel()==1) {
		modmat = CompMatrix(t,mc,&ntm,TRUE,FALSE);
		gw->setTransform(modmat);
		DoIcon(lp,FALSE);
	}

	if (ip && (
		ip->GetSubObjectLevel()==1 ||
		ip->GetSubObjectLevel()==2)) {
			modmat = CompMatrix(t,mc,&ntm);
			gw->setTransform(modmat);
			DrawCenterMark(lp,unitBox);		
	}

	gw->setRndLimits(savedLimits);	
	if (gw->checkHitCode()) {
		vpt->LogHit(inode, mc, gw->getHitDistance(), 0, NULL); 
		return 1;
	}
	return 0;
}

int SelMod::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags, ModContext *mc) 
{	
	if ( ! vpt || ! vpt->IsAlive() )
	{
		// why are we here
		DbgAssert(!_T("Invalid viewport!"));
		return FALSE;
	}

	GraphicsWindow *gw = vpt->getGW();
	// Transform the gizmo with the node.
	Matrix3 modmat, ntm = inode->GetObjectTM(t);
	DrawLineProc lp(gw);

	modmat = CompMatrix(t,mc,&ntm,TRUE,FALSE);	
	gw->setTransform(modmat);	
	DoIcon(lp, ip&&ip->GetSubObjectLevel()==1);

	modmat = CompMatrix(t,mc,&ntm);
	gw->setTransform(modmat);
	if (ip && (
		ip->GetSubObjectLevel()==1 ||
		ip->GetSubObjectLevel()==2)) {
			//gw->setColor(LINE_COLOR, (float)1.0, (float)1.0, (float)0.0);
			gw->setColor(LINE_COLOR,GetUIColor(COLOR_SEL_GIZMOS));
	} else {
		//gw->setColor( LINE_COLOR, (float).85, (float).5, (float)0.0);
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_GIZMOS));
	}
	DrawCenterMark(lp,unitBox);
	return 0;	
}

void SelMod::GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc) 
{
	if ( ! vpt || ! vpt->IsAlive() )
	{
		// why are we here
		DbgAssert(!_T("Invalid viewport!"));
		return;
	}

	// Need the correct bound box for proper damage rect calcs.
	Matrix3 modmat, ntm = inode->GetObjectTM(t);	
	modmat = CompMatrix(t,mc,&ntm,TRUE,FALSE);		
	BoxLineProc bproc(&modmat);
	DoIcon(bproc,FALSE);

	modmat = CompMatrix(t,mc,&ntm);
	DrawCenterMark(bproc,unitBox);

	box = bproc.Box();	
}

void SelMod::GetSubObjectCenters(
								 SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc)
{	
	Matrix3 modmat, ntm = node->GetObjectTM(t);		
	modmat = CompMatrix(t,mc,&ntm);
	cb->Center(modmat.GetTrans(),0);	
}

void SelMod::GetSubObjectTMs(
							 SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc)
{
	Matrix3 ntm = node->GetObjectTM(t), modmat;
	modmat = CompMatrix(t,mc,&ntm);
	cb->TM(modmat,0);
}

void SelMod::ActivateSubobjSel(int level, XFormModes& modes )
{	
	switch (level) {
		case 1: // Modifier box
			modes = XFormModes(moveMode,rotMode,nuscaleMode,uscaleMode,squashMode,NULL);
			break;
		case 2: // Modifier Center
			modes = XFormModes(moveMode,NULL,NULL,NULL,NULL,NULL);
			break;
	}
	NotifyDependents(FOREVER,PART_DISPLAY,REFMSG_CHANGE);
}


//----------------------------------------------------------------


#define NUM_SEGS	16

static void DoSphereIcon(BOOL sel,float radius, PolyLineProc& lp)
{
	float u;
	Point3 pt[3];

	if (sel)
		lp.SetLineColor(GetUIColor(COLOR_SEL_GIZMOS));
	else		
		lp.SetLineColor(GetUIColor(COLOR_GIZMOS));

	// XY
	pt[0] = Point3(radius,0.0f,0.0f);
	for (int i=1; i<=NUM_SEGS; i++) {
		u = float(i)/float(NUM_SEGS) * TWOPI;
		pt[1].x = (float)cos(u) * radius;
		pt[1].y = (float)sin(u) * radius;
		pt[1].z = 0.0f;
		lp.proc(pt,2);
		pt[0] = pt[1];
	}

	// YZ	
	pt[0] = Point3(0.0f,radius,0.0f);
	for (int i=1; i<=NUM_SEGS; i++) {
		u = float(i)/float(NUM_SEGS) * TWOPI;
		pt[1].y = (float)cos(u) * radius;
		pt[1].z = (float)sin(u) * radius;
		pt[1].x = 0.0f;
		lp.proc(pt,2);
		pt[0] = pt[1];
	}

	// ZX	
	pt[0] = Point3(0.0f,0.0f,radius);
	for (int i=1; i<=NUM_SEGS; i++) {		
		u = float(i)/float(NUM_SEGS) * TWOPI;
		pt[1].z = (float)cos(u) * radius;
		pt[1].x = (float)sin(u) * radius;
		pt[1].y = 0.0f;
		lp.proc(pt,2);
		pt[0] = pt[1];
	}
}

static void DoCylinderIcon(BOOL sel,float radius, float height, PolyLineProc& lp)
{
	float u;
	Point3 pt[5], opt;

	if (sel)
		lp.SetLineColor(GetUIColor(COLOR_SEL_GIZMOS));
	else		
		lp.SetLineColor(GetUIColor(COLOR_GIZMOS));

	height *= 0.5f;

	opt = Point3(radius,0.0f,-height);
	for (int i=1; i<=NUM_SEGS; i++) {
		u = float(i)/float(NUM_SEGS) * TWOPI;
		pt[0]   = opt;

		pt[1].x = (float)cos(u) * radius;
		pt[1].y = (float)sin(u) * radius;
		pt[1].z = -height;

		pt[2].x = pt[1].x;
		pt[2].y = pt[1].y;
		pt[2].z = height;

		pt[3]   = opt;
		pt[3].z = height;

		lp.proc(pt,4);		
		opt = pt[1];
	}
}


static int lStart[12] = {0,1,3,2,4,5,7,6,0,1,2,3};
static int lEnd[12]   = {1,3,2,0,5,7,6,4,4,5,6,7};

static void DoBoxIcon(BOOL sel,float length, PolyLineProc& lp)
{
	Point3 pt[3];

	length *= 0.5f;
	Box3 box;
	box.pmin = Point3(-length,-length,-length);
	box.pmax = Point3( length, length, length);

	if (sel) //lp.SetLineColor(1.0f,1.0f,0.0f);
		lp.SetLineColor(GetUIColor(COLOR_SEL_GIZMOS));
	else //lp.SetLineColor(0.85f,0.5f,0.0f);		
		lp.SetLineColor(GetUIColor(COLOR_GIZMOS));

	for (int i=0; i<12; i++) {
		pt[0] = box[lStart[i]];
		pt[1] = box[lEnd[i]];
		lp.proc(pt,2);
	}
}
void SelMod::DisableAffectRegion (TimeValue t) 
{
	IParamMap2 *afrmap = pblock2_afr->GetMap();
	afrmap->Enable(sel_use_ar, FALSE);
	afrmap->Enable(sel_falloff, FALSE);
	afrmap->Enable(sel_pinch, FALSE);
	afrmap->Enable(sel_bubble, FALSE);
}

void SelMod::EnableAffectRegion (TimeValue t) 
{
	int sl;
	pblock2->GetValue (sel_level, t, sl, FOREVER);
	int v;
	pblock2->GetValue (sel_volume, t, v, FOREVER);
	BOOL use;

//if ((vol!=3) && (selLevel == 1))
//if ( (level == 1))
//watje 5-26-99
	if (( (v < 4 ) && (sl == 1)) ||
		( (v == 7 ) && (sl == 1)) )
	{
		pblock2_afr->GetValue (sel_use_ar, t, use, FOREVER);

		IParamMap2 *afrmap = pblock2_afr->GetMap();
		afrmap->Enable(sel_use_ar, TRUE);
		if (useAR)
		{
			afrmap->Enable(sel_falloff, TRUE);
			afrmap->Enable(sel_pinch, TRUE);
			afrmap->Enable(sel_bubble, TRUE);
		}
	}
}


#define ID_CHUNK 0x1000
#define NODE_CHUNK 0x1010

IOResult SelMod::SaveLocalData(ISave *isave, LocalModData *pld)
{
	SelModData *p;
	IOResult	res;
	ULONG		nb;

	p = (SelModData*)pld;

	isave->BeginChunk(ID_CHUNK);
	res = isave->Write(&p->id, sizeof(int), &nb);
	isave->EndChunk();

	ULONG id = isave->GetRefID(p->selfNode);

	isave->BeginChunk(NODE_CHUNK);
	isave->Write(&id,sizeof(ULONG),&nb);
	isave->EndChunk();

	return IO_OK;
}

IOResult SelMod::LoadLocalData(ILoad *iload, LocalModData **pld)
{
	IOResult	res;
	ULONG		nb;

	int id;
	SelModData *p= new SelModData();
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case ID_CHUNK:
				iload->Read(&id,sizeof(int), &nb);
				p->id = id;
				break;
			case NODE_CHUNK:
				ULONG id;
				iload->Read(&id,sizeof(ULONG), &nb);
				if (id!=0xffffffff)
					{
					iload->RecordBackpatch(id,(void**)&p->selfNode);
					}
				break;

		}
		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}

	*pld = p;
	return IO_OK;
}

int SelMod::NumSubObjTypes() 
{ 
	return 2;
}

ISubObjType *SelMod::GetSubObjType(int i) 
{	
	static bool initialized = false;
	if(!initialized)
	{
		initialized = true;
		SOT_Apparatus.SetName(GetString(IDS_RB_APPARATUS));
		SOT_Center.SetName(GetString(IDS_RB_CENTER));
		SOT_Vertex.SetName(GetString(IDS_RB_VERTEX));
		SOT_Polygon.SetName(GetString(IDS_EM_POLY));

	}

	switch(i)
	{
	case -1:
		{
			int l;
			pblock2->GetValue (sel_level,0,l,FOREVER);
			
			switch(l)
			{
			case 0: return NULL;
			case 1: return &SOT_Vertex;
			case 2:	return &SOT_Polygon;
			}
		}
	case 0:
		return &SOT_Apparatus;
	case 1:
		return &SOT_Center;
	}

	return NULL;
}

void SelMod::DoAccept(TimeValue t)
{

}

void SelMod::SetPinch(TimeValue t, float pinch)
{
	pblock2_afr->SetValue (sel_pinch, t, pinch);
}

void SelMod::SetBubble(TimeValue t, float bubble)
{
	pblock2_afr->SetValue (sel_bubble, t, bubble);
}

void SelMod::SetFalloff(TimeValue t,float falloff)
{
	pblock2_afr->SetValue (sel_falloff, t, falloff);
}

float SelMod::GetPinch(TimeValue t)
{
	return pblock2_afr->GetFloat (sel_pinch, t);
}

float SelMod::GetBubble(TimeValue t)
{
	return pblock2_afr->GetFloat (sel_bubble, t);
}

float SelMod::GetFalloff(TimeValue t)
{
	return pblock2_afr->GetFloat (sel_falloff, t);
}

void SelMod::EditSoftSelectionMode()
{
	CommandMode *currentMode = ip->GetCommandMode ();
	if(currentMode!=editSSMode)
		ip->PushCommandMode(editSSMode);
	else
		ip->DeleteMode(editSSMode);
}
