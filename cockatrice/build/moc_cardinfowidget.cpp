/****************************************************************************
** Meta object code from reading C++ file 'cardinfowidget.h'
**
** Created: Thu Jul 30 20:38:11 2009
**      by: The Qt Meta Object Compiler version 61 (Qt 4.5.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../src/cardinfowidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'cardinfowidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 61
#error "This file was generated using the moc from 4.5.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_CardInfoWidget[] = {

 // content:
       2,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   12, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors

 // slots: signature, parameters, type, tag, flags
      21,   16,   15,   15, 0x0a,
      49,   40,   15,   15, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_CardInfoWidget[] = {
    "CardInfoWidget\0\0card\0setCard(CardInfo*)\0"
    "cardName\0setCard(QString)\0"
};

const QMetaObject CardInfoWidget::staticMetaObject = {
    { &QFrame::staticMetaObject, qt_meta_stringdata_CardInfoWidget,
      qt_meta_data_CardInfoWidget, 0 }
};

const QMetaObject *CardInfoWidget::metaObject() const
{
    return &staticMetaObject;
}

void *CardInfoWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_CardInfoWidget))
        return static_cast<void*>(const_cast< CardInfoWidget*>(this));
    return QFrame::qt_metacast(_clname);
}

int CardInfoWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QFrame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: setCard((*reinterpret_cast< CardInfo*(*)>(_a[1]))); break;
        case 1: setCard((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 2;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
