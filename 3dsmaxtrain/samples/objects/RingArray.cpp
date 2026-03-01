//**************************************************************************/
// Copyright (c) 2006 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
// DESCRIPTION: Defines a ring array system object
// AUTHOR: Unknown - created Nov.11.1994
//***************************************************************************/

#include "prim.h"
#include <props.h>
#include <dummy.h>
#include <Simpobj.h>
#include <notify.h>
#include <IIndirectRefTargContainer.h>
#include <3dsmaxport.h>
#include "MouseCursors.h"

// External declarations
extern HINSTANCE hInstance;

// Constants
#define DRIVEN_CONTROL_CLASS_ID 0x9100
#define IID_RING_ARRAY_DRIVER Interface_ID(0x638c6c52, 0xc140d40)

#define DECL_FUNC_NO_INTERVAL_1(funcName)\
	auto funcName(TimeValue t, Interval&& valid = FOREVER) { return funcName(t, valid); }

//------------------------------------------------------------------------------
class RingDriver: public ReferenceTarget, public FPMixinInterface 
{
	public:
		RingDriver(int numNodes);
		~RingDriver();

		// Methods for retrieving\setting the i-th node of the system. 
		// i is a zero based index.
		INode* GetDrivenNode(int i);
		void SetDrivenNode(int i, INode * node);
		// Methods for accessing the ring array's parameters 
		void SetNum(TimeValue t, int n, BOOL notify=TRUE); 
		void SetRad(TimeValue t, float r); 
		void SetCyc(TimeValue t, float r); 
		void SetAmp(TimeValue t, float r); 
		void SetPhs(TimeValue t, float r); 
		int GetNum(TimeValue t, Interval& valid);
		DECL_FUNC_NO_INTERVAL_1(GetNum)
		float GetRad(TimeValue t, Interval& valid);
		DECL_FUNC_NO_INTERVAL_1(GetRad)
		float GetCyc(TimeValue t, Interval& valid);
		DECL_FUNC_NO_INTERVAL_1(GetCyc)
		float GetAmp(TimeValue t, Interval& valid);
		DECL_FUNC_NO_INTERVAL_1(GetAmp)
		float GetPhs(TimeValue t, Interval& valid);
		DECL_FUNC_NO_INTERVAL_1(GetPhs)

		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method, int id);
		void UpdateUI(TimeValue t);
		void UpdateAnimKeyBrackets(TimeValue t, int pbIndex);

		// From Animatable
		int NumSubs()  { return 1; }
		Animatable* SubAnim(int i) { return mPBlock; }
		TSTR SubAnimName(int i, bool localized) override { return localized ? GetString(IDS_DS_RINGARRAYPAR) : _T("Ring Array Parameters");}
		Class_ID ClassID() { return Class_ID(RINGARRAY_CLASS_ID,0); }
		SClass_ID SuperClassID() { return SYSTEM_CLASS_ID; }
		void GetClassName(MSTR& s, bool localized) const override { s = localized ? GetString(IDS_DB_RING_ARRAY_CLASS) : _T("Ring Array"); }
		void DeleteThis() { delete this; }		
		void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev );
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next );
		void GetSystemNodes(INodeTab &nodes, SysNodeContext ctxt);

		// From Reference Target
		RefTargetHandle Clone(RemapDir& remap);
		int NumRefs() { return kNUM_REFS;	};
		RefTargetHandle GetReference(int i);
private:
		virtual void SetReference(int i, RefTargetHandle rtarg);
public:

		// IO
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);

		// From ReferenceMaker
		RefResult NotifyRefChanged(const Interval&, RefTargetHandle, PartID&, RefMessage, BOOL);
		
		// --- BaseInterface methods
		virtual BaseInterface*	GetInterface(Interface_ID);
		virtual void						DeleteInterface() { delete this; }

		// --- From FPInterface
		virtual FPInterfaceDesc* GetDesc();
		virtual FPInterfaceDesc* GetDescByID(Interface_ID id);
		
	public:
		// --- Function publishing 
		enum FPEnums 	{ 
			kfpSystemNodeContexts,
		};

		enum FPIDs { 
		 kRM_GetNodes,
		 kRM_GetNode,
		 kRM_GetNodeNum, kRM_SetNodeNum,
		 kRM_GetCycle, kRM_SetCycle,
		 kRM_GetAmplitute, kRM_SetAmplitude,
		 kRM_GetRadius, kRM_SetRadius,
		 kRM_GetPhase, kRM_SetPhase,
		}; 
		
		BEGIN_FUNCTION_MAP
			FN_2(kRM_GetNodes, TYPE_INT, fpGetNodes, TYPE_INODE_TAB_BR, TYPE_ENUM);
			// use TYPE_INDEX because Maxscript has 1 based arrays
			FN_1(kRM_GetNode, TYPE_INODE, GetDrivenNode, TYPE_INDEX);
			PROP_FNS(kRM_GetNodeNum, fpGetNodeNum, kRM_SetNodeNum, fpSetNodeNum, TYPE_INT);
			PROP_FNS(kRM_GetCycle, fpGetCycle, kRM_SetCycle, fpSetCycle, TYPE_FLOAT);
			PROP_FNS(kRM_GetPhase, fpGetPhase, kRM_SetPhase, fpSetPhase, TYPE_FLOAT);
			PROP_FNS(kRM_GetAmplitute, fpGetAmplitude, kRM_SetAmplitude, fpSetAmplitude, TYPE_FLOAT);
			PROP_FNS(kRM_GetRadius, fpGetRadius, kRM_SetRadius, fpSetRadius, TYPE_FLOAT);
		END_FUNCTION_MAP

		int fpGetNodes(Tab<INode*>& systemNodes, unsigned int context) {
			GetSystemNodes(static_cast<INodeTab&>(systemNodes), static_cast<SysNodeContext>(context));
			return systemNodes.Count();
		}

		int fpGetNodeNum() {
			return GetNum(GetCOREInterface()->GetTime());
		}
		void fpSetNodeNum(int nodeNum) {
			SetNum(GetCOREInterface()->GetTime(), nodeNum);
		}
		float fpGetCycle() {
			return GetCyc(GetCOREInterface()->GetTime());
		}
		void fpSetCycle(float cycle) {
			SetCyc(GetCOREInterface()->GetTime(), cycle);
		}
		float fpGetPhase() {
			return GetPhs(GetCOREInterface()->GetTime());
		}
		void fpSetPhase(float phase) {
			SetPhs(GetCOREInterface()->GetTime(), phase);
		}
		float fpGetAmplitude() {
			return GetAmp(GetCOREInterface()->GetTime());
		}
		void fpSetAmplitude(float amplitude) {
			SetAmp(GetCOREInterface()->GetTime(), amplitude);
		}
		float fpGetRadius() {
			return GetRad(GetCOREInterface()->GetTime());
		}
		void fpSetRadius(float radius) {
			SetRad(GetCOREInterface()->GetTime(), radius);
		}


	public:
		// Parameter block indices
		enum EParamBlkIdx 
		{
			PB_RAD = 0,
			PB_CYC = 1,
			PB_AMP = 2,
			PB_PHS = 3,
		};
		// Class vars
		HWND hDriverParams;
		static IObjParam *iObjParams;
		static int dlgNum;
		static float dlgRadius;
		static float dlgAmplitude;
		static float dlgCycles;
		static float dlgPhase;
		ISpinnerControl *numSpin;
		ISpinnerControl *radSpin;
		ISpinnerControl *ampSpin;
		ISpinnerControl *cycSpin;
		ISpinnerControl *phsSpin;

		// Default parameter values
		static const int kDEF_NODE_NUM, kMIN_NODE_NUM, kMAX_NODE_NUM;
		static const float kDEF_RADIUS, kMIN_RADIUS, kMAX_RADIUS;
		static const float kDEF_AMPLITUDE, kMIN_AMPLITUDE, kMAX_AMPLITUDE;
		static const float kDEF_CYCLES, kMIN_CYCLES, kMAX_CYCLES;
		static const float kDEF_PHASE, kMIN_PHASE, kMAX_PHASE;

	private:
		// List of node ptr loaded from legacy files
		Tab<INode*> mLegacyNodeTab;
		// Loads files that are considered legacy
		IOResult DoLoadLegacy(ILoad *iload);
		// Loads non-legacy files
		IOResult DoLoad(ILoad *iload);

		enum eReferences 
		{
			kREF_PARAM_BLK = 0,
			kREF_NODE_CONTAINER = 1,
			kNUM_REFS,
		};

		// References
		IParamBlock* mPBlock;
		IIndirectRefTargContainer* mNodes;

		// Function publishing
		static FPInterfaceDesc mFPInterfaceDesc;

		// File IO related const
		static const USHORT kNUMNODES_CHUNK = 0x100;
		static const USHORT kNODE_ID_CHUNK = 0x110;
		static const USHORT kVERSION_CHUNK = 0x120;
		// Version number, saved into max file
		static const unsigned short kVERSION_01	= 1;
		static const unsigned short kVERSION_CURRENT = kVERSION_01;
		unsigned short mVerLoaded;

	private:
		class RingDriverPLC;
		class NodeNumRestore;

	friend RingDriverPLC;
	friend NodeNumRestore;
};

