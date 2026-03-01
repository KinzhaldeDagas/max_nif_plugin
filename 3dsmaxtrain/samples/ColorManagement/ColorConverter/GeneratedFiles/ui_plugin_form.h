/********************************************************************************
** Form generated from reading UI file 'plugin_form.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PLUGIN_FORM_H
#define UI_PLUGIN_FORM_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PluginRollup
{
public:
    QVBoxLayout *verticalLayout;
    QPushButton *pushButton;

    void setupUi(QWidget *PluginRollup)
    {
        if (PluginRollup->objectName().isEmpty())
            PluginRollup->setObjectName("PluginRollup");
        PluginRollup->resize(205, 41);
        verticalLayout = new QVBoxLayout(PluginRollup);
        verticalLayout->setObjectName("verticalLayout");
        pushButton = new QPushButton(PluginRollup);
        pushButton->setObjectName("pushButton");

        verticalLayout->addWidget(pushButton);


        retranslateUi(PluginRollup);

        QMetaObject::connectSlotsByName(PluginRollup);
    } // setupUi

    void retranslateUi(QWidget *PluginRollup)
    {
        PluginRollup->setWindowTitle(QCoreApplication::translate("PluginRollup", "Form", nullptr));
        pushButton->setText(QCoreApplication::translate("PluginRollup", "ColorConverter", nullptr));
    } // retranslateUi

};

namespace Ui {
    class PluginRollup: public Ui_PluginRollup {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PLUGIN_FORM_H
