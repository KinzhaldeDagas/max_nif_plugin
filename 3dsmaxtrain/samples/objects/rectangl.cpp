/**********************************************************************
 *<
	FILE: rectangl.cpp

	DESCRIPTION:  An rectangular spline object implementation

	CREATED BY: Tom Hudson

	HISTORY: created 23 February 1995
			 Fillets added 1/12/97 - Audrey Peterson

 *>	Copyright (c) 1995,1996,1997 All Rights Reserved.
 **********************************************************************/

#include "prim.h" 

#include "splshape.h"
#include "iparamm.h"
// This is based on the simple spline object...
#include "simpspl.h"


#define MIN_LENGTH		float(0)
#define MAX_LENGTH		float( 1.0E30)
#define MIN_WIDTH		float(0)
#define MAX_WIDTH		float( 1.0E30)

#define DEF_LENGTH		float(0.0)
#define DEF_WIDTH		float(0.0)

#define CREATE_EDGE 0
#define CREATE_CENTER 1
#define CREATE_3POINT 2

class RectangleObjCreateCallBack;

class RectangleObject: public SimpleSpline, public IParamArray {
		friend class RectangleObjCreateCallBack;

	public:	
		// Class vars
		static IParamMap *pmapCreate;
		static IParamMap *pmapTypeIn;
		static IParamMap *pmapParam;
		static IObjParam *ip;
		static int dlgCreateMeth;
		static Point3 crtPos;		
		static float crtLength;
		static float crtWidth;
		static float crtFillet;
		
		void BuildShape(TimeValue t,BezierShape& ashape);

		RectangleObject();
		~RectangleObject();

		//  inherited virtual methods:

	    IOResult Load(ILoad *iload);
		CreateMouseCallBack* GetCreateMouseCallBack();
		void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
		const TCHAR *GetObjectName(bool localized) const override { return localized ? GetString(IDS_TH_RECTANGLE) : _T("Rectangle"); }
		void InitNodeName(TSTR& s) { s = GetString(IDS_TH_RECTANGLE); }		
		Class_ID ClassID() { return Class_ID(RECTANGLE_CLASS_ID,0); }  
		void GetClassName(MSTR& s, bool localized) const override { s = localized ? GetString(IDS_TH_RECTANGLE_CLASS) : _T("Rectangle"); }
		RefTargetHandle Clone(RemapDir& remap);
		BOOL ValidForDisplay(TimeValue t);
		int Display(TimeValue t, INode* inode, ViewExp* vpt, int flags) override;

		// From IParamArray
		BOOL SetValue(int i, TimeValue t, int v);
		BOOL SetValue(int i, TimeValue t, float v);
		BOOL SetValue(int i, TimeValue t, Point3 &v);
		BOOL GetValue(int i, TimeValue t, int &v, Interval &ivalid);
		BOOL GetValue(int i, TimeValue t, float &v, Interval &ivalid);
		BOOL GetValue(int i, TimeValue t, Point3 &v, Interval &ivalid);

		ParamDimension *GetParameterDim(int pbIndex);
		TSTR GetParameterName(int pbIndex, bool localized);

		void InvalidateUI() { if (pmapParam) pmapParam->Invalidate(); }

		// Shape angle display -- Controls the display of the shape angle value
		// alongside the cursor when Shape Angle Constraints are active
		bool mDisplayShapeAngle = false;
		float mShapeAngleValue = 0.0f;
		IPoint2 mShapeAnglePosition;	// Viewport coordinates
	};				

//------------------------------------------------------

class RectangleObjClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void*			Create(BOOL loading = FALSE) { return new RectangleObject; }
	const TCHAR*	ClassName() { return GetString(IDS_TH_RECTANGLE_CLASS); }
	const TCHAR*	NonLocalizedClassName() { return _T("Rectangle"); }
	SClass_ID		SuperClassID() { return SHAPE_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(RECTANGLE_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetString(IDS_TH_SPLINES);  }
	void			ResetClassParams(BOOL fileReset);
	};

static RectangleObjClassDesc rectangleObjDesc;

ClassDesc* GetRectangleDesc() { return &rectangleObjDesc; }

// in prim.cpp  - The dll instance handle
extern HINSTANCE hInstance;

