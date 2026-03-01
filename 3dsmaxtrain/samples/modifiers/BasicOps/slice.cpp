/**********************************************************************
 *<
	FILE: Slice.cpp

	DESCRIPTION: Slice Modifier

	CREATED BY: Steve Anderson

	HISTORY: created 11/18/97
			08/29/2020 - M. Kaustinen: PB2 conversion, QT, New Features.

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#include "slice.h"

#include "iparamm2.h"
#include "MaxIcon.h"
#include "MeshNormalSpec.h"
#include <ReferenceSaveManager.h>
#include "INodeTransformMonitor.h"
#include "../../maxsdk/samples/controllers/NodeTransformMonitor.h"

#include "FaceAlign.h"

#include "ui_Slice_Modifier.h"

// Capping support
#include "splshape.h"
#include "MeshDLib.h"
#include "RealWorldMapUtils.h"
#include <Geom/trig.h>


static GenSubObjType SOT_SlicePlane(26);

constexpr unsigned int PBLOCK_REF = 0;
constexpr unsigned int PLANE_REF = 1;
constexpr unsigned int	NODE_TRANSFORM_MONITOR_REF = 2;

class SliceMod : public Modifier, public ObjectAlignInterface {
public:
	IParamBlock2 *pblock2;
	Control *plane;

	// ParamBlock2 enums
	enum { slice_params };
	enum {
		slice_type,
		slice_face,
		slice_clean,
		slice_format,	// Planar/Radial
		// Planar Options
		slice_planar_x,
		slice_planar_y,
		slice_planar_z,
		slice_planar_flip_x,
		slice_planar_flip_y,
		slice_planar_flip_z,
		slice_planar_reserve,
		// Radial Options
		slice_radial_axis,
		slice_radial_angle1,
		slice_radial_angle2,
		// Node Reference
		slice_node_ref,
		// Radial
		slice_radial_type,
		// Capping
		slice_cap,
		slice_cap_material,
		slice_cap_matid,
	};
	// Slice types (planar)
	enum {
		planar_add_type,
		planar_split_type,
		planar_remove_top_type,
		planar_remove_bottom_type,
	};

	// Slice types (radial)
	enum {
		radial_remove_top_type,
		radial_remove_bottom_type,
	};

	// Face slice types
	enum {
		face_type_tri,
		face_type_poly,
		face_type_auto
	};
	// Slice Formats (Planar, Radial)
	enum {
		slice_planar,
		slice_radial,
	};

	static IObjParam *ip;
	static SliceMod *editMod;

	static MoveModBoxCMode *moveMode;
	static RotateModBoxCMode *rotMode;
	static UScaleModBoxCMode *uscaleMode;
	static NUScaleModBoxCMode *nuscaleMode;
	static SquashModBoxCMode *squashMode;

	static FaceAlignMode *faceAlignMode;

	SliceMod(BOOL bLoading);

	// From Animatable
	void DeleteThis() { delete this; }
	void GetClassName(MSTR& s, bool localized) const override { s = localized ? GetString(IDS_SLICEMOD) : _T("Slice"); }
	virtual Class_ID ClassID() { return SLICE_CLASS_ID;}
	void BeginEditParams(IObjParam  *ip, ULONG flags,Animatable *prev);
	void EndEditParams(IObjParam *ip,ULONG flags,Animatable *next);
	RefTargetHandle Clone(RemapDir& remap);
	IOResult Load (ILoad *iload);
	IOResult Save(ISave *isave);
	bool SpecifySaveReferences(ReferenceSaveManager& referenceSaveManager);
	const TCHAR *GetObjectName(bool localized) const override {return localized ? GetString(IDS_SLICEMOD) : _T("Slice");}
	BOOL AssignController(Animatable *control,int subAnim);
	int SubNumToRefNum(int subNum);

	// From modifier
	ChannelMask ChannelsUsed()  {return ALL_CHANNELS;}
	ChannelMask ChannelsChanged() {return ALL_CHANNELS-SUBSEL_TYPE_CHANNEL;}
	Class_ID InputType() { return mapObjectClassID; }
	void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
	Interval LocalValidity(TimeValue t);

	// From BaseObject
	int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc);
	int Display(TimeValue t, INode* inode, ViewExp *vpt, int flagst, ModContext *mc);
	void GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc);
	void Move (TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE );
	void Rotate (TimeValue t, Matrix3 & partm, Matrix3 & tmAxis, Quat & val, BOOL localOrigin=FALSE);
	void Scale (TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE);
	CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;}
	void GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
	void GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
	void ActivateSubobjSel(int level, XFormModes& modes);
	
	int NumSubObjTypes();
	ISubObjType *GetSubObjType(int i);
	
	int NumRefs() { return 3; }
	RefTargetHandle GetReference(int i);
private:
	virtual void SetReference(int i, RefTargetHandle rtarg);
public:

	virtual int    NumParamBlocks() { return 1; }     // return number of ParamBlocks in this instance
	virtual IParamBlock2* GetParamBlock(int /*i*/) { return pblock2; } // return i'th ParamBlock
	virtual IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock2->ID() == id) ? pblock2 : NULL; } // return id'd ParamBlock

	int NumSubs() {return 2;}
	Animatable* SubAnim(int i) {return GetReference(i);}
	TSTR SubAnimName(int i, bool localized) override;

	RefResult NotifyRefChanged( const Interval& changeInt,RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate);
	void NotifyInputChanged(const Interval& changeInt, PartID partID, RefMessage message, ModContext* mc);

	// Object Align Methods
	bool IsRemoveNegative() override
	{
		if (pblock2->GetInt(slice_format) != slice_planar)
			return false;

		return (pblock2->GetInt(slice_type) == planar_remove_bottom_type);
	}

	Matrix3	GetControllerTM(TimeValue t) override
	{
		Interval valid;
		Matrix3  tm;
		plane->GetValue(t, &tm, valid, CTRL_RELATIVE);
		return tm;
	}

	Matrix3	GetRefObjectTM(TimeValue t) override
	{
		Matrix3 tm;

		INode*	n = pblock2->GetINode(slice_node_ref);
		if (n) {
			tm = n->GetObjectTM(t);
		}
		return tm;
	}

	void	SetControllerPacket(TimeValue t, SetXFormPacket &sxfp) override
	{
		plane->SetValue(t, &sxfp, TRUE, CTRL_RELATIVE);
	}

	void	Move(TimeValue t, Point3 &p) override
	{
		Matrix3	m;
		Move(t, m, m, p);
	}

	bool	HasObject() override
	{
		return (pblock2->GetINode(slice_node_ref) != nullptr);
	}

	void ExitPick() override
	{
		if (mRollup)
			mRollup->ToggleAlignButton();

		// Force re-eval
		NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	}

	void	PreRotate(Matrix3& m) override
	{
		int		axis = -1;
		bool	bPlanar = (pblock2->GetInt(slice_format) == slice_planar);
		if (bPlanar) {
			if (pblock2->GetInt(slice_planar_x)) {
				axis = X_AXIS;
			} else if (pblock2->GetInt(slice_planar_y)) {
				axis = Y_AXIS;
			} else if (pblock2->GetInt(slice_planar_z)) {
				axis = Z_AXIS;
			}
		}
		else {
			// Radial option
			axis = pblock2->GetInt(slice_radial_axis);
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
		} else if (axis == Y_AXIS)
		{
			m.PreRotateX(HALFPI);
		}
		else if (axis == Z_AXIS)
		{
			m.PreRotateZ(PI);
		}
	};

	// Local Routines
	bool	IsLegacy() { return iLoadVersion == OLD_SLICE_VERSION; }
	void	DrawPlanarGizmos(GraphicsWindow* gw, const Matrix3& ptm, float size);
	void	DrawRadialGizmos(GraphicsWindow* gw, const Matrix3& ptm, float size, TimeValue t);
	Matrix3 CompMatrix(TimeValue t,INode *inode,ModContext *mc);
	void	AlignToFace();
	void	ClearObject();

	void	ClearGizmoTransform()
	{
		SetXFormPacket pckt(Matrix3{}, Matrix3{});
		plane->SetValue(0, &pckt, TRUE, CTRL_RELATIVE);
		// Force redraw
		if (ip != nullptr) {
			ip->RedrawViews(ip->GetTime());
		}
	}

	// Mesh capping
	BOOL	CreatePolyFromMeshEdges(Mesh & mesh, PolyShape& ps, AdjEdgeList *ae, const Matrix3 &m);
	BOOL	CreatePolyFromMNMeshEdges(MNMesh & mesh, PolyShape& ps, MNMeshBorder &mb, const Matrix3 &m);
	void	MakeMeshCapTexture(Mesh& mesh,const Point3& pCenter, float dUV, const Matrix3& itm, int fstart, int fend);
	void	MakeMNMeshCapTexture(MNMesh& mesh, const Point3& pCenter, float dUV, const Matrix3& itm, int fstart, int fend);
	Tab<int>		meshMapping;

	SliceRollup* mRollup;

	// Versioning for old loads
	int		iLoadVersion;

	// Track our movements in cases where there are reference nodes
	NodeTransformMonitor* transformMonitor;
};

//--- ClassDescriptor and class vars ---------------------------------

IObjParam       *SliceMod::ip        = NULL;
SliceMod     *SliceMod::editMod   = NULL;

MoveModBoxCMode *SliceMod::moveMode  = NULL;
RotateModBoxCMode *SliceMod::rotMode  = NULL;
UScaleModBoxCMode *SliceMod::uscaleMode  = NULL;
NUScaleModBoxCMode *SliceMod::nuscaleMode  = NULL;
SquashModBoxCMode *SliceMod::squashMode  = NULL;

