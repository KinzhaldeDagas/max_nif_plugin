//**************************************************************************/
// Copyright (c) 2022 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#pragma once

#include <guiddef.h>
#include <CGuid.h>
#include "maxheap.h"
#include "strclass.h"
#include "utilexp.h"

// Forward declarations
class QString;

namespace MaxSDK
{
	struct MaxGuid : public GUID, public MaxHeapOperators
	{
		/**
		 * \brief Constructors.
		 */
		MaxGuid()
		{
			*this = CLSID_NULL;
		}
		UtilExport explicit MaxGuid(const char*);
		UtilExport explicit MaxGuid(const wchar_t*);
		UtilExport explicit MaxGuid(const QString&);
		explicit MaxGuid(const MSTR& idStr) : MaxGuid(idStr.data()) {}

		/**
		 * \brief Assignment operator.
		 */
		MaxGuid& operator=(const GUID& rhs)
		{
			*static_cast<GUID*>(this) = rhs;
			return *this;
		}
		/**
		 * \brief Less than operator needed for using MaxSDK::MaxGuid in unordered set, maps.
		 */
		UtilExport bool operator<(const MaxGuid& rhs) const;

		/**
		 * \brief Create a MaxGuild value with random guid value.
		 */
		UtilExport static MaxGuid CreateMaxGuid();

		/**
		 * \brief Returns a string representation of the guid value.
		 * \param lowerCaseResult If true, return string is lower case, otherwise is upper case.
		 * \param includeBraces If true, return string contains opening and closing braces.
		 * \return The string representation of the guid value
		 */
		UtilExport MSTR ToString(bool lowerCaseResult = true, bool includeBraces = false) const;
		/**
		 * \brief Converts a string representation of a guid value to a guid value.
		 * \param pString The string representation of a guid value. If null or an empty string, a guid value 
		 * corresponding to an invalid guid is used and the function returns true.
		 * \param guid The MaxGuid value to set.
		 * \param throwExceptionOnError If false and pString is not a proper string representation of a guid value, 
		 * a guid value corresponding to an invalid guid is used and the function returns true. If true and pString is not 
		 * a proper string representation of a guid value, a MaxException is thrown. 
		 * \return True if null or an empty string, or if string could be converted to a guid value.
		 */
		UtilExport static bool StringToMaxGuid(const MCHAR* pString, MaxGuid& guid, bool throwExceptionOnError = false);
		/**
		 * \brief Returns whether valid guid value (!= CLSID_NULL).
		 */
		UtilExport bool IsValid() const;
	};
}

//------------------------------------------------------------------------------
// Hash function for using MaxSDK::MaxGuid in unordered set, maps.
//------------------------------------------------------------------------------
template <>
struct std::hash<MaxSDK::MaxGuid>
{
	UtilExport std::size_t operator()(const MaxSDK::MaxGuid& maxGuid) const;
};
