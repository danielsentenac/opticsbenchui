/****************************************************************************
** Meta object code from reading C++ file 'OpticsBenchUIMain.h'
**
** Created: Tue Apr 12 11:25:19 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "OpticsBenchUIMain.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'OpticsBenchUIMain.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_OpticsBenchUIMain[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
      14,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      38,   19,   18,   18, 0x05,
      77,   72,   18,   18, 0x05,

 // slots: signature, parameters, type, tag, flags
      96,   18,   18,   18, 0x0a,
     114,   18,   18,   18, 0x0a,
     142,  129,   18,   18, 0x0a,
     164,   18,   18,   18, 0x0a,
     182,   18,   18,   18, 0x0a,
     198,   18,   18,   18, 0x0a,
     222,   18,   18,   18, 0x0a,
     248,   18,   18,   18, 0x0a,
     275,   18,   18,   18, 0x0a,
     307,   18,   18,   18, 0x0a,
     327,   18,   18,   18, 0x0a,
     340,  334,   18,   18, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_OpticsBenchUIMain[] = {
    "OpticsBenchUIMain\0\0isopencamerawindow\0"
    "isopenCameraWindow(QVector<bool>)\0"
    "path\0setDbPath(QString)\0openacquisition()\0"
    "openanalysis()\0cameraNumber\0"
    "openCameraWindow(int)\0openMotorWindow()\0"
    "openDacWindow()\0showDacWarning(QString)\0"
    "showMotorWarning(QString)\0"
    "showCameraWarning(QString)\0"
    "showAcquisitionWarning(QString)\0"
    "showDocumentation()\0open()\0index\0"
    "closeTab(int)\0"
};

const QMetaObject OpticsBenchUIMain::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_OpticsBenchUIMain,
      qt_meta_data_OpticsBenchUIMain, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &OpticsBenchUIMain::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *OpticsBenchUIMain::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *OpticsBenchUIMain::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_OpticsBenchUIMain))
        return static_cast<void*>(const_cast< OpticsBenchUIMain*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int OpticsBenchUIMain::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: isopenCameraWindow((*reinterpret_cast< QVector<bool>(*)>(_a[1]))); break;
        case 1: setDbPath((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: openacquisition(); break;
        case 3: openanalysis(); break;
        case 4: openCameraWindow((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: openMotorWindow(); break;
        case 6: openDacWindow(); break;
        case 7: showDacWarning((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 8: showMotorWarning((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 9: showCameraWarning((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 10: showAcquisitionWarning((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 11: showDocumentation(); break;
        case 12: open(); break;
        case 13: closeTab((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 14;
    }
    return _id;
}

// SIGNAL 0
void OpticsBenchUIMain::isopenCameraWindow(QVector<bool> _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void OpticsBenchUIMain::setDbPath(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
