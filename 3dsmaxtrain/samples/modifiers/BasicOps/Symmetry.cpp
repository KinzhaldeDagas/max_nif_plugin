/**********************************************************************
 *<
	FILE: symmetry.cpp

	DESCRIPTION: Symmetry Modifier (for Meshes, PolyMeshes.)

	CREATED BY: Steve Anderson, based on Vertex Weld, Mirror, and Slice modifiers.

	HISTORY: created 9/20/2001
			08/29/2020 - M. Kaustinen:QT Migration, New Features

 *>	Copyright (c) 2001 Discreet, All Rights Reserved.
 **********************************************************************/

#include "Symmetry.h"

#include "iparamm2.h"
#include "iFnPub.h"
#include "MeshDLib.h"
#include "MeshNormalSpec.h"
#include "MaxIcon.h"
#include "INodeTransformMonitor.h"
#include "../../maxsdk/samples/controllers/NodeTransformMonitor.h"

#include "FaceAlign.h"

#include "ui_Symmetry_Modifier.h"
#include <Geom/trig.h>

constexpr unsigned int kSYM_PBLOCK_REF = 0;
constexpr unsigned int kSYM_MIRROR_REF = 1;
constexpr unsigned int	NODE_TRANSFORM_MONITOR_REF = 2;

// Improved welding
constexpr DWORD	MN_WELD_SIDE1 = (MN_USER << 1);
constexpr DWORD MN_WELD_SIDE2 = (MN_USER << 2);
constexpr float WELD_THRESHOLD = 1e-2f;

constexpr float EDGE_WELD_THRESHOLD = WELD_THRESHOLD * 0.7f;

constexpr float EDGE_WELD_PERCENTAGE = WELD_THRESHOLD * 0.7f;

constexpr float MIRRORWELD_THRESHOLD_2023_1 = 1.1e-020f;
constexpr float MIRRORWELD_THRESHOLD_2023_2 = 1.1e-010f;




class SymmetryMod : public Modifier, public ObjectAlignInterface, public FPMixinInterface
{
	Control *mp_mirror;
	static IObjParam *mp_ip;
	static MoveModBoxCMode    *mp_moveMode;
	static RotateModBoxCMode  *mp_rotMode;
	static UScaleModBoxCMode *mp_scaleMode;
	static NUScaleModBoxCMode *mp_nuScaleMode;
	static SquashModBoxCMode	*mp_squashMode;

	static FaceAlignMode *faceAlignMode;

public:
	//script enum
	enum
	{
		//sets the modifier to the latest version, note this is a one way path
		kFn_ResetModifierVersion,
		//returns the modifiers current version
		kFn_ModifierVersion,
		//Updates the mirror weld value which is 1.1e-20 for 2023.1 and earlier.  It is  1.1e-10 afterwards
		//used to update older versions since we cannot break topo for older versions
		kFn_UpdateWeldThreshold,

		//these let you reset back the modifier to an older version of the algorithm 
		//use at your own risk
		kFn_GetModifierVersion, kFn_SetModifierVersion, //see Symmetry.h OLD_SYMMETRY_VERSION for valid values
		kFn_GetLegacyWeld, kFn_SetLegacyWeld,
		kFn_GetMirrorWeldThreshold, kFn_SetMirrorWeldThreshold

	};
	// param block entries
	enum {
		kSymAxis, kSymSlice, 
		kSymWeld, 
		kSymThreshold, 

		kSymFlip, 
		kSymFormat, 
		// Planar Options
		kSymPlanar_X, kSymPlanar_Y,	kSymPlanar_Z, kSymPlanarFlip_X,	kSymPlanarFlip_Y, kSymPlanarFlip_Z,
		// Radial
		kSymRadialAxis,	kSymRadialFlip,	kSymRadialCount, kSymRadialMirror,
		// Ref Node
		kSymNodeRef,
		kSymProximityCheck,
		//This is the  percentage used to determine whether an edge on the collapse plane is collapsed
		//This is multiplied by the average edge length and anything less than that is collapsed
		kEdgeCollapsePercent

	};

	// Symmetry Formats (Planar, Radial)
	enum {
		symmetry_planar,
		symmetry_radial,
	};

	IParamBlock2 *mp_pblock;

	SymmetryMod(BOOL bLoading = FALSE);

	FPInterfaceDesc* GetDesc();

	// From Animatable
	void DeleteThis() { delete this; }
	void GetClassName(MSTR& s, bool localized) const override {s = localized ? GetString(IDS_SYMMETRY_MOD) : _T("Symmetry");}
	virtual Class_ID ClassID () { return kSYMMETRY_CLASS_ID;}
	RefTargetHandle Clone (RemapDir& remap);
	const TCHAR *GetObjectName(bool localized) const override { return localized ? GetString(IDS_SYMMETRY_MOD) : _T("Symmetry"); }
	BOOL AssignController(Animatable *control,int subAnim);
	int SubNumToRefNum(int subNum);

	void BeginEditParams (IObjParam  *ip, ULONG flags,Animatable *prev);
	void EndEditParams (IObjParam *ip,ULONG flags,Animatable *next);		

	// From modifier
	// Since we're changing topology, all these other channels are affected as well:
	ChannelMask ChannelsUsed()  {return GEOM_CHANNEL|TOPO_CHANNEL|TEXMAP_CHANNEL|SELECT_CHANNEL|SUBSEL_TYPE_CHANNEL|VERTCOLOR_CHANNEL; }
	ChannelMask ChannelsChanged() {return GEOM_CHANNEL|TOPO_CHANNEL|SELECT_CHANNEL|TEXMAP_CHANNEL|VERTCOLOR_CHANNEL; }
	Class_ID InputType() { return defObjectClassID; }
	void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
	Interval LocalValidity(TimeValue t);

	// From BaseObject
	int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc);
	int Display(TimeValue t, INode* inode, ViewExp *vpt, int flagst, ModContext *mc);
	void GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc);		
	void Move(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin);
	void Rotate(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Quat& val, BOOL localOrigin);
	void Scale (TimeValue t, Matrix3 & partm, Matrix3 & tmAxis, Point3 & val, BOOL localOrigin);
	CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;} 
	void GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
	void GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
	void ActivateSubobjSel(int level, XFormModes& modes);

	// Subobject API
	int NumSubObjTypes () { return 1; }
	ISubObjType *GetSubObjType (int i);

	// ParamBlock2 access:
	int NumParamBlocks () { return 1; }
	IParamBlock2* GetParamBlock(int i) { return mp_pblock; }
	IParamBlock2* GetParamBlockByID(BlockID id) { return (mp_pblock->ID() == id) ? mp_pblock : NULL; }

	// Reference Management:
	int NumRefs() {return 3;}
	RefTargetHandle GetReference(int i);

	IOResult Load(ILoad *iload);
	IOResult Save(ISave *isave);
	bool SpecifySaveReferences(ReferenceSaveManager& referenceSaveManager);

	bool mUseRampageWeldMath;

	//MXS Publishing

//published functions		 
		//Function Publishing method (Mixin Interface)
		//******************************
	BaseInterface* GetInterface(Interface_ID id)
	{
		if (id == SYMMETRY_MXS_INTERFACE)
			return (SymmetryMod*)this;
		BaseInterface* pInterface = FPMixinInterface::GetInterface(id);
		if (pInterface != NULL)
		{
			return pInterface;
		}
		// return the GetInterface() of its super class
		return Modifier::GetInterface(id);
	}
	//Function Publishing System
	//Function Map For Mixin Interface
	//*************************************************
	BEGIN_FUNCTION_MAP
		VFN_0(kFn_ResetModifierVersion, fnResetModifierVersion);
		FN_0(kFn_ModifierVersion, TYPE_INT, fnModifierVersion);
		VFN_0(kFn_UpdateWeldThreshold, fnUpdateWeldThreshold);

		PROP_FNS(kFn_GetModifierVersion, fnGetModifierVersion, kFn_SetModifierVersion, fnSetModifierVersion, TYPE_INT);
		PROP_FNS(kFn_GetLegacyWeld, fnGetModifierVersion, kFn_SetLegacyWeld, fnSetModifierVersion, TYPE_BOOL);
		PROP_FNS(kFn_GetMirrorWeldThreshold, fnGetMirrorWeldThreshold, kFn_SetMirrorWeldThreshold, fnSetMirrorWeldThreshold, TYPE_FLOAT);

	END_FUNCTION_MAP

	/* Reset the version of the modifier to latest version */
	void fnResetModifierVersion()
	{
		iLoadVersion = SYMMETRY_LATEST_VERSION;
	}

	int fnModifierVersion()
	{
		return iLoadVersion;
	}

	void fnUpdateWeldThreshold()
	{
		mMirrorWeldThreshold = MIRRORWELD_THRESHOLD_2023_2;
	}

	int fnGetModifierVersion()
	{
		return iLoadVersion;
	}
	void fnSetModifierVersion(int ver)
	{
		if ( (ver >= OLD_SYMMETRY_VERSION) && (ver <= SYMMETRY_VERSION_V300))
			iLoadVersion = ver;
	}

	BOOL fnGetLegacyWeld()
	{
		return mUseRampageWeldMath;
	}
	void fnSetLegacyWeld(BOOL useLegacy)
	{
		mUseRampageWeldMath = useLegacy;
	}

	float fnGetMirrorWeldThreshold()
	{
		return mMirrorWeldThreshold;
	}
	void fnSetMirrorWeldThreshold(float threshold)
	{
		mMirrorWeldThreshold = threshold;
	}

private:
	virtual void SetReference(int i, RefTargetHandle rtarg);
public:
	RefResult NotifyRefChanged( const Interval& changeInt,RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate);

	// Animatable management:
	int NumSubs() {return 2;}
	Animatable* SubAnim(int i) {return (Animatable*)GetReference(i); }
	TSTR SubAnimName(int i, bool localized) override;

	// Object Align Methods
	Matrix3	GetControllerTM(TimeValue t) override
	{
		Interval valid;
		Matrix3  tm;
		mp_mirror->GetValue(t, &tm, valid, CTRL_RELATIVE);
		return tm;
	}

	Matrix3	GetRefObjectTM(TimeValue t) override
	{
		Matrix3 tm;
		INode*	n = mp_pblock->GetINode(kSymNodeRef);
		if (n) {
			tm = n->GetObjectTM(t);
		}
		return tm;
	}

	void	SetControllerPacket(TimeValue t, SetXFormPacket &sxfp) override
	{
		mp_mirror->SetValue(t, &sxfp, TRUE, CTRL_RELATIVE);
	}

	void	Move(TimeValue t, Point3 &p) override
	{
		Matrix3	m;
		Move(t, m, m, p, FALSE);
	}

	bool	HasObject() override
	{
		return (mp_pblock->GetINode(kSymNodeRef) != nullptr);
	}

	void ExitPick() override
	{
		if (mRollup)
			mRollup->ToggleAlignButton();

		// Force re-eval
		NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	}

	void EnterPick() override
	{
		// Force re-eval so we can show only the original object
		NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	}

	void	PreRotate(Matrix3& m) override
	{
		int		axis = -1;
		bool	bPlanar = (mp_pblock->GetInt(kSymFormat) == symmetry_planar);
		if (bPlanar) {
			if (mp_pblock->GetInt(kSymPlanar_X)) {
				axis = X_AXIS;
			}
			else if (mp_pblock->GetInt(kSymPlanar_Y)) {
				axis = Y_AXIS;
			}
			else if (mp_pblock->GetInt(kSymPlanar_Z)) {
				axis = Z_AXIS;
			}
		}
		else {
			// Radial option
			axis = mp_pblock->GetInt(kSymRadialAxis);
			if (axis == Z_AXIS)
				axis = X_AXIS;
			else if (axis == X_AXIS)
				axis = Y_AXIS;
			else
				axis = Z_AXIS;
		}

		if (axis == X_AXIS)
		{
			m.PreRotateY(-HALFPI);
		}
		else if (axis == Y_AXIS)
		{
			m.PreRotateX(HALFPI);
		}
		else if (axis == Z_AXIS)
		{
			m.PreRotateZ(PI);
		}
	};

	// Local methods:
	Matrix3 CompMatrix(TimeValue t, INode *inode, ModContext *mc);
	void SliceTriObject (Mesh & mesh, const Point3 & N, float offset);
	/*
	Mirror the objects along the mirror entry plane
	mesh : the mesh to be mirrored faces that are tagged MN_USER are cloned and mirrored
	me : the mirror plane data
	normalMapChannel : the this is the map channel that has normals stuffed in them.  The symmetry stuff the
	normals into a map channel and then does it operation and then extracts the normals from the map channel 
	back into the normal spec
	originalfaces : where to start copying/mirroring face from
	bFlip : whether to flip the copied mesh faces
	bWeld : whether to weld the vertices along the mirror plane
	bThreshold : the distance from the plane that will cause a vertex to be mirrored to its pair
	vertexPair : is an tab for each vertex with an index back to the source vertex from the mirror
	it will only have a valid value also if the vertex is an open vertex otherwise the value will be -1
	*/
	void MirrorTriObject(Mesh & mesh, const MirrorEntry & me, int normalMapChannel, int originalfaces, bool bFlip, bool bWeld, float threshold, Tab<int>& vertexPairs);

	/*
	Welds vertices near the mirror plane, new algorthym also chackes to make sure they are a matching
	mirror pair
	mesh : the mesh to be mirrored faces that are tagged MN_USER are cloned and mirrored
	N : normal of the plane
	offset : offset from (0,0,0) that determines the plane facing
	threshold : distance from the slice plane that trip a weld
	vertexPair : is an tab for each vertex with an index back to the source vertex from the mirror
	it will only have a valid value also if the vertex is an open vertex otherwise the value will be -1

	*/
	void WeldTriObject (Mesh & mesh, const Point3 & N, float offset, float threshold, const Tab<int>& vertexPairs);
	void ModifyTriObject(TimeValue t, TriObject *tobj, bool bRadial, bool bMirror, BOOL weld, float thresh);
	void DoRadialWeld(Mesh & mesh, int vertexChunkSize, int faceChunkSize, bool bMirror, int iSliceCount, int iMirrorCount, float thresh);
	void WeldSlicedEdge(Mesh & mesh, const SliceEntry& se, float thresh);

	void SlicePolyObject (MNMesh & mesh, Point3 N, float offset);
	/*
	Mirror the objects along the mirror entry plane
	mesh : the mesh to be mirrored faces that are tagged MN_USER are cloned and mirrored
	me : the mirror plane data
	uselegacy : will use the old OLD_SYMMETRY_VERSION of the mirror
	vertexPair : is an tab for each vertex with an index back to the source vertex from the mirror
	it will only have a valid value also if the vertex is an open vertex otherwise the value will be -1
	*/
	void MirrorPolyObject(MNMesh & mesh, const MirrorEntry & me, bool useLegacy, Tab<int>& vertexPairs);
	/*
	Welds an open vertices that lie along the slice plane, the new version will only wield matching pairs
	mesh : the mesh to be mirrored faces that are tagged MN_USER are cloned and mirrored
	N : normal of the plane
	offset : ??offset from (0,0,0) that determines the plane facing
	threshold : distance from the slice plane that trip a weld
	UseProximityThreshold : use the old legacy weld that weld everything
	startVerts : the start vertex index of the mirrored vertices
	vertexPairs  : the data from MirroPolyObject that holds which vertex are mirrored from where
	radial : whether the symmetr is radial or not since it has to be dealt differently since there is a core
	         NOTE this is only for PolyObjects, TriObjects do it differently and do a post weld cleanup
	*/
	bool WeldPolyObject (MNMesh & mesh, const Point3 & N, float offset, float threshold, bool UseProximityThreshold, int startVerts, const Tab<int>& vertexPairs, bool radial);
	/*
	Collapses short edges along the slice plane
	mesh : the mesh to be mirrored faces that are tagged MN_USER are cloned and mirrored
	N : normal of the plane
	offset : ??offset from (0,0,0) that determines the plane facing
	threshold : any edges on the mirror plance less than this will be collapsed
	*/
	bool CollapsePolySliceEdge(MNMesh & mesh, const Point3 & N, float offset, float threshold);
	void RemovePolySpurs (MNMesh & mesh);
	void ModifyPolyObject(TimeValue t, PolyObject *pobj, bool bRadial, bool bMirror, BOOL weld, float thresh);

	bool IsLegacy() { return iLoadVersion == OLD_SYMMETRY_VERSION; }
	/*
	Returns whether to use the new edge weld threshold that uses a percentage of the edge length
	or use the hard coded edge length to determine what edges along the seam to clean up
	*/
	bool UseNewEdgeWeldThreshold() { return iLoadVersion >= SYMMETRY_VERSION_V300; }
	/*
	Returns whether to use the new weld algorithm.  This alogrithm restricts the weld to matching 
	mirrored vertex verus the old algorithm which allowed you to weld to any vertex across the 
	mirror plane
	*/
	bool UseNewWeld() { return iLoadVersion >= SYMMETRY_VERSION_V300; }

	void	DrawPlanarGizmos(GraphicsWindow* gw, const Matrix3& ptm, float size);
	void	DrawRadialGizmos(GraphicsWindow* gw, const Matrix3& ptm, float size, TimeValue t);
	void OnLegacyAxisOrFlip();
	void SetLegacyAxisAndFlip();
	void	AlignToFace();
	void	ClearObject();

	void	ClearGizmoTransform()
	{
		if (mp_mirror != nullptr)
		{
			SetXFormPacket pckt(Matrix3{}, Matrix3{});
			mp_mirror->SetValue(0, &pckt, TRUE, CTRL_RELATIVE);	
		}
		// Force redraw
		if (mp_ip != nullptr) {
			mp_ip->RedrawViews(mp_ip->GetTime());
		}
	}

	SymmetryRollup* mRollup;

	// Versioning for old loads
	int		iLoadVersion;

	// Tables of operations we need to peform
	Tab<SliceEntry> sliceTab;
	Tab<MirrorEntry> mirrorTab;

	// Track our movements in cases where there are reference nodes
	NodeTransformMonitor* transformMonitor;

	//threshold used to weld the center seam
	float mMirrorWeldThreshold = MIRRORWELD_THRESHOLD_2023_2;
};

