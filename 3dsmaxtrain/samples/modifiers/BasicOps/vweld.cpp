/**********************************************************************
 *<
	FILE: vweld.cpp

	DESCRIPTION: Vertex Weld Modifier (for Meshes, PolyMeshes, Patches.)

	CREATED BY: Steve Anderson, based on Face Extrude modifier by Berteig.

	HISTORY: created 9/1/2001
			08/29/2020 - M. Kaustinen:QT Migration
			11/04/2024 - M. Kaustinen: Spline Weld, Statistics, DEAD_VERT fix for polys

 *>	Copyright (c) 2001 Discreet, All Rights Reserved.
 **********************************************************************/

#include "vweld.h"

#include "iparamm2.h"
#include "MeshDLib.h"
#include "splshape.h"

#include "ui_VWeld_Modifier.h"

const unsigned int kVW_PBLOCK_REF(0);

// Version information
const USHORT kChunkUseRampageWeldMath = 0x200;
const USHORT kChunkFillInMesh = 0x210;
const USHORT kVersionChuck = 0x220;

const enum WeldVersions
{
	WELD_LEGACY,
	WELD_SPLINE_AWARE
};

// Knot entry per spline
class SplineKnotInfo
{
public:
	Spline3D* spline;
	Point3	point;

	BOOL	bSkip;
	float	minVal;
	int		poly;
	int		iNeighbor;
	float	fDistance;
	BOOL	bFirst;

	void ResestNeighbor(float threshold)
	{
		iNeighbor = -1;
		fDistance = threshold;
	}
};

class VWeldMod : public Modifier {	
	IParamBlock2 *mp_pblock;

public:
	VWeldMod();

	// From Animatable
	void DeleteThis() { delete this; }
	void GetClassName(MSTR& s, bool localized) const override {s = localized ? GetString(IDS_VERTEX_WELD_MOD) : _T("Vertex Weld");}  
	virtual Class_ID ClassID () { return kVERTEX_WELD_CLASS_ID;}
	RefTargetHandle Clone (RemapDir& remap);
	const TCHAR *GetObjectName(bool localized) const override { return localized ? GetString(IDS_VERTEX_WELD_MOD) : _T("Vertex Weld"); }

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
	CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;}

	// ParamBlock2 access:
	int NumParamBlocks () { return 1; }
	IParamBlock2* GetParamBlock(int i) { return mp_pblock; }
	IParamBlock2* GetParamBlockByID(BlockID id) { return (mp_pblock->ID() == id) ? mp_pblock : NULL; }

	// Reference Management:
	int NumRefs() {return 1;}
	RefTargetHandle GetReference(int i);
private:
	virtual void SetReference(int i, RefTargetHandle rtarg);

	int weldVersion;
public:
	RefResult NotifyRefChanged( const Interval& changeInt,RefTargetHandle hTarget, 
		PartID& partID, RefMessage message, BOOL propagate) { return REF_SUCCEED; }

	// Animatable management:
	int NumSubs() {return 1;}
	Animatable* SubAnim(int i) {return mp_pblock;}
	TSTR SubAnimName(int i, bool localized) override { return _T(""); }

	IOResult Load(ILoad *iload);
	IOResult Save(ISave *isave);
	bool mUseRampageWeldMath = false;
	bool mFillInMesh = true;	// When true, makes sure the MNMesh is filled-in prior to modification

	// Local methods
	void ConvertPatchSelection (PatchMesh & mesh);
	void ModifyPatchObject(TimeValue t, ModContext &mc, PatchObject *pobj, PatchMesh& mesh);

	void SetPolyFlags (MNMesh & mesh, DWORD flag);
	bool WeldShortPolyEdges (MNMesh & mesh, float thresh, DWORD flag);
	void ModifyPolyObject(TimeValue t, ModContext &mc, PolyObject *pobj);

	void ConvertTriSelection (Mesh & mesh, BitArray & targetVerts);
	void ModifyTriObject(TimeValue t, ModContext &mc, TriObject *tobj);

	void Append(Spline3D* mainSpline, Spline3D* attachSpline, bool bReverse);
	void GetNextSplineIndex(Tab<SplineKnotInfo>& skiTab, IntTab& frontMap, IntTab& backMap, SplineKnotInfo* iKnot, bool& bReverse, int& next);
	void CloseSpline(Spline3D* spline);
	void ModifySplineObject(TimeValue t, SplineShape* shape);

	VWeldRollup* mRollup;
	QString statusString;
	bool	bRollupOpen;

	void SetWeldVersion(int version)
	{
		if (version != WELD_SPLINE_AWARE)
			weldVersion = WELD_LEGACY;
		else
			weldVersion = WELD_SPLINE_AWARE;
	}

	int	GetWeldVersion()
	{
		if (weldVersion == WELD_SPLINE_AWARE)
			return weldVersion;

		return WELD_LEGACY;
	}

	void ShowStatusString()
	{
		if (mRollup && bRollupOpen)
			mRollup->UpdateStatusString(statusString);
	}

	void UpdateStatusString(int selectCount, int start, int end)
	{	
		TSTR	status;
		if (selectCount > 1)
			status.printf(GetString(IDS_WELD_N_OBJECTS_SELECTED), selectCount);
		else
			status.printf(GetString(IDS_WELD_STATS), start, end);

		statusString = status;
		ShowStatusString();
	}

};