// class variable for sphere class.
IParamMap *RectangleObject::pmapCreate = NULL;
IParamMap *RectangleObject::pmapParam  = NULL;
IParamMap *RectangleObject::pmapTypeIn = NULL;
IObjParam *RectangleObject::ip         = NULL;
int RectangleObject::dlgCreateMeth = CREATE_EDGE;
Point3 RectangleObject::crtPos         = Point3(0,0,0);
float RectangleObject::crtLength       = 0.0f;
float RectangleObject::crtWidth        = 0.0f;
float RectangleObject::crtFillet        = 0.0f;

void RectangleObjClassDesc::ResetClassParams(BOOL fileReset)
	{
	RectangleObject::dlgCreateMeth   = CREATE_EDGE;
	RectangleObject::crtPos          = Point3(0,0,0);
	RectangleObject::crtLength       = 0.0f;
	RectangleObject::crtWidth        = 0.0f;
	RectangleObject::crtFillet       = 0.0f;
	}

// Parameter map indices
#define PB_LENGTH		0
#define PB_WIDTH		1
#define PB_FILLET		2

// Non-parameter block indices
#define PB_CREATEMETHOD		0
#define PB_TI_POS			1
#define PB_TI_LENGTH		2
#define PB_TI_WIDTH			3
#define PB_TI_FILLET		4

//
//
//	Creation method

static int createMethIDs[] = {IDC_CREATEEDGE,IDC_CREATECENTER,IDC_CREATE_3POINT};

static ParamUIDesc descCreate[] = {
	// Diameter/radius
	ParamUIDesc(PB_CREATEMETHOD,TYPE_RADIO,createMethIDs,3)
	};
#define CREATEDESC_LENGTH 1

//
//
// Type in

static ParamUIDesc descTypeIn[] = {
	
	// Position
	ParamUIDesc(
		PB_TI_POS,
		EDITTYPE_UNIVERSE,
		IDC_TI_POSX,IDC_TI_POSXSPIN,
		IDC_TI_POSY,IDC_TI_POSYSPIN,
		IDC_TI_POSZ,IDC_TI_POSZSPIN,
		-99999999.0f,99999999.0f,
		SPIN_AUTOSCALE),
	
	// Length
	ParamUIDesc(
		PB_TI_LENGTH,
		EDITTYPE_UNIVERSE,
		IDC_LENGTHEDIT,IDC_LENSPINNER,
		MIN_LENGTH,MAX_LENGTH,
		SPIN_AUTOSCALE),	

	// Width
	ParamUIDesc(
		PB_TI_WIDTH,
		EDITTYPE_UNIVERSE,
		IDC_WIDTHEDIT,IDC_WIDTHSPINNER,
		MIN_WIDTH,MAX_WIDTH,
		SPIN_AUTOSCALE),

	// Fillet
	ParamUIDesc(
		PB_TI_FILLET,
		EDITTYPE_UNIVERSE,
		IDC_FILLET,IDC_FILLETSPINNER,
		MIN_WIDTH,MAX_WIDTH,
		SPIN_AUTOSCALE)
			
	};
#define TYPEINDESC_LENGTH 4

//
//
// Parameters

static ParamUIDesc descParam[] = {
	// Length
	ParamUIDesc(
		PB_LENGTH,
		EDITTYPE_UNIVERSE,
		IDC_LENGTHEDIT,IDC_LENSPINNER,
		MIN_LENGTH,MAX_LENGTH,
		SPIN_AUTOSCALE),	
	
	// Width
	ParamUIDesc(
		PB_WIDTH,
		EDITTYPE_UNIVERSE,
		IDC_WIDTHEDIT,IDC_WIDTHSPINNER,
		MIN_WIDTH,MAX_WIDTH,
		SPIN_AUTOSCALE),	
	
	// Fillet
	ParamUIDesc(
		PB_FILLET,
		EDITTYPE_UNIVERSE,
		IDC_FILLET,IDC_FILLETSPINNER,
		MIN_WIDTH,MAX_WIDTH,
		SPIN_AUTOSCALE)	
	
	};
#define PARAMDESC_LENGTH 3


static ParamBlockDescID descVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_FLOAT, NULL, TRUE, 1 },
 };
static ParamBlockDescID descVer1[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },
	{ TYPE_FLOAT, NULL, TRUE, 1 },
	{ TYPE_FLOAT, NULL, TRUE, 2 },
 };
#define PBLOCK_LENGTH	3

// Array of old versions
static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer0,2,0)
	};
#define NUM_OLDVERSIONS	1	

