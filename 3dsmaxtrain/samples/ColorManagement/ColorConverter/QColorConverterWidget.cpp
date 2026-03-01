//**************************************************************************/
// Copyright (c) 1998-2022 Autodesk, Inc.
// All rights reserved.
// 
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#include "QColorConverterWidget.h"

// MaxSDK
#include <Notify.h>
#include <Geom/Color.h>

#include "Utils/ColorManagementAPIs.h"

using namespace ColorManagementAPIs;

QColorConverterWidget::QColorConverterWidget(QWidget* parent)
    : QDialog(parent)
{
    ui.setupUi(this);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // Init
    InitColorSpaces();
    InitInputModes();
    InitRendModes();
    InitOutputModes();
    SetInputColor(Qt::red);
    SyncInputColor();

    // Events
    connect(ui.pushButton_In_Pick, &QColorEyeDropper::colorChanged, this, &QColorConverterWidget::onInputPickColorChanged);
    connect(ui.pushButton_Cmp_Pick, &QColorEyeDropper::colorChanged, this, &QColorConverterWidget::onCmpPickColorChanged);
    connect(ui.pushButton_Out_Clone, &QPushButton::clicked, this, &QColorConverterWidget::onCloneClicked);

    connect(ui.doubleSpinBox_In_R, &QDoubleSpinBox::valueChanged, this, &QColorConverterWidget::onInputColorComponentChanged);
    connect(ui.doubleSpinBox_In_G, &QDoubleSpinBox::valueChanged, this, &QColorConverterWidget::onInputColorComponentChanged);
    connect(ui.doubleSpinBox_In_B, &QDoubleSpinBox::valueChanged, this, &QColorConverterWidget::onInputColorComponentChanged);
    connect(ui.doubleSpinBox_Cmp_R, &QDoubleSpinBox::valueChanged, this, &QColorConverterWidget::onCmpColorComponentChanged);
    connect(ui.doubleSpinBox_Cmp_G, &QDoubleSpinBox::valueChanged, this, &QColorConverterWidget::onCmpColorComponentChanged);
    connect(ui.doubleSpinBox_Cmp_B, &QDoubleSpinBox::valueChanged, this, &QColorConverterWidget::onCmpColorComponentChanged);
    connect(ui.doubleSpinBox_Rend_R, &QDoubleSpinBox::valueChanged, this, &QColorConverterWidget::onRendColorComponentChanged);
    connect(ui.doubleSpinBox_Rend_G, &QDoubleSpinBox::valueChanged, this, &QColorConverterWidget::onRendColorComponentChanged);
    connect(ui.doubleSpinBox_Rend_B, &QDoubleSpinBox::valueChanged, this, &QColorConverterWidget::onRendColorComponentChanged);
    connect(ui.doubleSpinBox_Exposure, &QDoubleSpinBox::valueChanged, this, &QColorConverterWidget::onOutputExposureChanged);
    connect(ui.doubleSpinBox_Gamma, &QDoubleSpinBox::valueChanged, this, &QColorConverterWidget::onOutputGammaChanged);

    connect(ui.comboBox_In_Space, &QComboBox::currentIndexChanged, this, &QColorConverterWidget::onInputColorSpaceChanged);
    connect(ui.comboBox_Rend_Space, &QComboBox::currentIndexChanged, this, &QColorConverterWidget::onRendColorSpaceChanged);
    connect(ui.comboBox_Out_Space, &QComboBox::currentIndexChanged, this, &QColorConverterWidget::onOutputColorSpaceChanged);
    connect(ui.comboBox_Rend_Mode, &QComboBox::currentIndexChanged, this, &QColorConverterWidget::onRenderModeChanged);
    connect(ui.comboBox_Out_Mode, &QComboBox::currentIndexChanged, this, &QColorConverterWidget::onOutputModeChanged);
    connect(ui.comboBox_View_Space, &QComboBox::currentIndexChanged, this, &QColorConverterWidget::onColorSpaceChanged);

    // Hook up Color Pipeline notification
	RegisterNotification(OnColorPipelineSettingsChanged, this, NOTIFY_COLOR_MANAGEMENT_POST_CHANGE);
}

