//**************************************************************************/
// Copyright (c) 1998-2022 Autodesk, Inc.
// All rights reserved.
// 
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#pragma once

// MaxSDK
#include <strclass.h>
#include <Geom/Color.h>


// Utilities to wrap 3dsMax ColorManagement APIs
namespace ColorManagementAPIs
{
    // File IO
    size_t GetNumFileIOColorSpaces();    
    MSTR GetFileIOColorSpace(size_t index);

    // Rendering
    size_t GetNumRenderingColorSpaces();
    MSTR GetRenderingColorSpace(size_t index);
    MSTR GetCurrentRenderingColorSpace();

    // Display
    size_t GetNumDisplays();
    MSTR GetDisplay(size_t index);
    MSTR GetDefaultDisplay();

    // View 
    size_t GetNumViews(const MSTR& rendSpace, const MSTR& display);
    MSTR GetView(const MSTR& rendSpace, const MSTR& display, size_t index);
    MSTR GetDefaultView(const MSTR& display);

    // ColorPipeline Mode
    MSTR GetCurrentColorPipelineModeName();

    // Convert color from one space to another space
    Color ConvertColor(const Color& inColor, const MSTR& fromSpace, const MSTR& toSpace);
    // Convert color from one space to another space with view transform, exposure and gamma
    Color ConvertColor(const Color& inColor, const MSTR& fromSpace, const MSTR& toSpace, const MSTR& view, float exposure, float gamma);
};