// Current version
#define CURRENT_VERSION	1
static ParamVersionDesc curVersion(descVer1,PBLOCK_LENGTH,CURRENT_VERSION);

class RectPostLoadCallback : public PostLoadCallback {
public:
	ParamBlockPLCB *cb;
	RectPostLoadCallback(ParamBlockPLCB *c) {cb=c;}
	virtual void proc(ILoad *iload) {
		if (cb == NULL || !cb->IsValid())
		{
			delete this;
			return;
		}
		// The call to IsValid above verified that the pblock is safe
		DWORD oldVer = ((RectangleObject*)(cb->GetTarget()))->pblock->GetVersion();
		ReferenceTarget *targ = cb->GetTarget();
		cb->proc(iload);
		cb = NULL;
		if (oldVer==0) {
			((RectangleObject*)targ)->pblock->SetValue(PB_FILLET,0,0.0f);
		}
		delete this;
	}
	virtual int Priority() { return 0; }
};
IOResult RectangleObject::Load(ILoad *iload)
	{  	iload->RegisterPostLoadCallback(
			new RectPostLoadCallback(
				new ParamBlockPLCB(versions,NUM_OLDVERSIONS,&curVersion,this,USERPBLOCK)));
	return SimpleSpline::Load(iload);
	}


//--- TypeInDlgProc --------------------------------

class RectangleTypeInDlgProc : public ParamMapUserDlgProc {
	public:
		RectangleObject *ro;

		RectangleTypeInDlgProc(RectangleObject *d) {ro=d;}
		INT_PTR DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() {delete this;}
	};

INT_PTR RectangleTypeInDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_TI_CREATE: {
					if (ro->crtLength==0.0 && ro->crtWidth==0.0) return TRUE;
					
					// Return focus to the top spinner
					SetFocus(GetDlgItem(hWnd, IDC_TI_POSX));
					
					// We only want to set the value if the object is 
					// not in the scene.
					if (ro->TestAFlag(A_OBJ_CREATING)) {
						ro->pblock->SetValue(PB_LENGTH,0,ro->crtLength);
						ro->pblock->SetValue(PB_WIDTH,0,ro->crtWidth);
						ro->pblock->SetValue(PB_FILLET,0,ro->crtFillet);
						}

					Matrix3 tm(1);
					tm.SetTrans(ro->crtPos);
					ro->ip->NonMouseCreate(tm);
					// NOTE that calling NonMouseCreate will cause this
					// object to be deleted. DO NOT DO ANYTHING BUT RETURN.
					return TRUE;	
					}
				}
			break;	
		}
	return FALSE;
	}

void RectangleObject::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev)
	{
	SimpleSpline::BeginEditParams(ip,flags,prev);
	this->ip = ip;

	if (pmapCreate && pmapParam) {
		
		// Left over from last shape ceated
		pmapCreate->SetParamBlock(this);
		pmapTypeIn->SetParamBlock(this);
		pmapParam->SetParamBlock(pblock);
	} else {
		
		// Gotta make a new one.
		if (flags&BEGIN_EDIT_CREATE) {
			pmapCreate = CreateCPParamMap(
				descCreate,CREATEDESC_LENGTH,
				this,
				ip,
				hInstance,
				MAKEINTRESOURCE(IDD_RECTANGLEPARAM1),
				GetString(IDS_TH_CREATION_METHOD),
				0);

			pmapTypeIn = CreateCPParamMap(
				descTypeIn,TYPEINDESC_LENGTH,
				this,
				ip,
				hInstance,
				MAKEINTRESOURCE(IDD_RECTANGLEPARAM3),
				GetString(IDS_TH_KEYBOARD_ENTRY),
				APPENDROLL_CLOSED);
			}

		pmapParam = CreateCPParamMap(
			descParam,PARAMDESC_LENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_RECTANGLEPARAM2),
			GetString(IDS_TH_PARAMETERS),
			0);
		}

	if(pmapTypeIn) {
		// A callback for the type in.
		pmapTypeIn->SetUserDlgProc(new RectangleTypeInDlgProc(this));
		}
	}
		
