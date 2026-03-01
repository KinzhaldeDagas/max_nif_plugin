/********************************************************************************
** Form generated from reading UI file 'CameraMap.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CAMERAMAP_H
#define UI_CAMERAMAP_H

#include <Qt/QmaxSpinBox.h>
#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_CameraMapRollup
{
public:
    QGridLayout *gridLayout;
    MaxSDK::QmaxSpinBox *MapChannel;
    QPushButton *zbuffer;
    QLabel *label;
    QPushButton *mask;
    QLabel *label_4;
    QCheckBox *useMask;
    QCheckBox *backFace;
    QCheckBox *useZBuffer;
    MaxSDK::QmaxDoubleSpinBox *angleThreshold;
    MaxSDK::QmaxDoubleSpinBox *ZFudge;
    QPushButton *camera;
    QPushButton *texture;
    QLabel *label_2;
    QCheckBox *maskUsesProjection;
    QLabel *label_5;
    QGroupBox *groupBox;
    QGridLayout *aspectLayout;
    QRadioButton *mode_bitmap;
    MaxSDK::QmaxDoubleSpinBox *vaspect;
    MaxSDK::QmaxDoubleSpinBox *haspect;
    QLabel *label_6;
    QRadioButton *mode_custom;
    QRadioButton *mode_legacy;
    QLabel *label_3;
    QButtonGroup *aspectMode;

    void setupUi(QWidget *CameraMapRollup)
    {
        if (CameraMapRollup->objectName().isEmpty())
            CameraMapRollup->setObjectName("CameraMapRollup");
        CameraMapRollup->resize(313, 427);
        gridLayout = new QGridLayout(CameraMapRollup);
        gridLayout->setObjectName("gridLayout");
        MapChannel = new MaxSDK::QmaxSpinBox(CameraMapRollup);
        MapChannel->setObjectName("MapChannel");

        gridLayout->addWidget(MapChannel, 0, 2, 1, 1);

        zbuffer = new QPushButton(CameraMapRollup);
        zbuffer->setObjectName("zbuffer");

        gridLayout->addWidget(zbuffer, 3, 1, 1, 2);

        label = new QLabel(CameraMapRollup);
        label->setObjectName("label");
        label->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

        gridLayout->addWidget(label, 1, 0, 1, 1, Qt::AlignLeft);

        mask = new QPushButton(CameraMapRollup);
        mask->setObjectName("mask");

        gridLayout->addWidget(mask, 5, 1, 1, 2);

        label_4 = new QLabel(CameraMapRollup);
        label_4->setObjectName("label_4");
        label_4->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

        gridLayout->addWidget(label_4, 0, 1, 1, 1, Qt::AlignRight);

        useMask = new QCheckBox(CameraMapRollup);
        useMask->setObjectName("useMask");

        gridLayout->addWidget(useMask, 5, 0, 1, 1);

        backFace = new QCheckBox(CameraMapRollup);
        backFace->setObjectName("backFace");

        gridLayout->addWidget(backFace, 7, 0, 1, 2);

        useZBuffer = new QCheckBox(CameraMapRollup);
        useZBuffer->setObjectName("useZBuffer");

        gridLayout->addWidget(useZBuffer, 3, 0, 1, 1);

        angleThreshold = new MaxSDK::QmaxDoubleSpinBox(CameraMapRollup);
        angleThreshold->setObjectName("angleThreshold");
        angleThreshold->setSingleStep(0.500000000000000);

        gridLayout->addWidget(angleThreshold, 9, 2, 1, 1);

        ZFudge = new MaxSDK::QmaxDoubleSpinBox(CameraMapRollup);
        ZFudge->setObjectName("ZFudge");

        gridLayout->addWidget(ZFudge, 4, 2, 1, 1);

        camera = new QPushButton(CameraMapRollup);
        camera->setObjectName("camera");

        gridLayout->addWidget(camera, 1, 1, 1, 2);

        texture = new QPushButton(CameraMapRollup);
        texture->setObjectName("texture");

        gridLayout->addWidget(texture, 2, 1, 1, 2);

        label_2 = new QLabel(CameraMapRollup);
        label_2->setObjectName("label_2");
        label_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label_2, 4, 1, 1, 1);

        maskUsesProjection = new QCheckBox(CameraMapRollup);
        maskUsesProjection->setObjectName("maskUsesProjection");

        gridLayout->addWidget(maskUsesProjection, 6, 0, 1, 2);

        label_5 = new QLabel(CameraMapRollup);
        label_5->setObjectName("label_5");
        label_5->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

        gridLayout->addWidget(label_5, 2, 0, 1, 1, Qt::AlignLeft);

        groupBox = new QGroupBox(CameraMapRollup);
        groupBox->setObjectName("groupBox");
        aspectLayout = new QGridLayout(groupBox);
        aspectLayout->setObjectName("aspectLayout");
        mode_bitmap = new QRadioButton(groupBox);
        aspectMode = new QButtonGroup(CameraMapRollup);
        aspectMode->setObjectName("aspectMode");
        aspectMode->addButton(mode_bitmap);
        mode_bitmap->setObjectName("mode_bitmap");

        aspectLayout->addWidget(mode_bitmap, 1, 0, 1, 4);

        vaspect = new MaxSDK::QmaxDoubleSpinBox(groupBox);
        vaspect->setObjectName("vaspect");

        aspectLayout->addWidget(vaspect, 3, 3, 1, 1);

        haspect = new MaxSDK::QmaxDoubleSpinBox(groupBox);
        haspect->setObjectName("haspect");

        aspectLayout->addWidget(haspect, 3, 1, 1, 1);

        label_6 = new QLabel(groupBox);
        label_6->setObjectName("label_6");

        aspectLayout->addWidget(label_6, 3, 2, 1, 1);

        mode_custom = new QRadioButton(groupBox);
        aspectMode->addButton(mode_custom);
        mode_custom->setObjectName("mode_custom");
        mode_custom->setChecked(true);

        aspectLayout->addWidget(mode_custom, 3, 0, 1, 1, Qt::AlignTop);

        mode_legacy = new QRadioButton(groupBox);
        aspectMode->addButton(mode_legacy);
        mode_legacy->setObjectName("mode_legacy");

        aspectLayout->addWidget(mode_legacy, 0, 0, 1, 4);

        aspectLayout->setColumnStretch(0, 2);
        aspectLayout->setColumnStretch(1, 1);
        aspectLayout->setColumnStretch(3, 1);

        gridLayout->addWidget(groupBox, 10, 0, 1, 3);

        label_3 = new QLabel(CameraMapRollup);
        label_3->setObjectName("label_3");
        label_3->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label_3, 9, 0, 1, 2);

        gridLayout->setColumnStretch(1, 2);
        gridLayout->setColumnStretch(2, 1);

        retranslateUi(CameraMapRollup);

        QMetaObject::connectSlotsByName(CameraMapRollup);
    } // setupUi

    void retranslateUi(QWidget *CameraMapRollup)
    {
        zbuffer->setText(QCoreApplication::translate("CameraMapRollup", "ZBuffer", nullptr));
        label->setText(QCoreApplication::translate("CameraMapRollup", "Camera:", nullptr));
        mask->setText(QCoreApplication::translate("CameraMapRollup", "Alpha", nullptr));
        label_4->setText(QCoreApplication::translate("CameraMapRollup", "Map Channel:", nullptr));
        useMask->setText(QCoreApplication::translate("CameraMapRollup", "Mask:", nullptr));
        backFace->setText(QCoreApplication::translate("CameraMapRollup", "Remove Back Face Pixels", nullptr));
        useZBuffer->setText(QCoreApplication::translate("CameraMapRollup", "ZBuffer Mask:", nullptr));
        angleThreshold->setSuffix(QCoreApplication::translate("CameraMapRollup", "\342\200\212\302\260", nullptr));
        camera->setText(QCoreApplication::translate("CameraMapRollup", "Pick Camera", nullptr));
        texture->setText(QCoreApplication::translate("CameraMapRollup", "Texture", nullptr));
        label_2->setText(QCoreApplication::translate("CameraMapRollup", "ZFudge:", nullptr));
        maskUsesProjection->setText(QCoreApplication::translate("CameraMapRollup", "Mask Uses the Camera Projection", nullptr));
        label_5->setText(QCoreApplication::translate("CameraMapRollup", "Texture:", nullptr));
        groupBox->setTitle(QCoreApplication::translate("CameraMapRollup", "Aspect Ratio", nullptr));
#if QT_CONFIG(tooltip)
        mode_bitmap->setToolTip(QCoreApplication::translate("CameraMapRollup", "<b>Attached Bitmap:</b><br/>\n"
"In this mode, the aspect ratio of any attached <b>Bitmap</b> is automatically used. However, if any other type of texture is used, from which the inherent aspect ratio can not be deduced, the <b>Custom</b> aspect ratio is used as a fallback.", nullptr));
#endif // QT_CONFIG(tooltip)
        mode_bitmap->setText(QCoreApplication::translate("CameraMapRollup", "Attached Bitmap (when available)", nullptr));
#if QT_CONFIG(tooltip)
        vaspect->setToolTip(QCoreApplication::translate("CameraMapRollup", "<html><head/><body><p>Vertical Aspect Ratio</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        haspect->setToolTip(QCoreApplication::translate("CameraMapRollup", "<html><head/><body><p>Horizontal Aspect Ratio</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        label_6->setText(QCoreApplication::translate("CameraMapRollup", "/", nullptr));
#if QT_CONFIG(tooltip)
        mode_custom->setToolTip(QCoreApplication::translate("CameraMapRollup", "<b>Custom:</b><br/>\n"
"Set any custom width-to-height ratio for use as the projections aspect ratio. This ratio is also used if the <b>Attached Bitmap</b> mode is selected, but the attached texture <i>isn't</i> a pure <b>Bitmap</b>.", nullptr));
#endif // QT_CONFIG(tooltip)
        mode_custom->setText(QCoreApplication::translate("CameraMapRollup", "Custom:", nullptr));
#if QT_CONFIG(tooltip)
        mode_legacy->setToolTip(QCoreApplication::translate("CameraMapRollup", "<b>Rendered Image:</b><br/>\n"
"Matches the aspect ratio of the image currently being rendered, which is the legacy behavior. In this mode, there is a risk of the projection changing when output resolution is changed.", nullptr));
#endif // QT_CONFIG(tooltip)
        mode_legacy->setText(QCoreApplication::translate("CameraMapRollup", "Rendered Image (Legacy Mode)", nullptr));
        label_3->setText(QCoreApplication::translate("CameraMapRollup", "Angle Threshold:", nullptr));
        (void)CameraMapRollup;
    } // retranslateUi

};

namespace Ui {
    class CameraMapRollup: public Ui_CameraMapRollup {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CAMERAMAP_H