//------------------------------------------------------------------------------
// PostLoadCallback
//------------------------------------------------------------------------------
class RingDriver::RingDriverPLC : public PostLoadCallback
{
	public:
		RingDriverPLC(RingDriver& ringDriver) : mRingDriver(ringDriver) { }
		void proc(ILoad *iload) 
		{
			for (int i = 0; i<mRingDriver.mLegacyNodeTab.Count(); ++i) {
				mRingDriver.SetDrivenNode(i, mRingDriver.mLegacyNodeTab[i]);
			}
			delete this;
		}

	private:
		RingDriver& mRingDriver;
};

//------------------------------------------------------------------------------
// Undo \ redo
//------------------------------------------------------------------------------

class RingDriver::NodeNumRestore: public RestoreObj 
{
	const RingDriver& mRingDriver;
	// Number of nodes before change (undo changes current node num to this value)
	int mPrevNodeNum;
	// Number of nodes after change (redo changes current node num to this value)
	int mNewNodeNum;

	public:
		NodeNumRestore(const RingDriver& ringDriver, int newNodeNum); 
		~NodeNumRestore() {}
		void Restore(int isUndo);
		void Redo();
		TSTR Description() { return _T("NodeNumRestore"); }
};

RingDriver::NodeNumRestore::NodeNumRestore(
	const RingDriver& ringDriver, 
	int newNodeNum)
: mRingDriver(ringDriver)
{ 
	mPrevNodeNum = mRingDriver.mNodes->GetNumItems(); 
	mNewNodeNum = newNodeNum;
}

void RingDriver::NodeNumRestore::Restore(int isUndo) 
{
	if (mRingDriver.hDriverParams && mRingDriver.numSpin) {
		mRingDriver.numSpin->SetValue(mPrevNodeNum, FALSE );
	}
	const_cast<RingDriver&>(mRingDriver).NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
}

void RingDriver::NodeNumRestore::Redo() 
{	
	if (mRingDriver.hDriverParams && mRingDriver.numSpin) {
		mRingDriver.numSpin->SetValue(mNewNodeNum, FALSE );
	}
	const_cast<RingDriver&>(mRingDriver).NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
}

//------------------------------------------------------------------------------
// Ring Driver
//-------------------------------------------------------------------------------
Control* GetNewDrivenControl(RingDriver *driver, int i);

//------------------------------------------------------------------------------
class RingDriverClassDesc:public ClassDesc 
{
	public:
	int IsPublic() { return 1; }
	void*	Create(BOOL loading = FALSE) { 
		return new RingDriver((loading ? 0 : RingDriver::dlgNum)); 
	}
	const TCHAR*	ClassName() { return GetString(IDS_DB_RING_ARRAY_CLASS); }
	const TCHAR*	NonLocalizedClassName() { return _T("Ring Array"); }
	int BeginCreate(Interface *i);
	int EndCreate(Interface *i);
	SClass_ID SuperClassID() { return SYSTEM_CLASS_ID; }
	Class_ID ClassID() { return Class_ID(RINGARRAY_CLASS_ID,0); }
	const TCHAR* Category() { return _T("");  }
};
static RingDriverClassDesc theRingDriverClassDesc;

ClassDesc* GetRingDriverDesc() { return &theRingDriverClassDesc; }

//------------------------------------------------------------------------------
const int RingDriver::kDEF_NODE_NUM = 4;
const int RingDriver::kMIN_NODE_NUM = 1;
const int RingDriver::kMAX_NODE_NUM = 200;

const float RingDriver::kDEF_RADIUS = 100.0f;
const float RingDriver::kMIN_RADIUS = 0.0f;
const float RingDriver::kMAX_RADIUS = 500.0f;

const float RingDriver::kDEF_AMPLITUDE = 20.0f;
const float RingDriver::kMIN_AMPLITUDE = 0.0f;
const float RingDriver::kMAX_AMPLITUDE = 500.0f;

const float RingDriver::kDEF_CYCLES = 3.0f;
const float RingDriver::kMIN_CYCLES = 0.0f;
const float RingDriver::kMAX_CYCLES = 10.0f;

const float RingDriver::kDEF_PHASE = 1.0f;
const float RingDriver::kMIN_PHASE = -1000.0f;
const float RingDriver::kMAX_PHASE = 1000.0f;


IObjParam *RingDriver::iObjParams = NULL;

int RingDriver::dlgNum = 	RingDriver::kDEF_NODE_NUM;
float RingDriver::dlgRadius = RingDriver::kDEF_RADIUS;
float RingDriver::dlgAmplitude = RingDriver::kDEF_AMPLITUDE;
float RingDriver::dlgCycles = RingDriver::kDEF_CYCLES;
float RingDriver::dlgPhase = RingDriver::kDEF_PHASE;

//------------------------------------------------------------------------------
RingDriver::RingDriver(int numNodes = 0) : mNodes(NULL), mPBlock(NULL), hDriverParams(NULL),
numSpin(NULL), radSpin(NULL), ampSpin(NULL), cycSpin(NULL), phsSpin(NULL)

{
	ParamBlockDesc desc[] = {
		{ TYPE_FLOAT, NULL, TRUE },
		{ TYPE_FLOAT, NULL, TRUE },
		{ TYPE_FLOAT, NULL, TRUE },
		{ TYPE_FLOAT, NULL, TRUE }
	};

	ReplaceReference(kREF_PARAM_BLK, CreateParameterBlock( desc, 4 ));
	IIndirectRefTargContainer* nodeMonCont = 
		reinterpret_cast<IIndirectRefTargContainer*>(GetCOREInterface()->CreateInstance(
		REF_TARGET_CLASS_ID, INDIRECT_REFTARG_CONTAINER_CLASS_ID));
	ReplaceReference(kREF_NODE_CONTAINER, nodeMonCont);

	SetRad( TimeValue(0), dlgRadius );
	SetCyc( TimeValue(0), dlgCycles );
	SetAmp( TimeValue(0), dlgAmplitude );
	SetPhs( TimeValue(0), dlgPhase );

	if (numNodes > 0) 
	{
		HoldSuspend holdSuspend;
		mNodes->SetItem(numNodes-1, NULL);
	}
}