FaceAlignMode*      SliceMod::faceAlignMode = nullptr;

class SliceClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return 1; }
	void*			Create(BOOL loading = FALSE) { return new SliceMod(loading); }
	const TCHAR*	ClassName() { return GetString(IDS_SLICEMOD); }
	const TCHAR*	NonLocalizedClassName() { return _T("Slice"); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return SLICE_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_MAX_STANDARD); }

	// ClassDesc2 adders
	virtual const TCHAR*	InternalName() { return _T("Slice"); }	// returns fixed parsable name (scripter-visible name)
	virtual bool UseOnlyInternalNameForMAXScriptExposure() { return true; }
	virtual HINSTANCE HInstance() { return hInstance; }			// returns owning module handle

	virtual MaxSDK::QMaxParamBlockWidget* CreateQtWidget(ReferenceMaker& owner, IParamBlock2& paramBlock, const MapID, MSTR& rollupTitle, int&, int&) override
	{
		if (paramBlock.ID() == SliceMod::slice_params) {
			SliceRollup*rr = new SliceRollup();
			((SliceMod&)owner).mRollup = rr;
			rr->SetParamBlock(&owner, &paramBlock);
			//: this is a comment for the translation team 
			rollupTitle = SliceRollup::tr("Slice");
			return rr;
		}
		return nullptr;
	}
};

static SliceClassDesc sliceDesc;
extern ClassDesc* GetSliceModDesc() {return &sliceDesc;}

// --- QT Setup ----------------------------

SliceRollup::SliceRollup(QWidget* /*parent*/) :
	QMaxParamBlockWidget(),
	m_UI(new Ui::SliceRollup())
{
	mMod = nullptr;
	m_UI->setupUi(this);

	// Set up Slice Format
	if (m_UI->SliceFormat)
	{
		m_UI->SliceFormat->clear();
		m_UI->SliceFormat->addItem(tr("Planar"), QVariant(SliceMod::slice_planar));
		m_UI->SliceFormat->addItem(tr("Radial"), QVariant(SliceMod::slice_radial));
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
	mActionReset->setStatusTip(tr("Reset transform"));
	// Setup Align to Face button
	connect(m_UI->AlignToFace, SIGNAL(clicked()), this, SLOT(AlignToFace()));
	connect(m_UI->ResetTransform, SIGNAL(clicked()), this, SLOT(ResetTransform()));
	m_UI->ResetTransform->setIcon(MaxSDK::LoadMaxMultiResIcon(MSTR(_T("StateSets\\Refresh.png"))));

	// We need a custom right-click menu for object picker
	m_UI->ReferenceObject->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_UI->ReferenceObject, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(ObjectRightClick(const QPoint&)));
	mActionClear = new QAction(tr("Clear"), this);
	mActionClear->setStatusTip(tr("Clear the reference object"));

	// Set up Radial Axis Enum
	if (m_UI->RadialAxis)
	{
		m_UI->RadialAxis->setId(m_UI->RadialX, X_AXIS);
		m_UI->RadialAxis->setId(m_UI->RadialY, Y_AXIS);
		m_UI->RadialAxis->setId(m_UI->RadialZ, Z_AXIS);
	}

	// Setup Planar Slice Type Enum
	if (m_UI->Slice_Type)
	{
		m_UI->Slice_Type->setId(m_UI->refineMesh_radio, SliceMod::planar_add_type);
		m_UI->Slice_Type->setId(m_UI->splitMesh_radio, SliceMod::planar_split_type);
		m_UI->Slice_Type->setId(m_UI->removeTop_radio, SliceMod::planar_remove_top_type);
		m_UI->Slice_Type->setId(m_UI->removeBottom_radio, SliceMod::planar_remove_bottom_type);
	}

	// Setup Radial Slice Type Enum
	if (m_UI->Radial_Type)
	{
		m_UI->Radial_Type->setId(m_UI->radial_removeTop_radio, SliceMod::radial_remove_top_type);
		m_UI->Radial_Type->setId(m_UI->radial_removeBottom_radio, SliceMod::radial_remove_bottom_type);
	}

	// Update the Operate On drop list
	if (m_UI->Faces___Polygons_Toggle)
	{
		m_UI->Faces___Polygons_Toggle->clear();
		m_UI->Faces___Polygons_Toggle->addItem(tr("Automatic"), QVariant(SliceMod::face_type_auto));
		m_UI->Faces___Polygons_Toggle->addItem(tr("Poly"), QVariant(SliceMod::face_type_poly));
		m_UI->Faces___Polygons_Toggle->addItem(tr("Mesh"), QVariant(SliceMod::face_type_tri));
	}
}

SliceRollup::~SliceRollup(void)
{
	if (mMod)
		mMod->mRollup = nullptr;

	delete m_UI;
}

void SliceRollup::SetParamBlock(ReferenceMaker* owner, IParamBlock2* const /*param_block*/)
{
	mMod = (SliceMod*)owner;
}

void SliceRollup::UpdateUI(const TimeValue t)
{
	EnableControls(t);
}

void SliceRollup::UpdateParameterUI(const TimeValue t, const ParamID param_id, const int tab_index)
{
	if ((param_id == SliceMod::slice_format) || (param_id == SliceMod::slice_type) || (param_id == SliceMod::slice_cap) ||
		(param_id == SliceMod::slice_planar_x) || (param_id == SliceMod::slice_planar_y) ||	(param_id == SliceMod::slice_planar_z) ||
		(param_id == SliceMod::slice_cap_material))
	{
		EnableControls(t);
	}
	else if (param_id == SliceMod::slice_node_ref)
		UpdatePickButton();
}

void SliceRollup::EnableControls(const TimeValue t)
{
	if (!mMod || !(mMod->pblock2))
		return;

	// Update Slice Format
	bool bRadial = mMod->pblock2->GetInt(SliceMod::slice_format) == SliceMod::slice_radial;
	m_UI->RadialAxis_groupBox->setHidden(!bRadial);
	m_UI->RadialSlice_groupBox->setHidden(!bRadial);
	m_UI->PlanarAxis_groupBox->setHidden(bRadial);
	// Radio options
	m_UI->refineMesh_radio->setHidden(bRadial);
	m_UI->splitMesh_radio->setHidden(bRadial);
	m_UI->removeBottom_radio->setHidden(bRadial);
	m_UI->removeTop_radio->setHidden(bRadial);
	m_UI->radial_removeTop_radio->setHidden(!bRadial);
	m_UI->radial_removeBottom_radio->setHidden(!bRadial);

	// Update Planar specific UI elements
	if (!bRadial)
	{
		// Flip
		bool	bSliceX = (mMod->pblock2->GetInt(SliceMod::slice_planar_x) != 0);
		bool	bSliceY = (mMod->pblock2->GetInt(SliceMod::slice_planar_y) != 0);
		bool	bSliceZ = (mMod->pblock2->GetInt(SliceMod::slice_planar_z) != 0);
		m_UI->PlanarFlipX->setEnabled(bSliceX);
		m_UI->PlanarFlipY->setEnabled(bSliceY);
		m_UI->PlanarFlipZ->setEnabled(bSliceZ);

		// Cap
		int		iType = mMod->pblock2->GetInt(SliceMod::slice_type);
		bool	bCapable = (iType == SliceMod::planar_remove_top_type) || (iType == SliceMod::planar_remove_bottom_type);
		m_UI->Cap->setEnabled(bCapable);

		bCapable &= (mMod->pblock2->GetInt(SliceMod::slice_cap) != 0);
		m_UI->SetMaterial->setEnabled(bCapable);
		bCapable &= (mMod->pblock2->GetInt(SliceMod::slice_cap_material) != 0);
		m_UI->MaterialIDL->setEnabled(bCapable);
		m_UI->MaterialID->setEnabled(bCapable);
	}
	else {
		m_UI->Cap->setEnabled(true);
		bool bCapable = (mMod->pblock2->GetInt(SliceMod::slice_cap) != 0);
		m_UI->SetMaterial->setEnabled(bCapable);
		bCapable = (mMod->pblock2->GetInt(SliceMod::slice_cap) != 0);
		(mMod->pblock2->GetInt(SliceMod::slice_cap_material) != 0);
		m_UI->MaterialIDL->setEnabled(bCapable);
		m_UI->MaterialID->setEnabled(bCapable);
	}
	UpdatePickButton();
}

void SliceRollup::UpdatePickButton()
{
	if (mMod && !mMod->HasObject())
	{
		m_UI->ReferenceObject->setText(tr("Pick Object"));
	}
}

void SliceRollup::AlignToFace()
{
	if (mMod)
		mMod->AlignToFace();
}

void SliceRollup::ToggleAlignButton()
{
	m_UI->AlignToFace->setChecked(false);
}

void SliceRollup::ResetTransform()
{
	if (mMod)
		mMod->ClearObject();
}

void	SliceRollup::ObjectRightClick(const QPoint & pos)
{
	QMenu menu;
	menu.addAction(mActionClear);

	QAction* selectedItem = menu.exec(m_UI->ReferenceObject->mapToGlobal(pos));

	if (selectedItem == mActionClear)
	{
		mMod->ClearObject();
	}
}

void	SliceRollup::AlignRightClick(const QPoint & pos)
{
	QMenu menu;
	menu.addAction(mActionReset);

	QAction* selectedItem = menu.exec(m_UI->ReferenceObject->mapToGlobal(pos));

	if (selectedItem == mActionReset)
	{
		mMod->ClearGizmoTransform();
	}
}

//--- Parameter map/block descriptors (Legacy) ----------------------------

#define PB_TYPE  0
#define PB_POLY  1
#define PB_CLEAN 2

static ParamBlockDescID descVer0[] = {
	{ TYPE_INT, NULL, FALSE, PB_TYPE },		// Type
};

