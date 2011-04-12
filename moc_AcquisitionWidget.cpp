/****************************************************************************
** Meta object code from reading C++ file 'AcquisitionWidget.h'
**
** Created: Tue Apr 12 11:25:50 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "AcquisitionWidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'AcquisitionWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_AcquisitionWidget[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      27,   19,   18,   18, 0x05,

 // slots: signature, parameters, type, tag, flags
      48,   18,   18,   18, 0x0a,
      57,   18,   18,   18, 0x0a,
      66,   18,   18,   18, 0x0a,
      72,   18,   18,   18, 0x0a,
      95,   79,   18,   18, 0x0a,
     129,  116,   18,   18, 0x0a,
     162,  151,   18,   18, 0x0a,
     197,  181,   18,   18, 0x0a,
     229,  222,   18,   18, 0x0a,
     254,  247,   18,   18, 0x0a,
     292,  273,   18,   18, 0x0a,
     332,  326,   18,   18, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_AcquisitionWidget[] = {
    "AcquisitionWidget\0\0message\0"
    "showWarning(QString)\0update()\0remove()\0"
    "run()\0stop()\0positionQString\0"
    "getPosition(QString)\0imagesuccess\0"
    "getCameraStatus(bool)\0dacsuccess\0"
    "getDacStatus(bool)\0treatmentstatus\0"
    "getTreatmentStatus(bool)\0record\0"
    "getAcquiring(int)\0number\0getFilenumber(int)\0"
    "isopencamerawindow\0isopenCameraWindow(QVector<bool>)\0"
    "_path\0setDbPath(QString)\0"
};

const QMetaObject AcquisitionWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_AcquisitionWidget,
      qt_meta_data_AcquisitionWidget, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &AcquisitionWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *AcquisitionWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *AcquisitionWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_AcquisitionWidget))
        return static_cast<void*>(const_cast< AcquisitionWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int AcquisitionWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: showWarning((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: update(); break;
        case 2: remove(); break;
        case 3: run(); break;
        case 4: stop(); break;
        case 5: getPosition((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 6: getCameraStatus((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 7: getDacStatus((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 8: getTreatmentStatus((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 9: getAcquiring((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 10: getFilenumber((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 11: isopenCameraWindow((*reinterpret_cast< QVector<bool>(*)>(_a[1]))); break;
        case 12: setDbPath((*reinterpret_cast< QString(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 13;
    }
    return _id;
}

// SIGNAL 0
void AcquisitionWidget::showWarning(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
