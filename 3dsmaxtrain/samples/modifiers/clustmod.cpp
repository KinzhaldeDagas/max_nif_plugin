/**********************************************************************
 *<
	FILE: clustmod.cpp   

	DESCRIPTION:  Vertex cluster animating modifier - XForm

	CREATED BY: Rolf Berteig

	HISTORY: created 24 August, 1995

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "mods.h"
#include "buildver.h"

#include "simpmod.h"
#include "iparamm2.h"

// Qt Auto UI
#include "ModifierQtUI.h"
#include "ui_ClustMod.h"


#define CLUSTER_VERSION_CHUNK 100
#define CLUSTER_MODIFIER_CHUNK 200

#define TIMENOW (GetCOREInterface()->GetTime())

ClustModRollup::ClustModRollup(QWidget* /*parent*/)
    :QMaxParamBlockWidget(),
    ui(new Ui::ClustModRollup())
{
    ui->setupUi(this);
}


ClustModRollup::~ClustModRollup(void)
{
    delete ui;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
//		Cluster Modifier 
//		Better known as Xform Modifier, not sure where the original name Cluster came from
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ClustMod : public SimpleMod2 {	

	public:
		enum ClustVersions {
			CLUST_UNVERSIONED,
			CLUST_2021
		};

		enum
		{
			clust_params,
		};
		
		enum ParamBlockEntryIDs
		{
			pb_preserveNormals,
		};
		

		ClustMod();
		void SetClustVersion(ClustVersions version);
		void DeleteThis() {delete this;}
		void GetClassName(MSTR& s, bool localized) const override { s= localized ? GetString(IDS_RB_XFORM_CLASS) : _T("XForm"); } 
		Class_ID ClassID() { return Class_ID(CLUSTOSM_CLASS_ID,0);}
		IOResult Load(ILoad *iload);
		IOResult Save(ISave * isave);
		RefTargetHandle Clone(RemapDir& remap);
		const TCHAR *GetObjectName(bool localized) const override { return localized ? GetString(IDS_RB_XFORM) : _T("XForm"); }
		Deformer& GetDeformer(TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat);	
		void BeginEditParams(IObjParam* ip, ULONG flags, Animatable* prev);		
		void EndEditParams(IObjParam* ip, ULONG flags, Animatable* next);
		ChannelMask ChannelsChanged()
		{
			if (ChangesTopology())
				return GEOM_CHANNEL | TOPO_CHANNEL;

			// We prefer this as it makes the modifier much more performant
			return GEOM_CHANNEL;
		}
		void ModifyObject(TimeValue t, ModContext &mc, ObjectState * os, INode *node);
		
		int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc);
		int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags, ModContext *mc);
		void GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc);
		void GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
		void GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
		void Move(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin);
		void ActivateSubobjSel(int level, XFormModes& modes);
		bool ChangesTopology();
		void SetInFileLoad(bool val)
		{
			mInFileLoad = val;
		}

private:
	int selLevel;
	ClustVersions clustVersion = CLUST_2021;
	int preserveNormals;
	bool mInFileLoad = false;
	};


