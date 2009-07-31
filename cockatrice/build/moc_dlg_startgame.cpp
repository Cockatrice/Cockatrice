/****************************************************************************
** Meta object code from reading C++ file 'dlg_startgame.h'
**
** Created: Fri Jul 31 01:20:56 2009
**      by: The Qt Meta Object Compiler version 61 (Qt 4.5.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../src/dlg_startgame.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'dlg_startgame.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 61
#error "This file was generated using the moc from 4.5.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_DlgStartGame[] = {

 // content:
       2,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   12, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors

 // signals: signature, parameters, type, tag, flags
      20,   14,   13,   13, 0x05,

 // slots: signature, parameters, type, tag, flags
      47,   13,   13,   13, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_DlgStartGame[] = {
    "DlgStartGame\0\0cards\0newDeckLoaded(QStringList)\0"
    "actLoad()\0"
};

const QMetaObject DlgStartGame::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_DlgStartGame,
      qt_meta_data_DlgStartGame, 0 }
};

const QMetaObject *DlgStartGame::metaObject() const
{
    return &staticMetaObject;
}

void *DlgStartGame::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DlgStartGame))
        return static_cast<void*>(const_cast< DlgStartGame*>(this));
    return QDialog::qt_metacast(_clname);
}

int DlgStartGame::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: newDeckLoaded((*reinterpret_cast< const QStringList(*)>(_a[1]))); break;
        case 1: actLoad(); break;
        default: ;
        }
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void DlgStartGame::newDeckLoaded(const QStringList & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
