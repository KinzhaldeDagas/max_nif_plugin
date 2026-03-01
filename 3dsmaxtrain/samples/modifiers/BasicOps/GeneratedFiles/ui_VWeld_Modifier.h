/********************************************************************************
** Form generated from reading UI file 'VWeld_Modifier.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_VWELD_MODIFIER_H
#define UI_VWELD_MODIFIER_H

#include <Qt/QmaxSpinBox.h>
#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_VWeldRollup
{
public:
    QGridLayout *gridLayout_5;
    MaxSDK::QmaxDoubleSpinBox *threshold;
    QLabel *label;
    QGroupBox *statusGroupBox;
    QGridLayout *gridLayout_6;
    QLabel *numberOfClones;

    void setupUi(QWidget *VWeldRollup)
    {
        if (VWeldRollup->objectName().isEmpty())
            VWeldRollup->setObjectName("VWeldRollup");
        VWeldRollup->resize(289, 96);
        QFont font;
        font.setFamilies({QString::fromUtf8("MS Shell Dlg 2")});
        VWeldRollup->setFont(font);
        gridLayout_5 = new QGridLayout(VWeldRollup);
        gridLayout_5->setObjectName("gridLayout_5");
        threshold = new MaxSDK::QmaxDoubleSpinBox(VWeldRollup);
        threshold->setObjectName("threshold");

        gridLayout_5->addWidget(threshold, 2, 1, 1, 1);

        label = new QLabel(VWeldRollup);
        label->setObjectName("label");
        label->setLayoutDirection(Qt::LeftToRight);
        label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_5->addWidget(label, 2, 0, 1, 1);

        statusGroupBox = new QGroupBox(VWeldRollup);
        statusGroupBox->setObjectName("statusGroupBox");
        statusGroupBox->setEnabled(true);
        statusGroupBox->setFlat(true);
        statusGroupBox->setCheckable(false);
        statusGroupBox->setChecked(false);
        gridLayout_6 = new QGridLayout(statusGroupBox);
        gridLayout_6->setObjectName("gridLayout_6");
        gridLayout_6->setHorizontalSpacing(5);
        numberOfClones = new QLabel(statusGroupBox);
        numberOfClones->setObjectName("numberOfClones");
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(numberOfClones->sizePolicy().hasHeightForWidth());
        numberOfClones->setSizePolicy(sizePolicy);
        numberOfClones->setFont(font);
        numberOfClones->setFrameShape(QFrame::Box);
        numberOfClones->setFrameShadow(QFrame::Sunken);
        numberOfClones->setAlignment(Qt::AlignCenter);

        gridLayout_6->addWidget(numberOfClones, 0, 0, 1, 1);


        gridLayout_5->addWidget(statusGroupBox, 3, 0, 1, 2);

        gridLayout_5->setColumnStretch(0, 1);
        gridLayout_5->setColumnStretch(1, 2);

        retranslateUi(VWeldRollup);

        QMetaObject::connectSlotsByName(VWeldRollup);
    } // setupUi

    void retranslateUi(QWidget *VWeldRollup)
    {
        VWeldRollup->setWindowTitle(QCoreApplication::translate("VWeldRollup", "Form", nullptr));
#if QT_CONFIG(tooltip)
        threshold->setToolTip(QCoreApplication::translate("VWeldRollup", "The distance, in scene units, within which vertices are automatically combined.", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        label->setToolTip(QCoreApplication::translate("VWeldRollup", "Vertex weld threshold", nullptr));
#endif // QT_CONFIG(tooltip)
        label->setText(QCoreApplication::translate("VWeldRollup", "Threshold", nullptr));
        statusGroupBox->setTitle(QCoreApplication::translate("VWeldRollup", "Vertex Count", nullptr));
        numberOfClones->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class VWeldRollup: public Ui_VWeldRollup {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_VWELD_MODIFIER_H
