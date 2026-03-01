//**************************************************************************/
// Copyright (c) 2007 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
#pragma unmanaged
#include "suntypes.h"
#include "sunclass.h"
#include "natLight.h"
#include "compass.h"
//#include "citylist.h"
#include "DaylightSystemFactory2.h"
#include <INodeMonitor.h>
#include <istdplug.h>

///-----------------------------------------------------------------------------
/// Maxscript Function Publishing 
///-----------------------------------------------------------------------------

// Sole instance and FP static interface descriptor.
DaylightSystemFactory2* DaylightSystemFactory2::sTheDaylightSystemFactory = nullptr;
float DaylightSystemFactory2::kDefaultCompassDiameter = 100.0f;
float DaylightSystemFactory2::kDefaultOrbitalScale = 70.0f;

void DaylightSystemFactory2::RegisterInstance()
{
	if(nullptr == sTheDaylightSystemFactory)
	{
		sTheDaylightSystemFactory  = new DaylightSystemFactory2(
			IID_DAYLIGHT_SYSTEM_FACTORY2,
			_T("DaylightSystemFactory2"), // interface name used by maxscript - don't localize it!
			0, 
			NULL, 
			FP_CORE,

			DaylightSystemFactory2::eCreateDaylightSystem, _T("Create"), 0, TYPE_INTERFACE, 0, 2, 
			_T("sunClass"), 0, TYPE_CLASS, f_inOut, FPP_IN_PARAM, f_keyArgDefault, NULL,
			_T("skyClass"), 0, TYPE_CLASS, f_inOut, FPP_IN_PARAM, f_keyArgDefault, NULL,

			p_end
		);
	}
}

DaylightSystemFactory2& DaylightSystemFactory2::GetInstance()
{
	return *sTheDaylightSystemFactory;
}

void DaylightSystemFactory2::Destroy()
{
	delete sTheDaylightSystemFactory;
	sTheDaylightSystemFactory = nullptr;
}

FPInterface* DaylightSystemFactory2::fpCreate(
	ClassDesc* sunCD, 
	ClassDesc* skyCD)
{
	IDaylightSystem2* daylight2 = NULL;
	Class_ID sunCDCid;
	if (sunCD)
		sunCDCid = sunCD->ClassID();
	Class_ID skyCDCid;
	if (skyCD)
		skyCDCid = skyCD->ClassID();
	INode* assemblyHeadNode = Create(
		daylight2, 
		(sunCD ? &sunCDCid : NULL), 
		(skyCD ? &skyCDCid : NULL));

	if (NULL == assemblyHeadNode != NULL)
	{
		daylight2 = NULL;
	}
	return daylight2;
}

///-----------------------------------------------------------------------------
/// DaylightSystemFactory2 Implementation
///-----------------------------------------------------------------------------


INode* DaylightSystemFactory2::Create(
	IDaylightSystem2*& pDaylight,
	const Class_ID* sunClassID,
	const Class_ID* skyClassID)
{
	Interface* ip = GetCOREInterface();
	IObjCreate* pICreate = ip->GetIObjCreate();
	DbgAssert(pICreate != NULL);

	//Create a new SunDriver - which controls the time and geographic location of the system
	SunDriver* pSunDriver = new SunDriver(true);
	DbgAssert(pSunDriver != NULL);

	// Create the NaturalLightAssembly - it is a helper object controls the sun and sky components.
	NatLightAssembly* pNatLightHelper = NULL;
	INode* newNode = NatLightAssembly::CreateAssembly(pNatLightHelper, pICreate, sunClassID, skyClassID);
	DbgAssert(newNode != NULL);
	DbgAssert(pNatLightHelper != NULL);

	//Create and set several controllers.
	pNatLightHelper->SetMultController(new DrivenControl(pSunDriver,LIGHT_MULT));
	pNatLightHelper->SetIntenseController(new DrivenControl(pSunDriver,LIGHT_INTENSE));
	pNatLightHelper->SetSkyCondController(new DrivenControl(pSunDriver,LIGHT_SKY));	
	newNode->SetTMController(new DrivenControl(pSunDriver,LIGHT_TM));

	// Create the compass object 
	CompassRoseObject* compassObj = (CompassRoseObject*)pICreate->CreateInstance(HELPER_CLASS_ID, COMPASS_CLASS_ID);
	DbgAssert(compassObj != NULL);
	//Set the compass axis length to a default length just for visual purpose.
	//This parameter doesnt affect the precision of the sun.
	compassObj->axisLength = kDefaultCompassDiameter;

	//Create a dummy node to hold everything together.
	//This results on the compass node being the parent of the daylight node.
	INode* compassNode = pICreate->CreateObjectNode(compassObj);
	// Attach the new node as a child of the central node.
	compassNode->AttachChild(newNode);

	//Create a node monitor.
	if(pSunDriver->theObjectMon)
	{
		INodeMonitor* nodeMon = static_cast<INodeMonitor*>(pSunDriver->theObjectMon->GetInterface(IID_NODEMONITOR));
		DbgAssert(nodeMon != NULL);
		nodeMon->SetNode(newNode);
	}

	//the driver references the point so it can get
	//notified when the	user rotates it
	pSunDriver->ReplaceReference(REF_POINT, compassNode );

	//We have to create controllers for look at and multiplier, these controllers are not
	//created in the constructor.

	//create a new PRS controller for the sun's lookat
	Control* pSunLookAt = static_cast<Control*>(CreateInstance(CTRL_MATRIX3_CLASS_ID, Class_ID(PRS_CONTROL_CLASS_ID,0)));
	pSunDriver->ReplaceReference(REF_LOOKAT, pSunLookAt);

	//Initialize the rotation for the look at controller 
	Control* pLookatRot = static_cast<Control*>(CreateInstance(CTRL_ROTATION_CLASS_ID, Class_ID(LOOKAT_CONSTRAINT_CLASS_ID, 0)));
	Animatable* a = pLookatRot;
	ILookAtConstRotation* ipLookatRot = GetILookAtConstInterface(a);
	DbgAssert(ipLookatRot != NULL);
	ipLookatRot->AppendTarget(pSunDriver->thePoint);
	ipLookatRot->SetTargetAxis(2);
	ipLookatRot->SetTargetAxisFlip(true);
	ipLookatRot->SetVLisAbs(false);
	ipLookatRot->SetVectorLineLength(100.0f);
	pSunDriver->theLookat->SetRotationController(pLookatRot);

	//Create and a new controller for the sun's multiplier.
	Control* pMultiplier = static_cast<Control*>(
		CreateInstance(CTRL_FLOAT_CLASS_ID, Class_ID(HYBRIDINTERP_FLOAT_CLASS_ID,0)));
	pSunDriver->ReplaceReference(REF_MULTIPLIER, pMultiplier );

	//Set the orbital scale to a default value, just for visual purpose.
	//This parameter doesnt affect the precision of the sun.
	pSunDriver->SetRad(ip->GetTime(),kDefaultOrbitalScale);

	//We are creating a daylight system.
	pSunDriver->daylightSystem = true;

	//fill in the IDaylightSystem pointer for access to sun and sky objects.
	pDaylight = dynamic_cast<IDaylightSystem2*>(pNatLightHelper->GetInterface(IID_DAYLIGHT_SYSTEM2));

	return newNode;
}


