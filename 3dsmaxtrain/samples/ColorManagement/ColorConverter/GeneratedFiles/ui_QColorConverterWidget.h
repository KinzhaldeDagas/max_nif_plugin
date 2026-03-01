/********************************************************************************
** Form generated from reading UI file 'QColorConverterWidget.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QCOLORCONVERTERWIDGET_H
#define UI_QCOLORCONVERTERWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "utils/qcoloreyedropper.h"
#include "utils/qcolorrectwidget.h"

QT_BEGIN_NAMESPACE

class Ui_QColorConverterWidget
{
public:
    QGridLayout *gridLayout_6;
    QGroupBox *groupBox_Cmp;
    QGridLayout *gridLayout_4;
    QColorEyeDropper *pushButton_Cmp_Pick;
    QComboBox *comboBox_Cmp_Space;
    QSpacerItem *verticalSpacer_4;
    QColorRectWidget *widget_Cmp_Rect;
    QVBoxLayout *verticalLayout_4;
    QDoubleSpinBox *doubleSpinBox_Cmp_R;
    QDoubleSpinBox *doubleSpinBox_Cmp_G;
    QDoubleSpinBox *doubleSpinBox_Cmp_B;
    QComboBox *comboBox_Cmp_Mode;
    QGroupBox *groupBox_In;
    QGridLayout *gridLayout;
    QColorEyeDropper *pushButton_In_Pick;
    QColorRectWidget *widget_In_Rect;
    QComboBox *comboBox_In_Space;
    QSpacerItem *verticalSpacer;
    QVBoxLayout *verticalLayout;
    QDoubleSpinBox *doubleSpinBox_In_R;
    QDoubleSpinBox *doubleSpinBox_In_G;
    QDoubleSpinBox *doubleSpinBox_In_B;
    QComboBox *comboBox_In_Mode;
    QGroupBox *groupBox_Out;
    QGridLayout *gridLayout_3;
    QPushButton *pushButton_Out_Clone;
    QVBoxLayout *verticalLayout_3;
    QDoubleSpinBox *doubleSpinBox_Out_R;
    QDoubleSpinBox *doubleSpinBox_Out_G;
    QDoubleSpinBox *doubleSpinBox_Out_B;
    QComboBox *comboBox_Out_Space;
    QDoubleSpinBox *doubleSpinBox_Gamma;
    QDoubleSpinBox *doubleSpinBox_Exposure;
    QColorRectWidget *widget_Out_Rect;
    QSpacerItem *verticalSpacer_3;
    QComboBox *comboBox_Out_Mode;
    QComboBox *comboBox_View_Space;
    QGroupBox *groupBox_Rend;
    QGridLayout *gridLayout_2;
    QComboBox *comboBox_Rend_Space;
    QColorRectWidget *widget_Rend_Rect;
    QSpacerItem *verticalSpacer_2;
    QVBoxLayout *verticalLayout_2;
    QDoubleSpinBox *doubleSpinBox_Rend_R;
    QDoubleSpinBox *doubleSpinBox_Rend_G;
    QDoubleSpinBox *doubleSpinBox_Rend_B;
    QComboBox *comboBox_Rend_Mode;
    QColorEyeDropper *pushButton_Rend_Pick;

    void setupUi(QWidget *QColorConverterWidget)
    {
        if (QColorConverterWidget->objectName().isEmpty())
            QColorConverterWidget->setObjectName("QColorConverterWidget");
        QColorConverterWidget->resize(1120, 212);
        gridLayout_6 = new QGridLayout(QColorConverterWidget);
        gridLayout_6->setObjectName("gridLayout_6");
        groupBox_Cmp = new QGroupBox(QColorConverterWidget);
        groupBox_Cmp->setObjectName("groupBox_Cmp");
        groupBox_Cmp->setFlat(true);
        gridLayout_4 = new QGridLayout(groupBox_Cmp);
        gridLayout_4->setObjectName("gridLayout_4");
        pushButton_Cmp_Pick = new QColorEyeDropper(groupBox_Cmp);
        pushButton_Cmp_Pick->setObjectName("pushButton_Cmp_Pick");
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(pushButton_Cmp_Pick->sizePolicy().hasHeightForWidth());
        pushButton_Cmp_Pick->setSizePolicy(sizePolicy);
        pushButton_Cmp_Pick->setFlat(false);

        gridLayout_4->addWidget(pushButton_Cmp_Pick, 2, 0, 1, 3);

        comboBox_Cmp_Space = new QComboBox(groupBox_Cmp);
        comboBox_Cmp_Space->setObjectName("comboBox_Cmp_Space");
        comboBox_Cmp_Space->setEnabled(false);

        gridLayout_4->addWidget(comboBox_Cmp_Space, 1, 0, 1, 3);

        verticalSpacer_4 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_4->addItem(verticalSpacer_4, 4, 0, 1, 3);

        widget_Cmp_Rect = new QColorRectWidget(groupBox_Cmp);
        widget_Cmp_Rect->setObjectName("widget_Cmp_Rect");
        widget_Cmp_Rect->setMinimumSize(QSize(50, 50));
        widget_Cmp_Rect->setAutoFillBackground(false);

        gridLayout_4->addWidget(widget_Cmp_Rect, 3, 1, 1, 2);

        verticalLayout_4 = new QVBoxLayout();
        verticalLayout_4->setObjectName("verticalLayout_4");
        doubleSpinBox_Cmp_R = new QDoubleSpinBox(groupBox_Cmp);
        doubleSpinBox_Cmp_R->setObjectName("doubleSpinBox_Cmp_R");
        doubleSpinBox_Cmp_R->setEnabled(true);
        doubleSpinBox_Cmp_R->setDecimals(5);
        doubleSpinBox_Cmp_R->setMaximum(9999.989999999999782);
        doubleSpinBox_Cmp_R->setSingleStep(0.100000000000000);

        verticalLayout_4->addWidget(doubleSpinBox_Cmp_R);

        doubleSpinBox_Cmp_G = new QDoubleSpinBox(groupBox_Cmp);
        doubleSpinBox_Cmp_G->setObjectName("doubleSpinBox_Cmp_G");
        doubleSpinBox_Cmp_G->setEnabled(true);
        doubleSpinBox_Cmp_G->setDecimals(5);
        doubleSpinBox_Cmp_G->setMaximum(9999.989999999999782);
        doubleSpinBox_Cmp_G->setSingleStep(0.100000000000000);

        verticalLayout_4->addWidget(doubleSpinBox_Cmp_G);

        doubleSpinBox_Cmp_B = new QDoubleSpinBox(groupBox_Cmp);
        doubleSpinBox_Cmp_B->setObjectName("doubleSpinBox_Cmp_B");
        doubleSpinBox_Cmp_B->setEnabled(true);
        doubleSpinBox_Cmp_B->setDecimals(5);
        doubleSpinBox_Cmp_B->setMaximum(9999.989999999999782);
        doubleSpinBox_Cmp_B->setSingleStep(0.100000000000000);

        verticalLayout_4->addWidget(doubleSpinBox_Cmp_B);


        gridLayout_4->addLayout(verticalLayout_4, 3, 0, 1, 1);

        comboBox_Cmp_Mode = new QComboBox(groupBox_Cmp);
        comboBox_Cmp_Mode->setObjectName("comboBox_Cmp_Mode");
        comboBox_Cmp_Mode->setEnabled(false);

        gridLayout_4->addWidget(comboBox_Cmp_Mode, 0, 0, 1, 3);


        gridLayout_6->addWidget(groupBox_Cmp, 0, 3, 1, 1);

        groupBox_In = new QGroupBox(QColorConverterWidget);
        groupBox_In->setObjectName("groupBox_In");
        groupBox_In->setFlat(true);
        gridLayout = new QGridLayout(groupBox_In);
        gridLayout->setObjectName("gridLayout");
        pushButton_In_Pick = new QColorEyeDropper(groupBox_In);
        pushButton_In_Pick->setObjectName("pushButton_In_Pick");
        sizePolicy.setHeightForWidth(pushButton_In_Pick->sizePolicy().hasHeightForWidth());
        pushButton_In_Pick->setSizePolicy(sizePolicy);
        pushButton_In_Pick->setFlat(false);

        gridLayout->addWidget(pushButton_In_Pick, 2, 0, 1, 2);

        widget_In_Rect = new QColorRectWidget(groupBox_In);
        widget_In_Rect->setObjectName("widget_In_Rect");
        widget_In_Rect->setMinimumSize(QSize(50, 50));
        widget_In_Rect->setAutoFillBackground(false);

        gridLayout->addWidget(widget_In_Rect, 3, 1, 1, 1);

        comboBox_In_Space = new QComboBox(groupBox_In);
        comboBox_In_Space->setObjectName("comboBox_In_Space");

        gridLayout->addWidget(comboBox_In_Space, 1, 0, 1, 2);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout->addItem(verticalSpacer, 4, 0, 1, 2);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName("verticalLayout");
        doubleSpinBox_In_R = new QDoubleSpinBox(groupBox_In);
        doubleSpinBox_In_R->setObjectName("doubleSpinBox_In_R");
        doubleSpinBox_In_R->setDecimals(5);
        doubleSpinBox_In_R->setMaximum(9999.989999999999782);
        doubleSpinBox_In_R->setSingleStep(0.100000000000000);

        verticalLayout->addWidget(doubleSpinBox_In_R);

        doubleSpinBox_In_G = new QDoubleSpinBox(groupBox_In);
        doubleSpinBox_In_G->setObjectName("doubleSpinBox_In_G");
        doubleSpinBox_In_G->setDecimals(5);
        doubleSpinBox_In_G->setMaximum(9999.989999999999782);
        doubleSpinBox_In_G->setSingleStep(0.100000000000000);

        verticalLayout->addWidget(doubleSpinBox_In_G);

        doubleSpinBox_In_B = new QDoubleSpinBox(groupBox_In);
        doubleSpinBox_In_B->setObjectName("doubleSpinBox_In_B");
        doubleSpinBox_In_B->setDecimals(5);
        doubleSpinBox_In_B->setMaximum(9999.989999999999782);
        doubleSpinBox_In_B->setSingleStep(0.100000000000000);

        verticalLayout->addWidget(doubleSpinBox_In_B);


        gridLayout->addLayout(verticalLayout, 3, 0, 1, 1);

        comboBox_In_Mode = new QComboBox(groupBox_In);
        comboBox_In_Mode->setObjectName("comboBox_In_Mode");
        comboBox_In_Mode->setEnabled(false);

        gridLayout->addWidget(comboBox_In_Mode, 0, 0, 1, 2);


        gridLayout_6->addWidget(groupBox_In, 0, 0, 1, 1);

        groupBox_Out = new QGroupBox(QColorConverterWidget);
        groupBox_Out->setObjectName("groupBox_Out");
        groupBox_Out->setFlat(true);
        gridLayout_3 = new QGridLayout(groupBox_Out);
        gridLayout_3->setObjectName("gridLayout_3");
        pushButton_Out_Clone = new QPushButton(groupBox_Out);
        pushButton_Out_Clone->setObjectName("pushButton_Out_Clone");
        pushButton_Out_Clone->setEnabled(true);
        QSizePolicy sizePolicy1(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(pushButton_Out_Clone->sizePolicy().hasHeightForWidth());
        pushButton_Out_Clone->setSizePolicy(sizePolicy1);
        pushButton_Out_Clone->setCheckable(false);

        gridLayout_3->addWidget(pushButton_Out_Clone, 2, 5, 1, 1);

        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName("verticalLayout_3");
        doubleSpinBox_Out_R = new QDoubleSpinBox(groupBox_Out);
        doubleSpinBox_Out_R->setObjectName("doubleSpinBox_Out_R");
        doubleSpinBox_Out_R->setEnabled(false);
        doubleSpinBox_Out_R->setDecimals(5);
        doubleSpinBox_Out_R->setMaximum(9999.989999999999782);
        doubleSpinBox_Out_R->setSingleStep(0.100000000000000);

        verticalLayout_3->addWidget(doubleSpinBox_Out_R);

        doubleSpinBox_Out_G = new QDoubleSpinBox(groupBox_Out);
        doubleSpinBox_Out_G->setObjectName("doubleSpinBox_Out_G");
        doubleSpinBox_Out_G->setEnabled(false);
        doubleSpinBox_Out_G->setDecimals(5);
        doubleSpinBox_Out_G->setMaximum(9999.989999999999782);
        doubleSpinBox_Out_G->setSingleStep(0.100000000000000);

        verticalLayout_3->addWidget(doubleSpinBox_Out_G);

        doubleSpinBox_Out_B = new QDoubleSpinBox(groupBox_Out);
        doubleSpinBox_Out_B->setObjectName("doubleSpinBox_Out_B");
        doubleSpinBox_Out_B->setEnabled(false);
        doubleSpinBox_Out_B->setDecimals(5);
        doubleSpinBox_Out_B->setMaximum(9999.989999999999782);
        doubleSpinBox_Out_B->setSingleStep(0.100000000000000);

        verticalLayout_3->addWidget(doubleSpinBox_Out_B);


        gridLayout_3->addLayout(verticalLayout_3, 3, 0, 1, 1);

        comboBox_Out_Space = new QComboBox(groupBox_Out);
        comboBox_Out_Space->setObjectName("comboBox_Out_Space");

        gridLayout_3->addWidget(comboBox_Out_Space, 1, 0, 1, 6);

        doubleSpinBox_Gamma = new QDoubleSpinBox(groupBox_Out);
        doubleSpinBox_Gamma->setObjectName("doubleSpinBox_Gamma");
        doubleSpinBox_Gamma->setMinimum(0.010000000000000);
        doubleSpinBox_Gamma->setMaximum(5.000000000000000);
        doubleSpinBox_Gamma->setSingleStep(0.100000000000000);
        doubleSpinBox_Gamma->setValue(1.000000000000000);

        gridLayout_3->addWidget(doubleSpinBox_Gamma, 2, 4, 1, 1);

        doubleSpinBox_Exposure = new QDoubleSpinBox(groupBox_Out);
        doubleSpinBox_Exposure->setObjectName("doubleSpinBox_Exposure");
        doubleSpinBox_Exposure->setMinimum(-99.000000000000000);
        doubleSpinBox_Exposure->setSingleStep(0.100000000000000);

        gridLayout_3->addWidget(doubleSpinBox_Exposure, 2, 3, 1, 1);

        widget_Out_Rect = new QColorRectWidget(groupBox_Out);
        widget_Out_Rect->setObjectName("widget_Out_Rect");
        widget_Out_Rect->setMinimumSize(QSize(50, 50));
        widget_Out_Rect->setAutoFillBackground(false);

        gridLayout_3->addWidget(widget_Out_Rect, 3, 1, 1, 5);

        verticalSpacer_3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_3->addItem(verticalSpacer_3, 4, 0, 1, 6);

        comboBox_Out_Mode = new QComboBox(groupBox_Out);
        comboBox_Out_Mode->setObjectName("comboBox_Out_Mode");

        gridLayout_3->addWidget(comboBox_Out_Mode, 0, 0, 1, 6);

        comboBox_View_Space = new QComboBox(groupBox_Out);
        comboBox_View_Space->setObjectName("comboBox_View_Space");

        gridLayout_3->addWidget(comboBox_View_Space, 2, 0, 1, 3);


        gridLayout_6->addWidget(groupBox_Out, 0, 2, 1, 1);

        groupBox_Rend = new QGroupBox(QColorConverterWidget);
        groupBox_Rend->setObjectName("groupBox_Rend");
        groupBox_Rend->setFlat(true);
        gridLayout_2 = new QGridLayout(groupBox_Rend);
        gridLayout_2->setObjectName("gridLayout_2");
        comboBox_Rend_Space = new QComboBox(groupBox_Rend);
        comboBox_Rend_Space->setObjectName("comboBox_Rend_Space");

        gridLayout_2->addWidget(comboBox_Rend_Space, 1, 0, 1, 3);

        widget_Rend_Rect = new QColorRectWidget(groupBox_Rend);
        widget_Rend_Rect->setObjectName("widget_Rend_Rect");
        widget_Rend_Rect->setMinimumSize(QSize(50, 50));
        widget_Rend_Rect->setAutoFillBackground(false);

        gridLayout_2->addWidget(widget_Rend_Rect, 3, 2, 1, 1);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_2->addItem(verticalSpacer_2, 4, 0, 1, 3);

        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName("verticalLayout_2");
        doubleSpinBox_Rend_R = new QDoubleSpinBox(groupBox_Rend);
        doubleSpinBox_Rend_R->setObjectName("doubleSpinBox_Rend_R");
        doubleSpinBox_Rend_R->setEnabled(false);
        doubleSpinBox_Rend_R->setDecimals(5);
        doubleSpinBox_Rend_R->setMaximum(9999.989999999999782);
        doubleSpinBox_Rend_R->setSingleStep(0.100000000000000);

        verticalLayout_2->addWidget(doubleSpinBox_Rend_R);

        doubleSpinBox_Rend_G = new QDoubleSpinBox(groupBox_Rend);
        doubleSpinBox_Rend_G->setObjectName("doubleSpinBox_Rend_G");
        doubleSpinBox_Rend_G->setEnabled(false);
        doubleSpinBox_Rend_G->setDecimals(5);
        doubleSpinBox_Rend_G->setMaximum(9999.989999999999782);
        doubleSpinBox_Rend_G->setSingleStep(0.100000000000000);

        verticalLayout_2->addWidget(doubleSpinBox_Rend_G);

        doubleSpinBox_Rend_B = new QDoubleSpinBox(groupBox_Rend);
        doubleSpinBox_Rend_B->setObjectName("doubleSpinBox_Rend_B");
        doubleSpinBox_Rend_B->setEnabled(false);
        doubleSpinBox_Rend_B->setDecimals(5);
        doubleSpinBox_Rend_B->setMaximum(9999.989999999999782);
        doubleSpinBox_Rend_B->setSingleStep(0.100000000000000);

        verticalLayout_2->addWidget(doubleSpinBox_Rend_B);


        gridLayout_2->addLayout(verticalLayout_2, 3, 0, 1, 2);

        comboBox_Rend_Mode = new QComboBox(groupBox_Rend);
        comboBox_Rend_Mode->setObjectName("comboBox_Rend_Mode");

        gridLayout_2->addWidget(comboBox_Rend_Mode, 0, 0, 1, 3);

        pushButton_Rend_Pick = new QColorEyeDropper(groupBox_Rend);
        pushButton_Rend_Pick->setObjectName("pushButton_Rend_Pick");
        pushButton_Rend_Pick->setEnabled(false);
        sizePolicy1.setHeightForWidth(pushButton_Rend_Pick->sizePolicy().hasHeightForWidth());
        pushButton_Rend_Pick->setSizePolicy(sizePolicy1);
        pushButton_Rend_Pick->setCheckable(false);

        gridLayout_2->addWidget(pushButton_Rend_Pick, 2, 0, 1, 3);


        gridLayout_6->addWidget(groupBox_Rend, 0, 1, 1, 1);


        retranslateUi(QColorConverterWidget);

        pushButton_Cmp_Pick->setDefault(false);
        pushButton_In_Pick->setDefault(false);


        QMetaObject::connectSlotsByName(QColorConverterWidget);
    } // setupUi

    void retranslateUi(QWidget *QColorConverterWidget)
    {
        QColorConverterWidget->setWindowTitle(QCoreApplication::translate("QColorConverterWidget", "ColorConverter", nullptr));
        groupBox_Cmp->setTitle(QCoreApplication::translate("QColorConverterWidget", "Compare", nullptr));
#if QT_CONFIG(tooltip)
        pushButton_Cmp_Pick->setToolTip(QCoreApplication::translate("QColorConverterWidget", "Pick a color to compare with the output color", nullptr));
#endif // QT_CONFIG(tooltip)
        pushButton_Cmp_Pick->setText(QString());
#if QT_CONFIG(tooltip)
        doubleSpinBox_Cmp_R->setToolTip(QCoreApplication::translate("QColorConverterWidget", "Red", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        doubleSpinBox_Cmp_G->setToolTip(QCoreApplication::translate("QColorConverterWidget", "Green", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        doubleSpinBox_Cmp_B->setToolTip(QCoreApplication::translate("QColorConverterWidget", "Blue", nullptr));
#endif // QT_CONFIG(tooltip)
        groupBox_In->setTitle(QCoreApplication::translate("QColorConverterWidget", "Input", nullptr));
#if QT_CONFIG(tooltip)
        pushButton_In_Pick->setToolTip(QCoreApplication::translate("QColorConverterWidget", "Pick a color to convert", nullptr));
#endif // QT_CONFIG(tooltip)
        pushButton_In_Pick->setText(QString());
#if QT_CONFIG(tooltip)
        doubleSpinBox_In_R->setToolTip(QCoreApplication::translate("QColorConverterWidget", "Red", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        doubleSpinBox_In_G->setToolTip(QCoreApplication::translate("QColorConverterWidget", "Green", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        doubleSpinBox_In_B->setToolTip(QCoreApplication::translate("QColorConverterWidget", "Blue", nullptr));
#endif // QT_CONFIG(tooltip)
        groupBox_Out->setTitle(QCoreApplication::translate("QColorConverterWidget", "Output", nullptr));
#if QT_CONFIG(tooltip)
        pushButton_Out_Clone->setToolTip(QCoreApplication::translate("QColorConverterWidget", "Copy the color to compare", nullptr));
#endif // QT_CONFIG(tooltip)
        pushButton_Out_Clone->setText(QCoreApplication::translate("QColorConverterWidget", "Clone", nullptr));
#if QT_CONFIG(tooltip)
        doubleSpinBox_Gamma->setToolTip(QCoreApplication::translate("QColorConverterWidget", "Post Display Gamma", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        doubleSpinBox_Exposure->setToolTip(QCoreApplication::translate("QColorConverterWidget", "Scene Linear Exposure", nullptr));
#endif // QT_CONFIG(tooltip)
        groupBox_Rend->setTitle(QCoreApplication::translate("QColorConverterWidget", "Rendering", nullptr));
        pushButton_Rend_Pick->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class QColorConverterWidget: public Ui_QColorConverterWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QCOLORCONVERTERWIDGET_H
