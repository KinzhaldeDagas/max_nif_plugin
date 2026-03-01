//**************************************************************************/
// Copyright (c) 2006 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
/**********************************************************************
 	FILE: SimpleFaceDataValidator.h

	DESCRIPTION:	Header file for validator ojbects for SimpleFaceData.
					Class declarations for:
					- face index validator
					- parameter type validator
					- parameter array size validator
					- class id format validator

	AUTHOR: ktong - created 02.17.2006
***************************************************************************/

#ifndef _SIMPLEFACEDATAVALIDATOR_H_
#define _SIMPLEFACEDATAVALIDATOR_H_

#include "ISimpleFaceDataChannel.h"

// face index validator
class ParamIndexValidator : public FPValidator {
	bool Validate(FPInterface* fpi, FunctionID fid, int paramNum, FPValue& val, TSTR& msg);
};

// value parameter type validator
class ParamTypeValidator : public FPValidator {
	bool Validate(FPInterface* fpi, FunctionID fid, int paramNum, FPValue& val, TSTR& msg);
};

// value array validator - size and element type
class ParamArrayValidator : public FPValidator {
	bool Validate(FPInterface* fpi, FunctionID fid, int paramNum, FPValue& val, TSTR& msg);
};

// bitarray size validator
class ParamSelectValidator : public FPValidator {
	bool Validate(FPInterface* fpi, FunctionID fid, int paramNum, FPValue& val, TSTR& msg);
};

// class id array format validator
class ParamClassIDValidator : public FPValidator {
	bool Validate(FPInterface* fpi, FunctionID fid, int paramNum, FPValue& val, TSTR& msg);
};

// object type validator. Emesh and Epoly only.
class ParamObjectValidator : public FPValidator {
	bool Validate(FPInterface* fpi, FunctionID fid, int paramNum, FPValue& val, TSTR& msg);
};

#endif
//EOF