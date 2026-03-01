/****************************************************************************
** Meta object code from reading C++ file 'listControlUI.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../listControlUI.h"
#include <QtGui/qtextcursor.h>
#include <QtGui/qscreen.h>
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'listControlUI.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.5.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSListControlTableViewENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSListControlTableViewENDCLASS = QtMocHelpers::stringData(
    "ListControlTableView"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSListControlTableViewENDCLASS_t {
    uint offsetsAndSizes[2];
    char stringdata0[21];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSListControlTableViewENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSListControlTableViewENDCLASS_t qt_meta_stringdata_CLASSListControlTableViewENDCLASS = {
    {
        QT_MOC_LITERAL(0, 20)   // "ListControlTableView"
    },
    "ListControlTableView"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSListControlTableViewENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

Q_CONSTINIT const QMetaObject ListControlTableView::staticMetaObject = { {
    QMetaObject::SuperData::link<QTableView::staticMetaObject>(),
    qt_meta_stringdata_CLASSListControlTableViewENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSListControlTableViewENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSListControlTableViewENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<ListControlTableView, std::true_type>
    >,
    nullptr
} };

void ListControlTableView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    (void)_o;
    (void)_id;
    (void)_c;
    (void)_a;
}

const QMetaObject *ListControlTableView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ListControlTableView::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSListControlTableViewENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QTableView::qt_metacast(_clname);
}

int ListControlTableView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTableView::qt_metacall(_c, _id, _a);
    return _id;
}
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSListControlRollupENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSListControlRollupENDCLASS = QtMocHelpers::stringData(
    "ListControlRollup",
    "AverageStateChanged",
    "",
    "state",
    "RadioFanClicked",
    "checked",
    "RadioChainClicked",
    "RadioAgainstIdentityClicked",
    "RadioLerpPreviousClicked",
    "RadioWeightModeClicked",
    "RadioIndexModeClicked",
    "IndexSpinnerValueChanged",
    "index",
    "TableDoubleClicked",
    "QModelIndex",
    "TableRowChanged",
    "current",
    "previous",
    "ButtonSetActiveClicked",
    "ButtonCutClicked",
    "ButtonPasteClicked",
    "ButtonDeleteClicked",
    "ButtonAppendClicked",
    "ButtonAssignClicked",
    "ByNameIndexEditingFinished",
    "TableContextMenu",
    "pos"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSListControlRollupENDCLASS_t {
    uint offsetsAndSizes[54];
    char stringdata0[18];
    char stringdata1[20];
    char stringdata2[1];
    char stringdata3[6];
    char stringdata4[16];
    char stringdata5[8];
    char stringdata6[18];
    char stringdata7[28];
    char stringdata8[25];
    char stringdata9[23];
    char stringdata10[22];
    char stringdata11[25];
    char stringdata12[6];
    char stringdata13[19];
    char stringdata14[12];
    char stringdata15[16];
    char stringdata16[8];
    char stringdata17[9];
    char stringdata18[23];
    char stringdata19[17];
    char stringdata20[19];
    char stringdata21[20];
    char stringdata22[20];
    char stringdata23[20];
    char stringdata24[27];
    char stringdata25[17];
    char stringdata26[4];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSListControlRollupENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSListControlRollupENDCLASS_t qt_meta_stringdata_CLASSListControlRollupENDCLASS = {
    {
        QT_MOC_LITERAL(0, 17),  // "ListControlRollup"
        QT_MOC_LITERAL(18, 19),  // "AverageStateChanged"
        QT_MOC_LITERAL(38, 0),  // ""
        QT_MOC_LITERAL(39, 5),  // "state"
        QT_MOC_LITERAL(45, 15),  // "RadioFanClicked"
        QT_MOC_LITERAL(61, 7),  // "checked"
        QT_MOC_LITERAL(69, 17),  // "RadioChainClicked"
        QT_MOC_LITERAL(87, 27),  // "RadioAgainstIdentityClicked"
        QT_MOC_LITERAL(115, 24),  // "RadioLerpPreviousClicked"
        QT_MOC_LITERAL(140, 22),  // "RadioWeightModeClicked"
        QT_MOC_LITERAL(163, 21),  // "RadioIndexModeClicked"
        QT_MOC_LITERAL(185, 24),  // "IndexSpinnerValueChanged"
        QT_MOC_LITERAL(210, 5),  // "index"
        QT_MOC_LITERAL(216, 18),  // "TableDoubleClicked"
        QT_MOC_LITERAL(235, 11),  // "QModelIndex"
        QT_MOC_LITERAL(247, 15),  // "TableRowChanged"
        QT_MOC_LITERAL(263, 7),  // "current"
        QT_MOC_LITERAL(271, 8),  // "previous"
        QT_MOC_LITERAL(280, 22),  // "ButtonSetActiveClicked"
        QT_MOC_LITERAL(303, 16),  // "ButtonCutClicked"
        QT_MOC_LITERAL(320, 18),  // "ButtonPasteClicked"
        QT_MOC_LITERAL(339, 19),  // "ButtonDeleteClicked"
        QT_MOC_LITERAL(359, 19),  // "ButtonAppendClicked"
        QT_MOC_LITERAL(379, 19),  // "ButtonAssignClicked"
        QT_MOC_LITERAL(399, 26),  // "ByNameIndexEditingFinished"
        QT_MOC_LITERAL(426, 16),  // "TableContextMenu"
        QT_MOC_LITERAL(443, 3)   // "pos"
    },
    "ListControlRollup",
    "AverageStateChanged",
    "",
    "state",
    "RadioFanClicked",
    "checked",
    "RadioChainClicked",
    "RadioAgainstIdentityClicked",
    "RadioLerpPreviousClicked",
    "RadioWeightModeClicked",
    "RadioIndexModeClicked",
    "IndexSpinnerValueChanged",
    "index",
    "TableDoubleClicked",
    "QModelIndex",
    "TableRowChanged",
    "current",
    "previous",
    "ButtonSetActiveClicked",
    "ButtonCutClicked",
    "ButtonPasteClicked",
    "ButtonDeleteClicked",
    "ButtonAppendClicked",
    "ButtonAssignClicked",
    "ByNameIndexEditingFinished",
    "TableContextMenu",
    "pos"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSListControlRollupENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
      18,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,  122,    2, 0x09,    1 /* Protected */,
       4,    1,  125,    2, 0x09,    3 /* Protected */,
       6,    1,  128,    2, 0x09,    5 /* Protected */,
       7,    1,  131,    2, 0x09,    7 /* Protected */,
       8,    1,  134,    2, 0x09,    9 /* Protected */,
       9,    1,  137,    2, 0x09,   11 /* Protected */,
      10,    1,  140,    2, 0x09,   13 /* Protected */,
      11,    1,  143,    2, 0x09,   15 /* Protected */,
      13,    1,  146,    2, 0x09,   17 /* Protected */,
      15,    2,  149,    2, 0x09,   19 /* Protected */,
      18,    1,  154,    2, 0x09,   22 /* Protected */,
      19,    1,  157,    2, 0x09,   24 /* Protected */,
      20,    1,  160,    2, 0x09,   26 /* Protected */,
      21,    1,  163,    2, 0x09,   28 /* Protected */,
      22,    1,  166,    2, 0x09,   30 /* Protected */,
      23,    1,  169,    2, 0x09,   32 /* Protected */,
      24,    0,  172,    2, 0x09,   34 /* Protected */,
      25,    1,  173,    2, 0x09,   35 /* Protected */,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Bool,    5,
    QMetaType::Void, QMetaType::Bool,    5,
    QMetaType::Void, QMetaType::Bool,    5,
    QMetaType::Void, QMetaType::Bool,    5,
    QMetaType::Void, QMetaType::Bool,    5,
    QMetaType::Void, QMetaType::Bool,    5,
    QMetaType::Void, QMetaType::Int,   12,
    QMetaType::Void, 0x80000000 | 14,   12,
    QMetaType::Void, 0x80000000 | 14, 0x80000000 | 14,   16,   17,
    QMetaType::Void, QMetaType::Bool,    5,
    QMetaType::Void, QMetaType::Bool,    5,
    QMetaType::Void, QMetaType::Bool,    5,
    QMetaType::Void, QMetaType::Bool,    5,
    QMetaType::Void, QMetaType::Bool,    5,
    QMetaType::Void, QMetaType::Bool,    5,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QPoint,   26,

       0        // eod
};

