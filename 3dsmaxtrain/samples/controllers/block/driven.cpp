/**********************************************************************
 *<
   FILE: driven.cpp

   DESCRIPTION: A controller that is driven by the driver and the sub control

   CREATED BY: Peter Watje

   HISTORY: Oct 15, 1998

 *>   Copyright (c) 1998, All Rights Reserved.
 **********************************************************************/
#include "block.h"
#include "units.h"
#include "driverblock.h"
#include "istdplug.h"

#include "3dsmaxport.h"

static DrivenFloatClassDesc drivenFloatCD;
ClassDesc* GetDrivenFloatDesc() {return &drivenFloatCD;}

static DrivenPosClassDesc drivenPosCD;
ClassDesc* GetDrivenPosDesc() {return &drivenPosCD;}

static DrivenPoint3ClassDesc drivenPoint3CD;
ClassDesc* GetDrivenPoint3Desc() {return &drivenPoint3CD;}

static DrivenRotationClassDesc drivenRotationCD;
ClassDesc* GetDrivenRotationDesc() {return &drivenRotationCD;}

static DrivenScaleClassDesc drivenScaleCD;
ClassDesc* GetDrivenScaleDesc() {return &drivenScaleCD;}


INT_PTR CALLBACK NewLinkDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK NewDriverDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//------------------------------------------------------------

DrivenControl::DrivenControl() 
   {  
   range     = Interval(GetAnimStart(),GetAnimEnd());
   driver = NULL;
   scratchControl = NULL;
   sub = NULL;
   driverPresent = FALSE;
	propBlockID = 0;
	propSubID = 0;
   } 

DrivenControl::~DrivenControl() 
   {  
   int ct = blockID.Count();
   for (int i = 0; i < ct; i++)
      RemoveControl(0);

   if (scratchControl)
      {
      scratchControl->MaybeAutoDelete();
      }
   scratchControl = NULL;
   } 

int DrivenControl::NumSubs() 
   {
   return 0;
   }


void DrivenControl::SetValue(TimeValue t, void *val, int commit, GetSetMethod method)
   {
// sub->SetValue(t,val,commit,method);
   }

void DrivenControl::EnumIKParams(IKEnumCallback &callback)
   {
   if (scratchControl)
      scratchControl->EnumIKParams(callback);
   }

BOOL DrivenControl::CompDeriv(TimeValue t,Matrix3& ptm,IKDeriv& derivs,DWORD flags)
   {
   if (scratchControl)
      return scratchControl->CompDeriv(t,ptm,derivs,flags);
   else return FALSE;
   }

void DrivenControl::MouseCycleCompleted(TimeValue t)
   {
   if (scratchControl)
      scratchControl->MouseCycleCompleted(t);
   }

void DrivenControl::AddNewKey(TimeValue t,DWORD flags)
   {
   if (scratchControl)
      scratchControl->AddNewKey(t,flags);
   }

void DrivenControl::CloneSelectedKeys(BOOL offset)
   {
   if (scratchControl)
      scratchControl->CloneSelectedKeys(offset);
   }

void DrivenControl::DeleteKeys(DWORD flags)
   {
   if (scratchControl)
      scratchControl->DeleteKeys(flags);
   }

void DrivenControl::SelectKeys(TrackHitTab& sel, DWORD flags)
   {
   if (scratchControl)
      scratchControl->SelectKeys(sel,flags);
   }

BOOL DrivenControl::IsKeySelected(int index)
   {
   if (scratchControl)
      return scratchControl->IsKeySelected(index);
   return FALSE;
   }

void DrivenControl::CopyKeysFromTime(TimeValue src,TimeValue dst,DWORD flags)
   {
   if (scratchControl)
      scratchControl->CopyKeysFromTime(src,dst,flags);

   }

void DrivenControl::DeleteKeyAtTime(TimeValue t)
   {
   if (scratchControl)
      scratchControl->DeleteKeyAtTime(t);
   }

BOOL DrivenControl::IsKeyAtTime(TimeValue t,DWORD flags)
   {
   if (scratchControl)
      return scratchControl->IsKeyAtTime(t,flags);
   return FALSE;
   }

BOOL DrivenControl::GetNextKeyTime(TimeValue t,DWORD flags,TimeValue &nt)
   {
   if (scratchControl)
      return scratchControl->GetNextKeyTime(t,flags,nt);
   return FALSE;
   }

int DrivenControl::GetKeyTimes(Tab<TimeValue> &times,Interval range,DWORD flags)
   {
   if (scratchControl)
      return scratchControl->GetKeyTimes(times,range,flags);
   return 0;
   }

int DrivenControl::GetKeySelState(BitArray &sel,Interval range,DWORD flags)
   {
   if (scratchControl)
      return scratchControl->GetKeySelState(sel,range,flags);
   return 0;
   }




