//**************************************************************************/
// Copyright (c) 2009 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#pragma once

#include <ref.h>
#include <mouseman.h>
#include <Geom/matrix3.h>

// forward declaration
class SunDriver;
class INode;
class IObjCreate;
class ClassDesc;

// This is the class used to manage the creation process of the ring array
// in the 3D viewports.
class SunDriverCreationManager : public MouseCallBack, ReferenceMaker 
{
private:
	INode* node0;
	// This holds a pointer to the SunDriver object.  This is used
	// to call methods on the sun driver such as BeginEditParams() and
	// EndEditParams() which bracket the editing of the UI parameters.
	SunDriver* theDriver;
	// This holds the interface pointer for calling methods provided
	// by MAX.
	IObjCreate *createInterface;

	ClassDesc *cDesc;
	// This holds the nodes TM relative to the CP
	Matrix3 mat;
	// This holds the initial mouse point the user clicked when
	// creating the ring array.
	IPoint2 pt0;
	// This holds the center point of the ring array
	Point3 center;
	// This flag indicates the dummy nodes have been hooked up to
	// the driver node and the entire system has been created.
	BOOL attachedToNode;
	bool daylightSystem;

	// This method is called to create a new SunDriver object (theDriver)
	// and begin the editing of the systems user interface parameters.
	void CreateNewDriver(HWND rollup);

	// This flag is used to catch the reference message that is sent
	// when the system plug-in itself selects a node in the scene.
	// When the user does this, the plug-in recieves a reference message
	// that it needs to respond to.
	int ignoreSelectionChange;

	virtual void GetClassName(MSTR& s, bool localized) const override { s = _M("SunDriverCreationManager"); } // from Animatable

	// --- Inherited virtual methods of ReferenceMaker ---
	// use named constants to keep track of references
	static const int NODE_0_REFERENCE_INDEX = 0;
	static const int SUN_DRIVER_REFERENCE_INDEX = 1;
	static const int REFERENCE_COUNT = 2;

	// This returns the number of references this item has.
	virtual int NumRefs();

	// This methods retrieves the ith reference.
	virtual RefTargetHandle GetReference(int i);

	// This methods stores the ith reference.
	virtual void SetReference(int i, RefTargetHandle rtarg);

	// This method recieves the change notification messages sent
	// when the the reference target changes.
	RefResult NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, 
		PartID& partID, RefMessage message, BOOL propagate);

public:
	// This method is called just before the creation command mode is
	// pushed on the command stack.
	void Begin( IObjCreate *ioc, ClassDesc *desc, bool daylight );
	// This method is called after the creation command mode is finished 
	// and is about to be popped from the command stack.
	void End();

	// Constructor.
	SunDriverCreationManager();
	
	// --- Inherited virtual methods from MouseCallBack
	// This is the method that handles the user / mouse interaction
	// when the system plug-in is being created in the viewports.
	int proc( HWND hwnd, int msg, int point, int flag, IPoint2 m );

#if defined(NO_MOTION_PANEL)
	bool IsActive() const { return theDriver != NULL; }
#endif	// defined(NO_MOTION_PANEL)
};

