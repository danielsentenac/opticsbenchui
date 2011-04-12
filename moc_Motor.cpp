/****************************************************************************
** Meta object code from reading C++ file 'Motor.h'
**
** Created: Tue Apr 12 11:25:47 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "Motor.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Motor.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Motor[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: signature, parameters, type, tag, flags
      16,    7,    6,    6, 0x05,
      47,   35,    6,    6, 0x05,
      79,   71,    6,    6, 0x05,
     100,    6,    6,    6, 0x05,

 // slots: signature, parameters, type, tag, flags
     112,    6,    6,    6, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_Motor[] = {
    "Motor\0\0position\0getPosition(float)\0"
    "description\0getDescription(QString)\0"
    "message\0showWarning(QString)\0stopTimer()\0"
    "operationComplete()\0"
};

const QMetaObject Motor::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Motor,
      qt_meta_data_Motor, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Motor::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Motor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Motor::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Motor))
        return static_cast<void*>(const_cast< Motor*>(this));
    return QObject::qt_metacast(_clname);
}

int Motor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: getPosition((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 1: getDescription((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: showWarning((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 3: stopTimer(); break;
        case 4: operationComplete(); break;
        default: ;
        }
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void Motor::getPosition(float _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Motor::getDescription(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void Motor::showWarning(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void Motor::stopTimer()
{
    QMetaObject::activate(this, &staticMetaObject, 3, 0);
}
QT_END_MOC_NAMESPACE
