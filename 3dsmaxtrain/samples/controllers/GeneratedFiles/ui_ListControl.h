/********************************************************************************
** Form generated from reading UI file 'ListControl.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LISTCONTROL_H
#define UI_LISTCONTROL_H

#include <Qt/QmaxSpinBox.h>
#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "listControlUI.h"

QT_BEGIN_NAMESPACE

class Ui_ListControlRollup
{
public:
    QGridLayout *gridLayout;
    QGroupBox *groupBoxWeight;
    QVBoxLayout *verticalLayout;
    QCheckBox *average;
    QGroupBox *groupBoxDependency;
    QVBoxLayout *verticalLayout_4;
    QRadioButton *radioFan;
    QRadioButton *radioChain;
    QGroupBox *groupBoxWeightMethod;
    QVBoxLayout *verticalLayout_3;
    QRadioButton *radioAgainstIdentity;
    QRadioButton *radioLerpPrevious;
    QHBoxLayout *horizontalLayout;
    QRadioButton *radioWeightMode;
    QRadioButton *radioIndexMode;
    QGridLayout *gridLayoutTable;
    ListControlTableView *tableView;
    QGridLayout *buttonLayoutBelow;
    QToolButton *btnSetActiveBelow;
    QToolButton *btnDeleteBelow;
    QToolButton *btnCutBelow;
    QToolButton *btnPasteBelow;
    QToolButton *btnAppendBelow;
    QToolButton *btnAssignBelow;
    QGroupBox *groupBoxIndex;
    QGridLayout *gridLayout_3;
    MaxSDK::QmaxSpinBox *index;
    QLineEdit *lineEditByName;
    QLabel *labelIndex;
    QLabel *labelByName;

    void setupUi(QWidget *ListControlRollup)
    {
        if (ListControlRollup->objectName().isEmpty())
            ListControlRollup->setObjectName("ListControlRollup");
        ListControlRollup->resize(155, 445);
        QSizePolicy sizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(ListControlRollup->sizePolicy().hasHeightForWidth());
        ListControlRollup->setSizePolicy(sizePolicy);
        ListControlRollup->setMinimumSize(QSize(40, 100));
        gridLayout = new QGridLayout(ListControlRollup);
        gridLayout->setObjectName("gridLayout");
        groupBoxWeight = new QGroupBox(ListControlRollup);
        groupBoxWeight->setObjectName("groupBoxWeight");
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Maximum);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(groupBoxWeight->sizePolicy().hasHeightForWidth());
        groupBoxWeight->setSizePolicy(sizePolicy1);
        groupBoxWeight->setFlat(true);
        verticalLayout = new QVBoxLayout(groupBoxWeight);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setSizeConstraint(QLayout::SetDefaultConstraint);
        verticalLayout->setContentsMargins(3, 3, 3, 3);
        average = new QCheckBox(groupBoxWeight);
        average->setObjectName("average");

        verticalLayout->addWidget(average);

        groupBoxDependency = new QGroupBox(groupBoxWeight);
        groupBoxDependency->setObjectName("groupBoxDependency");
        groupBoxDependency->setFlat(true);
        verticalLayout_4 = new QVBoxLayout(groupBoxDependency);
        verticalLayout_4->setObjectName("verticalLayout_4");
        verticalLayout_4->setContentsMargins(3, 3, 3, 3);
        radioFan = new QRadioButton(groupBoxDependency);
        radioFan->setObjectName("radioFan");
        radioFan->setChecked(true);

        verticalLayout_4->addWidget(radioFan);

        radioChain = new QRadioButton(groupBoxDependency);
        radioChain->setObjectName("radioChain");

        verticalLayout_4->addWidget(radioChain);


        verticalLayout->addWidget(groupBoxDependency);

        groupBoxWeightMethod = new QGroupBox(groupBoxWeight);
        groupBoxWeightMethod->setObjectName("groupBoxWeightMethod");
        QFont font;
        font.setFamilies({QString::fromUtf8("MS Shell Dlg 2")});
        groupBoxWeightMethod->setFont(font);
        groupBoxWeightMethod->setFlat(true);
        verticalLayout_3 = new QVBoxLayout(groupBoxWeightMethod);
        verticalLayout_3->setObjectName("verticalLayout_3");
        verticalLayout_3->setContentsMargins(3, 3, 3, 0);
        radioAgainstIdentity = new QRadioButton(groupBoxWeightMethod);
        radioAgainstIdentity->setObjectName("radioAgainstIdentity");
        radioAgainstIdentity->setChecked(true);

        verticalLayout_3->addWidget(radioAgainstIdentity);

        radioLerpPrevious = new QRadioButton(groupBoxWeightMethod);
        radioLerpPrevious->setObjectName("radioLerpPrevious");
        radioLerpPrevious->setChecked(false);

        verticalLayout_3->addWidget(radioLerpPrevious);


        verticalLayout->addWidget(groupBoxWeightMethod);


        gridLayout->addWidget(groupBoxWeight, 7, 0, 1, 1);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalLayout->setSizeConstraint(QLayout::SetDefaultConstraint);
        radioWeightMode = new QRadioButton(ListControlRollup);
        radioWeightMode->setObjectName("radioWeightMode");
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(radioWeightMode->sizePolicy().hasHeightForWidth());
        radioWeightMode->setSizePolicy(sizePolicy2);
        radioWeightMode->setChecked(true);

        horizontalLayout->addWidget(radioWeightMode);

        radioIndexMode = new QRadioButton(ListControlRollup);
        radioIndexMode->setObjectName("radioIndexMode");
        sizePolicy2.setHeightForWidth(radioIndexMode->sizePolicy().hasHeightForWidth());
        radioIndexMode->setSizePolicy(sizePolicy2);

        horizontalLayout->addWidget(radioIndexMode);


        gridLayout->addLayout(horizontalLayout, 6, 0, 1, 1);

        gridLayoutTable = new QGridLayout();
        gridLayoutTable->setObjectName("gridLayoutTable");
        tableView = new ListControlTableView(ListControlRollup);
        tableView->setObjectName("tableView");
        QSizePolicy sizePolicy3(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy3.setHorizontalStretch(1);
        sizePolicy3.setVerticalStretch(1);
        sizePolicy3.setHeightForWidth(tableView->sizePolicy().hasHeightForWidth());
        tableView->setSizePolicy(sizePolicy3);
        tableView->setContextMenuPolicy(Qt::CustomContextMenu);
        tableView->setFrameShape(QFrame::NoFrame);
        tableView->setFrameShadow(QFrame::Plain);
        tableView->setEditTriggers(QAbstractItemView::DoubleClicked);
        tableView->setSelectionMode(QAbstractItemView::SingleSelection);
        tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        tableView->horizontalHeader()->setMinimumSectionSize(25);
        tableView->horizontalHeader()->setDefaultSectionSize(75);
        tableView->horizontalHeader()->setHighlightSections(false);
        tableView->verticalHeader()->setVisible(false);
        tableView->verticalHeader()->setDefaultSectionSize(23);
        tableView->verticalHeader()->setHighlightSections(false);

        gridLayoutTable->addWidget(tableView, 0, 0, 1, 1);


        gridLayout->addLayout(gridLayoutTable, 0, 0, 1, 1);

        buttonLayoutBelow = new QGridLayout();
        buttonLayoutBelow->setObjectName("buttonLayoutBelow");
        btnSetActiveBelow = new QToolButton(ListControlRollup);
        btnSetActiveBelow->setObjectName("btnSetActiveBelow");
        btnSetActiveBelow->setEnabled(false);
        QSizePolicy sizePolicy4(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(btnSetActiveBelow->sizePolicy().hasHeightForWidth());
        btnSetActiveBelow->setSizePolicy(sizePolicy4);

        buttonLayoutBelow->addWidget(btnSetActiveBelow, 2, 0, 1, 1);

        btnDeleteBelow = new QToolButton(ListControlRollup);
        btnDeleteBelow->setObjectName("btnDeleteBelow");
        btnDeleteBelow->setEnabled(false);
        sizePolicy4.setHeightForWidth(btnDeleteBelow->sizePolicy().hasHeightForWidth());
        btnDeleteBelow->setSizePolicy(sizePolicy4);

        buttonLayoutBelow->addWidget(btnDeleteBelow, 2, 1, 1, 1);

        btnCutBelow = new QToolButton(ListControlRollup);
        btnCutBelow->setObjectName("btnCutBelow");
        btnCutBelow->setEnabled(false);
        sizePolicy4.setHeightForWidth(btnCutBelow->sizePolicy().hasHeightForWidth());
        btnCutBelow->setSizePolicy(sizePolicy4);

        buttonLayoutBelow->addWidget(btnCutBelow, 3, 0, 1, 1);

        btnPasteBelow = new QToolButton(ListControlRollup);
        btnPasteBelow->setObjectName("btnPasteBelow");
        btnPasteBelow->setEnabled(false);
        sizePolicy4.setHeightForWidth(btnPasteBelow->sizePolicy().hasHeightForWidth());
        btnPasteBelow->setSizePolicy(sizePolicy4);

        buttonLayoutBelow->addWidget(btnPasteBelow, 3, 1, 1, 1);

        btnAppendBelow = new QToolButton(ListControlRollup);
        btnAppendBelow->setObjectName("btnAppendBelow");
        sizePolicy4.setHeightForWidth(btnAppendBelow->sizePolicy().hasHeightForWidth());
        btnAppendBelow->setSizePolicy(sizePolicy4);

        buttonLayoutBelow->addWidget(btnAppendBelow, 1, 0, 1, 1);

        btnAssignBelow = new QToolButton(ListControlRollup);
        btnAssignBelow->setObjectName("btnAssignBelow");
        btnAssignBelow->setEnabled(false);
        sizePolicy4.setHeightForWidth(btnAssignBelow->sizePolicy().hasHeightForWidth());
        btnAssignBelow->setSizePolicy(sizePolicy4);

        buttonLayoutBelow->addWidget(btnAssignBelow, 1, 1, 1, 1);


        gridLayout->addLayout(buttonLayoutBelow, 4, 0, 1, 1);

        groupBoxIndex = new QGroupBox(ListControlRollup);
        groupBoxIndex->setObjectName("groupBoxIndex");
        groupBoxIndex->setFlat(true);
        gridLayout_3 = new QGridLayout(groupBoxIndex);
        gridLayout_3->setObjectName("gridLayout_3");
        gridLayout_3->setContentsMargins(3, 3, 3, 3);
        index = new MaxSDK::QmaxSpinBox(groupBoxIndex);
        index->setObjectName("index");

        gridLayout_3->addWidget(index, 0, 1, 1, 1);

        lineEditByName = new QLineEdit(groupBoxIndex);
        lineEditByName->setObjectName("lineEditByName");

        gridLayout_3->addWidget(lineEditByName, 1, 1, 1, 1);

        labelIndex = new QLabel(groupBoxIndex);
        labelIndex->setObjectName("labelIndex");
        labelIndex->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_3->addWidget(labelIndex, 0, 0, 1, 1);

        labelByName = new QLabel(groupBoxIndex);
        labelByName->setObjectName("labelByName");
        labelByName->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_3->addWidget(labelByName, 1, 0, 1, 1);

        gridLayout_3->setColumnStretch(0, 1);
        gridLayout_3->setColumnStretch(1, 2);

        gridLayout->addWidget(groupBoxIndex, 9, 0, 1, 1);


        retranslateUi(ListControlRollup);

        QMetaObject::connectSlotsByName(ListControlRollup);
    } // setupUi

    void retranslateUi(QWidget *ListControlRollup)
    {
        ListControlRollup->setWindowTitle(QCoreApplication::translate("ListControlRollup", "Form", nullptr));
        groupBoxWeight->setTitle(QCoreApplication::translate("ListControlRollup", "Weight", nullptr));
        average->setText(QCoreApplication::translate("ListControlRollup", "Normalize Weights", nullptr));
        groupBoxDependency->setTitle(QCoreApplication::translate("ListControlRollup", "List dependency mode", nullptr));
        radioFan->setText(QCoreApplication::translate("ListControlRollup", "Fan", nullptr));
        radioChain->setText(QCoreApplication::translate("ListControlRollup", "Chain", nullptr));
        groupBoxWeightMethod->setTitle(QCoreApplication::translate("ListControlRollup", "Weighting Method", nullptr));
        radioAgainstIdentity->setText(QCoreApplication::translate("ListControlRollup", "Weight against identity", nullptr));
        radioLerpPrevious->setText(QCoreApplication::translate("ListControlRollup", "LERP from previous", nullptr));
        radioWeightMode->setText(QCoreApplication::translate("ListControlRollup", "Weight", nullptr));
        radioIndexMode->setText(QCoreApplication::translate("ListControlRollup", "Index", nullptr));
        btnSetActiveBelow->setText(QCoreApplication::translate("ListControlRollup", "Set Active", nullptr));
        btnDeleteBelow->setText(QCoreApplication::translate("ListControlRollup", "Delete", nullptr));
        btnCutBelow->setText(QCoreApplication::translate("ListControlRollup", "Cut", nullptr));
        btnPasteBelow->setText(QCoreApplication::translate("ListControlRollup", "Paste", nullptr));
        btnAppendBelow->setText(QCoreApplication::translate("ListControlRollup", "Append", nullptr));
        btnAssignBelow->setText(QCoreApplication::translate("ListControlRollup", "Assign", nullptr));
        groupBoxIndex->setTitle(QCoreApplication::translate("ListControlRollup", "Index", nullptr));
        labelIndex->setText(QCoreApplication::translate("ListControlRollup", "Index", nullptr));
        labelByName->setText(QCoreApplication::translate("ListControlRollup", "By Name", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ListControlRollup: public Ui_ListControlRollup {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LISTCONTROL_H
