/****************************************************************************
** Meta object code from reading C++ file 'Dac.h'
**
** Created: Tue Apr 12 11:25:40 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "Dac.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Dac.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Dac[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: signature, parameters, type, tag, flags
      17,    5,    4,    4, 0x05,
      49,   41,    4,    4, 0x05,
      79,   70,    4,    4, 0x05,
     113,  103,    4,    4, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_Dac[] = {
    "Dac\0\0description\0getDescription(QString)\0"
    "message\0showWarning(QString)\0outputs,\0"
    "getOutputs(int,QString)\0dacvalues\0"
    "getOutputValues(QVector<float>*)\0"
};

const QMetaObject Dac::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Dac,
      qt_meta_data_Dac, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Dac::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Dac::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Dac::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Dac))
        return static_cast<void*>(const_cast< Dac*>(this));
    return QObject::qt_metacast(_clname);
}

int Dac::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: getDescription((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: showWarning((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: getOutputs((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 3: getOutputValues((*reinterpret_cast< QVector<float>*(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void Dac::getDescription(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Dac::showWarning(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void Dac::getOutputs(int _t1, QString _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void Dac::getOutputValues(QVector<float> * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_END_MOC_NAMESPACE