class ClustModLoad : public PostLoadCallback {
public:
	ClustMod *n;
	ClustModLoad(ClustMod *ns) { n = ns; }
	void proc(ILoad *iLoad)
	{
		static constexpr DWORD versionCoeff = 1 << 16;
		n->SetInFileLoad(false);

		IParamBlock2* paramblock = n->GetParamBlock(ClustMod::clust_params);
		if (paramblock == nullptr)
			return;

		IParamBlock2PostLoadInfo* info = (IParamBlock2PostLoadInfo*)(n->GetParamBlock(ClustMod::clust_params)->GetInterface(IPARAMBLOCK2POSTLOADINFO_ID));
		if (info == nullptr)
		{
			//ParamBlock wasn't defined until 2021. Version older than that will be considered unversioned
			if (iLoad->FileSaveAsVersion() < MAX_RELEASE_R23)
				n->SetClustVersion(ClustMod::CLUST_UNVERSIONED);
			return;

		}
		DWORD paramVer = info->GetVersion();

		//versioning wasn't added until 2023. So XForms from 2021 / 2022 will have a P_VERSION that contains a HIWORD with the release version.
		if(paramVer > versionCoeff)
			paramVer = paramVer / versionCoeff;

		switch (paramVer)
		{
		case ClustMod::CLUST_UNVERSIONED: n->SetClustVersion(ClustMod::CLUST_UNVERSIONED); break;
		case MAX_RELEASE_R23:
		case MAX_RELEASE_R24:
		case ClustMod::CLUST_2021:
		default:n->SetClustVersion(ClustMod::CLUST_2021); break;
		}


		delete this;
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//		Cluster Deformer
//		This is the class whose MAP method is called to deform each point
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ClustDeformer : public Deformer
{
public:
	Matrix3 tm;

	ClustDeformer();
	ClustDeformer(Matrix3& modmat);

	Point3 Map(int i, Point3 p) const override;
	bool IsThreadSafe() const override;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//		Cluster Modifier Class Descriptor
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ClustClassDesc:public ClassDesc2 {

public:

	int 			IsPublic() {return 1;}
	void*			Create(BOOL loading = FALSE) {return new ClustMod;}
	const TCHAR*	ClassName() { return GetString(IDS_RB_XFORM_CLASS); }
	const TCHAR*	NonLocalizedClassName() { return _T("XForm"); }
	SClass_ID		SuperClassID() {return OSM_CLASS_ID; }
	Class_ID		ClassID() {return Class_ID(CLUSTOSM_CLASS_ID,0); }
	const TCHAR* 	Category() {return GetString(IDS_RB_DEFDEFORMATIONS);}
	virtual const TCHAR* InternalName() { return _T("ClusterModifier"); }
	virtual HINSTANCE HInstance() { return hInstance; }

	virtual MaxSDK::QMaxParamBlockWidget* CreateQtWidget(ReferenceMaker& owner, IParamBlock2& paramBlock,
			const MapID paramMapID, MSTR& rollupTitle, int& /*rollupFlags*/, int& /*rollupCategory*/) override
	{
        if (paramBlock.ID() == ClustMod::clust_params)
        {
            rollupTitle = GetString(IDS_RB_RESETXFORM);
            return new ClustModRollup;
        }
        DbgAssert(0);
        return nullptr;
	}
};

static ClustClassDesc clustDesc;
extern ClassDesc* GetClustModDesc() { return &clustDesc; }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//		Cluster Modifier Accessor
//		Allowing for a callback mechanism on GetValue/SetValue 
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ClustAccessor : public PBAccessor
{
	void Set(PB2Value& /*v*/, ReferenceMaker* owner, ParamID id, int /*tabIndex*/, TimeValue t) override
	{
		Interface* ip = GetCOREInterface();
		ip->RedrawViews(t, REDRAW_NORMAL);
	}
};

static ClustAccessor clustAccessor;


static ParamBlockDesc2 clust_param_blk(ClustMod::clust_params, _T("Xform Parameters"), 0, &clustDesc, P_AUTO_CONSTRUCT + P_AUTO_UI_QT + P_VERSION, static_cast<DWORD>(ClustMod::CLUST_2021) ,SIMPMOD_PBLOCKREF,
	// Params 
	ClustMod::pb_preserveNormals, _T("PreserveNormals"), TYPE_BOOL, P_RESET_DEFAULT, IDS_XFORM_PRESERVE_NORMALS, 
		p_default, TRUE,
		p_accessor, &clustAccessor,
		p_end, 
	p_end
);

ClustDeformer::ClustDeformer() 
{ 	
	tm.IdentityMatrix();	
}

ClustDeformer::ClustDeformer(Matrix3& modmat) 
{	
	tm = modmat;		
} 

Point3 ClustDeformer::Map(int, Point3 p) const
{
	p = p * tm;
	return p;
}

bool ClustDeformer::IsThreadSafe() const
{
	return true;
}

bool ClustMod::ChangesTopology()
{
	// Topology changes (faces are flipped) when preserve normals are enabled and the tmController matrix has parity
	if (pblock2->GetInt(pb_preserveNormals) == TRUE)
	{
		if (tmControl && !mInFileLoad) // don't evaluate unless scene set up
		{
			Interval valid = FOREVER;
			Matrix3 ctm;
			tmControl->GetValue(TIMENOW, &ctm, valid, CTRL_RELATIVE);
			return ctm.Parity();
		}
	}

	return false;
}

IOResult ClustMod::Load(ILoad *iload)
{
	iload->RegisterPostLoadCallback(new ClustModLoad(this));
	mInFileLoad = true;

	IOResult res;
	ULONG nb = 0;
	int version = 0;
	while (IO_OK == (res = iload->OpenChunk())) {
		switch (iload->CurChunkID())
		{
		case CLUSTER_VERSION_CHUNK:
			res = iload->Read(&version, sizeof(int), &nb);
			break;
		case CLUSTER_MODIFIER_CHUNK:
			res = Modifier::Load(iload);
			break;
		}
		iload->CloseChunk();
		if (res != IO_OK) return res;
	}

	clustVersion = static_cast<ClustVersions>(version);

	return IOResult::IO_OK;
}

IOResult ClustMod::Save(ISave* isave)
{
	ULONG nb = 0;
	int version = clustVersion;
	// Just saving the version number out to the file
	isave->BeginChunk(CLUSTER_VERSION_CHUNK);
	isave->Write(&version, sizeof(int), &nb);
	isave->EndChunk();
	// 2023.3 and later save this additional chunk, with the base Modifier class data (MAXX-69013)
	isave->BeginChunk(CLUSTER_MODIFIER_CHUNK);
	Modifier::Save(isave);
	isave->EndChunk();

	return IO_OK;
}

RefTargetHandle ClustMod::Clone(RemapDir& remap)
{
	ClustMod* newmod = new ClustMod();
	newmod->SimpleMod2Clone(this, remap);
	BaseClone(this, newmod, remap);
	return (newmod);
}

ClustMod::ClustMod()
{
	pblock2 = nullptr;
	
	GetClustModDesc()->MakeAutoParamBlocks(this);
	assert(pblock2);

	selLevel = 1;
	preserveNormals = 1;
}

void ClustMod::SetClustVersion(ClustVersions version)
{
	clustVersion = version;
	if(clustVersion < 1)
		pblock2->SetValue(pb_preserveNormals, TIME_PosInfinity,  false);
}

Deformer& ClustMod::GetDeformer(TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat)
{	
	static ClustDeformer deformer;
	assert(0);
	return deformer;
}

void ClustMod::BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev)
{
	SimpleMod2::BeginEditParams(ip,flags,prev);

	GetClustModDesc()->BeginEditParams(ip, this, flags, prev);
}

void ClustMod::EndEditParams(IObjParam* ip, ULONG flags, Animatable* next)
{
	SimpleMod2::EndEditParams(ip,flags,next);
	GetClustModDesc()->EndEditParams(ip, this, flags, next);
}

void ClustMod::ActivateSubobjSel(int level, XFormModes& modes)
	{
	selLevel = level;
	SimpleMod2::ActivateSubobjSel(level,modes);
	}

static Matrix3 CompTM(
		Matrix3 &ptm, Matrix3 &ctm, Matrix3 *mctm, int i)
	{
	if (mctm) {
		if (i) return  ptm * ctm * Inverse(*mctm);
		else return *mctm * ptm * ctm * Inverse(*mctm);
	} else {
		return ptm * ctm;
		}
	}

	void ClustMod::ModifyObject(TimeValue t, ModContext &mc, ObjectState * os, INode *node)
{
	Interval valid = FOREVER;
	Matrix3 ptm(1), ctm(1);

	if (posControl) posControl->GetValue(t, &ptm, valid, CTRL_RELATIVE);
	if (tmControl) tmControl->GetValue(t, &ctm, valid, CTRL_RELATIVE);
	
	pblock2->GetValue(pb_preserveNormals, t, preserveNormals, valid);

	// Checking parity for mirroring (scaling one or three axis negatively will return TRUE)
	if (ctm.Parity() && preserveNormals)
	{
		// Patch Object
		if (os->obj->IsSubClassOf(patchObjectClassID))
		{
			PatchObject* patchObj = (PatchObject*)os->obj;
			PatchMesh& patchMesh = patchObj->GetPatchMesh(t);
			BOOL useSel = patchMesh.selLevel >= PO_PATCH;

			patchMesh.FlipPatchNormal(useSel ? -1 : -2); // -1 selected, -2 all

			patchObj->UpdateValidity(TOPO_CHAN_NUM, valid);
		}
		// Poly Object
		else if (os->obj->IsSubClassOf(polyObjectClassID))
		{
			PolyObject* pPolyOb = (PolyObject*)os->obj;
			MNMesh& mesh = pPolyOb->GetMesh();

			// Check for faces selection mode 
			if (mesh.selLevel == MNM_SL_FACE)
			{
				mesh.FlipElementNormals(MN_SEL);
			}
			// Other selection modes
			else
			{
				for (int i = 0; i < mesh.FNum(); i++)
				{
					mesh.f[i].SetFlag(MN_WHATEVER, !mesh.f[i].GetFlag(MN_DEAD));
				}
				mesh.FlipElementNormals(MN_WHATEVER);
			}
			// No current support for specified normals
			mesh.ClearSpecifiedNormals();

			pPolyOb->UpdateValidity(GEOM_CHAN_NUM, valid);
			pPolyOb->UpdateValidity(TOPO_CHAN_NUM, valid);
		}
		// Others (Tri Objects or Convertible to Tri Objects, do nothing for the rest)
		else 
		{
			TriObject* triObj = nullptr;

			// Tri Objects
			if (os->obj->IsSubClassOf(triObjectClassID))
			{
				triObj = (TriObject*)os->obj;

			}
			// Convertible to Tri Objects
			else if (os->obj->CanConvertToType(triObjectClassID)) 
			{
					triObj = (TriObject*)os->obj->ConvertToType(t, triObjectClassID);
					os->obj = triObj; // Replace in the pipeline
			}

			if (triObj)
			{
				Mesh& triMesh = triObj->GetMesh();
				BOOL useSel = triMesh.selLevel == MESH_FACE;
				const BitArray& faceSel = triMesh.FaceSel();
				for (int i = 0; i < triMesh.getNumFaces(); i++)
				{
					// Flip normals of all or just selected faces depending on the selection mode
					if (!useSel || faceSel[i])
						triMesh.FlipNormal(int((DWORD)i));
				}
				triMesh.InvalidateTopologyCache();
				triObj->UpdateValidity(TOPO_CHAN_NUM, valid);
			}
		}
	}
	
	// Actual reset Xform work is done with the help of the deformer
	Matrix3 tm = CompTM(ptm,ctm,mc.tm,0);
	ClustDeformer deformer(tm);
	os->obj->Deform(&deformer, TRUE);
	os->obj->UpdateValidity(GEOM_CHAN_NUM,valid);
}

// RB 3/31/99: Push mc->box through this to handle the case where an XForm
// mod is applied to an empty sub-object selection.
static Box3& MakeBoxNotEmpty(Box3 &box)
	{
	static Box3 smallBox(Point3(-5,-5,-5),Point3( 5, 5, 5));
	if (box.IsEmpty()) return smallBox;
	else return box;
	}

int ClustMod::HitTest(
		TimeValue t, INode* inode, int type, int crossing, 
		int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc)
	{	
	int savedLimits;
	Matrix3 obtm = inode->GetObjectTM(t);
	GraphicsWindow *gw = vpt->getGW();
	HitRegion hr;
	MakeHitRegion(hr,type, crossing,4,p);
	gw->setHitRegion(&hr);
	gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);
	gw->clearHitCode();	
	gw->setTransform(obtm);

	Matrix3 ptm(1), ctm(1);
	if (posControl) posControl->GetValue(t,&ptm,FOREVER,CTRL_RELATIVE);
	if (tmControl) tmControl->GetValue(t,&ctm,FOREVER,CTRL_RELATIVE);	

	if (ip && ip->GetSubObjectLevel() == 1) {		
		Matrix3 tm = CompTM(ptm,ctm,mc->tm,1);

		ClustDeformer deformer(tm);
		if (mc->box->pmin==mc->box->pmax) {
			Point3 pt = mc->box->pmin * tm;
			gw->marker(&pt,ASTERISK_MRKR);
		} else {
			DoModifiedBox(MakeBoxNotEmpty(*mc->box),deformer,DrawLineProc(gw));
			}
		}

	if (ip && (ip->GetSubObjectLevel() == 1 ||
	           ip->GetSubObjectLevel() == 2)) {		
		//obtm = ctm * obtm;
		if (mc->tm) obtm = ctm * Inverse(*mc->tm) * obtm;
		else obtm = ctm * obtm;

		gw->setTransform(obtm);
		DrawCenterMark(DrawLineProc(gw),MakeBoxNotEmpty(*mc->box));
		}

	gw->setRndLimits(savedLimits);	
	if (gw->checkHitCode()) {
		vpt->LogHit(inode, mc, gw->getHitDistance(), 0, NULL); 
		return 1;
		}
	return 0;
	}

int ClustMod::Display(
		TimeValue t, INode* inode, ViewExp *vpt, 
		int flags, ModContext *mc)
	{
	// Transform the gizmo with the node.
	Matrix3 obtm = inode->GetObjectTM(t);

	GraphicsWindow *gw = vpt->getGW();
	gw->setTransform(obtm);

	Matrix3 ptm(1), ctm(1);
	if (posControl) posControl->GetValue(t,&ptm,FOREVER,CTRL_RELATIVE);
	if (tmControl) tmControl->GetValue(t,&ctm,FOREVER,CTRL_RELATIVE);	
	Matrix3 tm = CompTM(ptm,ctm,mc->tm,1);	

	ClustDeformer deformer(tm);	
	if (ip && ip->GetSubObjectLevel() == 1) {
		//gw->setColor( LINE_COLOR, (float)1.0, (float)1.0, (float)0.0);		
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_SEL_GIZMOS));
	} else {
		//gw->setColor( LINE_COLOR, (float).85, (float).5, (float)0.0);
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_GIZMOS));
		}
	if (mc->box->pmin==mc->box->pmax) {
		Point3 pt = mc->box->pmin * tm;
		gw->marker(&pt,ASTERISK_MRKR);		
	} else {
		DoModifiedBox(MakeBoxNotEmpty(*mc->box),deformer,DrawLineProc(gw));
		}

	//obtm = ctm * obtm;
	if (mc->tm) obtm = ctm * Inverse(*mc->tm) * obtm;
	else obtm = ctm * obtm;
	
	gw->setTransform(obtm);
	if ( ip && (ip->GetSubObjectLevel() == 1 ||
	            ip->GetSubObjectLevel() == 2) ) {		
		//gw->setColor( LINE_COLOR, (float)1.0, (float)1.0, (float)0.0);
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_SEL_GIZMOS));
	} else {
		//gw->setColor( LINE_COLOR, (float).85, (float).5, (float)0.0);
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_GIZMOS));
		}	
	DrawCenterMark(DrawLineProc(gw),MakeBoxNotEmpty(*mc->box));	
	return 0;
	}