RefTargetHandle RingDriver::Clone(RemapDir& remap) 
{
  RingDriver* newm = new RingDriver(0);	
	newm->ReplaceReference(kREF_PARAM_BLK, remap.CloneRef(mPBlock));
	newm->ReplaceReference(kREF_NODE_CONTAINER, remap.CloneRef(mNodes));

	BaseClone(this, newm, remap);
	return(newm);
}

RingDriver::~RingDriver() 
{
}

void RingDriver::UpdateUI(TimeValue t)
{
	if (hDriverParams) 
	{
		radSpin->SetValue( GetRad(t), FALSE );
		ampSpin->SetValue( GetAmp(t), FALSE );
		cycSpin->SetValue( GetCyc(t), FALSE );
		phsSpin->SetValue( GetPhs(t), FALSE );
		numSpin->SetValue( GetNum(t), FALSE );
		UpdateAnimKeyBrackets(t, PB_RAD);
		UpdateAnimKeyBrackets(t, PB_AMP);
		UpdateAnimKeyBrackets(t, PB_CYC);
		UpdateAnimKeyBrackets(t, PB_PHS);
	}
}

void RingDriver::UpdateAnimKeyBrackets(TimeValue t, int i)
{
	switch (i)
	{
		case PB_RAD:
			radSpin->SetKeyBrackets(mPBlock->KeyFrameAtTime(PB_RAD,t));
		break;
		
		case PB_AMP:
			ampSpin->SetKeyBrackets(mPBlock->KeyFrameAtTime(PB_AMP,t));
		break;
		
		case PB_CYC:
			cycSpin->SetKeyBrackets(mPBlock->KeyFrameAtTime(PB_CYC,t));
		break;
		
		case PB_PHS:
			phsSpin->SetKeyBrackets(mPBlock->KeyFrameAtTime(PB_PHS,t));
		break;
		
		default:
			DbgAssert(false);
			break;
	}
}

INode* RingDriver::GetDrivenNode(int i) 
{ 
	if (mNodes != NULL) {
		ReferenceTarget* refTarg = mNodes->GetItem(i);
		if (refTarg != NULL) {
			return refTarg->GetTypedInterface<INode>();
		}
	}
	return NULL;
}

void RingDriver::SetDrivenNode(int i, INode * node) 
{
	if (mNodes != NULL) {
		mNodes->SetItem(i, node);
	}
}


void RingDriver::GetSystemNodes(INodeTab &nodes, SysNodeContext ctxt)
{
	switch (ctxt) 
	{
		case kSNCClone:
		case kSNCFileMerge:
		case kSNCFileSave:
		case kSNCDelete:
		{
			if (mNodes != NULL)
			{
				int nodeCount = mNodes->GetNumItems();
				for (int i = 0; i < nodeCount; i++) 
				{
					ReferenceTarget* refTarg = mNodes->GetItem(i);
					INode* node = NULL;
					if (refTarg != NULL) {
						node = refTarg->GetTypedInterface<INode>();
					}
					nodes.Append(1, &node);
				}
			}
		}
		break;
		
		default:
		break;
	}
}

RefTargetHandle RingDriver::GetReference(int i)  
{ 
	if (kREF_PARAM_BLK == i) {
		return mPBlock;
	}
	else if (kREF_NODE_CONTAINER == i) {
		return mNodes;
	}
	return NULL;
}


void RingDriver::SetReference(int i, RefTargetHandle rtarg) 
{
	if (kREF_PARAM_BLK == i) {
		mPBlock = static_cast<IParamBlock*>(rtarg); 
	}
	else if (kREF_NODE_CONTAINER == i) {
		mNodes = static_cast<IIndirectRefTargContainer*>(rtarg);
	}
}		

#define TWO_PI 6.283185307f

// This is the crux of the controller: it takes an input (parent) matrix, modifies
// it accordingly.  
void RingDriver::GetValue(
	TimeValue t, 
	void *val, 
	Interval &valid, 
	GetSetMethod method,
	int id) 
{
	float radius = 0.0f, amplitude = 0.0f, cycles = 0.0f, phase = 0.0f;
	radius = GetRad(t, valid);
	amplitude = GetAmp(t, valid);
	cycles = GetCyc(t, valid);
	phase = GetPhs(t, valid);
	Matrix3 tmat, *mat = (Matrix3*)val;
	tmat.IdentityMatrix();
	float ang = float(id)*TWO_PI/(float)GetNum(t, valid);
	tmat.Translate(Point3(radius, 0.0f, amplitude*(float)cos(cycles*ang + TWO_PI*phase)));
	tmat.RotateZ(ang);
	
	(*mat) = (method == CTRL_RELATIVE) ? tmat*(*mat) : tmat;

	// Make sure spinners track when animating and in Motion Panel
	UpdateUI(t);
}

RefResult RingDriver::NotifyRefChanged(
	const Interval& changeInt, 
	RefTargetHandle hTarget, 
	PartID& partID, 
	RefMessage message, 
	BOOL propagate ) 
{
	switch (message) 
	{
		case REFMSG_CHANGE:
			if(iObjParams)
				UpdateUI(iObjParams->GetTime());
			break;
		case REFMSG_GET_PARAM_DIM:
		{ 
			// the ParamBlock needs info to display in the tree view
			GetParamDim *gpd = (GetParamDim*)partID;
			switch (gpd->index)
			{
				case PB_RAD:
					gpd->dim = stdWorldDim;
				break;
				case PB_CYC:
					gpd->dim = stdWorldDim;
				break;
				case PB_AMP:
					gpd->dim = stdWorldDim;
				break;
				case PB_PHS:
					gpd->dim = stdWorldDim;
				break;
				default:
					DbgAssert(false);
				break;
			}
			return REF_HALT;
		}
		break;

		case REFMSG_GET_PARAM_NAME_LOCALIZED:
		{
			// the ParamBlock needs info to display in the tree view
			GetParamName *gpn = (GetParamName*)partID;
			switch (gpn->index) 
			{
				case PB_RAD: gpn->name = GetString(IDS_DS_RADIUS);    break;
				case PB_CYC: gpn->name = GetString(IDS_DS_CYCLES);    break;
				case PB_AMP: gpn->name = GetString(IDS_DS_AMPLITUDE); break;
				case PB_PHS: gpn->name = GetString(IDS_DS_PHASE);     break;
				default:     DbgAssert(false);                        break;
			}
			return REF_HALT;
		}
		break;

		case REFMSG_GET_PARAM_NAME_NONLOCALIZED:
		{
			// the ParamBlock needs info to display in the tree view
			GetParamName *gpn = (GetParamName*)partID;
			switch (gpn->index) 
			{
				case PB_RAD: gpn->name = _T("Radius");    break;
				case PB_CYC: gpn->name = _T("Cycles");    break;
				case PB_AMP: gpn->name = _T("Amplitude"); break;
				case PB_PHS: gpn->name = _T("Phase");     break;
				default:     DbgAssert(false);            break;
			}
			return REF_HALT;
		}
		break;

	} // end switch

	return (REF_SUCCEED);
}

