/****************************************************************************
** Meta object code from reading C++ file 'gamesmodel.h'
**
** Created: Thu Jul 30 20:38:03 2009
**      by: The Qt Meta Object Compiler version 61 (Qt 4.5.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../src/gamesmodel.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'gamesmodel.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 61
#error "This file was generated using the moc from 4.5.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_GamesModel[] = {

 // content:
       2,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   12, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors

 // slots: signature, parameters, type, tag, flags
      17,   12,   11,   11, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_GamesModel[] = {
    "GamesModel\0\0game\0updateGameList(ServerGame*)\0"
};

const QMetaObject GamesModel::staticMetaObject = {
    { &QAbstractTableModel::staticMetaObject, qt_meta_stringdata_GamesModel,
      qt_meta_data_GamesModel, 0 }
};

const QMetaObject *GamesModel::metaObject() const
{
    return &staticMetaObject;
}

void *GamesModel::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_GamesModel))
        return static_cast<void*>(const_cast< GamesModel*>(this));
    return QAbstractTableModel::qt_metacast(_clname);
}

int GamesModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractTableModel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: updateGameList((*reinterpret_cast< ServerGame*(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 1;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