Animatable* DrivenControl::SubAnim(int i) 
   {
      return NULL;
   }


TSTR DrivenControl::SubAnimName(int i, bool localized)
{
	return localized ? GetString(IDS_PW_SUB) : _T("Sub Control");
}

BOOL DrivenControl::AssignController(Animatable *control,int subAnim) 
   {
   return FALSE;
   }


int DrivenControl::NumRefs() 
   {
   return 2;
   }

RefTargetHandle DrivenControl::GetReference(int i) 
   {
	DbgAssert( i >= 0);
	DbgAssert( i < NumRefs());
	if (i==0) 
		return (RefTargetHandle) sub;
	else if (i==1) 
		return (RefTargetHandle) driver;
   else
      {
//    DebugPrint(_T("get reference error occurred\n"));
      return NULL;
      }
   }

void DrivenControl::SetReference(int i, RefTargetHandle rtarg) 
   {
	DbgAssert( i >= 0);
	DbgAssert( i < NumRefs());
	if (i==0) 
		sub = (Control*) rtarg;
   else if (i==1) 
      {
      if ((rtarg == NULL) && (driver))
         {
//tell the driver that I am being removed
         int ct = blockID.Count();
         for (int i = 0; i < ct; i++)
            RemoveControl(0);
         }

      driver = (DriverBlockControl*) rtarg;
		if (driver == NULL) 
			driverPresent = FALSE;
		else 
			driverPresent = TRUE;
      }
	else
		DebugPrint(_T("set reference error occurred\n"));
   }


void DrivenControl::Copy(Control *from)
   {
   if ( from->CanCopyTrack(FOREVER,0) )
      ReplaceReference(0,from);
   superID = from->SuperClassID();
   }

RefResult DrivenControl::NotifyRefChanged(
      const Interval& changeInt, 
      RefTargetHandle hTarget, 
      PartID& partID,  
	  RefMessage message, 
	  BOOL propagate)
   {
   switch (message) {

      case REFMSG_TARGET_DELETED:
         if (hTarget == driver) {
            driverPresent = FALSE;
            }

         break;
      case REFMSG_CHANGE:
         break;


      }
   return REF_SUCCEED;
   }



class DrivenRangeRestore : public RestoreObj {
   public:
      DrivenControl *cont;
      Interval ur, rr;
      DrivenRangeRestore(DrivenControl *c) 
         {
         cont = c;
         ur   = cont->range;
         }        
      void Restore(int isUndo) 
         {
         rr = cont->range;
         cont->range = ur;
         cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
         }
      void Redo()
         {
         cont->range = rr;
         cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
         }     
      void EndHold() 
         { 
         cont->ClearAFlag(A_HELD);
         }
      TSTR Description() { return _T("Driven control range"); }
   };


void DrivenControl::HoldRange()
   {
   if (theHold.Holding() && !TestAFlag(A_HELD)) {
      SetAFlag(A_HELD);
      theHold.Put(new DrivenRangeRestore(this));
      }
   }

void DrivenControl::EditTimeRange(Interval range,DWORD flags)
   {

   }

void DrivenControl::MapKeys(TimeMap *map,DWORD flags)
   {

   }
void DrivenControl::RemoveControl(int sel)

{
if (driver)
   {
   if (blockID[sel] < driver->Blocks.Count() )
      {
      for (int i = 0; i <driver->Blocks[blockID[sel]]->externalBackPointers.Count(); i++)
         { 
         if ( this == driver->Blocks[blockID[sel]]->externalBackPointers[i])
            {
            driver->Blocks[blockID[sel]]->externalBackPointers.Delete(i,1);
            i--;
            }
         }
      for (int i = 0; i <driver->Blocks[blockID[sel]]->backPointers.Count(); i++)
         { 
         if ( this == driver->Blocks[blockID[sel]]->backPointers[i])
            {
            driver->Blocks[blockID[sel]]->backPointers[i]= NULL;
            }
         }
      }
   }
blockID.Delete(sel,1);
subID.Delete(sel,1);

}

void DrivenControl::AddControl(int blockid,int subid )
{
	DbgAssert(driver);
	if (!driver)
		return;
		
blockID.Append(1,&blockid,1);
subID.Append(1,&subid,1);
DrivenControl *driven = this;
driver->Blocks[blockid]->externalBackPointers.Append(1,&driven,1);
if (sub == NULL)
   {
   ReplaceReference(0,CloneRefHierarchy(driver->Blocks[blockid]->controls[subid]));
   }
}