void RectangleObject::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
{
	SimpleSpline::EndEditParams(ip,flags,next);
	this->ip = NULL;

	if (flags & END_EDIT_REMOVEUI ) {
		if (pmapCreate) DestroyCPParamMap(pmapCreate);
		if (pmapTypeIn) DestroyCPParamMap(pmapTypeIn);
		DestroyCPParamMap(pmapParam);
		pmapParam  = NULL;
		pmapTypeIn = NULL;
		pmapCreate = NULL;
	}
	else
	{
		pmapTypeIn->SetUserDlgProc(nullptr);
		pmapCreate->SetParamBlock(nullptr);
		pmapTypeIn->SetParamBlock(nullptr);
		pmapParam->SetParamBlock(nullptr);
	}
}
#define CIRCLE_VECTOR_LENGTH 0.5517861843f

void RectangleObject::BuildShape(TimeValue t, BezierShape& ashape) {

	// Start the validity interval at forever and whittle it down.
	ivalid = FOREVER;
	float length,fillet;
	float width;
	pblock->GetValue(PB_LENGTH, t, length, ivalid);
	pblock->GetValue(PB_WIDTH, t, width, ivalid);
	pblock->GetValue(PB_FILLET, t, fillet, ivalid);
	LimitValue( length, MIN_LENGTH, MAX_LENGTH );
	LimitValue( width, MIN_WIDTH, MAX_WIDTH );
	LimitValue( fillet, MIN_WIDTH, MAX_WIDTH );
	// Delete the existing shape and create a new spline in it
	ashape.NewShape();

	// Get parameters from SimpleSpline and place them in the BezierShape
	int steps;
	BOOL optimize,adaptive;
	ipblock->GetValue(IPB_STEPS, t, steps, ivalid);
	ipblock->GetValue(IPB_OPTIMIZE, t, optimize, ivalid);
	ipblock->GetValue(IPB_ADAPTIVE, t, adaptive, ivalid);
	ashape.steps = adaptive ? -1 : steps;
	ashape.optimize = optimize;

	Spline3D *spline = ashape.NewSpline();

	// Now add all the necessary points
	// We'll add 'em as auto corners initially, have the spline package compute some vectors (because
	// I'm basically lazy and it does a great job, besides) then turn 'em into bezier corners!
	float l2 = length / 2.0f;
	float w2 = width / 2.0f;
	Point3 p = Point3(w2, l2, 0.0f);
	int pts=4;
	if (fillet>0)
	{ pts=8;
	  float cf=fillet*CIRCLE_VECTOR_LENGTH;
	  Point3 wvec=Point3(fillet,0.0f,0.0f),lvec=Point3(0.0f,fillet,0.0f);
	  Point3 cwvec=Point3(cf,0.0f,0.0f),clvec=Point3(0.0f,cf,0.0f);
	  Point3 p3=p-lvec,p2;
	  spline->AddKnot(SplineKnot(KTYPE_BEZIER,LTYPE_CURVE,p3,p3-clvec,p3+clvec));
	  p=p-wvec;
	  spline->AddKnot(SplineKnot(KTYPE_BEZIER,LTYPE_CURVE,p,p+cwvec,p-cwvec));
	  p=Point3(-w2,l2,0.0f);p2=p+wvec;
	  spline->AddKnot(SplineKnot(KTYPE_BEZIER,LTYPE_CURVE,p2,p2+cwvec,p2-cwvec));
	  p=p-lvec;
	  spline->AddKnot(SplineKnot(KTYPE_BEZIER,LTYPE_CURVE,p,p+clvec,p-clvec));
	  p=Point3(-w2,-l2,0.0f);p3=p+lvec;
	  spline->AddKnot(SplineKnot(KTYPE_BEZIER,LTYPE_CURVE,p3,p3+clvec,p3-clvec));
	  p=p+wvec;
	  spline->AddKnot(SplineKnot(KTYPE_BEZIER,LTYPE_CURVE,p,p-cwvec,p+cwvec));
	  p = Point3(w2, -l2, 0.0f);p3=p-wvec;
	  spline->AddKnot(SplineKnot(KTYPE_BEZIER,LTYPE_CURVE,p3,p3-cwvec,p3+cwvec));
	  p=p+lvec;
	  spline->AddKnot(SplineKnot(KTYPE_BEZIER,LTYPE_CURVE,p,p-clvec,p+clvec));
	spline->SetClosed();
	spline->ComputeBezPoints();
	} 
	else
	{spline->AddKnot(SplineKnot(KTYPE_CORNER,LTYPE_CURVE,p,p,p));
	p = Point3(-w2, l2, 0.0f);
	spline->AddKnot(SplineKnot(KTYPE_CORNER,LTYPE_CURVE,p,p,p));
	p = Point3(-w2, -l2, 0.0f);
	spline->AddKnot(SplineKnot(KTYPE_CORNER,LTYPE_CURVE,p,p,p));
	p = Point3(w2, -l2, 0.0f);
	spline->AddKnot(SplineKnot(KTYPE_CORNER,LTYPE_CURVE,p,p,p));
	spline->SetClosed();
	spline->ComputeBezPoints();
	for(int i = 0; i < 4; ++i)
		spline->SetKnotType(i, KTYPE_BEZIER_CORNER);
	}
	spline->SetClosed();
	spline->ComputeBezPoints();
	for(int i = 0; i < pts; ++i)
		spline->SetKnotType(i, KTYPE_BEZIER_CORNER);
	ashape.UpdateSels();	// Make sure it readies the selection set info
	ashape.InvalidateGeomCache();
	}