void RingDriver::SetNum(TimeValue t, int n, BOOL notify) 
{ 
	if (n < 1 || n == GetNum(t)) {
		return;
	}

	if (mNodes != NULL && mNodes->GetNumItems() > 0) 
	{
		if (theHold.Holding()) {
			theHold.Put(new NodeNumRestore(*this, n));
		}

		int nodeCount = mNodes->GetNumItems();
		if (n < nodeCount) 
		{
			// remove nodes;
			for (int i = nodeCount-1; i >= n; i--) 
			{
				ReferenceTarget* refTarg = mNodes->GetItem(i);
				// If the user is in create mode, and has not created any system nodes yet
				// the reftarget will be NULL. In this case, we still need to shrink the node array
				if (refTarg != NULL) 
				{
					INode* node = refTarg->GetTypedInterface<INode>();
					if (node) 
					{
						if (node->Selected()) 
						{
							// Dont want to delete selected nodes, so just stop and update the
							// spinner in the UI. We won't be called recursively, since the number 
							// of nodes has been updated already
							n = i+1;
							if (hDriverParams) 
							{
								DbgAssert(numSpin != NULL);
								numSpin->SetValue(n, TRUE);
							}
							break;
						}
						node->Delete(t, TRUE);
					}
				}
				mNodes->RemoveItem(i);
			}
		}
		else 
		{
			// add (indirect refs to) nodes
			mNodes->SetItem(n-1, NULL);

			// Create the new nodes
			if (mNodes->GetItem(0) != NULL)
			{
				ReferenceTarget* refTarg = mNodes->GetItem(0);
				INode* firstNode = refTarg->GetTypedInterface<INode>();
				DbgAssert(firstNode != NULL);
				Object* obj = firstNode->GetObjectRef();
				DbgAssert(obj);
				INode* parentNode = firstNode->GetParentNode();
				DbgAssert(parentNode != NULL);
				Interface* ip = GetCOREInterface();
				for (int i = nodeCount; i < n; i++) 
				{
					INode* newNode = ip->CreateObjectNode(obj);
					Control* driven = GetNewDrivenControl(this, i);
					newNode->SetTMController(driven);
					newNode->FlagForeground(t, FALSE);
					parentNode->AttachChild(newNode);
					SetDrivenNode(i, newNode);
				}
			}
		}
	}
	DbgAssert(mNodes->GetNumItems() == n);

	if (notify) {
		NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
	}
}

int RingDriver::GetNum(TimeValue t, Interval& valid ) 
{ 	
	DbgAssert(mNodes != NULL);
	return mNodes->GetNumItems();
}


//------------------------------------------------------------------------------
void RingDriver::SetRad(TimeValue t, float r) 
{ 
	mPBlock->SetValue( PB_RAD, t, r );
	NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
}

float RingDriver::GetRad(TimeValue t, Interval& valid ) 
{ 	
	float f = 0.0f;
	mPBlock->GetValue( PB_RAD, t, f, valid );
	return f;
}

//--------------------------------------------------
void RingDriver::SetCyc(TimeValue t, float r) 
{ 
	mPBlock->SetValue( PB_CYC, t, r );
	NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
}

float RingDriver::GetCyc(TimeValue t, Interval& valid ) 
{ 	
	float f = 0.0f;
	mPBlock->GetValue( PB_CYC, t, f, valid );
	return f;
}

//--------------------------------------------------
void RingDriver::SetAmp(TimeValue t, float r) 
{ 
	mPBlock->SetValue( PB_AMP, t, r );
	NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
}

float RingDriver::GetAmp(TimeValue t, Interval& valid ) 
{ 	
	float f = 0.0f;
	mPBlock->GetValue( PB_AMP, t, f, valid );
	return f;
}

//--------------------------------------------------
void RingDriver::SetPhs(TimeValue t, float r) 
{ 
	mPBlock->SetValue( PB_PHS, t, r );
	NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
}

float RingDriver::GetPhs(TimeValue t, Interval& valid ) 
{ 	
	float f = 0.0f;
	mPBlock->GetValue( PB_PHS, t, f, valid );
	return f;
}

//------------------------------------------------------------------------------
INT_PTR CALLBACK DriverParamDialogProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
  RingDriver *mc = DLGetWindowLongPtr<RingDriver *>( hDlg);
	if (!mc && message != WM_INITDIALOG ) {
		return FALSE;
	}
	
	DbgAssert(mc->iObjParams);
	switch ( message ) 
	{
		case WM_INITDIALOG:
			mc = (RingDriver *)lParam;
      DLSetWindowLongPtr( hDlg, mc);
			SetDlgFont( hDlg, mc->iObjParams->GetAppHFont() );
			
			mc->radSpin  = GetISpinner(GetDlgItem(hDlg, IDC_RADSPINNER));
			mc->cycSpin  = GetISpinner(GetDlgItem(hDlg, IDC_CYCSPINNER));
			mc->ampSpin  = GetISpinner(GetDlgItem(hDlg, IDC_AMPSPINNER));
			mc->phsSpin  = GetISpinner(GetDlgItem(hDlg, IDC_PHSSPINNER));
			mc->numSpin  = GetISpinner(GetDlgItem(hDlg, IDC_NUMSPINNER));

			mc->radSpin->SetLimits(RingDriver::kMIN_RADIUS, RingDriver::kMAX_RADIUS, FALSE);
			mc->cycSpin->SetLimits(RingDriver::kMIN_CYCLES, RingDriver::kMAX_CYCLES, FALSE);
			mc->ampSpin->SetLimits(RingDriver::kMIN_AMPLITUDE, RingDriver::kMAX_AMPLITUDE, FALSE);
			mc->phsSpin->SetLimits(RingDriver::kMIN_PHASE, RingDriver::kMAX_PHASE, FALSE);
			mc->numSpin->SetLimits(RingDriver::kMIN_NODE_NUM, RingDriver::kMAX_NODE_NUM, FALSE);

			mc->radSpin->SetScale(float(0.1) );
			mc->ampSpin->SetScale(float(0.1) );
			mc->phsSpin->SetScale(float(0.1) );
			mc->numSpin->SetScale(float(0.1) );

			mc->radSpin->SetValue( mc->GetRad(mc->iObjParams->GetTime()), FALSE );
			mc->cycSpin->SetValue( mc->GetCyc(mc->iObjParams->GetTime()), FALSE );
			mc->ampSpin->SetValue( mc->GetAmp(mc->iObjParams->GetTime()), FALSE );
			mc->phsSpin->SetValue( mc->GetPhs(mc->iObjParams->GetTime()), FALSE );
			mc->numSpin->SetValue( mc->GetNum(mc->iObjParams->GetTime()), FALSE );

			mc->radSpin->LinkToEdit( GetDlgItem(hDlg,IDC_RADIUS), EDITTYPE_POS_UNIVERSE );			
			mc->cycSpin->LinkToEdit( GetDlgItem(hDlg,IDC_CYCLES), EDITTYPE_FLOAT );			
			mc->ampSpin->LinkToEdit( GetDlgItem(hDlg,IDC_AMPLITUDE), EDITTYPE_FLOAT );			
			mc->phsSpin->LinkToEdit( GetDlgItem(hDlg,IDC_PHASE), EDITTYPE_FLOAT );			
			mc->numSpin->LinkToEdit( GetDlgItem(hDlg,IDC_NUMNODES), EDITTYPE_INT );			
			
			return FALSE;	// DB 2/27

		case WM_DESTROY:
			ReleaseISpinner( mc->radSpin );
			ReleaseISpinner( mc->cycSpin );
			ReleaseISpinner( mc->ampSpin );
			ReleaseISpinner( mc->phsSpin );
			ReleaseISpinner( mc->numSpin );
			mc->radSpin = NULL;
			mc->cycSpin = NULL;
			mc->ampSpin = NULL;
			mc->phsSpin = NULL;
			mc->numSpin = NULL;
			return FALSE;

		case CC_SPINNER_CHANGE:	
		{
			if (!theHold.Holding()) {
				theHold.Begin();
			}
			TimeValue t = mc->iObjParams->GetTime();
			switch ( LOWORD(wParam) ) 
			{
				case IDC_RADSPINNER: 
					mc->SetRad(t,  mc->radSpin->GetFVal() );  
					mc->UpdateAnimKeyBrackets(t, RingDriver::PB_RAD);
					break;
				case IDC_CYCSPINNER: 
					mc->SetCyc(t,  mc->cycSpin->GetFVal() );  
					mc->UpdateAnimKeyBrackets(t, RingDriver::PB_CYC);
					break;
				case IDC_AMPSPINNER: 
					mc->SetAmp(t,  mc->ampSpin->GetFVal() );  
					mc->UpdateAnimKeyBrackets(t, RingDriver::PB_AMP);
					break;
				case IDC_PHSSPINNER: 
					mc->SetPhs(t,  mc->phsSpin->GetFVal() );  
					mc->UpdateAnimKeyBrackets(t, RingDriver::PB_PHS);
					break;
				case IDC_NUMSPINNER: 
					mc->SetNum(t,  mc->numSpin->GetIVal() );  
					break;
			}
			DbgAssert(mc->iObjParams);
			mc->iObjParams->RedrawViews(t, REDRAW_INTERACTIVE, mc);
			return TRUE;
		}

		case CC_SPINNER_BUTTONDOWN:
			theHold.Begin();
			return TRUE;

		case WM_CUSTEDIT_ENTER:
		case CC_SPINNER_BUTTONUP:
			if (HIWORD(wParam) || message == WM_CUSTEDIT_ENTER) 
				theHold.Accept(GetString(IDS_DS_PARAMCHG));
			else 
				theHold.Cancel();
			mc->iObjParams->RedrawViews(mc->iObjParams->GetTime(), REDRAW_END, mc);
			return TRUE;

		case WM_MOUSEACTIVATE:
			mc->iObjParams->RealizeParamPanel();
			return FALSE;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			mc->iObjParams->RollupMouseMessage(hDlg, message, wParam, lParam);
			return FALSE;

		case WM_COMMAND:			
			return FALSE;

		default:
			return FALSE;
		}
}