//--- ClassDescriptor and class vars ---------------------------------

IObjParam       *SymmetryMod::mp_ip        = NULL;
MoveModBoxCMode    *SymmetryMod::mp_moveMode    = NULL;
RotateModBoxCMode  *SymmetryMod::mp_rotMode 	   = NULL;
UScaleModBoxCMode *SymmetryMod::mp_scaleMode = NULL;
NUScaleModBoxCMode *SymmetryMod::mp_nuScaleMode = NULL;
SquashModBoxCMode *SymmetryMod::mp_squashMode = NULL;

FaceAlignMode*      SymmetryMod::faceAlignMode = nullptr;



class SymmetryClassDesc : public ClassDesc2 {
public:
	int 			IsPublic() { return 1; }
	void*			Create(BOOL loading = FALSE);
	const TCHAR*	ClassName() { return GetString(IDS_SYMMETRY_MOD); }
	const TCHAR*	NonLocalizedClassName() { return _T("Symmetry"); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return kSYMMETRY_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_MAX_STANDARD);}

	// The following are used by MAX Script and the schematic view:
	const TCHAR* InternalName() { return _T("Symmetry"); }
	HINSTANCE HInstance() { return hInstance; }

	virtual MaxSDK::QMaxParamBlockWidget* CreateQtWidget(ReferenceMaker& owner, IParamBlock2& paramBlock, const MapID, MSTR& rollupTitle, int&, int&) override
	{
		if (paramBlock.ID() == kSYM_PBLOCK_REF) {
			SymmetryRollup*rr = new SymmetryRollup();
			((SymmetryMod&)owner).mRollup = rr;
			rr->SetParamBlock(&owner, &paramBlock);
			//: this is a comment for the translation team 
			rollupTitle = SymmetryRollup::tr("Symmetry");
			return rr;
		}
		return nullptr;
	}
};

static SymmetryClassDesc symDesc;
extern ClassDesc* GetSymmetryModDesc() {return &symDesc;}

// ParamBlock2: Enumerate the parameter blocks:
enum { kSymmetryParams };


// --- QT Setup ----------------------------

SymmetryRollup::SymmetryRollup(QWidget* /*parent*/) :
	QMaxParamBlockWidget(),
	m_UI(new Ui::SymmetryRollup())
{
	mMod = nullptr;
	m_UI->setupUi(this);

	// Set up Slice Format
	if (m_UI->SymmetryFormat)
	{
		m_UI->SymmetryFormat->clear();
		m_UI->SymmetryFormat->addItem(tr("Planar"), QVariant(SymmetryMod::symmetry_planar));
		m_UI->SymmetryFormat->addItem(tr("Radial"), QVariant(SymmetryMod::symmetry_radial));
	}

	// NOTE: Icons below are supplied via the 3ds Max shared icon resources.
	// For an example of using customized icons, see the "qtIconsDemo" project in the
	// Max SDK "howto/UI" sample.

	// Setup planar axis buttons
	m_UI->PlanarX->setIcon(MaxSDK::LoadMaxMultiResIcon(MSTR(_T("EditUVW\\AlignToX.png"))));
	m_UI->PlanarY->setIcon(MaxSDK::LoadMaxMultiResIcon(MSTR(_T("EditUVW\\AlignToY.png"))));
	m_UI->PlanarZ->setIcon(MaxSDK::LoadMaxMultiResIcon(MSTR(_T("EditUVW\\AlignToZ.png"))));

	// Setup radial axis buttons
	m_UI->RadialX->setIcon(MaxSDK::LoadMaxMultiResIcon(MSTR(_T("EditUVW\\AlignToX.png"))));
	m_UI->RadialY->setIcon(MaxSDK::LoadMaxMultiResIcon(MSTR(_T("EditUVW\\AlignToY.png"))));
	m_UI->RadialZ->setIcon(MaxSDK::LoadMaxMultiResIcon(MSTR(_T("EditUVW\\AlignToZ.png"))));

	// We need a custom right-click menu for the face picker (to reset the transform)
	m_UI->AlignToFace->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_UI->AlignToFace, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(AlignRightClick(const QPoint&)));
	mActionReset = new QAction(tr("Reset"), this);
	mActionReset->setToolTip(tr("Reset transform"));
	// Setup Align to Face button
	connect(m_UI->AlignToFace, SIGNAL(clicked()), this, SLOT(AlignToFace()));
	connect(m_UI->ResetTransform, SIGNAL(clicked()), this, SLOT(ResetTransform()));
	m_UI->ResetTransform->setIcon(MaxSDK::LoadMaxMultiResIcon(MSTR(_T("StateSets\\Refresh.png"))));

	// We need a custom right-click menu for object picker
	m_UI->ReferenceObject->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_UI->ReferenceObject, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(ObjectRightClick(const QPoint&)));
	mActionClear = new QAction(tr("Clear"), this);
	mActionClear->setToolTip(tr("Clear the reference object"));

	// Set up Radial Axis Enum
	if (m_UI->RadialAxis)
	{
		m_UI->RadialAxis->setId(m_UI->RadialX, X_AXIS);
		m_UI->RadialAxis->setId(m_UI->RadialY, Y_AXIS);
		m_UI->RadialAxis->setId(m_UI->RadialZ, Z_AXIS);
	}
}

SymmetryRollup::~SymmetryRollup(void)
{
	if (mMod)
		mMod->mRollup = nullptr;
	delete m_UI;
}

void SymmetryRollup::SetParamBlock(ReferenceMaker* owner, IParamBlock2* const /*param_block*/)
{
	mMod = (SymmetryMod*)owner;
}

void SymmetryRollup::UpdateUI(const TimeValue t)
{
	EnableControls(t);
}

void SymmetryRollup::UpdateParameterUI(const TimeValue t, const ParamID param_id, const int /*tab_index*/)
{
	switch (param_id)
	{
	case SymmetryMod::kSymWeld:
	case SymmetryMod::kSymFormat:
	case SymmetryMod::kSymRadialMirror:
	case SymmetryMod::kSymPlanar_X:
	case SymmetryMod::kSymPlanar_Y:
	case SymmetryMod::kSymPlanar_Z:
	case SymmetryMod::kSymProximityCheck:
	case SymmetryMod::kSymSlice:
		EnableControls(t);
		break;
	case SymmetryMod::kSymNodeRef:
		UpdatePickButton();
		break;
	}
}

void SymmetryRollup::EnableControls(const TimeValue t)
{
	if (!mMod || !(mMod->mp_pblock))
		return;

	// Update UI based on Slice Format
	bool bRadial = mMod->mp_pblock->GetInt(SymmetryMod::kSymFormat) == SymmetryMod::symmetry_radial;
	// Planar Visibility
	m_UI->PlanarX->setHidden(bRadial);
	m_UI->PlanarFlipX->setHidden(bRadial);
	m_UI->PlanarY->setHidden(bRadial);
	m_UI->PlanarFlipY->setHidden(bRadial);
	m_UI->PlanarZ->setHidden(bRadial);
	m_UI->PlanarFlipZ->setHidden(bRadial);

	// Radial Visibility
	m_UI->RadialX->setHidden(!bRadial);
	m_UI->RadialY->setHidden(!bRadial);
	m_UI->RadialZ->setHidden(!bRadial);
	m_UI->RadialFlip->setHidden(!bRadial);
	m_UI->countL->setHidden(!bRadial);
	m_UI->RadialCount->setHidden(!bRadial);
	m_UI->RadialMirror->setHidden(!bRadial);

	// Enable/Disable Controls
	if (bRadial)
	{
		bool mirrorEnabled = mMod->mp_pblock->GetInt(SymmetryMod::kSymRadialMirror, t) != 0;
		m_UI->RadialFlip->setEnabled(mirrorEnabled);
	}
	else {
		bool	bSliceX = (mMod->mp_pblock->GetInt(SymmetryMod::kSymPlanar_X) != 0);
		bool	bSliceY = (mMod->mp_pblock->GetInt(SymmetryMod::kSymPlanar_Y) != 0);
		bool	bSliceZ = (mMod->mp_pblock->GetInt(SymmetryMod::kSymPlanar_Z) != 0);
		m_UI->PlanarFlipX->setEnabled(bSliceX);
		m_UI->PlanarFlipY->setEnabled(bSliceY);
		m_UI->PlanarFlipZ->setEnabled(bSliceZ);
	}

	// Slice along mirror
	bool sliceAlongMirror = mMod->mp_pblock->GetInt(SymmetryMod::kSymSlice) != 0;
	m_UI->EdgeCollapsePercent->setEnabled(sliceAlongMirror);
	m_UI->labelSliceth->setEnabled(sliceAlongMirror);

	// Weld Threshold
	bool weldEnabled = mMod->mp_pblock->GetInt(SymmetryMod::kSymWeld, t) != 0;
	
	m_UI->threshold->setEnabled(weldEnabled);
	m_UI->labelth->setEnabled(weldEnabled);
	m_UI->UseProximityThreshold->setEnabled(weldEnabled);

	UpdatePickButton();
}

void SymmetryRollup::UpdatePickButton()
{
	if (mMod && !mMod->HasObject())
	{
		m_UI->ReferenceObject->setText(tr("Pick Object"));
	}
}

void SymmetryRollup::AlignToFace()
{
	if (mMod)
		mMod->AlignToFace();
}

void SymmetryRollup::ResetTransform()
{
	if (mMod)
		mMod->ClearObject();
}

void SymmetryRollup::ToggleAlignButton()
{
	m_UI->AlignToFace->setChecked(false);
}

void	SymmetryRollup::ObjectRightClick(const QPoint & pos)
{
	QMenu menu;
	menu.addAction(mActionClear);

	QAction* selectedItem = menu.exec(m_UI->ReferenceObject->mapToGlobal(pos));

	if (selectedItem == mActionClear)
	{
		mMod->ClearObject();
	}
}

void	SymmetryRollup::AlignRightClick(const QPoint & pos)
{
	QMenu menu;
	menu.addAction(mActionReset);

	QAction* selectedItem = menu.exec(m_UI->AlignToFace->mapToGlobal(pos));

	if (selectedItem == mActionReset)
	{
		mMod->ClearGizmoTransform();
	}
}

//--- Parameter map/block descriptors -------------------------------


class SymmetryPBAccessor : PBAccessor
{
	virtual void Set(PB2Value& /*v*/, ReferenceMaker* owner, ParamID id, int /*tabIndex*/, TimeValue /*t*/) override
	{
		if (id == SymmetryMod::kSymNodeRef)
		{
			SymmetryMod*	m = (SymmetryMod*)owner;
			if (m)
				m->ClearGizmoTransform();

		} else{
			Interface* ip = GetCOREInterface();
			ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
		}
	}
};
static SymmetryPBAccessor s_accessor;