RectangleObject::RectangleObject() : SimpleSpline() 
	{
	ReadyInterpParameterBlock();		// Build the interpolations parameter block in SimpleSpline
	ReplaceReference(USERPBLOCK, CreateParameterBlock(descVer1, PBLOCK_LENGTH, CURRENT_VERSION));
	assert(pblock);
	
	pblock->SetValue(PB_LENGTH,0,crtLength);
	pblock->SetValue(PB_WIDTH,0,crtWidth);	
	pblock->SetValue(PB_FILLET,0,crtFillet);	
 	}

RectangleObject::~RectangleObject()
	{
	DeleteAllRefsFromMe();
	pblock = NULL;
	UnReadyInterpParameterBlock();
	}

class RectangleObjCreateCallBack: public CreateMouseCallBack {
	RectangleObject *ob;
	Point3 p0,p1,p2,edge1Normal,edge1Perp,edge1Center;
	IPoint2 sp0,sp1,sp2;
	float w,l;
	int createType;
	public:
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat );
		void SetObj(RectangleObject *obj) { ob = obj; }
	};

int RectangleObjCreateCallBack::proc(ViewExp* vpt, int msg, int point, int flags, IPoint2 m, Matrix3& mat)
	{
		if (!vpt || !vpt->IsAlive())
		{
			// why are we here
			DbgAssert(!_T("Invalid viewport!"));
			return FALSE;
		}

		Point3 d;

		DWORD snapdim = SNAP_IN_3D;

		if (msg == MOUSE_FREEMOVE)
		{
			vpt->SnapPreview(m, m, NULL, snapdim);
		}

		if (msg == MOUSE_POINT || msg == MOUSE_MOVE)
		{
			// Ordinarily we don't want to display the angle
			ob->mDisplayShapeAngle = false;
			Matrix3 tm = vpt->getGW()->getTransform();
			switch (point)
			{
			case 0:
				sp0 = m;
				ob->suspendSnap = TRUE;
				createType = ob->dlgCreateMeth;
				p0 = vpt->SnapPoint(m, m, NULL, snapdim);
				p1 = p0 + Point3(.01, .01, .0);
				if (createType == CREATE_EDGE || createType == CREATE_3POINT)
					mat.SetTrans(float(.5) * (p0 + p1));
				else
					mat.SetTrans(p0);
				break;
			case 1: {
				sp1 = m;
				p1 = vpt->SnapPoint(m, m, NULL, snapdim);
				p1.z = p0.z;
				// If shape angle constraints are in use, constrain the snapped point
				if (createType == CREATE_3POINT && GetEnableShapeAngleConstraints() && (flags & MOUSE_SHIFT))
				{
					Point3 p0p1 = p1 - p0;
					float angIn = std::acos(Normalize(p0p1) % Point3(0, 1, 0));
					Point3 cross = p0p1 ^ Point3(0, 1, 0);
					if (cross.z > 0.0f)
						angIn = -angIn;
					float degIn = angIn * RAD_TO_DEG;
					float snapRad = GetShapeAngleConstraint() * DEG_TO_RAD;
					float half = snapRad * 0.5f;
					float angOut = 0.0f;
					if (angIn < 0.0f)
						angOut = -((float)floor(((-angIn) + half) / snapRad) * snapRad);
					else
						angOut = (float)floor((angIn + half) / snapRad) * snapRad;

					// Compute the snapped point location
					p1 = p0 + Point3(0.0f, Length(p0p1), 0.0f) * RotateZMatrix(angOut);

					// Get the degree value to display
					ob->mShapeAngleValue = angOut * RAD_TO_DEG;

					// We want to get the display coordinates per the construction plane
					GraphicsWindow* gw = vpt->getGW();
					Matrix3 tmHold = gw->getTransform();
					Matrix3 workTM;
					vpt->GetConstructionTM(workTM);
					gw->setTransform(workTM);

					// Get the graphics window coordinates we'll use for the text, slightly to the right and
					// above the endpoint
					IPoint3 out;
					gw->wTransPoint(&p1, &out);
					ob->mShapeAnglePosition = IPoint2(out.x + 10, out.y - 10);

					// Let the display know we want the angle displayed alongside the endpoint
					ob->mDisplayShapeAngle = true;

					// Restore the original transform
					gw->setTransform(tmHold);
				}
				d = p1 - p0;
				edge1Normal = Normalize(d);
				edge1Center = (p0 + p1) * 0.5f;
				if (createType == CREATE_3POINT)
				{
					l = Length(d);

					// Get a perpendicular vector
					edge1Perp = Point3(-edge1Normal.y, edge1Normal.x, 0.0f);

					// Get a transform where this vector is aligned with +y
					Point3 normal = Normalize(d);
					float dot = normal % Point3(0, 1, 0);
					Matrix3 positiveYTransform;
					Point3 axis = Normalize(normal ^ Point3(0, 1, 0));
					Quat theQuaternion = QFromAngAxis(std::acos(dot), axis);
					theQuaternion.MakeMatrix(positiveYTransform);
					mat = positiveYTransform;
					mat.SetTrans(edge1Center);
					ob->pblock->SetValue(PB_WIDTH, 0, 0.0001f); // Minimal width so we can see the shape
					ob->pblock->SetValue(PB_LENGTH, 0, l);
					ob->pmapParam->Invalidate();
					edge1Perp = Point3(-edge1Normal.y, edge1Normal.x, 0.0f);
					if (msg == MOUSE_POINT && (Length(m - sp0) < 3))
					{
						ob->suspendSnap = FALSE;
						return CREATE_ABORT;
					}
				}
				else
				{
					w = float(fabs(d.x));
					l = float(fabs(d.y));
					if (flags & MOUSE_CTRL)
					{
						if (createType == CREATE_EDGE)
						{
							float ysign = (d.y < 0.0f) ? -1.0f : 1.0f;
							mat.SetTrans(float(.5) * (p0 + Point3(p1.x, p0.y + ysign * w, 0.0f)));
						}
						else
						{
							mat.SetTrans(p0);
							w = w * 2.0f;
						}
						ob->pblock->SetValue(PB_LENGTH, 0, w);
						ob->pblock->SetValue(PB_WIDTH, 0, w);
					}
					else
					{
						if (createType == CREATE_EDGE)
							mat.SetTrans(edge1Center);
						else
						{
							mat.SetTrans(p0);
							w = w * 2.0f;
							l = l * 2.0f;
						}
						ob->pblock->SetValue(PB_WIDTH, 0, w);
						ob->pblock->SetValue(PB_LENGTH, 0, l);
					}
					ob->pmapParam->Invalidate();
					if (msg == MOUSE_POINT)
					{
						ob->suspendSnap = FALSE;
						return ((Length(m - sp0) < 3) ? CREATE_ABORT : CREATE_STOP);
					}
				}
			}
			break;
			case 2: {
				sp2 = m;
				p2 = vpt->SnapPoint(m, m, NULL, snapdim);
				p2.z = p0.z;
				// Figure out which side of the established edge this point is on...
				Point3 edge2Normal = Normalize(p2 - p0);
				Point3 cross = edge1Normal ^ edge2Normal;
				float sign = (cross.z < 0.0f) ? -1.0f : 1.0f;
				if (flags & MOUSE_CTRL)
				{
					// Constrain to square -- width set to length
					w = l;
				}
				else
				{
					// Compute this point's distance from the established edge; that will be the width
					float x1 = p1.x - p0.x;
					float y1 = p1.y - p0.y;
					float rpx = p2.x - p0.x;
					float rpy = p2.y - p0.y;
					float a2 = rpy * x1 - rpx * y1; // 2* area of triangle
					float x1Sq_plus_y1Sq = x1 * x1 + y1 * y1;
					w = std::sqrt(a2 * a2 / x1Sq_plus_y1Sq);
				}
				ob->pblock->SetValue(PB_WIDTH, 0, w);
				
				// Compute center for pivot
				mat.SetTrans(edge1Center - edge1Perp * -sign * w * .5f);

				if (msg == MOUSE_POINT)
				{
					ob->suspendSnap = FALSE;
					return ((Length(m - sp1) < 3) ? CREATE_ABORT : CREATE_STOP);
				}
			}
			}
		}
		else if (msg == MOUSE_ABORT)
		{
			return CREATE_ABORT;
		}

		return TRUE;
	}

