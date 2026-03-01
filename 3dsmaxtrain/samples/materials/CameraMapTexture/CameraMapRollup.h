//////////////////////////////////////////////////////////////////////////////
//
//  Copyright 2019 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.   
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QtWidgets/QWidget>
#include <Qt/QMaxParamBlockWidget.h>

namespace Ui {
    class CameraMapRollup;
} 

class CameraMapRollup : public MaxSDK::QMaxParamBlockWidget
{
    Q_OBJECT

public:
    CameraMapRollup(QWidget *parent = nullptr);
    ~CameraMapRollup();

    virtual void SetParamBlock(ReferenceMaker* owner, IParamBlock2* const param_block)
    {};
    virtual void UpdateUI(const TimeValue t) {};
    virtual void UpdateParameterUI(const TimeValue t, const ParamID param_id, const int tab_index)
    {};

private:
    Ui::CameraMapRollup *ui;
};
