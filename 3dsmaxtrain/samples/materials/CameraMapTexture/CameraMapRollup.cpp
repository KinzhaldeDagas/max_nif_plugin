//////////////////////////////////////////////////////////////////////////////
//
//  Copyright 2019 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.   
//
//////////////////////////////////////////////////////////////////////////////

#include "CameraMapTexture.h"
#include "CameraMapRollup.h"
#include "ui_CameraMap.h"
#include "winutil.h"

CameraMapRollup::CameraMapRollup(QWidget *parent)
    : QMaxParamBlockWidget()
{
    ui = new Ui::CameraMapRollup();
    ui->setupUi(this);
    ui->aspectMode->setId(ui->mode_bitmap, 0);
    ui->aspectMode->setId(ui->mode_custom, 1);
    ui->aspectMode->setId(ui->mode_legacy, 2);

    auto margins = ui->aspectLayout->contentsMargins(); 
    margins.setTop(margins.top() * 4);
    ui->aspectLayout->setContentsMargins(margins);

    auto height = ui->camera->minimumSizeHint().height();
    ui->backFace->setMinimumHeight(height);
}

CameraMapRollup::~CameraMapRollup()
{
    delete ui;
}