//--- ClassDescriptor and class vars ---------------------------------

class VertexWeldClassDesc : public ClassDesc2 {
public:
	int 			IsPublic() { return 1; }
	void*			Create(BOOL loading = FALSE) { return new VWeldMod; }
	const TCHAR*	ClassName() { return GetString(IDS_VERTEX_WELD_MOD); }
	const TCHAR*	NonLocalizedClassName() { return _T("Vertex Weld"); }
	SClass_ID		SuperClassID() { return OSM_CLASS_ID; }
	Class_ID		ClassID() { return kVERTEX_WELD_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_MAX_STANDARD);}

	// The following are used by MAX Script and the schematic view:
	const TCHAR* InternalName() { return _T("VertexWeld"); }
	HINSTANCE HInstance() { return hInstance; }

	virtual MaxSDK::QMaxParamBlockWidget* CreateQtWidget(ReferenceMaker& owner, IParamBlock2& paramBlock, const MapID, MSTR& rollupTitle, int&, int&) override
	{
		if (paramBlock.ID() == kVW_PBLOCK_REF) {
			VWeldRollup*rr = new VWeldRollup();
			((VWeldMod&)owner).mRollup = rr;
			rr->SetParamBlock(&owner, &paramBlock);
			//: this is a comment for the translation team 
			rollupTitle = VWeldRollup::tr("VertexWeld");
			return rr;
		}
		return nullptr;
	}
};

static VertexWeldClassDesc vweldDesc;
extern ClassDesc* GetVertexWeldModDesc() {return &vweldDesc;}

// --- QT Setup ----------------------------

VWeldRollup::VWeldRollup(QWidget* /*parent*/) :
	QMaxParamBlockWidget(),
	m_UI(new Ui::VWeldRollup())
{
	mMod = nullptr;
	m_UI->setupUi(this);
}

VWeldRollup::~VWeldRollup(void)
{
	delete m_UI;
}

void VWeldRollup::SetParamBlock(ReferenceMaker* owner, IParamBlock2* const /*param_block*/)
{
	mMod = (VWeldMod*)owner;
}

void VWeldRollup::UpdateStatusString(QString statusString)
{
	if (m_UI)
		m_UI->numberOfClones->setText(statusString);
}

//--- Parameter map/block descriptors -------------------------------

// ParamBlock2: Enumerate the parameter blocks:
enum { kVertexWeldParams };

// And enumerate the parameters within that block:
enum { kVwThreshold };

class VWeldPBAccessor : PBAccessor
{
	virtual void Set(PB2Value& /*v*/, ReferenceMaker* /*owner*/, ParamID /*id*/, int /*tabIndex*/, TimeValue t) override
	{
		Interface* ip = GetCOREInterface();
		ip->RedrawViews(t, REDRAW_NORMAL);
	}
};
static VWeldPBAccessor s_accessor;

// Parameters
static ParamBlockDesc2 vertex_weld_param_blk (kVertexWeldParams, _T("Vertex Weld Parameters"), 0, &vweldDesc,
											   P_AUTO_CONSTRUCT + +P_AUTO_UI_QT, kVW_PBLOCK_REF,
	// Parameters
	kVwThreshold, _T("threshold"), TYPE_FLOAT, P_RESET_DEFAULT|P_ANIMATABLE, IDS_VW_THRESHOLD,
		p_default, 0.1f,
		p_range, 0.0f, BIGFLOAT,
		p_accessor, &s_accessor,
		p_end,

	p_end
);

//--- VWeldMod methods -------------------------------


VWeldMod::VWeldMod():
	mp_pblock(nullptr),
	mRollup(nullptr)
{
	vweldDesc.MakeAutoParamBlocks(this);
	SetWeldVersion(WELD_SPLINE_AWARE);
	bRollupOpen = false;
}

void VWeldMod::BeginEditParams (IObjParam  *ip, ULONG flags,Animatable *prev) {
	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);

	// For PB2 we ask the ClassDesc2 to take care of the BeginEditParams
	vweldDesc.BeginEditParams(ip,this,flags,prev);
	bRollupOpen = true;
	ShowStatusString();
}

void VWeldMod::EndEditParams (IObjParam *ip,ULONG flags,Animatable *next) {
	TimeValue t = ip->GetTime();
	// NOTE: This flag must be cleared before sending the REFMSG_END_EDIT
	ClearAFlag(A_MOD_BEING_EDITED);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);

	// For PB2 we ask the ClassDesc2 to take care of the EndEditParams - NH
	vweldDesc.EndEditParams(ip,this,flags,next);
	bRollupOpen = false;
}

RefTargetHandle VWeldMod::Clone(RemapDir& remap) {
	VWeldMod *mod = new VWeldMod();
	mod->ReplaceReference(kVW_PBLOCK_REF,remap.CloneRef(mp_pblock));
	
	// Copy these option flags as well...
	mod->mUseRampageWeldMath = mUseRampageWeldMath;
	mod->mFillInMesh = mFillInMesh;

	BaseClone(this, mod, remap);
	return mod;
}

