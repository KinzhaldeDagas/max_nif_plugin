/**********************************************************************
 *<
	FILE: CameraMapTexture.cpp

	DESCRIPTION:	Appwizard generated plugin

	CREATED BY: Peter Watje

	HISTORY: April 20,2004 created

This is a camera map per pixel texture.   It takes an incoming channel and replaces it
with a camera projection.

 
2019-09-13 ADDED: Allow different aspect ratio for projection versus render..   

 
3) animation toggle.  support bitmap sequence as a projection.
  
5) semi accurate viewport display
 
6) some sort of baking utility.  


 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#include "CameraMapTexture.h"
#include "CameraMapRollup.h"
#include <IMtlRender_Compatibility.h>
#include <stdmat.h>
#include <bitmap.h>

#include <notify.h>
#include <Geom/trig.h>


using namespace MaxSDK::Graphics;

#define CAMERAMAPTEXTURE_CLASS_ID	Class_ID(0x60ec7a41, 0xa20dc7e)

#define SCANLINERENDERER_CLASS_ID	Class_ID(SREND_CLASS_ID,0)

#define NSUBTEX		2 // TODO: number of sub-textures supported by this plugin 
					  // 0 is the texture
					  // 1 is the mask

#define PBLOCK_REF	0

enum { cameramaptexture_params };

//TODO: Add enums for various parameters
enum { 
	pb_mapid,		//this is the map channel id
	pb_node,		//this is the node that is the camera, if the node is not a camera the z axis is used for the projection direction with a 45deg fov
	pb_texture,		//this is the texture that will use the camera projection
	pb_backface,	//this is toggle that turns on whether back facing vertices have the projection

	pb_usezbuffer, pb_zbuffer,pb_zfudge,   // this toggles on the ZBuffer mask
										  //  for best results render this without antialiasing
	pb_angle,		//the angle used to determine if a pixel is back facing
	pb_usemask,pb_mask, //this is just a standard mask used to mask the texture
	pb_animated,		//no longer used, but needs to stay here for older file formats
	pb_maskuseprojection, //this forces the mask to also use the camera projection otherwise it will use it original UVWs
	pb_affectbehindcam, //affect points behind the camera

    pb_aspectMode,
    pb_haspect,
    pb_vaspect,
};


class CameraMapTexture : public Texmap, 
	public MaxSDK::Graphics::IParameterTranslator{
	public:

		// Parameter block
		IParamBlock2	*pblock;	//ref 0

		Interval		ivalid;

		//From MtlBase
		ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) override;
		BOOL SetDlgThing(ParamDlg* dlg) override;
		void Update(TimeValue t, Interval& valid) override;
		void Reset() override;
		Interval Validity(TimeValue t) override;
		ULONG LocalRequirements(int subMtlNum) override
		{
			if ((subMtlNum==0) && (tex)) return tex->LocalRequirements(subMtlNum);
			else if ((subMtlNum==1) && (mask)) return mask->LocalRequirements(subMtlNum);
			return 0;
		}

		//TODO: Return the number of sub-textures
		int NumSubTexmaps() override { return NSUBTEX; }
		//TODO: Return the pointer to the 'i-th' sub-texmap
		Texmap* GetSubTexmap(int i) override 
		{ 
			Texmap *sm1 = nullptr;
			Interval iv;
			if (i==0)
			{
				pblock->GetValue(pb_texture,0,sm1,iv);
				return sm1; 
			}
			else
			{
				pblock->GetValue(pb_mask,0,sm1,iv);
				return sm1; 

			}
		}

		void SetSubTexmap(int i, Texmap* m) override
		{
			if (i == 0)
				pblock->SetValue(pb_texture, 0, m);
			else if (i == 1)
				pblock->SetValue(pb_mask, 0, m);
		}
		
		TSTR GetSubTexmapSlotName(int i, bool localized) override
		{
			if (i == 0) return localized ? GetString(IDS_TEXTURE) : _T("Texture");
			else        return localized ? GetString(IDS_MASK)    : _T("Mask");
		}

		//From Texmap
		RGBA EvalColor(ShadeContext& sc) override;
		float EvalMono(ShadeContext& sc) override;
		Point3 EvalNormalPerturb(ShadeContext& sc) override;

		//TODO: Returns TRUE if this texture can be used in the interactive renderer
		BOOL SupportTexDisplay() override { return FALSE; }
		void ActivateTexDisplay(BOOL onoff) override;
		DWORD_PTR GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker) override;
		
		//TODO: Return anim index to reference index
		int SubNumToRefNum(int subNum) override { return subNum; }
		
		
		// Loading/Saving
		IOResult Load(ILoad *iload) override;
		IOResult Save(ISave *isave) override;

		//From Animatable
		Class_ID ClassID() override {return CAMERAMAPTEXTURE_CLASS_ID;}
		SClass_ID SuperClassID() override { return TEXMAP_CLASS_ID; }

		void GetClassName(MSTR& s, bool localized) const override;

		RefTargetHandle Clone( RemapDir &remap ) override;
		RefResult NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, 
			PartID& partID, RefMessage message, BOOL propagate) override;


		int NumSubs() override { return 1; }
		Animatable* SubAnim(int i) override; 
		TSTR SubAnimName(int i, bool localized) override;

		// TODO: Maintain the number or references here 
		int NumRefs() override { return 1; }
		RefTargetHandle GetReference(int i) override;
private:
		void SetReference(int i, RefTargetHandle rtarg) override;
public:



		int	NumParamBlocks() override { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) override { return pblock; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) override { return (pblock->ID() == id) ? pblock : nullptr; } // return id'd ParamBlock

		void DeleteThis() override { delete this; }		
		//Constructor/Destructor

		CameraMapTexture(BOOL loading = false);
		~CameraMapTexture();	

		// this takes an existing shade context and creates a new one
		// overriding one map channel with the camera projection
		MyShadeContext BuildMyShadeContext(ShadeContext& sc);
		// this returns if a pixel should have mapping applied to it
		BOOL IsPixelHidden(ShadeContext& sc);
		
		// -- from InterfaceServer
		BaseInterface* GetInterface(Interface_ID id) override;

		//Variables to hold the param block data on update
		INode *cameraNode;			// the camera node
		int mapChannel;				// the map channel to replace
		Texmap *tex;				// the base texture
		Texmap *mask;				// mask to use against the texture
		ObjectState camOState;

		float fov;					// the FOV of the camera
		float aspectRatio = 1.0;	// Aspect ratio of the projection
		Matrix3 invCamTM;			// the matrix from world space to camera space

		BOOL backFace;				// whether to remove the back facing samples

		Point3 cameraPos;			// the position of the camera in world space

		BOOL useZBuffer;			// whether to use a zbuffer to mask off samples
		PBBitmap *zBufferPB;		// the zbuffer bitmap param block

		int width, height;			//the width and height of the zbuffer
		float *zbuffer;				//the raw zbuffer data

		float backFudge;			//this is fudge values to let the user relax the definition of what
									//behind/in front of the zbuffer.  Used to fix the problem that the 
									//zbuffer does not have fragment data.

		float angleThreshold;		//this is the angle threshold for determining back facing samples
									//this is in degrees and NOT radians 

		BOOL useMask;				// this is just a toggle to determine whether to use the mask or not
		BOOL maskUsesProjection;    // this toggle will force the mask to use the projection also

		BOOL affectBehindCam;		//this indicates the camera mapping affects points behind the camera


	public:
		// IParameterTranslator interfaces
		bool GetParameterValue(
			const TimeValue t, 
			const MCHAR* shaderParamName, 
			void* value, 
			ShaderParameterType type) override;
		bool GetShaderInputParameterName(SubMtlBaseType type, int subMtlBaseIndex, MSTR& shaderInputParamName) override;
		bool OnPreParameterTranslate() override { return true;}

		static void NotifySettingsChange(void* param, NotifyInfo* info)
		{
			if (auto cmp = static_cast<CameraMapTexture*>(param))
			{
				cmp->SettingsChange();
			}
		}

		// When a resolution setting changes and we are in legacy mode
		void SettingsChange()
		{
			// Do nothing unless we are in and we are in legacy mode
			if (pblock && pblock->GetInt(pb_aspectMode) < 2)
			{
				return;
			}
			// Setting the value causes an update
			pblock->SetValue(pb_aspectMode, 0, 2);
			Interface* ip = GetCOREInterface();
			ip->RedrawViews(ip->GetTime());
		}

	private:
		IShaderManager* GetShaderManager();
		IShaderManager* mpShaderManager;
};


class CameraMapTextureClassDesc:public ClassDesc2, public IMtlRender_Compatibility_MtlBase {
	public:

	CameraMapTextureClassDesc();

	int 			IsPublic() override { return TRUE; }
	void*			Create(BOOL loading = FALSE) override { return new CameraMapTexture(loading); }
	const TCHAR*	ClassName() override { return GetString(IDS_CLASS_NAME); }
	const TCHAR*	NonLocalizedClassName() { return _T("Camera Map Per Pixel "); }
	SClass_ID		SuperClassID() override { return TEXMAP_CLASS_ID; }
	Class_ID		ClassID() override { return CAMERAMAPTEXTURE_CLASS_ID; }
	const TCHAR* 	Category() override { return GetString(IDS_CATEGORY); }

	const TCHAR*	InternalName() override { return _T("CameraMapTexture"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() override { return hInstance; }				// returns owning module handle

	// -- from IMtlRender_Compability_MtlBase
	bool IsCompatibleWithRenderer(ClassDesc& rendererClassDesc) override;

	MaxSDK::QMaxParamBlockWidget* CreateQtWidget(ReferenceMaker& owner, IParamBlock2& paramBlock,
			const MapID paramMapID, MSTR& rollupTitle, int& rollupFlags, int& rollupCategory) override;

};

CameraMapTextureClassDesc::CameraMapTextureClassDesc() {

	IMtlRender_Compatibility_MtlBase::Init(*this);
}

// The class ID of the mental ray renderer
#define MRRENDERER_CLASSID Class_ID(0x58f67d6c, 0x4fcf3bc3)
bool CameraMapTextureClassDesc::IsCompatibleWithRenderer(ClassDesc& rendererClassDesc) {

	Class_ID classID = rendererClassDesc.ClassID();

	if((classID == MRRENDERER_CLASSID) || (classID == SCANLINERENDERER_CLASS_ID)) {
		return true;
	}
	else {
		// Return 'true' only if the renderer doesn't implement the compatibility interface.
		// This ensures that we are compatible with all renderers unless they specify the contrary.
		IMtlRender_Compatibility_Renderer* rendererCompatibility = Get_IMtlRender_Compatibility_Renderer(rendererClassDesc);
		return (rendererCompatibility == nullptr);
	}
}

MaxSDK::QMaxParamBlockWidget* CameraMapTextureClassDesc::CreateQtWidget(ReferenceMaker& owner, IParamBlock2& paramBlock,
		const MapID paramMapID, MSTR& rollupTitle, int& rollupFlags, int& rollupCategory)
{
	rollupTitle = GetString(IDS_PARAMS);
	return new CameraMapRollup();
}


static CameraMapTextureClassDesc CameraMapTextureDesc;
ClassDesc2* GetCameraMapTextureDesc() { return &CameraMapTextureDesc; }



static ParamBlockDesc2 cameramaptexture_param_blk ( cameramaptexture_params, _T("params"),  0, &CameraMapTextureDesc, 
	P_AUTO_CONSTRUCT + P_AUTO_UI_QT, PBLOCK_REF, 

	pb_node, 	_T("camera"),		TYPE_INODE, 		0,				IDS_PW_NODE,
		p_end, 

	// params
	pb_mapid, 			_T("MapChannel"), 		TYPE_INT, 	P_ANIMATABLE, 	IDS_SPIN, 
		p_default, 		1, 
		p_range, 		0,99, 
		p_nonLocalizedName, _T("Map Channel"),
		p_end,

	pb_texture, 	_T("texture"),		TYPE_TEXMAP, 		0,				IDS_TEXTURE,
		p_subtexno, 0,
		p_end, 

	pb_backface,	_T("backFace"), TYPE_BOOL, 0, IDS_BACKFACE,
		p_default,		TRUE,
		p_end,

	pb_usezbuffer,	_T("useZBuffer"), TYPE_BOOL, 0, IDS_USEZBUFFER,
		p_default,		FALSE,
		p_end,

	pb_zbuffer, _T("zbuffer"),	TYPE_BITMAP, 0, IDS_ZBUFFER,
		p_end,

	// params
	pb_zfudge, 			_T("ZFudge"), 		TYPE_FLOAT, 	P_ANIMATABLE, 	IDS_ZFUDGE, 
		p_default, 		1.0f, 
		p_range, 		0.0f,1000.0f, 
		p_nonLocalizedName, _T("Z Fudge"),
		p_end,

	// params
	pb_angle, 			_T("angleThreshold"), 		TYPE_FLOAT, 	P_ANIMATABLE, 	IDS_ANGLE ,
		p_default, 		90.0f,
		p_range, 		0.0f, 180.0f,
		p_nonLocalizedName, _T("Angle Threshold"),
		p_end,

	pb_mask, 	_T("mask"),		TYPE_TEXMAP, 		0,				IDS_MASK,
		p_subtexno, 1,
		p_end, 

	pb_usemask,	_T("useMask"), TYPE_BOOL, 0, IDS_USEMASK,
		p_default,		FALSE,
		p_end,

	pb_maskuseprojection,	_T("maskUsesProjection"), TYPE_BOOL, 0, IDS_MASKUSESPROJECTION,
		p_default,		TRUE,
		p_end,

	pb_affectbehindcam,	_T("affectBehindCam"), TYPE_BOOL, 0, IDS_AFFECTBEHINDCAM,
		p_default,		FALSE,
		p_end,

	pb_aspectMode, _T("aspectMode"), TYPE_INT, 0, IDS_ASPECTMODE,
		p_default, 2,
		p_range, 0, 2,
		p_end,

	pb_haspect,   _T("haspect"), TYPE_FLOAT, P_ANIMATABLE, IDS_ASPECT_H,
		p_default, 16.0f,
		p_nonLocalizedName, _T("Horizontal Aspect"),
		p_end,

	pb_vaspect,   _T("vaspect"), TYPE_FLOAT, P_ANIMATABLE, IDS_ASPECT_V,
		p_default, 9.0f,
		p_nonLocalizedName, _T("Vertical Aspect"),
		p_end,

	p_end
	);




//--- CameraMapTexture -------------------------------------------------------
CameraMapTexture::CameraMapTexture(BOOL loading)
		: tex(nullptr)
		, mask(nullptr)
		, zBufferPB(nullptr)
		, mpShaderManager(nullptr)
{
	// TODO: Add all the initializing stuff
	pblock = nullptr;
	CameraMapTextureDesc.MakeAutoParamBlocks(this);

	if (pblock && !loading)
	{
		// For old files (that are missing the new pb_aspectMode
		// parameter, we have set the param block default it to
		// Legacy mode. If the file has that parameter (so it's a new file),
		// this will be overwritten by the data in the file
		// However, when we are NOT loading the file, and creating
		// a brand new one, we want to present the user with a NEW and BETTER
		// default, hence this code.
		int value = 0;
		pblock->SetValue(pb_aspectMode, 0, value);
	}

	ivalid.SetEmpty();
	RegisterNotification(NotifySettingsChange, this, NOTIFY_RENDPARAM_CHANGED);
}

CameraMapTexture::~CameraMapTexture()
{
	UnRegisterNotification(NotifySettingsChange, this, NOTIFY_RENDPARAM_CHANGED);
	IShaderManagerCreator::GetInstance()->DeleteShaderManager(mpShaderManager);
	mpShaderManager = nullptr;
}

// From MtlBase
void CameraMapTexture::Reset()
{
	ivalid.SetEmpty();
}

void CameraMapTexture::Update(TimeValue t, Interval& valid) 
{	
	//TODO: Add code to evaluate anything prior to rendering

	if (!ivalid.InInterval(t))
	{
		NotifyDependents(FOREVER, PART_TEXMAP, REFMSG_DISPLAY_MATERIAL_CHANGE);
	}
	// Interval ivalid;
	ivalid = FOREVER;

	cameraPos = Point3(0.0f,0.0f,0.0f);
	fov = PI/4.0f;

	pblock->GetValue(pb_texture, t, tex, ivalid);


	if (tex)
	{
		// start by legacy default
		Interface* ip = GetCOREInterface();
		aspectRatio = ip->GetRendImageAspect();

		// Then check, do we have a better mode?
		switch (pblock->GetInt(pb_aspectMode, t))
		{
		case 0: // Use bitmap if available
		{
			if (BitmapTex* pBitmapTex = GetIBitmapTextInterface(tex))
			{
				if (Bitmap* pBitmap = pBitmapTex->GetBitmap(t))
				{
					float width = pBitmap->Width();
					float height = pBitmap->Height();

					aspectRatio = width / height;

					// Break from case ONLY if found, otherwise, fall-through to the
					// manual computation below!!
					break;
				}
			}
		}
		// INTENTIONAL FALLTHROUGH ! ! !    
		case 1: // Use custom value (FALLTHROUGH FROM ABOVE INTENTIONAL!!)
			{
				float width = 0.0f, height = 0.0f;
				pblock->GetValue(pb_haspect, t, width, valid);
				pblock->GetValue(pb_vaspect, t, height, valid);

				aspectRatio = width / height;
			}
		}
	}

	//get our data and store it off so the evals can use it
	//since we don't want to be calling getvalue in the evals
	pblock->GetValue(pb_node,t,cameraNode,ivalid);

	pblock->GetValue(pb_mapid,t,mapChannel,ivalid);

	pblock->GetValue(pb_mask,t,mask,ivalid);
	pblock->GetValue(pb_backface,t,backFace,ivalid);

	pblock->GetValue(pb_usezbuffer,t,useZBuffer,ivalid);

	pblock->GetValue(pb_zbuffer,t,zBufferPB,ivalid);

	pblock->GetValue(pb_zfudge,t,backFudge,ivalid);

	pblock->GetValue(pb_angle,t,angleThreshold,ivalid);
	
	pblock->GetValue(pb_usemask,t,useMask,ivalid);

	pblock->GetValue(pb_maskuseprojection,t,maskUsesProjection,ivalid);

	pblock->GetValue(pb_affectbehindcam,t,affectBehindCam,ivalid);

	// update our texture
	if (tex)
	{
		tex->Update(t, ivalid);
	}

	//update our mask
	if (mask)
	{
		mask->Update(t,ivalid);
	}

	//if we have a camera node extract the camera data
	if (cameraNode)
	{

		camOState = cameraNode->EvalWorldState(t);

		Interval cameraInterval = FOREVER;
		invCamTM = cameraNode->GetObjTMAfterWSM(t, &cameraInterval); //watje Defect 661145  switched to the AfterWSM to include cameras modified by theing like skew etc
		// invCamTM = cameraNode->GetObjectTM(t,&cameraInterval);
		ivalid &= cameraInterval;
		
		cameraPos = invCamTM.GetTrans();

		invCamTM = Inverse(invCamTM);

		if (camOState.obj->SuperClassID() == CAMERA_CLASS_ID)
		{
			fov = ((CameraObject *)camOState.obj)->GetFOV(t, valid);
		}
	}

	zbuffer = nullptr;
	//get our zbuffer info needed
	if (zBufferPB && useZBuffer)
	{
		width = zBufferPB->bi.Width();
		height = zBufferPB->bi.Height();

		zBufferPB->Load();
		if (zBufferPB->bm)
		{
			DWORD type{};
			zbuffer = (float*)zBufferPB->bm->GetChannel(BMM_CHAN_Z, type);
		}
	}

	valid = ivalid;	

}

Interval CameraMapTexture::Validity(TimeValue t)
{
	//TODO: Update ivalid here
	return ivalid;
}

ParamDlg* CameraMapTexture::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) 
{
	IAutoMParamDlg* dlg = CameraMapTextureDesc.CreateParamDlgs(hwMtlEdit, imp, this);
	//TODO: Set the user dialog proc of the param block, and do other initialization	
	return dlg;	
}

BOOL CameraMapTexture::SetDlgThing(ParamDlg* dlg)
{	
	return FALSE;
}



//From ReferenceMaker
RefTargetHandle CameraMapTexture::GetReference(int i) 
{
	return i == 0 ? pblock : nullptr;
}

void CameraMapTexture::SetReference(int i, RefTargetHandle rtarg)
{
	if (i == 0)
	{
		pblock = (IParamBlock2*)rtarg;
	}
}

void CameraMapTexture::GetClassName(MSTR& s, bool localized) const
{
	s = localized ? GetString(IDS_CLASS_NAME_IMP) : _T("Camera Map Per Pixel ");
}

//From ReferenceTarget 
RefTargetHandle CameraMapTexture::Clone(RemapDir &remap) 
{
	CameraMapTexture *mnew = new CameraMapTexture();
	*((MtlBase*)mnew) = *((MtlBase*)this); // copy superclass stuff
	//TODO: Add other cloning stuff
	BaseClone(this, mnew, remap);

	mnew->ReplaceReference(PBLOCK_REF,remap.CloneRef(pblock));

	return (RefTargetHandle)mnew;
}

	 
Animatable* CameraMapTexture::SubAnim(int i) 
{
	return i == 0 ? pblock : nullptr;
}

TSTR CameraMapTexture::SubAnimName(int i, bool localized)
{
	return i == 0 ? (localized ? GetString(IDS_PARAMS) : _T("Camera Map Parameters")) : _T("");
}

RefResult CameraMapTexture::NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, 
   PartID& partID, RefMessage message, BOOL propagate ) 
{
	switch (message) {
		case REFMSG_CHANGE:
			{
			ivalid.SetEmpty();
			if (hTarget == pblock)
				{
			// see if this message came from a changing parameter in the pblock,
			// if so, limit rollout update to the changing item and update any active viewport texture
				ParamID changing_param = pblock->LastNotifyParamID();
				cameramaptexture_param_blk.InvalidateUI(changing_param);
				if ((partID & PART_TM) != 0)
				{
					// Use a customized value for camera tm change
					const PartID PART_CAMERA = 1 << 25;
					partID |= PART_CAMERA;
				}
				}
			break;
			}
		}
	return(REF_SUCCEED);
	}

IOResult CameraMapTexture::Save(ISave* isave)
{
	return IO_OK;
}

IOResult CameraMapTexture::Load(ILoad* iload)
{
	return IO_OK;
}

//this builds our new shade context from an existing one
//and puts in our camera projection into it
MyShadeContext CameraMapTexture::BuildMyShadeContext(ShadeContext& sc)
{
	MyShadeContext myContext;
	myContext.sc = &sc;
	myContext.mode = sc.mode;
	myContext.doMaps = sc.doMaps;
	myContext.filterMaps = sc.filterMaps;
	myContext.shadow = sc.shadow;
	myContext.backFace = sc.backFace;
	myContext.mtlNum = sc.mtlNum;

	myContext.ambientLight = sc.ambientLight;
	myContext.nLights = sc.nLights;
	myContext.rayLevel = sc.rayLevel;

	myContext.xshadeID = sc.xshadeID;
	myContext.atmosSkipLight = sc.atmosSkipLight;
	myContext.globContext = sc.globContext;
	myContext.out = sc.out;

	myContext.mapChannelID = mapChannel;
	// Get the Field of View parameter from the camera we reference
	// Compute the scale factors
	double xScale = -0.5 / (tan(0.5*(double)fov));

	float yScale = xScale*aspectRatio;

	// Transform the points into screen space
	float zScale = 1.0f;
	float distance, x, y, z;

	Point3 p = sc.P();
	Point3 cameraSpace = p;

	Point3 worldP = sc.PointTo(sc.P(), REF_WORLD);
	p = worldP * invCamTM;
	x = p.x; 
	y = p.y; 
	z = p.z;

	distance = (float) sqrt(x*x + y*y + z*z);
	myContext.myUVW.x = p.x*xScale/z + 0.5f;
	myContext.myUVW.y = p.y*yScale/z + 0.5f;
	myContext.myUVW.z = distance;

	return myContext;
}

//this checks to see if a pixel should be rendered with the
//projection
BOOL CameraMapTexture::IsPixelHidden(ShadeContext& sc)
{

	//first check the backfacing
	if (backFace)
	{
		//compute the angle between our normal and the vector from our node to camera origin
		Point3 nodePos = sc.PointFrom(cameraPos,REF_WORLD);
		Point3 v = FNormalize(nodePos - sc.P());
		float d = DotProd(sc.Normal(), v);
		float angle = 0.0f;
		if (d != 0.0f)
		{
			angle = acos(d) * 180.0f/PI;
		}
		if (angle > angleThreshold)
		{
			return TRUE;
		}
	}


	//get the pixel in camera space,
	//convert to world, then to our projection camera
	Point3 tp = sc.P();
	tp = sc.PointTo(tp, REF_WORLD) * invCamTM;

	// check if the point is behind the camera, and affect behind camera is disabled
	if ((!affectBehindCam) && (tp.z > 0))
	{
		return TRUE;
	}


	//now check the zbuffer if need be
	if (zbuffer && useZBuffer)
	{
		double x, y, z;
		x = tp.x; 
		y = tp.y; 
		z = tp.z;

		double px,py;

		//compute where we are in screen space
		double xScale = -0.5 / (tan(0.5*(double)fov));
		float yScale = xScale*aspectRatio;

		px = tp.x*xScale/z + 0.5;
		py = tp.y*yScale/z + 0.5;

		py = 1.0f - py;

		//figure out were we are in the bitmap
		int ix = (px * (float)(width-1) + 0.5);
		int iy = (py * (float)(height -1) + 0.5);
		int index = iy * width + ix;

		//if out of the UV space the pixel is hidden we don't tile the map, should we?
		if (px < 0.0f)		return TRUE;
		else if (px > 1.0f)	return TRUE;

		else if (py < 0.0f) return TRUE;
		else if (py > 1.0f) return TRUE;
		else
		{
			
			z = zbuffer[index];
			float dif = fabs(z - tp.z);
			//see if we arr behind the zbuffer
			if (dif > backFudge)
			{
				//we are behind the mask now comes the hack
				//since we dont have fragment data in the ZBuffer we can't exactly determine if this
				//sample is behind/infront of the mask
				//so we check the neighboring pixels, if they any one of them is infront of the mask
				//the sample is infront of the mask.  This creates a potential 1 pixel errors 
				//but is fine since the altenative is that it will create a halo
				int startX, endX;
				int startY, endY;
				startX = ix-1;
				endX = ix+1;
				startY = iy-1;
				endY = iy+1;

				if (startX < 0) startX = 0;
				if (startY < 0) startY = 0;
				if (endX > (width-1)) endX = (width-1);
				if (endY > (height-1)) endX = (height-1);
				BOOL pass = FALSE;

				for (int j = startY; j <= endY; j++)
				{
					for (int i = startX; i <= endX; i++)
					{
						int index = j * width + i;
						z = zbuffer[index];
						float dif = fabs(z - tp.z);
						if (dif < backFudge)
						{
							pass = TRUE;
							i = endX + 1;
							j = endY + 1;
						}

					}
				}

				if (!pass)
					return TRUE;

			}			
		}
	}

	return FALSE;

}

AColor CameraMapTexture::EvalColor(ShadeContext& sc)
{
	// TODO: Evaluate the color of texture map for the context.
	AColor c;
	if (tex)
	{
		ObjectState camOState;
		Interval valid;

		MyShadeContext myContext = BuildMyShadeContext(sc);

		if (IsPixelHidden(sc))
		{
			c.r = 0.0f;
			c.g = 0.0f;
			c.b = 0.0f;
			c.a = 0.0f;
		}
		else
		{
			c = tex->EvalColor(myContext);
			if (mask && useMask)
			{
				float m = maskUsesProjection ? mask->EvalMono(myContext) : mask->EvalMono(sc);
				c *= m;
			}
		}
	}
	else
	{
		c.r = 0.0f;
		c.g = 0.0f;
		c.b = 0.0f;
		c.a = 0.0f;
	}
	return c;
}

float CameraMapTexture::EvalMono(ShadeContext& sc)
{
	float c;
	if (tex)
	{
		ObjectState camOState;
		Interval valid;

		MyShadeContext myContext = BuildMyShadeContext(sc);

		if (IsPixelHidden(sc))
		{
			c = 0.0f;
		}
		else
		{
			c = tex->EvalMono(myContext);
			if (mask && useMask)
			{
				float m = maskUsesProjection ? mask->EvalMono(myContext) : mask->EvalMono(sc);
				c *= m;
			}
		}
	}
	else
	{
		c = 0.0f;
	}
	return c;
}

///not sure if this will work
Point3 CameraMapTexture::EvalNormalPerturb(ShadeContext& sc)
{
	if (tex)
	{
		ObjectState camOState;
		Interval valid;

		MyShadeContext myContext = BuildMyShadeContext(sc);


		if (IsPixelHidden(sc))
			return Point3(0.0f,0.f,0.0f);

		Point3 c = tex->EvalNormalPerturb(myContext);


		if ((mask) && useMask)
		{
			float m = 0.0f;
			if (maskUsesProjection)
				m = mask->EvalMono(myContext);
			else m = mask->EvalMono(sc);			
			c *= m;
		}
		return c;
	}	
	return Point3(0, 0, 0);
}


void CameraMapTexture::ActivateTexDisplay(BOOL onoff)
{
	//TODO: Implement this only if SupportTexDisplay() returns TRUE
}

DWORD_PTR CameraMapTexture::GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker)
{
	//TODO: Return the texture handle to this texture map
	return NULL;
}

BaseInterface* CameraMapTexture::GetInterface(Interface_ID id) {

	if (id == ISHADER_MANAGER_INTERFACE_ID) {
		return GetShaderManager();
	}
	else if (id ==IPARAMETER_TRANSLATOR_INTERFACE_ID) {
		return static_cast<IParameterTranslator*>(this);
	}
	else {
		return Texmap::GetInterface(id);
	}
}

bool CameraMapTexture::GetParameterValue(
	const TimeValue t, 
	const MCHAR* shaderParamName, 
	void* value, 
	ShaderParameterType)
{
	if (!ivalid.InInterval(t))
	{
		Interval valid;
		Update(t,valid);
	}

	if (_tcscmp(shaderParamName, _M("camera_matViewproj")) == 0 && cameraNode && tex)
	{
		CameraState cs;
		Interval iv;

		CameraObject* pCameraObject = nullptr;
		const ObjectState& os = cameraNode->EvalWorldState(t);
		pCameraObject = (CameraObject *)os.obj;
		pCameraObject->EvalCameraState(t,iv,&cs);

		#define MATRIX_EPSILON   0.000001f

		float frustumDepth = cs.yon - cs.hither;
		float oneOverDepth = (frustumDepth < MATRIX_EPSILON ) ? 1.0f / MATRIX_EPSILON : (1.0f / frustumDepth);
		Matrix44 projectionMatrix;
		projectionMatrix.MakeIdentity();
		if (!cs.isOrtho)
		{
			projectionMatrix._11 = 1.0f / tan(0.5f * cs.fov);
			projectionMatrix._22 = projectionMatrix._11 * aspectRatio;
			projectionMatrix._33 = -(cs.yon * oneOverDepth);
			projectionMatrix._43 = (oneOverDepth==0)? -1 : ((-cs.yon * cs.hither) * oneOverDepth);
			projectionMatrix._34 = -1.0f;
			projectionMatrix._44 = 0.0f;
		}
		else
		{
			float focalDist = 1.0f;
			if(cameraNode->GetTarget())
			{
				focalDist = Length(cameraNode->GetNodeTM(t).GetTrans() - cameraNode->GetTarget()->GetNodeTM(t).GetTrans());
			}
			else
			{
				focalDist = cs.tdist;
			}

			float w = 2.0f * focalDist * (float)tan(cs.fov/2.0);
			float h = w / aspectRatio; 
			projectionMatrix._11 = 2.0f / w;
			projectionMatrix._22 = 2.0f / h;
			projectionMatrix._33 = -oneOverDepth;
			projectionMatrix._43 = -cs.hither * oneOverDepth;
		}

		Matrix3 imat;
		imat = cameraNode->GetObjTMAfterWSM(t,&iv);
		for (int i=0; i<3; i++) 
		{
			imat.SetRow(i,Normalize(imat.GetRow(i)));
		}
		Matrix3 affineTM = Inverse(imat);
		Matrix44 viewMatrix;
		MaxWorldMatrixToMatrix44(viewMatrix, affineTM);
		viewMatrix.MakeMultiply(projectionMatrix);
		*((Matrix44*)value) = viewMatrix;
		return true;
	}
	else if (_tcscmp(shaderParamName, _M("camera_map")) == 0)
	{
		*((Point4*)value) = Point4(0.0f,0.0f,0.0f,0.0f);
		return true;
	}

	return false;
}

bool CameraMapTexture::GetShaderInputParameterName(SubMtlBaseType type, int subMtlBaseIndex, MSTR& shaderInputParamName)
{
	if (subMtlBaseIndex == 0)
	{
		shaderInputParamName = _M("camera_map");
		return true;
	}
	return false;
}

IShaderManager* CameraMapTexture::GetShaderManager()
{
	if (!mpShaderManager)
	{
		mpShaderManager = IShaderManagerCreator::GetInstance()->CreateShaderManager(
				IShaderManager::ShaderTypeAMG, _M("max_CameraTextureMap"), _M(""), this);
	}
	return mpShaderManager;
}
