/********************************************************************************
** Form generated from reading UI file 'QHyperlinkSecurityWarningDialog.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QHYPERLINKSECURITYWARNINGDIALOG_H
#define UI_QHYPERLINKSECURITYWARNINGDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>

QT_BEGIN_NAMESPACE

class Ui_hyperlinkSecurityWarningDialog
{
public:
    QGridLayout *gridLayout;
    QLabel *warningIconLabel;
    QSpacerItem *indentSpacer;
    QLabel *messageLabel;
    QHBoxLayout *buttonBoxLayout;
    QPushButton *learnMorePushButton;
    QSpacerItem *buttonBoxSpacer;
    QPushButton *doNotOpenPushButton;
    QPushButton *openPushButton;

    void setupUi(QDialog *hyperlinkSecurityWarningDialog)
    {
        if (hyperlinkSecurityWarningDialog->objectName().isEmpty())
            hyperlinkSecurityWarningDialog->setObjectName("hyperlinkSecurityWarningDialog");
        hyperlinkSecurityWarningDialog->resize(303, 69);
        gridLayout = new QGridLayout(hyperlinkSecurityWarningDialog);
        gridLayout->setObjectName("gridLayout");
        gridLayout->setSizeConstraint(QLayout::SetFixedSize);
        warningIconLabel = new QLabel(hyperlinkSecurityWarningDialog);
        warningIconLabel->setObjectName("warningIconLabel");
        warningIconLabel->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);

        gridLayout->addWidget(warningIconLabel, 0, 0, 2, 1);

        indentSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(indentSpacer, 0, 1, 2, 1);

        messageLabel = new QLabel(hyperlinkSecurityWarningDialog);
        messageLabel->setObjectName("messageLabel");
        messageLabel->setTextFormat(Qt::PlainText);
        messageLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

        gridLayout->addWidget(messageLabel, 0, 2, 1, 1);

        buttonBoxLayout = new QHBoxLayout();
        buttonBoxLayout->setObjectName("buttonBoxLayout");
        learnMorePushButton = new QPushButton(hyperlinkSecurityWarningDialog);
        learnMorePushButton->setObjectName("learnMorePushButton");

        buttonBoxLayout->addWidget(learnMorePushButton);

        buttonBoxSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        buttonBoxLayout->addItem(buttonBoxSpacer);

        doNotOpenPushButton = new QPushButton(hyperlinkSecurityWarningDialog);
        doNotOpenPushButton->setObjectName("doNotOpenPushButton");

        buttonBoxLayout->addWidget(doNotOpenPushButton);

        openPushButton = new QPushButton(hyperlinkSecurityWarningDialog);
        openPushButton->setObjectName("openPushButton");

        buttonBoxLayout->addWidget(openPushButton);


        gridLayout->addLayout(buttonBoxLayout, 2, 0, 1, 4);


        retranslateUi(hyperlinkSecurityWarningDialog);

        QMetaObject::connectSlotsByName(hyperlinkSecurityWarningDialog);
    } // setupUi

    void retranslateUi(QDialog *hyperlinkSecurityWarningDialog)
    {
        hyperlinkSecurityWarningDialog->setWindowTitle(QCoreApplication::translate("hyperlinkSecurityWarningDialog", "Security Warning", nullptr));
        learnMorePushButton->setText(QCoreApplication::translate("hyperlinkSecurityWarningDialog", "Learn More", nullptr));
        doNotOpenPushButton->setText(QCoreApplication::translate("hyperlinkSecurityWarningDialog", "Do Not Open", nullptr));
        openPushButton->setText(QCoreApplication::translate("hyperlinkSecurityWarningDialog", "Open", nullptr));
    } // retranslateUi

};

namespace Ui {
    class hyperlinkSecurityWarningDialog: public Ui_hyperlinkSecurityWarningDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QHYPERLINKSECURITYWARNINGDIALOG_H
