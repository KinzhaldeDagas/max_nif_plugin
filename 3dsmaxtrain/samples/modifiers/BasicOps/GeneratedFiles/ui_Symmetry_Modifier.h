/********************************************************************************
** Form generated from reading UI file 'Symmetry_Modifier.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SYMMETRY_MODIFIER_H
#define UI_SYMMETRY_MODIFIER_H

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
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SymmetryRollup
{
public:
    QGridLayout *gridLayout_3;
    QPushButton *AlignToFace;
    QPushButton *ResetTransform;
    QGroupBox *PlanarAxis_groupBox;
    QGridLayout *gridLayout_4;
    QCheckBox *PlanarFlipZ;
    QVBoxLayout *PlanarLayout;
    QPushButton *PlanarX;
    QPushButton *PlanarY;
    QPushButton *PlanarZ;
    QCheckBox *PlanarFlipX;
    QCheckBox *PlanarFlipY;
    QGridLayout *RadialLayout;
    QPushButton *RadialX;
    QPushButton *RadialY;
    QPushButton *RadialZ;
    QComboBox *SymmetryFormat;
    QPushButton *ReferenceObject;
    QGroupBox *PlanarOptions_groupBox;
    QGridLayout *gridLayout;
    QCheckBox *RadialMirror;
    QHBoxLayout *horizontalLayout_3;
    QSpacerItem *horizontalSpacer_2;
    QCheckBox *UseProximityThreshold;
    MaxSDK::QmaxSpinBox *RadialCount;
    QCheckBox *weld;
    MaxSDK::QmaxWorldSpinBox *threshold;
    QCheckBox *slice;
    QLabel *labelth;
    QHBoxLayout *horizontalLayout_2;
    QSpacerItem *horizontalSpacer;
    QCheckBox *RadialFlip;
    QLabel *countL;
    MaxSDK::QmaxWorldSpinBox *EdgeCollapsePercent;
    QLabel *labelSliceth;
    QButtonGroup *RadialAxis;

    void setupUi(QWidget *SymmetryRollup)
    {
        if (SymmetryRollup->objectName().isEmpty())
            SymmetryRollup->setObjectName("SymmetryRollup");
        SymmetryRollup->resize(202, 484);
        QFont font;
        font.setFamilies({QString::fromUtf8("MS Shell Dlg 2")});
        SymmetryRollup->setFont(font);
        gridLayout_3 = new QGridLayout(SymmetryRollup);
        gridLayout_3->setObjectName("gridLayout_3");
        AlignToFace = new QPushButton(SymmetryRollup);
        AlignToFace->setObjectName("AlignToFace");
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(AlignToFace->sizePolicy().hasHeightForWidth());
        AlignToFace->setSizePolicy(sizePolicy);
        AlignToFace->setCheckable(true);

        gridLayout_3->addWidget(AlignToFace, 2, 0, 1, 1);

        ResetTransform = new QPushButton(SymmetryRollup);
        ResetTransform->setObjectName("ResetTransform");
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(ResetTransform->sizePolicy().hasHeightForWidth());
        ResetTransform->setSizePolicy(sizePolicy1);
        ResetTransform->setMinimumSize(QSize(23, 0));
        ResetTransform->setMaximumSize(QSize(23, 16777215));
        ResetTransform->setFont(font);
        QIcon icon;
        icon.addFile(QString::fromUtf8("C:/ICONS CATALOGUE/Dark/MaxScript/ParamWiring/Refresh_32.png"), QSize(), QIcon::Normal, QIcon::Off);
        ResetTransform->setIcon(icon);

        gridLayout_3->addWidget(ResetTransform, 2, 1, 1, 1);

        PlanarAxis_groupBox = new QGroupBox(SymmetryRollup);
        PlanarAxis_groupBox->setObjectName("PlanarAxis_groupBox");
        gridLayout_4 = new QGridLayout(PlanarAxis_groupBox);
        gridLayout_4->setObjectName("gridLayout_4");
        PlanarFlipZ = new QCheckBox(PlanarAxis_groupBox);
        PlanarFlipZ->setObjectName("PlanarFlipZ");

        gridLayout_4->addWidget(PlanarFlipZ, 4, 1, 1, 1);

        PlanarLayout = new QVBoxLayout();
        PlanarLayout->setObjectName("PlanarLayout");
        PlanarX = new QPushButton(PlanarAxis_groupBox);
        PlanarX->setObjectName("PlanarX");
        QSizePolicy sizePolicy2(QSizePolicy::Ignored, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(PlanarX->sizePolicy().hasHeightForWidth());
        PlanarX->setSizePolicy(sizePolicy2);
        QIcon icon1;
        icon1.addFile(QString::fromUtf8("C:/ICONS CATALOGUE/Dark/EditUVW/AlignToX_48.png"), QSize(), QIcon::Normal, QIcon::Off);
        PlanarX->setIcon(icon1);
        PlanarX->setCheckable(true);

        PlanarLayout->addWidget(PlanarX);

        PlanarY = new QPushButton(PlanarAxis_groupBox);
        PlanarY->setObjectName("PlanarY");
        sizePolicy2.setHeightForWidth(PlanarY->sizePolicy().hasHeightForWidth());
        PlanarY->setSizePolicy(sizePolicy2);
        QIcon icon2;
        icon2.addFile(QString::fromUtf8("C:/ICONS CATALOGUE/Dark/EditUVW/AlignToY_48.png"), QSize(), QIcon::Normal, QIcon::Off);
        PlanarY->setIcon(icon2);
        PlanarY->setCheckable(true);

        PlanarLayout->addWidget(PlanarY);

        PlanarZ = new QPushButton(PlanarAxis_groupBox);
        PlanarZ->setObjectName("PlanarZ");
        sizePolicy2.setHeightForWidth(PlanarZ->sizePolicy().hasHeightForWidth());
        PlanarZ->setSizePolicy(sizePolicy2);
        QIcon icon3;
        icon3.addFile(QString::fromUtf8("C:/ICONS CATALOGUE/Dark/EditUVW/AlignToZ_48.png"), QSize(), QIcon::Normal, QIcon::Off);
        PlanarZ->setIcon(icon3);
        PlanarZ->setCheckable(true);
        PlanarZ->setChecked(true);

        PlanarLayout->addWidget(PlanarZ);


        gridLayout_4->addLayout(PlanarLayout, 2, 0, 3, 1);

        PlanarFlipX = new QCheckBox(PlanarAxis_groupBox);
        PlanarFlipX->setObjectName("PlanarFlipX");

        gridLayout_4->addWidget(PlanarFlipX, 2, 1, 1, 1);

        PlanarFlipY = new QCheckBox(PlanarAxis_groupBox);
        PlanarFlipY->setObjectName("PlanarFlipY");

        gridLayout_4->addWidget(PlanarFlipY, 3, 1, 1, 1);

        RadialLayout = new QGridLayout();
        RadialLayout->setObjectName("RadialLayout");
        RadialX = new QPushButton(PlanarAxis_groupBox);
        RadialAxis = new QButtonGroup(SymmetryRollup);
        RadialAxis->setObjectName("RadialAxis");
        RadialAxis->addButton(RadialX);
        RadialX->setObjectName("RadialX");
        sizePolicy2.setHeightForWidth(RadialX->sizePolicy().hasHeightForWidth());
        RadialX->setSizePolicy(sizePolicy2);
        RadialX->setIcon(icon1);
        RadialX->setCheckable(true);

        RadialLayout->addWidget(RadialX, 0, 0, 1, 1);

        RadialY = new QPushButton(PlanarAxis_groupBox);
        RadialAxis->addButton(RadialY);
        RadialY->setObjectName("RadialY");
        sizePolicy2.setHeightForWidth(RadialY->sizePolicy().hasHeightForWidth());
        RadialY->setSizePolicy(sizePolicy2);
        RadialY->setIcon(icon2);
        RadialY->setCheckable(true);

        RadialLayout->addWidget(RadialY, 0, 1, 1, 1);

        RadialZ = new QPushButton(PlanarAxis_groupBox);
        RadialAxis->addButton(RadialZ);
        RadialZ->setObjectName("RadialZ");
        sizePolicy2.setHeightForWidth(RadialZ->sizePolicy().hasHeightForWidth());
        RadialZ->setSizePolicy(sizePolicy2);
        RadialZ->setIcon(icon3);
        RadialZ->setCheckable(true);
        RadialZ->setChecked(true);

        RadialLayout->addWidget(RadialZ, 0, 2, 1, 1);


        gridLayout_4->addLayout(RadialLayout, 0, 0, 1, 2);


        gridLayout_3->addWidget(PlanarAxis_groupBox, 1, 0, 1, 2);

        SymmetryFormat = new QComboBox(SymmetryRollup);
        SymmetryFormat->addItem(QString());
        SymmetryFormat->addItem(QString());
        SymmetryFormat->setObjectName("SymmetryFormat");

        gridLayout_3->addWidget(SymmetryFormat, 0, 0, 1, 2);

        ReferenceObject = new QPushButton(SymmetryRollup);
        ReferenceObject->setObjectName("ReferenceObject");
        sizePolicy.setHeightForWidth(ReferenceObject->sizePolicy().hasHeightForWidth());
        ReferenceObject->setSizePolicy(sizePolicy);

        gridLayout_3->addWidget(ReferenceObject, 3, 0, 1, 2);

        PlanarOptions_groupBox = new QGroupBox(SymmetryRollup);
        PlanarOptions_groupBox->setObjectName("PlanarOptions_groupBox");
        gridLayout = new QGridLayout(PlanarOptions_groupBox);
        gridLayout->setObjectName("gridLayout");
        RadialMirror = new QCheckBox(PlanarOptions_groupBox);
        RadialMirror->setObjectName("RadialMirror");

        gridLayout->addWidget(RadialMirror, 1, 0, 1, 3);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName("horizontalLayout_3");
        horizontalSpacer_2 = new QSpacerItem(10, 0, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_2);

        UseProximityThreshold = new QCheckBox(PlanarOptions_groupBox);
        UseProximityThreshold->setObjectName("UseProximityThreshold");

        horizontalLayout_3->addWidget(UseProximityThreshold);


        gridLayout->addLayout(horizontalLayout_3, 7, 0, 1, 3);

        RadialCount = new MaxSDK::QmaxSpinBox(PlanarOptions_groupBox);
        RadialCount->setObjectName("RadialCount");
        RadialCount->setMinimum(2);
        RadialCount->setMaximum(1000);

        gridLayout->addWidget(RadialCount, 0, 1, 1, 2);

        weld = new QCheckBox(PlanarOptions_groupBox);
        weld->setObjectName("weld");

        gridLayout->addWidget(weld, 5, 0, 1, 3);

        threshold = new MaxSDK::QmaxWorldSpinBox(PlanarOptions_groupBox);
        threshold->setObjectName("threshold");
        threshold->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);

        gridLayout->addWidget(threshold, 6, 2, 1, 1);

        slice = new QCheckBox(PlanarOptions_groupBox);
        slice->setObjectName("slice");

        gridLayout->addWidget(slice, 3, 0, 1, 3);

        labelth = new QLabel(PlanarOptions_groupBox);
        labelth->setObjectName("labelth");
        labelth->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(labelth, 6, 0, 1, 2);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        horizontalSpacer = new QSpacerItem(10, 0, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);

        RadialFlip = new QCheckBox(PlanarOptions_groupBox);
        RadialFlip->setObjectName("RadialFlip");

        horizontalLayout_2->addWidget(RadialFlip);


        gridLayout->addLayout(horizontalLayout_2, 2, 0, 1, 3);

        countL = new QLabel(PlanarOptions_groupBox);
        countL->setObjectName("countL");
        countL->setLayoutDirection(Qt::LeftToRight);
        countL->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(countL, 0, 0, 1, 1);

        EdgeCollapsePercent = new MaxSDK::QmaxWorldSpinBox(PlanarOptions_groupBox);
        EdgeCollapsePercent->setObjectName("EdgeCollapsePercent");
        EdgeCollapsePercent->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);
        EdgeCollapsePercent->setValue(0.010000000000000);

        gridLayout->addWidget(EdgeCollapsePercent, 4, 2, 1, 1);

        labelSliceth = new QLabel(PlanarOptions_groupBox);
        labelSliceth->setObjectName("labelSliceth");
        labelSliceth->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(labelSliceth, 4, 0, 1, 2);


        gridLayout_3->addWidget(PlanarOptions_groupBox, 4, 0, 1, 2);

        QWidget::setTabOrder(SymmetryFormat, RadialX);
        QWidget::setTabOrder(RadialX, RadialY);
        QWidget::setTabOrder(RadialY, RadialZ);
        QWidget::setTabOrder(RadialZ, PlanarX);
        QWidget::setTabOrder(PlanarX, PlanarFlipX);
        QWidget::setTabOrder(PlanarFlipX, PlanarY);
        QWidget::setTabOrder(PlanarY, PlanarFlipY);
        QWidget::setTabOrder(PlanarFlipY, PlanarZ);
        QWidget::setTabOrder(PlanarZ, PlanarFlipZ);
        QWidget::setTabOrder(PlanarFlipZ, AlignToFace);
        QWidget::setTabOrder(AlignToFace, ResetTransform);
        QWidget::setTabOrder(ResetTransform, ReferenceObject);
        QWidget::setTabOrder(ReferenceObject, RadialCount);
        QWidget::setTabOrder(RadialCount, RadialMirror);
        QWidget::setTabOrder(RadialMirror, RadialFlip);
        QWidget::setTabOrder(RadialFlip, slice);
        QWidget::setTabOrder(slice, weld);
        QWidget::setTabOrder(weld, threshold);

        retranslateUi(SymmetryRollup);

        SymmetryFormat->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(SymmetryRollup);
    } // setupUi

    void retranslateUi(QWidget *SymmetryRollup)
    {
        SymmetryRollup->setWindowTitle(QCoreApplication::translate("SymmetryRollup", "Form", nullptr));
#if QT_CONFIG(tooltip)
        AlignToFace->setToolTip(QCoreApplication::translate("SymmetryRollup", "Select a face that is used to align the gizmo", nullptr));
#endif // QT_CONFIG(tooltip)
        AlignToFace->setText(QCoreApplication::translate("SymmetryRollup", "Align to Face", nullptr));
#if QT_CONFIG(tooltip)
        ResetTransform->setToolTip(QCoreApplication::translate("SymmetryRollup", "Reset Gizmo", nullptr));
#endif // QT_CONFIG(tooltip)
        ResetTransform->setText(QString());
        PlanarAxis_groupBox->setTitle(QCoreApplication::translate("SymmetryRollup", "Mirror Axis:", nullptr));
        PlanarFlipZ->setText(QCoreApplication::translate("SymmetryRollup", "Flip", nullptr));
        PlanarX->setText(QString());
        PlanarY->setText(QString());
        PlanarZ->setText(QString());
        PlanarFlipX->setText(QCoreApplication::translate("SymmetryRollup", "Flip", nullptr));
        PlanarFlipY->setText(QCoreApplication::translate("SymmetryRollup", "Flip", nullptr));
        RadialX->setText(QString());
        RadialY->setText(QString());
        RadialZ->setText(QString());
        SymmetryFormat->setItemText(0, QCoreApplication::translate("SymmetryRollup", "Planar", nullptr));
        SymmetryFormat->setItemText(1, QCoreApplication::translate("SymmetryRollup", "Radial", nullptr));

#if QT_CONFIG(tooltip)
        ReferenceObject->setToolTip(QCoreApplication::translate("SymmetryRollup", "Select to pick a reference object from your scene", nullptr));
#endif // QT_CONFIG(tooltip)
        ReferenceObject->setText(QCoreApplication::translate("SymmetryRollup", "Pick Object", nullptr));
        PlanarOptions_groupBox->setTitle(QCoreApplication::translate("SymmetryRollup", "Symmetry Options:", nullptr));
#if QT_CONFIG(tooltip)
        RadialMirror->setToolTip(QCoreApplication::translate("SymmetryRollup", "Mirrors half of geometry to generate clean seams", nullptr));
#endif // QT_CONFIG(tooltip)
        RadialMirror->setText(QCoreApplication::translate("SymmetryRollup", "Mirror Symmetry", nullptr));
#if QT_CONFIG(tooltip)
        UseProximityThreshold->setToolTip(QCoreApplication::translate("SymmetryRollup", "Weld vertices based on distance.", nullptr));
#endif // QT_CONFIG(tooltip)
        UseProximityThreshold->setText(QCoreApplication::translate("SymmetryRollup", "Use Proximity Weld", nullptr));
#if QT_CONFIG(tooltip)
        weld->setToolTip(QCoreApplication::translate("SymmetryRollup", "Weld vertices from cut seams", nullptr));
#endif // QT_CONFIG(tooltip)
        weld->setText(QCoreApplication::translate("SymmetryRollup", "Weld Seam", nullptr));
#if QT_CONFIG(tooltip)
        slice->setToolTip(QCoreApplication::translate("SymmetryRollup", "Removes original mesh outside of the slice planes ", nullptr));
#endif // QT_CONFIG(tooltip)
        slice->setText(QCoreApplication::translate("SymmetryRollup", "Slice Along Mirror", nullptr));
        labelth->setText(QCoreApplication::translate("SymmetryRollup", "Threshold:", nullptr));
#if QT_CONFIG(tooltip)
        RadialFlip->setToolTip(QCoreApplication::translate("SymmetryRollup", "Flip direction of Mirror Symmetry", nullptr));
#endif // QT_CONFIG(tooltip)
        RadialFlip->setText(QCoreApplication::translate("SymmetryRollup", "Flip", nullptr));
        countL->setText(QCoreApplication::translate("SymmetryRollup", "Count:", nullptr));
        labelSliceth->setText(QCoreApplication::translate("SymmetryRollup", "Threshold:", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SymmetryRollup: public Ui_SymmetryRollup {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SYMMETRY_MODIFIER_H