void RingDriver::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{
	iObjParams = ip;
	
	if (!hDriverParams) 
	{
		hDriverParams = ip->AddRollupPage( 
				hInstance, 
				MAKEINTRESOURCE(IDD_SAMPLEPARAM),
				DriverParamDialogProc,
				GetString(IDS_RB_PARAMETERS), 
				(LPARAM)this );		
		ip->RegisterDlgWnd(hDriverParams);
		
	} 
}
		
void RingDriver::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
{
	if (hDriverParams == NULL) {
		return;
	}
	dlgRadius   = radSpin->GetFVal();
	dlgAmplitude   = ampSpin->GetFVal();
	dlgCycles   = cycSpin->GetFVal();
	dlgPhase   = phsSpin->GetFVal();
	dlgNum   = numSpin->GetIVal();
	
	DLSetWindowLongPtr(hDriverParams, 0);
	ip->UnRegisterDlgWnd(hDriverParams);
	ip->DeleteRollupPage(hDriverParams);
	hDriverParams = NULL;
	
	iObjParams = NULL;
}

// IO
IOResult RingDriver::Save(ISave *isave) 
{
	ULONG nb;
	// Version chunck must be first 
	isave->BeginChunk(kVERSION_CHUNK);	
	unsigned short ver = kVERSION_CURRENT;
	isave->Write(&ver, sizeof(unsigned short), &nb);			
	isave->EndChunk();

	return IO_OK;
}

IOResult RingDriver::Load(ILoad *iload) 
{
	// Legacy max files do not contain a RingDriver version number
	IOResult res = IO_OK;
	USHORT next = iload->PeekNextChunkID();
	if (next != kVERSION_CHUNK)
		res = DoLoadLegacy(iload); 
	else
		res = DoLoad(iload);
	return res;
}

IOResult RingDriver::DoLoad(ILoad *iload)
{
	IOResult res;

	while (IO_OK == (res = iload->OpenChunk())) 
	{
		switch (iload->CurChunkID()) 
		{
			case kVERSION_CHUNK:
			{
				ULONG 	nb;
				iload->Read(&mVerLoaded, sizeof(unsigned short), &nb);
				if (kVERSION_CURRENT > mVerLoaded) {
					iload->SetObsolete();
				}
			}
			break;
		}
		iload->CloseChunk();
	}
	return IO_OK;
}

IOResult RingDriver::DoLoadLegacy(ILoad *iload)
{
	ULONG nb;
	IOResult res;
	int numNodes = -1;
	while (IO_OK == (res = iload->OpenChunk())) 
	{
		switch (iload->CurChunkID())  
		{
			case kNUMNODES_CHUNK: 
			{
				res = iload->Read(&numNodes, sizeof(numNodes), &nb);
				mLegacyNodeTab.SetCount(numNodes);
			}
			break;
			
			case kNODE_ID_CHUNK:
				DbgAssert(numNodes >= 0);
				ULONG id = -1;
				for (int i = 0; i<numNodes; i++) 
				{
					iload->Read(&id, sizeof(ULONG), &nb);
					if (id != 0xffffffff) {
						iload->RecordBackpatch(id, (void**)&mLegacyNodeTab[i]);
					}
				}
			break;
		}
	
		iload->CloseChunk();
		if (res != IO_OK) {
			return res;
		}
	}
	iload->SetObsolete();

	iload->RegisterPostLoadCallback(new RingDriverPLC(*this));

	return IO_OK;
}


//------------------------------------------------------------------------------
// Driven controls
//------------------------------------------------------------------------------
class DrivenControl : public Control
{
	public:		
		RingDriver *driver;
		ULONG id;

		DrivenControl(BOOL loading=FALSE) { driver = NULL; id = 0; }
		DrivenControl(const DrivenControl& ctrl);
		DrivenControl(const RingDriver* m, int i);
		void SetID( ULONG i) { id = i;}
		virtual ~DrivenControl() {}	
		DrivenControl& operator=(const DrivenControl& ctrl);

		// From Control
		void Copy(Control *from) {}
		void CommitValue(TimeValue t) {}
		void RestoreValue(TimeValue t) {}
		virtual BOOL IsLeaf() {return FALSE;}
		void GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);
		void SetValue(TimeValue t, void *val, int commit=1, GetSetMethod method=CTRL_ABSOLUTE);
		BOOL IsReplaceable() {return FALSE;}
		BOOL CanCopyAnim() {return FALSE;}

		// From Animatable
		void* GetInterface(ULONG id);
		int NumSubs()  { return driver ? driver->NumSubs() : 0; }
		Animatable* SubAnim(int i) { return driver->SubAnim(i); }
		TSTR SubAnimName(int i, bool localized) override { return driver->SubAnimName(i, localized); }
		Class_ID ClassID() { return Class_ID(DRIVEN_CONTROL_CLASS_ID,0); }  
		SClass_ID SuperClassID() { return CTRL_MATRIX3_CLASS_ID; }  
		void GetClassName(MSTR& s, bool localized) const override { s = localized ? GetString(IDS_DB_DRIVENCONTROL_CLASS) : _T("DrivenControl"); }
		void DeleteThis() { delete this; }		
		int IsKeyable(){ return 0;}
		void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev) { DbgAssert(driver); driver->BeginEditParams(ip,flags,prev); } 
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next) { DbgAssert(driver); driver->EndEditParams(ip,flags,next); } 
		// IO
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);

		// From ReferenceTarget
		RefTargetHandle Clone(RemapDir& remap);
		int NumRefs() { return 1;	};	
		RefTargetHandle GetReference(int i)  { DbgAssert(i==0); return driver; }
