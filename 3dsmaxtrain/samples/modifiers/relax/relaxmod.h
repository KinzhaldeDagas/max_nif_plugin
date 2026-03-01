/**********************************************************************
 *<
	FILE: relaxmod.h

	DESCRIPTION:

	CREATED BY: Steve Anderson

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#ifndef __RELAX_MODS__H
#define __RELAX_MODS__H

#include "Max.h"
#include "iparamm2.h"
#include "relaxres.h"

extern ClassDesc2* GetRelaxModDesc();
extern TCHAR *GetString(int sid);
extern HINSTANCE hInstance;

#include <Qt/QMaxParamBlockWidget.h>

namespace Ui {
	class RelaxRollup;
};

class RelaxMod;

class RelaxRollup : public MaxSDK::QMaxParamBlockWidget
{
	Q_OBJECT

public:
	RelaxRollup(QWidget* parent = nullptr);
	virtual ~RelaxRollup();

	virtual void SetParamBlock(ReferenceMaker* owner, IParamBlock2* const param_block) override;
	virtual void UpdateUI(const TimeValue t) override;
	virtual void UpdateParameterUI(const TimeValue t, const ParamID param_id, const int tab_index) override;

public Q_SLOTS:


private:
	Ui::RelaxRollup* m_UI;
	RelaxMod* mMod;
};

#endif

