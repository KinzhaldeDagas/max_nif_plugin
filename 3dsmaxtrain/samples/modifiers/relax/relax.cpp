/**********************************************************************
 *<
	FILE: relax.cpp

	DESCRIPTION:  Averages verts with nearby verts.

	CREATED BY: Steve Anderson

	HISTORY: created March 4, 1996
			3/29/20 - PB2/QT Migration, Preserve Volume (M. Kaustinen)

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/

#include "relaxmod.h"
#include <tbb/blocked_range.h>
#include <tbb/partitioner.h>
#include <tbb/parallel_for.h>
#include "PerformanceTools.h"

// QT Support
#include "ui_Relax.h"

//--- Relax -----------------------------------------------------------

// Version info added for MAXr4, where we can now be applied to a PatchMesh and retain identity!
#define RELAXMOD_VER1 1
#define RELAXMOD_VER4 4

#define RELAXMOD_CURRENT_VERSION RELAXMOD_VER4

#define RELAX_OSM_CLASS_ID	100

#define MAX_ITER	999999999
#define MIN_ITER	0
#define MAX_RELAX	1.0f
#define MIN_RELAX	-1.0f
#define UIMIN_RELAX	0.0f

#define DEF_RELAX		0.5f
#define DEF_ITER		1
#define DEF_BOUNDARY	0
#define DEF_SADDLE		0
#define DEF_PRESERVE	1

#define	PBLOCK_REF	0

RelaxRollup::RelaxRollup(QWidget* /*parent*/) :
	QMaxParamBlockWidget(),
	m_UI(new Ui::RelaxRollup())
{
	mMod = nullptr;
	m_UI->setupUi(this);
}

RelaxRollup::~RelaxRollup(void)
{
	delete m_UI;
}

void RelaxRollup::SetParamBlock(ReferenceMaker* owner, IParamBlock2* const /*param_block*/)
{
	mMod = (RelaxMod*)owner;
}

void RelaxRollup::UpdateUI(const TimeValue /*t*/)
{
}

void RelaxRollup::UpdateParameterUI(const TimeValue /*t*/, const ParamID /*param_id*/, const int /*tab_index*/)
{
}


typedef Tab<DWORD> DWordTab;

class RelaxModData : public LocalModData {
public:
	DWordTab *nbor;	// Array of neighbors for each vert.
	BitArray boundaryVert;	// Flag if Vertex is on a Boundary
	int *fnum;		// Number of faces for each vert.
	BitArray sel;		// Selection information.
	int vnum;		// Size of above arrays
	Interval ivalid;	// Validity interval of arrays.

	RelaxModData ();
	~RelaxModData () { Clear(); }
	void Clear();
	void SetVNum (int num);
	void MaybeAppendNeighbor(int vert, int index, int &max)
	{
		for (int k1=0; k1<max; k1++)
		{
			if (nbor[vert][k1] == index)
				return;
		}
		DWORD dwi = (DWORD)index;
		nbor[vert].Append (1, &dwi, 1);
		max++;
	}
	LocalModData *Clone ();
};

RelaxModData::RelaxModData ()
{
	nbor = NULL;
	fnum = NULL;
	vnum = 0;
	ivalid = NEVER;
}

void RelaxModData::Clear () {
	if (nbor) delete [] nbor;
	nbor = NULL;
	if (fnum) delete [] fnum;
	fnum = NULL;
}

void RelaxModData::SetVNum (int num)
{
	if (num==vnum) return;
	Clear();
	vnum = num;
	if (num<1) return;
	nbor = new DWordTab[vnum];

	boundaryVert.SetSize(vnum);
	fnum = new int[vnum];
	sel.SetSize (vnum);
}

LocalModData *RelaxModData::Clone ()
{
	RelaxModData *clone;
	clone = new RelaxModData ();
	clone->SetVNum (vnum);
	for (int i=0; i<vnum; i++) {
		clone->nbor[i] = nbor[i];
	}
	memcpy_s(clone->fnum, vnum * sizeof(int), fnum, vnum*sizeof(int));
	clone->boundaryVert = boundaryVert;
	clone->sel = sel;
	clone->ivalid = ivalid;
	return clone;
}

class RelaxMod: public Modifier {
protected:
	IParamBlock2 *pblock2;
	int version;

	static IObjParam *ip;

public:

	// ParamBlock2
	enum { relax_params };
	enum {
		relax_amount,
		relax_iterations,
		relax_boundary,
		relax_saddle,
		relax_preserve
	};

	static RelaxMod *editMod;

	// From Animatable
	void DeleteThis() { delete this; }
	void GetClassName(MSTR& s, bool localized) const override { s= localized ? GetString(IDS_RELAXMOD) : _T("RelaxMod"); }  
	virtual Class_ID ClassID() { return Class_ID(RELAX_OSM_CLASS_ID,32670); }		
	RefTargetHandle Clone(RemapDir& remap);
	const TCHAR *GetObjectName(bool localized) const override { return localized ? GetString (IDS_RELAX) : _T("Relax"); }
	// IO
	bool SpecifySaveReferences(ReferenceSaveManager& referenceSaveManager);
	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);

	RelaxMod();
	virtual ~RelaxMod();

	ChannelMask ChannelsUsed()  { return GEOM_CHANNEL | TOPO_CHANNEL | SUBSEL_TYPE_CHANNEL | SELECT_CHANNEL; }
	ChannelMask ChannelsChanged() { return GEOM_CHANNEL; }
	void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
	void NotifyInputChanged(const Interval& changeInt, PartID partID, RefMessage message, ModContext *mc);
	Class_ID InputType() { return defObjectClassID; }
	Interval LocalValidity(TimeValue t);

	// From BaseObject
	BOOL ChangeTopology() {return FALSE;}

	int NumRefs() {return 1;}
	virtual RefTargetHandle GetReference(int i)
	{
		if (i == PBLOCK_REF)
			return pblock2;
		return nullptr;
	}

	virtual int    NumParamBlocks() { return 1; }     // return number of ParamBlocks in this instance
	virtual IParamBlock2* GetParamBlock(int /*i*/) { return pblock2; } // return i'th ParamBlock
	virtual IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock2->ID() == id) ? pblock2 : NULL; } // return id'd ParamBlock