void VWeldMod::ModifyObject (TimeValue t, ModContext &mc, ObjectState *os, INode *node)
{
	ModContextList mcList;
	INodeTab nodes;
	GetCOREInterface()->GetModContexts(mcList, nodes);
	int		numContexts = mcList.Count();

	int	startVerts = 0;
	int endVerts = 0;

	if (os->obj->IsSubClassOf(triObjectClassID))
	{
		startVerts = ((TriObject*)os->obj)->mesh.getNumVerts();
		ModifyTriObject(t, mc, (TriObject*)os->obj);
		endVerts = ((TriObject*)os->obj)->mesh.getNumVerts();
	}
	else {
		if (os->obj->IsSubClassOf(polyObjectClassID))
		{
			startVerts = ((PolyObject*)os->obj)->GetMesh().numv;
			ModifyPolyObject(t, mc, (PolyObject*)os->obj);
			endVerts = ((PolyObject*)os->obj)->GetMesh().numv;
		}
		else {
			if (os->obj->IsSubClassOf(patchObjectClassID))
			{
				PatchMesh& mesh = ((PatchObject*)os->obj)->GetPatchMesh(t);
				startVerts = mesh.getNumVerts();
				ModifyPatchObject(t, mc, (PatchObject*)os->obj, mesh);
				endVerts = mesh.getNumVerts();
			}
			else
			{
				if (GetWeldVersion() == WELD_SPLINE_AWARE)
				{
					SplineShape* shape = nullptr;
					bool	bConverted = false;

					if (os->obj->IsSubClassOf(splineShapeClassID))
						shape = (SplineShape*)os->obj;
					else {
						if (os->obj->CanConvertToType(splineShapeClassID)) {
							shape = (SplineShape*)os->obj->ConvertToType(t, splineShapeClassID);
							if (shape != os->obj)
								bConverted = true;
						}
					}

					if (shape)
					{
						// With splines, vert counts include bezier handles
						startVerts = shape->shape.GetNumVerts() /3;
						ModifySplineObject(t, shape);
						if (bConverted)
							os->obj = shape;

						endVerts = shape->shape.GetNumVerts() / 3;
					}
				} else if (os->obj->CanConvertToType (triObjectClassID))
				{
					TriObject *tobj = (TriObject *) os->obj->ConvertToType (t, triObjectClassID);
					startVerts = tobj->mesh.getNumVerts();
					ModifyTriObject (t, mc, tobj);
					endVerts = tobj->mesh.getNumVerts();

					os->obj = (Object *) tobj;
				}
			}
		}
	}
	
	UpdateStatusString(numContexts, startVerts, endVerts);
}

void VWeldMod::ConvertPatchSelection (PatchMesh & mesh) {
	int i;

	switch (mesh.selLevel) {
	case PATCH_OBJECT:
		mesh.VertSel().SetAll ();
		break;
	case PATCH_VERTEX:
		// Don't need to do anything.
		break;
	case PATCH_EDGE:
	{
		mesh.VertSel().ClearAll ();
		BitArray& vertSel = mesh.VertSel();
		const BitArray& edgeSel = mesh.EdgeSel();
		for (i=0; i<mesh.getNumEdges(); i++) {
			if (!edgeSel[i]) continue;
			vertSel.Set(mesh.edges[i].v1,TRUE);
			vertSel.Set(mesh.edges[i].v2,TRUE);
		}
		break;
	}
	case PATCH_PATCH:
	{
		mesh.VertSel().ClearAll ();
		BitArray& vertSel = mesh.VertSel();
		for (i=0; i<mesh.getNumPatches(); i++) {
			if (!mesh.patchSel[i]) continue;
			for (int j=0; j<mesh.patches[i].type; j++) vertSel.Set (mesh.patches[i].v[j]);
		}
		break;
	}
	}
}

void VWeldMod::ModifyPatchObject (TimeValue t, ModContext &mc, PatchObject *pobj, PatchMesh& mesh) {
	//PatchMesh &mesh = pobj->GetPatchMesh (t);
	Interval iv = FOREVER;
	float thresh;

	mp_pblock->GetValue (kVwThreshold, t, thresh, iv);
	if (thresh<0.0f) thresh=0.0f;

	// Convert existing selection (at whatever level) to vertex selection:
	// NOTE that if there is an incoming vertex selection, but we're at a different selection level,
	// we lose the vertex selection in this process.  Unavoidable until PatchMesh::Weld starts
	// accepting a BitArray targetVerts argument.
	ConvertPatchSelection (mesh);

	// Weld the vertices
	BOOL found = mesh.Weld (thresh);

	pobj->UpdateValidity (GEOM_CHAN_NUM, iv);
	pobj->UpdateValidity (TOPO_CHAN_NUM, iv);
	pobj->UpdateValidity (VERT_COLOR_CHAN_NUM, iv);
	pobj->UpdateValidity (TEXMAP_CHAN_NUM, iv);
	pobj->UpdateValidity (SELECT_CHAN_NUM, iv);
}

