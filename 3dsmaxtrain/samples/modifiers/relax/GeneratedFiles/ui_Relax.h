/********************************************************************************
** Form generated from reading UI file 'Relax.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_RELAX_H
#define UI_RELAX_H

#include <Qt/QmaxSpinBox.h>
#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_RelaxRollup
{
public:
    QGridLayout *gridLayout;
    QLabel *iterationsL;
    QLabel *amountL;
    MaxSDK::QmaxSpinBox *Iterations;
    MaxSDK::QmaxDoubleSpinBox *Value;
    QCheckBox *Boundary;
    QCheckBox *Saddle;
    QCheckBox *PreserveVolume;

    void setupUi(QWidget *RelaxRollup)
    {
        if (RelaxRollup->objectName().isEmpty())
            RelaxRollup->setObjectName("RelaxRollup");
        RelaxRollup->resize(206, 158);
        gridLayout = new QGridLayout(RelaxRollup);
        gridLayout->setObjectName("gridLayout");
        iterationsL = new QLabel(RelaxRollup);
        iterationsL->setObjectName("iterationsL");
        iterationsL->setLayoutDirection(Qt::LeftToRight);
        iterationsL->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(iterationsL, 1, 0, 1, 1);

        amountL = new QLabel(RelaxRollup);
        amountL->setObjectName("amountL");
        amountL->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(amountL, 0, 0, 1, 1);

        Iterations = new MaxSDK::QmaxSpinBox(RelaxRollup);
        Iterations->setObjectName("Iterations");
        Iterations->setValue(1);

        gridLayout->addWidget(Iterations, 1, 1, 1, 1);

        Value = new MaxSDK::QmaxDoubleSpinBox(RelaxRollup);
        Value->setObjectName("Value");
        Value->setMaximum(1.000000000000000);
        Value->setSingleStep(0.100000000000000);
        Value->setValue(0.100000000000000);

        gridLayout->addWidget(Value, 0, 1, 1, 1);

        Boundary = new QCheckBox(RelaxRollup);
        Boundary->setObjectName("Boundary");
        Boundary->setChecked(true);

        gridLayout->addWidget(Boundary, 2, 0, 1, 2);

        Saddle = new QCheckBox(RelaxRollup);
        Saddle->setObjectName("Saddle");
        Saddle->setChecked(true);

        gridLayout->addWidget(Saddle, 3, 0, 1, 2);

        PreserveVolume = new QCheckBox(RelaxRollup);
        PreserveVolume->setObjectName("PreserveVolume");
        PreserveVolume->setChecked(true);

        gridLayout->addWidget(PreserveVolume, 4, 0, 1, 2);

        QWidget::setTabOrder(Value, Iterations);
        QWidget::setTabOrder(Iterations, Boundary);
        QWidget::setTabOrder(Boundary, Saddle);
        QWidget::setTabOrder(Saddle, PreserveVolume);

        retranslateUi(RelaxRollup);

        QMetaObject::connectSlotsByName(RelaxRollup);
    } // setupUi

    void retranslateUi(QWidget *RelaxRollup)
    {
        RelaxRollup->setWindowTitle(QCoreApplication::translate("RelaxRollup", "Form", nullptr));
#if QT_CONFIG(tooltip)
        iterationsL->setToolTip(QCoreApplication::translate("RelaxRollup", "<html><head/><body><p>Sets the number of times the relax function is applied.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        iterationsL->setText(QCoreApplication::translate("RelaxRollup", "Iterations:", nullptr));
#if QT_CONFIG(tooltip)
        amountL->setToolTip(QCoreApplication::translate("RelaxRollup", "<html><head/><body><p>Sets the amount of relax applied with each iteration.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        amountL->setText(QCoreApplication::translate("RelaxRollup", "Relax Value:", nullptr));
#if QT_CONFIG(tooltip)
        Boundary->setToolTip(QCoreApplication::translate("RelaxRollup", "<html><head/><body><p>Controls whether vertices at the edges of open meshes are moved.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        Boundary->setText(QCoreApplication::translate("RelaxRollup", "Keep Boundary Points Fixed", nullptr));
#if QT_CONFIG(tooltip)
        Saddle->setToolTip(QCoreApplication::translate("RelaxRollup", "<html><head/><body><p>Preserves the original positions of vertices farthest away from the object center.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        Saddle->setText(QCoreApplication::translate("RelaxRollup", "Save Outer Corners", nullptr));
#if QT_CONFIG(tooltip)
        PreserveVolume->setToolTip(QCoreApplication::translate("RelaxRollup", "<html><head/><body><p>Tries to maintain the original shape while relaxing the mesh.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        PreserveVolume->setText(QCoreApplication::translate("RelaxRollup", "Preserve Volume", nullptr));
    } // retranslateUi

};

namespace Ui {
    class RelaxRollup: public Ui_RelaxRollup {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_RELAX_H
