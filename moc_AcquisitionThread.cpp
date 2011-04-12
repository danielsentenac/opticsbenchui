/****************************************************************************
** Meta object code from reading C++ file 'AcquisitionThread.h'
**
** Created: Tue Apr 12 11:25:53 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "AcquisitionThread.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'AcquisitionThread.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_AcquisitionThread[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: signature, parameters, type, tag, flags
      35,   19,   18,   18, 0x05,
      69,   56,   18,   18, 0x05,
     102,   91,   18,   18, 0x05,
     138,  121,   18,   18, 0x05,
     170,  163,   18,   18, 0x05,
     195,  188,   18,   18, 0x05,

 // slots: signature, parameters, type, tag, flags
     217,  214,   18,   18, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_AcquisitionThread[] = {
    "AcquisitionThread\0\0positionQString\0"
    "getPosition(QString)\0imagesuccess\0"
    "getCameraStatus(bool)\0dacsuccess\0"
    "getDacStatus(bool)\0treatmentsuccess\0"
    "getTreatmentStatus(bool)\0record\0"
    "getAcquiring(int)\0number\0getFilenumber(int)\0"
    ",,\0setImageFromCamera(uchar*,int,int)\0"
};

const QMetaObject AcquisitionThread::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_AcquisitionThread,
      qt_meta_data_AcquisitionThread, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &AcquisitionThread::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *AcquisitionThread::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *AcquisitionThread::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_AcquisitionThread))
        return static_cast<void*>(const_cast< AcquisitionThread*>(this));
    return QThread::qt_metacast(_clname);
}

int AcquisitionThread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: getPosition((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: getCameraStatus((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: getDacStatus((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: getTreatmentStatus((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: getAcquiring((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: getFilenumber((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: setImageFromCamera((*reinterpret_cast< uchar*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        default: ;
        }
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void AcquisitionThread::getPosition(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void AcquisitionThread::getCameraStatus(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void AcquisitionThread::getDacStatus(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void AcquisitionThread::getTreatmentStatus(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void AcquisitionThread::getAcquiring(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void AcquisitionThread::getFilenumber(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}
QT_END_MOC_NAMESPACE