void DrivenControl::CollapseControl()
{
#define ID_TV_GETSELECTED  680

	MaxSDK::Array<TrackViewPick> res;

SendMessage(trackHWND,WM_COMMAND,ID_TV_GETSELECTED,(LPARAM)&res);
	if (res.length() == 1)
   {
   if (driverPresent)
      {
      Control *mc = (Control *) CloneRefHierarchy(driver->blendControl);

      for (int ct = 0; ct < scratchControl->NumMultCurves(); ct++)
         scratchControl->DeleteMultCurve(ct);
      scratchControl->AppendMultCurve(mc);
      }
   int ct = blockID.Count();
   for (int i = 0; i < ct; i++)
      RemoveControl(0);

   res[0].client->AssignController(CloneRefHierarchy(scratchControl),res[0].subNum);

   NotifyDependents(FOREVER,0,REFMSG_CHANGE);
   NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
   }
}


#define COUNT_CHUNK     0x01010
#define DATA_CHUNK      0x01020


IOResult DrivenControl::Save(ISave *isave)
   {     
   ULONG nb;   
//count
   int count = blockID.Count();
   isave->BeginChunk(COUNT_CHUNK);
   isave->Write(&count,sizeof(count),&nb);         
   isave->EndChunk();
//id data 
   for (int i =0; i < count; i++)
      {
      isave->BeginChunk(DATA_CHUNK);
      isave->Write(&blockID[i],sizeof(int),&nb);         
      isave->Write(&subID[i],sizeof(int),&nb);        
      isave->EndChunk();
      }
   return IO_OK;
   }

IOResult DrivenControl::Load(ILoad *iload)
   {
   int ID =  0;
   ULONG nb;
   IOResult res = IO_OK;
   int ix = 0;
   while (IO_OK==(res=iload->OpenChunk())) 
      {
      ID = iload->CurChunkID();
      if (ID ==COUNT_CHUNK)
         {
         int ct;
         iload->Read(&ct, sizeof(ct), &nb);
         blockID.SetCount(ct);
         subID.SetCount(ct);
         }
      else if (ID == DATA_CHUNK)
         {
         int bID,sID;
         iload->Read(&bID, sizeof(int), &nb);
         iload->Read(&sID, sizeof(int), &nb);
         blockID[ix] = bID;
         subID[ix++] = sID;
         }
      iload->CloseChunk();
      if (res!=IO_OK)  return res;
      }

//rebuild all tempcontrols 
   return IO_OK;
   }


//--------------------------------------------------------------------


RefTargetHandle DrivenControl::Clone(RemapDir& remap)
   {
   DrivenControl *cont = new DrivenControl;
   cont->sub = NULL;
   cont->driver = NULL;
   cont->scratchControl = NULL;

   cont->ReplaceReference(0,sub);
   cont->ReplaceReference(1,driver);
   cont->blockID = blockID;
   cont->subID = subID;
   cont->driverPresent = driverPresent;

   CloneControl(cont,remap);
   cont->UpdateDriven();
   BaseClone(this, cont, remap);
   return cont;
   }


void DrivenControl::GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method)
{
}

//------------------------------------------------------------
//Driven Float Controller
//------------------------------------------------------------

RefTargetHandle DrivenFloatControl::Clone(RemapDir& remap)
   {
   DrivenFloatControl *cont = new DrivenFloatControl;
   cont->sub = NULL;
   cont->driver = NULL;
   cont->scratchControl = NULL;

   cont->ReplaceReference(0,sub);
   cont->ReplaceReference(1,driver);
   cont->blockID = blockID;
   cont->subID = subID;
   cont->driverPresent = driverPresent;
   if (driver)
      {
      for (int i = 0; i < blockID.Count(); i++)
         {
         DrivenControl *c = (DrivenControl *)cont;
         driver->Blocks[blockID[i]]->externalBackPointers.Append(1,&c,1);
         }
      }
   CloneControl(cont,remap);
   cont->UpdateDriven();
   BaseClone(this, cont, remap);
   return cont;
   }

DrivenFloatControl::DrivenFloatControl() 
   {  

   } 

void DrivenFloatControl::UpdateDriven()
{
   if (scratchControl == NULL && sub)
      scratchControl = (Control *) CloneRefHierarchy(sub);

   if (scratchControl == NULL)
      return;

   scratchControl->DeleteKeys(TRACK_DOALL);
   float f = 0.0f;
   scratchControl->SetValue(0,&f);

   if (driver)
      driver->Update(scratchControl,blockID,subID);
}

void DrivenFloatControl::GetValue(
      TimeValue t, void *val, Interval &valid, GetSetMethod method)
   {
   if ( (sub == NULL) || (!driverPresent) || (blockID.Count()==0))
      {
      if (method == CTRL_ABSOLUTE)
         {
         float *v = ((float*)val);
         *v = 0.0f;
         }
      return;
      }

   if (scratchControl == NULL)
      {
      UpdateDriven();
      }
   float *tv = ((float*)val);

   if (driver)
	{
		// should we be passing the float tv here? or val?
		driver->GetValue3(scratchControl,t,tv,valid,blockID,subID,range,method);
	}
   }