protected:
	virtual void SetReference(int i, RefTargetHandle rtarg) {pblock2=(IParamBlock2*)rtarg;}
public:

 	int NumSubs() { return 1; }  
	Animatable* SubAnim(int i) { return pblock2; }
	TSTR SubAnimName(int i, bool localized) override { return localized ? GetString(IDS_PARAMETERS) : _T("Parameters");}

	RefResult NotifyRefChanged( const Interval& changeInt,RefTargetHandle hTarget, 
	   PartID& partID, RefMessage message, BOOL propagate);

	CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;} 

	void BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev);
	void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next);

	void UpdateUI(TimeValue t) {}
	Interval GetValidity(TimeValue t);

	RelaxRollup* mRollup;
};

class RelaxClassDesc:public ClassDesc2 {
	public:
	virtual int 		IsPublic() { return TRUE; }
	virtual void*		Create(BOOL loading = FALSE) { return new RelaxMod; }
	virtual const TCHAR*	ClassName() { return GetString(IDS_RELAX); }
	virtual const TCHAR*	NonLocalizedClassName() { return _T("Relax"); }
	virtual SClass_ID	SuperClassID() { return OSM_CLASS_ID; }
	virtual Class_ID	ClassID() { return  Class_ID(RELAX_OSM_CLASS_ID,32670); }
	virtual const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }

	virtual const TCHAR*	InternalName() { return _T("Relax"); }	// returns fixed parsable name (scripter-visible name)
	virtual bool UseOnlyInternalNameForMAXScriptExposure() { return true; }
	virtual HINSTANCE HInstance() { return hInstance; }			// returns owning module handle

	virtual MaxSDK::QMaxParamBlockWidget* CreateQtWidget(ReferenceMaker& owner, IParamBlock2& paramBlock, const MapID, MSTR& rollupTitle, int&, int&) override
	{
		if (paramBlock.ID() == RelaxMod::relax_params) {
			RelaxRollup*rr = new RelaxRollup();
			((RelaxMod&)owner).mRollup = rr;
			rr->SetParamBlock(&owner, &paramBlock);
			//: this is a comment for the translation team 
			rollupTitle = RelaxRollup::tr("Relax");
			return rr;
		}
		return nullptr;
	}
};

static RelaxClassDesc RelaxDesc;
extern ClassDesc2* GetRelaxModDesc() { return &RelaxDesc; }

class RelaxPBAccessor : PBAccessor
{
	virtual void Set(PB2Value& /*v*/, ReferenceMaker* /*owner*/, ParamID /*id*/, int /*tabIndex*/, TimeValue t) override
	{
		Interface* ip = GetCOREInterface();
		ip->RedrawViews(t, REDRAW_NORMAL);
	}
};
static RelaxPBAccessor s_accessor;

static ParamBlockDesc2 relax_param_blk(RelaxMod::relax_params, _T("Relax Parameters"), 0, &RelaxDesc, P_AUTO_CONSTRUCT + P_AUTO_UI_QT, PBLOCK_REF,
	// params
	RelaxMod::relax_amount, _T("Value"), TYPE_FLOAT, P_ANIMATABLE | P_RESET_DEFAULT, IDS_RVALUE,
	p_default, DEF_RELAX,
	p_range, UIMIN_RELAX, MAX_RELAX,
	p_accessor, &s_accessor,
	p_nonLocalizedName, _T("Relax Value"),
	p_end,

	RelaxMod::relax_iterations, _T("Iterations"), TYPE_INT, P_ANIMATABLE | P_RESET_DEFAULT, IDS_ITERATIONS,
	p_default, 1,
	p_range, 1, MAX_ITER,
	p_accessor, &s_accessor,
	p_end,

	RelaxMod::relax_boundary, _T("Boundary"), TYPE_BOOL, P_RESET_DEFAULT, IDS_BOUNDARY,
	p_default, DEF_BOUNDARY,
	p_accessor, &s_accessor,
	p_nonLocalizedName, _T("Keep Boundary Pts Fixed"),
	p_end,

	RelaxMod::relax_saddle, _T("Saddle"), TYPE_BOOL, P_RESET_DEFAULT, IDS_SADDLE,
	p_default, DEF_SADDLE,
	p_accessor, &s_accessor,
	p_end,

	RelaxMod::relax_preserve, _T("PreserveVolume"), TYPE_BOOL, P_RESET_DEFAULT, IDS_PRESERVE_VOLUME,
	p_default, DEF_PRESERVE,
	p_accessor, &s_accessor,
	p_end,

	p_end
);

IObjParam*	RelaxMod::ip	= NULL;
RelaxMod *RelaxMod::editMod = NULL;

//--- Parameter map/block descriptors (Legacy) -------------------------------

#define PB_RELAX	0
#define PB_ITER		1
#define PB_BOUNDARY	2
#define PB_SADDLE	3

static ParamBlockDescID descVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE,  PB_RELAX },
	{ TYPE_INT,   NULL, TRUE,  PB_ITER },
	{ TYPE_INT,   NULL, FALSE, PB_BOUNDARY }
};

