//**************************************************************************/
// Copyright (c) 2006 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
// DESCRIPTION: Defines a class for monitoring a RefTarg for deletion.
// AUTHOR: Larry.Minton - created April.20.2006
//***************************************************************************/

#include "IRefTargMonitor.h"
#include "IRefTargMonitorClass.h"
#include "IIndirectRefMaker.h"

class RefTargMonitorClass : public ReferenceTarget, public IRefTargMonitorClass, public IRefTargMonitor, public IIndirectReferenceMaker {
public:
	RefTargMonitorRefMaker *theRefTargWatcher;
	BYTE loadVer;
	bool suspendDelMessagePassing;
	bool suspendSetMessagePassing;
	bool deleteMe;
	bool shouldPersistIndirectRef;

	RefTargMonitorClass();
	RefTargMonitorClass(RefTargetHandle theRefTarg);
	~RefTargMonitorClass();

	// IRefTargMonitorClass

	void SetRefTarg(RefTargetHandle theRefTarg);
	RefTargetHandle GetRefTarg();
	bool GetPersist();
	void SetPersist(bool persist);

	// IRefTargMonitor

	RefResult ProcessRefTargMonitorMsg(
		const Interval& changeInt, 
		RefTargetHandle hTarget, 
		PartID& partID,  
		RefMessage message,
		bool fromRefTarg,
		bool propagate, 
		RefTargMonitorRefMaker* caller);

	int ProcessEnumDependents(DependentEnumProc* dep);

	// IIndirectReferenceMaker

	int NumIndirectRefs();
	RefTargetHandle GetIndirectReference(int i);
	void SetIndirectReference(int i, RefTargetHandle rtarg);
	BOOL ShouldPersistIndirectRef(RefTargetHandle ref);

	// ReferenceMaker

	RefResult NotifyRefChanged(
		const Interval& changeInt, 
		RefTargetHandle hTarget, 
		PartID& partID,  
		RefMessage message, 
		BOOL propagate);

	RefTargetHandle Clone(RemapDir& remap);

	IOResult Load(ILoad *iload);
	IOResult Save(ISave *isave);
	int RemapRefOnLoad(int iref);

	void DeleteThis();
	void GetClassName(MSTR& s, bool localized) const override {s=_T("RefTargMonitor");} 
	Class_ID ClassID() {return REFTARGMONITOR_CLASS_ID;}
	SClass_ID SuperClassID() {return REF_TARGET_CLASS_ID;}

	int NumRefs();
	ReferenceTarget* GetReference(int i);
private:
	virtual void SetReference(int i, RefTargetHandle rtarg);
public:

	void* GetInterface(DWORD in_interfaceID)
	{
		if (IID_IINDIRECTREFERENCEMAKER == in_interfaceID) 
			return (IIndirectReferenceMaker*)this; 
		else if (IID_REFTARG_MONITOR == in_interfaceID)
			return (IRefTargMonitor*)this;
		else 
			return ReferenceTarget::GetInterface(in_interfaceID);
	}


	BaseInterface* GetInterface(Interface_ID in_interfaceID){ 
		if (in_interfaceID == IID_REFTARGMONITOR_CLASS) 
			return (IRefTargMonitorClass*)this; 
		else 
			return ReferenceTarget::GetInterface(in_interfaceID);
	}
};