private:
		virtual void SetReference(int i, RefTargetHandle rtarg) { DbgAssert(i==0); driver = (RingDriver *)rtarg; }		
public:
		RefResult NotifyRefChanged(const Interval&, RefTargetHandle, PartID&, RefMessage, BOOL) {return REF_SUCCEED;}
	};



class DrivenControlClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 0; }
	void*			Create(BOOL loading = FALSE) { return new DrivenControl(); }
	const TCHAR*	ClassName() { return GetString(IDS_DB_DRIVEN_CONTROL); }
	const TCHAR*	NonLocalizedClassName() { return _T("Driven Control"); }
	SClass_ID		SuperClassID() { return CTRL_MATRIX3_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(DRIVEN_CONTROL_CLASS_ID,0); }
	const TCHAR* 	Category() { return _T("");  }
	};

static DrivenControlClassDesc drivenDesc;

ClassDesc* GetDrivenControlDesc() { return &drivenDesc; }

Control* GetNewDrivenControl(RingDriver *driver, int i) 
{
	return new DrivenControl(driver,i);
}

DrivenControl::DrivenControl(const DrivenControl& ctrl) 
{
	driver = ctrl.driver;
	id = ctrl.id;
}

DrivenControl::DrivenControl(const RingDriver* m, int i) 
{
	driver = NULL;
	id = i;
	ReplaceReference( 0, (ReferenceTarget *)m);
}

RefTargetHandle DrivenControl::Clone(RemapDir& remap) 
{
	DrivenControl *sl = new DrivenControl;
	sl->id = id;
	sl->ReplaceReference(0, remap.CloneRef(driver));
	BaseClone(this, sl, remap);
	return sl;
}

DrivenControl& DrivenControl::operator=(const DrivenControl& ctrl) 
{
	driver = ctrl.driver;
	id = ctrl.id;
	return (*this);
}

void DrivenControl::GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method) 
{
	DbgAssert(driver);
	driver->GetValue(t,val,valid,method,id);	
}


void DrivenControl::SetValue(TimeValue t, void *val, int commit, GetSetMethod method) { }

void* DrivenControl::GetInterface(ULONG id) 
{
	if (id== I_DRIVER)
		return (ReferenceTarget*)driver;
	else 
		return Control::GetInterface(id);
}

// IO
#define DRIVEN_ID_CHUNK 0x200
IOResult DrivenControl::Save(ISave *isave) 
{
	ULONG nb;
	isave->BeginChunk(DRIVEN_ID_CHUNK);
	isave->Write(&id,sizeof(id), &nb);
	isave->EndChunk();
	return IO_OK;
}

IOResult DrivenControl::Load(ILoad *iload) 
{
	ULONG nb;
	IOResult res;
	while (IO_OK == (res = iload->OpenChunk())) 
	{
		switch(iload->CurChunkID())  
		{
			case DRIVEN_ID_CHUNK:
				res = iload->Read(&id,sizeof(id), &nb);
			break;
		}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
	}
	return IO_OK;
}

//------------------------------------------------------------------------------
class RingDriverCreationManager : public MouseCallBack, ReferenceMaker 
{
	public:
		CreateMouseCallBack *createCB;	
		INode *node0;
		RingDriver *theDriver;
		IObjCreate *createInterface;
		ClassDesc *cDesc;
		Matrix3 mat;  // the nodes TM relative to the CP
		IPoint2 pt0;
		Point3 center;
		BOOL attachedToNode;
		int lastPutCount;

		void CreateNewDriver();
			
		int ignoreSelectionChange;

		virtual void GetClassName(MSTR& s, bool localized) const override { s = _M("RingDriverCreationManager"); }
		int NumRefs() { return 1; }
		RefTargetHandle GetReference(int i);
private:
		virtual void SetReference(int i, RefTargetHandle rtarg);
public:

		// StdNotifyRefChanged calls this, which can change the partID to new value 
		// If it doesnt depend on the particular message& partID, it should return
		// REF_DONTCARE
    RefResult NotifyRefChanged(
			const Interval& changeInt, 
			RefTargetHandle hTarget, 
    	PartID& partID,  
		RefMessage message, 
		BOOL propagate);

		void Begin( IObjCreate *ioc, ClassDesc *desc );
		void End();
		
		RingDriverCreationManager()	{
			ignoreSelectionChange = FALSE;
		}
		int proc( HWND hwnd, int msg, int point, int flag, IPoint2 m );
};

#define CID_BONECREATE	CID_USER + 1

class RingDriverCreateMode : public CommandMode 
{
		RingDriverCreationManager proc;
	public:
		void Begin( IObjCreate *ioc, ClassDesc *desc ) { proc.Begin( ioc, desc ); }
		void End() { proc.End(); }
		int Class() { return CREATE_COMMAND; }
		int ID() { return CID_BONECREATE; }
		MouseCallBack *MouseProc(int *numPoints) { *numPoints = 100000; return &proc; }
		ChangeForegroundCallback *ChangeFGProc() { return CHANGE_FG_SELECTED; }
		BOOL ChangeFG( CommandMode *oldMode ) { return (oldMode->ChangeFGProc() != CHANGE_FG_SELECTED); }
		void EnterMode() { SetCursor( UI::MouseCursors::LoadMouseCursor(UI::MouseCursors::Crosshair)); }
		void ExitMode() { SetCursor( LoadCursor(NULL, IDC_ARROW) ); }
		BOOL IsSticky() { return FALSE; }
};

static RingDriverCreateMode theRingDriverCreateMode;

void RingDriverCreationManager::Begin( IObjCreate *ioc, ClassDesc *desc )
{
	createInterface = ioc;
	cDesc           = desc;
	createCB        = NULL;
	node0			= NULL;
	theDriver 		= NULL;
	attachedToNode = FALSE;
	CreateNewDriver();
}

void RingDriverCreationManager::SetReference(int i, RefTargetHandle rtarg) 
{ 
	switch(i) 
	{
		case 0: 
			node0 = (INode *)rtarg; 
		break;
		default: 
			DbgAssert(0); 
		break;
	}
}

RefTargetHandle RingDriverCreationManager::GetReference(int i) 
{ 
	switch(i) 
	{
		case 0: 
			return (RefTargetHandle)node0;
		default: 
			DbgAssert(0); 
		break;
	}
	return NULL;
}

void RingDriverCreationManager::End()
{
	if (theDriver) 
	{
		BOOL bDestroy = TRUE;
		theDriver->EndEditParams( (IObjParam*)createInterface, bDestroy, NULL );
		if ( !attachedToNode ) 
		{
			theHold.Suspend(); 
			//delete lgtObject;
			theDriver->DeleteAllRefsFromMe();
			theDriver->DeleteAllRefsToMe();
			theDriver->DeleteThis(); 
			theDriver = NULL;
			theHold.Resume();


			// DS 8/21/97: If something has been put on the undo stack since this 
			// object was created, we have to flush the undo stack.
			if (theHold.GetGlobalPutCount()!=lastPutCount) {
				GetSystemSetting(SYSSET_CLEAR_UNDO);
			}
		} 
		else if ( node0 ) 
		{
		 // Get rid of the references.
			DeleteAllRefsFromMe();
		}
		theDriver = NULL; //JH 9/15/97
	}	
}