Q_CONSTINIT const QMetaObject ListControlRollup::staticMetaObject = { {
    QMetaObject::SuperData::link<MaxSDK::QMaxParamBlockWidget::staticMetaObject>(),
    qt_meta_stringdata_CLASSListControlRollupENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSListControlRollupENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSListControlRollupENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<ListControlRollup, std::true_type>,
        // method 'AverageStateChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'RadioFanClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'RadioChainClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'RadioAgainstIdentityClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'RadioLerpPreviousClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'RadioWeightModeClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'RadioIndexModeClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'IndexSpinnerValueChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'TableDoubleClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QModelIndex &, std::false_type>,
        // method 'TableRowChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QModelIndex &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QModelIndex &, std::false_type>,
        // method 'ButtonSetActiveClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'ButtonCutClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'ButtonPasteClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'ButtonDeleteClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'ButtonAppendClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'ButtonAssignClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'ByNameIndexEditingFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'TableContextMenu'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QPoint &, std::false_type>
    >,
    nullptr
} };

void ListControlRollup::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        auto *_t = static_cast<ListControlRollup *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->AverageStateChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 1: _t->RadioFanClicked((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 2: _t->RadioChainClicked((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 3: _t->RadioAgainstIdentityClicked((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 4: _t->RadioLerpPreviousClicked((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 5: _t->RadioWeightModeClicked((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 6: _t->RadioIndexModeClicked((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 7: _t->IndexSpinnerValueChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 8: _t->TableDoubleClicked((*reinterpret_cast< std::add_pointer_t<QModelIndex>>(_a[1]))); break;
        case 9: _t->TableRowChanged((*reinterpret_cast< std::add_pointer_t<QModelIndex>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QModelIndex>>(_a[2]))); break;
        case 10: _t->ButtonSetActiveClicked((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 11: _t->ButtonCutClicked((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 12: _t->ButtonPasteClicked((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 13: _t->ButtonDeleteClicked((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 14: _t->ButtonAppendClicked((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 15: _t->ButtonAssignClicked((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 16: _t->ByNameIndexEditingFinished(); break;
        case 17: _t->TableContextMenu((*reinterpret_cast< std::add_pointer_t<QPoint>>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject *ListControlRollup::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ListControlRollup::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSListControlRollupENDCLASS.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "ReferenceMaker"))
        return static_cast< ReferenceMaker*>(this);
    if (!strcmp(_clname, "TimeChangeCallback"))
        return static_cast< TimeChangeCallback*>(this);
    return MaxSDK::QMaxParamBlockWidget::qt_metacast(_clname);
}

int ListControlRollup::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = MaxSDK::QMaxParamBlockWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 18)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 18;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 18)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 18;
    }
    return _id;
}
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSListControlTableModelENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSListControlTableModelENDCLASS = QtMocHelpers::stringData(
    "ListControlTableModel"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSListControlTableModelENDCLASS_t {
    uint offsetsAndSizes[2];
    char stringdata0[22];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSListControlTableModelENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSListControlTableModelENDCLASS_t qt_meta_stringdata_CLASSListControlTableModelENDCLASS = {
    {
        QT_MOC_LITERAL(0, 21)   // "ListControlTableModel"
    },
    "ListControlTableModel"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSListControlTableModelENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

Q_CONSTINIT const QMetaObject ListControlTableModel::staticMetaObject = { {
    QMetaObject::SuperData::link<QAbstractTableModel::staticMetaObject>(),
    qt_meta_stringdata_CLASSListControlTableModelENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSListControlTableModelENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSListControlTableModelENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<ListControlTableModel, std::true_type>
    >,
    nullptr
} };

void ListControlTableModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    (void)_o;
    (void)_id;
    (void)_c;
    (void)_a;
}

const QMetaObject *ListControlTableModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ListControlTableModel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSListControlTableModelENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QAbstractTableModel::qt_metacast(_clname);
}

int ListControlTableModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractTableModel::qt_metacall(_c, _id, _a);
    return _id;
}
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSListTableEntryEditDelegateENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSListTableEntryEditDelegateENDCLASS = QtMocHelpers::stringData(
    "ListTableEntryEditDelegate",
    "WeightSpinnerValueChanged",
    "",
    "value"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSListTableEntryEditDelegateENDCLASS_t {
    uint offsetsAndSizes[8];
    char stringdata0[27];
    char stringdata1[26];
    char stringdata2[1];
    char stringdata3[6];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSListTableEntryEditDelegateENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSListTableEntryEditDelegateENDCLASS_t qt_meta_stringdata_CLASSListTableEntryEditDelegateENDCLASS = {
    {
        QT_MOC_LITERAL(0, 26),  // "ListTableEntryEditDelegate"
        QT_MOC_LITERAL(27, 25),  // "WeightSpinnerValueChanged"
        QT_MOC_LITERAL(53, 0),  // ""
        QT_MOC_LITERAL(54, 5)   // "value"
    },
    "ListTableEntryEditDelegate",
    "WeightSpinnerValueChanged",
    "",
    "value"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSListTableEntryEditDelegateENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   20,    2, 0x10a,    1 /* Public | MethodIsConst  */,

 // slots: parameters
    QMetaType::Void, QMetaType::Double,    3,

       0        // eod
};

Q_CONSTINIT const QMetaObject ListTableEntryEditDelegate::staticMetaObject = { {
    QMetaObject::SuperData::link<QStyledItemDelegate::staticMetaObject>(),
    qt_meta_stringdata_CLASSListTableEntryEditDelegateENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSListTableEntryEditDelegateENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSListTableEntryEditDelegateENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<ListTableEntryEditDelegate, std::true_type>,
        // method 'WeightSpinnerValueChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>
    >,
    nullptr
} };

void ListTableEntryEditDelegate::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        auto *_t = static_cast<ListTableEntryEditDelegate *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->WeightSpinnerValueChanged((*reinterpret_cast< std::add_pointer_t<double>>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject *ListTableEntryEditDelegate::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ListTableEntryEditDelegate::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSListTableEntryEditDelegateENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QStyledItemDelegate::qt_metacast(_clname);
}

int ListTableEntryEditDelegate::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QStyledItemDelegate::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 1;
    }
    return _id;
}
QT_WARNING_POP
