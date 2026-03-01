//**************************************************************************/
// Copyright (c) 1998-2022 Autodesk, Inc.
// All rights reserved.
// 
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#pragma once

#include <QDialog>
#include "ui_QColorConverterWidget.h"

// MaxSDK
#include <Qt/QMaxDockWidget.h>

struct NotifyInfo;

class QColorConverterWidget :
    public QDialog
{
    Q_OBJECT

public:
    QColorConverterWidget(QWidget* parent = nullptr);
    ~QColorConverterWidget();

    void UpdateTitle();

public Q_SLOTS:
    void onInputPickColorChanged(const QColor&);
    void onCmpPickColorChanged(const QColor&);
    void onCloneClicked();
    void onInputColorComponentChanged(double);
    void onCmpColorComponentChanged(double);
    void onRendColorComponentChanged(double val);
    void onColorSpaceChanged(int index);
    void onInputColorSpaceChanged(int index);
    void onRendColorSpaceChanged(int index);
    void onOutputColorSpaceChanged(int index);
    void onOutputExposureChanged(double val);
    void onOutputGammaChanged(double val);
    void onRenderModeChanged(int index);
    void onOutputModeChanged(int index);

private:
    static void OnColorPipelineSettingsChanged(void* param, NotifyInfo* info);

    void InitColorSpaces();
    void InitInputModes();
    void InitRendModes();
    void InitOutputModes();
    void UpdateInputColorSpaces();
    void UpdateViewColorSpaces();
    void UpdateOutputColorSpaces();
    void UpdateRendColorSpaces();

    bool IsDoingRenderingSpaceConversion() const;
    bool IsOutputToColorSpace() const;

    QColor GetInputColor() const;
    void SetInputColor(const QColor& color);

    QColor GetCmpColor() const;
    void SetCmpColor(const QColor& color);

    QColor GetRendColor() const;
    void SetRendColor(const QColor& color);

    QColor GetOutputColor() const;
    void SetOutputColor(const QColor& color);

    void SyncInputColor();
    void SyncRendColor();
    void SyncOutputColor(bool skipRend, bool skipView, float exposure, float gamma);

    Ui::QColorConverterWidget ui;
};

static QPointer<MaxSDK::QmaxDockWidget> s_theConverterDockWidget = nullptr;