static ParamBlockDescID descVer1[] = {
	{ TYPE_FLOAT, NULL, TRUE,  PB_RELAX },
	{ TYPE_INT,   NULL, TRUE,  PB_ITER },
	{ TYPE_INT,   NULL, FALSE, PB_BOUNDARY },
	{ TYPE_INT,   NULL, FALSE, PB_SADDLE }
};

#define PBLOCK_LENGTH	4

// Array of old versions
static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer0,3,0),
	ParamVersionDesc(descVer1,PBLOCK_LENGTH,1)
};
#define NUM_OLDVERSIONS	2

// Current version
#define CURRENT_VERSION	1

//--- RelaxMod -------------------------------

RelaxMod::RelaxMod() :
	pblock2(NULL),
	mRollup(NULL)
{
	RelaxDesc.MakeAutoParamBlocks(this);
	version = RELAXMOD_CURRENT_VERSION;
}

RelaxMod::~RelaxMod() { }

Interval RelaxMod::LocalValidity(TimeValue t) {
	// if being edited, return NEVER forces a cache to be built 
	// after previous modifier.
	if (TestAFlag(A_MOD_BEING_EDITED)) return NEVER;  
	Interval valid = GetValidity(t);	
	return valid;
}

RefTargetHandle RelaxMod::Clone(RemapDir& remap) {
	RelaxMod* newmod = new RelaxMod();	
	newmod->ReplaceReference(0,remap.CloneRef(pblock2));
	newmod->version = version;
	BaseClone(this, newmod, remap);
	return(newmod);
}

static void FindVertexAngles(PatchMesh &pm, float *vang) {
	int i;
	for (i=0; i<pm.numVerts + pm.numVecs; i++) vang[i] = 0.0f;
	for (i=0; i<pm.numPatches; i++) {
		Patch &p = pm.patches[i];
		for (int j=0; j<p.type; j++) {
			Point3 d1 = pm.vecs[p.vec[j*2]].p - pm.verts[p.v[j]].p;
			Point3 d2 = pm.vecs[p.vec[((j+p.type-1)%p.type)*2+1]].p - pm.verts[p.v[j]].p;
			float len = LengthSquared(d1);
			if (len == 0) continue;
			d1 /= Sqrt(len);
			len = LengthSquared (d2);
			if (len==0) continue;
			d2 /= Sqrt(len);
			float cs = DotProd (d1, d2);
			if (cs>=1) continue;	// angle of 0
			if (cs<=-1) vang[p.v[j]] += PI;
			else vang[p.v[j]] += (float) acos (cs);
		}
	}
}

static void FindVertexAngles (MNMesh &mm, Point3* verts, float *vang) {
	int i;
	for (i=0; i<mm.numv; i++) vang[i] = 0.0f;
	for (i=0; i<mm.numf; i++) {
		int *vv = mm.f[i].vtx;
		int deg = mm.f[i].deg;
		for (int j=0; j<deg; j++)
		{
			Point3 d1 = verts[vv[(j + 1) % deg]] - verts[vv[j]];
			Point3 d2 = verts[vv[(j + deg - 1) % deg]] - verts[vv[j]];
			float len = LengthSquared(d1);
			if (len == 0) continue;
			d1 /= Sqrt(len);
			len = LengthSquared (d2);
			if (len==0) continue;
			d2 /= Sqrt(len);
			float cs = DotProd (d1, d2);
			// STEVE: What about angles over PI?
			if (cs>=1) continue;	// angle of 0
			if (cs<=-1) vang[vv[j]] += PI;
			else vang[vv[j]] += (float) acos (cs);
		}
	}
}

