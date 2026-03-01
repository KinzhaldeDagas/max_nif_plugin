//**************************************************************************/
// Copyright (c) 2011 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#pragma once

#include <CATAPI/IHub.h>

/**********************************************************************
 * IHub: Published functions for the Hub
 */

class IHubFP : public CatAPI::IHub {
public:
	enum {
//		fnCollapsePoseToCurrLayer,
//		fnCollapseTimeRangeToCurrLayer,
//		fnResetTransforms,

//		fnKeyFreeform,
		fnAddArm,
		fnAddLeg,
		fnAddSpine,
		fnAddTail,

		propGetPinHub,
		propSetPinHub,
		propTMOrient,
		propDangleCtrl
	};

	BEGIN_FUNCTION_MAP

		VFN_2(fnAddArm, AddArm, TYPE_BOOL, TYPE_BOOL);
		VFN_2(fnAddLeg, AddLeg, TYPE_BOOL, TYPE_BOOL);
		VFN_1(fnAddSpine, AddSpine, TYPE_INT);
		VFN_1(fnAddTail, AddTail, TYPE_INT);

		PROP_FNS(propGetPinHub, GetPinHub, propSetPinHub, SetPinHub, TYPE_BOOL);
//		RO_PROP_FN(propTMOrient,	GettmOrient,	TYPE_MATRIX3_BR);
//		RO_PROP_FN(propDangleCtrl,	GetDangleCtrl,	TYPE_CONTROL);

	END_FUNCTION_MAP

	FPInterfaceDesc* GetDesc();

	static FPInterfaceDesc* GetFnPubDesc();
};