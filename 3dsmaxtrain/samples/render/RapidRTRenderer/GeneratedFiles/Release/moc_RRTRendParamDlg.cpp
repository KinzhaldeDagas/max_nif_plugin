/****************************************************************************
** Meta object code from reading C++ file 'RRTRendParamDlg.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../RRTRendParamDlg.h"
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
#error "The header file 'RRTRendParamDlg.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSMaxSCOPERapidRTTranslatorSCOPERenderParamsQtDialogENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSMaxSCOPERapidRTTranslatorSCOPERenderParamsQtDialogENDCLASS = QtMocHelpers::stringData(
    "Max::RapidRTTranslator::RenderParamsQtDialog",
    "render_method_combo_changed",
    "",
    "new_index"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSMaxSCOPERapidRTTranslatorSCOPERenderParamsQtDialogENDCLASS_t {
    uint offsetsAndSizes[8];
    char stringdata0[45];
    char stringdata1[28];
    char stringdata2[1];
    char stringdata3[10];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSMaxSCOPERapidRTTranslatorSCOPERenderParamsQtDialogENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSMaxSCOPERapidRTTranslatorSCOPERenderParamsQtDialogENDCLASS_t qt_meta_stringdata_CLASSMaxSCOPERapidRTTranslatorSCOPERenderParamsQtDialogENDCLASS = {
    {
        QT_MOC_LITERAL(0, 44),  // "Max::RapidRTTranslator::Rende..."
        QT_MOC_LITERAL(45, 27),  // "render_method_combo_changed"
        QT_MOC_LITERAL(73, 0),  // ""
        QT_MOC_LITERAL(74, 9)   // "new_index"
    },
    "Max::RapidRTTranslator::RenderParamsQtDialog",
    "render_method_combo_changed",
    "",
    "new_index"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSMaxSCOPERapidRTTranslatorSCOPERenderParamsQtDialogENDCLASS[] = {

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
       1,    1,   20,    2, 0x08,    1 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    3,

       0        // eod
};

Q_CONSTINIT const QMetaObject Max::RapidRTTranslator::RenderParamsQtDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<MaxSDK::QMaxParamBlockWidget::staticMetaObject>(),
    qt_meta_stringdata_CLASSMaxSCOPERapidRTTranslatorSCOPERenderParamsQtDialogENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSMaxSCOPERapidRTTranslatorSCOPERenderParamsQtDialogENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSMaxSCOPERapidRTTranslatorSCOPERenderParamsQtDialogENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<RenderParamsQtDialog, std::true_type>,
        // method 'render_method_combo_changed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>
    >,
    nullptr
} };

void Max::RapidRTTranslator::RenderParamsQtDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        auto *_t = static_cast<RenderParamsQtDialog *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->render_method_combo_changed((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject *Max::RapidRTTranslator::RenderParamsQtDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Max::RapidRTTranslator::RenderParamsQtDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSMaxSCOPERapidRTTranslatorSCOPERenderParamsQtDialogENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return MaxSDK::QMaxParamBlockWidget::qt_metacast(_clname);
}

int Max::RapidRTTranslator::RenderParamsQtDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = MaxSDK::QMaxParamBlockWidget::qt_metacall(_c, _id, _a);
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
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSMaxSCOPERapidRTTranslatorSCOPEImageQualityQtDialogENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSMaxSCOPERapidRTTranslatorSCOPEImageQualityQtDialogENDCLASS = QtMocHelpers::stringData(
    "Max::RapidRTTranslator::ImageQualityQtDialog"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSMaxSCOPERapidRTTranslatorSCOPEImageQualityQtDialogENDCLASS_t {
    uint offsetsAndSizes[2];
    char stringdata0[45];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSMaxSCOPERapidRTTranslatorSCOPEImageQualityQtDialogENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSMaxSCOPERapidRTTranslatorSCOPEImageQualityQtDialogENDCLASS_t qt_meta_stringdata_CLASSMaxSCOPERapidRTTranslatorSCOPEImageQualityQtDialogENDCLASS = {
    {
        QT_MOC_LITERAL(0, 44)   // "Max::RapidRTTranslator::Image..."
    },
    "Max::RapidRTTranslator::ImageQualityQtDialog"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSMaxSCOPERapidRTTranslatorSCOPEImageQualityQtDialogENDCLASS[] = {

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

Q_CONSTINIT const QMetaObject Max::RapidRTTranslator::ImageQualityQtDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<MaxSDK::QMaxParamBlockWidget::staticMetaObject>(),
    qt_meta_stringdata_CLASSMaxSCOPERapidRTTranslatorSCOPEImageQualityQtDialogENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSMaxSCOPERapidRTTranslatorSCOPEImageQualityQtDialogENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSMaxSCOPERapidRTTranslatorSCOPEImageQualityQtDialogENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<ImageQualityQtDialog, std::true_type>
    >,
    nullptr
} };

void Max::RapidRTTranslator::ImageQualityQtDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    (void)_o;
    (void)_id;
    (void)_c;
    (void)_a;
}

const QMetaObject *Max::RapidRTTranslator::ImageQualityQtDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Max::RapidRTTranslator::ImageQualityQtDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSMaxSCOPERapidRTTranslatorSCOPEImageQualityQtDialogENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return MaxSDK::QMaxParamBlockWidget::qt_metacast(_clname);
}

int Max::RapidRTTranslator::ImageQualityQtDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = MaxSDK::QMaxParamBlockWidget::qt_metacall(_c, _id, _a);
    return _id;
}
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSMaxSCOPERapidRTTranslatorSCOPEAdvancedParamsQtDialogENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSMaxSCOPERapidRTTranslatorSCOPEAdvancedParamsQtDialogENDCLASS = QtMocHelpers::stringData(
    "Max::RapidRTTranslator::AdvancedParamsQtDialog"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSMaxSCOPERapidRTTranslatorSCOPEAdvancedParamsQtDialogENDCLASS_t {
    uint offsetsAndSizes[2];
    char stringdata0[47];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSMaxSCOPERapidRTTranslatorSCOPEAdvancedParamsQtDialogENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSMaxSCOPERapidRTTranslatorSCOPEAdvancedParamsQtDialogENDCLASS_t qt_meta_stringdata_CLASSMaxSCOPERapidRTTranslatorSCOPEAdvancedParamsQtDialogENDCLASS = {
    {
        QT_MOC_LITERAL(0, 46)   // "Max::RapidRTTranslator::Advan..."
    },
    "Max::RapidRTTranslator::AdvancedParamsQtDialog"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSMaxSCOPERapidRTTranslatorSCOPEAdvancedParamsQtDialogENDCLASS[] = {

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

Q_CONSTINIT const QMetaObject Max::RapidRTTranslator::AdvancedParamsQtDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<MaxSDK::QMaxParamBlockWidget::staticMetaObject>(),
    qt_meta_stringdata_CLASSMaxSCOPERapidRTTranslatorSCOPEAdvancedParamsQtDialogENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSMaxSCOPERapidRTTranslatorSCOPEAdvancedParamsQtDialogENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSMaxSCOPERapidRTTranslatorSCOPEAdvancedParamsQtDialogENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<AdvancedParamsQtDialog, std::true_type>
    >,
    nullptr
} };

void Max::RapidRTTranslator::AdvancedParamsQtDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    (void)_o;
    (void)_id;
    (void)_c;
    (void)_a;
}

const QMetaObject *Max::RapidRTTranslator::AdvancedParamsQtDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Max::RapidRTTranslator::AdvancedParamsQtDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSMaxSCOPERapidRTTranslatorSCOPEAdvancedParamsQtDialogENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return MaxSDK::QMaxParamBlockWidget::qt_metacast(_clname);
}

int Max::RapidRTTranslator::AdvancedParamsQtDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = MaxSDK::QMaxParamBlockWidget::qt_metacall(_c, _id, _a);
    return _id;
}
QT_WARNING_POP