void VWeldMod::SetPolyFlags (MNMesh & mesh, DWORD flag) {
	// Convert existing selection (at whatever level) to vertex selection:
	mesh.ClearVFlags (flag);
	if (mesh.selLevel == MNM_SL_OBJECT) {
		for (int i=0; i<mesh.numv; i++) mesh.v[i].SetFlag (flag);
	} else {
		mesh.PropegateComponentFlags (MNM_SL_VERTEX, flag, mesh.selLevel, MN_SEL);
	}
}

// (This code was copied from EditPolyObj::EpfnCollapse.)
bool VWeldMod::WeldShortPolyEdges (MNMesh & mesh, float thresh, DWORD flag) {
	// In order to collapse vertices, we turn them into edge selections,
	// where the edges are shorter than the weld threshold.
	bool canWeld = false;
	mesh.ClearEFlags (flag);
	float threshSq = thresh*thresh;
	for (int i=0; i<mesh.nume; i++) {
		if (mesh.e[i].GetFlag (MN_DEAD)) continue;
		if (!mesh.v[mesh.e[i].v1].GetFlag (flag)) continue;
		if (!mesh.v[mesh.e[i].v2].GetFlag (flag)) continue;
		if (LengthSquared (mesh.P(mesh.e[i].v1) - mesh.P(mesh.e[i].v2)) > threshSq) continue;
		mesh.e[i].SetFlag (flag);
		canWeld = true;
	}
	if (!canWeld) return false;

	MNMeshUtilities mmu(&mesh);
	return mmu.CollapseEdges(flag);
}

void VWeldMod::ModifyPolyObject (TimeValue t, ModContext &mc, PolyObject *pobj) {
	MNMesh &mesh = pobj->GetMesh();

	if (mUseRampageWeldMath)
		mesh.SetFlag(MN_MESH_USE_MAX2012_WELD_MATH,TRUE);

	// Older versions did not fill in the mesh; this can cause the mesh.WeldBorderVerts call below
	// to fail as it requires a filled-in mesh. Fill in the mesh if needed.
	if (mFillInMesh && !mesh.GetFlag(MN_MESH_FILLED_IN))
		mesh.FillInMesh();

	// Luna task 747
	// We cannot support specified normals in Vertex Weld at this time.
	mesh.ClearSpecifiedNormals();

	Interval iv = FOREVER;
	float thresh;

	mp_pblock->GetValue (kVwThreshold, t, thresh, iv);
	if (thresh<0.0f) thresh=0.0f;
	SetPolyFlags (mesh, MN_USER);

	// Weld the suitable border vertices:
	bool haveWelded = false;
	if (mesh.WeldBorderVerts (thresh, MN_USER))
	{
		if (GetWeldVersion() != WELD_SPLINE_AWARE)
			mesh.CollapseDeadStructs();
		haveWelded = true;
	}

	// Weld vertices that share short edges:
	if (WeldShortPolyEdges (mesh, thresh, MN_USER))
		haveWelded = true;

	if (haveWelded)
	{
		// Collapsing moved to here for all welding cases
		if (GetWeldVersion() == WELD_SPLINE_AWARE)
			mesh.CollapseDeadStructs();
		mesh.InvalidateTopoCache ();
		mesh.FillInMesh ();
	}

	pobj->UpdateValidity (GEOM_CHAN_NUM, iv);
	pobj->UpdateValidity (TOPO_CHAN_NUM, iv);
	pobj->UpdateValidity (VERT_COLOR_CHAN_NUM, iv);
	pobj->UpdateValidity (TEXMAP_CHAN_NUM, iv);
	pobj->UpdateValidity (SELECT_CHAN_NUM, iv);
}

void VWeldMod::ConvertTriSelection (Mesh & mesh, BitArray & targetVerts) {
	targetVerts.SetSize (mesh.numVerts);
	targetVerts.ClearAll ();

	int i, j;
	switch (mesh.selLevel) {
	case MESH_OBJECT:
		targetVerts.SetAll ();
		break;
	case MESH_VERTEX:
		targetVerts = mesh.VertSel();
		break;
	case MESH_EDGE:
	{
		const BitArray& edgeSel = mesh.EdgeSel();
		for (i=0; i<mesh.numFaces; i++) {
			for (j=0; j<3; j++) {
				if (!edgeSel[i*3+j]) continue;
				targetVerts.Set (mesh.faces[i].v[j]);
				targetVerts.Set (mesh.faces[i].v[(j+1)%3]);
			}
		}
		break;
	}
	case MESH_FACE:
	{
		const BitArray& faceSel = mesh.FaceSel();
		for (i=0; i<mesh.numFaces; i++) {
			if (!faceSel[i]) continue;
			for (j=0; j<3; j++) targetVerts.Set (mesh.faces[i].v[j]);
		}
		break;
	}
	}
}

