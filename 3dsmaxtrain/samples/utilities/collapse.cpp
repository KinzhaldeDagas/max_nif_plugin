/**********************************************************************
 *<
	FILE: collapse.cpp

	DESCRIPTION:  A collapse utility

	CREATED BY: Rolf Berteig

	HISTORY: created 11/20/96

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/
#include "util.h"
#include "utilapi.h"
#include "mnmath.h"
#include "modstack.h"
#include "ICustAttribCollapseManager.h"
#include "ICustAttribContainer.h"
#include <ifnpub.h>

#define COLLAPSE_CLASS_ID		Class_ID(0xb338aad8,0x13c75c33)
#define COLLAPSE_INTERFACE_ID Interface_ID(0x45536327, 0xb5b12ba6)
#define COLLAPSE_CNAME			GetString(IDS_RB_COLLAPSE)

class ICollapseUtilFp  :public FPStaticInterface {
public:
	virtual void DoCollapse(Tab<INode*>& nodes) = 0;
	virtual int GetOutputType() = 0;
	virtual void SetOutputType(int outputType) = 0;
	virtual int GetCollapseTo() = 0;
	virtual void SetCollapseTo(int collapseTo) = 0;
	virtual BOOL GetDoBoolOp() = 0;
	virtual void SetDoBoolOp(BOOL doBool) = 0;
	virtual int GetBoolOp() = 0;
	virtual void SetBoolOp(int boolOp) = 0;

	enum CollapseMethods {
		kDoCollapse,
		kGetOutputType,
		kSetOutputType,
		kGetDoBool,
		kSetDoBool,
		kGetCollapseTo,
		kSetCollapseTo,
		kSetBoolType,
		kGetBoolType,
	};

	enum CollapseEnumList {
		OutputTypeEnum,
		CollapseToEnum,
		BoolTypeEnum
	};

	enum OutputTypes {
		MESH = IDC_C_MESH,
		STACKRESULT = IDC_C_STACKRESULT
	};

	enum CollapseToEnum {
		SINGLE = IDC_C_SINGLE,
		MULTIPLE = IDC_C_MULTIPLE
	};

	enum BoolTypeEnum {
		UNION =  IDC_C_UNION,
		SUBTRACTION = IDC_C_SUBTRACTION,
		INTERSECTION = IDC_C_INTERSECTION
	};
};

class CollapseUtilFp :public ICollapseUtilFp {
public:

	FPInterfaceDesc* GetDesc() override;
	void DoCollapse(Tab<INode*>& nodes) override;
	int GetOutputType() override;
	void SetOutputType(int outputType) override;
	int GetCollapseTo() override;
	void SetCollapseTo(int collapseTo) override;
	BOOL GetDoBoolOp() override;
	void SetDoBoolOp(BOOL doBoolOp) override;
	int GetBoolOp() override;
	void SetBoolOp(int boolOp) override;


	DECLARE_DESCRIPTOR(CollapseUtilFp)
	BEGIN_FUNCTION_MAP
		VFN_1(kDoCollapse, DoCollapse, TYPE_INODE_TAB_BR);
		
		VFN_1(kSetOutputType, SetOutputType, TYPE_ENUM);
		FN_0(kGetOutputType, TYPE_ENUM, GetOutputType);
		
		VFN_1(kSetCollapseTo, SetCollapseTo, TYPE_ENUM);
		FN_0(kGetCollapseTo, TYPE_ENUM, GetCollapseTo);

		VFN_1(kSetDoBool, SetDoBoolOp, TYPE_BOOL);
		FN_0(kGetDoBool, TYPE_BOOL, GetDoBoolOp);

		VFN_1(kSetBoolType, SetBoolOp, TYPE_ENUM);
		FN_0(kGetBoolType, TYPE_ENUM, GetBoolOp);
	END_FUNCTION_MAP

};


class CollapseUtil : public UtilityObj, public MeshOpProgress {
	public:
		IUtil *iu;
		Interface *ip;
		HWND hPanel;
		int collapseTo, outputType, boolType;
		BOOL dobool, canceled;
		int total;

		CollapseUtil();
		void BeginEditParams(Interface *ip,IUtil *iu);
		void EndEditParams(Interface *ip,IUtil *iu);
		void DeleteThis() {}
		void SelectionSetChanged(Interface *ip,IUtil *iu);

		void Init(HWND hWnd);
		void Destroy(HWND hWnd);
		void SetStates(HWND hWnd, bool setFromCode = false);

		void DoCollapse(const Tab<INode*>* nodes = NULL);

		// From MeshOpProgress
		void Init(int total);		
		BOOL Progress(int p);
	};
static CollapseUtil theCollapseUtil;

class CollapseUtilClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void*			Create(BOOL loading = FALSE) {return &theCollapseUtil;}
	const TCHAR*	ClassName() {return COLLAPSE_CNAME;}
	const TCHAR*	NonLocalizedClassName() { return _T("Collapse"); }
	SClass_ID		SuperClassID() {return UTILITY_CLASS_ID;}
	Class_ID		ClassID() {return COLLAPSE_CLASS_ID;}
	const TCHAR* 	Category() {return _T("");}
	};

static CollapseUtilClassDesc collapseUtilDesc;
ClassDesc* GetCollapseUtilDesc() {return &collapseUtilDesc;}


static INT_PTR CALLBACK CollapseUtilDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	switch (msg) {
		case WM_INITDIALOG:
			theCollapseUtil.Init(hWnd);
			break;

		case WM_DESTROY:
			theCollapseUtil.Destroy(hWnd);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
					theCollapseUtil.iu->CloseUtility();
					break;
					
				case IDC_C_MULTIPLE:
				case IDC_C_SINGLE:
					theCollapseUtil.collapseTo = LOWORD(wParam);
					theCollapseUtil.SetStates(hWnd);
					break;

				case IDC_C_BOOL:
					theCollapseUtil.dobool = IsDlgButtonChecked(hWnd,IDC_C_BOOL);
					theCollapseUtil.SetStates(hWnd);
					break;

				case IDC_C_UNION:
				case IDC_C_SUBTRACTION:
				case IDC_C_INTERSECTION:
					theCollapseUtil.boolType = LOWORD(wParam);
					theCollapseUtil.SetStates(hWnd);
					break;

				case IDC_C_STACKRESULT:
				case IDC_C_MESH:
					theCollapseUtil.outputType = LOWORD(wParam);
					theCollapseUtil.SetStates(hWnd);					
					break;

				case IDC_C_COLLAPSE:
					theCollapseUtil.DoCollapse();
					break;
				}
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			theCollapseUtil.ip->RollupMouseMessage(hWnd,msg,wParam,lParam); 
			break;

		default:
			return FALSE;
		}
	return TRUE;
	}	


CollapseUtil::CollapseUtil()
	{	
	collapseTo = IDC_C_SINGLE;
	outputType = IDC_C_MESH;	
	dobool     = FALSE;
	boolType   = IDC_C_UNION;
	}

void CollapseUtil::BeginEditParams(Interface *ip,IUtil *iu) 
	{
	this->iu = iu;
	this->ip = ip;
	hPanel = ip->AddRollupPage(
		hInstance,
		MAKEINTRESOURCE(IDD_COLLAPSE_PANEL),
		CollapseUtilDlgProc,
		GetString(IDS_RB_COLLAPSE),
		0);
	}
	
void CollapseUtil::EndEditParams(Interface *ip,IUtil *iu) 
	{
	this->iu = NULL;
	this->ip = NULL;
	ip->DeleteRollupPage(hPanel);
	hPanel = NULL;
	}

void CollapseUtil::Init(HWND hWnd)
	{
	hPanel = hWnd;
	CheckDlgButton(hWnd,collapseTo,TRUE);
	CheckDlgButton(hWnd,outputType,TRUE);
	CheckDlgButton(hWnd,IDC_C_BOOL,dobool);
	CheckDlgButton(hWnd,boolType,TRUE);
	SelectionSetChanged(ip,iu);	
	}

void CollapseUtil::SetStates(HWND hWnd, bool setfromCode)
	{
	if (ip->GetSelNodeCount()) {
		EnableWindow(GetDlgItem(hPanel,IDC_C_COLLAPSE),TRUE);
	} else {
		EnableWindow(GetDlgItem(hPanel,IDC_C_COLLAPSE),FALSE);
		}

	if (theCollapseUtil.outputType==IDC_C_STACKRESULT) {
		EnableWindow(GetDlgItem(hWnd,IDC_C_BOOL),FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_C_MULTIPLE),FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_C_SINGLE),FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_C_COLLPASETOLABEL),FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_C_UNION),FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_C_INTERSECTION),FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_C_SUBTRACTION),FALSE);
	} else {

		if (ip->GetSelNodeCount()<2) {
			if (!setfromCode)
			{
				collapseTo = IDC_C_SINGLE;
				CheckDlgButton(hWnd, IDC_C_SINGLE, TRUE);
				CheckDlgButton(hWnd, IDC_C_MULTIPLE, FALSE);
			}
			EnableWindow(GetDlgItem(hWnd,IDC_C_MULTIPLE),FALSE);
		}
		else {
			EnableWindow(GetDlgItem(hWnd,IDC_C_MULTIPLE),TRUE);
			}
		
		EnableWindow(GetDlgItem(hWnd,IDC_C_SINGLE),TRUE);
		EnableWindow(GetDlgItem(hWnd,IDC_C_COLLPASETOLABEL),TRUE);

		if (collapseTo==IDC_C_SINGLE) {
			EnableWindow(GetDlgItem(hWnd,IDC_C_BOOL),TRUE);
			if (theCollapseUtil.dobool) {
				EnableWindow(GetDlgItem(hWnd,IDC_C_UNION),TRUE);
				EnableWindow(GetDlgItem(hWnd,IDC_C_INTERSECTION),TRUE);
				EnableWindow(GetDlgItem(hWnd,IDC_C_SUBTRACTION),TRUE);			
			} else {
				EnableWindow(GetDlgItem(hWnd,IDC_C_UNION),FALSE);
				EnableWindow(GetDlgItem(hWnd,IDC_C_INTERSECTION),FALSE);
				EnableWindow(GetDlgItem(hWnd,IDC_C_SUBTRACTION),FALSE);
				}
		} else {
			EnableWindow(GetDlgItem(hWnd,IDC_C_BOOL),FALSE);
			EnableWindow(GetDlgItem(hWnd,IDC_C_UNION),FALSE);
			EnableWindow(GetDlgItem(hWnd,IDC_C_INTERSECTION),FALSE);
			EnableWindow(GetDlgItem(hWnd,IDC_C_SUBTRACTION),FALSE);			
			}				
		}
	}

void CollapseUtil::Destroy(HWND hWnd)
	{		
	hPanel = NULL;
	}

void CollapseUtil::SelectionSetChanged(Interface *ip,IUtil *iu)
	{
	SetStates(hPanel);
	if (ip->GetSelNodeCount()==1) {
		SetDlgItemText(hPanel,IDC_SEL_NAME,ip->GetSelNode(0)->GetName());
	} else if (ip->GetSelNodeCount()) {
		SetDlgItemText(hPanel,IDC_SEL_NAME,GetString(IDS_RB_MULTISEL));
	} else {
		SetDlgItemText(hPanel,IDC_SEL_NAME,GetString(IDS_RB_NONESEL));
		}	
	}

class CollapseRestore : public RestoreObj {
public:		   				
	BOOL mOnlyForRedo;
	static BOOL mMode;
	CollapseRestore(BOOL onlyForRedo = FALSE) { mOnlyForRedo = onlyForRedo; }	
	void Restore(int isUndo) {
		if (mOnlyForRedo) {
			mMode = GetCOREInterface7()->InManipMode();
			if (mMode) {
				GetCOREInterface7()->EndManipulateMode();
			}
			GetCOREInterface7()->SuspendEditing();
		}
		else {
			if (mMode) {
				mMode = FALSE;
				GetCOREInterface7()->StartManipulateMode();
			}
			GetCOREInterface7()->ResumeEditing();
		}
	}
	void Redo() {
		if (!mOnlyForRedo) {
			mMode = GetCOREInterface7()->InManipMode();
			if (mMode) {
				GetCOREInterface7()->EndManipulateMode();
			}
			GetCOREInterface7()->SuspendEditing();
		}
		else {
			if (mMode) {
				mMode = FALSE;
				GetCOREInterface7()->StartManipulateMode();
			}
			GetCOREInterface7()->ResumeEditing();
		}
	}

	TSTR Description() {return TSTR(_T("Restore Manipulate State"));}
};

BOOL CollapseRestore::mMode = FALSE;

void CollapseUtil::DoCollapse(const Tab<INode*>* nodes)
{
	INode* node = NULL;
	INode* tnode;
	TypedSingleRefMaker<TriObject> tobj;
	INodeTab collapseNodes, delNodes;

	ICustAttribCollapseManager* iCM = ICustAttribCollapseManager::GetICustAttribCollapseManager();

	Matrix3 tm1, tm2;
	int type;

	switch (boolType)
	{
	default:
	case IDC_C_UNION:
		type = MESHBOOL_UNION;
		break;
	case IDC_C_INTERSECTION:
		type = MESHBOOL_INTERSECTION;
		break;
	case IDC_C_SUBTRACTION:
		type = MESHBOOL_DIFFERENCE;
		break;
	}

	if (nodes)
	{
		for (int i = 0; i < nodes->Count(); i++)
		{
			node = (*nodes)[i];
			collapseNodes.Append(1, &node);
		}
		if (nodes->Count() < 2)
			collapseTo = IDC_C_SINGLE;
	}
	else
	{
		for (int i = 0; i < ip->GetSelNodeCount(); i++)
		{
			// Get a selected node
			node = ip->GetSelNode(i);
			collapseNodes.Append(1, &node);
		}

		if (ip->GetSelNodeCount() < 2)
		{

			collapseTo = IDC_C_SINGLE;
			CheckDlgButton(hPanel, IDC_C_SINGLE, true);
			SetStates(hPanel);
		}
	}

	// if there are no nodes to collapse exit
	if (node == NULL)
		return;

	theHold.Begin();
	if (theHold.Holding())
		theHold.Put(new CollapseRestore);
	canceled = FALSE;

	while (collapseNodes.Count() > 0)
	{
		// Get a node
		node = collapseNodes[0];
		collapseNodes.Delete(0, 1);

		// Get the node's object (exclude WSMs)
		TypedSingleRefMaker<Object> oldObj{ node->GetObjectRef() };
		// Check for NULL
		if (!oldObj)
			continue;

		// Skip bones
		if (oldObj->ClassID() == Class_ID(BONE_CLASS_ID, 0))
			continue;

		// RB 6/14/99: Skip system nodes too
		Control* tmCont = node->GetTMController();
		if (tmCont && GetDriverController(tmCont))
			continue;

		// NH 14 April 04: Added support for the maintaining CAs on stack collapse
		// LAM - 7/22/05 - DoCollapse does not affect WSMs, So we shouldn't enumerate them....
		bool ignoreBaseObjectCAs = false;
		if (iCM && iCM->GetCustAttribSurviveCollapseState())
		{
			NotifyCollapseMaintainCustAttribEnumProc2 PreNCEP(true, node);
			EnumGeomPipeline(&PreNCEP, oldObj);
		}
		else
		{
			NotifyCollapseEnumProc PreNCEP(true, node);
			EnumGeomPipeline(&PreNCEP, oldObj);
		}

		ObjectState os = oldObj->Eval(ip->GetTime());

		HoldSuspend hs;
		Object* _obj = os.obj->CollapseObject();
		if (_obj == os.obj)
		{
			// if we are cloning the base object, the clone will take care of the CAs
			Object* theBaseObject = oldObj->FindBaseObject();
			if (_obj == theBaseObject)
				ignoreBaseObjectCAs = true;

			_obj = (Object*)CloneRefHierarchy(_obj);
		}
		TypedSingleRefMaker<Object> obj{ _obj };
		hs.Resume();

		if (outputType == IDC_C_STACKRESULT)
		{
			obj->SetSubSelState(
					0); // Reset the selection level to object level (in case it happens to have an edit mesh modifier
			// Make the result of the stack the new object
			node->SetObjectRef(obj);
			GetCOREInterface7()->InvalidateObCache(node);
			node->NotifyDependents(FOREVER, 0, REFMSG_SUBANIM_STRUCTURE_CHANGED);

			// NS: 4/6/00 Notify all mods and objs in the pipleine, that they have been collapsed
			// NH 14 April 04: Added support for the maintaining CAs on stack collapse
			if (iCM && iCM->GetCustAttribSurviveCollapseState())
			{
				NotifyCollapseMaintainCustAttribEnumProc2 PostNCEP(false, node, ignoreBaseObjectCAs, obj);
				EnumGeomPipeline(&PostNCEP, oldObj);
			}
			else
			{
				NotifyCollapseEnumProc PostNCEP(false, node, obj);
				EnumGeomPipeline(&PostNCEP, oldObj);
			}
			oldObj = nullptr;
		}
		else
		{
			if (collapseTo == IDC_C_SINGLE)
			{
				if (obj->CanConvertToType(triObjectClassID))
				{
					// Convert to a TriObject
					hs.Suspend();
					TypedSingleRefMaker<TriObject> ntobj{ (TriObject*)obj->ConvertToType(
							ip->GetTime(), triObjectClassID) };
					hs.Resume();
					if (ntobj != obj)
						ignoreBaseObjectCAs = false;

					if (!tobj)
					{
						// First one
						tobj = ntobj.Get();
						tnode = node;
						node->SetObjectRef(tobj);
						GetCOREInterface7()->InvalidateObCache(node);
						node->NotifyDependents(FOREVER, 0, REFMSG_SUBANIM_STRUCTURE_CHANGED);
						tm1 = node->GetObjTMBeforeWSM(ip->GetTime());
					}
					else if (dobool)
					{
						Mesh mesh;
						tm2 = node->GetObjTMBeforeWSM(ip->GetTime());

						TSTR title(GetString(IDS_RB_BOOLEAN));
						title = title + TSTR(node->GetName());
						ip->ProgressStart(title, TRUE, nullptr, nullptr);

						MNMesh op1(tobj->GetMesh());
						MNMesh op2(ntobj->GetMesh());
						op1.Transform(tm1);
						op2.Transform(tm2);
						op1.PrepForBoolean();
						op2.PrepForBoolean();
						MNMesh out;
						out.MakeBoolean(op1, op2, type, this);
						out.Transform(Inverse(tm1));
						mesh.FreeAll();
						out.OutToTri(mesh);

						ip->ProgressEnd();
						if (canceled)
						{
							oldObj = nullptr;
							obj = nullptr;
							ntobj = nullptr;
							break;
						}

						tobj->GetMesh() = mesh;
						delNodes.Append(1, &node);
					}
					else
					{
						Mesh mesh;
						tm2 = node->GetObjTMBeforeWSM(ip->GetTime());
						CombineMeshes(mesh, tobj->GetMesh(), ntobj->GetMesh(), &tm1, &tm2);
						tobj->GetMesh() = mesh;
						delNodes.Append(1, &node);
						collapseNodes.Append(1, &tnode);
						ntobj = nullptr;
						tobj = nullptr;
					}

					// NS: 4/6/00 Notify all mods and objs in the pipeline, that they have been collapsed
					// NH 14 April 04: Added support for the maintaining CAs on stack collapse
					if (iCM && iCM->GetCustAttribSurviveCollapseState())
					{
						NotifyCollapseMaintainCustAttribEnumProc2 PostNCEP(false, node, ignoreBaseObjectCAs, tobj);
						EnumGeomPipeline(&PostNCEP, oldObj);
					}
					else
					{
						NotifyCollapseEnumProc PostNCEP(false, node, tobj);
						EnumGeomPipeline(&PostNCEP, oldObj);
					}
					oldObj = nullptr;
					obj = nullptr;
				}
				else
				{
					// Can't convert it.
					obj = nullptr;
				}
			}
			else
			{
				if (obj->CanConvertToType(triObjectClassID))
				{
					// Convert it to a TriObject and make that the new object
					tobj = (TriObject*)obj->ConvertToType(ip->GetTime(), triObjectClassID);
					if (tobj != obj)
						ignoreBaseObjectCAs = false;
					node->SetObjectRef(tobj);
					GetCOREInterface7()->InvalidateObCache(node);
					node->NotifyDependents(FOREVER, 0, REFMSG_SUBANIM_STRUCTURE_CHANGED);

					// NS: 4/6/00 Notify all mods and objs in the pipleine, that they have been collapsed
					// NH 14 April 04: Added support for the maintaining CAs on stack collapse
					if (iCM && iCM->GetCustAttribSurviveCollapseState())
					{
						NotifyCollapseMaintainCustAttribEnumProc2 PostNCEP(false, node, ignoreBaseObjectCAs, tobj);
						EnumGeomPipeline(&PostNCEP, oldObj);
					}
					else
					{
						NotifyCollapseEnumProc PostNCEP(false, node, tobj);
						EnumGeomPipeline(&PostNCEP, oldObj);
					}
					oldObj = nullptr;
					obj = nullptr;
				}
			}
		}
	}

	if (canceled)
		theHold.Cancel();
	else
	{
		GetCOREInterface14()->DeleteNodes(delNodes, true, false);
		if (theHold.Holding())
			theHold.Put(new CollapseRestore(TRUE));
		theHold.Accept(GetString(IDS_COLLAPSE));
		ip->RedrawViews(ip->GetTime());
	}
}

void CollapseUtil::Init(int total)
	{
	this->total = total;
	canceled = FALSE;
	}

BOOL CollapseUtil::Progress(int p)
	{
	int pct = total?(p*100)/total:100;
	ip->ProgressUpdate(pct);
	if (ip->GetCancel()) {
		ip->SetCancel(FALSE);
		canceled = TRUE;
		return FALSE;
	} else {
		return TRUE;
		}
	}

static CollapseUtilFp fp_collapse(
	COLLAPSE_INTERFACE_ID, 	_T("Collapse"), 0, &collapseUtilDesc, FP_STATIC_METHODS,
	
	CollapseUtilFp::kDoCollapse , _T("DoCollapse"), 0, TYPE_VOID,0,1,
	_T("nodeSet"), 0, TYPE_INODE_TAB_BR,

	CollapseUtilFp::kGetOutputType, _T("GetOutputType"), 0, TYPE_ENUM, CollapseUtilFp::OutputTypeEnum , 0, 0,
	CollapseUtilFp::kSetOutputType, _T("SetOutputType"), 0, TYPE_VOID, 0, 1,
	_T("outputType"), 0, TYPE_ENUM, CollapseUtilFp::OutputTypeEnum,

	CollapseUtilFp::kGetCollapseTo, _T("GetCollapseTo"), 0, TYPE_ENUM, CollapseUtilFp::CollapseToEnum , 0, 0,
	CollapseUtilFp::kSetCollapseTo, _T("SetCollapseTo"), 0, TYPE_VOID, 0, 1,
	_T("CollapseToType"),0, TYPE_ENUM, CollapseUtilFp::CollapseToEnum,

	CollapseUtilFp::kGetDoBool, _T("GetDoBool"), 0, TYPE_BOOL, 0,0,
	CollapseUtilFp::kSetDoBool, _T("SetDoBool"), 0, TYPE_VOID, 0, 1,
	_T("DoBool"), 0, TYPE_BOOL,

	CollapseUtilFp::kGetBoolType, _T("GetBoolType"), 0, TYPE_ENUM, CollapseUtilFp::BoolTypeEnum, 0, 0,
	CollapseUtilFp::kSetBoolType, _T("SetBoolType"), 0, TYPE_VOID, 0, 1,
	_T("BoolType"), 0, TYPE_ENUM, CollapseUtilFp::BoolTypeEnum,

	enums,
	CollapseUtilFp::OutputTypeEnum, 2,
	_T("Mesh"), CollapseUtilFp::MESH,
	_T("StackResult"), CollapseUtilFp::STACKRESULT,
	
	CollapseUtilFp::CollapseToEnum, 2,
	_T("Single"), CollapseUtilFp::SINGLE,
	_T("Multiple"), CollapseUtilFp::MULTIPLE,

	CollapseUtilFp::BoolTypeEnum, 3,
	_T("UNION"), CollapseUtilFp::UNION,
	_T("INTERSECTION"), CollapseUtilFp::INTERSECTION,
	_T("SUBTRACTION"), CollapseUtilFp::SUBTRACTION,

	p_end
);

FPInterfaceDesc* CollapseUtilFp::GetDesc()
{
	return &fp_collapse;
}

void CollapseUtilFp::DoCollapse(Tab<INode*>& nodes)
{
	GetCOREInterface7()->SuspendEditing((1 << TASK_MODE_MODIFY), TRUE);

	if(theCollapseUtil.ip == NULL)
		theCollapseUtil.ip = GetCOREInterface();

	theCollapseUtil.DoCollapse(&nodes);
	GetCOREInterface7()->ResumeEditing((1 << TASK_MODE_MODIFY), TRUE);
}

void CollapseUtilFp::SetOutputType (int outputType)
{
	theCollapseUtil.outputType = outputType;
	if (theCollapseUtil.hPanel != NULL)
	{
		CheckDlgButton(theCollapseUtil.hPanel, IDC_C_STACKRESULT, outputType == CollapseUtilFp::STACKRESULT);
		CheckDlgButton(theCollapseUtil.hPanel, IDC_C_MESH, outputType == CollapseUtilFp::MESH);
		theCollapseUtil.SetStates(theCollapseUtil.hPanel);
	}
	
}

int CollapseUtilFp::GetOutputType()
{
	return theCollapseUtil.outputType;
}


int CollapseUtilFp::GetCollapseTo()
{
	return theCollapseUtil.collapseTo;
}

void CollapseUtilFp::SetCollapseTo(int collapseTo)
{
	theCollapseUtil.collapseTo = collapseTo;
	if (theCollapseUtil.hPanel != NULL) 
	{
		CheckDlgButton(theCollapseUtil.hPanel, IDC_C_SINGLE, collapseTo == CollapseUtilFp::SINGLE);
		CheckDlgButton(theCollapseUtil.hPanel, IDC_C_MULTIPLE, collapseTo == CollapseUtilFp::MULTIPLE);
		theCollapseUtil.SetStates(theCollapseUtil.hPanel,true);
	}
}

BOOL CollapseUtilFp::GetDoBoolOp()
{
	return theCollapseUtil.dobool;
}

void CollapseUtilFp::SetDoBoolOp(BOOL doBoolOp)
{
	theCollapseUtil.dobool = doBoolOp;
	if (theCollapseUtil.hPanel != NULL)
	{
		CheckDlgButton(theCollapseUtil.hPanel, IDC_C_BOOL, doBoolOp);
		theCollapseUtil.SetStates(theCollapseUtil.hPanel);
	}

}

int CollapseUtilFp::GetBoolOp()
{
	return theCollapseUtil.boolType;
}

void CollapseUtilFp::SetBoolOp(int boolOp)
{
	theCollapseUtil.boolType = boolOp;

	if (theCollapseUtil.hPanel != NULL)
	{
		CheckDlgButton(theCollapseUtil.hPanel, IDC_C_UNION, boolOp == CollapseUtilFp::UNION);
		CheckDlgButton(theCollapseUtil.hPanel, IDC_C_INTERSECTION, boolOp == CollapseUtilFp::INTERSECTION);
		CheckDlgButton(theCollapseUtil.hPanel, IDC_C_SUBTRACTION, boolOp == CollapseUtilFp::SUBTRACTION);
		theCollapseUtil.SetStates(theCollapseUtil.hPanel,true);
	}
}
