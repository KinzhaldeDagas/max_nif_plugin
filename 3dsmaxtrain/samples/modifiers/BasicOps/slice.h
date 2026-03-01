#pragma once

#include "BasicOps.h"


const Class_ID SLICE_CLASS_ID(0x2e3741fe, 0x12702ab7);
// Version manangement
static constexpr int	OLD_SLICE_VERSION = 100;
static constexpr int	NEW_SLICE_VERSION = 200;

// QT Support

#include <Qt/QMaxParamBlockWidget.h>

namespace Ui {
	class SliceRollup;
};

class SliceMod;

class SliceRollup : public MaxSDK::QMaxParamBlockWidget
{
	Q_OBJECT

public:
	explicit SliceRollup(QWidget* parent = nullptr);
	virtual ~SliceRollup();

	virtual void SetParamBlock(ReferenceMaker* owner, IParamBlock2* const param_block) override;
	virtual void UpdateUI(const TimeValue t) override;
	virtual void UpdateParameterUI(const TimeValue t, const ParamID param_id, const int tab_index) override;

	void	EnableControls(const TimeValue t);
	void	ToggleAlignButton();
	void	UpdatePickButton();

public Q_SLOTS:
	void	AlignToFace();
	void	ResetTransform();
	void	AlignRightClick(const QPoint & pos);
	void	ObjectRightClick(const QPoint & pos);

private:
	Ui::SliceRollup* m_UI;
	SliceMod* mMod;

	QAction*		mActionClear = nullptr;
	QAction*		mActionReset = nullptr;
};