QColorConverterWidget::~QColorConverterWidget()
{
	UnRegisterNotification(OnColorPipelineSettingsChanged, this, NOTIFY_COLOR_MANAGEMENT_POST_CHANGE);
}

void QColorConverterWidget::OnColorPipelineSettingsChanged(void* param, NotifyInfo* info)
{
    auto w = static_cast<QColorConverterWidget*>(param);
	if (w && info->intcode == NOTIFY_COLOR_MANAGEMENT_POST_CHANGE)
    {
        w->UpdateTitle();

        w->InitColorSpaces();
        w->SyncInputColor();
    }
}

void QColorConverterWidget::UpdateTitle()
{
    QString modeName = GetCurrentColorPipelineModeName();
    if (parentWidget())
    {
        parentWidget()->setWindowTitle(QString("%1, %2 %3").arg(tr("Color Converter")).arg(tr("Mode: ")).arg(modeName));
    }
}

void QColorConverterWidget::InitColorSpaces()
{
    UpdateInputColorSpaces();
    UpdateRendColorSpaces();
    UpdateOutputColorSpaces();
    UpdateViewColorSpaces();
}

void QColorConverterWidget::InitInputModes()
{
    QSignalBlocker s1(ui.comboBox_In_Mode);
    ui.comboBox_In_Mode->clear();

    ui.comboBox_In_Mode->addItem(tr("Color Space"));
    ui.comboBox_In_Mode->setCurrentIndex(0);
}

void QColorConverterWidget::InitRendModes()
{
    QSignalBlocker s1(ui.comboBox_Rend_Mode);
    ui.comboBox_Rend_Mode->clear();

    ui.comboBox_Rend_Mode->addItem(tr("Do Rendering Space Conversion"));
    ui.comboBox_Rend_Mode->addItem(tr("Skip Rendering Space Conversion"));
    ui.comboBox_Rend_Mode->setCurrentIndex(0);
}

void QColorConverterWidget::InitOutputModes()
{
    QSignalBlocker s1(ui.comboBox_Out_Mode);
    ui.comboBox_Out_Mode->clear();

    ui.comboBox_Out_Mode->addItem(tr("Color Space"));
    ui.comboBox_Out_Mode->addItem(tr("Display/View Transform"));
    ui.comboBox_Out_Mode->setCurrentIndex(1);
}

void QColorConverterWidget::UpdateInputColorSpaces()
{
    // Input
    QSignalBlocker s1(ui.comboBox_In_Space);
    ui.comboBox_In_Space->clear();

    // Query input color spaces
    size_t nColSpace = GetNumFileIOColorSpaces();
    for (size_t i = 0; i < nColSpace; ++i)
    {
        QString name = GetFileIOColorSpace(i);
        ui.comboBox_In_Space->addItem(name);
    }
}

void QColorConverterWidget::UpdateRendColorSpaces()
{
    // Render
    QSignalBlocker s2(ui.comboBox_Rend_Space);
    ui.comboBox_Rend_Space->clear();

    // Query rendering/working color spaces
    QString curRendSpace = GetCurrentRenderingColorSpace();
    auto nColSpace = GetNumRenderingColorSpaces();
    for (size_t i = 0; i < nColSpace; ++i)
    {
        QString name = GetRenderingColorSpace(i);
        ui.comboBox_Rend_Space->addItem(name);
    }
    ui.comboBox_Rend_Space->setCurrentText(curRendSpace);
}

