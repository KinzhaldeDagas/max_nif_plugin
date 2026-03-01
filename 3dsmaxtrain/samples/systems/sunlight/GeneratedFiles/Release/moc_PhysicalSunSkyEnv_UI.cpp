/****************************************************************************
** Meta object code from reading C++ file 'PhysicalSunSkyEnv_UI.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../PhysicalSunSkyEnv_UI.h"
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
#error "The header file 'PhysicalSunSkyEnv_UI.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSPhysicalSunSkyEnvSCOPEMainPanelWidgetENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSPhysicalSunSkyEnvSCOPEMainPanelWidgetENDCLASS = QtMocHelpers::stringData(
    "PhysicalSunSkyEnv::MainPanelWidget",
    "create_sun_positioner_button_clicked",
    ""
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSPhysicalSunSkyEnvSCOPEMainPanelWidgetENDCLASS_t {
    uint offsetsAndSizes[6];
    char stringdata0[35];
    char stringdata1[37];
    char stringdata2[1];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSPhysicalSunSkyEnvSCOPEMainPanelWidgetENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSPhysicalSunSkyEnvSCOPEMainPanelWidgetENDCLASS_t qt_meta_stringdata_CLASSPhysicalSunSkyEnvSCOPEMainPanelWidgetENDCLASS = {
    {
        QT_MOC_LITERAL(0, 34),  // "PhysicalSunSkyEnv::MainPanelW..."
        QT_MOC_LITERAL(35, 36),  // "create_sun_positioner_button_..."
        QT_MOC_LITERAL(72, 0)   // ""
    },
    "PhysicalSunSkyEnv::MainPanelWidget",
    "create_sun_positioner_button_clicked",
    ""
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSPhysicalSunSkyEnvSCOPEMainPanelWidgetENDCLASS[] = {

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
       1,    0,   20,    2, 0x09,    1 /* Protected */,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject PhysicalSunSkyEnv::MainPanelWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<MaxSDK::QMaxParamBlockWidget::staticMetaObject>(),
    qt_meta_stringdata_CLASSPhysicalSunSkyEnvSCOPEMainPanelWidgetENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSPhysicalSunSkyEnvSCOPEMainPanelWidgetENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSPhysicalSunSkyEnvSCOPEMainPanelWidgetENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<MainPanelWidget, std::true_type>,
        // method 'create_sun_positioner_button_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void PhysicalSunSkyEnv::MainPanelWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        auto *_t = static_cast<MainPanelWidget *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->create_sun_positioner_button_clicked(); break;
        default: ;
        }
    }
    (void)_a;
}

const QMetaObject *PhysicalSunSkyEnv::MainPanelWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PhysicalSunSkyEnv::MainPanelWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSPhysicalSunSkyEnvSCOPEMainPanelWidgetENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return MaxSDK::QMaxParamBlockWidget::qt_metacast(_clname);
}

int PhysicalSunSkyEnv::MainPanelWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
QT_WARNING_POP