// Parameters
static ParamBlockDesc2 symmetry_param_blk (kSymmetryParams, _T("Symmetry Parameters"), 0, &symDesc,
											   P_AUTO_CONSTRUCT + P_AUTO_UI_QT, kSYM_PBLOCK_REF,
	// Parameters
	SymmetryMod::kSymAxis, _T("Axis"), TYPE_INT, P_RESET_DEFAULT, IDS_SYM_AXIS,
		p_default, X_AXIS,
		p_range,	X_AXIS, Z_AXIS,
		p_accessor, &s_accessor,
		p_end,

	SymmetryMod::kSymFlip, _T("flip"), TYPE_BOOL, P_RESET_DEFAULT, IDS_SYM_FLIP,
		p_default, 0,
		p_accessor, &s_accessor,
		p_end,

	SymmetryMod::kSymSlice, _T("slice"), TYPE_INT, P_RESET_DEFAULT, IDS_SYM_SLICE,
		p_default, 1,
		p_accessor, &s_accessor,
		p_end,

	SymmetryMod::kSymWeld, _T("weld"), TYPE_INT, P_RESET_DEFAULT, IDS_SYM_WELD,
		p_default, 1,
		p_accessor, &s_accessor,
		p_end,

	SymmetryMod::kSymThreshold, _T("threshold"), TYPE_FLOAT, P_RESET_DEFAULT|P_ANIMATABLE, IDS_SYM_THRESHOLD,
		p_default, 0.01f,
		p_range, 0.0f, BIGFLOAT,
		p_accessor, &s_accessor,
		p_end,

	// New features
	SymmetryMod::kSymFormat, _T("SymmetryFormat"), TYPE_INT, P_RESET_DEFAULT, IDS_SYMMETRY_FORMAT,
	p_default, SymmetryMod::symmetry_planar,
	p_range, SymmetryMod::symmetry_planar, SymmetryMod::symmetry_radial,
	p_accessor, &s_accessor,
	p_end,

	// Planar Options
	SymmetryMod::kSymPlanar_X, _T("PlanarX"), TYPE_BOOL, P_RESET_DEFAULT, IDS_PLANARX,
	p_default, TRUE,
	p_accessor, &s_accessor,
	p_end,

	SymmetryMod::kSymPlanar_Y, _T("PlanarY"), TYPE_BOOL, P_RESET_DEFAULT, IDS_PLANARY,
	p_default, FALSE,
	p_accessor, &s_accessor,
	p_end,

	SymmetryMod::kSymPlanar_Z, _T("PlanarZ"), TYPE_BOOL, P_RESET_DEFAULT, IDS_PLANARZ,
	p_default, FALSE,
	p_accessor, &s_accessor,
	p_end,

	SymmetryMod::kSymPlanarFlip_X, _T("PlanarFlipX"), TYPE_BOOL, P_RESET_DEFAULT, IDS_PLANAR_FLIPX,
	p_default, FALSE,
	p_accessor, &s_accessor,
	p_end,

	SymmetryMod::kSymPlanarFlip_Y, _T("PlanarFlipY"), TYPE_BOOL, P_RESET_DEFAULT, IDS_PLANAR_FLIPY,
	p_default, FALSE,
	p_accessor, &s_accessor,
	p_end,

	SymmetryMod::kSymPlanarFlip_Z, _T("PlanarFlipZ"), TYPE_BOOL, P_RESET_DEFAULT, IDS_PLANAR_FLIPZ,
	p_default, FALSE,
	p_accessor, &s_accessor,
	p_end,

	// Radial Options
	SymmetryMod::kSymRadialAxis, _T("RadialAxis"), TYPE_INT, P_RESET_DEFAULT, IDS_RADIAL_AXIS,
	p_default, Z_AXIS,
	p_range, X_AXIS, Z_AXIS,
	p_accessor, &s_accessor,
	p_end,

	SymmetryMod::kSymRadialFlip, _T("RadialFlip"), TYPE_BOOL, P_RESET_DEFAULT, IDS_RADIAL_FLIP,
	p_default, FALSE,
	p_accessor, &s_accessor,
	p_end,

	SymmetryMod::kSymRadialCount, _T("RadialCount"), TYPE_INT, P_RESET_DEFAULT | P_ANIMATABLE, IDS_RADIAL_COUNT,
	p_default, 2,
	p_range, 2, 1000,
	p_accessor, &s_accessor,
	p_end,

	SymmetryMod::kSymRadialMirror, _T("RadialMirror"), TYPE_BOOL, P_RESET_DEFAULT, IDS_RADIAL_MIRROR,
	p_default, FALSE,
	p_accessor, &s_accessor,
	p_end,

	// Object Reference
	SymmetryMod::kSymNodeRef, _T("ReferenceObject"), TYPE_INODE, P_AUTO_UI, IDS_REFERENCE_OBJECT,
	p_accessor, &s_accessor,
	p_end,

	SymmetryMod::kSymProximityCheck, _T("UseProximityThreshold"), TYPE_BOOL, P_RESET_DEFAULT | P_ANIMATABLE, IDS_SYM_PROXIMITY_ACTIVE,
	p_default, FALSE,
	p_accessor, &s_accessor,
	p_end,

	SymmetryMod::kEdgeCollapsePercent, _T("EdgeCollapsePercent"), TYPE_FLOAT, P_RESET_DEFAULT , IDS_EDGE_COLLAPSE_PERCENT, 
	p_default, 0.01f,
	p_range, 0.0f, BIGFLOAT, 
	p_accessor, &s_accessor,
	p_end,												   

	p_end
);

static FPInterfaceDesc symmetry_interface(
	SYMMETRY_MXS_INTERFACE, _T("symmetryOps"), 0, &symDesc, FP_MIXIN,

	SymmetryMod::kFn_ResetModifierVersion, _T("resetVersion"), 0, TYPE_VOID, 0, 0,
	SymmetryMod::kFn_ModifierVersion, _T("version"), 0, TYPE_INT, 0, 0,

	SymmetryMod::kFn_UpdateWeldThreshold, _T("updateWeldThreshold"), 0, TYPE_VOID, 0, 0,


	properties,
	SymmetryMod::kFn_GetModifierVersion, SymmetryMod::kFn_SetModifierVersion, _T("symmetry_version"), 0, TYPE_INT,
	SymmetryMod::kFn_GetLegacyWeld, SymmetryMod::kFn_SetLegacyWeld, _T("legacyWeld"), 0, TYPE_BOOL,
	SymmetryMod::kFn_GetMirrorWeldThreshold, SymmetryMod::kFn_SetMirrorWeldThreshold, _T("mirrorWeldThreshold"), 0, TYPE_FLOAT,

	p_end
);

void *SymmetryClassDesc::Create(BOOL loading)
{
	AddInterface(&symmetry_interface);
	return new SymmetryMod;
}

//--- SymmetryMod methods -------------------------------


SymmetryMod::SymmetryMod(BOOL bLoading) :
	mp_pblock(nullptr),
	mp_mirror(nullptr),
	mRollup(nullptr),
	transformMonitor(nullptr)
{
	mUseRampageWeldMath = false;
	symDesc.MakeAutoParamBlocks(this);
	if (!bLoading) {
		iLoadVersion = SYMMETRY_LATEST_VERSION;
		ReplaceReference(kSYM_MIRROR_REF, NewDefaultMatrix3Controller());
	}
}

FPInterfaceDesc* SymmetryMod::GetDesc()
{
	return &symmetry_interface;
}



void SymmetryMod::BeginEditParams (IObjParam  *ip, ULONG flags,Animatable *prev) {
	mp_ip = ip;

	// Create sub object editing modes.
	mp_moveMode    = new MoveModBoxCMode(this,ip);
	mp_rotMode     = new RotateModBoxCMode(this,ip);
	mp_scaleMode = new UScaleModBoxCMode (this, ip);
	mp_nuScaleMode = new NUScaleModBoxCMode (this, ip);
	mp_squashMode = new SquashModBoxCMode (this, ip);
	faceAlignMode = new FaceAlignMode(this, this, ip);

	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);

	// For PB2 we ask the ClassDesc2 to take care of the BeginEditParams
	symDesc.BeginEditParams(ip,this,flags,prev);
}

void SymmetryMod::EndEditParams (IObjParam *ip,ULONG flags,Animatable *next) {
	mp_ip = NULL;

	TimeValue t = ip->GetTime();
	// NOTE: This flag must be cleared before sending the REFMSG_END_EDIT
	ClearAFlag(A_MOD_BEING_EDITED);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);

	// Eliminate our command modes.
	ip->DeleteMode(mp_moveMode);
	ip->DeleteMode(mp_rotMode);
	ip->DeleteMode (mp_scaleMode);
	ip->DeleteMode (mp_nuScaleMode);
	ip->DeleteMode(mp_squashMode);
	ip->DeleteMode(faceAlignMode);
	if ( mp_moveMode ) delete mp_moveMode;
	mp_moveMode = NULL;
	if ( mp_rotMode ) delete mp_rotMode;
	mp_rotMode = NULL;
	if ( mp_scaleMode ) delete mp_scaleMode;
	mp_scaleMode = NULL;
	if ( mp_nuScaleMode ) delete mp_nuScaleMode;
	mp_nuScaleMode = NULL;
	if ( mp_squashMode ) delete mp_squashMode;
	mp_squashMode = NULL;
	if (faceAlignMode) delete faceAlignMode;
	faceAlignMode = nullptr;

	// For PB2 we ask the ClassDesc2 to take care of the EndEditParams - NH
	symDesc.EndEditParams(ip,this,flags,next);
}

RefTargetHandle SymmetryMod::Clone(RemapDir& remap) {
	SymmetryMod *mod = new SymmetryMod();
	//copy local non pb2 data
	mod->mUseRampageWeldMath = mUseRampageWeldMath;
	mod->iLoadVersion = iLoadVersion;
	mod->mMirrorWeldThreshold = mMirrorWeldThreshold;

	mod->ReplaceReference(kSYM_PBLOCK_REF,remap.CloneRef(mp_pblock));
	mod->ReplaceReference(kSYM_MIRROR_REF,remap.CloneRef(mp_mirror));
	mod->ReplaceReference(NODE_TRANSFORM_MONITOR_REF, remap.CloneRef(transformMonitor));
	BaseClone(this, mod, remap);
	return mod;
}