void ClustMod::GetWorldBoundBox(
		TimeValue t,INode* inode, ViewExp *vpt, 
		Box3& box, ModContext *mc)
	{

	// Need the correct bound box for proper damage rect calcs.
	Matrix3 obtm = inode->GetObjectTM(t);
	GraphicsWindow *gw = vpt->getGW();
	
	Matrix3 ptm(1), ctm(1);
	if (posControl) posControl->GetValue(t,&ptm,FOREVER,CTRL_RELATIVE);
	if (tmControl) tmControl->GetValue(t,&ctm,FOREVER,CTRL_RELATIVE);	
	Matrix3 tm = CompTM(ptm,ctm,mc->tm,1);
	ClustDeformer deformer(tm);	
	
	BoxLineProc bp1(&obtm);
	DoModifiedBox(MakeBoxNotEmpty(*mc->box), deformer, bp1);
	box = bp1.Box();

	//obtm = ctm * obtm;
	if (mc->tm) obtm = ctm * Inverse(*mc->tm) * obtm;
	else obtm = ctm * obtm;

	BoxLineProc bp2(&obtm);		
	DrawCenterMark(bp2,MakeBoxNotEmpty(*mc->box));
	box += bp2.Box();
	}

void ClustMod::GetSubObjectCenters(
		SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc)
	{
	Matrix3 obtm = node->GetObjectTM(t);
	Matrix3 ptm(1), ctm(1);
	if (posControl) posControl->GetValue(t,&ptm,FOREVER,CTRL_RELATIVE);	
	if (tmControl) tmControl->GetValue(t,&ctm,FOREVER,CTRL_RELATIVE);

	if (cb->Type()==SO_CENTER_PIVOT) {
		//Matrix3 mat = ctm * obtm;
		Matrix3 mat;		
		if (mc->tm) mat = ctm * Inverse(*mc->tm) * obtm;
		else mat = ctm * obtm;
		cb->Center(mat.GetTrans(),0);
	} else {		
		Matrix3 tm = CompTM(ptm,ctm,mc->tm,1);
		ClustDeformer deformer(tm);
		BoxLineProc bp1(&obtm);
		DoModifiedBox(MakeBoxNotEmpty(*mc->box), deformer, bp1);
		cb->Center(bp1.Box().Center(),0);
		}
	}

