/****************************************************************************
** Meta object code from reading C++ file 'carddatabase.h'
**
** Created: Thu Jul 30 20:38:09 2009
**      by: The Qt Meta Object Compiler version 61 (Qt 4.5.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../src/carddatabase.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'carddatabase.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 61
#error "This file was generated using the moc from 4.5.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_CardDatabase[] = {

 // content:
       2,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   12, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors

 // slots: signature, parameters, type, tag, flags
      19,   14,   13,   13, 0x0a,
      43,   13,   13,   13, 0x2a,
      60,   14,   13,   13, 0x0a,
      88,   13,   13,   13, 0x2a,

       0        // eod
};

static const char qt_meta_stringdata_CardDatabase[] = {
    "CardDatabase\0\0path\0updatePicsPath(QString)\0"
    "updatePicsPath()\0updateDatabasePath(QString)\0"
    "updateDatabasePath()\0"
};

const QMetaObject CardDatabase::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_CardDatabase,
      qt_meta_data_CardDatabase, 0 }
};

const QMetaObject *CardDatabase::metaObject() const
{
    return &staticMetaObject;
}

void *CardDatabase::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_CardDatabase))
        return static_cast<void*>(const_cast< CardDatabase*>(this));
    return QObject::qt_metacast(_clname);
}

int CardDatabase::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: updatePicsPath((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: updatePicsPath(); break;
        case 2: updateDatabasePath((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: updateDatabasePath(); break;
        default: ;
        }
        _id -= 4;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
