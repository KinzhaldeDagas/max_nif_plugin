//**************************************************************************/
// Copyright (c) 2021 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#pragma once

#include <ui_QHyperlinkSecurityWarningDialog.h>

#include <strclass.h>

class QHyperlinkSecurityWarningDialog : public QDialog
{
	Q_OBJECT

public:

	QHyperlinkSecurityWarningDialog(QWidget* parent, const TSTR& hyperlink);

protected:

	virtual bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override;

private Q_SLOTS:

	void on_learnMorePushButton_clicked(bool checked);
	void on_openPushButton_clicked(bool checked);
	void on_doNotOpenPushButton_clicked(bool checked);

private:

	void InitializeUI(const TSTR& hyperlink);

	Ui_hyperlinkSecurityWarningDialog ui;
};