//------------------------------------------------------------
//Driven Pos Controller
//------------------------------------------------------------

RefTargetHandle DrivenPosControl::Clone(RemapDir& remap)
   {
   DrivenPosControl *cont = new DrivenPosControl;
   cont->sub = NULL;
   cont->driver = NULL;
   cont->scratchControl = NULL;
   cont->ReplaceReference(0,sub);
   cont->ReplaceReference(1,driver);
   cont->blockID = blockID;
   cont->subID = subID;
   cont->driverPresent = driverPresent;
   if (driver)
      {
      for (int i = 0; i < blockID.Count(); i++)
         {
         DrivenControl *c = (DrivenControl *)cont;
         driver->Blocks[blockID[i]]->externalBackPointers.Append(1,&c,1);
         }
      }
   CloneControl(cont,remap);
   cont->UpdateDriven();
   BaseClone(this, cont, remap);
   return cont;
   }

DrivenPosControl::DrivenPosControl() 
   {  

   } 


void DrivenPosControl::UpdateDriven()
{
   // CAL-06/03/02: sub could be NULL
   if (scratchControl == NULL && sub)
      scratchControl = (Control *) CloneRefHierarchy(sub);

   if (scratchControl == NULL)
      return;

   scratchControl->DeleteKeys(TRACK_DOALL);
   Point3 f(0.0f,0.f,0.0f);
   scratchControl->SetValue(0,&f);

   if (driver)
      driver->Update(scratchControl,blockID,subID);
}



void DrivenPosControl::GetValue(
      TimeValue t, void *val, Interval &valid, GetSetMethod method)
   {
   if ( (sub == NULL) || (!driverPresent) || (blockID.Count()==0))
      {
      if (method == CTRL_ABSOLUTE)
         {
         Point3 *v = ((Point3*)val);
         *v = Point3(0.0f,0.0f,0.0f);
         }
      else
         {
         Point3 f(0.0f,0.0f,0.0f);
         Matrix3 *v = ((Matrix3*)val);
         v->PreTranslate(f);
         
         }

      return;
      }
//copy keys into scratch control
   if (scratchControl == NULL)
      {
      UpdateDriven();
      }

   if (driver)
      driver->GetValue3(scratchControl,t,val,valid,blockID,subID,range,method);
   }

//------------------------------------------------------------
//Driven Point3 Controller
//------------------------------------------------------------

RefTargetHandle DrivenPoint3Control::Clone(RemapDir& remap)
   {
   DrivenPoint3Control *cont = new DrivenPoint3Control;
// *cont = *this;
   cont->sub = NULL;
   cont->driver = NULL;
   cont->scratchControl = NULL;

   cont->ReplaceReference(0,sub);
   cont->ReplaceReference(1,driver);
   cont->blockID = blockID;
   cont->subID = subID;
   cont->driverPresent = driverPresent;
   if (driver)
      {
      for (int i = 0; i < blockID.Count(); i++)
         {
         DrivenControl *c = (DrivenControl *)cont;
         driver->Blocks[blockID[i]]->externalBackPointers.Append(1,&c,1);
         }
      }
   CloneControl(cont,remap);
   cont->UpdateDriven();
   BaseClone(this, cont, remap);
   return cont;
   }

DrivenPoint3Control::DrivenPoint3Control() 
   {  

   } 


void DrivenPoint3Control::UpdateDriven()
{
   if (scratchControl == NULL && sub)
      scratchControl = (Control *) CloneRefHierarchy(sub);

   if (scratchControl == NULL)
      return;

   scratchControl->DeleteKeys(TRACK_DOALL);
   Point3 f(0.0f,0.f,0.0f);
   scratchControl->SetValue(0,&f);

   if (driver)
      driver->Update(scratchControl,blockID,subID);
}

void DrivenPoint3Control::GetValue(
      TimeValue t, void *val, Interval &valid, GetSetMethod method)
   {
   if ( (sub == NULL) || (!driverPresent) || (blockID.Count()==0))
      {
      Point3 *v = ((Point3*)val);
      *v = Point3(0.0f,0.0f,0.0f);
      return;
      }
//copy keys into scratch control

   if (scratchControl == NULL)
      {
      UpdateDriven();
      }

   if (driver)
      driver->GetValue3(scratchControl,t,val,valid,blockID,subID,range,method);
   }



//------------------------------------------------------------
//Driven rotation Controller
//------------------------------------------------------------