static RectangleObjCreateCallBack rectangleCreateCB;

CreateMouseCallBack* RectangleObject::GetCreateMouseCallBack() {
	rectangleCreateCB.SetObj(this);
	return(&rectangleCreateCB);
	}

//
// Reference Managment:
//

RefTargetHandle RectangleObject::Clone(RemapDir& remap) {
	RectangleObject* newob = new RectangleObject();
	newob->SimpleSplineClone(this, remap);
	newob->ReplaceReference(USERPBLOCK,remap.CloneRef(pblock));	
	newob->ivalid.SetEmpty();	
	BaseClone(this, newob, remap);
	return(newob);
	}

BOOL RectangleObject::ValidForDisplay(TimeValue t) {
	float length, width;
	pblock->GetValue(PB_LENGTH, t, length, ivalid);
	pblock->GetValue(PB_WIDTH, t, width, ivalid);
	return (length == 0 || width == 0) ? FALSE : TRUE;
	}

int RectangleObject::Display(TimeValue t, INode* inode, ViewExp* vpt, int flags)
{
	// Call the base class to display the bulk of this shape...
	int result = SimpleSpline::Display(t, inode, vpt, flags);
	// If we need to display our angle information, do that now
	if (mDisplayShapeAngle)
	{
		// Draw the text
		GraphicsWindow* gw = vpt->getGW();
		MSTR angleString;
		angleString.printf(_T("%.1f\xB0"), mShapeAngleValue);
		IPoint3 pos(mShapeAnglePosition.x, mShapeAnglePosition.y, 0.0f);
		gw->wText(&pos, angleString);
		return 1;
	}
	return result;
}