void SymmetryMod::ModifyObject (TimeValue t, ModContext &mc, ObjectState *os, INode *inode)
{
	// Don't modify while aligning to the Face so we actually see the original object
	if (faceAlignMode && faceAlignMode->bActive)
		return;

	Interval incomingGeoIV;
	incomingGeoIV.SetInfinite();
	if (os->obj != nullptr)
	{
		//need to store incoming geo interval since this may affect the topo interval at the end
		incomingGeoIV = os->obj->ChannelValidity(t, GEOM_CHAN_NUM);
	}



	Interval iv = FOREVER;
	int slice, weld;
	float thresh;

	mp_pblock->GetValue(kSymSlice, t, slice, iv);
	mp_pblock->GetValue(kSymWeld, t, weld, iv);
	mp_pblock->GetValue(kSymThreshold, t, thresh, iv);
	if (thresh < 0.0f)
		thresh = 0.0f;

	// Symmetry Format
	int		iSymmetryFormat = mp_pblock->GetInt(kSymFormat);
	if (iSymmetryFormat != symmetry_radial)
		iSymmetryFormat = symmetry_planar;

	// Get transform from mp_mirror controller:
	Matrix3 tm;

	INode*	objRef = mp_pblock->GetINode(kSymNodeRef);
	if (objRef)
	{
		tm = objRef->GetObjectTM(t, &iv); // *objectTM; // refTM;  *objectTM;

		// Get ourself
		int which = 0;

		INode*	thisNode = GetNodeFromModContext(&mc, which);
		if (thisNode)
			tm *= Inverse(thisNode->GetObjectTM(t, &iv));

		// Add a monitor for the transform for future use
		if (!transformMonitor)
		{
			// Create a Transform Monitor
			const auto refTransformMonitor = static_cast<ReferenceTarget*>(CreateInstance(REF_TARGET_CLASS_ID, NODETRANSFORMMONITOR_CLASS_ID));
			DbgAssert(refTransformMonitor);

			// Assign the transform monitor reference
			ReplaceReference(NODE_TRANSFORM_MONITOR_REF, refTransformMonitor);
		}

		if (transformMonitor && (transformMonitor->GetNode() != thisNode))
		{
			transformMonitor->SetNode(thisNode);
			transformMonitor->SetForwardTransformChangeMsgs(true);
		}
	}

	mp_mirror->GetValue(t, &tm, iv, CTRL_RELATIVE);
	if (mc.tm)
		tm = tm * (Inverse(*mc.tm));

	Point3 origin = tm.GetTrans();

	sliceTab.ZeroCount();
	mirrorTab.ZeroCount();

	bool bMirror = false;

	if (iSymmetryFormat == symmetry_radial)
	{
		int		radialCount = 2;
		mp_pblock->GetValue(kSymRadialCount, t, radialCount, iv);
		int	iAxis = mp_pblock->GetInt(kSymRadialAxis);
		bool bFlip = (mp_pblock->GetInt(kSymRadialFlip) != 0);
		bMirror = (mp_pblock->GetInt(kSymRadialMirror) != 0);

		float	fAngle = TWOPI / radialCount;
		float	fHalfAngle = fAngle * 0.5f;

		if (iAxis == X_AXIS)
		{
			// Mirror down the center first
			if (bMirror)
			{
				Point3 Axis(0.0f, bFlip ? -1.0f : 1.0f, 0.0f);
				Point3 N = Normalize(tm*Axis - origin);
				float off = DotProd(N, origin);
				MirrorEntry	me(tm, Y_AXIS, N, off);
				mirrorTab.Append(1, &me);

				SliceEntry	se(N, off);
				sliceTab.Append(1, &se);
			}

			// Radial mirror
			for (int i = 1; i < radialCount; i++)
			{
				Matrix3 m1 = tm;
				m1.PreRotateX(fAngle*i);
				Point3 N = Normalize(m1*Point3::YAxis - origin);
				float off = DotProd(N, origin);
				MirrorEntry	me(tm, m1, N, off);
				mirrorTab.Append(1, &me);
			}

			// We need to slice half on either side
			if (slice)
			{
				Matrix3 m1 = tm;
				m1.PreRotateX(-fHalfAngle);
				Point3 N = -Normalize(m1*Point3::YAxis - origin);
				float off = DotProd(N, origin);

				SliceEntry	se(N, off);
				sliceTab.Append(1, &se);

				m1 = tm;
				m1.PreRotateX(fHalfAngle); // +HALFPI);
				N = Normalize(m1*Point3::YAxis - origin);
				off = DotProd(N, origin);

				SliceEntry	se2(N, off);
				sliceTab.Append(1, &se2);
			}
		}
		else if (iAxis == Y_AXIS)
		{
			// Mirror down the center first
			if (bMirror)
			{
				Point3 Axis(0.0f, 0.0f, bFlip ? -1.0f : 1.0f);
				Point3 N = Normalize(tm*Axis - origin);
				float off = DotProd(N, origin);
				MirrorEntry	me(tm, Z_AXIS, N, off);
				mirrorTab.Append(1, &me);

				SliceEntry	se(N, off);
				sliceTab.Append(1, &se);
			}

			// Radial mirror
			for (int i = 1; i < radialCount; i++)
			{
				Matrix3 m1 = tm;
				m1.PreRotateY(fAngle*i);
				Point3 N = Normalize(m1*Point3::ZAxis - origin);
				float off = DotProd(N, origin);
				MirrorEntry	me(tm, m1, N, off);
				mirrorTab.Append(1, &me);
			}

			// We need to slice half on either side
			if (slice)
			{
				Matrix3 m1 = tm;
				m1.PreRotateY(-fHalfAngle);
				Point3 N = -Normalize(m1*Point3::ZAxis - origin);
				float off = DotProd(N, origin);

				SliceEntry	se(N, off);
				sliceTab.Append(1, &se);

				m1 = tm;
				m1.PreRotateY(fHalfAngle); // +HALFPI);
				N = Normalize(m1*Point3::ZAxis - origin);
				off = DotProd(N, origin);

				SliceEntry	se2(N, off);
				sliceTab.Append(1, &se2);
			}
		}
		else // Z Axis
		{
			// Mirror down the center first
			if (bMirror)
			{
				Point3 Axis(bFlip ? -1.0f : 1.0f, 0.0f,  0.0f);
				Point3 N = Normalize(tm*Axis - origin);
				float off = DotProd(N, origin);
				MirrorEntry	me(tm, X_AXIS, N, off);
				mirrorTab.Append(1, &me);

				SliceEntry	se(N, off);
				sliceTab.Append(1, &se);
			}

			// Radial mirror
			for (int i = 1; i < radialCount; i++)
			{
				Matrix3 m1 = tm;
				m1.PreRotateZ(fAngle*i);
				Point3 N = Normalize(m1*Point3::XAxis - origin);
				float off = DotProd(N, origin);
				MirrorEntry	me(tm, m1, N, off);
				mirrorTab.Append(1, &me);
			}

			// We need to slice half on either side
			if (slice)
			{
				Matrix3 m1 = tm;
				m1.PreRotateZ(-fHalfAngle);
				Point3 N = -Normalize(m1*Point3::XAxis - origin);
				float off = DotProd(N, origin);

				SliceEntry	se(N, off);
				sliceTab.Append(1, &se);

				m1 = tm;
				m1.PreRotateZ(fHalfAngle); // +HALFPI);
				N = Normalize(m1*Point3::XAxis - origin);
				off = DotProd(N, origin);

				SliceEntry	se2(N, off);
				sliceTab.Append(1, &se2);
			}
		}
	}
	else {
		// Axis selection for Planar
		bool bSymmX = (mp_pblock->GetInt(kSymPlanar_X) != 0);
		bool bSymmY = (mp_pblock->GetInt(kSymPlanar_Y) != 0);
		bool bSymmZ = (mp_pblock->GetInt(kSymPlanar_Z) != 0);
		
		if (bSymmX)
		{
			bool bFlip = (mp_pblock->GetInt(kSymPlanarFlip_X) != 0);
			Point3 Axis(bFlip ? -1.0f : 1.0f, 0.0f, 0.0f);
			Point3 N = Normalize(tm*Axis - origin);
			float off = DotProd(N, origin);
			MirrorEntry	me(tm, X_AXIS, N, off);
			mirrorTab.Append(1, &me);

			if (slice)
			{
				SliceEntry	se(N, off);
				sliceTab.Append(1, &se);
			}
		}
		if (bSymmY)
		{
			bool bFlip = (mp_pblock->GetInt(kSymPlanarFlip_Y) != 0);
			Point3 Axis(0.0f, bFlip ? -1.0f : 1.0f, 0.0f);
			Point3 N = Normalize(tm*Axis - origin);
			float off = DotProd(N, origin);

			MirrorEntry	me(tm, Y_AXIS, N, off);
			mirrorTab.Append(1, &me);

			if (slice)
			{
				SliceEntry	se(N, off);
				sliceTab.Append(1, &se);
			}
		}
		if (bSymmZ)
		{
			bool bFlip = (mp_pblock->GetInt(kSymPlanarFlip_Z) != 0);
			Point3 Axis(0.0f, 0.0f, bFlip ? -1.0f : 1.0f);
			Point3 N = Normalize(tm*Axis - origin);
			float off = DotProd(N, origin);

			MirrorEntry	me(tm, Z_AXIS, N, off);
			mirrorTab.Append(1, &me);
			if (slice)
			{
				SliceEntry	se(N, off);
				sliceTab.Append(1, &se);
			}
		}
	}

	// Modify the object if there is work to do
	if (sliceTab.Count() || mirrorTab.Count())
	{
		// Apply these transforms depending on object type
		if (os->obj->IsSubClassOf(triObjectClassID))
		{
			ModifyTriObject(t, (TriObject *)os->obj, (iSymmetryFormat == symmetry_radial), bMirror, weld, thresh);
		}
		else {
			if (os->obj->IsSubClassOf(polyObjectClassID))
			{
				ModifyPolyObject(t, (PolyObject *)os->obj, (iSymmetryFormat == symmetry_radial), bMirror, weld, thresh);
			}
			else {
				if (os->obj->CanConvertToType(triObjectClassID)) {
					TriObject *tobj = (TriObject *)os->obj->ConvertToType(t, triObjectClassID);
					ModifyTriObject(t, tobj, (iSymmetryFormat == symmetry_radial), bMirror, weld, thresh);
					os->obj = (Object *)tobj;
				}
			}
		}
	}

	os->obj->UpdateValidity(GEOM_CHAN_NUM, iv);
	//This is tricky since an incoming geo change can change the topo channel even though no symmetry parameters
	//directly change the interval.  For example if a modifier below symmetry that only changes the geo channel
	//pushes a vertex across the mirror plane that will change the topo but the topo interval will not be correct 
	//if you just use just the iv interval.  So you need to look at the incoming geo interval and intersect that
	//with your modifiers interval
	incomingGeoIV = incomingGeoIV & iv;

	os->obj->UpdateValidity(TOPO_CHAN_NUM, incomingGeoIV);
	os->obj->UpdateValidity(VERT_COLOR_CHAN_NUM, incomingGeoIV);
	os->obj->UpdateValidity(TEXMAP_CHAN_NUM, incomingGeoIV);
	os->obj->UpdateValidity(SELECT_CHAN_NUM, incomingGeoIV);
}

void SymmetryMod::SlicePolyObject (MNMesh & mesh, Point3 N, float offset) {
	// Steve Anderson 9/14/2002
	// Using the new "MN_MESH_TEMP_1" flag to override Slice selection behavior,
	// which is undesirable here.
	mesh.SetFlag (MN_MESH_TEMP_1);
	// Slice off everything below the plane:
	mesh.Slice (N, offset, MNEPS, false, true, false, MN_SEL, TRIANGULATION_LEGACY);
	mesh.ClearFlag (MN_MESH_TEMP_1);

	// Make sure we have a valid edge list:
	if (!mesh.GetFlag (MN_MESH_FILLED_IN)) mesh.FillInMesh();

	// Mark the vertices on the plane boundary:
	mesh.ClearVFlags (MN_USER);
	for (int i=0; i<mesh.numv; i++) {
		if (mesh.v[i].GetFlag (MN_DEAD)) continue;
		float dist = DotProd (N, mesh.P(i)) - offset;
		if (fabsf(dist) > MNEPS) continue;
		mesh.v[i].SetFlag (MN_USER);
	}

	// Strip out faces on the mirror plane:  (These aren't always removed by slice.)
	// Find the faces that use only mirror-plane vertices:
	mesh.ClearFFlags (MN_USER);
	mesh.PropegateComponentFlags (MNM_SL_FACE, MN_USER,
		MNM_SL_VERTEX, MN_USER, true);
	mesh.DeleteFlaggedFaces (MN_USER);

	// Clear out dead components:
	mesh.CollapseDeadStructs ();
}

void SymmetryMod::MirrorPolyObject (MNMesh & mesh, const MirrorEntry & me, bool useLegacy, Tab<int>& vertexPairs)
{
	// Make the mirror copy of the faces marked as MN_USER
	int oldnumv = mesh.numv;
	int oldnumf = mesh.numf;

	//first we need to store off a tab of all the faces we are going to clone
	Tab<int> facesToBeCopied;
	facesToBeCopied.Resize(mesh.numf);
	//loop thru the faces
	for (int i = 0; i < mesh.numf; ++i)
	{
		const MNFace& face = mesh.f[i];
		//any face that is not dead and tagged throw the index into our tab
		if (face.GetFlag(MN_DEAD)) continue;
		if (face.GetFlag(MN_USER))
		{
			facesToBeCopied.Append(1, &i);
		}
	}

	//clone the faces
	mesh.CloneFaces(MN_USER, true);

	//build our pair list, we have to use faces to compute this since the vertex order of the
	//cloned faces may not match the order of the source
	//The face order though does match
	if (UseNewWeld())
	{
		//we are using edges list so need to make sure this is filled in
		if (!mesh.GetFlag(MN_MESH_FILLED_IN))
			mesh.FillInMesh();
		//create a pair matching for our mirrored vertices fill out with -1 first
		//than fill in the pairs
		vertexPairs.SetCount(mesh.numv);
		for (int& pair : vertexPairs)
		{
			pair = -1;
		}

		//loop thru our to be copied faces and find the copied face
		for (int sourceFaceIndex = 0; sourceFaceIndex < facesToBeCopied.Count(); ++sourceFaceIndex)
			{
				int copiedFaceIndex = oldnumf + sourceFaceIndex;
				const MNFace& sourceFace = mesh.f[sourceFaceIndex];
				const MNFace& copiedFace = mesh.f[copiedFaceIndex];
				//sanity check if not something has gone horribly wrong
				if (sourceFace.deg == copiedFace.deg)
				{
					for (int corner = 0; corner < sourceFace.deg; ++corner)
					{
						//see if this is an open vertex
						int prevCorner = corner - 1;
						if (prevCorner < 0)
							prevCorner = sourceFace.deg - 1;
						//get incoming and out going edge at this corner
						int currentEdgeIndex = sourceFace.edg[corner];
						int prevEdgeIndex = sourceFace.edg[prevCorner];

						const MNEdge& cEdge = mesh.e[currentEdgeIndex];
						const MNEdge& pEdge = mesh.e[prevEdgeIndex];

						//one of the eedges must be open since we only want to weld open vertices
						if (((cEdge.f1 == -1) || (cEdge.f2 == -1)) ||
							((pEdge.f1 == -1) || (pEdge.f2 == -1)))
						{
							//if it is open store it as a potential weld target with it mirrored vertex
							vertexPairs[copiedFace.vtx[corner]] = sourceFace.vtx[corner];
						}


					}
				}
				else
				{
					DbgAssert(0);
				}

			
		}
	}
	// Transform the vertices to their mirror images:
	if (useLegacy) {	// Original method to ensure same output
		for (int i = oldnumv; i < mesh.numv; i++)
			mesh.v[i].p = (me.legacyInvTM*mesh.v[i].p)*me.legacyTM;
	} else {
		// Transform the vertices to their mirror images:
		for (int i = oldnumv; i < mesh.numv; i++) {
			mesh.v[i].p = me.tm*mesh.v[i].p;			
		}
	}
}

// After a slice, we might have slivers of faces which are not actually on the "border" of the slice, find these
// instances and collapse
bool SymmetryMod::CollapsePolySliceEdge(MNMesh & mesh, const Point3 & N, float offset, float thresh)
{	
	// Mark the vertices within the welding threshold of the plane
	mesh.ClearVFlags(MN_USER);
	for (int i = 0; i < mesh.numv; i++) {
		if (mesh.v[i].GetFlag(MN_DEAD)) continue;
		float dist = DotProd(N, mesh.P(i)) - offset;
		if (fabsf(dist) > thresh) continue;
		mesh.v[i].SetFlag(MN_USER);
	}

	//the incoming thresh was hard coded to an epsilon
	//the new method now ignores that and uses a percentage of 
	//of the edges along the slice plane instead
	if (UseNewEdgeWeldThreshold())
	{		
		double totalEdgeLen = 0.0;
		size_t totalEdgeCount = 0;
		//find the average length of edges along the slice plane
		for (int i = 0; i < mesh.nume; i++) {
			if (mesh.e[i].GetFlag(MN_DEAD)) continue;
			if (!mesh.v[mesh.e[i].v1].GetFlag(MN_USER)) continue;
			if (!mesh.v[mesh.e[i].v2].GetFlag(MN_USER)) continue;
			
			float	edgeLen = Length(mesh.P(mesh.e[i].v1) - mesh.P(mesh.e[i].v2));
			totalEdgeLen += edgeLen;
			++totalEdgeCount;
		}

		//use our percent from paramblock and use that as our threshold times the averge edge length
		if (totalEdgeCount > 0)
		{
			double per = 0;
			per = double(mp_pblock->GetFloat(kEdgeCollapsePercent));
			thresh = (totalEdgeLen / double(totalEdgeCount)) * per;
		}
	}

	// In order to collapse vertices, we turn them into edge selections,
	// where the edges are shorter than the weld threshold.
	bool canWeld = false;
	mesh.ClearEFlags(MN_USER);
	float threshSq = thresh * thresh;
	for (int i = 0; i < mesh.nume; i++) {
		if (mesh.e[i].GetFlag(MN_DEAD)) continue;
		if (!mesh.v[mesh.e[i].v1].GetFlag(MN_USER)) continue;
		if (!mesh.v[mesh.e[i].v2].GetFlag(MN_USER)) continue;
		float	fLen2 = LengthSquared(mesh.P(mesh.e[i].v1) - mesh.P(mesh.e[i].v2));
		if (fLen2 > threshSq)
			continue;

		mesh.e[i].SetFlag(MN_USER);
		canWeld = true;
	}
	if (canWeld)
	{
		MNMeshUtilities mmu(&mesh);
		mmu.CollapseEdges(MN_USER);
	}

	return canWeld;
}