static ParamBlockDescID descVer1[] = {
	{ TYPE_INT, NULL, FALSE, PB_TYPE },		// Type
	{ TYPE_INT, NULL, FALSE, PB_POLY },		// Ignore hidden edges
};

static ParamBlockDescID descVer2[] = {
	{ TYPE_INT, NULL, FALSE, PB_TYPE },		// Type
	{ TYPE_INT, NULL, FALSE, PB_POLY },		// Ignore hidden edges
	{ TYPE_INT, NULL, FALSE, PB_CLEAN },	// Clean dead structs
};

// Array of old versions, including last PB1 version
static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer0, 1, 0),
	ParamVersionDesc(descVer1, 2, 1),
	ParamVersionDesc(descVer2, 3, 2)
};

#define PBLOCK_LENGTH	3
#define NUM_OLDVERSIONS 3
#define CURRENT_VERSION	2
#define VERSION_FOR_2019	1
#define LENGTH_FOR_2019	2

//-----------------------------------------------------------

class SlicePBAccessor : PBAccessor
{
	virtual void Set(PB2Value& /*v*/, ReferenceMaker* owner, ParamID id, int /*tabIndex*/, TimeValue /*t*/) override
	{
		if (id == SliceMod::slice_node_ref)
		{
			SliceMod* m = (SliceMod*)owner;
			if (m)
				m->ClearGizmoTransform();
		}
		else {
			Interface* ip = GetCOREInterface();
			ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
		}
	}
};
static SlicePBAccessor s_accessor;

static ParamBlockDesc2 slice_param_blk(SliceMod::slice_params, _T("Slice Parameters"), 0, &sliceDesc, P_AUTO_CONSTRUCT + P_AUTO_UI_QT, PBLOCK_REF,
	// params
	SliceMod::slice_type, _T("Slice_Type"), TYPE_INT, P_RESET_DEFAULT, IDS_SLICE_TYPE,
	p_default,		SliceMod::planar_add_type,
	p_range,		SliceMod::planar_add_type, SliceMod::planar_remove_bottom_type,
	p_accessor, &s_accessor,
	p_nonLocalizedName, _T("Slice_Type"),
	p_end,

	SliceMod::slice_face, _T("Faces___Polygons_Toggle"), TYPE_INT, P_RESET_DEFAULT, IDS_FACE_TYPE,
	p_default, SliceMod::face_type_auto,
	p_range, SliceMod::face_type_tri, SliceMod::face_type_auto,
	p_accessor, &s_accessor,
	p_nonLocalizedName, _T("Faces / Polygons Toggle"),
	p_end,

	// Maintained here for backwards compatibility
	SliceMod::slice_clean, _T("Clean"), TYPE_BOOL, P_RESET_DEFAULT, IDS_SLICE_CLEAN,
	p_default, TRUE,
	p_accessor, &s_accessor,
	p_nonLocalizedName, _T("Clean"),
	p_end,

	// New features
	SliceMod::slice_format, _T("SliceFormat"), TYPE_INT, P_RESET_DEFAULT, IDS_SLICE_FORMAT,
	p_default, SliceMod::slice_planar,
	p_range, SliceMod::slice_planar, SliceMod::slice_radial,
	p_accessor, &s_accessor,
	p_nonLocalizedName, _T("SliceFormat"),
	p_end,

	SliceMod::slice_planar_x, _T("PlanarX"), TYPE_BOOL, P_RESET_DEFAULT, IDS_PLANARX,
	p_default, FALSE,
	p_accessor, &s_accessor,
	p_nonLocalizedName, _T("PlanarX"),
	p_end,

	SliceMod::slice_planar_y, _T("PlanarY"), TYPE_BOOL, P_RESET_DEFAULT, IDS_PLANARY,
	p_default, FALSE,
	p_accessor, &s_accessor,
	p_nonLocalizedName, _T("PlanarY"),
	p_end,

	SliceMod::slice_planar_z, _T("PlanarZ"), TYPE_BOOL, P_RESET_DEFAULT, IDS_PLANARZ,
	p_default, TRUE,
	p_accessor, &s_accessor,
	p_nonLocalizedName, _T("PlanarZ"),
	p_end,

	SliceMod::slice_planar_flip_x, _T("PlanarFlipX"), TYPE_BOOL, P_RESET_DEFAULT, IDS_PLANAR_FLIPX,
	p_default, FALSE,
	p_accessor, &s_accessor,
	p_nonLocalizedName, _T("PlanarFlipX"),
	p_end,

	SliceMod::slice_planar_flip_y, _T("PlanarFlipY"), TYPE_BOOL, P_RESET_DEFAULT, IDS_PLANAR_FLIPY,
	p_default, FALSE,
	p_accessor, &s_accessor,
	p_nonLocalizedName, _T("PlanarFlipY"),
	p_end,

	SliceMod::slice_planar_flip_z, _T("PlanarFlipZ"), TYPE_BOOL, P_RESET_DEFAULT, IDS_PLANAR_FLIPZ,
	p_default, FALSE,
	p_accessor, &s_accessor,
	p_nonLocalizedName, _T("PlanarFlipZ"),
	p_end,

	SliceMod::slice_radial_axis, _T("RadialAxis"), TYPE_INT, P_RESET_DEFAULT, IDS_RADIAL_AXIS,
	p_default, Z_AXIS,
	p_range, X_AXIS, Z_AXIS,
	p_accessor, &s_accessor,
	p_nonLocalizedName, _T("RadialAxis"),
	p_end,

	SliceMod::slice_radial_angle1, _T("Angle1"), TYPE_FLOAT, P_RESET_DEFAULT | P_ANIMATABLE, IDS_RADIAL_ANGLE1,
	p_default, 0.0f,
	p_range, 0.0f, 180.0f,
	p_accessor, &s_accessor,
	p_nonLocalizedName, _T("Angle1"),
	p_end,

	SliceMod::slice_radial_angle2, _T("Angle2"), TYPE_FLOAT, P_RESET_DEFAULT | P_ANIMATABLE, IDS_RADIAL_ANGLE2,
	p_default, 180.0f,
	p_range, 0.0f, 180.0f,
	p_accessor, &s_accessor,
	p_nonLocalizedName, _T("Angle2"),
	p_end,

	SliceMod::slice_node_ref, _T("ReferenceObject"), TYPE_INODE, P_AUTO_UI, IDS_REFERENCE_OBJECT,
	p_accessor, &s_accessor,
	p_nonLocalizedName, _T("ReferenceObject"),
	p_end,

	SliceMod::slice_radial_type, _T("Radial_Type"), TYPE_INT, P_RESET_DEFAULT, IDS_RADIAL_TYPE,
	p_default, SliceMod::radial_remove_top_type,
	p_range, SliceMod::radial_remove_top_type, SliceMod::radial_remove_bottom_type,
	p_accessor, &s_accessor,
	p_nonLocalizedName, _T("Radial_Type"),
	p_end,

	SliceMod::slice_cap, _T("Cap"), TYPE_BOOL, P_RESET_DEFAULT, IDS_CAP_HOLES,
	p_default, FALSE,
	p_accessor, &s_accessor,
	p_nonLocalizedName, _T("Cap"),
	p_end,

	SliceMod::slice_cap_material, _T("SetMaterial"), TYPE_BOOL, P_RESET_DEFAULT, IDS_SET_MATERIAL,
	p_default, FALSE,
	p_accessor, &s_accessor,
	p_nonLocalizedName, _T("SetMaterial"),
	p_end,

	SliceMod::slice_cap_matid, _T("MaterialID"), TYPE_INT, P_RESET_DEFAULT, IDS_MATERIAL_ID,
	p_default, 1,
	p_range, 1, 65535,
	p_accessor, &s_accessor,
	p_nonLocalizedName, _T("MaterialID"),
	p_end,

	p_end
);


//--- Slice mod methods -------------------------------

SliceMod::SliceMod(BOOL bLoading = FALSE) :
	plane(nullptr),
	pblock2(nullptr),
	mRollup(nullptr),
	transformMonitor(nullptr)
{
	sliceDesc.MakeAutoParamBlocks(this);

	if (!bLoading) {
		iLoadVersion = NEW_SLICE_VERSION;
		ReplaceReference(PLANE_REF, NewDefaultMatrix3Controller());
	}
}

void SliceMod::BeginEditParams (IObjParam  *ip, ULONG flags,Animatable *prev)
{
	this->ip = ip;
	editMod  = this;

	// Create sub object editing modes.
	moveMode = new MoveModBoxCMode (this,ip);
	rotMode = new RotateModBoxCMode (this,ip);
	uscaleMode = new UScaleModBoxCMode (this,ip);
	nuscaleMode = new NUScaleModBoxCMode (this,ip);
	squashMode = new SquashModBoxCMode (this, ip);
	faceAlignMode = new FaceAlignMode(this, this, ip);

	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);

	sliceDesc.BeginEditParams(ip, this, flags, prev);
}

void SliceMod::EndEditParams(IObjParam *ip,ULONG flags,Animatable *next)
{
	sliceDesc.EndEditParams(ip, this, flags, next);

	this->ip = NULL;
	editMod  = NULL;

	TimeValue t = ip->GetTime();

	// NOTE: This flag must be cleared before sending the REFMSG_END_EDIT
	ClearAFlag(A_MOD_BEING_EDITED);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);

	ip->DeleteMode(moveMode);
	if (moveMode) delete moveMode;
	moveMode = NULL;
	ip->DeleteMode(rotMode);
	if (rotMode) delete rotMode;
	rotMode = NULL;
	ip->DeleteMode(uscaleMode);
	if (uscaleMode) delete uscaleMode;
	uscaleMode = NULL;
	ip->DeleteMode(nuscaleMode);
	if (nuscaleMode) delete nuscaleMode;
	nuscaleMode = NULL;
	ip->DeleteMode(squashMode);
	if (squashMode) delete squashMode;
	squashMode = NULL;
	ip->DeleteMode(faceAlignMode);
	if (faceAlignMode) delete faceAlignMode;
	faceAlignMode = nullptr;
}

