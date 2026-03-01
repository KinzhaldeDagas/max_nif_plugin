#pragma once

#include "BasicOps.h"


const Class_ID kVERTEX_WELD_CLASS_ID(0x709029e0, 0x2cfa07bd);

// QT Support

#include <Qt/QMaxParamBlockWidget.h>

namespace Ui {
	class VWeldRollup;
};

class VWeldMod;

class VWeldRollup : public MaxSDK::QMaxParamBlockWidget
{
	Q_OBJECT

public:
	explicit VWeldRollup(QWidget* parent = nullptr);
	virtual ~VWeldRollup();

	virtual void SetParamBlock(ReferenceMaker* owner, IParamBlock2* const param_block) override;
	virtual void UpdateUI(const TimeValue t) override {};
	virtual void UpdateParameterUI(const TimeValue t, const ParamID param_id, const int tab_index) override {};

	// Custom
	void		UpdateStatusString(QString string);

public Q_SLOTS:

private:
	Ui::VWeldRollup* m_UI;
	VWeldMod* mMod;
};