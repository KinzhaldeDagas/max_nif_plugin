/**********************************************************************
 *<
   FILE: surfmod.cpp

   DESCRIPTION:  Varius surface modifiers

   CREATED BY: Rolf Berteig

   HISTORY: 11/07/95

 *>   Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#include "mods.h"
#include "iparamm2.h"
#include "resourceOverride.h"
#include "MeshNormalSpec.h"
#include "MNNormalSpec.h"
#include "splshape.h"

// Version info added for MAXr4, where we can now be applied to a PatchMesh and retain identity!
#define MATMOD_VER1 1
#define MATMOD_VER4 4
#define MATMOD_VER5 5 // Adds native Spline support for Material Modifier

#define MATMOD_CURRENT_VERSION MATMOD_VER5

namespace
{
	template<typename POLY_OBJ_TYPE> void ClearSpecifiedNormals(POLY_OBJ_TYPE* poly)
	{
		if (poly)
		{
			auto& mesh = poly->GetMesh();
			mesh.ClearSpecifiedNormals();
		}
	}

    // Clears the selected normal faces from mesh mesh's specified normals container, if it is defined.
    template <typename MeshType>
    void ClearSpecifiedNormals(MeshType& mesh, const IntTab& selectedFaces)
    {
        using NormalSpecTypePtr = decltype(mesh.GetSpecifiedNormals());

        // If a specified normals container is defined, clear the entries for the selected faces now, indicating that the
        // default normal calculation based upon smoothing groups should be employed 
        NormalSpecTypePtr normals = mesh.GetSpecifiedNormals();
        if (normals != nullptr)
        {
            using NormalFaceTypePtr = decltype(normals->GetFaceArray());

            int nNormalFaces = normals->GetNumFaces();
            int nFacesSelected = selectedFaces.Count();
            NormalFaceTypePtr normalFaces = normals->GetFaceArray();
            for (int f = 0; f != nFacesSelected; ++f)
            {
                DbgAssert(selectedFaces[f] >= 0);
                if (selectedFaces[f] < nNormalFaces)
                {
                    (normalFaces[selectedFaces[f]]).Clear();
                }
            }
        }
    }

    // Recomputes the non-explicit normals stored in mesh mesh's specified normals container, if one is defined. Specifically,
    // mesh is set as its specified normals container's parent, the flags given by flagsToClear are cleared on the container,
    // CheckNormals is called, and the flags given by flagsToSet are set on the container.
    template <typename MeshType>
    void RecomputeNormals(MeshType& mesh, DWORD flagsToClear, DWORD flagsToSet = 0)
    {
        using NormalSpecTypePtr = decltype(mesh.GetSpecifiedNormals());

        NormalSpecTypePtr normals = mesh.GetSpecifiedNormals();
        if (normals != nullptr)
        {
            normals->SetParent(&mesh);
            normals->ClearFlag(flagsToClear);
            normals->CheckNormals();
            normals->SetFlag(flagsToSet);
        }
    }
}

class MatMod : public Modifier { 
   public:
      IParamBlock *pblock;
      static IParamMap *pmapParam;
      int version;

      MatMod();

      // From Animatable
      void DeleteThis() { delete this; }
      void GetClassName(MSTR& s, bool localized) const override { s= localized ? GetString(IDS_RB_MATMOD) : _T("matMod"); }  
      virtual Class_ID ClassID() { return Class_ID(MATERIALOSM_CLASS_ID,0);}
      void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
      void EndEditParams( IObjParam *ip,ULONG flags,Animatable *next);     
      const TCHAR *GetObjectName(bool localized) const override { return localized ? GetString(IDS_RB_MATERIAL3) : _T("Material"); }
      CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;} 

      ChannelMask ChannelsUsed()  {return OBJ_CHANNELS;}
      ChannelMask ChannelsChanged() {return GEOM_CHANNEL|TOPO_CHANNEL;}
      Class_ID InputType() {return defObjectClassID;}
      void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
      Interval LocalValidity(TimeValue t);
      IParamArray *GetParamBlock() {return pblock;}
      int GetParamBlockIndex(int id) {return id;}

      int NumRefs() {return 1;}
      RefTargetHandle GetReference(int i) {return pblock;}
private:
      virtual void SetReference(int i, RefTargetHandle rtarg) {pblock = (IParamBlock*)rtarg;}
public:

      int NumSubs() {return 1;}
      Animatable* SubAnim(int i) {return pblock;}
      TSTR SubAnimName(int i, bool localized) override {return localized ? GetString(IDS_RB_PARAMETERS) : _T("Parameters");}

      RefTargetHandle Clone(RemapDir& remap);
      RefResult NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate);
      // IO
      IOResult Save(ISave *isave);
      IOResult Load(ILoad *iload);
   };



//--- ClassDescriptor and class vars ---------------------------------

IParamMap *MatMod::pmapParam = NULL;



class MatClassDesc:public ClassDesc {
   public:
   int         IsPublic() { return 1; }
   void*         Create(BOOL loading = FALSE) { return new MatMod; }
   const TCHAR*  ClassName() { return GetString(IDS_RB_MATERIAL3_CLASS); }
   const TCHAR*  NonLocalizedClassName() { return _T("Material"); }
   SClass_ID      SuperClassID() { return OSM_CLASS_ID; }
   Class_ID    ClassID() { return Class_ID(MATERIALOSM_CLASS_ID,0); }
   const TCHAR*   Category() { return GetString(IDS_RB_DEFSURFACE);}
   };

static MatClassDesc matDesc;
extern ClassDesc* GetMatModDesc() { return &matDesc; }


//--- Parameter map/block descriptors -------------------------------

#define PB_MATID 0

//
//
// Parameters

static ParamUIDesc descParam[] = {
   
   // Material ID
   ParamUIDesc(
      PB_MATID,
      EDITTYPE_INT,
      IDC_MATID,IDC_MATIDSPIN,
      1.0f,(float)0xffff,
      0.1f),   
   };
#define PARAMDESC_LENGH 1


static ParamBlockDescID descVer0[] = {
   { TYPE_INT, NULL, TRUE, 0 }};
#define PBLOCK_LENGTH   1

#define CURRENT_VERSION 0


//--- MatMod methods -------------------------------


MatMod::MatMod()
   {  
   pblock = NULL;
   ReplaceReference(0, CreateParameterBlock(descVer0, PBLOCK_LENGTH, CURRENT_VERSION));  
   pblock->SetValue(PB_MATID,0,1);
   version = MATMOD_CURRENT_VERSION;
   }

void MatMod::BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev)
   {
   pmapParam = CreateCPParamMap(
      descParam,PARAMDESC_LENGH,
      pblock,
      ip,
      hInstance,
      MAKEINTRESOURCE(IDD_MATERIALPARAM),
      GetString(IDS_RB_PARAMETERS),
      0);      
   }
      
void MatMod::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
   {  
   DestroyCPParamMap(pmapParam);
   pmapParam = NULL;
   }

Interval MatMod::LocalValidity(TimeValue t)
   {
   int i;
   Interval valid = FOREVER;
   pblock->GetValue(PB_MATID,t,i,valid);  
   return valid;
   }

RefTargetHandle MatMod::Clone(RemapDir& remap) 
   {
   MatMod* newmod = new MatMod();   
   newmod->ReplaceReference(0,remap.CloneRef(pblock));   
   newmod->version = version;
   BaseClone(this, newmod, remap);
   return newmod;
   }

static void DoMaterialSet(TriObject *triOb, int id) {
   BOOL useSel = triOb->GetMesh().selLevel == MESH_FACE;

   const BitArray& faceSel = triOb->GetMesh().FaceSel();
   for (int i=0; i<triOb->GetMesh().getNumFaces(); i++) {
      if (!useSel || faceSel[i]) {
         triOb->GetMesh().setFaceMtlIndex(i,(MtlID)id);
         }
      }
   triOb->GetMesh().InvalidateTopologyCache();
   }

void MatMod::ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node)
   {
   Interval valid = FOREVER;
   int id;
   pblock->GetValue(PB_MATID,t,id,valid); 
   id--;
   if (id<0) id = 0;
   if (id>0xffff) id = 0xffff;

   // For version 4 and later, we process patch meshes as they are and pass them on.  Earlier
   // versions converted to TriMeshes (done below).  For adding other new types of objects, add
   // them here!
   if(version >= MATMOD_VER4 && os->obj->IsSubClassOf(patchObjectClassID)) {
      PatchObject *patchOb = (PatchObject *)os->obj;
      PatchMesh &pmesh = patchOb->GetPatchMesh(t);
      BOOL useSel = pmesh.selLevel >= PO_PATCH;

      for (int i=0; i<pmesh.getNumPatches(); i++) {
         if (!useSel || pmesh.patchSel[i]) {
            pmesh.setPatchMtlIndex(i,(MtlID)id);
            }
         }
      pmesh.InvalidateGeomCache();  // Do this because there isn't a topo cache in PatchMesh
                  
      patchOb->UpdateValidity(TOPO_CHAN_NUM,valid);      
      }
   // For version 5 and later, we handle splines (and objects that can convert to splines) natively
   else if (version >= MATMOD_VER5 && (os->obj->CanConvertToType(splineShapeClassID)))
	  {
       	  bool del = false;
		  SplineShape* shape = (SplineShape*)os->obj->ConvertToType(t, splineShapeClassID);
		  if (shape)
		  {
              // If we converted, stuff back to the object
			  if (shape != os->obj)
				  os->obj = shape;

              bool bSel = shape->GetSelLevel() == SS_SEGMENT;

              int iNumPolys = shape->shape.splineCount;
              
              for (int iPoly = 0; iPoly < iNumPolys; iPoly++)
			  {
				  Spline3D* thisSpline = shape->shape.splines[iPoly];
				  BitArray& ssel = shape->shape.segSel[iPoly];

				  int iNumSegs = thisSpline->Segments();
				  for (int iSeg = 0; iSeg < iNumSegs; iSeg++)
				  {
					  if (!bSel || ssel[iSeg])
					  {
						  thisSpline->SetMatID(iSeg, (MtlID)id);
					  }
				  }
			  }

              shape->InvalidateGeomCache();
			  shape->UpdateValidity(TOPO_CHAN_NUM, valid);
		  }
	  }
	  else
   // Process PolyObjects
   if(os->obj->IsSubClassOf(polyObjectClassID)) {
      PolyObject *polyOb = (PolyObject *)os->obj;
      MNMesh &mesh = polyOb->GetMesh();
      BOOL useSel = mesh.selLevel == MNM_SL_FACE;

      for (int i=0; i<mesh.numf; i++) {
         if (!useSel || mesh.f[i].GetFlag(MN_SEL)) {
            mesh.f[i].material = (MtlID)id;
         }
      }
      polyOb->UpdateValidity(TOPO_CHAN_NUM,valid);    
   }
   else  // If it's a TriObject, process it
   if(os->obj->IsSubClassOf(triObjectClassID)) {
      TriObject *triOb = (TriObject *)os->obj;
      DoMaterialSet(triOb, id);
	  triOb->UpdateValidity(TOPO_CHAN_NUM,valid);   

      // If we have specified normals mark them as supported, to prevent them from being removed
	  MeshNormalSpec* pNormals = triOb->GetMesh().GetSpecifiedNormals();
	  if (pNormals && pNormals->GetNumFaces())
	  {
		  // Here we do any rebuilding or recomputing that's necessary:
		  pNormals->SetParent(&(triOb->GetMesh()));
		  pNormals->CheckNormals();
		  // And here we inform the system that this modifier has supported the specified normals.
		  pNormals->SetFlag(MESH_NORMAL_MODIFIER_SUPPORT);
	  }
   }
   else  // Fallback position: If it can convert to a TriObject, do it!
   if(os->obj->CanConvertToType(triObjectClassID)) {
      TriObject  *triOb = (TriObject *)os->obj->ConvertToType(t, triObjectClassID);
      // Now stuff this into the pipeline!
      os->obj = triOb;

      DoMaterialSet(triOb, id);
      triOb->UpdateValidity(TOPO_CHAN_NUM,valid);     
      }
   else
      return;     // Do nothing if it can't convert to triObject
   }

RefResult MatMod::NotifyRefChanged(
      const Interval& changeInt,
      RefTargetHandle hTarget,
      PartID& partID,
      RefMessage message,
      BOOL propagate)
      {
   switch (message) {
      case REFMSG_CHANGE:
         if (pmapParam && pmapParam->GetParamBlock()==pblock) {
            pmapParam->Invalidate();
            }
         break;

      case REFMSG_GET_PARAM_DIM: {
         GetParamDim *gpd = (GetParamDim*)partID;
         switch (gpd->index) {
            case 0:
            default: gpd->dim = defaultDim; break;
            }
         return REF_HALT;
         }

      case REFMSG_GET_PARAM_NAME_LOCALIZED: {
         GetParamName *gpn = (GetParamName*)partID;
         switch (gpn->index) {
            case PB_MATID: gpn->name = GetString(IDS_RB_MATERIALID); break;
            default:       gpn->name = TSTR(_T(""));                 break;
            }
         return REF_HALT;
         }

      case REFMSG_GET_PARAM_NAME_NONLOCALIZED: {
         GetParamName *gpn = (GetParamName*)partID;
         switch (gpn->index) {
            case PB_MATID: gpn->name = _T("Material ID"); break;
            default:       gpn->name = TSTR(_T(""));      break;
            }
         return REF_HALT;
         }
      }
   return REF_SUCCEED;
   }

#define MATMOD_VERSION_CHUNK  0x1000

IOResult MatMod::Save(ISave *isave) {
   ULONG nb;
   Modifier::Save(isave);
   isave->BeginChunk (MATMOD_VERSION_CHUNK);
   isave->Write (&version, sizeof(int), &nb);
   isave->EndChunk();
   return IO_OK;
   }

IOResult MatMod::Load(ILoad *iload) {
   Modifier::Load(iload);
   IOResult res;
   ULONG nb;
   version = MATMOD_VER1;  // Set default version to old one
   while (IO_OK==(res=iload->OpenChunk())) {
      switch(iload->CurChunkID())  {
         case MATMOD_VERSION_CHUNK:
            res = iload->Read(&version,sizeof(int),&nb);
            break;
         }
      iload->CloseChunk();
      if (res!=IO_OK) 
         return res;
      }
   return IO_OK;
   }

//-------------------------------------------------------------------
//-------------------------------------------------------------------

// Version info added for MAXr4, where we can now be applied to a PatchMesh and retain identity!
#define SMOOTHMOD_VER1 1
#define SMOOTHMOD_VER4 4

#define SMOOTHMOD_CURRENT_VERSION SMOOTHMOD_VER4

// References in SmoothMod:
enum { REF_SMOOTH_PBLOCK };

class SmoothMod : public Modifier { 
public:
   IParamBlock2 *pblock;
   //static IParamMap *pmapParam;
   int version;

   SmoothMod();

   // From Animatable
   void DeleteThis() { delete this; }
   void GetClassName(MSTR& s, bool localized) const override { s= localized ? GetString(IDS_RB_SMOOTHMOD) : _T("SmoothMod"); }  
   virtual Class_ID ClassID() { return Class_ID(SMOOTHOSM_CLASS_ID,0);}
   void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
   void EndEditParams( IObjParam *ip,ULONG flags,Animatable *next);     
   const TCHAR *GetObjectName(bool localized) const override { return localized ? GetString(IDS_RB_SMOOTH2) : _T("Smooth"); }
   CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;} 

   ChannelMask ChannelsUsed()  {return OBJ_CHANNELS;}
   ChannelMask ChannelsChanged() {return GEOM_CHANNEL|TOPO_CHANNEL;}
   Class_ID InputType() {return defObjectClassID;}
   void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
   Interval LocalValidity(TimeValue t);

   int NumParamBlocks () { return 1; }
   IParamBlock2 *GetParamBlock(int i) {return pblock;}
   IParamBlock2 *GetParamBlockByID (short id) { return (pblock->ID() == id) ? pblock : NULL; }
   //int GetParamBlockIndex(int id) {return id;}

   int NumRefs() {return 1;}
   RefTargetHandle GetReference(int i) {return pblock;}
private:
   virtual void SetReference(int i, RefTargetHandle rtarg) {pblock = (IParamBlock2*)rtarg;}
public:
   IOResult Load(ILoad *iload);

   int NumSubs() {return 1;}
   Animatable* SubAnim(int i) {return pblock;}
   TSTR SubAnimName(int i, bool localized) override {return localized ? GetString(IDS_RB_PARAMETERS) : _T("Parameters");}

   RefTargetHandle Clone(RemapDir& remap);
   RefResult NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate);
   // IO
   IOResult Save(ISave *isave);
};



//--- ClassDescriptor and class vars ---------------------------------
class SmoothClassDesc : public ClassDesc2 {
public:
   int         IsPublic() { return 1; }
   void*         Create(BOOL loading = FALSE) {return new SmoothMod;}
   const TCHAR*  ClassName() { return GetString(IDS_RB_SMOOTH2_CLASS); }
   const TCHAR*  NonLocalizedClassName() { return _T("Smooth"); }
   SClass_ID      SuperClassID() { return OSM_CLASS_ID; }
   Class_ID    ClassID() { return Class_ID(SMOOTHOSM_CLASS_ID,0); }
   const TCHAR*   Category() { return GetString(IDS_RB_DEFSURFACE);}
   const TCHAR*   InternalName()          { return _T("SmoothModifier"); } // for scripter.
   HINSTANCE      HInstance()             { return hInstance; }
};

static SmoothClassDesc smoothDesc;
extern ClassDesc* GetSmoothModDesc() { return &smoothDesc; }


//--- Parameter map/block descriptors -------------------------------

// Enumerate parameter blocks:
enum { smooth_params };
// Parameters in the block:
enum { sm_autosmooth, sm_threshold, sm_smoothbits, sm_prevent_indirect };

static ParamBlockDesc2 smooth_param_desc (smooth_params, _T("smoothParams"),
                           IDS_RB_PARAMETERS, &smoothDesc,
                           P_AUTO_CONSTRUCT | P_AUTO_UI, REF_SMOOTH_PBLOCK,
   // Rollout description:
   IDD_SMOOTHPARAM, IDS_RB_PARAMETERS, 0, 0, NULL,

   // params
   sm_autosmooth, _T("autoSmooth"), TYPE_BOOL, P_RESET_DEFAULT, IDS_RB_AUTOSMOOTH,
      p_default, false,
      p_enable_ctrls, 2, sm_threshold, sm_prevent_indirect,
      p_ui, TYPE_SINGLECHECKBOX, IDC_SMOOTH_AUTO,
      p_end,

   sm_prevent_indirect, _T("preventIndirect"), TYPE_BOOL, P_RESET_DEFAULT, IDS_PREVENT_INDIRECT,
      p_default, false,
      p_ui, TYPE_SINGLECHECKBOX, IDC_SMOOTH_PREVENTINDIRECT,
      p_end,

   sm_threshold, _T("threshold"), TYPE_ANGLE, P_RESET_DEFAULT|P_ANIMATABLE, IDS_RB_THRESHOLD,
      p_default, PI/6.0f,
      p_range, 0.0f, 180.0f,
      p_ui, TYPE_SPINNER, EDITTYPE_POS_FLOAT,
         IDC_SMOOTH_THRESH, IDC_SMOOTH_THRESHSPIN, .1f,
      p_end,

   // NOTE That this should be TYPE_DWORD, but that type isn't yet supported in paramblocks.
   sm_smoothbits, _T("smoothingBits"), TYPE_INT, P_RESET_DEFAULT, IDS_VS_SMGROUP,
      p_default, 0,
      // No UI - we handle the UI ourselves.
      p_end,
   p_end
);

//--- SmoothDlgProc --------------------------------

class SmoothDlgProc : public ParamMap2UserDlgProc {
   SmoothMod *mod;
   bool uiValid;
public:
   SmoothDlgProc () : mod(NULL), uiValid(false) { }
   INT_PTR DlgProc (TimeValue t, IParamMap2 *map, HWND hWnd,
      UINT msg, WPARAM wParam, LPARAM lParam);
   void DeleteThis() { }
   void Update (HWND hWnd, TimeValue t);
   void SetMod (SmoothMod *m) { mod=m; }
   void Invalidate (HWND hWnd) { uiValid = false; InvalidateRect (hWnd, NULL, false); }
};

static SmoothDlgProc theSmoothDlgProc;

// NOTE: This depends on the IDC_SMOOTH indices being sequential!
void SmoothDlgProc::Update (HWND hWnd, TimeValue t) {
   if (!mod || !mod->pblock || !hWnd) return;

   int autoSmooth;
   mod->pblock->GetValue (sm_autosmooth, t, autoSmooth, FOREVER);

   int bits;
   mod->pblock->GetValue (sm_smoothbits, t, bits, FOREVER);
   for (int i=IDC_SMOOTH_GRP1; i<IDC_SMOOTH_GRP1+32; i++) {
      ICustButton *iBut = GetICustButton(GetDlgItem(hWnd,i));
      iBut->SetCheck ((bits & (1<<(i-IDC_SMOOTH_GRP1))) ? true : false);
      iBut->Enable (!autoSmooth);
      ReleaseICustButton (iBut);
   }

   EnableWindow (GetDlgItem (hWnd, IDC_SMOOTH_GROUP_BOX), !autoSmooth);
   EnableWindow (GetDlgItem (hWnd, IDC_SMOOTH_THRESH_LABEL), autoSmooth);
   uiValid = true;
}

INT_PTR SmoothDlgProc::DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd,
                     UINT msg, WPARAM wParam, LPARAM lParam) {
   switch (msg) {
   case WM_INITDIALOG:
      {
         for (int i=IDC_SMOOTH_GRP1; i<IDC_SMOOTH_GRP1+32; i++) {
            ICustButton *iBut = GetICustButton(GetDlgItem(hWnd,i));
            iBut->SetType (CBT_CHECK);
            ReleaseICustButton (iBut);
         }
      }
      uiValid = false;
      break;

   case WM_PAINT:
      if (uiValid) break;
      Update (hWnd, t);
      break;

   case WM_COMMAND:
      if (LOWORD(wParam)>=IDC_SMOOTH_GRP1 &&
         LOWORD(wParam)<=IDC_SMOOTH_GRP32) {
         IParamBlock2 *pblock = (IParamBlock2*)map->GetParamBlock();
         ICustButton *iBut = GetICustButton(GetDlgItem(hWnd,LOWORD(wParam)));
         int bits;
         pblock->GetValue (sm_smoothbits, t, bits, FOREVER);
         int shift = LOWORD(wParam) - IDC_SMOOTH_GRP1;
         if (iBut->IsChecked()) bits |= 1<<shift;
         else bits &= ~(1<<shift);
         theHold.Begin ();
         pblock->SetValue (sm_smoothbits, t, bits);
         theHold.Accept (GetString (IDS_VS_SMGROUP));
         ReleaseICustButton(iBut);
         return REDRAW_VIEWS;
      }
      break;
   }
   return FALSE;
}

//--- SmoothMod methods -------------------------------

SmoothMod::SmoothMod() {
   pblock = NULL;
   version = SMOOTHMOD_CURRENT_VERSION;
   smoothDesc.MakeAutoParamBlocks (this);
}

// Old parameter block descriptions:
static ParamBlockDescID descSmoothVer0[] = {
   { TYPE_INT, NULL, FALSE, sm_autosmooth },
   { TYPE_FLOAT, NULL, TRUE, sm_threshold },
   { TYPE_INT, NULL, FALSE, sm_smoothbits }
};

static ParamBlockDescID descSmoothVer1[] = {
   { TYPE_INT,   NULL, FALSE, sm_autosmooth },
   { TYPE_FLOAT, NULL, TRUE, sm_threshold },
   { TYPE_INT,   NULL, FALSE, sm_smoothbits },
   { TYPE_INT,   NULL, FALSE, sm_prevent_indirect },
};

// Array of old versions
static ParamVersionDesc versionsSmooth[] = {
   ParamVersionDesc(descSmoothVer0,3,0),
   ParamVersionDesc(descSmoothVer1,4,1)
};
#define NUM_OLDSMOOTHVERSIONS 2

#define SMOOTHMOD_VERSION_CHUNK  0x1000

IOResult SmoothMod::Load(ILoad *iload) {
   Modifier::Load(iload);
   iload->RegisterPostLoadCallback(
      new ParamBlock2PLCB(versionsSmooth, NUM_OLDSMOOTHVERSIONS, &smooth_param_desc,
                     this, REF_SMOOTH_PBLOCK));
   IOResult res;
   ULONG nb;
   version = SMOOTHMOD_VER1;  // Set default version to old one
   while (IO_OK==(res=iload->OpenChunk())) {
      switch(iload->CurChunkID())  {
      case SMOOTHMOD_VERSION_CHUNK:
         res = iload->Read(&version,sizeof(int),&nb);
         break;
      }
      iload->CloseChunk();
      if (res!=IO_OK) return res;
   }

   return IO_OK;
}

IOResult SmoothMod::Save(ISave *isave) {
   ULONG nb;
   Modifier::Save(isave);
   isave->BeginChunk (SMOOTHMOD_VERSION_CHUNK);
   isave->Write (&version, sizeof(int), &nb);
   isave->EndChunk();
   return IO_OK;
}

void SmoothMod::BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev) {
   TimeValue t = ip->GetTime();
   NotifyDependents(Interval(t,t), PART_ALL, REFMSG_BEGIN_EDIT);
   SetAFlag (A_MOD_BEING_EDITED);

   smoothDesc.BeginEditParams (ip, this, flags, prev);
   theSmoothDlgProc.SetMod (this);
   smoothDesc.SetUserDlgProc (&smooth_param_desc, &theSmoothDlgProc);
}

void SmoothMod::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next) {
   ClearAFlag (A_MOD_BEING_EDITED);
   TimeValue t = ip->GetTime();
   NotifyDependents(Interval(t,t), PART_ALL, REFMSG_END_EDIT);

   smoothDesc.EndEditParams (ip, this, flags, next);
   theSmoothDlgProc.SetMod (NULL);
}

Interval SmoothMod::LocalValidity(TimeValue t) {
  // aszabo|feb.05.02 If we are being edited, return NEVER 
   // to forces a cache to be built after previous modifier.
   if (TestAFlag(A_MOD_BEING_EDITED))
      return NEVER;
   float f;
   Interval valid = FOREVER;
   // Only one animatable parameter:
   pblock->GetValue(sm_threshold,t,f,valid); 
   return valid;
}

RefTargetHandle SmoothMod::Clone(RemapDir& remap) {
   SmoothMod* newmod = new SmoothMod();
   newmod->ReplaceReference (REF_SMOOTH_PBLOCK, remap.CloneRef(pblock));
   newmod->version = version;
   BaseClone(this, newmod, remap);
   return newmod;
}

void SmoothMod::ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node) {
   Interval valid = FOREVER;
   int autoSmooth, bits = 0, prevent = 0;
   float thresh = 0.0f;
   pblock->GetValue(sm_autosmooth, t, autoSmooth, valid);
   if (autoSmooth) {
      pblock->GetValue(sm_threshold, t, thresh, valid);
      pblock->GetValue(sm_prevent_indirect, t, prevent, valid);
   } else {
      pblock->GetValue(sm_smoothbits, t, bits, valid);
   }
   // For version 4 and later, we process patch meshes as they are and pass them on.  Earlier
   // versions converted to TriMeshes (done below).  For adding other new types of objects, add
   // them here!
   bool done = false;
   if(version >= MATMOD_VER4 && os->obj->IsSubClassOf(patchObjectClassID)) {
      PatchObject *patchOb = (PatchObject *)os->obj;
      PatchMesh &pmesh = patchOb->GetPatchMesh(t);
	  BOOL useSel = pmesh.selLevel >= PO_PATCH;

      if (autoSmooth) pmesh.AutoSmooth (thresh, useSel, prevent);
      else {
         for (int i=0; i<pmesh.getNumPatches(); i++) {
            if (!useSel || pmesh.patchSel[i]) pmesh.patches[i].smGroup = (DWORD)bits;
         }
      }
      pmesh.InvalidateGeomCache();  // Do this because there isn't a topo cache in PatchMesh
      patchOb->UpdateValidity(TOPO_CHAN_NUM,valid);
      done = true;
   }

   if (!done && os->obj->IsSubClassOf (polyObjectClassID))
   {
      IntTab selectedFaces;

      // If we have been passed a face selection on the input mesh, reset the vertex normals for the selected faces only to the
      // default treatment; if the entire mesh is selected, or if we are not using a face selection, clear the specified
      // normals container from the mesh
      PolyObject* pPolyOb = static_cast<PolyObject*>(os->obj);
      MNMesh& mesh = pPolyOb->GetMesh();
      BOOL useSel = (mesh.selLevel == MNM_SL_FACE);
      if (useSel)
      {
         for (int f = 0; f != mesh.numf; ++f)
         {
            const MNFace& faceCurr = mesh.f[f];
            bool isDead = faceCurr.GetFlag(MN_DEAD);
            bool isSelected = faceCurr.GetFlag(MN_SEL);
            if (isSelected && (!isDead))
            {
               selectedFaces.Append(1, &f, 10);
            }
         }
      }

      int nFacesSelected = selectedFaces.Count();
      if (useSel && (nFacesSelected != mesh.numf))
      {
         ClearSpecifiedNormals(mesh, selectedFaces);
      }
      else
      {
         ClearSpecifiedNormals(pPolyOb);
      }

	  // Perform smoothing as specified
      if (autoSmooth)
      {
         mesh.AutoSmooth (thresh, useSel, prevent);
      }
      else if (useSel)
      {
         for (int f = 0; f != nFacesSelected; ++f)
         {
            (mesh.f[selectedFaces[f]]).smGroup = (DWORD)bits;
         }
      }
      else
      {
         for (int f = 0; f != mesh.numf; ++f)
         {
            (mesh.f[f]).smGroup = (DWORD)bits;
         }
      }

      // If the mesh has a specified normals container, which wasn't removed above, recompute any normals using the default
      // calculation now
      RecomputeNormals(mesh, MNNORMAL_NORMALS_BUILT | MNNORMAL_NORMALS_COMPUTED);

      pPolyOb->UpdateValidity(TOPO_CHAN_NUM,valid);
      done = true;
   }

   TriObject *triOb = NULL;
   if (!done) {
      if (os->obj->IsSubClassOf(triObjectClassID)) triOb = (TriObject *)os->obj;
      else {
         // Convert to triobject if we can.
         if(os->obj->CanConvertToType(triObjectClassID)) {
            TriObject  *triOb = (TriObject *)os->obj->ConvertToType(t, triObjectClassID);
			
            // We'll need to stuff this back into the pipeline:
            os->obj = triOb;

            // Convert validities:
            Interval objValid = os->obj->ChannelValidity (t, TOPO_CHAN_NUM);
            triOb->SetChannelValidity (TOPO_CHAN_NUM, objValid);
            triOb->SetChannelValidity (GEOM_CHAN_NUM,
               objValid & os->obj->ChannelValidity (t, GEOM_CHAN_NUM));
            triOb->SetChannelValidity (TEXMAP_CHAN_NUM,
               objValid & os->obj->ChannelValidity (t, TEXMAP_CHAN_NUM));
            triOb->SetChannelValidity (VERT_COLOR_CHAN_NUM,
               objValid & os->obj->ChannelValidity (t, VERT_COLOR_CHAN_NUM));
            triOb->SetChannelValidity (DISP_ATTRIB_CHAN_NUM,
               objValid & os->obj->ChannelValidity (t, DISP_ATTRIB_CHAN_NUM));
         }
      }
   }

   if (triOb)   // one way or another, there's a triobject to smooth.
   {
      IntTab selectedFaces;

      // If we have been passed a face selection on the input mesh, reset the vertex normals for the selected faces only to the
      // default treatment; if the entire mesh is selected, or if we are not using a face selection, clear the specified
      // normals container from the mesh
      Mesh& mesh = triOb->GetMesh();
      BOOL useSel = (mesh.selLevel == MESH_FACE);
      if (useSel)
      {
		 const BitArray& faceSel = mesh.FaceSel();
         for (int f = 0; f != mesh.numFaces; ++f)
         {
            if (faceSel[f])
            {
               selectedFaces.Append(1, &f, 10);
            }
         }
      }

      bool doRecompute = false;
      int nFacesSelected = selectedFaces.Count();
      if (useSel && (nFacesSelected != mesh.numFaces))
      {
         ClearSpecifiedNormals(mesh, selectedFaces);
         doRecompute = true;
      }
      else
      {
         ClearSpecifiedNormals(triOb);
      }

      // Perform smoothing as specified
      if (autoSmooth)
      {
         mesh.AutoSmooth (thresh, useSel, prevent);
      }
      else if (useSel)
      {
         for (int f = 0; f != nFacesSelected; ++f)
         {
            (mesh.faces[selectedFaces[f]]).smGroup = (DWORD)bits;
         }
      }
      else
      {
         for (int f = 0; f != mesh.numFaces; ++f)
         {
            (mesh.faces[f]).smGroup = (DWORD)bits;
         }
      }

      // If the mesh has a specified normals container, recompute any normals using the default calculation now, if
      // appropriate; note that since Mesh::ClearSpecifiedNormals, called by the first variant of ClearSpecifiedNormals, does
      // not actually remove the specified normals container from the mesh, we must be careful to recompute only if we are
      // not operating on the entire mesh
      if (doRecompute)
      {
         RecomputeNormals(mesh, MESH_NORMAL_NORMALS_BUILT | MESH_NORMAL_NORMALS_COMPUTED, MESH_NORMAL_MODIFIER_SUPPORT);
      }

      triOb->GetMesh().InvalidateTopologyCache();
      triOb->UpdateValidity(TOPO_CHAN_NUM,valid);
   }
}

RefResult SmoothMod::NotifyRefChanged (const Interval& changeInt, RefTargetHandle hTarget,
                              PartID& partID, RefMessage message, BOOL propagate)
{
    if (hTarget == pblock)
    {
        switch (message)
        {
        case REFMSG_CHANGE:
            {
                ParamID idToUpdate = pblock->LastNotifyParamID();
                smooth_param_desc.InvalidateUI (idToUpdate);
                switch (idToUpdate)
                {
                case -1:
                case sm_smoothbits:
                case sm_autosmooth:
                    if (smoothDesc.NumParamMaps() > 0)
                    {
                        IParamMap2 *pmap = smoothDesc.GetParamMap(0);
                        if (pmap)
                        {
                            HWND hWnd = pmap->GetHWnd();
                            if (hWnd) theSmoothDlgProc.Invalidate (hWnd);
                        }
                    }
                    break;
                }
            }
            break;
        }

        // Ensure that the viewports are redrawn to reflect any changes that have been made
        Interface* coreInterface = GetCOREInterface();
        DbgAssert(coreInterface != nullptr);
        TimeValue timeCurr = coreInterface->GetTime();
        coreInterface->RedrawViews(timeCurr, REDRAW_NORMAL);
    }

    return REF_SUCCEED;
}


//-----------------------------------------------------------
//-----------------------------------------------------------

// Version info added for MAXr4, where we can now be applied to a PatchMesh and retain identity!
#define NORMALMOD_VER1 1
#define NORMALMOD_VER4 4

#define NORMALMOD_CURRENT_VERSION NORMALMOD_VER4

class NormalMod : public Modifier { 
   public:
      IParamBlock *pblock;
      static IParamMap *pmapParam;
      int version;

      NormalMod();

      // From Animatable
      void DeleteThis() { delete this; }
      void GetClassName(MSTR& s, bool localized) const override { s= localized ? GetString(IDS_RB_NORMALMOD) : _T("NormalMod"); }  
      virtual Class_ID ClassID() { return Class_ID(NORMALOSM_CLASS_ID,0);}
      void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
      void EndEditParams( IObjParam *ip,ULONG flags,Animatable *next);     
      const TCHAR *GetObjectName(bool localized) const override { return localized ? GetString(IDS_RB_NORMAL) : _T("Normal"); }
      CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;} 

      ChannelMask ChannelsUsed()  {return OBJ_CHANNELS;}
      ChannelMask ChannelsChanged() {return GEOM_CHANNEL|TOPO_CHANNEL|TEXMAP_CHANNEL|VERTCOLOR_CHANNEL;}
      Class_ID InputType() {return defObjectClassID;}
      void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
      Interval LocalValidity(TimeValue t);
      IParamArray *GetParamBlock() {return pblock;}
      int GetParamBlockIndex(int id) {return id;}

      int NumRefs() {return 1;}
      RefTargetHandle GetReference(int i) {return pblock;}
private:
      virtual void SetReference(int i, RefTargetHandle rtarg) {pblock = (IParamBlock*)rtarg;}
public:

      int NumSubs() {return 1;}
      Animatable* SubAnim(int i) {return pblock;}
      TSTR SubAnimName(int i, bool localized) override {return localized ? GetString(IDS_RB_PARAMETERS) : _T("Parameters");}

      RefTargetHandle Clone(RemapDir& remap);
      RefResult NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate);
      // IO
      IOResult Load(ILoad *iload);
      IOResult Save(ISave *isave);
   };



//--- ClassDescriptor and class vars ---------------------------------

IParamMap *NormalMod::pmapParam = NULL;



class NormalClassDesc:public ClassDesc {
   public:
   int         IsPublic() { return 1; }
   void*         Create(BOOL loading = FALSE) { return new NormalMod;}
   const TCHAR*  ClassName() { return GetString(IDS_RB_NORMAL_CLASS); }
   const TCHAR*  NonLocalizedClassName() { return _T("Normal"); }
   SClass_ID      SuperClassID() { return OSM_CLASS_ID; }
   Class_ID    ClassID() { return Class_ID(NORMALOSM_CLASS_ID,0);}
   const TCHAR*   Category() { return GetString(IDS_RB_DEFSURFACE);}
   };

static NormalClassDesc normalDesc;
extern ClassDesc* GetNormalModDesc() { return &normalDesc; }


//--- Parameter map/block descriptors -------------------------------

#define PB_UNIFY  0
#define PB_FLIP   1

//
//
// Parameters

static ParamUIDesc descNormParam[] = {
   // Unify
   ParamUIDesc(PB_UNIFY,TYPE_SINGLECHECKBOX,IDC_NORM_UNIFY),

   // Flip
   ParamUIDesc(PB_FLIP,TYPE_SINGLECHECKBOX,IDC_NORM_FLIP),
   };
#define NORMPARAMDESC_LENGH 2


static ParamBlockDescID descNormVer0[] = {
   { TYPE_INT, NULL, FALSE, 0 },
   { TYPE_INT, NULL, FALSE, 1 }};
#define NORMPBLOCK_LENGTH  2

#define CURRENT_NORMVERSION   0


//--- NormalMod methods -------------------------------


NormalMod::NormalMod()
   {  
   pblock = NULL;
   ReplaceReference(0, CreateParameterBlock(descNormVer0, NORMPBLOCK_LENGTH, CURRENT_NORMVERSION));  
   pblock->SetValue(PB_FLIP,0,1);
   version = NORMALMOD_CURRENT_VERSION;
   }

#define NORMALMOD_VERSION_CHUNK  0x1000

IOResult NormalMod::Load(ILoad *iload)
   {
   Modifier::Load(iload);
   IOResult res;
   ULONG nb;
   version = NORMALMOD_VER1;  // Set default version to old one
   while (IO_OK==(res=iload->OpenChunk())) {
      switch(iload->CurChunkID())  {
         case NORMALMOD_VERSION_CHUNK:
            res = iload->Read(&version,sizeof(int),&nb);
            break;
         }
      iload->CloseChunk();
      if (res!=IO_OK) 
         return res;
      }
   return IO_OK;
   }

IOResult NormalMod::Save(ISave *isave) {
   ULONG nb;
   Modifier::Save(isave);
   isave->BeginChunk (NORMALMOD_VERSION_CHUNK);
   isave->Write (&version, sizeof(int), &nb);
   isave->EndChunk();
   return IO_OK;
   }

void NormalMod::BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev)
   {
   pmapParam = CreateCPParamMap(
      descNormParam,NORMPARAMDESC_LENGH,
      pblock,
      ip,
      hInstance,
      MAKEINTRESOURCE(IDD_NORMALPARAM),
      GetString(IDS_RB_PARAMETERS),
      0);      
   }
      
void NormalMod::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
   {  
   DestroyCPParamMap(pmapParam);
   pmapParam = NULL;
   }

Interval NormalMod::LocalValidity(TimeValue t)
   {  
   return FOREVER;
   }

RefTargetHandle NormalMod::Clone(RemapDir& remap) 
   {
   NormalMod* newmod = new NormalMod();   
   newmod->ReplaceReference(0,remap.CloneRef(pblock));   
   BaseClone(this, newmod, remap);
   return newmod;
   }

void FlipMeshNormal(Mesh *mesh,DWORD face) {
   mesh->FlipNormal(int(face));
   /*
   DWORD vis  = 0;
   if (mesh->faces[face].flags&EDGE_A) vis |= EDGE_A;
   if (mesh->faces[face].flags&EDGE_B) vis |= EDGE_C;
   if (mesh->faces[face].flags&EDGE_C) vis |= EDGE_B;
   DWORD temp = mesh->faces[face].v[0];
   mesh->faces[face].v[0] = mesh->faces[face].v[1];
   mesh->faces[face].v[1] = temp;            
   mesh->faces[face].flags &= ~EDGE_ALL;
   mesh->faces[face].flags |= vis;
   if (mesh->tvFace) {     
      temp = mesh->tvFace[face].t[0];
      mesh->tvFace[face].t[0] = mesh->tvFace[face].t[1];
      mesh->tvFace[face].t[1] = temp;     
   }
   */
}