RefResult RingDriverCreationManager::NotifyRefChanged(
	const Interval& changeInt, 
	RefTargetHandle hTarget, 
	PartID& partID,  
	RefMessage message, 
	BOOL propagate) 
{
	switch (message) 
	{
		case REFMSG_TARGET_SELECTIONCHANGE:
		 	if ( ignoreSelectionChange ) {
				break;
			}
		 	if (theDriver) 
			{
				// this will set node0 ==NULL;
				DeleteAllRefsFromMe();
				goto endEdit;
			}
			else 
			{
				return REF_SUCCEED;  //JH 9.15.97 
			}
			// fall through

		case REFMSG_TARGET_DELETED:
			if (hTarget == node0)
			{
				node0 = NULL;
			}
			if (theDriver) 
			{
				endEdit:
				BOOL bDestroy = FALSE;
				theDriver->EndEditParams( (IObjParam*)createInterface, bDestroy, NULL );
				theDriver = NULL;
				node0 = NULL;
				CreateNewDriver();	
				attachedToNode = FALSE;
			}
		break;		
	} // end switch
	return REF_SUCCEED;
}

void RingDriverCreationManager::CreateNewDriver()
{
	theDriver = new RingDriver(RingDriver::dlgNum);
	
	// Start the edit params process
	theDriver->BeginEditParams( (IObjParam*)createInterface, BEGIN_EDIT_CREATE,NULL );
	lastPutCount = theHold.GetGlobalPutCount();
}

#define DUMSZ 20.0f
#define BOXSZ 20.0f

static BOOL needToss;

int RingDriverCreationManager::proc( 
	HWND hwnd,
	int msg,
	int point,
	int flag,
	IPoint2 m )
{	
	int res = TRUE;
	INode *newNode,*dummyNode;	
	float r;
	ViewExp& vpx = createInterface->GetViewExp(hwnd); 
	DbgAssert(vpx.IsAlive() );

	switch ( msg ) 
	{
		case MOUSE_POINT:
		{
			if (point==0) 
			{
				pt0 = m;	
				DbgAssert(theDriver);

				mat.IdentityMatrix();
				if ( createInterface->SetActiveViewport(hwnd) ) {
					return FALSE;
				}
				if (createInterface->IsCPEdgeOnInView()) { 
					return FALSE;
				}
				if ( attachedToNode ) 
				{
			  	// send this one on its way
			  	theDriver->EndEditParams( (IObjParam*)createInterface,0,NULL );
					
					// Get rid of the references.
					DeleteAllRefsFromMe();

					// new object
					CreateNewDriver();   // creates theDriver
				}

				needToss = theHold.GetGlobalPutCount()!=lastPutCount;

			  theHold.Begin();	 // begin hold for undo
				mat.IdentityMatrix();
				center = vpx.SnapPoint(m, m, NULL, SNAP_IN_PLANE);
				mat.SetTrans(center);

				// Create a dummy object & node
				DummyObject *dumObj = (DummyObject *)createInterface->CreateInstance(
					HELPER_CLASS_ID, Class_ID(DUMMY_CLASS_ID, 0)); 			
				DbgAssert(dumObj);					
				dummyNode = createInterface->CreateObjectNode(dumObj);
				dumObj->SetBox(Box3(Point3(-DUMSZ, -DUMSZ, -DUMSZ), Point3(DUMSZ, DUMSZ, DUMSZ)));

				// make a box object
				GenBoxObject *ob = (GenBoxObject *)createInterface->
					CreateInstance(GEOMOBJECT_CLASS_ID,Class_ID(BOXOBJ_CLASS_ID,0));
				ob->SetParams(BOXSZ,BOXSZ,BOXSZ,1,1,1,FALSE); 

				// Make a bunch of nodes, hook the box object to and a
				// driven controller of the driver control to each
				for (int i = 0; i < theDriver->GetNum(createInterface->GetTime()); i++) 
				{
					newNode = createInterface->CreateObjectNode(ob);
					DrivenControl* driven = new DrivenControl(theDriver,i);
					newNode->SetTMController(driven);
					dummyNode->AttachChild(newNode);
					theDriver->SetDrivenNode(i,newNode);
				}

				// select the dummy node.
				attachedToNode = TRUE;

				// Reference the node so we'll get notifications.
				ReplaceReference( 0, theDriver->GetDrivenNode(0) );
				theDriver->SetRad(TimeValue(0), 0.0f);
				mat.SetTrans(vpx.SnapPoint(m, m, NULL, SNAP_IN_PLANE));
				createInterface->SetNodeTMRelConstPlane(dummyNode, mat);
				res = TRUE;
			}
			else 
			{
				// select a node so if go into modify branch, see params 
				ignoreSelectionChange = TRUE;
			 	createInterface->SelectNode( theDriver->GetDrivenNode(0) );
				ignoreSelectionChange = FALSE;
				theHold.Accept(IDS_DS_CREATE);
				res = FALSE;
			}
			createInterface->RedrawViews(createInterface->GetTime(), REDRAW_NORMAL, theDriver);  
		}
		break;
		case MOUSE_MOVE:
			if (node0) 
			{
				r = (float)fabs(vpx.SnapLength(vpx.GetCPDisp(center, Point3(0,1,0), pt0, m)));
				theDriver->SetRad(0, r);
				theDriver->radSpin->SetValue(r, FALSE );
				createInterface->RedrawViews(createInterface->GetTime(), REDRAW_NORMAL, theDriver);
			}
			res = TRUE;
			break;

		case MOUSE_FREEMOVE:
			SetCursor( UI::MouseCursors::LoadMouseCursor(UI::MouseCursors::Crosshair) );
			vpx.SnapPreview(m, m, NULL, SNAP_IN_3D);
			break;

		case MOUSE_PROPCLICK:
			// right click while between creations
			createInterface->RemoveMode(NULL);
			break;

		case MOUSE_ABORT:
			DbgAssert(theDriver);
			theDriver->EndEditParams( (IObjParam*)createInterface, 0,NULL );
			theHold.Cancel();  // undo the changes
			// DS 8/21/97: If something has been put on the undo stack since this object was 
			// created, we have to flush the undo stack.
			if (needToss) {
				GetSystemSetting(SYSSET_CLEAR_UNDO);
			}
			DeleteAllRefsFromMe();
			CreateNewDriver();	
			createInterface->RedrawViews(createInterface->GetTime(),REDRAW_END,theDriver); 
			attachedToNode = FALSE;
			res = FALSE;						
			break;
	} // end switch
	
 
	return res;
}

int RingDriverClassDesc::BeginCreate(Interface *i)
{
	SuspendSetKeyMode();
	IObjCreate *iob = i->GetIObjCreate();
	
	theRingDriverCreateMode.Begin( iob, this );
	iob->PushCommandMode( &theRingDriverCreateMode );
	
	return TRUE;
}

int RingDriverClassDesc::EndCreate(Interface *i)
{
	ResumeSetKeyMode();
	theRingDriverCreateMode.End();
	i->RemoveMode( &theRingDriverCreateMode );
	return TRUE;
}