void VWeldMod::ModifyTriObject (TimeValue t, ModContext &mc, TriObject *tobj) {
	Mesh &mesh = tobj->GetMesh();
	Interval iv = FOREVER;
	
	float threshold;
	mp_pblock->GetValue (kVwThreshold, t, threshold, iv);
	if (threshold<0.0f) threshold=0.0f;

	// Convert existing selection (at whatever level) to vertex selection:
	BitArray targetVerts;
	ConvertTriSelection (mesh, targetVerts);

	// Weld the vertices
	MeshDelta tmd(mesh);
	BOOL found = tmd.WeldByThreshold (mesh, targetVerts, threshold);
	tmd.Apply (mesh);

	tobj->UpdateValidity (GEOM_CHAN_NUM, iv);
	tobj->UpdateValidity (TOPO_CHAN_NUM, iv);
	tobj->UpdateValidity (VERT_COLOR_CHAN_NUM, iv);
	tobj->UpdateValidity (TEXMAP_CHAN_NUM, iv);
	tobj->UpdateValidity (SELECT_CHAN_NUM, iv);
}

void VWeldMod::Append(Spline3D* mainSpline, Spline3D* attachSpline, bool bReverse)
{
	const int lastMain = mainSpline->KnotCount() - 1;

	DbgAssert((lastMain >= 1) || (attachSpline->KnotCount() > 1));

	if (bReverse)
		attachSpline->Reverse();

	const Point3	endA = mainSpline->GetKnotPoint(lastMain);
	const Point3	startB = attachSpline->GetKnotPoint(0);
	const Point3	pMid = (endA + startB) * 0.5f;

	mainSpline->SetKnotPoint(lastMain, pMid);
	mainSpline->SetLineType(lastMain, attachSpline->GetLineType(0));
	mainSpline->SetMatID(lastMain, attachSpline->GetMatID(0));

	// Adjust OutVector if necessary
	const int	mainKnotType = mainSpline->GetKnotType(lastMain);
	const int	attachKnotType = attachSpline->GetKnotType(0);

	// Don't adjust auto
	if ((mainKnotType != KTYPE_AUTO) || (attachKnotType != KTYPE_AUTO))
	{
		// If both aren't Corner Type, force to Bezier Corner and update OutVec
		if ((mainKnotType != KTYPE_CORNER) || (attachKnotType != KTYPE_CORNER))
		{
			mainSpline->SetKnotType(lastMain, KTYPE_BEZIER_CORNER);
			// Note: We don't move our in/out vectors, this way the original shape is better maintained
			mainSpline->SetOutVec(lastMain, attachSpline->GetOutVec(0));
		}
	}

	// Copy the remaining
	const int attachCount = attachSpline->KnotCount();
	for (int i = 1; i < attachCount; i++)
		mainSpline->AddKnot(attachSpline->GetKnot(i));
}

void VWeldMod::CloseSpline(Spline3D* spline)
{
	DbgAssert(spline);
	const int	lastKnot = spline->KnotCount() - 1;
	const Point3	p0 = spline->GetKnotPoint(0);
	const Point3	pLast = spline->GetKnotPoint(lastKnot);
	const Point3	pMid = (p0 + pLast) * 0.5f;

	// Adjust OutVector if necessary
	const int	firstKnotType = spline->GetKnotType(0);
	const int	lastKnotType = spline->GetKnotType(lastKnot);

	if ((firstKnotType != KTYPE_AUTO) || (lastKnotType != KTYPE_AUTO))
	{
		// If both aren't Corner Type, force to Bezier Corner and update OutVec
		if ((firstKnotType != KTYPE_CORNER) || (lastKnotType != KTYPE_CORNER))
		{
			spline->SetKnotType(0, KTYPE_BEZIER_CORNER);
			// Note: Don't move the In/Out Vec based on the midpoint,
			// spline appearance is better maintained without it
			spline->SetInVec(0, spline->GetInVec(lastKnot));
		}
	}

	spline->SetKnotPoint(0, pMid);
	spline->DeleteKnot(lastKnot);
	spline->SetClosed();
}

// Sort for deletion
static int DeleteSort(const void* elem1, const void* elem2)
{
	int* e1 = (int*)elem1;
	int* e2 = (int*)elem2;

	return (*e1 - *e2);
}

class PolyAppendInfo
{
public:
	int		poly;
	bool	bReverse;
};

// Sort based on minVal
static int KnotSort(const void* elem1, const void* elem2)
{
	SplineKnotInfo* k1 = (SplineKnotInfo*)elem1;
	SplineKnotInfo* k2 = (SplineKnotInfo*)elem2;

	if (k1->minVal < k2->minVal)
		return -1;

	if (k1->minVal > k2->minVal)
		return 1;

	// if the same, sort by poly number
	return k1->poly - k2->poly;

}

void VWeldMod::GetNextSplineIndex(Tab<SplineKnotInfo>& skiTab, IntTab& frontMap, IntTab& backMap, SplineKnotInfo* iKnot, bool& bReverse, int& next)
{
	bReverse = iKnot->bFirst;
	if (!iKnot->bSkip)
		iKnot->bSkip = TRUE;
	
	// What we want is the other index for same poly which
	// we are neighboring (if there is one)
	next = iKnot->iNeighbor;
	if (next >= 0)
	{
		int nextPoly = skiTab[next].poly;
		
		int	iFront = frontMap[nextPoly];
		if (iFront >= 0)
			skiTab[iFront].bSkip = TRUE;

		int		iBack = backMap[nextPoly];
		if (iBack >= 0)
			skiTab[iBack].bSkip = TRUE;

		if (skiTab[next].bFirst)
			next = iBack;
		else
			next = iFront;
	}
}

