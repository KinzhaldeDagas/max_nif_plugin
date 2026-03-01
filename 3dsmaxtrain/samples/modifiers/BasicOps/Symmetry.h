#pragma once

#include "BasicOps.h"


const Class_ID kSYMMETRY_CLASS_ID(0x2d7e702b, 0xbe41a6e);

#define SYMMETRY_MXS_INTERFACE Interface_ID(0xDE21A34f, 0x8A411AA1)

// Version manangement
//version 100 is legacy symmetry
//version 200 added the new weld options
//version 300 fixes the edge length collapse MAXX-64434 and changes the weld to only weld matching pairs on the on weld MAXX-63866 
static constexpr int	OLD_SYMMETRY_VERSION = 100;
static constexpr int	NEW_SYMMETRY_VERSION = 200;
static constexpr int	SYMMETRY_VERSION_V300 = 300;


//everytime you up the version change it here 
static constexpr int	SYMMETRY_LATEST_VERSION = SYMMETRY_VERSION_V300;


// QT Support

#include <Qt/QMaxParamBlockWidget.h>

namespace Ui {
	class SymmetryRollup;
};

class SymmetryMod;

class SymmetryRollup : public MaxSDK::QMaxParamBlockWidget
{
	Q_OBJECT

public:
	explicit SymmetryRollup(QWidget* parent = nullptr);
	virtual ~SymmetryRollup();

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
	Ui::SymmetryRollup* m_UI;
	SymmetryMod* mMod;

	QAction*		mActionClear = nullptr;
	QAction*		mActionReset = nullptr;
};

class SliceEntry
{
public:
	SliceEntry(Point3 p, float o)
	{
		plane = p;
		offset = o;
	}

	Point3	plane;
	float	offset;
};

class MirrorEntry
{
public:
	MirrorEntry(Matrix3 m, int axis, Point3 p, float o)
	{
		Matrix3 itm = Inverse(m);
		Point3 scale(1, 1, 1);
		scale[axis] = -1.0f;
		itm.Scale(scale, TRUE);

		legacyTM = m;
		legacyInvTM = itm;

		tm = itm * m;
		plane = p;
		offset = o;
	}

	MirrorEntry(Matrix3 m1, Matrix3 m2, Point3 p, float o)
	{
		// Legacy not applicable for this constructor (radial)
		Matrix3 itm = Inverse(m2);
		tm = itm * m1;

		plane = p;
		offset = o;
	}

	Matrix3	tm;
	Point3	plane;
	float	offset;

	// Original matrix representation for legacy compatibility
	Matrix3 legacyTM;
	Matrix3 legacyInvTM;
};