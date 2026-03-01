//////////////////////////////////////////////////////////////////////////////
//
//  Copyright 2015 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.   
//
//////////////////////////////////////////////////////////////////////////////
#pragma once

#include "../strclass.h"
#include <Geom/acolor.h>
#include <Geom/point2.h>
#include <Geom/matrix3.h>

#pragma warning(push)
// 4127 conditional expression is constant
// 4251 class 'type1' needs to have dll-interface to be used by clients of class 'type2'
// 4275 non - DLL-interface class 'class_1' used as base for DLL-interface class 'class_2'
#pragma warning(disable: 4127 4251 4275)
#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <QtWidgets/QtWidgets>
#pragma warning(pop)

#ifdef BLD_MAXQTBRIDGE
#define MaxQTExport __declspec( dllexport )
#else
#define MaxQTExport __declspec( dllimport )
#endif

/*! \file
 * \warning This API is unsupported and provided for experiment only.  It should not be used for production.
 * This API will change or disappear in a future release.
 */
namespace MaxSDK
{;
// Retrieve localized text from its resource ID.
QString QStringFromID(int resourceID);

} // namespace

// Enable 3ds Max data types to be used in a QVariant
// General note: Q_DECLARE_METATYPE() macro has to be outside namespaces
// For types in namespaces declare like this: Q_DECLARE_METATYPE(MyNamespace::MyStruct)
Q_DECLARE_METATYPE(AColor);
Q_DECLARE_METATYPE(Point2);
Q_DECLARE_METATYPE(Point3);
Q_DECLARE_METATYPE(Point4);
Q_DECLARE_METATYPE(Matrix3);


#include "QtMax.inline.h"
