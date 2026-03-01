//**************************************************************************/
// Copyright (c) 1998-2022 Autodesk, Inc.
// All rights reserved.
// 
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#pragma once

#include <maxapi.h>
#include <Qt/QMaxParamBlockWidget.h>
#include "ui_plugin_form.h"

namespace Ui
{
	class PluginRollup;
}

class QtPluginRollup : public MaxSDK::QMaxParamBlockWidget
{
	// This is a macro that connects this class to the Qt build system
	Q_OBJECT

public:
	explicit QtPluginRollup(QWidget* parent = nullptr);
	virtual ~QtPluginRollup(void);

	// Required by QMaxParamBlockWidget:
	void SetParamBlock(ReferenceMaker* /*owner*/, IParamBlock2* const /*param_block*/) override {};
	void UpdateUI(const TimeValue t) override
	{
		Interface* ip = GetCOREInterface();
		ip->RedrawViews(t, REDRAW_NORMAL);
	};
	void UpdateParameterUI(const TimeValue /*t*/, const ParamID /*param_id*/, const int /*tab_index*/) override {};

protected Q_SLOTS:
	void on_pushButton_clicked();

private:
	Ui::PluginRollup* ui;
};