void QColorConverterWidget::UpdateViewColorSpaces()
{
    // View
    QSignalBlocker s4(ui.comboBox_View_Space);
    ui.comboBox_View_Space->clear();

    // Query View-Transforms based on Rendering/Working color spaces and Displays
    bool foundDefaultView = false;
    QString curRendSpace = !IsDoingRenderingSpaceConversion() ? ui.comboBox_In_Space->currentText() : ui.comboBox_Rend_Space->currentText();
    QString curDisplay = ui.comboBox_Out_Space->currentText();
    QString defaultView = GetDefaultView(curDisplay);
    size_t nColSpace = GetNumViews(curRendSpace, curDisplay);
    for (size_t i = 0; i < nColSpace; ++i)
    {
        QString name = GetView(curRendSpace, curDisplay, i);
        ui.comboBox_View_Space->addItem(name);
        if (name == defaultView)
        {
            foundDefaultView = true;
        }
    }
    if (foundDefaultView)
    {
        ui.comboBox_View_Space->setCurrentText(defaultView);
    }
}

void QColorConverterWidget::UpdateOutputColorSpaces()
{
    if (IsOutputToColorSpace())
    {
        // Color spaces
        QSignalBlocker s1(ui.comboBox_Out_Space);
        ui.comboBox_Out_Space->clear();

        // Query output color spaces
        size_t nColSpace = GetNumFileIOColorSpaces();
        for (size_t i = 0; i < nColSpace; ++i)
        {
            QString name = GetFileIOColorSpace(i);
            ui.comboBox_Out_Space->addItem(name);
        }
    }
    else
    {
        // Display/View-Transform
        QSignalBlocker s3(ui.comboBox_Out_Space);
        ui.comboBox_Out_Space->clear();

        // Query Displays/View-Transforms
        QString defaultDisplay = GetDefaultDisplay();
        auto nColSpace = GetNumDisplays();
        for (size_t i = 0; i < nColSpace; ++i)
        {
            QString name = GetDisplay(i);
            ui.comboBox_Out_Space->addItem(name);
        }
        ui.comboBox_Out_Space->setCurrentText(defaultDisplay);
    }
}

void QColorConverterWidget::onInputPickColorChanged(const QColor& color)
{
    SetInputColor(color);

    SyncInputColor();
}

void QColorConverterWidget::onCmpPickColorChanged(const QColor& color)
{
    SetCmpColor(color);
    ui.widget_Cmp_Rect->UpdateColor(color);
}

void QColorConverterWidget::onCloneClicked()
{
    auto color = GetOutputColor();
    SetCmpColor(color);
    ui.widget_Cmp_Rect->UpdateColor(color);
}

QColor QColorConverterWidget::GetInputColor() const
{
    return QColor::fromRgbF(ui.doubleSpinBox_In_R->value(), ui.doubleSpinBox_In_G->value(), ui.doubleSpinBox_In_B->value());
}

QColor QColorConverterWidget::GetCmpColor() const
{
    return QColor::fromRgbF(ui.doubleSpinBox_Cmp_R->value(), ui.doubleSpinBox_Cmp_G->value(), ui.doubleSpinBox_Cmp_B->value());
}

void QColorConverterWidget::SetInputColor(const QColor& color)
{
    QSignalBlocker r(ui.doubleSpinBox_In_R);
    QSignalBlocker g(ui.doubleSpinBox_In_G);
    QSignalBlocker b(ui.doubleSpinBox_In_B);

    ui.doubleSpinBox_In_R->setValue(color.redF());
    ui.doubleSpinBox_In_G->setValue(color.greenF());
    ui.doubleSpinBox_In_B->setValue(color.blueF());
}

void QColorConverterWidget::SetCmpColor(const QColor& color)
{
    QSignalBlocker r(ui.doubleSpinBox_Cmp_R);
    QSignalBlocker g(ui.doubleSpinBox_Cmp_G);
    QSignalBlocker b(ui.doubleSpinBox_Cmp_B);

    ui.doubleSpinBox_Cmp_R->setValue(color.redF());
    ui.doubleSpinBox_Cmp_G->setValue(color.greenF());
    ui.doubleSpinBox_Cmp_B->setValue(color.blueF());
}