void VWeldMod::ModifySplineObject(TimeValue t, SplineShape* shape)
{
	if (!shape)
		return;

	int num_polys = shape->shape.splineCount;
	if (num_polys < 1)
		return;

	// Suspend
	SuspendAnimate();
	AnimateOff();

	Interval iv = FOREVER;

	float threshold;
	mp_pblock->GetValue(kVwThreshold, t, threshold, iv);
	if (threshold < 0.0f) threshold = 0.0f;

	// see if we need to worry about selection
	const bool bDoSelectionTest = (shape->GetSelLevel() == SS_VERTEX);
	ShapeVSel vSelection = shape->shape.vertSel;
	DbgAssert(!bDoSelectionTest || (vSelection.polys == num_polys));
	bool bMainStartSelected = true;
	bool bMainEndSelected = true;

	float	fThreshold2 = threshold * threshold;
	IntTab	deleteIndexes;
	deleteIndexes.Resize(num_polys / 2);

	Tab<SplineKnotInfo>	splineTab;
	splineTab.Resize(num_polys);

	// Populate a table with the start and end knots (if selected)
	SplineKnotInfo		ski;
	for (int poly = 0; poly < num_polys; poly++)
	{
		Spline3D* thisSpline = shape->shape.splines[poly];
		if (thisSpline && thisSpline->KnotCount() > 0)
		{
			int thisKnotCount = thisSpline->KnotCount();

			const BOOL bClosed = thisSpline->Closed();
			if (!bClosed)
			{
				ski.spline = thisSpline;
				ski.poly = poly;
				ski.fDistance = fThreshold2;
				ski.iNeighbor = -1;

				// Add first
				if (!bDoSelectionTest || vSelection.sel[poly][1])
				{
					ski.bFirst = TRUE;
					ski.point = thisSpline->GetKnotPoint(0);
					ski.minVal = ski.point.x;
					splineTab.Append(1, &ski);
				}

				// Add last
				if (!bDoSelectionTest || vSelection.sel[poly][(thisKnotCount * 3) - 2])
				{
					ski.bFirst = FALSE;
					ski.point = thisSpline->GetKnotPoint(thisKnotCount - 1);
					ski.minVal = ski.point.x;
					splineTab.Append(1, &ski);
				}
			}
		}
	}

	int num_knots = splineTab.Count();

	// bail early
	if (num_knots <= 1)
	{
		ResumeAnimate();
		shape->UpdateValidity(GEOM_CHAN_NUM, iv);
		shape->UpdateValidity(TOPO_CHAN_NUM, iv);
		return;
	}

	// Trivial case, only 1 weld to worry about
	if (num_knots == 2)
	{
		SplineKnotInfo &ski1 = splineTab[0];
		SplineKnotInfo& ski2 = splineTab[1];

		float fDist = LengthSquared(ski1.point - ski2.point);
		if (fDist < fThreshold2)
		{
			// Simple, just close this spline
			if (ski1.poly == ski2.poly)
				CloseSpline(ski1.spline);
			else
			{
				int deletePoly = -1;
				// If entry 1 is the end, append entry 2
				if (!ski1.bFirst)
				{
					Append(ski1.spline, ski2.spline, !ski2.bFirst);
					deletePoly = ski2.poly;
				}
				else
				{
					// if entry 2 is the end, append entry 1
					if (!ski2.bFirst)
					{
						Append(ski2.spline, ski1.spline, !ski1.bFirst);
						deletePoly = ski1.poly;
					}
					else
					{
						// Otherwise reverse entry 1, and append entry 2
						ski1.spline->Reverse(TRUE);
						Append(ski1.spline, ski2.spline, !ski2.bFirst);
						deletePoly = ski2.poly;
					}
				}
				if (deletePoly >= 0)
					shape->shape.DeleteSplines((unsigned int*) & deletePoly, 1);
			}
		}
	}
	else
	{
		// Sort so we can reject early
		splineTab.Sort(KnotSort);

		// Remap based on poly location
		// If you have a poly, these tables will tell
		// you where in the SplineTab each the
		// front and back entries can be found.
		// -1 indicates it does not exist

		IntTab		frontMap;
		IntTab		backMap;
		frontMap.SetCount(num_polys);
		backMap.SetCount(num_polys);
		for (int i = 0; i < num_polys; i++)
		{
			frontMap[i] = -1;
			backMap[i] = -1;
		}
		
		SplineKnotInfo* iKnot = splineTab.Addr(0);

		for (int i = 0; i < num_knots; i++)
		{
			if (iKnot->bFirst)
				frontMap[iKnot->poly] = i;
			else
				backMap[iKnot->poly] = i;
			iKnot++;
		}

		// For each entry, find the closest neighbor
		// if the closest neighbor already has a neighbor
		// assigned, clear the prior neighbor and flag for
		// rework.  Continue until there is no rework

		int		iReworkCount = 0;
		do
		{
			iReworkCount = 0;
			iKnot = splineTab.Addr(0);

			for (int i = 0; i < num_knots - 1; i++)
			{
				// Search for closest neighbor
				float	iValueMin = iKnot->minVal + threshold;
				float	iValueMax = iKnot->minVal - threshold;

				float	fClosestDistance = iKnot->fDistance;
				int		iClosestNeighbor = iKnot->iNeighbor;

				int	jStart = i + 1;
				SplineKnotInfo* jKnot = splineTab.Addr(jStart);
				for (int j = jStart; j < num_knots; j++)
				{
					if (jKnot->minVal >= iValueMin)
					{
						break;
					}
					else if (jKnot->minVal > iValueMax)
					{
						float	fDist = LengthSquared(iKnot->point - jKnot->point);
						if (fDist < fClosestDistance)
						{
							// Need to also check if we're closer than the neighbor's distance
							if (splineTab[j].fDistance > fDist)
							{
								fClosestDistance = fDist;
								iClosestNeighbor = j;
								if (fDist == 0.0f)
									break;
							}
						}
					}

					jKnot++;
				}

				// Now add (if different)
				if (iClosestNeighbor != iKnot->iNeighbor)
				{
					// Check to see if we need to update others
					if (iKnot->iNeighbor >= 0)
					{
						splineTab[iKnot->iNeighbor].ResestNeighbor(fThreshold2);
						iReworkCount++;
					}

					iKnot->iNeighbor = iClosestNeighbor;
					iKnot->fDistance = fClosestDistance;

					// Need to register if with the neighbour too
					jKnot = splineTab.Addr(iClosestNeighbor);
					int jKnotNeighbor = jKnot->iNeighbor;
					if (jKnotNeighbor >= 0)
					{
						splineTab[jKnotNeighbor].ResestNeighbor(fThreshold2);
						iReworkCount++;
					}
					jKnot->iNeighbor = i;
					jKnot->fDistance = fClosestDistance;
				}
				iKnot++;
			}
		} while (iReworkCount != 0);

		// We now have each entry with their respective closest neighbor
		// Combine these into a continuous list of neighbor polys
		// Combine so the list will only require appending polys
		// one after the other

		iKnot = splineTab.Addr(0);

		// Keep a table of poly connections, ordered front to back
		Tab<PolyAppendInfo>	connectionList;
		connectionList.Resize(num_knots);

		PolyAppendInfo		pai;
		for (int i = 0; i < num_knots; i++)
		{
			if (iKnot->bSkip)
			{
				iKnot++;
				continue;
			}

			//int thisNeighbor = iKnot->iNeighbor;
			iKnot->bSkip = TRUE;

			bool	bClosed = false;
			int		iStartPoly = iKnot->poly;

			SplineKnotInfo* ski = iKnot;

			// Since we are moving forward, we want to start with the ending knot of this poly
			if (ski->bFirst)
			{
				int index = backMap[iStartPoly];
				// it is possible that the other end isn't selected
				if (index < 0)
					ski = nullptr;
				else {
					ski = splineTab.Addr(index);
					ski->bSkip = TRUE;
				}
			}
			else
			{
				// flag front knot too
				int index = frontMap[iStartPoly];
				if (index >= 0)
					splineTab[index].bSkip = TRUE;
			}

			if (ski)
			{
				// Add ourselves
				pai.poly = iStartPoly;
				pai.bReverse = false;
				connectionList.Append(1, &pai);

				// Get next
				//ski = splineTab.Addr(thisNeighbor);
				int thisNeighbor = ski->iNeighbor;

				while (thisNeighbor >= 0)
				{
					ski = splineTab.Addr(thisNeighbor);
					ski->bSkip = TRUE;

					// check for closure
					if (ski->poly == iStartPoly)
					{
						bClosed = true;
						break;
					}

					// append connection
					pai.poly = ski->poly;
					pai.bReverse = !ski->bFirst;
					connectionList.Append(1, &pai);

					// Get our complement knot
					int compIndex = ski->bFirst ? backMap[ski->poly] : frontMap[ski->poly];
					if (compIndex >= 0)
					{
						ski = splineTab.Addr(compIndex);
						ski->bSkip = TRUE;
						thisNeighbor = ski->iNeighbor;
					}
					else
						thisNeighbor = -1;
				}
			}
			if (!bClosed)
			{
				// Now go backwards, pushing the polys in front of our
				// connection list
				
				ski = iKnot;

				// Make sure we are starting with the front
				if (!ski->bFirst)
				{
					DbgAssert(iStartPoly == ski->poly);

					int index = frontMap[iStartPoly];
					// it is possible that the other end isn't selected
					if (index < 0)
						ski = nullptr;
					else
						ski = splineTab.Addr(index);
				}
				if (ski)
				{
					// Add this connection if doesn't already exist
					int		connectionCount = connectionList.Count();
					int		lastPoly = -1;
					if (connectionCount > 0) {
						lastPoly = connectionList[connectionCount - 1].poly;
					}
					else
					{
						// Add ourselves
						lastPoly = ski->poly;
						pai.poly = lastPoly;
						pai.bReverse = false;
						connectionList.Append(1, &pai);					
					}
					DbgAssert(lastPoly >= 0);

					int thisNeighbor = ski->iNeighbor;

					while (thisNeighbor >= 0)
					{
						ski = splineTab.Addr(thisNeighbor);
						ski->bSkip = TRUE;

						// check for closure
						if (ski->poly == iStartPoly)
						{
							bClosed = true;
							break;
						}

						// prepend connection
						pai.poly = ski->poly;
						pai.bReverse = ski->bFirst;
						connectionList.Insert(0, 1, &pai);

						// Get our complement knot and its neighbor
						int compIndex = ski->bFirst ? backMap[ski->poly] : frontMap[ski->poly];
						if (compIndex >= 0)
						{
							ski = splineTab.Addr(compIndex);
							ski->bSkip = TRUE;
							thisNeighbor = ski->iNeighbor;
						}
						else
							thisNeighbor = -1;
					}

				}
			}

			// Now take the poly connection list and build
			// a single poly, starting with the first poly in the list
			// and adding each poly's knots
			int connectionCount = connectionList.Count();
			if (connectionCount > 0)
			{
				int		polyA = connectionList[0].poly;
				Spline3D* splineA = shape->shape.splines[polyA];
				if (connectionCount > 1)
				{
					if (connectionList[0].bReverse)
					{
						splineA->Reverse();
						//splineA->ComputeBezPoints();
					}

					for (int j = 1; j < connectionCount; j++)
					{
						int polyB = connectionList[j].poly;
						bool bReverse = connectionList[j].bReverse;
						Spline3D* splineB = shape->shape.splines[polyB];
						Append(splineA, splineB, bReverse);
						deleteIndexes.Append(1, &polyB);
					}
				}
				if (bClosed)
					CloseSpline(splineA);

				splineA->InvalidateGeomCache();
			}

			// Move to next knot and build its connection list
			iKnot++;
			connectionList.ZeroCount();
		}

		// Delete any polys removed
		int deleteCount = deleteIndexes.Count();
		if (deleteCount)
		{
			// DeleteSplines wants the list in ascending order, so sort first
			deleteIndexes.Sort(&DeleteSort);
			shape->shape.DeleteSplines((const unsigned int*)deleteIndexes.Addr(0), deleteCount);
		}
	}

	// Resume
	shape->InvalidateGeomCache();
	shape->shape.UpdateSels();	// important as it causes the object to always update

	ResumeAnimate();

	shape->UpdateValidity(GEOM_CHAN_NUM, iv);
	shape->UpdateValidity(TOPO_CHAN_NUM, iv);

	// Necessary for updates on underlying modifiers to appear in the viewport
	GetCOREInterface()->RedrawViews(t, REDRAW_NORMAL);
}