bool SymmetryMod::WeldPolyObject (MNMesh & mesh, const Point3 & N, float offset, float threshold, bool UseProximityThreshold, int startVerts, const Tab<int>& vertexPairs, bool radial)
{
	bool	bWelded = false;
	if (IsLegacy() || UseProximityThreshold)
	{	
		// Mark the vertices within the welding threshold of the plane
		mesh.ClearVFlags(MN_USER);
		for (int i = 0; i < mesh.numv; i++) {
			if (mesh.v[i].GetFlag(MN_DEAD)) continue;
			float dist = DotProd(N, mesh.P(i)) - offset;
			if (fabsf(dist) > threshold) continue;
			mesh.v[i].SetFlag(MN_USER);
		}

		// Do the welding:
		bWelded = mesh.WeldBorderVerts(threshold, MN_USER);
	}
	//new weld system that only welds matching mirror paired vertices instead of any possible border vertices within the threshold
	//radial symmetry does not support the new weld since it will have issues at the core
	//so fall back to the old proximity weld for now
	else if (UseNewWeld() && !radial)
	{
		int mirroredVertexCount = mesh.numv - startVerts;
		int startMirrorSource = startVerts - mirroredVertexCount;

		mesh.ClearVFlags(MN_WELD_SIDE1);
		mesh.ClearVFlags(MN_WELD_SIDE2);
		//loop thru our new copied and mirrored vertices
		for (int i = startVerts; i < mesh.numv; ++i,++startMirrorSource)
		{
			//must first have a valid mirror pair			
			if (vertexPairs[i] != -1)
			{
				//get the the vertex source and the mirror
				Point3& sourceP = mesh.v[vertexPairs[i]].p;
				Point3& mirrorP = mesh.v[i].p;
				//do our threshold check
				float d = Length(sourceP - mirrorP);
				if (d < threshold)
				{						
					//move both to the center so the weld operation does not have to check a threshold
					Point3 center = (sourceP + mirrorP) * 0.5f;
					sourceP = center;
					mirrorP = center;
					//and set the weld flag for both
					mesh.v[i].SetFlag(MN_WELD_SIDE1);
					mesh.v[vertexPairs[i]].SetFlag(MN_WELD_SIDE2);
				}
			}			
		}
		//now weld only vertices border vertices that are exactly on top and with the flags
		//NOTE an absolute super tiny threshold is used since if you pass in a 0 threshold WeldBorderVerts 
		//it assumes everything to be wielded and also the length of 2 points on top of each other  does
		//not absolutely equal 0.0 there is some slop in there
		bWelded = mesh.WeldBorderVerts(mMirrorWeldThreshold, MN_WELD_SIDE1, MN_WELD_SIDE2);
	}
	else {
		// MN_USER already has verts on plane tagged, determine what side of the plane they are on
		mesh.ClearVFlags(MN_WELD_SIDE1);
		mesh.ClearVFlags(MN_WELD_SIDE2);

		// Tag the two sides
		for (int i = 0; i < startVerts; i++)
			mesh.v[i].SetFlag(MN_WELD_SIDE1);

		for (int i=startVerts; i<mesh.numv; i++)
			mesh.v[i].SetFlag(MN_WELD_SIDE2);

		// Do the welding
		bWelded = mesh.WeldBorderVerts(threshold, MN_WELD_SIDE1, MN_WELD_SIDE2);
	}

	if (bWelded) { // If result was true, we have some MN_DEAD components
		mesh.CollapseDeadStructs();
		RemovePolySpurs(mesh);
	}

	return bWelded;
}

void SymmetryMod::RemovePolySpurs (MNMesh & mesh) {
	// Make sure we don't have any "spurs".
	for (int i=0; i<mesh.numv; i++) {
		if (mesh.v[i].GetFlag (MN_DEAD)) continue;
		if (mesh.vedg[i].Count() != 1) continue;
		int vid = i;
		while ((!mesh.v[vid].GetFlag (MN_DEAD)) && (mesh.vedg[vid].Count() == 1)) {
			int edge = mesh.vedg[vid][0];
			int otherEnd = mesh.e[edge].OtherVert (vid);
			mesh.RemoveSpur (edge);
			// The other end might be a tip of a spur now:
			vid = otherEnd;
			if (vid == i) {	// shouldn't happen - extra check to prevent loops.
				DbgAssert (0);
				break;
			}
		}
	}
}

void SymmetryMod::ModifyPolyObject (TimeValue t, PolyObject *pobj, bool bRadial, bool bMirror, BOOL weld, float thresh)
{
	MNMesh &mesh = pobj->GetMesh();

	if (mUseRampageWeldMath)
		mesh.SetFlag(MN_MESH_USE_MAX2012_WELD_MATH,TRUE);

	// Luna task 747
	// We cannot support specified normals in Symmetry at this time.
	mesh.ClearSpecifiedNormals();

	// If we're mirroring, we need to slice and mirror first
	if (bMirror) {
		DbgAssert(sliceTab.Count());
		DbgAssert(mirrorTab.Count());

		// Slice
		SlicePolyObject(mesh, sliceTab[0].plane, sliceTab[0].offset);
		// After we slice, check and remove thin faces at/near the edge
		if (!IsLegacy())
		{
			CollapsePolySliceEdge(mesh, mirrorTab[0].plane, mirrorTab[0].offset, EDGE_WELD_THRESHOLD);
		}

		// Mirror
		int startFaces = mesh.numf;
		int	startVerts = mesh.numv;

		for (int i = 0; i < mesh.numf; i++)
			mesh.f[i].SetFlag(MN_USER);

		Tab<int> radialVertexPairs;
		MirrorPolyObject(mesh, mirrorTab[0], IsLegacy(), radialVertexPairs);
		DbgAssert(mesh.CheckAllData());

		mesh.FlipElementNormals(MN_USER);	// flag should now be set only on clones.

		if (weld)
		{
			bool proximity_checked = (mp_pblock->GetInt(SymmetryMod::kSymProximityCheck, t) != 0);
			bool useLegacyAlgo = proximity_checked && sliceTab.Count() > 1;
			WeldPolyObject(mesh, mirrorTab[0].plane, mirrorTab[0].offset, thresh, useLegacyAlgo, startVerts, radialVertexPairs,false);
		}
		else {
			// Clear
			for (int i = startFaces; i < mesh.numf; i++)
				mesh.f[i].ClearFlag(MN_USER);
		}
	}

	int		iSliceCount = sliceTab.Count();	
	for (int i = bMirror; i < iSliceCount; i++)
	{
		SlicePolyObject(mesh, sliceTab[i].plane, sliceTab[i].offset);
		// After we slice, check and remove thin faces at/near the edge
		if (!IsLegacy())
		{
			CollapsePolySliceEdge(mesh, mirrorTab[0].plane, mirrorTab[0].offset, EDGE_WELD_THRESHOLD);
		}
	}

	//we want to remove dead struct since they can hose up mirror indexing data
	if (UseNewWeld())
		mesh.CollapseDeadStructs();
	

	int		iMirrorCount = mirrorTab.Count();
	if (iMirrorCount)
	{

		int	radialFaces = mesh.numf;
		int	lastFaceCount = radialFaces;
		Tab<int> vertexPairs;
		for (int i = bMirror; i < iMirrorCount; i++)
		{
			int startFaces = mesh.numf;

			// If radial we only want to flag the original faces
			// otherwise we want to flag all existing faces
			for (int j = 0; j < (bRadial?radialFaces: lastFaceCount); j++)
				mesh.f[j].SetFlag(MN_USER);

			int	startVerts = mesh.numv;

			MirrorPolyObject(mesh, mirrorTab[i], bRadial?false:IsLegacy(), vertexPairs);

			if (!bRadial)
				mesh.FlipElementNormals(MN_USER);	// flag should now be set only on clones.

			DbgAssert(mesh.CheckAllData());

			if (weld)
			{
				const bool UseLegacyAlgo = false;
				WeldPolyObject(mesh, mirrorTab[i].plane, mirrorTab[i].offset, thresh, UseLegacyAlgo, startVerts, vertexPairs, bRadial);
			}

			// For Radial, we want to clear the new Faces
			if (bRadial && (i != (iMirrorCount - 1)))
			{
				for (int j = startFaces; j < mesh.numf; j++)
					mesh.f[j].ClearFlag(MN_USER);
			}
			lastFaceCount = startFaces;
		}
	}

	// Cleanup after creating new vertices
	mesh.EliminateBadVerts();
}

void SymmetryMod::SliceTriObject (Mesh & mesh, const Point3 & N, float offset) {
	// Steve Anderson 9/14/2002
	// Using the new "MESH_TEMP_1" flag to override Slice selection behavior,
	// which is undesirable here.
	mesh.SetFlag (MESH_TEMP_1);
	MeshDelta slicemd;
	slicemd.Slice (mesh, N, offset, false, true);
	slicemd.Apply (mesh);
	mesh.ClearFlag (MESH_TEMP_1);

	// We need to strip out faces on the mirror plane itself.
	// (These aren't always removed by slice.)

	// Mark vertices at the plane boundary:
	BitArray targetVerts(mesh.numVerts);

	for (int i=0; i<mesh.numVerts; i++) {
		float dist = DotProd (N, mesh.verts[i]) - offset;
		if (fabsf(dist) > MNEPS) continue;
		targetVerts.Set (i);
	}
	BitArray delFaces(mesh.numFaces);
	for (int i=0; i<mesh.numFaces; i++) {
      int j;
		for (j=0; j<3; j++) {
			if (!targetVerts[mesh.faces[i].v[j]]) break;
		}
		if (j<3) continue;
		// Face needs to be deleted.
		delFaces.Set (i);
	}

	if (delFaces.AnyBitSet()) {
		BitArray delVerts;
		mesh.DeleteFaceSet(delFaces, &delVerts);
		mesh.DeleteVertSet(delVerts);
	}
}

static const int INVALID_NORMALMAPCHANNEL = -1;

void SymmetryMod::MirrorTriObject (Mesh & mesh, const MirrorEntry & me, int normalMapChannel, int oldnumf, bool bFlip, bool bWeld, float threshold, Tab<int>& vertexPairs)
{	
	int	oldnumv = mesh.numVerts;
	if (!oldnumv)
		return;

	if (oldnumf < 0)
		oldnumf = mesh.numFaces;

	const Matrix3&	mirrorTM = me.tm;

	// Hang on to a copy of the incoming face selection:
	BitArray inputFaceSel = mesh.FaceSel();

	int oldNumNormals = 0;
	if (normalMapChannel != INVALID_NORMALMAPCHANNEL)
	{
		MeshMap& map = mesh.Map(normalMapChannel);
		oldNumNormals = map.vnum;
	}

	BitArray fset(mesh.numFaces);
	if (bFlip)
		fset.SetAll ();
	else {
		for (int i = 0; i < oldnumf; i++)
			fset.Set(i);
	}

	Tab<int> facesToBeCopied;
	facesToBeCopied.Resize(mesh.numFaces);
	//loop thru the faces
	for (int i = 0; i < mesh.numFaces; ++i)
	{
		const Face& face = mesh.faces[i];
		//any face that is not dead and tagged throw the index into our tab
		if (fset[i])
		{
			facesToBeCopied.Append(1, &i);
		}
	}
	

	mesh.CloneFaces (fset);	// Clears selection on originals, sets it on new faces.

		//build our pair list, we have to use faces to compute this since the vertex order of the
	//cloned faces may not match the order of the source
	//The face order though does match
	BitArray openVerts;
	if (UseNewWeld())
	{
		//we are using edges list so need to make sure this is filled in
		AdjEdgeList adjEdges(mesh,TRUE);
		
		
		openVerts.SetSize(mesh.numVerts);
		openVerts.ClearAll();
		
		for (int i = 0; i < adjEdges.edges.Count(); ++i)
		{
			const MEdge& e = adjEdges.edges[i];
			if ((e.f[0] == -1) || (e.f[1] == -1))
			{
				openVerts.Set(e.v[0]);
				openVerts.Set(e.v[1]);
			}
		}

		//create a pair matching for our mirrored vertices fill out with -1 first
		//than fill in the pairs
		vertexPairs.SetCount(mesh.numVerts);
		for (int& pair : vertexPairs)
		{
			pair = -1;
		}

		//loop thru our to be copied faces and find the copied face
		for (int sourceFaceIndex = 0; sourceFaceIndex < facesToBeCopied.Count(); ++sourceFaceIndex)
		{
			int copiedFaceIndex = oldnumf + sourceFaceIndex;
			const Face& sourceFace = mesh.faces[sourceFaceIndex];
			const Face& copiedFace = mesh.faces[copiedFaceIndex];
			{
				for (int corner = 0; corner < 3; ++corner)
				{
					//see if this is an open vertex
					int prevCorner = corner - 1;
					if (prevCorner < 0)
						prevCorner = 2;

					
					//one of the eedges must be open since we only want to weld open vertices
					if (openVerts[sourceFace.v[corner]])
					{
						//if it is open store it as a potential weld target with it mirrored vertex
						vertexPairs[copiedFace.v[corner]] = sourceFace.v[corner];
					}
				}
			}
		}
	}


	// If we're flipping and welding, then use the mirror vert method here
	if (bWeld && bFlip)
	{
		BitArray welded(mesh.numVerts);
		float	thresholdSquared = WELD_THRESHOLD * WELD_THRESHOLD;
		// Transform the cloned vertices to their mirror images and flag for welding if appropriate
		if (UseNewWeld())
		{
			thresholdSquared = threshold * threshold;
			for (int i = oldnumv; i < mesh.numVerts; i++)
			{
				Point3 p3Mirror = (mesh.verts[i] * mirrorTM);
				mesh.verts[i] = p3Mirror;
			}
			for (int i = oldnumv; i < mesh.numVerts; i++)
			{
				if (openVerts[i])
				{
					Point3& opposingVert = mesh.verts[vertexPairs[i]];
					Point3& vert = mesh.verts[i];

					float fDist = LengthSquared(opposingVert - vert) * 0.5f;
					if ((vertexPairs[i] != -1) && (fDist <= thresholdSquared))
					{

						welded.Set(i);
						Point3 center = (opposingVert + vert) * 0.5f;
						vert = center;
						opposingVert = center;
					}
				}
			}
		}
		else
		{
			for (int i = oldnumv; i < mesh.numVerts; i++)
			{
				Point3 p3Mirror = (mesh.verts[i] * mirrorTM);
				float	fDist = LengthSquared(mesh.verts[i] - p3Mirror);
				if (fDist <= thresholdSquared)
					welded.Set(i);
				else
					mesh.verts[i] = p3Mirror;
			}
		}

		if (welded.AnyBitSet())
		{

			// Point to original vertex for welded verts on the new faces
			for (int j = oldnumf; j < mesh.numFaces; j++)
			{

				Face* f = &(mesh.faces[j]);
				if (welded[f->v[0]]) {
					f->v[0] = mesh.faces[j - oldnumf].v[0];
				}
				if (welded[f->v[1]]) {
					f->v[1] = mesh.faces[j - oldnumf].v[1];
				}
				if (welded[f->v[2]]) {
					f->v[2] = mesh.faces[j - oldnumf].v[2];
				}

			}
			mesh.DeleteVertSet(welded);

		}
	} else {
		// Transform the cloned vertices to their mirror images and weld later (if necessary)
		if (IsLegacy()  && !bFlip)
		{
			for (int i = oldnumv; i < mesh.numVerts; i++)
			mesh.verts[i] = (me.legacyInvTM*mesh.verts[i])*me.legacyTM;
		}
		else {
			for (int i = oldnumv; i < mesh.numVerts; i++)
				mesh.verts[i] = (mesh.verts[i] * mirrorTM);
		}
	}

	// Restore selection of input faces:
	BitArray& faceSel = mesh.FaceSel();
	for (int i=0; i<oldnumf; i++)
		faceSel.Set (i, inputFaceSel[i]);

	// Flip over new faces and select to match input:
	for (int i=oldnumf; i<mesh.numFaces; i++) {
		if(bFlip)
			mesh.FlipNormal (i);
		faceSel.Set (i, inputFaceSel[i-oldnumf]);
	}

	//flip specified normals/faces
	if (normalMapChannel != INVALID_NORMALMAPCHANNEL)
	{
		MeshMap& map = mesh.Map(normalMapChannel);
		int numNormals = map.vnum;
		
		for (int i = oldNumNormals; i < numNormals; i++)
		{
			Point3 n = map.tv[i];
			n = VectorTransform(n,mirrorTM);
			map.tv[i] = n;
		}
	}
}