void RelaxMod::ModifyObject(TimeValue t, ModContext &mc, ObjectState * os, INode *node)
{
	using namespace MaxSDK::PerformanceTools;
	
	// Min number of verts per thread
	constexpr	int	minGrainSize = 100;

	Interval valid = FOREVER;

	// Update Validity Interval
	valid &= os->obj->ChannelValidity (t, GEOM_CHAN_NUM);
	valid &= os->obj->ChannelValidity (t, TOPO_CHAN_NUM);
	valid &= os->obj->ChannelValidity (t, SUBSEL_TYPE_CHAN_NUM);
	valid &= os->obj->ChannelValidity (t, SELECT_CHAN_NUM);
	Matrix3 modmat,minv;
	
	if (mc.localData == NULL) mc.localData = new RelaxModData;
	RelaxModData *rd = (RelaxModData *) mc.localData;

	TriObject *triObj = NULL;
	PatchObject *patchObj = NULL;
	PolyObject *polyObj = NULL;
	BOOL converted = FALSE;

	// For version 4 and later, we process patch or poly meshes as they are and pass them on. 
	// Earlier versions converted to TriMeshes (done below).
	// For adding other new types of objects, add them here!
	if(version >= RELAXMOD_VER4 && os->obj->IsSubClassOf(patchObjectClassID))
	{
		patchObj = (PatchObject *)os->obj;
	}
	else	// If it's a TriObject, process it
	if(os->obj->IsSubClassOf(triObjectClassID))
	{
		triObj = (TriObject *)os->obj;
	}
	else if (os->obj->IsSubClassOf (polyObjectClassID))
	{
		polyObj = (PolyObject *) os->obj;
	}
	else	// If it can convert to a TriObject, do it
	if(os->obj->CanConvertToType(triObjectClassID))
	{
		triObj = (TriObject *)os->obj->ConvertToType(t, triObjectClassID);
		converted = TRUE;
	}
	else
		return;		// We can't deal with it!

	Mesh *mesh = triObj ? &(triObj->GetMesh()) : NULL;
	PatchMesh &pmesh = patchObj ? patchObj->GetPatchMesh(t) : *((PatchMesh *)NULL);

	float relax;
	int iter;

	pblock2->GetValue (relax_amount,  t, relax,	 valid);
	pblock2->GetValue (relax_iterations,	 t, iter,	 valid);
	BOOL boundary = pblock2->GetInt(relax_boundary);
	BOOL saddle = pblock2->GetInt(relax_saddle); 
	BOOL preserve = pblock2->GetInt(relax_preserve);

	LimitValue (relax, MIN_RELAX, MAX_RELAX);
	LimitValue (iter, MIN_ITER, MAX_ITER);

	// For preservation, we need to scale below .707, otherwise it goes out of control
	if (preserve) {
		relax *= 0.707f;
	}

	// How much do we scale back
	constexpr	float	RELAX_BACK_FACTOR = -0.97f;
	float	expand = relax * RELAX_BACK_FACTOR;

	if(triObj)
	{
		//int i, j, max;
		DWORD selLevel = mesh->selLevel;
		// mjm - 4.8.99 - add support for soft selection
		// sca - 4.29.99 - extended soft selection support to cover EDGE and FACE selection levels.
		float *vsw = (selLevel!=MESH_OBJECT) ? mesh->getVSelectionWeights() : NULL;

		if (rd->ivalid.InInterval(t) && (mesh->numVerts != rd->vnum))
		{
			// Shouldn't happen, but does with Loft bug and may with other bugs.
			rd->ivalid.SetEmpty ();
		}

		if (!rd->ivalid.InInterval(t))
		{
			rd->SetVNum (mesh->numVerts);

			for (int i=0; i<rd->vnum; i++)
			{
				rd->fnum[i]=0;
				rd->nbor[i].ZeroCount();
			}
			rd->sel.ClearAll ();
			rd->boundaryVert.ClearAll();

			// Collect visibilty flags for all of the vert neighbours so we can simplify the neighbour list later
			BitArray* vis = new BitArray[rd->vnum];

			int k1, k2, origmax;
			const BitArray& faceSel = mesh->FaceSel();
			const BitArray& edgeSel = mesh->EdgeSel();
			for (int i=0; i<mesh->numFaces; i++)
			{
				DWORD* v = mesh->faces[i].v;

				for (int j=0; j<3; j++)
				{
					DWORD	index = v[j];
					DWORD	index1 = v[(j + 1) % 3];
					DWORD	index2 = v[(j + 2) % 3];

					if ((selLevel != MESH_OBJECT) && (selLevel != MESH_VERTEX))
					{
						if (selLevel == MESH_FACE)
						{
							if (faceSel[i])
								rd->sel.Set(index);
						}
						else if (selLevel == MESH_EDGE)
						{
							if ((edgeSel[i * 3 + j]) || (edgeSel[i * 3 + (j + 2) % 3]))
								rd->sel.Set(index);
						}
					}

					int max = origmax = rd->nbor[index].Count();
					rd->fnum[index]++;

					// Have we already added vert1
					for (k1 = 0; k1 < max; k1++)
					{
						if (rd->nbor[index][k1] == index1)
							break;
					}

					if (k1==max)
					{
						rd->nbor[index].Append(1, &index1, 1);
						max++;
					}

					// Have we already added vert2
					for (k2 = 0; k2 < max; k2++)
					{
						if (rd->nbor[index][k2] == index2)
							break;
					}

					if (k2==max)
					{
						rd->nbor[index].Append(1, &index2, 1);
						max++;
					}

					if (max > origmax)
						vis[index].SetSize(max, TRUE);

					if (mesh->faces[i].getEdgeVis(j))
						vis[index].Set(k1);
					else if (k1 >= origmax)
						vis[index].Clear(k1);

					if (mesh->faces[i].getEdgeVis((j + 2) % 3))
						vis[index].Set(k2);
					else if (k2 >= origmax)
						vis[index].Clear(k2);
				}
			}

			// Post process our neighbour array
			for (int i = 0; i < rd->vnum; i++) {

				int	max = rd->nbor[i].Count();

				// Flag if boundary vert (faces less that neighbour verts)
				rd->boundaryVert.Set(i, max > rd->fnum[i]);

				// Update our neighbhour list to only include visible ones as we only
				// care about those in our relax iterations.  Check backwards to speed up removals.
				for (int j = max - 1; j >= 0; j--)
				{
					if (!vis[i][j])
						rd->nbor[i].Delete(j, 1);
				}
			}

			if (vis)
				delete[] vis;

			if (selLevel==MESH_VERTEX)
				rd->sel = mesh->VertSel();
			else if (selLevel==MESH_OBJECT)
				rd->sel.SetAll ();

			rd->ivalid  = os->obj->ChannelValidity (t, TOPO_CHAN_NUM);
			rd->ivalid &= os->obj->ChannelValidity (t, SUBSEL_TYPE_CHAN_NUM);
			rd->ivalid &= os->obj->ChannelValidity (t, SELECT_CHAN_NUM);
		}

		Tab<float> vangles;
		if (saddle) vangles.SetCount (rd->vnum);

		Point3 *hold = new Point3[rd->vnum];
		// Add additional cache to speed up iterations
		Point3 *hold2 = new Point3[rd->vnum];

		// Cache the triobj value
		memcpy_s(hold, rd->vnum * sizeof(Point3), triObj->mesh.verts, rd->vnum * sizeof(Point3));
		memcpy_s(hold2, rd->vnum * sizeof(Point3), hold, rd->vnum * sizeof(Point3));

		// Reference pointers so we can toggle between the two
		Point3	*sourceArray = hold;
		Point3	*destArray = hold2;

		// Keep an array of weighted values for fast referal during iterations
		float *relaxArray = nullptr;
		float *expandArray = nullptr;
		if (vsw) {
			relaxArray = new float[rd->vnum];
			for (int i = 0; i < rd->vnum; i++)
			{
				relaxArray[i] = (!rd->sel[i]) ? relax * vsw[i] : relax;
			}

			if (preserve) {
				expandArray = new float[rd->vnum];
				for (int i = 0; i < rd->vnum; i++)
				{
					expandArray[i] = (!rd->sel[i]) ? expand * vsw[i] : expand;
				}
			}
		}

		for (int k=0; k<iter; k++)
		{
			if (saddle) {
				// Push verts back so we can re-calc angles
				if (k != 0) {
					for (int i = 0; i < rd->vnum; i++)
						triObj->SetPoint(i, sourceArray[i]);
				}
				mesh->FindVertexAngles(vangles.Addr(0));
			}
		
			tbb::parallel_for(tbb::blocked_range<int>(0, rd->vnum), [&](const auto& range)
			{
				for (int i = range.begin(); i < range.end(); i++)
				{
					if ((!rd->sel[i]) && (!vsw || vsw[i] == 0.0f))
						continue;

					if (saddle && (vangles[i] <= 2 * PI*.99999f))
						continue;

					if (boundary && rd->boundaryVert[i])
						continue;

					int max = rd->nbor[i].Count();
					if (max < 1)
						continue;

					Point3 avg = sourceArray[rd->nbor[i][0]];
					for (int j = 1; j < max; j++)
					{
						avg += sourceArray[rd->nbor[i][j]];
					}

					if (relaxArray)
						destArray[i] = (sourceArray[i] * (1.0f - relaxArray[i])) + (avg * (relaxArray[i] / max));
					else
						destArray[i] = (sourceArray[i] * (1.0f - relax)) + (avg * (relax / max));
				}
			});

			// swap arrays
			Point3	*pTemp = sourceArray;
			sourceArray = destArray;
			destArray = pTemp;

			// Preserve Volume
			if (preserve)
			{
				tbb::parallel_for(tbb::blocked_range<int>(0, rd->vnum), [&](const auto& range)
				{
					for (int i = range.begin(); i < range.end(); i++)
					{
						if ((!rd->sel[i]) && (!vsw || vsw[i] == 0))
							continue;

						if (saddle && (vangles[i] <= 2 * PI*.99999f))
							continue;

						if (boundary && rd->boundaryVert[i])
							continue;

						int max = rd->nbor[i].Count();
						if (max < 1)
							continue;

						Point3 avg = sourceArray[rd->nbor[i][0]];
						for (int j = 1; j < max; j++) {
							avg += sourceArray[rd->nbor[i][j]];
						}

						if (expandArray)
							destArray[i] = (sourceArray[i] * (1.0f - expandArray[i])) + (avg * (expandArray[i] / max));
						else
							destArray[i] = (sourceArray[i] * (1.0f - expand)) + (avg * (expand / max));
					}
				});

				// swap arrays
				pTemp = sourceArray;
				sourceArray = destArray;
				destArray = pTemp;
			}
		}

		// Now copy into the TriObj
		memcpy_s(triObj->mesh.verts, rd->vnum * sizeof(Point3), sourceArray, rd->vnum * sizeof(Point3));

		// Clean up
		if (hold)
			delete[] hold;
		if (hold2)
			delete[] hold2;
		if (relaxArray)
			delete[] relaxArray;
		if (expandArray)
			delete[] expandArray;
	}

	if (polyObj) {
		MNMesh & mm = polyObj->mm;
		float *vsw = (mm.selLevel!=MNM_SL_OBJECT) ? mm.getVSelectionWeights() : NULL;

		if (rd->ivalid.InInterval(t) && (mm.numv != rd->vnum)) {
			// Shouldn't happen, but does with Loft bug and may with other bugs.
			rd->ivalid.SetEmpty ();
		}

		if (!rd->ivalid.InInterval(t)) {
			rd->SetVNum (mm.numv);
			for (int i=0; i<rd->vnum; i++) {
				rd->fnum[i]=0;
				rd->nbor[i].ZeroCount();
			}
			rd->sel = mm.VertexTempSel ();

			int k1, k2, origmax;
			for (int i=0; i<mm.numf; i++) {
				int deg = mm.f[i].deg;
				int *vtx = mm.f[i].vtx;
				for (int j=0; j<deg; j++) {
					Tab<DWORD> & nbor = rd->nbor[vtx[j]];
					int max = origmax = nbor.Count();
					rd->fnum[vtx[j]]++;
					DWORD va = vtx[(j+1)%deg];
					DWORD vb = vtx[(j+deg-1)%deg];
					for (k1=0; k1<max; k1++) if (nbor[k1] == va) break;
					if (k1==max) { nbor.Append (1, &va, 1); max++; }
					for (k2=0; k2<max; k2++) if (nbor[k2] == vb) break;
					if (k2==max) { nbor.Append (1, &vb, 1); max++; }
				}
			}
			rd->ivalid = os->obj->ChannelValidity (t, TOPO_CHAN_NUM);
			rd->ivalid &= os->obj->ChannelValidity (t, SUBSEL_TYPE_CHAN_NUM);
			rd->ivalid &= os->obj->ChannelValidity (t, SELECT_CHAN_NUM);
		}

		// Keep an array of weighted values for fast referal during iterations
		float *relaxArray = nullptr;
		float *expandArray = nullptr;
		if (vsw) {
			relaxArray = new float[rd->vnum];
			for (int i = 0; i < rd->vnum; i++)
			{
				relaxArray[i] = (!rd->sel[i]) ? relax * vsw[i] : relax;
			}

			if (preserve) {
				expandArray = new float[rd->vnum];
				for (int i = 0; i < rd->vnum; i++)
				{
					expandArray[i] = (!rd->sel[i]) ? expand * vsw[i] : expand;
				}
			}
		}

		Tab<float> vangles;
		if (saddle) vangles.SetCount (rd->vnum);

		Point3 *hold = new Point3[rd->vnum];
		// Add additional cache to speed up iterations
		Point3 *hold2 = new Point3[rd->vnum];

		// Cache the Vertex values
		for (int i = 0; i < rd->vnum; i++)
			hold[i] = mm.P(i);

		memcpy_s(hold2, rd->vnum * sizeof(Point3), hold, rd->vnum * sizeof(Point3));

		// Reference pointers so we can toggle between the two
		Point3	*sourceArray = hold;
		Point3	*destArray = hold2;

		for (int k = 0; k < iter; k++)
		{
			if (saddle)
			{
				FindVertexAngles(mm, sourceArray, vangles.Addr(0));
			}

			tbb::parallel_for(tbb::blocked_range<int>(0, rd->vnum), [&](const auto& range)
			{
				for (int i = range.begin(); i < range.end(); i++)
				{
					if ((!rd->sel[i]) && (!vsw || vsw[i] == 0))
						continue;

					if (saddle && (vangles[i] <= 2 * PI*.99999f))
						continue;

					int max = rd->nbor[i].Count();
					if ((max < 1) || (boundary && (rd->fnum[i] < max)))
						continue;

					Point3 avg = sourceArray[rd->nbor[i][0]];
					for (int j = 1; j < max; j++) {
						avg += sourceArray[rd->nbor[i][j]];
					}

					if (relaxArray)
						destArray[i] = sourceArray[i] * (1.0f - relaxArray[i]) + (avg * (relaxArray[i] / max));
					else
						destArray[i] = sourceArray[i] * (1.0f - relax) + (avg * (relax / max));
				}
			});

			// Swap Arrays
			Point3	*pTemp = sourceArray;
			sourceArray = destArray;
			destArray = pTemp;

			// Now expand for volume preservation
			if (preserve)
			{
				tbb::parallel_for(tbb::blocked_range<int>(0, rd->vnum), [&](const auto& range)
				{
					for (int i = range.begin(); i < range.end(); i++)
					{
						if ((!rd->sel[i]) && (!vsw || vsw[i] == 0))
							continue;

						if (saddle && (vangles[i] <= 2 * PI*.99999f))
							continue;

						int max = rd->nbor[i].Count();
						if ((max < 1) || (boundary && (rd->fnum[i] < max)))
							continue;

						Point3 avg = sourceArray[rd->nbor[i][0]];
						for (int j = 1; j < max; j++) {
							avg += sourceArray[rd->nbor[i][j]];
						}

						if (expandArray)
							destArray[i] = (sourceArray[i] * (1.0f - expandArray[i])) + (avg * (expandArray[i] / max));
						else
							destArray[i] = (sourceArray[i] * (1.0f - expand)) + (avg * (expand / max));
					}
				});
		
				// swap arrays
				pTemp = sourceArray;
				sourceArray = destArray;
				destArray = pTemp;
			}
		}

		// Now copy into the Poly
		for (int i = 0; i < rd->vnum; i++)
			polyObj->SetPoint(i, sourceArray[i]);

		// Clean up
		if (hold)
			delete[] hold;
		if (hold2)
			delete[] hold2;
		if (relaxArray)
			delete[] relaxArray;
		if (expandArray)
			delete[] expandArray;
	}

	else
	if(patchObj) {

		DWORD selLevel = pmesh.selLevel;
		// mjm - 4.8.99 - add support for soft selection
		// sca - 4.29.99 - extended soft selection support to cover EDGE and FACE selection levels.
		float *vsw = (selLevel!=PATCH_OBJECT) ? pmesh.GetVSelectionWeights() : NULL;

		if (rd->ivalid.InInterval(t) && (pmesh.numVerts != rd->vnum)) {
			// Shouldn't happen, but does with Loft bug and may with other bugs.
			rd->ivalid.SetEmpty ();
		}

		if (!rd->ivalid.InInterval(t)) {
			int vecBase = pmesh.numVerts;
			rd->SetVNum (pmesh.numVerts + pmesh.numVecs);
			for (int i=0; i<rd->vnum; i++) {
				rd->fnum[i]=1;		// For patches, this means it's not a boundary
				rd->nbor[i].ZeroCount();
			}
			rd->sel.ClearAll ();
			for (int i=0; i<pmesh.numPatches; i++) {
				Patch &p = pmesh.patches[i];
				int vecLimit = p.type * 2;
				for (int j=0; j<p.type; j++) {
					PatchEdge &e = pmesh.edges[p.edge[j]];
					BOOL isBoundary = (e.patches.Count() < 2) ? TRUE : FALSE;
					int theVert = p.v[j];
					int nextVert = p.v[(j+1)%p.type];
					int nextVec = p.vec[j*2] + vecBase;
					int nextVec2 = p.vec[j*2+1] + vecBase;
					int prevEdge = (j+p.type-1)%p.type;
					int prevVec = p.vec[prevEdge*2+1] + vecBase;
					int prevVec2 = p.vec[prevEdge*2] + vecBase;
					int theInterior = p.interior[j] + vecBase;
					// Establish selection bits
					if ((selLevel==PATCH_PATCH) && pmesh.patchSel[i]) {
						rd->sel.Set(theVert);
						rd->sel.Set(nextVec);
						rd->sel.Set(prevVec);
						rd->sel.Set(theInterior);
						}
					else
					if ((selLevel==PATCH_EDGE) && pmesh.edgeSel[p.edge[j]]) {
						rd->sel.Set(e.v1);
						rd->sel.Set(e.vec12 + vecBase);
						rd->sel.Set(e.vec21 + vecBase);
						rd->sel.Set(e.v2);
						}
					else
					if ((selLevel==PATCH_VERTEX) && pmesh.vertSel[theVert]) {
						rd->sel.Set(theVert);
						rd->sel.Set(nextVec);
						rd->sel.Set(prevVec);
						rd->sel.Set(theInterior);
						}

					// Set boundary flags if necessary
					if(isBoundary) {
						rd->fnum[theVert] = 0;
						rd->fnum[nextVec] = 0;
						rd->fnum[nextVec2] = 0;
						rd->fnum[nextVert] = 0;
						}

					// First process the verts
					int work = theVert;
					int max = rd->nbor[work].Count();
					// Append the neighboring vectors
					rd->MaybeAppendNeighbor(work, nextVec, max);
					rd->MaybeAppendNeighbor(work, prevVec, max);
					rd->MaybeAppendNeighbor(work, theInterior, max);

					// Now process the edge vectors
					work = nextVec;
					max = rd->nbor[work].Count();
					// Append the neighboring points
					rd->MaybeAppendNeighbor(work, theVert, max);
					rd->MaybeAppendNeighbor(work, theInterior, max);
					rd->MaybeAppendNeighbor(work, prevVec, max);
					rd->MaybeAppendNeighbor(work, nextVec2, max);
					rd->MaybeAppendNeighbor(work, p.interior[(j+1)%p.type] + vecBase, max);

					work = prevVec;
					max = rd->nbor[work].Count();
					// Append the neighboring points
					rd->MaybeAppendNeighbor(work, theVert, max);
					rd->MaybeAppendNeighbor(work, theInterior, max);
					rd->MaybeAppendNeighbor(work, nextVec, max);
					rd->MaybeAppendNeighbor(work, prevVec2, max);
					rd->MaybeAppendNeighbor(work, p.interior[(j+p.type-1)%p.type] + vecBase, max);

					// Now append the interior, if not auto
					if(!p.IsAuto()) {
						work = theInterior;
						max = rd->nbor[work].Count();
						// Append the neighboring points
						rd->MaybeAppendNeighbor(work, p.v[j], max);
						rd->MaybeAppendNeighbor(work, nextVec, max);
						rd->MaybeAppendNeighbor(work, nextVec2, max);
						rd->MaybeAppendNeighbor(work, prevVec, max);
						rd->MaybeAppendNeighbor(work, prevVec2, max);
						for(int k = 1; k < p.type; ++k)
							rd->MaybeAppendNeighbor(work, p.interior[(j+k)%p.type] + vecBase, max);
						}
					}
				}
	// mjm - begin - 4.8.99
			if (selLevel==PATCH_VERTEX) {
				for (int i=0; i<pmesh.numVerts; ++i) {
					if (pmesh.vertSel[i]) rd->sel.Set(i);
				}
			}
			else if (selLevel==PATCH_OBJECT) rd->sel.SetAll();
	// mjm - end
			rd->ivalid  = os->obj->ChannelValidity (t, TOPO_CHAN_NUM);
			rd->ivalid &= os->obj->ChannelValidity (t, SUBSEL_TYPE_CHAN_NUM);
			rd->ivalid &= os->obj->ChannelValidity (t, SELECT_CHAN_NUM);
		}

		// Keep an array of weighted values for fast referal during iterations
		float *relaxArray = nullptr;
		float *expandArray = nullptr;
		if (vsw) {
			relaxArray = new float[rd->vnum];
			for (int i = 0; i < rd->vnum; i++)
			{
				relaxArray[i] = (!rd->sel[i]) ? relax * vsw[i] : relax;
			}

			if (preserve) {
				expandArray = new float[rd->vnum];
				for (int i = 0; i < rd->vnum; i++)
				{
					expandArray[i] = (!rd->sel[i]) ? expand * vsw[i] : expand;
				}
			}
		}

		Tab<float> vangles;
		if (saddle)
			vangles.SetCount (rd->vnum);

		Point3 *hold = new Point3[rd->vnum];
		// Add additional cache to speed up iterations
		Point3 *hold2 = new Point3[rd->vnum];

		// Cache the Vertex values
		for (int i = 0; i < rd->vnum; i++)
			hold[i] = patchObj->GetPoint(i);

		memcpy_s(hold2, rd->vnum * sizeof(Point3), hold, rd->vnum * sizeof(Point3));

		// Reference pointers so we can toggle between the two
		Point3	*sourceArray = hold;
		Point3	*destArray = hold2;

		for (int k=0; k<iter; k++) {
			if (saddle) {
				if (k != 0) {
					for (int i = 0; i < rd->vnum; i++)
					{
						patchObj->SetPoint(i, sourceArray[i]);
					}
				}
				FindVertexAngles(pmesh, vangles.Addr(0));
			}

			tbb::parallel_for(tbb::blocked_range<int>(0, rd->vnum), [&](const auto& range)
			{
				for (int i = range.begin(); i < range.end(); i++)
				{

					if ((!rd->sel[i]) && (!vsw || vsw[i] == 0))
						continue;

					if (saddle && (i < pmesh.numVerts) && (vangles[i] <= 2 * PI * .99999f))
						continue;

					int max = rd->nbor[i].Count();
					if ((max < 1) || (boundary && !rd->fnum[i]))
						continue;

					Point3 avg = sourceArray[rd->nbor[i][0]];
					for (int j = 1; j < max; j++)
					{
						avg += sourceArray[rd->nbor[i][j]];
					}

					if (relaxArray)
						relax = relaxArray[i];

					destArray[i] = sourceArray[i] * (1.0f - relax) + (avg * (relax / max));
				}
			});

			// Swap Arrays
			Point3	*pTemp = sourceArray;
			sourceArray = destArray;
			destArray = pTemp;

			// Now expand for volume preservation
			if (preserve)
			{
				tbb::parallel_for(tbb::blocked_range<int>(0, rd->vnum), [&](const auto& range)
				{
					for (int i = range.begin(); i < range.end(); i++)
					{
						//for (int i = 0; i < rd->vnum; i++) {

						if ((!rd->sel[i]) && (!vsw || vsw[i] == 0))
							continue;
						// mjm - end
						if (saddle && (i < pmesh.numVerts) && (vangles[i] <= 2 * PI*.99999f))
							continue;

						int max = rd->nbor[i].Count();
						if ((max < 1) || (boundary && !rd->fnum[i]))
							continue;

						Point3 avg = sourceArray[rd->nbor[i][0]];
						for (int j = 1; j < max; j++) {
							avg += sourceArray[rd->nbor[i][j]];
						}

						if (expandArray)
							expand = expandArray[i];

						destArray[i] = (sourceArray[i] * (1.0f - expand)) + (avg * (expand / max));
					}
				});

				// swap arrays
				pTemp = sourceArray;
				sourceArray = destArray;
				destArray = pTemp;
			}
		}

		// Copy back
		for (int i = 0; i < rd->vnum; i++)
		{
			patchObj->SetPoint(i, sourceArray[i]);
		}

		// Clean up
		if (hold)
			delete[] hold;
		if (hold2)
			delete[] hold2;
		if (relaxArray)
			delete[] relaxArray;

		patchObj->patch.computeInteriors();
		patchObj->patch.ApplyConstraints();
	}
	
	if(!converted) {
		os->obj->SetChannelValidity(GEOM_CHAN_NUM, valid);
	} else {
		// Stuff converted object into the pipeline!
		triObj->SetChannelValidity(TOPO_CHAN_NUM, valid);
		triObj->SetChannelValidity(GEOM_CHAN_NUM, valid);
		triObj->SetChannelValidity(TEXMAP_CHAN_NUM, valid);
		triObj->SetChannelValidity(MTL_CHAN_NUM, valid);
		triObj->SetChannelValidity(SELECT_CHAN_NUM, valid);
		triObj->SetChannelValidity(SUBSEL_TYPE_CHAN_NUM, valid);
		triObj->SetChannelValidity(DISP_ATTRIB_CHAN_NUM, valid);

		os->obj = triObj;
	}
}