RefTargetHandle SliceMod::Clone(RemapDir& remap)
{
	SliceMod *mod = new SliceMod();
	mod->ReplaceReference(PBLOCK_REF,remap.CloneRef(pblock2));
	mod->ReplaceReference(PLANE_REF,remap.CloneRef(plane));
	mod->ReplaceReference(NODE_TRANSFORM_MONITOR_REF, remap.CloneRef(transformMonitor));
	BaseClone(this, mod, remap);
	return mod;
}

// MAXX-59793
// Since the "clean" parameter defaults to TRUE in PB2, when loading a file from 2019 or earlier the
// default value will also be TRUE. However, in PB1, the default behaviour is to set it to FALSE if it
// doesn't exist. For backwards compatability with loading old files (2019), we need to manually set the
// "clean" parameter 0 (FALSE) as a postload action.
//
class FixSlicePLCB : public PostLoadCallback {
public:
	SliceMod*	sm;
	FixSlicePLCB(SliceMod *s) { sm = s; }
	void proc(ILoad *iload)
	{
		if (sm && (HIWORD(iload->GetFileSaveVersion()) <= MAX_RELEASE_R21))
		{
			sm->pblock2->SetValue(SliceMod::slice_clean, 0, FALSE);
		}
		delete this;
	}
};

// Version management
#define	VERSION_CHUNK	0x1000

IOResult SliceMod::Load(ILoad *iload)
{
	// Load the base class first
	IOResult res = Modifier::Load(iload);
	if (res != IO_OK)
		return res;

	ParamBlock2PLCB* plcb = new ParamBlock2PLCB(versions, NUM_OLDVERSIONS, &slice_param_blk, this, PBLOCK_REF);
	iload->RegisterPostLoadCallback(plcb);
	iload->RegisterPostLoadCallback(new FixSlicePLCB(this));

	// Load File version, set to legacy to see if it is overwritten
	iLoadVersion = OLD_SLICE_VERSION;

	ULONG	nb;
	while (IO_OK == (res = iload->OpenChunk())) {
		switch (iload->CurChunkID()) {
		case VERSION_CHUNK:
			res = iload->Read(&iLoadVersion, sizeof(int), &nb);
			break;
		}
		iload->CloseChunk();
		if (res != IO_OK)
			return res;
	}

	return IO_OK;
}

IOResult SliceMod::Save(ISave *isave)
{
	// Save the baseclass stuff first
	IOResult res = Modifier::Save(isave);
	if (res != IO_OK)
		return res;

	ULONG nb;

	isave->BeginChunk(VERSION_CHUNK);
	isave->Write(&iLoadVersion, sizeof(int), &nb);
	isave->EndChunk();

	return IO_OK;
}

bool SliceMod::SpecifySaveReferences(ReferenceSaveManager& referenceSaveManager)
{
	// if saving to previous version that used pb1 instead of pb2...
	DWORD saveVersion = GetSavingVersion();
	if (saveVersion != 0 && saveVersion <= MAX_RELEASE_R23)
	{
		ProcessPB2ToPB1SaveToPrevious(this, pblock2, PBLOCK_REF, descVer2, PBLOCK_LENGTH, CURRENT_VERSION);
	}
	return __super::SpecifySaveReferences(referenceSaveManager);
}

// Create a polyshape from border edges of an MNMesh
BOOL SliceMod::CreatePolyFromMNMeshEdges(MNMesh & mesh, PolyShape& pShape, MNMeshBorder &mb, const Matrix3 &m)
{
	meshMapping.ZeroCount();
	if (mb.Num() < 1)
		return FALSE;

	pShape.SetNumLines(0, FALSE);
	int			ipolys = 0;

	for (int i = 0; i < mb.Num(); i++)
	{
		if (!mb.LoopTarg(i))
			continue;

		IntTab* intLoop = mb.Loop(i);
		int		thisCount = intLoop->Count();
		// ignore single edge folding back onto itself
		if (thisCount > 1)
		{
			PolyLine pLine;

			int	thisCount1 = thisCount -1;

			// Add the first entry
			int	thisEdge = (*intLoop)[0];
			int	thisVtx = mesh.e[thisEdge].v1;

			Point3	priorPoint = m.PointTransform(mesh.v[thisVtx].p);
			Point3	firstPoint = priorPoint;

			meshMapping.Append(1, &thisVtx);
			PolyPt	pt(priorPoint);
			pLine.Append(pt);

			// Add the remaining entries
			for (int j =1; j <= thisCount1; j++)
			{
				thisEdge = (*intLoop)[j];
				DbgAssert(thisEdge < mesh.nume);

				thisVtx = mesh.e[thisEdge].v1;
				DbgAssert(thisVtx < mesh.numv);

				Point3	thisPoint = m.PointTransform(mesh.v[thisVtx].p);

				// Make sure the points are not on top of eachother (in x and y) in which case we should skip
				if ( (j == 0) || (fabs(thisPoint.x - priorPoint.x) >= MNEPS) || (fabs(thisPoint.y - priorPoint.y) >= MNEPS) )
				{
					// Special case, the last entry if the same as the first, skip
					if ((j == thisCount1) && (fabs(thisPoint.x - firstPoint.x) < MNEPS) && (fabs(thisPoint.y - firstPoint.y) < MNEPS))
							break;

					meshMapping.Append(1, &thisVtx);

					pt = PolyPt(thisPoint);
					pLine.Append(pt);
				}
				priorPoint = thisPoint;
			}

			if (pLine.numPts > 2) {
				pLine.Close();
				pLine.SetNoSelfInt();
				pShape.Append(pLine);
			}
			else {
				// Remove the entries for our map
				int mapCount = meshMapping.Count();
				if (mapCount && pLine.numPts)
					meshMapping.Delete(mapCount - pLine.numPts, pLine.numPts);
			}
		}
	}
	if (pShape.numLines < 1)
		return FALSE;
	
	pShape.InvalidateGeomCache(FALSE);
	return TRUE;
}

// Create a polyshapes from edges along selected vertices of a mesh
BOOL SliceMod::CreatePolyFromMeshEdges (Mesh & mesh, PolyShape& pShape, AdjEdgeList *ae, const Matrix3& m)
{
	meshMapping.ZeroCount();
	pShape.SetNumLines(0, FALSE);

	if (mesh.VertSel().IsEmpty())
		return FALSE;

	// Edge processed flag
	BitArray done(ae->edges.Count());

	// Map based on the edges we find
	IntTab		edgeVertMaping;

	BitArray& vertSel = mesh.VertSel();
	for (int i = 0; i < ae->edges.Count(); i++)
	{
		if (done[i])
			continue;

		// Mark this edge as done
		done.Set(i);

		int	v0 = ae->edges[i].v[0];
		int v1 = ae->edges[i].v[1];

		// Check that the edge is associated with both selected vertexes
		if (!vertSel[v0] || !(vertSel[v1]))
			continue;

		edgeVertMaping.ZeroCount();
		edgeVertMaping.Append(1, &v0);
		edgeVertMaping.Append(1, &v1);

		int nextv = v1, start = v0;

		while (1) {
			DWORDTab &ve = ae->list[nextv];
			int j;
			for (j = 0; j < ve.Count(); j++)
			{
				if (done[ve[j]])
					continue;

				// Check that the edge is associated with both selected vertexes
				if (vertSel[ae->edges[ve[j]].v[0]] && vertSel[ae->edges[ve[j]].v[1]])
					break;
			}
			if (j == ve.Count())
				break;

			// Mark this edge as done
			done.Set(ve[j]);

			if (ae->edges[ve[j]].v[0] == nextv)
				nextv = (int)ae->edges[ve[j]].v[1];
			else
				nextv = (int)ae->edges[ve[j]].v[0];

			edgeVertMaping.Append(1, &nextv);
		}
		int lastV = nextv;

		// Now trace backwards
		nextv = start;
		while (1) {
			DWORDTab &ve = ae->list[nextv];
			int j;
			for (j = 0; j < ve.Count(); j++) {
				if (done[ve[j]])
					continue;

				// Check that the edge is associated with both selected vertexes
				if (vertSel[ae->edges[ve[j]].v[0]] && vertSel[ae->edges[ve[j]].v[1]])
					break;
			}
			if (j == ve.Count()) break;

			// Mark this edge as done
			done.Set(ve[j]);

			if (ae->edges[ve[j]].v[0] == nextv)
				nextv = (int)ae->edges[ve[j]].v[1];
			else
				nextv = (int)ae->edges[ve[j]].v[0];

			edgeVertMaping.Insert(0, 1, &nextv);
		}

		// Only interested if processing if we're closed and there is at least 4 verts (including the return vert)
		int		edgeVertCount = edgeVertMaping.Count();
		if (edgeVertCount >= 4 && (nextv == lastV))
		{
			// Add our points, provided that they are not too close to each other
			
			// Ignore the last entry (closed)
			edgeVertCount--;
			edgeVertMaping.Delete(edgeVertCount, 1);

			// build our polyline
			PolyLine		pLine;

			// First entry
			Point3		p3First = m.PointTransform(mesh.verts[edgeVertMaping[0]]);
			pLine.Append(p3First);
			meshMapping.Append(1, &(edgeVertMaping[0]));

			Point3	p3Prior = p3First;
			for (int e = 1; e < edgeVertCount; e++)
			{
				Point3	p3This = m.PointTransform(mesh.verts[edgeVertMaping[e]]);
				if ((fabs(p3This.x - p3Prior.x) >= MNEPS) || (fabs(p3This.y - p3Prior.y) >= MNEPS))
				{
					PolyPt		pt(p3This);
					pLine.Append(pt);
					meshMapping.Append(1, &(edgeVertMaping[e]));
				}
				p3Prior = p3This;
			}

			// check that the last value isn't too close to the first
			if ((fabs(p3First.x - p3Prior.x) < MNEPS) && (fabs(p3First.y - p3Prior.y) < MNEPS))
			{
				meshMapping.Delete(meshMapping.Count() - 1, 1);
				pLine.Delete(pLine.numPts - 1);
			}

			// Do we have 3 verts or more?  If so, append polyline to the shape
			if (pLine.numPts >= 3)
			{
				pLine.Close();
				pLine.SetNoSelfInt();
				pShape.Append(pLine);
			}
		}
	}

	if (pShape.numLines < 1)
		return FALSE;

	pShape.InvalidateGeomCache(FALSE);
	return TRUE;
}

