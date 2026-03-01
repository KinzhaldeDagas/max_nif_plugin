//**************************************************************************/
// Copyright (c) 2006 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
/**********************************************************************
	FILE: SimpleFaceDataValidator.cpp

	DESCRIPTION:	Implementation of validator ojbects for SimpleFaceDataAttrib.
					Class definitions for:
					- face index validator
					- parameter type validator
					- parameter array size validator
					- class id format validator

	AUTHOR: ktong - created 02.17.2006
***************************************************************************/

#include "SimpleFaceData.h"
#include "SimpleFaceDataValidator.h"
#include "SimpleFaceDataCommon.h"
#include <maxscript/maxscript.h>
#include <maxscript/foundation/functions.h>
#include <triobj.h>
#include <polyobj.h>

// ----------------------------------------------------------------------------
bool ParamIndexValidator::Validate(FPInterface* fpi, FunctionID /*fid*/, int /*paramNum*/, FPValue& val, TSTR& msg)
{
	DbgAssert(fpi);

	ISimpleFaceDataChannel* pCA = static_cast<ISimpleFaceDataChannel*>(fpi);
	if ((val.i >=0) && (val.i < pCA->NumFaces())) {
		return true;
	} else {
		msg = GetString(IDS_VALIDATE_ERROR_INDEX);
	}
	return false;
}

// ----------------------------------------------------------------------------
bool ParamTypeValidator::Validate(FPInterface* fpi, FunctionID /*fid*/, int /*paramNum*/, FPValue& val, TSTR& msg)
{
	DbgAssert(fpi);
	ISimpleFaceDataChannel* pCA = static_cast<ISimpleFaceDataChannel*>(fpi);
	
	bool res = false;
	try {
		// Use the maxscript SDK to try a type conversion to a compatible type
		FPValue temp;
		InterfaceFunction::val_to_FPValue(val.v, (ParamType2)(pCA->ChannelType()), temp);
		InterfaceFunction::release_param(temp, (ParamType2)(pCA->ChannelType()), val.v);
		res = true;
	} catch (...) {
		msg = GetString(IDS_VALIDATE_ERROR_TYPE);
		res = false;
	}
	return res;
}

// ----------------------------------------------------------------------------
bool ParamArrayValidator::Validate(FPInterface* fpi, FunctionID /*fid*/, int /*paramNum*/, FPValue& val, TSTR& msg)
{
	DbgAssert(fpi);

	ISimpleFaceDataChannel* pCA = static_cast<ISimpleFaceDataChannel*>(fpi);

	bool res = false;
	try  {
		// Use the maxscript SDK to try a type conversion to a compatible array type
		FPValue temp;
		InterfaceFunction::val_to_FPValue(val.v, (ParamType2)(pCA->ChannelType() + TYPE_TAB), temp);
		res = true;
		InterfaceFunction::release_param(temp, (ParamType2)(pCA->ChannelType() + TYPE_TAB), val.v);
		if (!is_array(val.v) || ((Array*)(val.v))->size != (int)pCA->NumFaces()) {
			msg = GetString(IDS_VALIDATE_ERROR_ARRAYSIZE);
			res = false;
		}
	} catch (...) {
		msg = GetString(IDS_VALIDATE_ERROR_TYPE);
		res = false;
	}
	return res;
}

// ----------------------------------------------------------------------------
bool ParamSelectValidator::Validate(FPInterface* fpi, FunctionID /*fid*/, int /*paramNum*/, FPValue& val, TSTR& msg)
{
	DbgAssert(fpi);

	ISimpleFaceDataChannel* pCA = static_cast<ISimpleFaceDataChannel*>(fpi);
	if (val.bits->GetSize() <= pCA->NumFaces()) { // check bitarray size
		return true;
	} else {
		msg = GetString(IDS_VALIDATE_ERROR_SELECTSIZE);
	}
	return false;
}

// ----------------------------------------------------------------------------
bool ParamClassIDValidator::Validate(FPInterface* /*fpi*/, FunctionID /*fid*/, int /*paramNum*/, FPValue& val, TSTR& msg)
{
	if (is_tab(val.type) && (val.ptr_tab->Count() == 2)) { // check array and array size
		return true;
	} else {
		msg = GetString(IDS_VALIDATE_ERROR_CLASSID);
	}
	return false;
};

// ----------------------------------------------------------------------------
bool ParamObjectValidator::Validate(FPInterface* /*fpi*/, FunctionID /*fid*/, int /*paramNum*/, FPValue& val, TSTR& msg)
{
	Object* pObj = SimpleFaceDataCommon::FindBaseFromObject(val.obj);
	if (pObj && 
		(pObj->IsSubClassOf(triObjectClassID) ||
		pObj->IsSubClassOf(polyObjectClassID))
		){
		return true;
	} else {
		msg = GetString(IDS_VALIDATE_ERROR_OBJECT);
	}
	return false;
};