void SymmetryMod::WeldTriObject (Mesh & mesh, const Point3 & N, float offset, float threshold, const Tab<int>& vertexPairs)
{
	// Find vertices in target zone of mirror plane:
	BitArray targetVerts(mesh.numVerts);
	for (int i = 0; i < mesh.numVerts; i++) 
	{
		float dist = DotProd(N, mesh.verts[i]) - offset;
		if (fabsf(dist) > threshold) continue;
		if (UseNewWeld())
		{ 
			if (vertexPairs[i] != -1)
			{
				targetVerts.Set(i);

				Point3& opposingVert = mesh.verts[vertexPairs[i]];
				Point3& vert = mesh.verts[i];
				Point3 center = (opposingVert + vert)*0.5f;
				vert = center;
				opposingVert = center;
			}
		}
		else
		{
			targetVerts.Set(i);
		}
	}

	// Weld the suitable border vertices:
	MeshDelta tmd(mesh);
	BOOL found = false;
	if (UseNewWeld())
	{
		//now weld only vertices border vertices that are exactly on top and in the set
		//NOTE an absolute super tiny threshold is used since if you pass in a 0 threshold WeldBorderVerts 
		//it assumes everything to be wielded and also the length of 2 points on top of each other  does
		//not absolutely equal 0.0 there is some slop in there
		found = tmd.WeldByThreshold(mesh, targetVerts, mMirrorWeldThreshold);
	}
	else
	{
		found = tmd.WeldByThreshold(mesh, targetVerts, threshold);
	}
	if (found)
		tmd.Apply(mesh);


}

void SymmetryMod::DoRadialWeld(Mesh &mesh, int vertexChunkSize, int faceChunkSize, bool bMirror, int iSliceCount, int iMirrorCount, float thresh)
{	
	// We should only have 2 slices we're working with
	DbgAssert((iSliceCount - bMirror) == 2);
	DbgAssert(iMirrorCount > 1);

	if ((mesh.numVerts == 0) || (mesh.numFaces == 0))
		return;

	// If we're radial mirroring, we need to build a list of candidate verts for welding along the slice edges
	BitArray	slicedVerts(mesh.numVerts);

	// Tag our original verts against the 2 primary slicing planes
	// First plane
	Point3		N1 = sliceTab[bMirror].plane;
	float		offset1 = sliceTab[bMirror].offset;

	// second plane
	Point3		N2 = sliceTab[iSliceCount - 1].plane;
	float		offset2 = sliceTab[iSliceCount - 1].offset;

	for (int v = 0; v < vertexChunkSize; v++)
	{
		float dist = DotProd(N1, mesh.verts[v]) - offset1;
		if (fabsf(dist) <= thresh)
		{
			slicedVerts.Set(v);
			continue;
		}
		dist = DotProd(N2, mesh.verts[v]) - offset2;
		if (fabsf(dist) <= thresh)
			slicedVerts.Set(v);
	}

	// If nothing selected, we can bail now
	if (slicedVerts.IsEmpty())
		return;

	// Due to orientation flips, check the last slicing plane against the first copy
	// and then tag the incremental verts (every vertexChunkSize increment) in the
	// subsequent copies

	int		iStartVert = vertexChunkSize;
	int		iEndVert = vertexChunkSize * 2;

	// Transform matrix for the last rotatation
	Matrix3		m = Inverse(mirrorTab[iMirrorCount - 1].tm);

	// First plane (rotated)
	N1 = m.VectorTransform(sliceTab[bMirror].plane);
	offset1 = sliceTab[bMirror].offset - DotProd(N1, -m.GetTrans());

	// Second plane (rotated)
	N2 = m.VectorTransform(sliceTab[iSliceCount - 1].plane);
	offset2 = sliceTab[iSliceCount - 1].offset - DotProd(N2, -m.GetTrans());

	// Check this plane against the verts created in the 1st copy
	for (int v = iStartVert; v < iEndVert; v++)
	{
		float dist = DotProd(N1, mesh.verts[v]) - offset1;
		if (fabsf(dist) <= thresh) {
			slicedVerts.Set(v);
			// Propogate to the all of the remaining copies
			int iOffset = vertexChunkSize;
			for (int j = bMirror + 1; j < iMirrorCount; j++, iOffset += vertexChunkSize)
				slicedVerts.Set(v + iOffset);
			continue;
		}
		dist = DotProd(N2, mesh.verts[v]) - offset2;
		if (fabsf(dist) <= thresh)
		{
			slicedVerts.Set(v);
			// Propogate to the all of the remaining copies
			int iOffset = vertexChunkSize;
			for (int j = bMirror + 1; j < iMirrorCount; j++, iOffset += vertexChunkSize)
				slicedVerts.Set(v + iOffset);
		}
	}

	// slicedVerts now tags all of the verts that might be welded
	// of those, tag the verts that need deletion
	BitArray		delVerts(mesh.numVerts);

	// It is possible that some verts that we plan to remove are already targets from other verts
	// so build a map of the vert moves
	IntTab			vertMap;
	vertMap.SetCount(mesh.numVerts);
	int* vert = vertMap.Addr(0);
	for (int i = 0; i < mesh.numVerts; i++)
		*vert++ = i;

	// Now that the we've tagged all of the verts, start checking for distance in each batch of copies
	int numGroups = iMirrorCount - bMirror + 1;
	if (numGroups == 2)	// Since we compare both sides of the slice, only need to check once if there are 2 groups
		numGroups = 1;

	// Loop through the group and look for matches with the next group
	float	fThresholdSquared = thresh * thresh;
	for (int group = 0; group < numGroups; group++)
	{
		// The last group checks the first group
		int		nextGroup = (group + 1) % numGroups;
		if (numGroups == 1)
			nextGroup = 1;

		// Search in groups
		int		myGroupVertStart = group * vertexChunkSize;
		int		myGroupVertEnd = myGroupVertStart + vertexChunkSize;

		int		nextGroupVertStart = nextGroup * vertexChunkSize;
		int		nextGroupVertEnd = nextGroupVertStart + vertexChunkSize;

		for (int v = myGroupVertStart; v < myGroupVertEnd; v++)
		{
			if (!slicedVerts[v])
				continue;

			// Find closest vert within threshold
			int		iClosestIndex = -1;
			float	fClosestDist = fThresholdSquared;

			for (int nextv = nextGroupVertStart; nextv < nextGroupVertEnd; nextv++)
			{
				if (!slicedVerts[nextv])
					continue;

				float	fDist = LengthSquared(mesh.verts[v] - mesh.verts[nextv]);
				if (fDist < fClosestDist)
				{
					iClosestIndex = nextv;
					fClosestDist = fDist;
				}
			}

			// Found our weld candidate, process
			if (iClosestIndex >= 0)
			{
				// Clear so we don't check again
				slicedVerts.Clear(iClosestIndex);

				int		iSourceVert = iClosestIndex;
				int		iDestVert = v;
				int		iSourceGroup = nextGroup;

				// See if we need to swap indexes (in the case of the last copy)
				// to ensure we don't delete an earlier vertex
				if (group > nextGroup)
				{
					std::swap(iSourceVert, iDestVert);
					iSourceGroup = group;
				}

				// Get my the actual vertex (in case it was welded in a prior round)
				iDestVert = vertMap[iDestVert];
				DbgAssert(!delVerts[iDestVert]);

				// Store my mapped point
				vertMap[iSourceVert] = iDestVert;

				// Tag for deletion
				delVerts.Set(iSourceVert);

				// Go through all of our faces within our group and substitute with the weld target vert
				int		iSourceStartFace = iSourceGroup * faceChunkSize;
				int		iSourceEndFace = iSourceStartFace + faceChunkSize;

				for (int f = iSourceStartFace; f < iSourceEndFace; f++)
				{
					Face* face = &(mesh.faces[f]);
					if (face->v[0] == iSourceVert)
						face->v[0] = iDestVert;

					if (face->v[1] == iSourceVert)
						face->v[1] = iDestVert;

					if (face->v[2] == iSourceVert)
						face->v[2] = iDestVert;
				}
			}
		}
	}

	// Delete all of the flagged verts
	if (delVerts.AnyBitSet())
		mesh.DeleteVertSet(delVerts);
}

void SymmetryMod::WeldSlicedEdge(Mesh & mesh, const SliceEntry& se, float thresh)
{
	// Weld long the edge of the tri we just sliced, necessary as sometimes the vertices created by slicing a face are very close

	// Tag verts along our slice plane
	BitArray	slicedVerts(mesh.numVerts);

	for (int v = 0; v < mesh.numVerts; v++)
	{
		float dist = DotProd(se.plane, mesh.verts[v]) - se.offset;
		if (fabsf(dist) <= MNEPS) {
			slicedVerts.Set(v);
		}
	}

	//the incoming thresh is tied to the UI threshold which controls both the weld along and to plane
	//new method the UI only controls the to plane threshold
	//the along plane uses a percentage of the average edge length to clean up the mesh
	
	if (UseNewEdgeWeldThreshold())
	{
		AdjEdgeList adjEdges(mesh, TRUE);

		double totalEdgeLen = 0.0;
		size_t totalEdgeCount = 0;
		//find the average length of edges of the mesh
		for (int i = 0; i < adjEdges.edges.Count(); i++) {
			int v1 = adjEdges.edges[i].v[0];
			int v2 = adjEdges.edges[i].v[1];
			float	edgeLen = 0.0f;
			edgeLen = Length(mesh.verts[v1] - mesh.verts[v2]);
			totalEdgeLen += edgeLen;
			++totalEdgeCount;
		}

		//use our percent from paramblock and use that as our threshold times the averge edge length
		if (totalEdgeCount > 0)
		{
			double per = 0;
			per = double(mp_pblock->GetFloat(kEdgeCollapsePercent));
			thresh = (totalEdgeLen / double(totalEdgeCount)) * per;
		}
		else 
			thresh = 0.0f;
	}
	// Utilize traditional mesh weld
	MeshDelta tmd(mesh);
	BOOL found = tmd.WeldByThreshold(mesh, slicedVerts, thresh);
	if (found)
		tmd.Apply(mesh);
}