void QColorConverterWidget::SyncInputColor()
{
    QColor color = GetInputColor();
    ui.widget_In_Rect->UpdateColor(color);

    if (!IsDoingRenderingSpaceConversion())
    {
        SyncOutputColor(true, IsOutputToColorSpace(), ui.doubleSpinBox_Exposure->value(), ui.doubleSpinBox_Gamma->value());
    }
    else
    {
        SyncRendColor();
    }
}

void QColorConverterWidget::onInputColorComponentChanged(double /*val*/)
{
    SyncInputColor();
}

void QColorConverterWidget::onCmpColorComponentChanged(double /*val*/)
{
    QColor color = GetCmpColor();
    ui.widget_Cmp_Rect->UpdateColor(color);
}

QColor QColorConverterWidget::GetRendColor() const
{
    return QColor::fromRgbF(ui.doubleSpinBox_Rend_R->value(), ui.doubleSpinBox_Rend_G->value(), ui.doubleSpinBox_Rend_B->value());
}

void QColorConverterWidget::SetRendColor(const QColor& color)
{
    QSignalBlocker r(ui.doubleSpinBox_Rend_R);
    QSignalBlocker g(ui.doubleSpinBox_Rend_G);
    QSignalBlocker b(ui.doubleSpinBox_Rend_B);

    ui.doubleSpinBox_Rend_R->setValue(color.redF());
    ui.doubleSpinBox_Rend_G->setValue(color.greenF());
    ui.doubleSpinBox_Rend_B->setValue(color.blueF());
}

void QColorConverterWidget::SyncRendColor()
{
    // Convert from Input to Render space
    auto fromSpace = ui.comboBox_In_Space->currentText();
    auto toSpace = ui.comboBox_Rend_Space->currentText();
    
    // Converting
    QColor inQColor = GetInputColor();
    Color inColor = Color((float)inQColor.redF(), (float)inQColor.greenF(), (float)inQColor.blueF());
    Color outColor = ConvertColor(inColor, fromSpace, toSpace);
    QColor outQColor = QColor::fromRgbF(outColor.r, outColor.g, outColor.b);

    // Sync to UI
    SetRendColor(outQColor);
    ui.widget_Rend_Rect->UpdateColor(outQColor);
    SyncOutputColor(false, IsOutputToColorSpace(), ui.doubleSpinBox_Exposure->value(), ui.doubleSpinBox_Gamma->value());
}

void QColorConverterWidget::SyncOutputColor(bool skipRend, bool outputToColorSpace, float exposure, float gamma)
{
    QColor outQColor;
    if (outputToColorSpace)
    {
        // Convert from Input/Render space to Output space
        auto fromSpace = skipRend ? ui.comboBox_In_Space->currentText() : ui.comboBox_Rend_Space->currentText();
        auto toSpace = ui.comboBox_Out_Space->currentText();

        // Converting
        QColor inQColor = skipRend ? GetInputColor() : GetRendColor();
        Color inColor = Color((float)inQColor.redF(), (float)inQColor.greenF(), (float)inQColor.blueF());
        Color outColor = ConvertColor(inColor, fromSpace, toSpace);
        outQColor = QColor::fromRgbF(outColor.r, outColor.g, outColor.b);
    }
    else
    {
        // Convert from Input/Render space to Display/View-Transform
        auto fromSpace = skipRend ? ui.comboBox_In_Space->currentText() : ui.comboBox_Rend_Space->currentText();
        auto toSpace = ui.comboBox_Out_Space->currentText();
        auto view = ui.comboBox_View_Space->currentText();

        // Converting
        QColor inQColor = skipRend ? GetInputColor() : GetRendColor();
        Color inColor = Color((float)inQColor.redF(), (float)inQColor.greenF(), (float)inQColor.blueF());
        Color outColor = ConvertColor(inColor, fromSpace, toSpace, view, exposure, gamma);
        outQColor = QColor::fromRgbF(outColor.r, outColor.g, outColor.b);
    }

    // Sync to UI
    SetOutputColor(outQColor);
    ui.widget_Out_Rect->UpdateColor(outQColor);
}