//------------------------------------------------------------------------------
// Ringarray mxs exposure
//------------------------------------------------------------------------------
// --- Mixin RingArray Driver interface ---
FPInterfaceDesc RingDriver::mFPInterfaceDesc(
  IID_RING_ARRAY_DRIVER, 
	_T("IRingArrayDriver"),		// Interface name used by maxscript - don't localize it!
	0,												// Res ID of description string
	&theRingDriverClassDesc,	// Class descriptor
	FP_MIXIN,

	// - Methods -
	RingDriver::kRM_GetNodes, _T("GetNodes"), 0, TYPE_INT, 0, 2,
		_T("nodes"), 0, TYPE_INODE_TAB_BR, f_inOut, FPP_IN_OUT_PARAM, 
		_T("context"), 0, TYPE_ENUM, RingDriver::kfpSystemNodeContexts, 
	RingDriver::kRM_GetNode, _T("GetNode"), 0, TYPE_INODE, 0, 1,
		_T("nodeIdx"), 0, TYPE_INDEX, f_inOut, FPP_IN_PARAM, 
	
	// - Properties -
	properties,
		RingDriver::kRM_GetNodeNum, RingDriver::kRM_SetNodeNum, _T("numNodes"),	0, TYPE_INT, 
			f_range, RingDriver::kMIN_NODE_NUM, RingDriver::kMAX_NODE_NUM,
		RingDriver::kRM_GetCycle, RingDriver::kRM_SetCycle, _T("cycles"),	0, TYPE_FLOAT, 
			f_range, RingDriver::kMIN_CYCLES, RingDriver::kMAX_CYCLES,
		RingDriver::kRM_GetPhase, RingDriver::kRM_SetPhase, _T("phase"),	0, TYPE_FLOAT, 
			f_range, RingDriver::kMIN_PHASE, RingDriver::kMAX_PHASE,
		RingDriver::kRM_GetAmplitute, RingDriver::kRM_SetAmplitude, _T("amplitude"),	0, TYPE_FLOAT, 
			f_range, RingDriver::kMIN_AMPLITUDE, RingDriver::kMAX_AMPLITUDE,
		RingDriver::kRM_GetRadius, RingDriver::kRM_SetRadius, _T("radius"),	0, TYPE_FLOAT, 
			f_range, RingDriver::kMIN_RADIUS, RingDriver::kMAX_RADIUS,

	enums,
		RingDriver::kfpSystemNodeContexts, 4, 
		_T("Ctx_Clone"), kSNCClone,
		_T("Ctx_Delete"), kSNCDelete,
		_T("Ctx_FileMerge"), kSNCFileMerge,
		_T("Ctx_FileSave"), kSNCFileSave,

	p_end 
);

FPInterfaceDesc* RingDriver::GetDesc() 
{ 
	return &mFPInterfaceDesc; 
}

FPInterfaceDesc* RingDriver::GetDescByID(Interface_ID id)
{
	if (IID_RING_ARRAY_DRIVER == id)
		return &mFPInterfaceDesc;
	else
		return FPMixinInterface::GetDescByID(id);
}

BaseInterface* RingDriver::GetInterface(Interface_ID id)
{
	if (IID_RING_ARRAY_DRIVER == id) 
		return static_cast<RingDriver*>(this); 
	else 
		return FPMixinInterface::GetInterface(id); 
}


// --- Static Mxs interface for creating RingArrays -----------------------------
#define IID_RINGARRAY_SYSTEM Interface_ID(0x7a5a74ed, 0x52304621)
class FPRingArraySystem : public FPStaticInterface 
{
	public:
   DECLARE_DESCRIPTOR(FPRingArraySystem);

   enum fnID {
     kRA_Create, 
   };

   BEGIN_FUNCTION_MAP
     FN_6(kRA_Create, TYPE_INTERFACE, fpCreate, TYPE_POINT3_BR, TYPE_INT, 
			TYPE_FLOAT, TYPE_FLOAT, TYPE_FLOAT, TYPE_FLOAT);
	 END_FUNCTION_MAP

	FPInterface* fpCreate(
		const Point3& posOrigin, 
		int numNodes,
		float amplitude,
		float radius,
		float cycles,
		float phase);
};

namespace { 

static FPRingArraySystem fpRingArray (
	IID_RINGARRAY_SYSTEM, 
	_T("RingArray"), 
	NULL, NULL, 
	FP_CORE,

	FPRingArraySystem::kRA_Create, _T("Create"), 0, TYPE_INTERFACE, 0, 6,
		_T("posOrigin"), 0, TYPE_POINT3_BR, f_inOut, FPP_IN_PARAM, f_keyArgDefault, &Point3::Origin,
		_T("numNodes"), 0, TYPE_INT, 
			f_inOut, FPP_IN_PARAM, f_keyArgDefault, RingDriver::kDEF_NODE_NUM,
			f_range, RingDriver::kMIN_NODE_NUM, RingDriver::kMAX_NODE_NUM,
		_T("amplitude"), 0, TYPE_FLOAT, 
			f_inOut, FPP_IN_PARAM, f_keyArgDefault, RingDriver::kDEF_AMPLITUDE,
			f_range, RingDriver::kMIN_AMPLITUDE, RingDriver::kMAX_AMPLITUDE,
		_T("radius"), 0, TYPE_FLOAT, 
			f_inOut, FPP_IN_PARAM, f_keyArgDefault, RingDriver::kDEF_RADIUS,
			f_range, RingDriver::kMIN_RADIUS, RingDriver::kMAX_RADIUS,
		_T("cycles"), 0, TYPE_FLOAT, 
			f_inOut, FPP_IN_PARAM, f_keyArgDefault, RingDriver::kDEF_CYCLES,
			f_range, RingDriver::kMIN_CYCLES, RingDriver::kMAX_CYCLES,
		_T("phase"), 0, TYPE_FLOAT, 
			f_inOut, FPP_IN_PARAM, f_keyArgDefault, RingDriver::kDEF_PHASE,
			f_range, RingDriver::kMIN_PHASE, RingDriver::kMAX_PHASE,
	p_end
);

};

FPInterface* FPRingArraySystem::fpCreate(
	const Point3& posOrigin, 
	int numNodes,
	float amplitude,
	float radius,
	float cycles,
	float phase)
{
	Interface* ip = GetCOREInterface();
	RingDriver* theDriver = new RingDriver(numNodes);
	DbgAssert(theDriver != NULL);

	// The creation params used in interactive mode need to be kept up-to-date
	// with the param values used via maxscript creation in order to ensure that 
	// the UI displays the values actually used via maxscript creation
	RingDriver::dlgNum = numNodes;
	RingDriver::dlgAmplitude = amplitude;
	RingDriver::dlgRadius = radius;
	RingDriver::dlgCycles = cycles;
	RingDriver::dlgPhase = phase;

	// Create a dummy object & node
	DummyObject* dumObj = (DummyObject*)ip->CreateInstance(HELPER_CLASS_ID, 
		Class_ID(DUMMY_CLASS_ID, 0)); 			
	DbgAssert(dumObj);					
	INode* dummyNode = ip->CreateObjectNode(dumObj);
	dumObj->SetBox(Box3(Point3(-DUMSZ, -DUMSZ, -DUMSZ), Point3(DUMSZ, DUMSZ, DUMSZ)));

	// make a box object
	GenBoxObject* ob = (GenBoxObject*)ip->CreateInstance(GEOMOBJECT_CLASS_ID, 
		Class_ID(BOXOBJ_CLASS_ID, 0));
	DbgAssert(ob != NULL);
	ob->SetParams(BOXSZ, BOXSZ, BOXSZ, 1, 1, 1, FALSE); 

	// Make a bunch of nodes with a driven controllers. Hook the nodes to the box 
	// and the driven ctrls to the ring driver
	for (int i = 0; i < theDriver->GetNum(ip->GetTime()); i++) 
	{
		INode* newNode = ip->CreateObjectNode(ob);
		DrivenControl* driven = new DrivenControl(theDriver, i);
		newNode->SetTMController(driven);
		dummyNode->AttachChild(newNode);
		theDriver->SetDrivenNode(i, newNode);
	}

	// Position the system's root node
	Matrix3 mat;
	mat.IdentityMatrix();
	mat.SetTrans(posOrigin);
	ip->SetNodeTMRelConstPlane(dummyNode, mat);

	// Set other parameters
	theDriver->SetAmp(ip->GetTime(), amplitude);
	theDriver->SetRad(ip->GetTime(), radius);
	theDriver->SetCyc(ip->GetTime(), cycles);
	theDriver->SetPhs(ip->GetTime(), phase);

	return static_cast<FPInterface*>(theDriver);
}

// EOF