void SliceMod::MakeMeshCapTexture(Mesh& mesh, const Point3& p3Center, float dUV, const Matrix3& itm, int fstart, int fend)
{
	if (fstart == fend)
		return;

	// Find out which verts are used by the capping faces
	BitArray capVerts(mesh.numVerts);

	for (int i = fstart; i < fend; ++i)
	{
		Face& f = mesh.faces[i];
		capVerts.Set(f.v[0]);
		capVerts.Set(f.v[1]);
		capVerts.Set(f.v[2]);
	}

	// Expand our TVverts to include entries for these referenced verts
	int	numCapVerts = capVerts.NumberSet();
	int baseTVert = mesh.getNumTVerts();
	mesh.setNumTVerts(baseTVert + numCapVerts, TRUE);

	IntTab capIndexes;
	capIndexes.SetCount(mesh.numVerts);

	Point3 p3UV = Point3(dUV, dUV, dUV);
	Point3	UVCenter = (p3Center * itm) * 0.5f; 

	// Do the TVerts
	for (int i = 0; i < mesh.numVerts; ++i)
	{
		if (capVerts[i]) {
			capIndexes[i] = baseTVert;
			mesh.setTVert(baseTVert, ((mesh.verts[i] * itm) + UVCenter) * p3UV);
			baseTVert++;
		}
	}

	// Do the TVFaces
	for (int i = fstart; i < fend; ++i)
	{
		Face& f = mesh.faces[i];
		mesh.tvFace[i] = TVFace(capIndexes[f.v[0]], capIndexes[f.v[1]], capIndexes[f.v[2]]);
	}
}

void SliceMod::MakeMNMeshCapTexture(MNMesh& mesh, const Point3& p3Center, float dUV, const Matrix3& itm, int fstart, int fend)
{
	if (fstart == fend)
		return;

	const	int MapChannel = 1;
	MNMap*	mmp = mesh.M(MapChannel);

	DbgAssert(mmp && !mmp->GetFlag(MN_DEAD));
	DbgAssert(mmp->numf >= fend);

	// Find out which verts are used by the capping faces
	BitArray capVerts(mesh.numv);

	for (int i = fstart; i < fend; ++i)
	{
		MNFace& f = mesh.f[i];
		int	deg = f.deg;
		for (int j = 0; j < deg; j++)
			capVerts.Set(f.vtx[j]);
	}
	
	// Expand our TVverts to include entries for these referenced verts
	int	numCapVerts = capVerts.NumberSet();
	int baseTVert = mmp->numv;
	mmp->setNumVerts(baseTVert + numCapVerts);

	IntTab capIndexes;
	capIndexes.SetCount(mesh.numv);

	Point3 p3UV = Point3(dUV, dUV, dUV);
	Point3	UVCenter = (p3Center * itm) * 0.5f;

	// Do the TVerts
	for (int i = 0; i < mesh.numv; ++i)
	{
		if (capVerts[i]) {
			capIndexes[i] = baseTVert;
			mmp->v[baseTVert] = ((mesh.v[i].p * itm) + UVCenter) * p3UV;
			baseTVert++;
		}
	}

	// Do the TVFaces
	for (int i = fstart; i < fend; ++i)
	{
		MNMapFace& tvf = mmp->f[i];
		MNFace& mmf = mesh.f[i];
		DbgAssert(tvf.deg == mmf.deg);

		for (int j = 0; j < mmf.deg; j++)
		{
			tvf.tv[j] = capIndexes[mmf.vtx[j]];
		}
	}
}

static void MatrixFromNormalAndOffset(const Point3& normal, const float &off, Matrix3& mat)
{
	Point3 vx;
	vx.z = .0f;
	vx.x = -normal.y;
	vx.y = normal.x;
	if (vx.x == .0f && vx.y == .0f) {
		vx.x = 1.0f;
	}
	mat.SetRow(0, vx);
	mat.SetRow(1, normal^vx);
	mat.SetRow(2, normal);
	mat.PreTranslate(Point3(0.0f, 0.0f, off));
	mat.NoScale();
}