void RelaxMod::NotifyInputChanged (const Interval& changeInt, PartID part, RefMessage message, ModContext *mc) {
	if (!(part & (PART_TOPO|PART_SELECT|PART_SUBSEL_TYPE))) return;
	RelaxModData *rd = (RelaxModData *) mc->localData;
	if (rd == NULL) return;
	rd->ivalid = NEVER;
}

void RelaxMod::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{
	this->ip = ip;
	editMod = this;

	TimeValue t = ip->GetTime();
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);
	SetAFlag(A_MOD_BEING_EDITED);

	RelaxDesc.BeginEditParams(ip, this, flags, prev);
}

void RelaxMod::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next)
{
	RelaxDesc.EndEditParams(ip, this, flags, next);

	this->ip = NULL;
	editMod = NULL;
	
	TimeValue t = ip->GetTime();

	// aszabo|feb.05.02 This flag must be cleared before sending the REFMSG_END_EDIT
	ClearAFlag(A_MOD_BEING_EDITED);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);
}

Interval RelaxMod::GetValidity(TimeValue t)	{
	int i;
	float f;
	// Only have to worry about animated parameters.
	Interval valid = FOREVER;
	pblock2->GetValue (relax_amount, t, f, valid);
	pblock2->GetValue (relax_iterations, t, i, valid);
	return valid;
}


