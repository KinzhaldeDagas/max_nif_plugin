/********************************************************************************
** Form generated from reading UI file 'Slice_Modifier.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SLICE_MODIFIER_H
#define UI_SLICE_MODIFIER_H

#include <Qt/QmaxSpinBox.h>
#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SliceRollup
{
public:
    QGridLayout *gridLayout_3;
    QPushButton *ReferenceObject;
    QGroupBox *Slice_TypeGroup;
    QGridLayout *gridLayout;
    QRadioButton *removeBottom_radio;
    QCheckBox *Cap;
    QRadioButton *splitMesh_radio;
    QRadioButton *removeTop_radio;
    QRadioButton *refineMesh_radio;
    QRadioButton *radial_removeBottom_radio;
    QHBoxLayout *horizontalLayout;
    QLabel *MaterialIDL;
    MaxSDK::QmaxSpinBox *MaterialID;
    QRadioButton *radial_removeTop_radio;
    QCheckBox *SetMaterial;
    QComboBox *SliceFormat;
    QPushButton *AlignToFace;
    QGroupBox *PlanarAxis_groupBox;
    QGridLayout *gridLayout_6;
    QPushButton *PlanarZ;
    QPushButton *PlanarX;
    QPushButton *PlanarY;
    QCheckBox *PlanarFlipY;
    QCheckBox *PlanarFlipZ;
    QCheckBox *PlanarFlipX;
    QPushButton *ResetTransform;
    QGroupBox *RadialAxis_groupBox;
    QGridLayout *gridLayout_2;
    QHBoxLayout *horizontalLayout_6;
    QPushButton *RadialX;
    QPushButton *RadialY;
    QPushButton *RadialZ;
    QGroupBox *RadialSlice_groupBox;
    QGridLayout *gridLayout_5;
    QLabel *MinL;
    MaxSDK::QmaxDoubleSpinBox *Angle1;
    QLabel *MaxL;
    MaxSDK::QmaxDoubleSpinBox *Angle2;
    QLabel *OperateL;
    QComboBox *Faces___Polygons_Toggle;
    QButtonGroup *Slice_Type;
    QButtonGroup *RadialAxis;
    QButtonGroup *Radial_Type;

    void setupUi(QWidget *SliceRollup)
    {
        if (SliceRollup->objectName().isEmpty())
            SliceRollup->setObjectName("SliceRollup");
        SliceRollup->resize(154, 669);
        gridLayout_3 = new QGridLayout(SliceRollup);
        gridLayout_3->setObjectName("gridLayout_3");
        ReferenceObject = new QPushButton(SliceRollup);
        ReferenceObject->setObjectName("ReferenceObject");

        gridLayout_3->addWidget(ReferenceObject, 5, 0, 1, 3);

        Slice_TypeGroup = new QGroupBox(SliceRollup);
        Slice_TypeGroup->setObjectName("Slice_TypeGroup");
        Slice_TypeGroup->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        Slice_TypeGroup->setFlat(false);
        gridLayout = new QGridLayout(Slice_TypeGroup);
        gridLayout->setObjectName("gridLayout");
        removeBottom_radio = new QRadioButton(Slice_TypeGroup);
        Slice_Type = new QButtonGroup(SliceRollup);
        Slice_Type->setObjectName("Slice_Type");
        Slice_Type->addButton(removeBottom_radio);
        removeBottom_radio->setObjectName("removeBottom_radio");

        gridLayout->addWidget(removeBottom_radio, 3, 0, 1, 1);

        Cap = new QCheckBox(Slice_TypeGroup);
        Cap->setObjectName("Cap");

        gridLayout->addWidget(Cap, 6, 0, 1, 1);

        splitMesh_radio = new QRadioButton(Slice_TypeGroup);
        Slice_Type->addButton(splitMesh_radio);
        splitMesh_radio->setObjectName("splitMesh_radio");

        gridLayout->addWidget(splitMesh_radio, 1, 0, 1, 1);

        removeTop_radio = new QRadioButton(Slice_TypeGroup);
        Slice_Type->addButton(removeTop_radio);
        removeTop_radio->setObjectName("removeTop_radio");

        gridLayout->addWidget(removeTop_radio, 2, 0, 1, 1);

        refineMesh_radio = new QRadioButton(Slice_TypeGroup);
        Slice_Type->addButton(refineMesh_radio);
        refineMesh_radio->setObjectName("refineMesh_radio");

        gridLayout->addWidget(refineMesh_radio, 0, 0, 1, 1);

        radial_removeBottom_radio = new QRadioButton(Slice_TypeGroup);
        Radial_Type = new QButtonGroup(SliceRollup);
        Radial_Type->setObjectName("Radial_Type");
        Radial_Type->addButton(radial_removeBottom_radio);
        radial_removeBottom_radio->setObjectName("radial_removeBottom_radio");

        gridLayout->addWidget(radial_removeBottom_radio, 5, 0, 1, 1);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        MaterialIDL = new QLabel(Slice_TypeGroup);
        MaterialIDL->setObjectName("MaterialIDL");

        horizontalLayout->addWidget(MaterialIDL);

        MaterialID = new MaxSDK::QmaxSpinBox(Slice_TypeGroup);
        MaterialID->setObjectName("MaterialID");
        MaterialID->setValue(1);

        horizontalLayout->addWidget(MaterialID);


        gridLayout->addLayout(horizontalLayout, 8, 0, 1, 1);

        radial_removeTop_radio = new QRadioButton(Slice_TypeGroup);
        Radial_Type->addButton(radial_removeTop_radio);
        radial_removeTop_radio->setObjectName("radial_removeTop_radio");

        gridLayout->addWidget(radial_removeTop_radio, 4, 0, 1, 1);

        SetMaterial = new QCheckBox(Slice_TypeGroup);
        SetMaterial->setObjectName("SetMaterial");

        gridLayout->addWidget(SetMaterial, 7, 0, 1, 1);


        gridLayout_3->addWidget(Slice_TypeGroup, 6, 0, 1, 3);

        SliceFormat = new QComboBox(SliceRollup);
        SliceFormat->addItem(QString());
        SliceFormat->addItem(QString());
        SliceFormat->setObjectName("SliceFormat");

        gridLayout_3->addWidget(SliceFormat, 0, 0, 1, 3);

        AlignToFace = new QPushButton(SliceRollup);
        AlignToFace->setObjectName("AlignToFace");
        AlignToFace->setCheckable(true);

        gridLayout_3->addWidget(AlignToFace, 4, 0, 1, 2);

        PlanarAxis_groupBox = new QGroupBox(SliceRollup);
        PlanarAxis_groupBox->setObjectName("PlanarAxis_groupBox");
        gridLayout_6 = new QGridLayout(PlanarAxis_groupBox);
        gridLayout_6->setObjectName("gridLayout_6");
        PlanarZ = new QPushButton(PlanarAxis_groupBox);
        PlanarZ->setObjectName("PlanarZ");
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(PlanarZ->sizePolicy().hasHeightForWidth());
        PlanarZ->setSizePolicy(sizePolicy);
        QIcon icon;
        icon.addFile(QString::fromUtf8("../ICONS CATALOGUE/Dark/EditUVW/AlignToZ_48.png"), QSize(), QIcon::Normal, QIcon::Off);
        PlanarZ->setIcon(icon);
        PlanarZ->setCheckable(true);
        PlanarZ->setChecked(true);

        gridLayout_6->addWidget(PlanarZ, 2, 0, 1, 1);

        PlanarX = new QPushButton(PlanarAxis_groupBox);
        PlanarX->setObjectName("PlanarX");
        sizePolicy.setHeightForWidth(PlanarX->sizePolicy().hasHeightForWidth());
        PlanarX->setSizePolicy(sizePolicy);
        QIcon icon1;
        icon1.addFile(QString::fromUtf8("../ICONS CATALOGUE/Dark/EditUVW/AlignToX_48.png"), QSize(), QIcon::Normal, QIcon::Off);
        PlanarX->setIcon(icon1);
        PlanarX->setCheckable(true);

        gridLayout_6->addWidget(PlanarX, 0, 0, 1, 1);

        PlanarY = new QPushButton(PlanarAxis_groupBox);
        PlanarY->setObjectName("PlanarY");
        sizePolicy.setHeightForWidth(PlanarY->sizePolicy().hasHeightForWidth());
        PlanarY->setSizePolicy(sizePolicy);
        QIcon icon2;
        icon2.addFile(QString::fromUtf8("../ICONS CATALOGUE/Dark/EditUVW/AlignToY_48.png"), QSize(), QIcon::Normal, QIcon::Off);
        PlanarY->setIcon(icon2);
        PlanarY->setCheckable(true);

        gridLayout_6->addWidget(PlanarY, 1, 0, 1, 1);

        PlanarFlipY = new QCheckBox(PlanarAxis_groupBox);
        PlanarFlipY->setObjectName("PlanarFlipY");

        gridLayout_6->addWidget(PlanarFlipY, 1, 1, 1, 1);

        PlanarFlipZ = new QCheckBox(PlanarAxis_groupBox);
        PlanarFlipZ->setObjectName("PlanarFlipZ");

        gridLayout_6->addWidget(PlanarFlipZ, 2, 1, 1, 1);

        PlanarFlipX = new QCheckBox(PlanarAxis_groupBox);
        PlanarFlipX->setObjectName("PlanarFlipX");

        gridLayout_6->addWidget(PlanarFlipX, 0, 1, 1, 1);


        gridLayout_3->addWidget(PlanarAxis_groupBox, 2, 0, 1, 3);

        ResetTransform = new QPushButton(SliceRollup);
        ResetTransform->setObjectName("ResetTransform");
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(ResetTransform->sizePolicy().hasHeightForWidth());
        ResetTransform->setSizePolicy(sizePolicy1);
        ResetTransform->setMinimumSize(QSize(23, 0));
        ResetTransform->setMaximumSize(QSize(23, 16777215));
        QFont font;
        font.setFamilies({QString::fromUtf8("MS Shell Dlg 2")});
        ResetTransform->setFont(font);
        QIcon icon3;
        icon3.addFile(QString::fromUtf8("../../../ICONS CATALOGUE/Dark/MaxScript/ParamWiring/Refresh_32.png"), QSize(), QIcon::Normal, QIcon::Off);
        ResetTransform->setIcon(icon3);

        gridLayout_3->addWidget(ResetTransform, 4, 2, 1, 1);

        RadialAxis_groupBox = new QGroupBox(SliceRollup);
        RadialAxis_groupBox->setObjectName("RadialAxis_groupBox");
        gridLayout_2 = new QGridLayout(RadialAxis_groupBox);
        gridLayout_2->setObjectName("gridLayout_2");
        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setSpacing(0);
        horizontalLayout_6->setObjectName("horizontalLayout_6");
        RadialX = new QPushButton(RadialAxis_groupBox);
        RadialAxis = new QButtonGroup(SliceRollup);
        RadialAxis->setObjectName("RadialAxis");
        RadialAxis->addButton(RadialX);
        RadialX->setObjectName("RadialX");
        sizePolicy.setHeightForWidth(RadialX->sizePolicy().hasHeightForWidth());
        RadialX->setSizePolicy(sizePolicy);
        RadialX->setIcon(icon1);
        RadialX->setCheckable(true);

        horizontalLayout_6->addWidget(RadialX);

        RadialY = new QPushButton(RadialAxis_groupBox);
        RadialAxis->addButton(RadialY);
        RadialY->setObjectName("RadialY");
        sizePolicy.setHeightForWidth(RadialY->sizePolicy().hasHeightForWidth());
        RadialY->setSizePolicy(sizePolicy);
        RadialY->setIcon(icon2);
        RadialY->setCheckable(true);

        horizontalLayout_6->addWidget(RadialY);

        RadialZ = new QPushButton(RadialAxis_groupBox);
        RadialAxis->addButton(RadialZ);
        RadialZ->setObjectName("RadialZ");
        sizePolicy.setHeightForWidth(RadialZ->sizePolicy().hasHeightForWidth());
        RadialZ->setSizePolicy(sizePolicy);
        RadialZ->setIcon(icon);
        RadialZ->setCheckable(true);
        RadialZ->setChecked(true);

        horizontalLayout_6->addWidget(RadialZ);


        gridLayout_2->addLayout(horizontalLayout_6, 0, 0, 1, 2);


        gridLayout_3->addWidget(RadialAxis_groupBox, 1, 0, 1, 3);

        RadialSlice_groupBox = new QGroupBox(SliceRollup);
        RadialSlice_groupBox->setObjectName("RadialSlice_groupBox");
        gridLayout_5 = new QGridLayout(RadialSlice_groupBox);
        gridLayout_5->setObjectName("gridLayout_5");
        MinL = new QLabel(RadialSlice_groupBox);
        MinL->setObjectName("MinL");
        MinL->setLayoutDirection(Qt::LeftToRight);
        MinL->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_5->addWidget(MinL, 0, 0, 1, 1);

        Angle1 = new MaxSDK::QmaxDoubleSpinBox(RadialSlice_groupBox);
        Angle1->setObjectName("Angle1");
        Angle1->setMaximum(180.000000000000000);

        gridLayout_5->addWidget(Angle1, 0, 1, 1, 1);

        MaxL = new QLabel(RadialSlice_groupBox);
        MaxL->setObjectName("MaxL");
        MaxL->setLayoutDirection(Qt::LeftToRight);
        MaxL->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_5->addWidget(MaxL, 1, 0, 1, 1);

        Angle2 = new MaxSDK::QmaxDoubleSpinBox(RadialSlice_groupBox);
        Angle2->setObjectName("Angle2");
        Angle2->setMaximum(180.000000000000000);
        Angle2->setValue(180.000000000000000);

        gridLayout_5->addWidget(Angle2, 1, 1, 1, 1);


        gridLayout_3->addWidget(RadialSlice_groupBox, 3, 0, 1, 3);

        OperateL = new QLabel(SliceRollup);
        OperateL->setObjectName("OperateL");
        OperateL->setLayoutDirection(Qt::LeftToRight);
        OperateL->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

        gridLayout_3->addWidget(OperateL, 7, 0, 1, 1);

        Faces___Polygons_Toggle = new QComboBox(SliceRollup);
        Faces___Polygons_Toggle->setObjectName("Faces___Polygons_Toggle");

        gridLayout_3->addWidget(Faces___Polygons_Toggle, 7, 1, 1, 2);

        QWidget::setTabOrder(SliceFormat, RadialX);
        QWidget::setTabOrder(RadialX, RadialY);
        QWidget::setTabOrder(RadialY, RadialZ);
        QWidget::setTabOrder(RadialZ, PlanarX);
        QWidget::setTabOrder(PlanarX, PlanarFlipX);
        QWidget::setTabOrder(PlanarFlipX, PlanarY);
        QWidget::setTabOrder(PlanarY, PlanarFlipY);
        QWidget::setTabOrder(PlanarFlipY, PlanarZ);
        QWidget::setTabOrder(PlanarZ, PlanarFlipZ);
        QWidget::setTabOrder(PlanarFlipZ, Angle1);
        QWidget::setTabOrder(Angle1, Angle2);
        QWidget::setTabOrder(Angle2, AlignToFace);
        QWidget::setTabOrder(AlignToFace, ResetTransform);
        QWidget::setTabOrder(ResetTransform, ReferenceObject);
        QWidget::setTabOrder(ReferenceObject, refineMesh_radio);
        QWidget::setTabOrder(refineMesh_radio, splitMesh_radio);
        QWidget::setTabOrder(splitMesh_radio, removeTop_radio);
        QWidget::setTabOrder(removeTop_radio, removeBottom_radio);
        QWidget::setTabOrder(removeBottom_radio, radial_removeTop_radio);
        QWidget::setTabOrder(radial_removeTop_radio, radial_removeBottom_radio);
        QWidget::setTabOrder(radial_removeBottom_radio, Cap);
        QWidget::setTabOrder(Cap, SetMaterial);
        QWidget::setTabOrder(SetMaterial, MaterialID);

        retranslateUi(SliceRollup);

        SliceFormat->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(SliceRollup);
    } // setupUi

    void retranslateUi(QWidget *SliceRollup)
    {
        SliceRollup->setWindowTitle(QCoreApplication::translate("SliceRollup", "Form", nullptr));
#if QT_CONFIG(tooltip)
        ReferenceObject->setToolTip(QCoreApplication::translate("SliceRollup", "Select to pick a reference object from your scene", nullptr));
#endif // QT_CONFIG(tooltip)
        ReferenceObject->setText(QCoreApplication::translate("SliceRollup", "Pick Object", nullptr));
#if QT_CONFIG(tooltip)
        Slice_TypeGroup->setToolTip(QString());
#endif // QT_CONFIG(tooltip)
        Slice_TypeGroup->setTitle(QCoreApplication::translate("SliceRollup", "Slice Type", nullptr));
#if QT_CONFIG(tooltip)
        removeBottom_radio->setToolTip(QCoreApplication::translate("SliceRollup", "Remove the negative portion of the mesh", nullptr));
#endif // QT_CONFIG(tooltip)
        removeBottom_radio->setText(QCoreApplication::translate("SliceRollup", "Remove Negative", nullptr));
        Cap->setText(QCoreApplication::translate("SliceRollup", "Cap", nullptr));
#if QT_CONFIG(tooltip)
        splitMesh_radio->setToolTip(QCoreApplication::translate("SliceRollup", "Split the the Mesh along the slicing plane, maintaining both sections", nullptr));
#endif // QT_CONFIG(tooltip)
        splitMesh_radio->setText(QCoreApplication::translate("SliceRollup", "Split Mesh", nullptr));
#if QT_CONFIG(tooltip)
        removeTop_radio->setToolTip(QCoreApplication::translate("SliceRollup", "Remove the positive portion of the mesh", nullptr));
#endif // QT_CONFIG(tooltip)
        removeTop_radio->setText(QCoreApplication::translate("SliceRollup", "Remove Positive", nullptr));
#if QT_CONFIG(tooltip)
        refineMesh_radio->setToolTip(QCoreApplication::translate("SliceRollup", "Refine the mesh to include new vertices created by the slice", nullptr));
#endif // QT_CONFIG(tooltip)
        refineMesh_radio->setText(QCoreApplication::translate("SliceRollup", "Refine Mesh", nullptr));
#if QT_CONFIG(tooltip)
        radial_removeBottom_radio->setToolTip(QCoreApplication::translate("SliceRollup", "Remove the negative portion of the mesh", nullptr));
#endif // QT_CONFIG(tooltip)
        radial_removeBottom_radio->setText(QCoreApplication::translate("SliceRollup", "Remove Negative", nullptr));
        MaterialIDL->setText(QCoreApplication::translate("SliceRollup", "Material  ID:", nullptr));
#if QT_CONFIG(tooltip)
        radial_removeTop_radio->setToolTip(QCoreApplication::translate("SliceRollup", "Remove the positive portion of the mesh", nullptr));
#endif // QT_CONFIG(tooltip)
        radial_removeTop_radio->setText(QCoreApplication::translate("SliceRollup", "Remove Positive", nullptr));
        SetMaterial->setText(QCoreApplication::translate("SliceRollup", "Set Cap Material", nullptr));
        SliceFormat->setItemText(0, QCoreApplication::translate("SliceRollup", "Planar", nullptr));
        SliceFormat->setItemText(1, QCoreApplication::translate("SliceRollup", "Radial", nullptr));

#if QT_CONFIG(tooltip)
        AlignToFace->setToolTip(QCoreApplication::translate("SliceRollup", "Select a face that is used to align the gizmo", nullptr));
#endif // QT_CONFIG(tooltip)
        AlignToFace->setText(QCoreApplication::translate("SliceRollup", "Align to Face", nullptr));
        PlanarAxis_groupBox->setTitle(QCoreApplication::translate("SliceRollup", "Slice Along", nullptr));
        PlanarZ->setText(QString());
        PlanarX->setText(QString());
        PlanarY->setText(QString());
        PlanarFlipY->setText(QCoreApplication::translate("SliceRollup", "Flip", nullptr));
        PlanarFlipZ->setText(QCoreApplication::translate("SliceRollup", "Flip", nullptr));
        PlanarFlipX->setText(QCoreApplication::translate("SliceRollup", "Flip", nullptr));
#if QT_CONFIG(tooltip)
        ResetTransform->setToolTip(QCoreApplication::translate("SliceRollup", "Reset Gizmo", nullptr));
#endif // QT_CONFIG(tooltip)
        ResetTransform->setText(QString());
        RadialAxis_groupBox->setTitle(QCoreApplication::translate("SliceRollup", "Slice Along", nullptr));
        RadialX->setText(QString());
        RadialY->setText(QString());
        RadialZ->setText(QString());
        RadialSlice_groupBox->setTitle(QCoreApplication::translate("SliceRollup", "Radial Slice", nullptr));
        MinL->setText(QCoreApplication::translate("SliceRollup", "Angle 1:", nullptr));
        MaxL->setText(QCoreApplication::translate("SliceRollup", "Angle 2:", nullptr));
        OperateL->setText(QCoreApplication::translate("SliceRollup", "Operate On:", nullptr));
        Faces___Polygons_Toggle->setCurrentText(QString());
    } // retranslateUi

};

namespace Ui {
    class SliceRollup: public Ui_SliceRollup {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SLICE_MODIFIER_H
