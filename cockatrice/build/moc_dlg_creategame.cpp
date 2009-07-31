/****************************************************************************
** Meta object code from reading C++ file 'dlg_creategame.h'
**
** Created: Thu Jul 30 20:38:01 2009
**      by: The Qt Meta Object Compiler version 61 (Qt 4.5.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../src/dlg_creategame.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'dlg_creategame.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 61
#error "This file was generated using the moc from 4.5.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_DlgCreateGame[] = {

 // content:
       2,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   12, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors

 // slots: signature, parameters, type, tag, flags
      15,   14,   14,   14, 0x08,
      32,   23,   14,   14, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_DlgCreateGame[] = {
    "DlgCreateGame\0\0actOK()\0response\0"
    "checkResponse(ServerResponse)\0"
};

const QMetaObject DlgCreateGame::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_DlgCreateGame,
      qt_meta_data_DlgCreateGame, 0 }
};

const QMetaObject *DlgCreateGame::metaObject() const
{
    return &staticMetaObject;
}

void *DlgCreateGame::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DlgCreateGame))
        return static_cast<void*>(const_cast< DlgCreateGame*>(this));
    return QDialog::qt_metacast(_clname);
}

int DlgCreateGame::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: actOK(); break;
        case 1: checkResponse((*reinterpret_cast< ServerResponse(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 2;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