void QColorConverterWidget::onRendColorComponentChanged(double /*val*/)
{
    SyncRendColor();
}

QColor QColorConverterWidget::GetOutputColor() const
{
    return QColor::fromRgbF(ui.doubleSpinBox_Out_R->value(), ui.doubleSpinBox_Out_G->value(), ui.doubleSpinBox_Out_B->value());
}

void QColorConverterWidget::onOutputExposureChanged(double /*val*/)
{
    SyncOutputColor(!IsDoingRenderingSpaceConversion(), IsOutputToColorSpace(), ui.doubleSpinBox_Exposure->value(), ui.doubleSpinBox_Gamma->value());
}

void QColorConverterWidget::onOutputGammaChanged(double /*val*/)
{
    SyncOutputColor(!IsDoingRenderingSpaceConversion(), IsOutputToColorSpace(), ui.doubleSpinBox_Exposure->value(), ui.doubleSpinBox_Gamma->value());
}

void QColorConverterWidget::SetOutputColor(const QColor& color)
{
    QSignalBlocker r(ui.doubleSpinBox_Out_R);
    QSignalBlocker g(ui.doubleSpinBox_Out_G);
    QSignalBlocker b(ui.doubleSpinBox_Out_B);

    ui.doubleSpinBox_Out_R->setValue(color.redF());
    ui.doubleSpinBox_Out_G->setValue(color.greenF());
    ui.doubleSpinBox_Out_B->setValue(color.blueF());
}

void QColorConverterWidget::onInputColorSpaceChanged(int index)
{
    if (!IsDoingRenderingSpaceConversion())
    {
        UpdateViewColorSpaces();
    }

    onColorSpaceChanged(index);
}

void QColorConverterWidget::onRendColorSpaceChanged(int index)
{
    UpdateViewColorSpaces();

    onColorSpaceChanged(index);
}

void QColorConverterWidget::onOutputColorSpaceChanged(int index)
{
    UpdateViewColorSpaces();

    onColorSpaceChanged(index);
}

void QColorConverterWidget::onColorSpaceChanged(int /*index*/)
{
    if (!IsDoingRenderingSpaceConversion())
    {
        SyncOutputColor(true, IsOutputToColorSpace(), ui.doubleSpinBox_Exposure->value(), ui.doubleSpinBox_Gamma->value());
    }
    else
    {
        SyncRendColor();
    }
}

void QColorConverterWidget::onRenderModeChanged(int /*index*/)
{
    bool doRender = IsDoingRenderingSpaceConversion();
    ui.doubleSpinBox_Rend_R->setEnabled(doRender);
    ui.doubleSpinBox_Rend_G->setEnabled(doRender);
    ui.doubleSpinBox_Rend_B->setEnabled(doRender);
    ui.comboBox_Rend_Space->setEnabled(doRender);

    // The view list depends on the input or rendering space
    UpdateViewColorSpaces();

    if (!doRender)
    {
        SetRendColor(Qt::black);
        ui.widget_Rend_Rect->SetDrawColor(false);
    }
    else
    {
        ui.widget_Rend_Rect->SetDrawColor(true);
    }

    SyncInputColor();
}

bool QColorConverterWidget::IsDoingRenderingSpaceConversion() const
{
    return ui.comboBox_Rend_Mode->currentIndex() == 0;
}

void QColorConverterWidget::onOutputModeChanged(int /*index*/)
{
    bool toColorSpace = IsOutputToColorSpace();
    ui.comboBox_View_Space->setEnabled(!toColorSpace);
    ui.doubleSpinBox_Gamma->setEnabled(!toColorSpace);
    ui.doubleSpinBox_Exposure->setEnabled(!toColorSpace);

    UpdateOutputColorSpaces();
    // The view list depends on the input or rendering space
    UpdateViewColorSpaces();

    SyncInputColor();
}

bool QColorConverterWidget::IsOutputToColorSpace() const
{
    return ui.comboBox_Out_Mode->currentIndex() == 0;
}