void SymmetryMod::ModifyTriObject (TimeValue t, TriObject *tobj, bool bRadial, bool bMirror, BOOL weld, float thresh)
{
	Mesh &mesh = tobj->GetMesh();

	// Slice operation does not handle NormalSpecs, but it handles mapping channels.
	// move our mesh normal data to a map channel
    MeshNormalSpec* pNormals = mesh.GetSpecifiedNormals();
    int normalMapChannel = INVALID_NORMALMAPCHANNEL;
    if (pNormals != nullptr)
    {
        int nNormals = pNormals->GetNumNormals();
        int nNormalFaces = pNormals->GetNumFaces();
        if ((nNormals != 0) && (nNormalFaces !=0))
        {
            pNormals->SetParent(&mesh);
		    pNormals->CheckNormals();

            // Find an empty map channel, and use it to store the the normals
            for (int mp = 0; mp < mesh.getNumMaps(); mp++)
            {			
                if (!mesh.mapSupport(mp)) 
                {
                    normalMapChannel = mp;
                    mesh.setMapSupport(normalMapChannel,TRUE);
                    MeshMap& map = mesh.Map(normalMapChannel);

                    // From the implementation below, we support propagation of normals only in cases where all normals are
                    // defined; this should be true in light of the call to CheckNormals above, however, if we encounter any
                    // undefined normals, set map mp back to unsupported, and set normalMapChannel back to
                    // INVALID_NORMALMAPCHANNEL
                    int indexFace = 0;
                    bool areAllNormalsValid = true;
                    while (areAllNormalsValid && (indexFace != map.fnum))
                    {
                        int indexVertex = 0;
                        while (areAllNormalsValid && (indexVertex != 3))
                        {
                            areAllNormalsValid = false;
                            int normalID = pNormals->Face(indexFace).GetNormalID(indexVertex);
                            if ((normalID >= 0) && (normalID < nNormals))
                            {
                                (map.tf[indexFace]).t[indexVertex] = normalID;
                                areAllNormalsValid = true;
                            }

                            ++indexVertex;
                        }

                        ++indexFace;
                    }

                    if (areAllNormalsValid)
                    {
                        map.setNumVerts(pNormals->GetNumNormals());
                        for (int i = 0; i < map.vnum; i++)
                        {
                            map.tv[i] = pNormals->Normal(i);
                        }
                    }
                    else
                    {
                        mesh.setMapSupport(normalMapChannel, FALSE);
                        normalMapChannel = INVALID_NORMALMAPCHANNEL;
                    }

                    // Make sure nothing is done with MeshNormalSpec (until data is copied back) 
                    pNormals->Clear();
                    break;
                }
            }
        }
    }

	// If we're mirroring, we need to slice and mirror first
	if (bMirror) {
		DbgAssert(sliceTab.Count());
		SliceTriObject(mesh, sliceTab[0].plane, sliceTab[0].offset);

		int		iOriginalFaces = mesh.numFaces;
		Tab<int> vertexPairs;
		MirrorTriObject(mesh, mirrorTab[0], normalMapChannel, iOriginalFaces, true, weld && !IsLegacy(), thresh, vertexPairs);

		bool proximity = (mp_pblock->GetInt(SymmetryMod::kSymProximityCheck, t) != 0);
		if (weld && (IsLegacy() || proximity) )
			WeldTriObject(mesh, mirrorTab[0].plane, mirrorTab[0].offset, thresh, vertexPairs);
	}

	
	// Slice
	int		iSliceCount = sliceTab.Count();
	for (int i = bMirror; i < iSliceCount; i++)
	{
		SliceTriObject(mesh, sliceTab[i].plane, sliceTab[i].offset);
		// If not legacy, weld very close verts along the slice edge
		if (!IsLegacy())
			WeldSlicedEdge(mesh, sliceTab[i], thresh);
	}

	// Mirror
	int		iMirrorCount = mirrorTab.Count(); //-1;
	int		nFaces = bRadial ? mesh.getNumFaces() : -1;
	int		preMirrorVerts = mesh.numVerts;

	
	if (iMirrorCount)
	{
		for (int i = bMirror; i < iMirrorCount; i++)
		{
			int		iOriginalFaces = mesh.numFaces;
			Tab<int> vertexPairs;
			MirrorTriObject(mesh, mirrorTab[i], normalMapChannel, nFaces, !bRadial, weld && !IsLegacy(), thresh, vertexPairs);
			// If welding either legacy or radial without any slices, use original weld method
			if (weld && (IsLegacy() || (bRadial && ((int)bMirror == iSliceCount))))
			{
				WeldTriObject(mesh, mirrorTab[i].plane, mirrorTab[i].offset, thresh, vertexPairs);
			}
		}

		// If radial weld with slices (but not legacy)
		if (bRadial && weld && !IsLegacy() && ((int)bMirror != iSliceCount))
			DoRadialWeld(mesh, preMirrorVerts, nFaces, bMirror, iSliceCount, iMirrorCount, thresh);
	}

	//now move the normals back
	if (pNormals && normalMapChannel != -1)
	{
		MeshMap& map = mesh.Map(normalMapChannel);
		pNormals->SetNumFaces(map.fnum);

		pNormals->SetNumNormals(map.vnum);
		pNormals->SetAllExplicit(true);
		BitArray temp(map.vnum);
		temp.SetAll();
		pNormals->SpecifyNormals(TRUE,&temp);

		for (int i = 0; i < map.vnum; i++)
		{
			pNormals->GetNormalArray()[i] = map.tv[i];
			pNormals->SetNormalExplicit(i,true);
		}	

		for (int i = 0; i < map.fnum; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				pNormals->SetNormalIndex(i,j,map.tf[i].t[j]);				
				MeshNormalFace& face = pNormals->Face(i);
				face.SpecifyAll(true);
			}
		}

		pNormals->SetFlag(MESH_NORMAL_MODIFIER_SUPPORT);

		for (int i = 0; i < pNormals->GetNumFaces(); i++)
		{
			for (int j = 0; j < 3; j++)
			{
				int id = pNormals->GetNormalIndex(i,j);	
			}
		}

		pNormals->CheckNormals();
		pNormals->SetParent(NULL);

		// Free the map channel
		mesh.setMapSupport(normalMapChannel,FALSE);
	}
}

Interval SymmetryMod::LocalValidity(TimeValue t) {
  // aszabo|feb.05.02 If we are being edited, 
	// return NEVER to forces a cache to be built after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;

	Interval iv = FOREVER;
	int foo;
	mp_pblock->GetValue(kSymAxis, t, foo, iv);
	mp_pblock->GetValue (kSymFlip, t, foo, iv);
	mp_pblock->GetValue(kSymWeld, t, foo, iv);
	mp_pblock->GetValue(kSymSlice, t, foo, iv);
	float thresh;
	mp_pblock->GetValue (kSymThreshold, t, thresh, iv);
	Matrix3 mat;		
	mp_mirror->GetValue(t,&mat,iv,CTRL_RELATIVE);
	return iv;
}

Matrix3 SymmetryMod::CompMatrix(TimeValue t, INode *inode, ModContext *mc)
{
	// Make sure we've set the bounding box size
	if (mc->localData == nullptr)
		mc->localData = static_cast<LocalModData *>(new BasicModData);

	BasicModData *smd = static_cast<BasicModData*>(mc->localData);

	if (smd->GetSize() < 0) {
		float size1 = (mc->box->pmax.x - mc->box->pmin.x)*.52f;
		float size2 = (mc->box->pmax.y - mc->box->pmin.y)*.52f;
		if (size2 > size1)
			size1 = size2;
		if (size1 < 1.0f)
			size1 = 1.0f;

		smd->SetSize(size1);
	}

	Interval iv;
	Matrix3 tm;
	if (mc && mc->tm)
		tm = Inverse(*(mc->tm));
	if (inode)
		tm *= inode->GetObjTMBeforeWSM(t, &iv);

	return tm;
}

void SymmetryMod::DrawPlanarGizmos(GraphicsWindow* gw, const Matrix3& ptm, float size)
{
	if (!gw)
		return;

	bool	bSliceX = (mp_pblock->GetInt(kSymPlanar_X) != 0);
	bool	bSliceY = (mp_pblock->GetInt(kSymPlanar_Y) != 0);
	bool	bSliceZ = (mp_pblock->GetInt(kSymPlanar_Z) != 0);

	Point3 rp[4];

	if (bSliceX) {
		rp[0] = Point3(0.0f, -size, -size)*ptm;
		rp[1] = Point3(0.0f, -size, size)*ptm;
		rp[2] = Point3(0.0f, size, size)*ptm;
		rp[3] = Point3(0.0f, size, -size)*ptm;
		gw->polyline(4, rp, NULL, NULL, TRUE, NULL);
	}

	if (bSliceY) {
		rp[0] = Point3(-size, 0.0f, -size)*ptm;
		rp[1] = Point3(-size, 0.0f, size)*ptm;
		rp[2] = Point3(size, 0.0f, size)*ptm;
		rp[3] = Point3(size, 0.0f, -size)*ptm;
		gw->polyline(4, rp, NULL, NULL, TRUE, NULL);
	}

	if (bSliceZ) {
		rp[0] = Point3(-size, -size, 0.0f)*ptm;
		rp[1] = Point3(-size, size, 0.0f)*ptm;
		rp[2] = Point3(size, size, 0.0f)*ptm;
		rp[3] = Point3(size, -size, 0.0f)*ptm;
		gw->polyline(4, rp, NULL, NULL, TRUE, NULL);
	}
}

void SymmetryMod::DrawRadialGizmos(GraphicsWindow* gw, const Matrix3& ptm, float size, TimeValue t)
{
	if (!gw)
		return;

	Matrix3	m1 = ptm;
	constexpr int	maxArcPointCount = 17;
	Point3 rp1[maxArcPointCount];

	int iRadialCount = mp_pblock->GetInt(kSymRadialCount, t);

	bool bMirror = (mp_pblock->GetInt(kSymRadialMirror) != 0);
	bool bSlice = (mp_pblock->GetInt(kSymSlice) != 0); //, t, slice, iv);

	float	fAngle = TWOPI / (iRadialCount); //-1);
	float	fHalfAngle = fAngle *0.5f;

	// Store our origin
	rp1[0] = Point3(0.0f, 0.0f, 0.0f)*m1;

	int	iAxis = mp_pblock->GetInt(kSymRadialAxis);
	if (iAxis == X_AXIS)
	{
		// arc
		float	fInc = (TWOPI - fAngle) / (maxArcPointCount - 2);
		m1.PreRotateX(fHalfAngle); // +HALFPI); // +PI);
		for (int i = 1; i < maxArcPointCount; i++)
		{
			rp1[i] = Point3(0.0f, 0.0f, size*0.9f)*m1;
			m1.PreRotateX(fInc);
		}
		gw->polyline(maxArcPointCount, rp1, NULL, NULL, TRUE, NULL);

		// draw the slicing planes
		if (bSlice) {
			m1 = ptm;
			float	fSize = size * 1.25f;
			float	fSize2 = size * 2.0f;

			rp1[1] = Point3(fSize2, 0.0f, 0.0f)*m1;
			m1.PreRotateX(-fHalfAngle + HALFPI);
			rp1[2] = Point3(fSize2, fSize, 0.0f)*m1;
			rp1[3] = Point3(0.0f, fSize, 0.0f)*m1;
			rp1[4] = rp1[0];

			m1 = ptm;
			m1.PreRotateX(fHalfAngle + HALFPI);
			rp1[5] = Point3(0.0f, fSize, 0.0f)*m1;
			rp1[6] = Point3(fSize2, fSize, 0.0f)*m1;
			rp1[7] = Point3(fSize2, 0.0f, 0.0f)*m1;

			gw->polyline(8, rp1, NULL, NULL, FALSE, NULL);
		} else {
			// Show axis vector
			rp1[1] = Point3(size, 0.0f, 0.0f)*m1;
			gw->polyline(2, rp1, NULL, NULL, FALSE, NULL);
		}

		// Show the slicing mirror
		if (bMirror)
		{
			m1 = ptm;
			float		fSize2 = size * 2;
			rp1[0] = Point3(-fSize2, 0.0f, -fSize2)*m1;
			rp1[1] = Point3(-fSize2, 0.0f, fSize2)*m1;
			rp1[2] = Point3(fSize2, 0.0f, fSize2)*m1;
			rp1[3] = Point3(fSize2, 0.f, -fSize2)*m1;
			gw->polyline(4, rp1, NULL, NULL, TRUE, NULL);
		}
	}
	else if (iAxis == Y_AXIS)
	{
		// arc
		float	fInc = (TWOPI - fAngle) / (maxArcPointCount - 2);
		m1.PreRotateY(fHalfAngle); // +HALFPI);
		for (int i = 1; i < maxArcPointCount; i++)
		{
			rp1[i] = Point3(size*0.9f, 0.0f, 0.0f)*m1;
			m1.PreRotateY(fInc);
		}
		gw->polyline(maxArcPointCount, rp1, NULL, NULL, TRUE, NULL);

		// draw the slicing planes
		if (bSlice) {
			m1 = ptm;
			float	fSize = size * 1.25f;
			float	fSize2 = size * 2.0f;

			rp1[1] = Point3(0.0f, fSize2, 0.0f)*m1;
			m1.PreRotateY(-fHalfAngle+HALFPI);
			rp1[2] = Point3(0.0f, fSize2, fSize)*m1;
			rp1[3] = Point3(0.0f, 0.0f, fSize)*m1;
			rp1[4] = rp1[0];

			m1 = ptm;
			m1.PreRotateY(fHalfAngle+HALFPI);
			rp1[5] = Point3(0.0f, 0.0f, fSize)*m1;
			rp1[6] = Point3(0.0f, fSize2, fSize)*m1;
			rp1[7] = Point3(0.0f, fSize2, 0.0f)*m1;

			gw->polyline(8, rp1, NULL, NULL, FALSE, NULL);
		}
		else {
			// Show axis vector
			rp1[1] = Point3(0.0f, size, 0.0f)*m1;
			gw->polyline(2, rp1, NULL, NULL, FALSE, NULL);
		}

		// Show the slicing mirror
		if (bMirror)
		{
			m1 = ptm;
			float		fSize2 = size * 2;
			rp1[0] = Point3(-fSize2, -fSize2, 0.0f)*m1;
			rp1[1] = Point3(-fSize2, fSize2, 0.0f)*m1;
			rp1[2] = Point3(fSize2, fSize2, 0.0f)*m1;
			rp1[3] = Point3(fSize2, -fSize2, 0.0f)*m1;
			gw->polyline(4, rp1, NULL, NULL, TRUE, NULL);
		}
	}
	else {
		// draw arc
		float	fInc = (TWOPI - fAngle) / (maxArcPointCount - 2);
		m1.PreRotateZ(fHalfAngle+HALFPI);
		for (int i = 1; i < maxArcPointCount; i++)
		{
			rp1[i] = Point3(size*0.9f, 0.0f, 0.0f)*m1;
			m1.PreRotateZ(fInc);
		}
		gw->polyline(maxArcPointCount, rp1, NULL, NULL, TRUE, NULL);

		// draw the slicing planes
		if (bSlice) {
			m1 = ptm;
			float	fSize = size * 1.25f;
			float	fSize2 = size * 2.0f;

			rp1[1] = Point3(0.0f, 0.0f, fSize2)*m1;
			m1.PreRotateZ(-fHalfAngle + HALFPI);
			rp1[2] = Point3(fSize, 0.0f, fSize2)*m1;
			rp1[3] = Point3(fSize, 0.0f, 0.0f)*m1;
			rp1[4] = rp1[0];

			m1 = ptm;
			m1.PreRotateZ(fHalfAngle + HALFPI);
			rp1[5] = Point3(fSize, 0.0f, 0.0f)*m1;
			rp1[6] = Point3(fSize, 0.0f, fSize2)*m1;
			rp1[7] = Point3(0.0f, 0.0f, fSize2)*m1;

			gw->polyline(8, rp1, NULL, NULL, FALSE, NULL);
		} else {
			// Show axis vector
			rp1[1] = Point3(0.0f, 0.0f, size)*m1;
			gw->polyline(2, rp1, NULL, NULL, FALSE, NULL);
		}

		// Show the slicing mirror
		if (bMirror)
		{
			m1 = ptm;
			float		fSize2 = size * 2;
			rp1[0] = Point3(0.0f, -fSize2, -fSize2)*m1;
			rp1[1] = Point3(0.0f, -fSize2, fSize2)*m1;
			rp1[2] = Point3(0.0f, fSize2, fSize2)*m1;
			rp1[3] = Point3(0.0f, fSize2, -fSize2)*m1;
			gw->polyline(4, rp1, NULL, NULL, TRUE, NULL);
		}
	}

}