void SliceMod::ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node)
{
	if (!os->obj->CanConvertToType(triObjectClassID))
		return;

	TriObject *tobj = nullptr;
	PolyObject *pobj = nullptr;
	Interval iv = FOREVER;

	int outputType, clean;

	pblock2->GetValue(slice_face, t, outputType, iv);
	pblock2->GetValue(slice_clean, t, clean, iv);

	// Slice Format
	int		iSliceFormat = pblock2->GetInt(slice_format);
	if (iSliceFormat != slice_radial)
		iSliceFormat = slice_planar;

	// Slicing Plane(s) and Offset(s), we can have up to 3
	Point3	slicePlanes[3];
	float	sliceOffsets[3];
	bool	sliceFlips[3] = { false, false, false };
	int		numSlices = 0;

	// Transform Matrix for slicing planes
	Matrix3 tm;

	INode*	objRef = pblock2->GetINode(slice_node_ref);
	if (objRef)
	{
		tm = objRef->GetObjectTM(t, &iv);

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

	plane->GetValue(t, &tm, iv, CTRL_RELATIVE);
	
	if (mc.tm)
		tm = tm * Inverse(*mc.tm);

	Point3 origin = tm.GetTrans();

	bool	bDeleteAll = false;

	// Get the slice types
	int		planar_type = planar_add_type;
	pblock2->GetValue(slice_type, t, planar_type, iv);

	int		radial_type = radial_remove_top_type;
	pblock2->GetValue(slice_radial_type, t, radial_type, iv);

	bool bSplit = (iSliceFormat == slice_radial) ? false : planar_type == planar_split_type;
	bool bRemove = (iSliceFormat == slice_radial) ? true : planar_type >= planar_remove_top_type;

	// Flag Hole Capping (but only if removing)
	bool	bCapHoles = (pblock2->GetInt(slice_cap) != FALSE);
	if (bCapHoles && !bRemove)
		bCapHoles = false;

	int		matID = -1;
	if (bCapHoles && pblock2->GetInt(slice_cap_material))
		matID = pblock2->GetInt(slice_cap_matid) - 1;	// 1 indexed in the UI

	// Radial values
	if (iSliceFormat == slice_radial)
	{
		float	fAngle1 = 0.0f;
		float	fAngle2 = 0.0f;
		pblock2->GetValue(slice_radial_angle1, t, fAngle1, iv);
		pblock2->GetValue(slice_radial_angle2, t, fAngle2, iv);

		if (fAngle1 != fAngle2)
		{
			if (fAngle2 > fAngle1)
			{
				float fTemp = fAngle1;
				fAngle1 = fAngle2;
				fAngle2 = fTemp;
			}

			fAngle1 = DegToRad(fAngle1);
			fAngle2 = DegToRad(fAngle2);

			int	iAxis = pblock2->GetInt(slice_radial_axis);

			Matrix3	m1 = tm;
			Matrix3 m2 = tm;

			if (fabs(fAngle1 - fAngle2) == PI)
				numSlices = 1;
			else {
				numSlices = 2;
			}

			if (iAxis == X_AXIS)
			{
				m1.PreRotateX(-fAngle1);
				Point3 N = Normalize(m1*Point3::YAxis - origin);
				if (radial_type != radial_remove_top_type) {
					sliceFlips[0] = true;
					N = -N;
				}

				sliceOffsets[0] = DotProd(N, origin );
				slicePlanes[0] = N;

				if (numSlices == 2)
				{
					m2.PreRotateX(-fAngle2);
					N = Normalize(m2*Point3::YAxis - origin);
					if (radial_type == radial_remove_top_type) {
						N = -N;
						sliceFlips[1] = true;
					}

					sliceOffsets[1] = DotProd(N, origin );
					slicePlanes[1] = N;
				}
			}
			else if (iAxis == Y_AXIS)
			{
				m1.PreRotateY(-fAngle1);
				Point3 N = Normalize(m1*Point3::ZAxis - origin);
				if (radial_type != radial_remove_top_type) {
					N = -N;
					sliceFlips[0] = true;
				}

				sliceOffsets[0] = DotProd(N, origin );
				slicePlanes[0] = N;

				if (numSlices == 2)
				{
					m2.PreRotateY(-fAngle2);
					N = Normalize(m2*Point3::ZAxis - origin );
					if (radial_type == radial_remove_top_type) {
						N = -N;
						sliceFlips[1] = true;
					}

					sliceOffsets[1] = DotProd(N, origin );
					slicePlanes[1] = N;
				}
			}
			else
			{
				m1.PreRotateZ(fAngle1);
				Point3 N = Normalize(m1*Point3::XAxis - origin );
				if (radial_type != radial_remove_top_type) {
					N = -N;
					sliceFlips[0] = true;
				}

				sliceOffsets[0] = DotProd(N, origin );
				slicePlanes[0] = N;

				if (numSlices == 2)
				{
					m2.PreRotateZ(fAngle2);
					N = Normalize(m2*Point3::XAxis - origin );
					if (radial_type == radial_remove_top_type) {
						N = -N;
						sliceFlips[1] = true;
					}

					sliceOffsets[1] = DotProd(N, origin );
					slicePlanes[1] = N;
				}
			}
		}
		else
			bDeleteAll = true;
	}
	else {
		// Axis selection for Planar
		bool bSliceX = (pblock2->GetInt(slice_planar_x) != 0);
		bool bSliceY = (pblock2->GetInt(slice_planar_y) != 0);
		bool bSliceZ = (pblock2->GetInt(slice_planar_z) != 0);

		// X Axis
		if (bSliceX)
		{
			bool bFlip = (pblock2->GetInt(slice_planar_flip_x) != 0);
			Point3 N = Normalize(tm*Point3::XAxis - origin );
			if ((planar_type == planar_remove_top_type) ^ bFlip) {
				N = -N;
				sliceFlips[0] = true;
			}

			sliceOffsets[0] = DotProd(N, origin );
			slicePlanes[0] = N;
			numSlices++;
		}

		// Y Axis
		if (bSliceY)
		{
			bool bFlip = (pblock2->GetInt(slice_planar_flip_y) != 0);
			Point3 N = Normalize(tm*Point3::YAxis - origin );
			if ((planar_type == planar_remove_top_type) ^ bFlip) {
				N = -N;
				sliceFlips[numSlices] = true;
			}

			sliceOffsets[numSlices] = DotProd(N, origin );
			slicePlanes[numSlices++] = N;
		}

		// Z Axis
		if (bSliceZ)
		{
			bool bFlip = (pblock2->GetInt(slice_planar_flip_z) != 0);
			Point3 N = Normalize(tm*Point3::ZAxis - origin );
			if ((planar_type == planar_remove_top_type) ^ bFlip) {
				N = -N;
				sliceFlips[numSlices] = true;
			}

			sliceOffsets[numSlices] = DotProd(N, origin );
			slicePlanes[numSlices++] = N;
		}
	}

	// Do our conversion to keep appearance consistent even if there is nothing to do
	bool	isPoly = outputType != face_type_tri;

	if (outputType != face_type_auto)
	{
		if (isPoly) {
			if (os->obj->IsSubClassOf(polyObjectClassID)) {
				pobj = (PolyObject *)os->obj;
			}
			else {
				if (os->obj->CanConvertToType(polyObjectClassID)) {
					pobj = (PolyObject *)os->obj->ConvertToType(t, polyObjectClassID);
				}
				else {
					tobj = (TriObject *)os->obj->ConvertToType(t, triObjectClassID);
					pobj = (PolyObject *)tobj->ConvertToType(t, polyObjectClassID);
					if (tobj != os->obj) {
						tobj->DeleteThis();
						tobj = NULL;
					}
				}
				if (pobj != os->obj) {
					os->obj = pobj;
					os->obj->UnlockObject();
				}
			}
		}
		else {
			if (os->obj->IsSubClassOf(triObjectClassID)) {
				tobj = (TriObject *)os->obj;
			}
			else {
				tobj = (TriObject *)os->obj->ConvertToType(t, triObjectClassID);
				if (tobj != os->obj) {
					os->obj = tobj;
					os->obj->UnlockObject();
				}
			}
		}
	}
	else // Choose the best based on input type
	{
		// Take a mesh if it exists
		if (os->obj->IsSubClassOf(triObjectClassID)) {
			tobj = (TriObject *)os->obj;
			isPoly = false;
		}
		else {
			// Otherwise force to poly
			if (os->obj->IsSubClassOf(polyObjectClassID)) {
				pobj = (PolyObject *)os->obj;
			}
			else {
				if (os->obj->CanConvertToType(polyObjectClassID)) {
					pobj = (PolyObject *)os->obj->ConvertToType(t, polyObjectClassID);
				}
				else {
					tobj = (TriObject *)os->obj->ConvertToType(t, triObjectClassID);
					pobj = (PolyObject *)tobj->ConvertToType(t, polyObjectClassID);
					if (tobj != os->obj) {
						tobj->DeleteThis();
						tobj = NULL;
					}
				}
				if (pobj != os->obj) {
					os->obj = pobj;
					os->obj->UnlockObject();
				}
			}
			isPoly = true;
		}
	}
	DbgAssert(pobj || tobj);

	if (numSlices)
	{
		if (isPoly)
		{
			MNMesh & mm = pobj->GetMesh();

			float	dUV = 1.0f;
			Point3	p3Center = Point3(0.0f, 0.0f, 0.0f);
			const	int MapChannel = 1;
			MNMap*	mmp = mm.M(MapChannel);
			bool bHasTextures = mmp && !mmp->GetFlag(MN_DEAD);

			if (bCapHoles && bHasTextures)
			{
				Box3 bbox = mm.getBoundingBox();
				p3Center = bbox.Center();
				// update dUV
				if (!(::GetUsePhysicalScaleUVs(this)))
				{
					Point3	dim = bbox.Width();
					float	max = std::max(std::max(dim.x, dim.y), dim.z);
					if (max > 0.0f)
						dUV = 1.0f / max;
				}
			}

			// Luna task 747
			// Specified normals not supported by Slice.
			mm.ClearSpecifiedNormals();

			// This is expensive and is aimed at reducing the poly count in an attempt to speed up
			// slicing. Now that slicing is much faster, this processing only adds unnecessary overhead. (~30%)
			// However, since it does change topology, do these steps for legacy files
			if (IsLegacy()) {
				mm.FenceMaterials();
				mm.FenceFaceSel();
				mm.FenceSmGroups();
				mm.FenceNonPlanarEdges();
				mm.MakePolyMesh();
			}

			for (int iSlice = 0; iSlice < numSlices; iSlice++)
			{
				mm.Slice(slicePlanes[iSlice], sliceOffsets[iSlice], MNEPS, bSplit, bRemove, (mm.selLevel == MNM_SL_FACE), MN_SEL, TRIANGULATION_LEGACY);

				// We have to cap on each iteration otherwise capping will fail
				if (bCapHoles)
				{
					Matrix3		ForwardTransform;

					Point3	N = slicePlanes[iSlice];
					float	offset = sliceOffsets[iSlice];
					if (sliceFlips[iSlice]) {
						N = -N;
						offset = -offset;
					}
					MatrixFromNormalAndOffset(N, offset, ForwardTransform);
					Matrix3		InverseTransform = Inverse(ForwardTransform);

					PolyShape	pShape;
					MNMeshBorder mb;
					mm.GetBorderFromSelectedVerts(mb);

					if (CreatePolyFromMNMeshEdges(mm, pShape, mb, InverseTransform))
					{
						ShapeHierarchy	hier;
						pShape.OrganizeCurves(t, &hier);
						if (!sliceFlips[iSlice]) {
							BitArray tempBA = hier.reverse;
							hier.reverse = ~tempBA;
						}
						pShape.Reverse(hier.reverse);

						int polys = pShape.numLines;

						MeshCapInfo capInfo;
						pShape.MakeCap(t, capInfo, CAPTYPE_MORPH);

						// Build information for capping
						MeshCapper capper(pShape);

						int vert = 0;
						for (int spoly = 0; spoly < polys; ++spoly)
						{
							PolyLine& line = pShape.lines[spoly];
							if (!line.numPts)
								continue;
							MeshCapPoly& capline = capper[spoly];
							int lverts = line.numPts;

							// Mark the location of the poly relative to the mesh based on orientation
							if (hier.reverse[spoly] == (sliceFlips[iSlice] == TRUE))
							{
								vert += lverts - 1;
								for (int v = 0; v < lverts; ++v) {
									capline.SetVert(v, meshMapping[vert]);
									vert--;
								}
								vert += lverts + 1;
							}
							else {
								for (int v = 0; v < lverts; ++v) {
									capline.SetVert(v, meshMapping[vert]);
									vert++;
								}
							}
						}

						// If the matID isn't specified, take the average MatID from the faces that lie along the edge
						int	thisMatID = matID;
						if (thisMatID < 0)
						{
							// Best use the edge data as we know we know the edge is selected during the slice
							BitArray eSel;
							mm.getEdgeSel(eSel);

							int numEdges = mm.ENum();
							DbgAssert(numEdges == eSel.GetSize());

							int	iMatCount = 0;
							int	iMatID = 0;
							// Tag the faces we care about, since we are on an open edge, we only care about face 1
							for (int ie = 0; ie < numEdges; ie++)
							{
								if (eSel[ie]) {
									int face = mm.e[ie].f1;
									DbgAssert(face >= 0);
									iMatID += mm.f[face].material;
									iMatCount++;
								}
							}
							if (iMatCount)
								thisMatID = iMatID / iMatCount;
							else
								thisMatID = 0;
						}

						int	oldFaces = mm.numf;

						// Cap
						capper.CapMNMesh(mm, capInfo, sliceFlips[iSlice] == FALSE, 0, &ForwardTransform, thisMatID);

						// If texturing, create the texture faces and vertices
						if (bHasTextures)
							MakeMNMeshCapTexture(mm, p3Center, dUV, InverseTransform, oldFaces, mm.numf);

						// Consolidate the face polys
						mm.MakePolyMeshRanged(oldFaces, mm.numf);
					}
				}
			}

			if (clean)
				mm.CollapseDeadStructs();

			mm.PrepForPipeline();
		}
		else {
			// update the face slice type (if necessary) in case we back save later
			//if (pblock2->GetInt(slice_face) != face_type_tri)
			//	pblock2->SetValue(slice_face, t, face_type_tri);

			// If mesh has specified normals, punt them
			Mesh & mesh = tobj->GetMesh();
			MeshNormalSpec* normals = (MeshNormalSpec*)mesh.GetInterface(MESH_NORMAL_SPEC_INTERFACE);
			if (normals)
				normals->ClearAndFree();

			// See if we have an adjacent edge list we can use for the first slice
			if (mc.localData == nullptr)
				mc.localData = static_cast<LocalModData *>(new BasicModData);

			BasicModData *smd = static_cast<BasicModData*>(mc.localData);

			// Otherwise create one
			if (!smd->CheckEdgeList(mesh.getNumVerts())) {
				smd->CacheEdges(mesh);
			}

			// See if we have texture co-ords
			BOOL	bHasTextures = mesh.getNumTVerts() > 0;
			float	dUV = 1.0f;
			Point3	p3Center = Point3(0.0f, 0.0f, 0.0f);
			if (bHasTextures && bCapHoles)
			{
				Box3 bbox = mesh.getBoundingBox();
				p3Center = bbox.Center();
				// update dUV
				if (!(::GetUsePhysicalScaleUVs(this)))
				{
					Point3	dim = bbox.Width();
					float	max = std::max(std::max(dim.x, dim.y), dim.z);
					if (max > 0.0f)
						dUV = 1.0f / max;
				}
			}

			for (int iSlice = 0; iSlice < numSlices; iSlice++) {
				// We use the vertex selection from the slice so clear current selection
				mesh.VertSel().ClearAll();

				BOOL	bChanged = SliceMesh(mesh, slicePlanes[iSlice], sliceOffsets[iSlice], bSplit, bRemove, iSlice == 0 ? smd->GetEdgeList() : nullptr);

				// Are we capping and there is something to do?
				if (bCapHoles && bChanged && mesh.numFaces)
				{
					// We might have a problem if the resultant verts are really, really close to each other
					// as it may appear that the edge poly is overlapping. Weld now to avoid problems later
					MeshDelta tmd(mesh);
					if (tmd.WeldByThreshold(mesh, mesh.VertSel(), MNEPS))
						tmd.Apply(mesh);

					AdjEdgeList ae(mesh, TRUE);

					Matrix3		ForwardTransform;

					Point3	N = slicePlanes[iSlice];
					float	offset = sliceOffsets[iSlice];
					if (sliceFlips[iSlice]) {
						N = -N;
						offset = -offset;
					}
					MatrixFromNormalAndOffset(N, offset, ForwardTransform);
					Matrix3		InverseTransform = Inverse(ForwardTransform);

					PolyShape	pShape;

					if (CreatePolyFromMeshEdges(mesh, pShape, &ae, InverseTransform))
					{
						ShapeHierarchy	hier;
						pShape.OrganizeCurves(t, &hier);
						if (!sliceFlips[iSlice]) {
							BitArray tempBA = hier.reverse;
							hier.reverse = ~tempBA;
						}
						pShape.Reverse(hier.reverse);

						int polys = pShape.numLines;

						MeshCapInfo capInfo;
						pShape.MakeCap(t, capInfo, CAPTYPE_MORPH);

						// Build information for capping
						MeshCapper capper(pShape);

						int vert = 0;
						for (int spoly = 0; spoly < polys; ++spoly)
						{
							PolyLine& line = pShape.lines[spoly];
							if (!line.numPts)
								continue;
							MeshCapPoly& capline = capper[spoly];
							int lverts = line.numPts;

							// Mark the location of the poly relative to the mesh based on orientation
							if (hier.reverse[spoly] == (sliceFlips[iSlice]==TRUE))
							{
								vert += lverts - 1;
								for (int v = 0; v < lverts; ++v) {
									capline.SetVert(v, meshMapping[vert]);
									vert--;
								}
								vert += lverts + 1;
							}
							else {
								for (int v = 0; v < lverts; ++v) {
									capline.SetVert(v, meshMapping[vert]);
									vert++;
								}
							}
						}

						int oldFaces = mesh.numFaces;

						// if the matid isn't specified, take the average (rounded down) matID across the faces that were sliced
						int	thisMatID = matID;
						if (thisMatID < 0)
						{
							int		avgMatID = 0;
							int		matCount = 0;
							BitArray& vertSel = mesh.VertSel();
							for (int iface = 0; iface < oldFaces; iface++)
							{
								for (int v = 0; v < 3; v++) {
									if (vertSel[mesh.faces[iface].v[v]])
									{
										avgMatID += mesh.faces[iface].getMatID();
										matCount++;
										break;
									}
								}
							}
							if (matCount)
								thisMatID = avgMatID / matCount;
							else
								thisMatID = 0;
						}

						capper.CapMesh(mesh, capInfo, sliceFlips[iSlice] == FALSE, 0, &ForwardTransform, thisMatID);

						// If texturing, create the texture faces and vertices
						if (bHasTextures) {
							MakeMeshCapTexture(mesh, p3Center, dUV, InverseTransform, oldFaces, mesh.numFaces);
						}
					}
				}
			}
		}
	}
	else if (bDeleteAll)
	{
		// Special case, with Radial which requires removal of the whole mesh
		if (isPoly)
		{
			MNMesh & mm = pobj->GetMesh();
			mm.setNumFaces(0);
			mm.setNumVerts(0);
		}
		else {
			Mesh & mesh = tobj->GetMesh();
			mesh.setNumFaces(0);
			mesh.setNumVerts(0);
		}
	}

	os->obj->UpdateValidity(GEOM_CHAN_NUM, iv);
	os->obj->UpdateValidity(TOPO_CHAN_NUM, iv);
	os->obj->UpdateValidity(SELECT_CHAN_NUM, iv);
	os->obj->UpdateValidity(TEXMAP_CHAN_NUM, iv);
	os->obj->UpdateValidity(VERT_COLOR_CHAN_NUM, iv);
}

Interval SliceMod::LocalValidity(TimeValue t) {
	//aszabo|feb.05.02 Return NEVER forces a cache to be built after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED)) 
		return NEVER;  
	Interval iv = FOREVER;
	Matrix3 tm;
	plane->GetValue (t, &tm, iv, CTRL_RELATIVE);
	return iv;
}

void SliceMod::DrawPlanarGizmos(GraphicsWindow* gw, const Matrix3& ptm, float size)
{
	if (!gw)
		return;

	bool	bSliceX = (pblock2->GetInt(slice_planar_x) != 0);
	bool	bSliceY = (pblock2->GetInt(slice_planar_y) != 0);
	bool	bSliceZ = (pblock2->GetInt(slice_planar_z) != 0);

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

void SliceMod::DrawRadialGizmos(GraphicsWindow* gw, const Matrix3& ptm, float size, TimeValue t)
{
	if (!gw)
		return;

	Matrix3	m1 = ptm;
	Point3 rp1[4];
	Point3 rp2[4];

	float fAngle1 = DegToRad(pblock2->GetFloat(slice_radial_angle1, t));
	float fAngle2 = DegToRad(pblock2->GetFloat(slice_radial_angle2, t));
	bool	bSecondCalc = (fAngle1 != fAngle2);

	int iSliceType = pblock2->GetInt(slice_radial_type);
	float	fAxis = size;
	if (iSliceType != radial_remove_bottom_type)
		fAxis = -fAxis;

	int	iAxis = pblock2->GetInt(slice_radial_axis);
	if (iAxis == X_AXIS)
	{
		m1.PreRotateX(-fAngle1);
		rp1[0] = Point3(-size, 0.0f, 0.0f)*m1;
		rp1[1] = Point3(size, 0.0f, 0.0f)*m1;
		rp1[2] = Point3(size, 0.0f, fAxis)*m1;
		rp1[3] = Point3(-size, 0.0f, fAxis)*m1;

		if (bSecondCalc) {
			Matrix3	m2 = ptm;
			m2.PreRotateX(-fAngle2);
			rp2[0] = Point3(-size, 0.0f, 0.0f)*m2;
			rp2[1] = Point3(size, 0.0f, 0.0f)*m2;
			rp2[2] = Point3(size, 0.0f, fAxis)*m2;
			rp2[3] = Point3(-size, 0.0f, fAxis)*m2;
		}
	}
	else if (iAxis == Y_AXIS)
	{
		m1.PreRotateY(-fAngle1);
		rp1[0] = Point3(0.0f, -size, 0.0f)*m1;
		rp1[1] = Point3(0.0f, size, 0.0f)*m1;
		rp1[2] = Point3(fAxis, size, 0.0f)*m1;
		rp1[3] = Point3(fAxis, -size, 0.0f)*m1;

		if (bSecondCalc)
		{
			Matrix3	m2 = ptm;
			m2.PreRotateY(-fAngle2);
			rp2[0] = Point3(0.0f, -size, 0.0f)*m2;
			rp2[1] = Point3(0.0f, size, 0.0f)*m2;
			rp2[2] = Point3(fAxis, size, 0.0f)*m2;
			rp2[3] = Point3(fAxis, -size, 0.0f)*m2;
		}
	}
	else {
		m1.PreRotateZ(fAngle1);
		rp1[0] = Point3(0.0f, 0.0f, -size)*m1;
		rp1[1] = Point3(0.0f, 0.0f, size)*m1;
		rp1[2] = Point3(0.0f, -fAxis, size)*m1;
		rp1[3] = Point3(0.0f, -fAxis, -size)*m1;

		if (bSecondCalc)
		{
			Matrix3	m2 = ptm;
			m2.PreRotateZ(fAngle2);
			rp2[0] = Point3(0.0f, 0.0f, -size)*m2;
			rp2[1] = Point3(0.0f, 0.0f, size)*m2;
			rp2[2] = Point3(0.0f, -fAxis, size)*m2;
			rp2[3] = Point3(0.0f, -fAxis, -size)*m2;
		}
	}

	gw->polyline(4, rp1, NULL, NULL, TRUE, NULL);
	if (bSecondCalc)
		gw->polyline(4, rp2, NULL, NULL, TRUE, NULL);
}

int SliceMod::HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc)
{
	if (!vpt || !vpt->IsAlive())
	{
		DbgAssert(!_T("Invalid viewport!"));
		return FALSE;
	}

	GraphicsWindow *gw = vpt->getGW();
	Point3 pt;
	HitRegion hr;
	int savedLimits, res = 0;

	bool	bPlanar = (pblock2->GetInt(slice_format) == slice_planar);
	INode* 	objRef = pblock2->GetINode(slice_node_ref);

	Matrix3 tm = CompMatrix(t, objRef ? objRef : inode, mc);
	BasicModData *smd = static_cast<BasicModData*>(mc->localData);

	MakeHitRegion(hr, type, crossing, 4, p);
	gw->setHitRegion(&hr);
	gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);
	gw->setTransform(tm);

	gw->clearHitCode();
	Matrix3 ptm;
	plane->GetValue(t, &ptm, FOREVER, CTRL_RELATIVE);

	if (bPlanar)
		DrawPlanarGizmos(gw, ptm, smd->GetSize());
	else
		DrawRadialGizmos(gw, ptm, smd->GetSize(), t);

	if (gw->checkHitCode()) {
		vpt->LogHit(inode, mc, gw->getHitDistance(), 0, NULL);
		res = 1;
	}

	gw->setRndLimits(savedLimits);
	return res;
}