void ClustMod::GetSubObjectTMs(
		SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc)
	{
	Matrix3 obtm = node->GetObjectTM(t);
	Matrix3 ptm(1), ctm(1);
	if (posControl) posControl->GetValue(t,&ptm,FOREVER,CTRL_RELATIVE);
	if (tmControl) tmControl->GetValue(t,&ctm,FOREVER,CTRL_RELATIVE);	
	Matrix3 tm = CompTM(ptm,ctm,mc->tm,1) * obtm;
	cb->TM(tm,0);
	}

void ClustMod::Move(
		TimeValue t, Matrix3& partm, Matrix3& tmAxis, 
		Point3& val, BOOL localOrigin) 
	{
	if (tmControl==NULL) {
		ReplaceReference(0,NewDefaultMatrix3Controller()); 
		NotifyDependents(FOREVER,0,REFMSG_CONTROLREF_CHANGE);
		}

	if (ip && ip->GetSubObjectLevel()==1) {				
		SetXFormPacket pckt(val,partm,tmAxis);
		tmControl->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);		
	} else {		
		if (posControl==NULL) {
			ReplaceReference(1,NewDefaultPositionController()); 
			NotifyDependents(FOREVER,0,REFMSG_CONTROLREF_CHANGE);
			}
		Matrix3 ptm = partm;
		Interval valid;
		if (tmControl)
			tmControl->GetValue(t,&ptm,valid,CTRL_RELATIVE);
		posControl->SetValue(t,-VectorTransform(tmAxis*Inverse(ptm),val),TRUE,CTRL_RELATIVE);
		
		SetXFormPacket pckt(val,partm,tmAxis);
		tmControl->SetValue(t,&pckt,TRUE,CTRL_RELATIVE);
		}
	}
