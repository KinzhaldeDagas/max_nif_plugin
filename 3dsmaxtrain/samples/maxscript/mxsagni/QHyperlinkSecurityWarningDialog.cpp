//**************************************************************************/
// Copyright (c) 2021 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#include "QHyperlinkSecurityWarningDialog.h"

#include <QtWidgets/QStyle>
#include <QtCore/QRegularExpression>

#include <helpsys.h>
#include <winutil.h>

QHyperlinkSecurityWarningDialog::QHyperlinkSecurityWarningDialog(QWidget* parent, const TSTR& hyperlink)
	: QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint)
{
	ui.setupUi(this);
	InitializeUI(hyperlink);
	setAttribute(Qt::WA_DeleteOnClose);
}

bool QHyperlinkSecurityWarningDialog::nativeEvent(const QByteArray& /*eventType*/, void* message, qintptr* result)
{
	MSG* msg = (MSG*)(message);
	if (msg->message == WM_HELP)
	{
		MaxSDK::IHelpSystem::GetInstance()->ShowProductHelpForTopic(idh_scene_embedded_hyperlink);
		*result = TRUE;
		return true;
	}

	return false;
}

void QHyperlinkSecurityWarningDialog::on_learnMorePushButton_clicked(bool /*checked*/)
{
	MaxSDK::IHelpSystem::GetInstance()->ShowProductHelpForTopic(idh_scene_embedded_hyperlink);
}

void QHyperlinkSecurityWarningDialog::on_openPushButton_clicked(bool /*checked*/)
{
	accept();
}

void QHyperlinkSecurityWarningDialog::on_doNotOpenPushButton_clicked(bool /*checked*/)
{
	reject();
}

void QHyperlinkSecurityWarningDialog::InitializeUI(const TSTR& hyperlink)
{
	// The margin/spacing/width/height values below match the ones used by the QMessageBox class
	layout()->setContentsMargins(MaxSDK::UIScaled(20), MaxSDK::UIScaled(20), MaxSDK::UIScaled(20), MaxSDK::UIScaled(20));
	layout()->setSpacing(MaxSDK::UIScaled(3));

	ui.warningIconLabel->setPixmap(style()->standardIcon(QStyle::SP_MessageBoxWarning, 0, this).pixmap(MaxSDK::UIScaled(32), MaxSDK::UIScaled(32)));

	ui.indentSpacer->changeSize(MaxSDK::UIScaled(7), MaxSDK::UIScaled(1), QSizePolicy::Fixed, QSizePolicy::Fixed);

	ui.buttonBoxLayout->setContentsMargins(0, MaxSDK::UIScaled(9), 0, 0);
	ui.buttonBoxLayout->setSpacing(MaxSDK::UIScaled(3));

	QString hyperlinkToDisplay = hyperlink;

	// Replace special characters with their escape sequences
	hyperlinkToDisplay.replace('\n', "\\n");
	hyperlinkToDisplay.replace('\r', "\\r");
	hyperlinkToDisplay.replace('\t', "\\t");

	// Add a newline every 175 characters to limit the horizontal size of the dialog
	hyperlinkToDisplay.replace(QRegularExpression("(.{175})"), "\\1\n");

	QString msg = QString(tr("A scene embedded script is trying to open:\n\n%1\n\n"
		"Do you trust this website? Only open websites you trust.")).arg(hyperlinkToDisplay);

	ui.messageLabel->setText(msg);
}