RefResult RelaxMod::NotifyRefChanged(const Interval& /*changeInt*/,  RefTargetHandle /*hTarget*/, PartID& /*partID*/, 
		RefMessage /*message*/, BOOL /*propagate*/ ) 
{
	return(REF_SUCCEED);
}

#define VERSION_CHUNK	0x1000
// IO
bool RelaxMod::SpecifySaveReferences(ReferenceSaveManager& referenceSaveManager)
{
	// if saving to previous version that used pb1 instead of pb2...
	DWORD saveVersion = GetSavingVersion();
	if (saveVersion != 0 && saveVersion <= MAX_RELEASE_R23)
	{
		ProcessPB2ToPB1SaveToPrevious(this, pblock2, PBLOCK_REF, descVer1, PBLOCK_LENGTH, CURRENT_VERSION);
	}
	return __super::SpecifySaveReferences(referenceSaveManager);
}

IOResult RelaxMod::Save(ISave *isave) {
	ULONG nb;
	Modifier::Save(isave);
	isave->BeginChunk (VERSION_CHUNK);
	isave->Write (&version, sizeof(int), &nb);
	isave->EndChunk();
	return IO_OK;
	}

IOResult RelaxMod::Load(ILoad *iload) {
	Modifier::Load(iload);

	ParamBlock2PLCB* plcb = new ParamBlock2PLCB(versions, NUM_OLDVERSIONS, &relax_param_blk, this, PBLOCK_REF);
	iload->RegisterPostLoadCallback(plcb);

	IOResult res;
	ULONG nb;
	version = RELAXMOD_VER1;	// Set default version to old one
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case VERSION_CHUNK:
				res = iload->Read(&version,sizeof(int),&nb);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
}