RefTargetHandle DrivenRotationControl::Clone(RemapDir& remap)
   {
   DrivenRotationControl *cont = new DrivenRotationControl;
// *cont = *this;
   cont->sub = NULL;
   cont->driver = NULL;
   cont->scratchControl = NULL;

   cont->ReplaceReference(0,sub);
   cont->ReplaceReference(1,driver);
   cont->blockID = blockID;
   cont->subID = subID;
   cont->driverPresent = driverPresent;
   if (driver)
      {
      for (int i = 0; i < blockID.Count(); i++)
         {
         DrivenControl *c = (DrivenControl *)cont;
         driver->Blocks[blockID[i]]->externalBackPointers.Append(1,&c,1);
         }
      }
   CloneControl(cont,remap);
   cont->UpdateDriven();
   BaseClone(this, cont, remap);
   return cont;
   }


DrivenRotationControl::DrivenRotationControl() 
   {  

   } 


void DrivenRotationControl::UpdateDriven()
{
   // CAL-06/03/02: sub could be NULL
   if (scratchControl == NULL && sub)
      scratchControl = (Control *) CloneRefHierarchy(sub);

   if (scratchControl == NULL)
      return;

   scratchControl->DeleteKeys(TRACK_DOALL);
   Quat f;
   f.Identity();
   scratchControl->SetValue(0,&f);

   if (driver)
      driver->Update(scratchControl,blockID,subID);
}

void DrivenRotationControl::GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method)
   {
   if ( (sub == NULL) || (!driverPresent) || (blockID.Count()==0))
      {
      Quat f;
      f.Identity();
      if (method == CTRL_ABSOLUTE)
         {
         Quat *v = ((Quat*)val);
         *v = f;
         return;
         }
      else
         {
         Matrix3 *v = ((Matrix3*)val);
         PreRotateMatrix(*v,f);
         return;           
         }
      }

//copy keys into scratch control
   if (scratchControl == NULL)
      {
      UpdateDriven();
      }

   if (driver)
      driver->GetValue3(scratchControl,t,val,valid,blockID,subID,range,method);
   }

//------------------------------------------------------------
//Driven Scale Controller
//------------------------------------------------------------

RefTargetHandle DrivenScaleControl::Clone(RemapDir& remap)
   {
   DrivenScaleControl *cont = new DrivenScaleControl;
// *cont = *this;
   cont->sub = NULL;
   cont->driver = NULL;
   cont->scratchControl = NULL;

   cont->ReplaceReference(0,sub);
   cont->ReplaceReference(1,driver);
   cont->blockID = blockID;
   cont->subID = subID;
   cont->driverPresent = driverPresent;
   if (driver)
      {
      for (int i = 0; i < blockID.Count(); i++)
         {
         DrivenControl *c = (DrivenControl *)cont;
         driver->Blocks[blockID[i]]->externalBackPointers.Append(1,&c,1);
         }
      }
   CloneControl(cont,remap);
   cont->UpdateDriven();
   BaseClone(this, cont, remap);
   return cont;
   }

DrivenScaleControl::DrivenScaleControl() 
   {  

   } 

void DrivenScaleControl::UpdateDriven()
{
   if (scratchControl == NULL && sub)
      scratchControl = (Control *) CloneRefHierarchy(sub);

   if (scratchControl == NULL)
      return;

   scratchControl->DeleteKeys(TRACK_DOALL);

   Quat f;
   f.Identity();
   Point3 p(1.0f,1.0f,1.0f);
   ScaleValue s(p,f); 

   scratchControl->SetValue(0,&s);

   if (driver)
      driver->Update(scratchControl,blockID,subID);
}

void DrivenScaleControl::GetValue(TimeValue t, void *val, Interval &valid, GetSetMethod method)
   {
   if ( (sub == NULL) || (!driverPresent) || (blockID.Count()==0))
      {
      Quat f;
      f.Identity();
      Point3 p(1.0f,1.0f,1.0f);
      if (method == CTRL_ABSOLUTE)
         {
         ScaleValue s(p,f); 
         ScaleValue *v = ((ScaleValue*)val);
         *v = s;
         return;
         }
      else
         {
         Matrix3 *mat = (Matrix3*)val;
         ScaleValue s(p,f); 
         ApplyScaling(*mat,s);
         return;           
         }

      }

   if (scratchControl == NULL)
      UpdateDriven();


   if (driver)
      driver->GetValue3(scratchControl,t,val,valid,blockID,subID,range,method);

   }