int SliceMod::Display(TimeValue t, INode* inode, ViewExp *vpt,	int flagst, ModContext *mc)
{
	if (!vpt || !vpt->IsAlive())
	{
		DbgAssert(!_T("Invalid viewport!"));
		return FALSE;
	}

	GraphicsWindow *gw = vpt->getGW();

	bool	bPlanar = (pblock2->GetInt(slice_format) == slice_planar);
	INode* 	objRef = pblock2->GetINode(slice_node_ref);

	Matrix3 tm = CompMatrix(t, objRef ? objRef : inode, mc);

	BasicModData *smd = static_cast<BasicModData*>(mc->localData);
	int savedLimits;

	gw->setRndLimits((savedLimits = gw->getRndLimits()) & ~GW_ILLUM);
	gw->setTransform(tm);

	// Draw rectangle representing slice plane.
	if (ip && ip->GetSubObjectLevel() == 1) {
		gw->setColor(LINE_COLOR, GetUIColor(COLOR_SEL_GIZMOS));
	}
	else {
		gw->setColor(LINE_COLOR, GetUIColor(COLOR_GIZMOS));
	}
	Matrix3 ptm;
	plane->GetValue(t, &ptm, FOREVER, CTRL_RELATIVE);

	if (bPlanar)
		DrawPlanarGizmos(gw, ptm, smd->GetSize());
	else
		DrawRadialGizmos(gw, ptm, smd->GetSize(), t);

	gw->setRndLimits(savedLimits);
	return 0;
}