Interval VWeldMod::LocalValidity(TimeValue t) {
  // aszabo|feb.05.02 If we are being edited, 
	// return NEVER to forces a cache to be built after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;
	Interval iv = FOREVER;
	float v;
	mp_pblock->GetValue(kVwThreshold,t,v,iv);
	return iv;
}

RefTargetHandle VWeldMod::GetReference(int i) {
	switch (i) {
	case kVW_PBLOCK_REF: return mp_pblock;
	default: return NULL;
	}
}

void VWeldMod::SetReference(int i, RefTargetHandle rtarg) {
	switch (i) {
	case kVW_PBLOCK_REF: mp_pblock = (IParamBlock2*)rtarg; break;
	}
}



IOResult VWeldMod::Load(ILoad *iload)
{
	Modifier::Load(iload);
	IOResult res = IO_OK;

	// When loading a file, default to the old "FillInMesh" behavior (no FillInMesh)...
	// Newer files will load the appropriate flag setting;
	// Older files will use the old "no FillInMesh" behavior.
	mFillInMesh = false;

	// Add Spline Support
	int	weldVersion = WELD_LEGACY;
	ULONG nb = 0;

	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID()) {
		case kChunkUseRampageWeldMath:
			mUseRampageWeldMath = true;
			break;
		case kChunkFillInMesh:
			mFillInMesh = true;
			break;
		case kVersionChuck:
			res = iload->Read(&weldVersion, sizeof(int), &nb);
			break;
		}
		iload->CloseChunk();
		if (res!=IO_OK)  return res;
	}

	SetWeldVersion(weldVersion);

	DWORD fileVer = iload->GetFileSaveVersion();
	DWORD hiFileVer = HIWORD(fileVer);
	if ( (hiFileVer <= MAX_RELEASE_R14) )
	{
		mUseRampageWeldMath = true;
	}
	return IO_OK;
}

IOResult VWeldMod::Save(ISave *isave)
{
	Modifier::Save(isave);

	if (mUseRampageWeldMath)
	{
		isave->BeginChunk(kChunkUseRampageWeldMath);
		isave->EndChunk();
	}
	if (mFillInMesh)
	{
		isave->BeginChunk(kChunkFillInMesh);
		isave->EndChunk();
	}

	ULONG nb = 0;
	int		thisVersion = GetWeldVersion();
	isave->BeginChunk(kVersionChuck);
	isave->Write(&thisVersion, sizeof(int), &nb);
	isave->EndChunk();

	return IO_OK;
}