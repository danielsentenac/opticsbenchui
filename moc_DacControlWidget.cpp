/****************************************************************************
** Meta object code from reading C++ file 'DacControlWidget.h'
**
** Created: Tue Apr 12 11:25:37 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "DacControlWidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DacControlWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_DacControlWidget[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      18,   17,   17,   17, 0x08,
      31,   17,   17,   17, 0x08,
      49,   42,   17,   17, 0x08,
      78,   66,   17,   17, 0x08,
     115,  102,   17,   17, 0x08,
     149,  139,   17,   17, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_DacControlWidget[] = {
    "DacControlWidget\0\0connectDac()\0"
    "resetDac()\0output\0setDacValue(int)\0"
    "description\0getDescription(QString)\0"
    "outputs,mode\0getOutputs(int,QString)\0"
    "dacvalues\0getOutputValues(QVector<float>*)\0"
};

const QMetaObject DacControlWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_DacControlWidget,
      qt_meta_data_DacControlWidget, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &DacControlWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *DacControlWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *DacControlWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DacControlWidget))
        return static_cast<void*>(const_cast< DacControlWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int DacControlWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: connectDac(); break;
        case 1: resetDac(); break;
        case 2: setDacValue((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: getDescription((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 4: getOutputs((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 5: getOutputValues((*reinterpret_cast< QVector<float>*(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 6;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
