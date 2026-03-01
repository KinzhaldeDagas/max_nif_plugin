//**************************************************************************/
// Copyright (c) 2024 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
#pragma once

#include "baseinterface.h"
#include "strclass.h"

namespace MaxSDK {

/** The IEditObjectContextProvider can be implemented as an additional interface
 * by any object to provide different edit contexts for the object when being
 * modified.  
 * The Command Panel checks the current modified object for this interface to
 * determine the context of open/close state and ordering of the rollups when
 * the object is being edited. This allows an object to implement different
 * persistent rollup states for different contexts.
 *
 * \see IRollupCallback::GetEditObjContext(MSTR& context) */
class IEditObjectContextProvider : public BaseInterface
{
public:
	static constexpr Interface_ID ID = Interface_ID(0x34d263d4, 0x14681510);
	Interface_ID GetID() override
	{
		return ID;
	}
	/** This method is used to retrieve the edit object context for the object.
	 * The context is used to determine the rollup ordering and open/close state
	 * for the object.
	 * 
	 * \note It is perfectly valid to have varying contexts based on internal
	 *       state.  
	 *
	 * \param context [out] The context string to be filled in by the method.  
	 * \return TRUE if a context is currently available and successfully
	 *         retrieved, FALSE otherwise. */
	virtual BOOL GetEditObjContext(MSTR& context) = 0;
};

}; // namespace MaxSDK