static INT_PTR CALLBACK DrivenDlgProc(
      HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void DrivenControl::EditTrackParams(
      TimeValue t,
      ParamDimensionBase *dim,
      const TCHAR *pname,
      HWND hParent,
      IObjParam *ip,
      DWORD flags)
   {

   if (flags & EDITTRACK_BUTTON)
      {
      trackHWND = hParent;
		// Display the driven dialog.
		// Make this stack based, so as not to confuse everyone about who deletes the memory
		DrivenDlg the_dlg(this,dim,pname,ip,hParent);
      }
   }

DrivenDlg::DrivenDlg(
      DrivenControl *cont,
      ParamDimensionBase *dim,
      const TCHAR *pname,
      IObjParam *ip,
      HWND hParent)
   {
   this->cont = NULL;
   this->ip   = ip;
   this->dim  = dim;
   valid = FALSE;

	HoldSuspend hs;
   ReplaceReference(0,cont);
	hs.Resume();
	// Create a modal dialog box that returns when the dialog is closed
	DialogBoxParam(hInstance,MAKEINTRESOURCE(IDD_DRIVENPARAMS), hParent,DrivenDlgProc,(LPARAM)this);
   }

DrivenDlg::~DrivenDlg()
   {
	HoldSuspend hs;
   DeleteAllRefsFromMe();
   }

void DrivenDlg::Invalidate()
   {
   valid = FALSE;
   }

void DrivenDlg::Update()
   {
   if (!valid && hWnd) {
      valid = TRUE;
      }
   }

void DrivenDlg::SetupUI(HWND hWnd)
   {
   this->hWnd = hWnd;

   SetupList();

   valid = FALSE;
   Update();
   }

void DrivenDlg::SetupList()
   {
//loop through list getting names
//nuke old lis
	SendMessage(GetDlgItem(hWnd,IDC_LIST1), LB_RESETCONTENT,0,0);
	SendMessage(GetDlgItem(hWnd,IDC_LIST1), LB_SETCOUNT,0,0);
   if (cont->driverPresent)
      {
      for (int i=0; i<cont->blockID.Count(); i++) 
         {
         int id = cont->blockID[i];
         int subid = cont->subID[i];
         if (id < cont->driver->Blocks.Count())
            {
            TSTR name = cont->driver->Blocks[id]->SubAnimName(subid, true);
            SendMessage(GetDlgItem(hWnd,IDC_LIST1),
               LB_ADDSTRING,0,(LPARAM)(const TCHAR*)name);
            }
      }
      }
   }

void DrivenDlg::SetButtonStates()
   {
   int sel = SendMessage(GetDlgItem(hWnd,IDC_LIST1),
         LB_GETCURSEL,0,0);
   if (sel!=LB_ERR) {
      if ((cont->blockID.Count() == 0) || (cont->subID.Count()==0))
         {
         EnableWindow(GetDlgItem(hWnd,IDC_REMOVE),FALSE);
         EnableWindow(GetDlgItem(hWnd,IDC_COLLAPSE),FALSE);
         }
      else {
         EnableWindow(GetDlgItem(hWnd,IDC_REMOVE),TRUE);
         EnableWindow(GetDlgItem(hWnd,IDC_COLLAPSE),TRUE);
         }
      }
   else
      {
      EnableWindow(GetDlgItem(hWnd,IDC_REMOVE),FALSE);
      EnableWindow(GetDlgItem(hWnd,IDC_COLLAPSE),FALSE);
      }
   }

BOOL DriverTrackViewFilter :: proc(Animatable *anim, Animatable *client,int subNum)
{
//make sure the parent is not a driven or 
if ( anim->ClassID() ==DRIVERBLOCK_CONTROL_CLASS_ID)
   return TRUE;
return FALSE;
}


void DrivenDlg::WMCommand(int id, int notify, HWND hCtrl)
   {
	switch (id)
	{
	case IDC_LIST_NAME:
		{

         int sel = SendMessage(GetDlgItem(hWnd,IDC_LIST1),
            LB_GETCURSEL,0,0);
         if (sel!=LB_ERR) {
            if ((cont->blockID.Count() == 0) || (cont->subID.Count()==0))
               {
               EnableWindow(GetDlgItem(hWnd,IDC_REMOVE),FALSE);
               EnableWindow(GetDlgItem(hWnd,IDC_COLLAPSE),FALSE);
               }
            else {
               EnableWindow(GetDlgItem(hWnd,IDC_REMOVE),TRUE);
               EnableWindow(GetDlgItem(hWnd,IDC_COLLAPSE),TRUE);
               }
            }

         break;
         }
      case IDC_LIST1:
         if (notify==LBN_SELCHANGE) {
            SetButtonStates();            
            }

         break;
      case IDC_LINK:
         {
         if (!cont->driverPresent)
            {
            DriverTrackViewFilter filter;
            TrackViewPick res;
            BOOL DriverOK = GetCOREInterface()->TrackViewPickDlg(hWnd,&res,&filter);
            if (DriverOK && (res.anim != NULL))
               {
               cont->ReplaceReference(1,res.anim,FALSE);
               cont->propBlockID = -1;
               cont->propSubID = -1;

               int OK = DialogBoxParam  (hInstance, MAKEINTRESOURCE(IDD_ADDNEWLINK),
                  hWnd, NewLinkDlgProc, (LPARAM)cont);
         
               if ((OK) && (cont->propSubID != -1) && (cont->propSubID != -1))
                  {
                  cont->AddControl(cont->propBlockID,cont->propSubID);
                  SetupList();
                  }

               }
            int sel = SendMessage(GetDlgItem(hWnd,IDC_LIST1),
               LB_GETCURSEL,0,0);

            if ((cont->blockID.Count() == 0) || (cont->subID.Count()==0))
               {
               EnableWindow(GetDlgItem(hWnd,IDC_REMOVE),FALSE);
               EnableWindow(GetDlgItem(hWnd,IDC_COLLAPSE),FALSE);
                  
               }
            else {
               if (sel!=LB_ERR)
                  {
                  EnableWindow(GetDlgItem(hWnd,IDC_REMOVE),TRUE);
                  EnableWindow(GetDlgItem(hWnd,IDC_COLLAPSE),TRUE);
                  }
               }

            }
         else
            {
            int OK = DialogBoxParam  (hInstance, MAKEINTRESOURCE(IDD_ADDNEWLINK),
               hWnd, NewLinkDlgProc, (LPARAM)cont);
         
            if ( (OK)  && (cont->propSubID != -1) && (cont->propSubID != -1))
               {
               cont->AddControl(cont->propBlockID,cont->propSubID);
               SetupList();
               }
            }
         Change(TRUE);
         break;
         }
      case IDC_REMOVE:
         {
         int sel = SendMessage(GetDlgItem(hWnd,IDC_LIST1),
            LB_GETCURSEL,0,0);
         cont->RemoveControl(sel);
         SendMessage(GetDlgItem(hWnd,IDC_LIST1),
            LB_DELETESTRING,sel,0);
         SetupList();
         sel = SendMessage(GetDlgItem(hWnd,IDC_LIST1),
            LB_GETCURSEL,0,0);

         if ((cont->blockID.Count() == 0) || (cont->subID.Count()==0))
            {
            EnableWindow(GetDlgItem(hWnd,IDC_REMOVE),FALSE);
            EnableWindow(GetDlgItem(hWnd,IDC_COLLAPSE),FALSE);
            }
         else {
            if (sel!=LB_ERR)
               {
               EnableWindow(GetDlgItem(hWnd,IDC_REMOVE),TRUE);
               EnableWindow(GetDlgItem(hWnd,IDC_COLLAPSE),TRUE);
               }
            }

         Change(TRUE);

         break;
         }
      case IDC_COLLAPSE:
         {
			SendMessage(GetDlgItem(hWnd,IDC_LIST1), LB_GETCURSEL,0,0);
         cont->CollapseControl();
         EndDialog(hWnd,1);
         break;
         }
      case IDOK:
         EndDialog(hWnd,1);
         break;
      case IDCANCEL:
         EndDialog(hWnd,0);
         break;
      }
   }


void DrivenDlg::Change(BOOL redraw)
   {
   cont->NotifyDependents(FOREVER,PART_ALL,REFMSG_CHANGE);
   UpdateWindow(GetParent(hWnd));   
	if (redraw) 
		ip->RedrawViews(ip->GetTime());
   }


class CheckForNonDrivenDlg : public DependentEnumProc {
	public:     
	   BOOL non;
	   ReferenceMaker *me;
	CheckForNonDrivenDlg(ReferenceMaker *m)
		: non(FALSE)
		, me(m)
	{ }
	int proc(ReferenceMaker *rmaker)
	{
		   if (rmaker==me) return DEP_ENUM_CONTINUE;
		   if (rmaker->SuperClassID()!=REF_MAKER_CLASS_ID &&
			   rmaker->ClassID()!=Class_ID(DRIVENDLG_CLASS_ID,0)) {
				   non = TRUE;
				   return DEP_ENUM_HALT;
		   }
		   return DEP_ENUM_SKIP; // just look at direct dependents
	   }
   };

void DrivenDlg::MaybeCloseWindow()
   {
   CheckForNonDrivenDlg check(cont);
   cont->DoEnumDependents(&check);
   if (!check.non) {
      PostMessage(hWnd,WM_CLOSE,0,0);
      }
   }



RefResult DrivenDlg::NotifyRefChanged(
      const Interval& /*changeInt*/, 
      RefTargetHandle hTarget, 
      PartID& partID,  
	  RefMessage message, 
	  BOOL /*propagate*/)
   {
   switch (message) {
      case REFMSG_CHANGE:
         Invalidate();        
         break;
      
      case REFMSG_REF_DELETED:
         MaybeCloseWindow();
         break;

	case REFMSG_TARGET_DELETED:
		MaybeCloseWindow();
		cont = NULL;
		break;
      }
   return REF_SUCCEED;
   }

static INT_PTR CALLBACK DrivenDlgProc(
      HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
   {
   DrivenDlg *dlg = DLGetWindowLongPtr<DrivenDlg*>(hWnd);

   switch (msg) {
      case WM_INITDIALOG:
         {
         dlg = (DrivenDlg*)lParam;
         DLSetWindowLongPtr(hWnd, lParam);
         dlg->SetupUI(hWnd);
         SendMessage(GetDlgItem(hWnd,IDC_LIST1),
            LB_SETCURSEL,0,0);
         int sel = SendMessage(GetDlgItem(hWnd,IDC_LIST1),
            LB_GETCURSEL,0,0);
         if (sel==-1) 
            {
            EnableWindow(GetDlgItem(hWnd,IDC_REMOVE),FALSE);
            EnableWindow(GetDlgItem(hWnd,IDC_COLLAPSE),FALSE);
            }

         break;

         }
      case WM_COMMAND:
         dlg->WMCommand(LOWORD(wParam),HIWORD(wParam),(HWND)lParam);                
         break;

      case WM_PAINT:
         dlg->Update();
         return 0;         
      
      case WM_CLOSE:
         DestroyWindow(hWnd);       
         break;

      case WM_DESTROY:                 
		// delete dlg; // This is stack based, and there is no reason to destroy it here
         break;
   
      case CC_COLOR_BUTTONDOWN:
         theHold.Begin();
         break;
      case CC_COLOR_BUTTONUP:
         if (HIWORD(wParam)) theHold.Accept(GetString(IDS_DS_PARAMCHG));
         else theHold.Cancel();
         break;
      case CC_COLOR_CHANGE: {
		// Explanation of what wParam and lParam is below
		// int i = LOWORD(wParam);
		// IColorSwatch *cs = (IColorSwatch*)lParam;
		int sel = SendMessage(GetDlgItem(hWnd,IDC_LIST1), LB_GETCURSEL,0,0);
         if (sel != -1)
            {
            if (HIWORD(wParam)) theHold.Begin();
            if (HIWORD(wParam)) {
               theHold.Accept(GetString(IDS_DS_PARAMCHG));
               }
            }
         break;
      }

      
      default:
         return FALSE;
      }
   return TRUE;
   }





INT_PTR CALLBACK NewLinkDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
   DrivenControl *slv = DLGetWindowLongPtr<DrivenControl*>(hWnd);

   static Tab<int> sid,bid;

   switch (msg) {
   case WM_INITDIALOG:
      {
      sid.ZeroCount();
      bid.ZeroCount();
      slv = (DrivenControl*)lParam;
      DLSetWindowLongPtr(hWnd, lParam);

//goto driver look at all sub block
      int count = slv->driver->Blocks.Count();
			for (int i = 0; i < count;i++)
         {
         TSTR blockName = slv->driver->SubAnimName(i+1, true);
         for (int j = 0;j < slv->driver->Blocks[i]->controls.Count();j++)
            {
            TSTR subName = slv->driver->Blocks[i]->SubAnimName(j, true);
            TSTR finalName = blockName + _T(" ") + subName;
//check if control is the same as ours
            if (slv->sub == NULL)
               {
               if (slv->driver->Blocks[i]->controls[j]->SuperClassID() == slv->SuperClassID())  
                  {
//add to list box
                  sid.Append(1,&j,1);
                  bid.Append(1,&i,1);
							SendMessage(GetDlgItem(hWnd,IDC_LIST1), LB_ADDSTRING,0,(LPARAM)(const TCHAR*)finalName);
                  }
               }
            else if (slv->driver->Blocks[i]->controls[j]->ClassID() == slv->sub->ClassID() )  
               {
//add to list box
               sid.Append(1,&j,1);
               bid.Append(1,&i,1);
               SendMessage(GetDlgItem(hWnd,IDC_LIST1),
                  LB_ADDSTRING,0,(LPARAM)(const TCHAR*)finalName);
               }
            }
         }  

			SendMessage(GetDlgItem(hWnd,IDC_LIST1), LB_SETCURSEL,0,0);
      CenterWindow(hWnd,GetParent(hWnd));
      break;
      }
      


   case WM_COMMAND:
      switch (LOWORD(wParam)) {
      case IDOK:
         {
         int sel = SendMessage(GetDlgItem(hWnd,IDC_LIST1),
                  LB_GETCURSEL,0,0);
         if (sel >=0)
            {
            slv->propBlockID = bid[sel];
            slv->propSubID = sid[sel];
            slv->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
            }

         EndDialog(hWnd,1);
         break;
         }
      case IDCANCEL:
         EndDialog(hWnd,0);
         break;
      }
      break;

   default:
      return FALSE;
   }
   return TRUE;
}

