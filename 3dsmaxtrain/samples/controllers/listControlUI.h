/**********************************************************************
 *<
	FILE: listControlUI.h

	DESCRIPTION:

	CREATED BY: Kelvin Zelt

	HISTORY: Created 11/11/2022

 *>	Copyright (c) 2022, All Rights Reserved.
 **********************************************************************/
#pragma once

#include "max.h"
#include "iparamm2.h"
#include <memory>

#include <Qt/QMaxParamBlockWidget.h>
#include <QAbstractTableModel>
#include <QTableView>
#include <QtWidgets/QStyledItemDelegate>

namespace Ui {
	class ListControlRollup;
};

class ListControl;
class ListControlTableModel;
namespace MaxSDK {
	class QmaxDoubleSpinBox;
}

enum ListTableColumns {
	eSetActiveCol,
	eNameCol,
	eWeightCol,
};

// This represents the table view for list controllers.
class ListControlTableView : public QTableView
{
	Q_OBJECT
public:
	explicit ListControlTableView(QWidget* parent = nullptr);

	//Override this to help it fit into the command panel rollout
	QSize minimumSizeHint() const override;
	QSize sizeHint() const override;
	void resizeEvent(QResizeEvent* event) override;
};

// Rollup for the list controllers
class ListControlRollup :
	public MaxSDK::QMaxParamBlockWidget, //Be a rollout
	public ReferenceMaker, //so that we can keep a reference to the owning controller
	public TimeChangeCallback //so that we can be notified when the time changes
{
	Q_OBJECT

public:
	ListControlRollup(bool inCommandPanel, IObjParam* ip);
	virtual ~ListControlRollup();

	void SetObjectParams(IObjParam* ip) {
		mIObjParams = ip;
	}
	IObjParam* GetObjectParams() const {
		return mIObjParams;
	}
	IParamMap2* GetParamMap() const {
		return mParamMap;
	}
	// Destroy the ParamMap2, called before the UI is destructed.
	void DestroyParamMap();

	void StartEdit(int startIndex, int prevIndex = -1) const;
	void EndEdit(int endIndex, int nextIndex = -1) const;

	//QMaxParamBlockWidget
	virtual void SetParamBlock(ReferenceMaker* owner, IParamBlock2* const param_block) override;
	virtual void UpdateUI(const TimeValue t) override; //Update entire UI
	virtual void UpdateParameterUI(const TimeValue t, const ParamID param_id, const int tab_index) override;

	//Update a specific part of the UI
	void UpdateUIPart(const TimeValue t, const ParamID param_id);

	// weak reference to the owning controller so rollout floater doesn't keep controller alive.
	BOOL IsRealDependency(ReferenceTarget* rtarg) override { return FALSE; }

	//ReferenceMaker
	virtual RefResult NotifyRefChanged(
		const Interval& changeInt,
		RefTargetHandle hTarget,
		PartID& partID,
		RefMessage message,
		BOOL propagate) override;
	int NumRefs() override;
	RefTargetHandle GetReference(int i) override;
private:
	virtual void SetReference(int i, RefTargetHandle rtarg) override;
public:
	
	//TimeChangeCallback
	virtual void TimeChanged(TimeValue t) override;

	// Put here because of const-ness on the edit delegate
	MaxSDK::QmaxDoubleSpinBox* weightSpinner = nullptr;

protected Q_SLOTS:
	void AverageStateChanged(int state);
	void RadioFanClicked(bool checked);
	void RadioChainClicked(bool checked);
	void RadioAgainstIdentityClicked(bool checked);
	void RadioLerpPreviousClicked(bool checked);
	void RadioWeightModeClicked(bool checked);
	void RadioIndexModeClicked(bool checked);
	void IndexSpinnerValueChanged(int index);
	void TableDoubleClicked(const QModelIndex& index);
	void TableRowChanged(const QModelIndex& current, const QModelIndex& previous);
	void ButtonSetActiveClicked(bool checked);
	void ButtonCutClicked(bool checked);
	void ButtonPasteClicked(bool checked);
	void ButtonDeleteClicked(bool checked);
	void ButtonAppendClicked(bool checked);
	void ButtonAssignClicked(bool checked);
	void ByNameIndexEditingFinished();
	void TableContextMenu(const QPoint& pos);

private:
	// Force entire model to be updated
	void InvalidateTableData() const;

	void UpdateTableButtons() const;

	// Helper method to avoid copy paste.
	int GetIndexFromSelectionModel() const;
	
	bool mInCommandPanel; //false = manually register to receive ref messages from owning controller
	Ui::ListControlRollup* m_UI;
	ListControl* cont;
	IObjParam* mIObjParams = nullptr;
	IParamMap2* mParamMap = nullptr; //Used to bind the spinners to the pblock entries
	std::unique_ptr<ListControlTableModel> tableModel;
};


// This represents the table view for list controllers.
class ListControlTableModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	explicit ListControlTableModel(ListControl* ownerListControl, QObject* parent = nullptr);

	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	Qt::ItemFlags flags(const QModelIndex& index) const override;
	bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	void InvalidateTableData();
	void InvalidateTableSize();
	
	// False = weight mode.
	// True = index mode.
	void SetMode(bool indexMode);
	bool GetMode() const {
		return mIndexMode;
	}

private:
	ListControl* cont = nullptr;

	// false = weight mode.
	// true = index mode.
	bool mIndexMode = false;
};

class ListTableEntryEditDelegate : public QStyledItemDelegate
{
	Q_OBJECT
public:
	ListTableEntryEditDelegate(ListControlRollup* rollout, ListControl* owner, QObject* parent = 0);
	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
	void destroyEditor(QWidget* editor, const QModelIndex& index) const override;
	void setEditorData(QWidget* editor, const QModelIndex& index) const override;
	void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
	void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

public Q_SLOTS:
	void WeightSpinnerValueChanged(double value) const;

private:
	//QTreeView* treeView; //view hosting this editDelegate
	ListControl* cont = nullptr;
	ListControlRollup* mRollout = nullptr;
};