void SliceMod::GetWorldBoundBox(TimeValue t, INode* inode, ViewExp *vpt, Box3& box, ModContext *mc)
{
	if (!vpt || !vpt->IsAlive())
	{
		box.Init();
		return;
	}

	Matrix3 tm = CompMatrix(t, inode, mc);
	BasicModData *smd = static_cast<BasicModData*>(mc->localData);
	Matrix3 ptm;
	plane->GetValue(t, &ptm, FOREVER, CTRL_RELATIVE);
	ptm *= tm;
	box.Init();

	float	size = smd->GetSize();
	box += Point3(-size, -size, 0.0f)*ptm;
	box += Point3(-size, size, 0.0f)*ptm;
	box += Point3(size, size, 0.0f)*ptm;
	box += Point3(size, -size, 0.0f)*ptm;
}

void SliceMod::Move(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin)
{
	SetXFormPacket pckt(val, partm, tmAxis);
	plane->SetValue(t, &pckt, TRUE, CTRL_RELATIVE);
}

void SliceMod::Rotate(TimeValue t, Matrix3 & partm, Matrix3 & tmAxis,Quat & val, BOOL localOrigin)
{
	INode* 	refNode = pblock2->GetINode(slice_node_ref, t);
	SetXFormPacket pckt(val, localOrigin, refNode? refNode->GetObjTMBeforeWSM(t):partm, tmAxis);
	plane->SetValue(t, &pckt, TRUE, CTRL_RELATIVE);
}

void SliceMod::Scale(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin)
{
	INode*	refNode = pblock2->GetINode(slice_node_ref, t);
	SetXFormPacket pckt(val, localOrigin, refNode? refNode->GetObjTMBeforeWSM(t):partm, tmAxis);
	plane->SetValue(t, &pckt, TRUE, CTRL_RELATIVE);
}

void SliceMod::GetSubObjectCenters(SubObjAxisCallback *cb, TimeValue t, INode *node, ModContext *mc)
{
	// See if we are referencing the source node or ourselves
	INode*	refNode = pblock2->GetINode(slice_node_ref, t);
	Matrix3 tm = CompMatrix(t, refNode ? refNode : node, mc); // (1);

	Matrix3 ptm;
	plane->GetValue(t, &ptm, FOREVER, CTRL_RELATIVE);
	cb->Center((ptm*tm).GetTrans(), 0);
}

// STEVE: Changed this after 3.1 so that it would return correct subobject
// local transform.
void SliceMod::GetSubObjectTMs(SubObjAxisCallback *cb, TimeValue t, INode *node, ModContext *mc)
{
	Matrix3 tm = CompMatrix(t, node, mc);
	Matrix3 ptm;
	plane->GetValue(t, &ptm, FOREVER, CTRL_RELATIVE);
	cb->TM((ptm*tm), 0);
}

void SliceMod::ActivateSubobjSel(int level, XFormModes& modes) {
	switch (level) {
	case 1: // The slicing plane.
		modes = XFormModes(moveMode, rotMode, uscaleMode, nuscaleMode, squashMode, NULL);
		break;
	}
	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
}

BOOL SliceMod::AssignController(Animatable *control, int subAnim) {
	if (subAnim == PLANE_REF) {
		ReplaceReference(PLANE_REF, (ReferenceTarget*)control);
		NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		NotifyDependents(FOREVER, PART_ALL, REFMSG_SUBANIM_STRUCTURE_CHANGED);
		return TRUE;
	}
	else {
		return FALSE;
	}
}

int SliceMod::SubNumToRefNum(int subNum) {
	if (subNum == PLANE_REF) return subNum;
	else return -1;
}

RefTargetHandle SliceMod::GetReference(int i) {
	switch (i) {
	case PBLOCK_REF: return pblock2;
	case PLANE_REF:  return plane;
	case NODE_TRANSFORM_MONITOR_REF: return transformMonitor;
	default: return NULL;
	}
}

void SliceMod::SetReference(int i, RefTargetHandle rtarg) {
	switch (i) {
	case PBLOCK_REF: pblock2 = (IParamBlock2*)rtarg; break;
	case PLANE_REF: plane = (Control*)rtarg; break;
	case NODE_TRANSFORM_MONITOR_REF: transformMonitor = (NodeTransformMonitor*)rtarg; break;
	}
}

TSTR SliceMod::SubAnimName(int i, bool localized)
{
	switch (i)
	{
	case PLANE_REF: return localized ? GetString(IDS_SLICEPLANE) : _T("Slice Plane");
	default: return _T("");
	}
}

RefResult SliceMod::NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate)
{
	// If we are using a reference object and we signal on the transformMonitor, update our object
	if ((hTarget == transformMonitor) && HasObject())
	{
		NotifyDependents(FOREVER, PART_TOPO, REFMSG_CHANGE);
	}

	return REF_SUCCEED;
}

void SliceMod::NotifyInputChanged(const Interval& /*changeInt*/, PartID partID, RefMessage /*message*/, ModContext* mc)
{
	// Did the Topology Change? If so, Invalidate edgelist cache
	if (partID | TOPO_CHANNEL)
	{
		BasicModData *smd = static_cast<BasicModData*>(mc->localData);
		if (smd)
			smd->ClearEdgeList();
	}
}

Matrix3 SliceMod::CompMatrix(TimeValue t, INode *inode, ModContext *mc)
{
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
	{
		tm = tm * inode->GetObjTMBeforeWSM(t, &iv);
	}
	return tm;
}

int SliceMod::NumSubObjTypes()
{
	return 1;
}

ISubObjType *SliceMod::GetSubObjType(int i)
{
	static bool initialized = false;
	if (!initialized)
	{
		initialized = true;
		SOT_SlicePlane.SetName(GetString(IDS_SLICEPLANE));
	}

	switch (i)
	{
	case 0:
		return &SOT_SlicePlane;
	}
	return NULL;
}

void SliceMod::ClearObject()
{
	pblock2->SetValue(slice_node_ref, 0, (INode*)nullptr);
	if (transformMonitor) {
		transformMonitor->SetNode(nullptr);
		transformMonitor->SetForwardTransformChangeMsgs(false);
		if (ip != nullptr) {
			ip->RedrawViews(ip->GetTime());
		}
	}
}

void SliceMod::AlignToFace()
{
	// We need to move out of subobject setting
	if (ip != nullptr && ip->GetSubObjectLevel() != 0) {
		ip->SetSubObjectLevel(0);
	}

	if (faceAlignMode)
		faceAlignMode->AlignPressed();
}