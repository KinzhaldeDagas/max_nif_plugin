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

#ifndef __NOTIFIER__H__
#define __NOTIFIER__H__

#include <max.h>
#include <Maxapi.h>

class NotifyCallback
{
public:
	virtual RefResult NotifyFunc(const Interval& changeInt,
            RefTargetHandle hTarget, PartID& partID,
            RefMessage message, BOOL propagate)=0;
};

class NotifyMgr : public ReferenceTarget {
   private:      
      NotifyCallback *mpNotifyCB;
      Tab<RefTargetHandle> mpReferences;
   public:
      // --- Inherited Virtual Methods From Animatable ---
      void GetClassName(MSTR& s, bool localized) const override { s = _T("Notifier"); }

      // --- Inherited Virtual Methods From ReferenceMaker ---
      RefResult NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget,
         PartID& partID, RefMessage message, BOOL propagate);
      int NumRefs();
      RefTargetHandle GetReference(int i);
      void SetReference(int i, RefTargetHandle rtarg);
	  void DeleteThis();
      // --- Methods From NotifyMgr ---
      NotifyMgr();
      ~NotifyMgr();
	  void SetNumberOfReferences(int num);
      BOOL CreateReference(int which, RefTargetHandle hTarget);
      BOOL RemoveReference(int which);
	  void RemoveAllReferences(); 
      void SetNotifyCB(NotifyCallback *cb);
      void ResetNotifyFunc();
};

#endif
