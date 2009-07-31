/****************************************************************************
** Meta object code from reading C++ file 'setsmodel.h'
**
** Created: Thu Jul 30 20:38:18 2009
**      by: The Qt Meta Object Compiler version 61 (Qt 4.5.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../src/setsmodel.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'setsmodel.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 61
#error "This file was generated using the moc from 4.5.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_SetsMimeData[] = {

 // content:
       2,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors

       0        // eod
};

static const char qt_meta_stringdata_SetsMimeData[] = {
    "SetsMimeData\0"
};

const QMetaObject SetsMimeData::staticMetaObject = {
    { &QMimeData::staticMetaObject, qt_meta_stringdata_SetsMimeData,
      qt_meta_data_SetsMimeData, 0 }
};

const QMetaObject *SetsMimeData::metaObject() const
{
    return &staticMetaObject;
}

void *SetsMimeData::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SetsMimeData))
        return static_cast<void*>(const_cast< SetsMimeData*>(this));
    return QMimeData::qt_metacast(_clname);
}

int SetsMimeData::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMimeData::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_SetsModel[] = {

 // content:
       2,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors

       0        // eod
};

static const char qt_meta_stringdata_SetsModel[] = {
    "SetsModel\0"
};

const QMetaObject SetsModel::staticMetaObject = {
    { &QAbstractTableModel::staticMetaObject, qt_meta_stringdata_SetsModel,
      qt_meta_data_SetsModel, 0 }
};

const QMetaObject *SetsModel::metaObject() const
{
    return &staticMetaObject;
}

void *SetsModel::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SetsModel))
        return static_cast<void*>(const_cast< SetsModel*>(this));
    return QAbstractTableModel::qt_metacast(_clname);
}

int SetsModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractTableModel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
QT_END_MOC_NAMESPACE