int SymmetryMod::HitTest (TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc)
{
	if ( ! vpt || ! vpt->IsAlive() )
	{
		// why are we here
		DbgAssert(!_T("Invalid viewport!"));
		return FALSE;
	}

	GraphicsWindow *gw = vpt->getGW();
	Point3 pt;
	HitRegion hr;
	int savedLimits;

	INode*	objRef = mp_pblock->GetINode(kSymNodeRef);

	Matrix3 tm = CompMatrix(t,objRef?objRef:inode,mc);

	MakeHitRegion(hr,type, crossing,4,p);
	gw->setHitRegion(&hr);	
	gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);	
	gw->setTransform(tm);
	gw->clearHitCode();

	bool	bPlanar = (mp_pblock->GetInt(kSymFormat) == symmetry_planar);

	Matrix3 ptm;
	mp_mirror->GetValue(t, &ptm, FOREVER, CTRL_RELATIVE);

	BasicModData *smd = static_cast<BasicModData*>(mc->localData);

	if (bPlanar)
		DrawPlanarGizmos(gw, ptm,  smd->GetSize());
	else
		DrawRadialGizmos(gw, ptm, smd->GetSize(), t);

	gw->setRndLimits(savedLimits);
	if (gw->checkHitCode()) {
		vpt->LogHit(inode, mc, gw->getHitDistance(), 0, NULL); 
		return 1;
		}
	return 0;
}

int SymmetryMod::Display (TimeValue t, INode* inode, ViewExp *vpt, int flagst, ModContext *mc)
{
	if ( ! vpt || ! vpt->IsAlive() )
	{
		// why are we here
		DbgAssert(!_T("Invalid viewport!"));
		return FALSE;
	}

	GraphicsWindow *gw = vpt->getGW();
	//Point3 pt[4];

	bool	bPlanar = (mp_pblock->GetInt(kSymFormat) == symmetry_planar);
	INode*	objRef = mp_pblock->GetINode(kSymNodeRef);
	Matrix3 tm = CompMatrix(t, objRef ? objRef : inode, mc);

	int savedLimits;

	gw->setRndLimits((savedLimits = gw->getRndLimits()) & ~GW_ILLUM);
	gw->setTransform(tm);
	if (mp_ip && mp_ip->GetSubObjectLevel() == 1) {
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_SEL_GIZMOS));
	} else {
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_GIZMOS));
	}

	Matrix3 ptm;
	mp_mirror->GetValue(t, &ptm, FOREVER, CTRL_RELATIVE);

	BasicModData *smd = (BasicModData *)mc->localData;

	if (bPlanar)
		DrawPlanarGizmos(gw, ptm, smd->GetSize());
	else
		DrawRadialGizmos(gw, ptm, smd->GetSize(), t);

	gw->setRndLimits(savedLimits);
	return 0;
}

void SymmetryMod::GetWorldBoundBox (TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc)
{	
	if ( ! vpt || ! vpt->IsAlive() )
	{
		box.Init();
		return;
	}

	Matrix3 tm = CompMatrix(t, inode, mc);

	BasicModData *smd = static_cast<BasicModData*>(mc->localData);

	Matrix3 ptm;
	mp_mirror->GetValue(t, &ptm, FOREVER, CTRL_RELATIVE);
	ptm *= tm;
	box.Init();

	float size = smd->GetSize();
	box += Point3(-size, -size, 0.0f)*ptm;
	box += Point3(-size, size, 0.0f)*ptm;
	box += Point3(size, size, 0.0f)*ptm;
	box += Point3(size, -size, 0.0f)*ptm;
}

void SymmetryMod::Move(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin) {
	SetXFormPacket pckt(val,partm,tmAxis);
	mp_mirror->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);
}

void SymmetryMod::Rotate(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Quat& val, BOOL localOrigin)
{
	INode*	objRef = mp_pblock->GetINode(kSymNodeRef);
	SetXFormPacket pckt(val,localOrigin, objRef?objRef->GetObjTMBeforeWSM(t):partm,tmAxis);
	mp_mirror->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);
}

void SymmetryMod::Scale (TimeValue t, Matrix3 & partm, Matrix3 & tmAxis, Point3 & val, BOOL localOrigin)
{
	INode*	objRef = mp_pblock->GetINode(kSymNodeRef);
	SetXFormPacket pckt(val, localOrigin, objRef ? objRef->GetObjTMBeforeWSM(t) : partm, tmAxis);
	mp_mirror->SetValue (t, &pckt, true, CTRL_RELATIVE);
}

void SymmetryMod::GetSubObjectCenters (SubObjAxisCallback *cb,TimeValue t, INode *node,ModContext *mc)
{
	INode*	objRef = mp_pblock->GetINode(kSymNodeRef);
	Matrix3 tm = CompMatrix(t,objRef?objRef:node,mc);

	Matrix3 ptm;
	mp_mirror->GetValue(t, &ptm, FOREVER, CTRL_RELATIVE);
	cb->Center((ptm*tm).GetTrans(),0);
}

void SymmetryMod::GetSubObjectTMs (SubObjAxisCallback *cb,TimeValue t, INode *node,ModContext *mc)
{
	Matrix3 tm = CompMatrix(t, node, mc);
	Matrix3 ptm;
	mp_mirror->GetValue(t, &ptm, FOREVER, CTRL_RELATIVE);
	cb->TM((ptm*tm), 0);
}

void SymmetryMod::ActivateSubobjSel(int level, XFormModes& modes) {
	switch (level) {
	case 1: // Mirror center
		modes = XFormModes (mp_moveMode, mp_rotMode, mp_scaleMode, mp_nuScaleMode, mp_squashMode, NULL);
		break;
	}
	NotifyDependents(FOREVER,PART_DISPLAY,REFMSG_CHANGE);
}

BOOL SymmetryMod::AssignController(Animatable *control,int subAnim) {
	if (subAnim==kSYM_MIRROR_REF) {
		ReplaceReference(kSYM_MIRROR_REF,(ReferenceTarget*)control);
		NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
		NotifyDependents(FOREVER,PART_ALL,REFMSG_SUBANIM_STRUCTURE_CHANGED);		
		return TRUE;
	} else {
		return FALSE;
	}
}

int SymmetryMod::SubNumToRefNum(int subNum) {
	if (subNum==kSYM_MIRROR_REF) return subNum;
	else return -1;
}

RefTargetHandle SymmetryMod::GetReference(int i) {
	switch (i) {
	case kSYM_PBLOCK_REF: return mp_pblock;
	case kSYM_MIRROR_REF: return mp_mirror;
	case NODE_TRANSFORM_MONITOR_REF: return transformMonitor;
	default: return NULL;
	}
}

void SymmetryMod::SetReference(int i, RefTargetHandle rtarg) {
	switch (i) {
	case kSYM_PBLOCK_REF: mp_pblock = (IParamBlock2*)rtarg; break;
	case kSYM_MIRROR_REF: mp_mirror = (Control*)rtarg; break;
	case NODE_TRANSFORM_MONITOR_REF: transformMonitor = (NodeTransformMonitor*)rtarg; break;
	}
}

void SymmetryMod::OnLegacyAxisOrFlip()
{
	// If legacy values are updated (usually through script or old file load), map to new pblock entries
	int iAxis = mp_pblock->GetInt(kSymAxis);
	BOOL bFlip =  mp_pblock->GetInt(kSymFlip);

	if (mp_pblock->GetInt(kSymFormat) == symmetry_planar)
	{
		// Set Planar Values
		mp_pblock->SetValue(kSymPlanar_X, 0, iAxis == X_AXIS);
		mp_pblock->SetValue(kSymPlanar_Y, 0, iAxis == Y_AXIS);
		mp_pblock->SetValue(kSymPlanar_Z, 0, iAxis == Z_AXIS);
		if (iAxis == X_AXIS)
			mp_pblock->SetValue(kSymPlanarFlip_X, 0, bFlip);
		else if (iAxis == Y_AXIS)
			mp_pblock->SetValue(kSymPlanarFlip_Y, 0, bFlip);
		else
			mp_pblock->SetValue(kSymPlanarFlip_Z, 0, bFlip);
	}
	else {
		// Set Radial Values
		mp_pblock->SetValue(kSymRadialAxis, 0, iAxis);
		mp_pblock->SetValue(kSymRadialFlip, 0, bFlip);
	}
}

void SymmetryMod::SetLegacyAxisAndFlip()
{
	// Defaults
	int iAxis = X_AXIS;
	BOOL bFlip = FALSE;

	// Try to match updated PB values to the equivalent legacy values
	if (mp_pblock->GetInt(kSymFormat) == symmetry_planar)
	{
		// Priority of values to use X, Y then Z
		if (mp_pblock->GetInt(kSymPlanar_X))
		{
			bFlip = mp_pblock->GetInt(kSymPlanarFlip_X);
		}
		else if (mp_pblock->GetInt(kSymPlanar_Y))
		{
			iAxis = Y_AXIS;
			bFlip = mp_pblock->GetInt(kSymPlanarFlip_Y);
		}
		else if (mp_pblock->GetInt(kSymPlanar_Z))
		{
			iAxis = Z_AXIS;
			bFlip = mp_pblock->GetInt(kSymPlanarFlip_Z);
		}
	}
	else
	{
		iAxis = mp_pblock->GetInt(kSymRadialAxis);
		bFlip = mp_pblock->GetInt(kSymRadialFlip);
	}

	// Apply to legacy entries
	mp_pblock->SetValue(kSymAxis, 0, iAxis);
	mp_pblock->SetValue(kSymFlip, 0, bFlip);
}

RefResult SymmetryMod::NotifyRefChanged( const Interval& changeInt,RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate)
{
	if ((message == REFMSG_CHANGE) && (hTarget == mp_pblock))
	{
		ParamID changing_param = mp_pblock->LastNotifyParamID();
		if ((changing_param == kSymAxis) || (changing_param == kSymFlip))
		{
			OnLegacyAxisOrFlip();
		}
	} else if ((hTarget == transformMonitor) && HasObject())
	{
		// If we are using a reference object and we signal on the transformMonitor, update our object
		NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	}
	
	return REF_SUCCEED;
}

TSTR SymmetryMod::SubAnimName(int i, bool localized)
{
	switch (i)
	{
	case kSYM_PBLOCK_REF: return localized ? GetString(IDS_PARAMETERS) : _T("Parameters");
	case kSYM_MIRROR_REF: return localized ? GetString(IDS_SYM_MIRROR) : _T("Mirror");
	}
	return _T("");
}

static GenSubObjType SOT_Center(18);

ISubObjType *SymmetryMod::GetSubObjType(int i) {
	static bool initialized = false;
	if(!initialized) {
		initialized = true;
		SOT_Center.SetName(GetString(IDS_SYM_MIRROR));
	}

	switch(i) {
	case 0: return &SOT_Center;
	}

	return NULL;
}

constexpr USHORT kChunkUseRampageWeldMath = 0x200;
constexpr USHORT kVersionChuck = 0x1000;
constexpr USHORT kWeldThresholdChuck = 0x1010;

// If we're loading from 2021 or earlier, map the legacy axis and flip
// values to the new PB entries
class FixSymPLCB : public PostLoadCallback {
public:
	SymmetryMod*	sm;
	FixSymPLCB(SymmetryMod *s) { sm = s; }
	void proc(ILoad *iload)
	{
		if (sm && (HIWORD(iload->GetFileSaveVersion()) <= MAX_RELEASE_R23))
		{
			sm->OnLegacyAxisOrFlip();
		}
		delete this;
	}
};

IOResult SymmetryMod::Load(ILoad *iload)
{
	IOResult res = Modifier::Load(iload);
	if (res != IO_OK)
		return res;

	iload->RegisterPostLoadCallback(new FixSymPLCB(this));

	iLoadVersion = OLD_SYMMETRY_VERSION;
	ULONG	nb;
	//original weld threshold value which caused some weird issue
	//so upped it but need to save it now so it is versioned
	mMirrorWeldThreshold = MIRRORWELD_THRESHOLD_2023_1;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID())
		{
			case kChunkUseRampageWeldMath:
				mUseRampageWeldMath = true;
				break;

			case kVersionChuck:
				res = iload->Read(&iLoadVersion, sizeof(int), &nb);
				break;
			case kWeldThresholdChuck:
				res = iload->Read(&mMirrorWeldThreshold, sizeof(float), &nb);
				break;
		}
		iload->CloseChunk();
		if (res!=IO_OK)  return res;
	}

	DWORD fileVer = iload->GetFileSaveVersion();
	DWORD hiFileVer = HIWORD(fileVer);
	if (hiFileVer <= MAX_RELEASE_R14)
	{
		mUseRampageWeldMath = true;
	}

	return IO_OK;
}

IOResult SymmetryMod::Save(ISave *isave)
{
	IOResult res = Modifier::Save(isave);
	if (res != IO_OK)
		return res;

	if (mUseRampageWeldMath)
	{
		isave->BeginChunk(kChunkUseRampageWeldMath);		
		isave->EndChunk();
	}

	ULONG nb;
	isave->BeginChunk(kVersionChuck);
	isave->Write(&iLoadVersion, sizeof(int), &nb);
	isave->EndChunk();

	isave->BeginChunk(kWeldThresholdChuck);
	isave->Write(&mMirrorWeldThreshold, sizeof(float), &nb);
	isave->EndChunk();

	return IO_OK;
}

bool SymmetryMod::SpecifySaveReferences(ReferenceSaveManager& referenceSaveManager)
{
	// If we're saving to 2021 or earlier, try to update legacy values to match
	DWORD saveVersion = GetSavingVersion();
	if (saveVersion != 0 && saveVersion <= MAX_RELEASE_R23)
	{
		SetLegacyAxisAndFlip();
	}
	return Modifier::SpecifySaveReferences(referenceSaveManager);
}

void SymmetryMod::ClearObject()
{
	mp_pblock->SetValue(kSymNodeRef, 0, (INode*)nullptr);
	if (transformMonitor) {
		transformMonitor->SetNode(nullptr);
		transformMonitor->SetForwardTransformChangeMsgs(false);
		if (mp_ip != nullptr) {
			mp_ip->RedrawViews(mp_ip->GetTime());
		}
	}	
}

void SymmetryMod::AlignToFace()
{
	if (mp_ip != nullptr && mp_ip->GetSubObjectLevel() != 0) {
		mp_ip->SetSubObjectLevel(0);
	}

	if (faceAlignMode) 
		faceAlignMode->AlignPressed();
}

