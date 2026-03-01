//**************************************************************************/
// Copyright (c) 2008 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
// DESCRIPTION: Reference Notifier
// AUTHOR: Michael Zyracki created 2008
//***************************************************************************/
#include "notifier.h"

// Constructor
NotifyMgr::NotifyMgr() {
	mpReferences.ZeroCount();
   mpNotifyCB = NULL;
}

// Destructor. Remove any references we have made.
NotifyMgr::~NotifyMgr() {
	//RemoveAllReferences();
	DeleteAllRefsFromMe();
}

void NotifyMgr::DeleteThis() 
{
	//delete refs but' don't delete this, since it's static.
	DeleteAllRefsFromMe();
}  


// Notification function.
// When we get a callback we check to see if we have a registered notify
// function. If we do, we forward the notification to the notify function,
RefResult NotifyMgr::NotifyRefChanged(const Interval& changeInt,
   RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate) {
   RefResult res = REF_SUCCEED;

   if (mpNotifyCB) {
      res = mpNotifyCB->NotifyFunc(changeInt, hTarget, partID, message, propagate);
   }
   return res;
}

// Number of references we have made. Currently we allow one single
// reference only.
int NotifyMgr::NumRefs() {
   return mpReferences.Count();
}

// Return the n'th reference
RefTargetHandle NotifyMgr::GetReference(int i)
{
	if(i>=0&&i<mpReferences.Count())
	{
		return mpReferences[i];
	}
   return NULL;
}

// Set the n'th reference.
void NotifyMgr::SetReference(int i, RefTargetHandle rtarg)
{
	if(i>=0&&i<mpReferences.Count())
	{
		mpReferences[i] = rtarg;
	}
}

// Exported function that let someone make a reference from
// outside of this class.

//set tab to NULL and set all to NULL
void NotifyMgr::SetNumberOfReferences(int num)
{

	RemoveAllReferences();
	mpReferences.SetCount(num);
	for(int i =0;i<num;++i)
	{
		mpReferences[i] = NULL;
	}	
}

BOOL NotifyMgr::CreateReference(int which,RefTargetHandle hTarget) {
   ReplaceReference(which, hTarget);
   return TRUE;
}

// Exported function that let someone remove a reference.
BOOL NotifyMgr::RemoveReference(int i)
{
	if(i>=0&&i<mpReferences.Count())
	{
		DeleteReference(i);
		mpReferences[i] = NULL;
   }
   return TRUE;
}

void NotifyMgr::RemoveAllReferences() 
{
	for(int i=0;i<mpReferences.Count();++i)
	{
		DeleteReference(i);
		mpReferences[i] = NULL;
	}
	mpReferences.ZeroCount();
}
// Exported function to set the function that will be used as a
// notification callback.
void NotifyMgr::SetNotifyCB(NotifyCallback *cb) {
   mpNotifyCB = cb;
}

// Exported function to clear the notification callback.
void NotifyMgr::ResetNotifyFunc() {
   RemoveAllReferences();
   mpNotifyCB = NULL;
}