static void DoNormalSet(TriObject *triOb, BOOL unify, BOOL flip) {
   BOOL useSel = triOb->GetMesh().selLevel == MESH_FACE;

   if (unify) {
      triOb->GetMesh().UnifyNormals(useSel);
      triOb->GetMesh().InvalidateTopologyCache ();
      }

   if (flip) {
	  const BitArray& faceSel = triOb->GetMesh().FaceSel();
      for (int i=0; i<triOb->GetMesh().getNumFaces(); i++) {
         if (!useSel || faceSel[i])
            FlipMeshNormal(&triOb->GetMesh(),(DWORD)i);
         }
      triOb->GetMesh().InvalidateTopologyCache ();
      }
   }

void NormalMod::ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node)
   {
   Interval valid = FOREVER;
   int flip, unify;
   pblock->GetValue(PB_FLIP,t,flip,valid);   
   pblock->GetValue(PB_UNIFY,t,unify,valid); 

   // For version 4 and later, we process patch meshes as they are and pass them on.  Earlier
   // versions converted to TriMeshes (done below).  For adding other new types of objects, add
   // them here!
   if(version >= MATMOD_VER4 && os->obj->IsSubClassOf(patchObjectClassID)) {
      PatchObject *patchOb = (PatchObject *)os->obj;
      PatchMesh &pmesh = patchOb->GetPatchMesh(t);
      BOOL useSel = pmesh.selLevel >= PO_PATCH;

      if (unify)
         pmesh.UnifyNormals(useSel);

      if (flip)
         pmesh.FlipPatchNormal(useSel ? -1 : -2);
                  
      patchOb->UpdateValidity(TOPO_CHAN_NUM,valid);      
      }
   else  // If it's a TriObject, process it

   if(os->obj->IsSubClassOf(triObjectClassID)) {
      TriObject *triOb = (TriObject *)os->obj;
      DoNormalSet(triOb, unify, flip);
      triOb->UpdateValidity(TOPO_CHAN_NUM,valid);     
      }

   // Process PolyObjects
   // note: Since PolyObjects must always have the normals alligned they do not 
   // need to support unify and they do not allow normal flips on selected faces
   else if(os->obj->IsSubClassOf(polyObjectClassID)) {
      PolyObject *pPolyOb = (PolyObject *)os->obj;
      MNMesh& mesh = pPolyOb->GetMesh();


      if (flip) {
         // flip selected faces only if entire elements are selected
         if (mesh.selLevel == MNM_SL_FACE) {
            // sca 12/8/2000: Use MNMesh flipping code instead of the code that was here.
            mesh.FlipElementNormals (MN_SEL);
         } else {
            // Flip the entire object if selected elements were not flipped
            for (int i=0; i<mesh.FNum(); i++) {
               mesh.f[i].SetFlag (MN_WHATEVER, !mesh.f[i].GetFlag(MN_DEAD));
            }
            mesh.FlipElementNormals (MN_WHATEVER);
         }

         // Luna task 747:
         // We cannot support specified normals here at this time.
		 // NOTE this assumes that both the topo and geo channels are to be freed
		 // this means that the modifier needs to also set the channels changed to
		 // geo and topo otherwise we will be deleting a channel we dont own.
         mesh.ClearSpecifiedNormals ();
      }

	  pPolyOb->UpdateValidity(GEOM_CHAN_NUM,valid);      
      pPolyOb->UpdateValidity(TOPO_CHAN_NUM,valid);      
      }

   else  // Fallback position: If it can convert to a TriObject, do it!
   if(os->obj->CanConvertToType(triObjectClassID)) {
      TriObject  *triOb = (TriObject *)os->obj->ConvertToType(t, triObjectClassID);
      // Now stuff this into the pipeline!
      os->obj = triOb;

      DoNormalSet(triOb, unify, flip);
      triOb->UpdateValidity(TOPO_CHAN_NUM,valid);     
      
      }
   else
      return;     // Do nothing if it can't convert to triObject
   }

RefResult NormalMod::NotifyRefChanged(
      const Interval& changeInt, 
      RefTargetHandle hTarget, 
         PartID& partID, 
		 RefMessage message, 
		 BOOL propagate) 
      {  
   switch (message) {
      case REFMSG_CHANGE:
         if (pmapParam && pmapParam->GetParamBlock()==pblock) {
            pmapParam->Invalidate();
            }
         break;
      }
   return REF_SUCCEED;
   }
