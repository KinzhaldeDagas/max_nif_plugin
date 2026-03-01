//**************************************************************************/
// Copyright (c) 2013 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
//Author : David Lanier

#pragma once

class ViewExp;
class ViewParams;
class INode;

namespace Max
{
namespace InteractiveRendering
{

//Misc. utilities functions
namespace Utils
{
    INode*  GetViewNode                 (ViewExp &viewportExp);
    bool    IsActiveShadeViewLocked     (void);
    void    GetViewParams               (ViewParams& outViewParams, ViewExp &viewportExp);
};


}}; //end of namespace 