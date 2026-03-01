/********************************************************************************
** Form generated from reading UI file 'ClustMod.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CLUSTMOD_H
#define UI_CLUSTMOD_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ClustModRollup
{
public:
    QGridLayout *gridLayout;
    QCheckBox *PreserveNormals;

    void setupUi(QWidget *ClustModRollup)
    {
        if (ClustModRollup->objectName().isEmpty())
            ClustModRollup->setObjectName("ClustModRollup");
        ClustModRollup->resize(227, 48);
        gridLayout = new QGridLayout(ClustModRollup);
        gridLayout->setObjectName("gridLayout");
        PreserveNormals = new QCheckBox(ClustModRollup);
        PreserveNormals->setObjectName("PreserveNormals");

        gridLayout->addWidget(PreserveNormals, 0, 0, 1, 1);


        retranslateUi(ClustModRollup);

        QMetaObject::connectSlotsByName(ClustModRollup);
    } // setupUi

    void retranslateUi(QWidget *ClustModRollup)
    {
        ClustModRollup->setWindowTitle(QCoreApplication::translate("ClustModRollup", "Form", nullptr));
        PreserveNormals->setText(QCoreApplication::translate("ClustModRollup", "Preserve Normals", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ClustModRollup: public Ui_ClustModRollup {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CLUSTMOD_H
