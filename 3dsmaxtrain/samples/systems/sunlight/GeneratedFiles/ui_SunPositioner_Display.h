/********************************************************************************
** Form generated from reading UI file 'SunPositioner_Display.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SUNPOSITIONER_DISPLAY_H
#define UI_SUNPOSITIONER_DISPLAY_H

#include <Qt/QmaxSpinBox.h>
#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <qt/QmaxSpinBox.h>

QT_BEGIN_NAMESPACE

class Ui_SunPositioner_Display
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox;
    QGridLayout *gridLayout;
    QLabel *label_3;
    QLabel *label;
    MaxSDK::QmaxDoubleSpinBox *north_direction_deg;
    QCheckBox *show_compass;
    MaxSDK::QmaxWorldSpinBox *compass_radius;
    QGroupBox *groupBox_2;
    QGridLayout *gridLayout_2;
    QLabel *label_2;
    MaxSDK::QmaxWorldSpinBox *sun_distance;

    void setupUi(QWidget *SunPositioner_Display)
    {
        if (SunPositioner_Display->objectName().isEmpty())
            SunPositioner_Display->setObjectName("SunPositioner_Display");
        SunPositioner_Display->resize(188, 209);
        verticalLayout = new QVBoxLayout(SunPositioner_Display);
        verticalLayout->setObjectName("verticalLayout");
        groupBox = new QGroupBox(SunPositioner_Display);
        groupBox->setObjectName("groupBox");
        gridLayout = new QGridLayout(groupBox);
        gridLayout->setObjectName("gridLayout");
        label_3 = new QLabel(groupBox);
        label_3->setObjectName("label_3");
        label_3->setMargin(0);

        gridLayout->addWidget(label_3, 4, 0, 1, 1);

        label = new QLabel(groupBox);
        label->setObjectName("label");
        label->setMargin(0);

        gridLayout->addWidget(label, 2, 0, 1, 1);

        north_direction_deg = new MaxSDK::QmaxDoubleSpinBox(groupBox);
        north_direction_deg->setObjectName("north_direction_deg");

        gridLayout->addWidget(north_direction_deg, 4, 1, 1, 1);

        show_compass = new QCheckBox(groupBox);
        show_compass->setObjectName("show_compass");
        show_compass->setChecked(true);

        gridLayout->addWidget(show_compass, 0, 0, 1, 2);

        compass_radius = new MaxSDK::QmaxWorldSpinBox(groupBox);
        compass_radius->setObjectName("compass_radius");

        gridLayout->addWidget(compass_radius, 2, 1, 1, 1);


        verticalLayout->addWidget(groupBox);

        groupBox_2 = new QGroupBox(SunPositioner_Display);
        groupBox_2->setObjectName("groupBox_2");
        gridLayout_2 = new QGridLayout(groupBox_2);
        gridLayout_2->setObjectName("gridLayout_2");
        label_2 = new QLabel(groupBox_2);
        label_2->setObjectName("label_2");

        gridLayout_2->addWidget(label_2, 0, 0, 1, 1);

        sun_distance = new MaxSDK::QmaxWorldSpinBox(groupBox_2);
        sun_distance->setObjectName("sun_distance");

        gridLayout_2->addWidget(sun_distance, 0, 1, 1, 1);


        verticalLayout->addWidget(groupBox_2);


        retranslateUi(SunPositioner_Display);
        QObject::connect(show_compass, &QCheckBox::toggled, label, &QLabel::setEnabled);
        QObject::connect(show_compass, SIGNAL(toggled(bool)), compass_radius, SLOT(setEnabled(bool)));

        QMetaObject::connectSlotsByName(SunPositioner_Display);
    } // setupUi

    void retranslateUi(QWidget *SunPositioner_Display)
    {
        SunPositioner_Display->setWindowTitle(QCoreApplication::translate("SunPositioner_Display", "Form", nullptr));
        groupBox->setTitle(QCoreApplication::translate("SunPositioner_Display", "Compass Rose", nullptr));
        label_3->setText(QCoreApplication::translate("SunPositioner_Display", "North Offset:", nullptr));
        label->setText(QCoreApplication::translate("SunPositioner_Display", "    Radius:", nullptr));
#if QT_CONFIG(tooltip)
        north_direction_deg->setToolTip(QCoreApplication::translate("SunPositioner_Display", "Rotates the compass rose, changing the cardinal directions used for positioning the sun according to date and time.", nullptr));
#endif // QT_CONFIG(tooltip)
        north_direction_deg->setSuffix(QCoreApplication::translate("SunPositioner_Display", "\313\232", nullptr));
#if QT_CONFIG(tooltip)
        show_compass->setToolTip(QCoreApplication::translate("SunPositioner_Display", "Shows or hides the compass rose from the viewport.", nullptr));
#endif // QT_CONFIG(tooltip)
        show_compass->setText(QCoreApplication::translate("SunPositioner_Display", "Show", nullptr));
#if QT_CONFIG(tooltip)
        compass_radius->setToolTip(QCoreApplication::translate("SunPositioner_Display", "The size of the compass rose, as displayed in the viewport.", nullptr));
#endif // QT_CONFIG(tooltip)
        groupBox_2->setTitle(QCoreApplication::translate("SunPositioner_Display", "Sun", nullptr));
        label_2->setText(QCoreApplication::translate("SunPositioner_Display", "Distance:", nullptr));
#if QT_CONFIG(tooltip)
        sun_distance->setToolTip(QCoreApplication::translate("SunPositioner_Display", "The distance of the sun from the compass rose, as displayed in the viewport.", nullptr));
#endif // QT_CONFIG(tooltip)
    } // retranslateUi

};

namespace Ui {
    class SunPositioner_Display: public Ui_SunPositioner_Display {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SUNPOSITIONER_DISPLAY_H
