/****************************************************************************
** Meta object code from reading C++ file 'MotorControlWidget.h'
**
** Created: Tue Apr 12 11:25:45 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "MotorControlWidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MotorControlWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MotorControlWidget[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      20,   19,   19,   19, 0x08,
      35,   19,   19,   19, 0x08,
      49,   19,   19,   19, 0x08,
      64,   19,   19,   19, 0x08,
      79,   19,   19,   19, 0x08,
     100,   91,   19,   19, 0x08,
     131,  119,   19,   19, 0x08,
     155,   19,   19,   19, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_MotorControlWidget[] = {
    "MotorControlWidget\0\0connectMotor()\0"
    "moveForward()\0moveBackward()\0"
    "moveAbsolute()\0stopMotor()\0position\0"
    "getPosition(float)\0description\0"
    "getDescription(QString)\0stopTimer()\0"
};

const QMetaObject MotorControlWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_MotorControlWidget,
      qt_meta_data_MotorControlWidget, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MotorControlWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MotorControlWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MotorControlWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MotorControlWidget))
        return static_cast<void*>(const_cast< MotorControlWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int MotorControlWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: connectMotor(); break;
        case 1: moveForward(); break;
        case 2: moveBackward(); break;
        case 3: moveAbsolute(); break;
        case 4: stopMotor(); break;
        case 5: getPosition((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 6: getDescription((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 7: stopTimer(); break;
        default: ;
        }
        _id -= 8;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
