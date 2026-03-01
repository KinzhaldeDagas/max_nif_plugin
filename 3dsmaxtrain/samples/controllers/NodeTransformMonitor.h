//**************************************************************************/
// Copyright (c) 2005 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
// DESCRIPTION: Defines a class for monitoring a Node for its PART_TM messages.
// AUTHOR: Larry.Minton - created May.14.2005
//***************************************************************************/

#include "INodeTransformMonitor.h"
#include "IIndirectRefMaker.h"

class NodeTransformMonitor : public ReferenceTarget, public INodeTransformMonitor, public IRefTargMonitor, public IIndirectReferenceMaker {
public:
	RefTargMonitorRefMaker *theNodeWatcher;
	BYTE loadVer;
	bool suspendTMMessagePassing;
	bool suspendDelMessagePassing;
	bool suspendFlagNodesMessagePassing;
	bool suspendProcessEnumDependents;
	bool suspendSetMessagePassing;
	bool deleteMe;
	bool forwardPartTM;
	bool forwardFlagNodes;
	bool forwardEnumDependents;

	NodeTransformMonitor();
	NodeTransformMonitor(INode *theNode, bool forwardTransformChangeMsgs = true, bool forwardFlagNodesMsgs = true, bool forwardEnumDependentsCalls = true);
	~NodeTransformMonitor();

	// INodeTransformMonitor

	void SetNode(INode *theNode);
	INode* GetNode();
	void SetForwardTransformChangeMsgs(bool state);
	bool GetForwardTransformChangeMsgs();
	void SetForwardFlagNodesMsgs(bool state);
	bool GetForwardFlagNodesMsgs();
	void SetForwardEnumDependentsCalls(bool state);
	bool GetForwardEnumDependentsCalls();

	// IRefTargMonitor

	RefResult ProcessRefTargMonitorMsg(
		const Interval& changeInt, 
		RefTargetHandle hTarget, 
		PartID& partID,  
		RefMessage message,
		bool fromNode,
		bool propagate, 
		RefTargMonitorRefMaker* caller);

	int ProcessEnumDependents(DependentEnumProc* dep);

	// IIndirectReferenceMaker

	int NumIndirectRefs();
	RefTargetHandle GetIndirectReference(int i);
	void SetIndirectReference(int i, RefTargetHandle rtarg);
	BOOL ShouldPersistIndirectRef(RefTargetHandle ref) { UNUSED_PARAM(ref); return TRUE; }

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
	void GetClassName(MSTR& s, bool localized) const override { UNUSED_PARAM(localized); s = _T("NodeTransformMonitor") ;}
	Class_ID ClassID() {return NODETRANSFORMMONITOR_CLASS_ID;}
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
		if (in_interfaceID == IID_NODETRANSFORMMONITOR) 
			return (INodeTransformMonitor*)this; 
		else 
			return ReferenceTarget::GetInterface(in_interfaceID);
	}
};
