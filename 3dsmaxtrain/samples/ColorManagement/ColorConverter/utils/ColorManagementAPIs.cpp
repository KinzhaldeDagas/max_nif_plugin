//**************************************************************************/
// Copyright (c) 1998-2022 Autodesk, Inc.
// All rights reserved.
// 
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#include "ColorManagementAPIs.h"

// MaxSDK
#include <ColorManagement/IColorPipelineMgr.h>

using namespace MaxSDK::ColorManagement;

namespace ColorManagementAPIs
{
    size_t GetNumFileIOColorSpaces()
    {
        auto cpmSettings = IColorPipelineMgr::GetInstance()->Settings();
        return cpmSettings->GetNumFileIOColorSpaces();
    }

    MSTR GetFileIOColorSpace(size_t index)
    {
        auto cpmSettings = IColorPipelineMgr::GetInstance()->Settings();
        return cpmSettings->GetFileIOColorSpace(index);
    }

    size_t GetNumRenderingColorSpaces()
    {
        auto cpmSettings = IColorPipelineMgr::GetInstance()->Settings();
        return cpmSettings->GetNumRenderingColorSpaces();
    }

    MSTR GetRenderingColorSpace(size_t index)
    {
        auto cpmSettings = IColorPipelineMgr::GetInstance()->Settings();
        return cpmSettings->GetRenderingColorSpace(index);
    }

    MSTR GetCurrentRenderingColorSpace()
    {
        auto cpmSettings = IColorPipelineMgr::GetInstance()->Settings();
        return cpmSettings->GetCurrentRenderingColorSpace();
    }

    size_t GetNumViews(const MSTR& rendSpace, const MSTR& display)
    {
        auto cpmSettings = IColorPipelineMgr::GetInstance()->Settings();
        return cpmSettings->GetNumViews(rendSpace, display);
    }

    MSTR GetView(const MSTR& rendSpace, const MSTR& display, size_t index)
    {
        auto cpmSettings = IColorPipelineMgr::GetInstance()->Settings();
        return cpmSettings->GetView(rendSpace, display, index);
    }

    MSTR GetDefaultView(const MSTR& display)
    {
        auto cpmSettings = IColorPipelineMgr::GetInstance()->Settings();
        return cpmSettings->GetDefaultView(display);
    }

    size_t GetNumDisplays()
    {
        auto cpmSettings = IColorPipelineMgr::GetInstance()->Settings();
        return cpmSettings->GetNumDisplays();
    }

    MSTR GetDisplay(size_t index)
    {
        auto cpmSettings = IColorPipelineMgr::GetInstance()->Settings();
        return cpmSettings->GetDisplay(index);
    }

    MSTR GetDefaultDisplay()
    {
        auto cpmSettings = IColorPipelineMgr::GetInstance()->Settings();
        return cpmSettings->GetDefaultDisplay();
    }

    // Utility to convert color from one space to another space with 3dsmax color management APIs
    Color ConvertColor(const Color& inColor, const MSTR& fromSpace, const MSTR& toSpace)
    {
        // Get the color conversion pipeline
        IColorPipelineMgr* pColorPipelineMgr = IColorPipelineMgr::GetInstance();
        std::shared_ptr<IColorPipeline> colorPipeline = pColorPipelineMgr->GetPipeline(fromSpace, toSpace);
        DbgAssertMsg(colorPipeline, pColorPipelineMgr->GetLastErrorText().data());
        if (!colorPipeline)
        {
            return inColor;
        }

        // Get the color converter
        std::shared_ptr<ColorConverter<Color, Color>> colorConverter = colorPipeline->GetColorConverter<Color, Color>();
        DbgAssertMsg(colorConverter, pColorPipelineMgr->GetLastErrorText().data());
        if (!colorConverter)
        {
            return inColor;
        }

        // Converting
        return colorConverter->ConvertColor(inColor);
    }

    Color ConvertColor(const Color& inColor, const MSTR& fromSpace, const MSTR& toSpace, const MSTR& view, float exposure, float gamma)
    {
        IColorPipelineMgr* pColorPipelineMgr = IColorPipelineMgr::GetInstance();
        std::shared_ptr<IColorPipeline> colorPipeline = pColorPipelineMgr->GetAdvancedViewingPipeline(fromSpace, toSpace, view);
        DbgAssertMsg(colorPipeline, pColorPipelineMgr->GetLastErrorText().data());
        if (!colorPipeline)
        {
            return inColor;
        }

        // Sete Exposure and Gamma to pipeline
        if (colorPipeline)
        {
            auto paramExp = colorPipeline->GetParameter(_T("exposure"));
            if (paramExp)
            {
                paramExp->SetValue(exposure);
            }

            auto paramGam = colorPipeline->GetParameter(_T("gamma"));
            if (paramGam)
            {
                paramGam->SetValue(gamma == 0.0f ? 1.0f : 1.0f / gamma);
            }
        }

        // Get the color converter
        std::shared_ptr<ColorConverter<Color, Color>> colorConverter = colorPipeline->GetColorConverter<Color, Color>();
        DbgAssertMsg(colorConverter, pColorPipelineMgr->GetLastErrorText().data());
        if (!colorConverter)
        {
            return inColor;
        }

        // Converting
        return colorConverter->ConvertColor(inColor);
    }

    MSTR GetCurrentColorPipelineModeName()
    {
        IColorPipelineMgr* pColorPipelineMgr = IColorPipelineMgr::GetInstance();
        MSTR modeName = _T("Unknown");
        switch (pColorPipelineMgr->GetColorPipelineMode())
        {
        case MaxSDK::ColorManagement::ColorPipelineMode::kUNMANAGED:
            modeName = _T("Unmanaged");
            break;
        case MaxSDK::ColorManagement::ColorPipelineMode::kGAMMA:
            modeName = _T("Gamma Workflow");
            break;
        case MaxSDK::ColorManagement::ColorPipelineMode::kOCIO_DEFAULT:
            modeName = _T("OCIO Default");
            break;
        case MaxSDK::ColorManagement::ColorPipelineMode::kOCIO_CUSTOM:
            modeName = _T("OCIO Custom");
            break;
        default:
            break;
        }
        return modeName;
    }
}