ParamDimension *RectangleObject::GetParameterDim(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_LENGTH:
		case PB_WIDTH:
		case PB_FILLET:
			return stdWorldDim;			
		default:
			return defaultDim;
		}
	}

TSTR RectangleObject::GetParameterName(int pbIndex, bool localized)
	{
	switch (pbIndex) {
		case PB_LENGTH: return localized ? GetString(IDS_RB_LENGTH) : _T("Length");
		case PB_WIDTH:  return localized ? GetString(IDS_RB_WIDTH)  : _T("Width");
		case PB_FILLET: return localized ? GetString(IDS_AP_FILLET) : _T("Corner Radius");
		default:        return _T("");
		}
	}

// From ParamArray
BOOL RectangleObject::SetValue(int i, TimeValue t, int v) 
	{
	switch (i) {
		case PB_CREATEMETHOD: dlgCreateMeth = v; break;
		}		
	return TRUE;
	}

BOOL RectangleObject::SetValue(int i, TimeValue t, float v)
	{
	switch (i) {				
		case PB_TI_LENGTH: crtLength = v; break;
		case PB_TI_WIDTH: crtWidth = v; break;
		case PB_TI_FILLET: crtFillet = v; break;
		}	
	return TRUE;
	}

BOOL RectangleObject::SetValue(int i, TimeValue t, Point3 &v) 
	{
	switch (i) {
		case PB_TI_POS: crtPos = v; break;
		}		
	return TRUE;
	}

BOOL RectangleObject::GetValue(int i, TimeValue t, int &v, Interval &ivalid) 
	{
	switch (i) {
		case PB_CREATEMETHOD: v = dlgCreateMeth; break;
		}
	return TRUE;
	}

BOOL RectangleObject::GetValue(int i, TimeValue t, float &v, Interval &ivalid) 
	{	
	switch (i) {		
		case PB_TI_LENGTH: v = crtLength; break;
		case PB_TI_WIDTH: v = crtWidth; break;
		case PB_TI_FILLET: v = crtFillet; break;
		}
	return TRUE;
	}

BOOL RectangleObject::GetValue(int i, TimeValue t, Point3 &v, Interval &ivalid) 
	{	
	switch (i) {		
		case PB_TI_POS: v = crtPos; break;		
		}
	return TRUE;
	}

