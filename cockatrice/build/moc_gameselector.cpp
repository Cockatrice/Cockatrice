/****************************************************************************
** Meta object code from reading C++ file 'gameselector.h'
**
** Created: Thu Jul 30 20:38:00 2009
**      by: The Qt Meta Object Compiler version 61 (Qt 4.5.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../src/gameselector.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'gameselector.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 61
#error "This file was generated using the moc from 4.5.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_GameSelector[] = {

 // content:
       2,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   12, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors

 // slots: signature, parameters, type, tag, flags
      14,   13,   13,   13, 0x08,
      26,   13,   13,   13, 0x08,
      39,   13,   13,   13, 0x08,
      58,   49,   13,   13, 0x08,
      95,   88,   13,   13, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_GameSelector[] = {
    "GameSelector\0\0actCreate()\0actRefresh()\0"
    "actJoin()\0response\0checkResponse(ServerResponse)\0"
    "status\0statusChanged(ProtocolStatus)\0"
};

const QMetaObject GameSelector::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_GameSelector,
      qt_meta_data_GameSelector, 0 }
};

const QMetaObject *GameSelector::metaObject() const
{
    return &staticMetaObject;
}

void *GameSelector::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_GameSelector))
        return static_cast<void*>(const_cast< GameSelector*>(this));
    return QWidget::qt_metacast(_clname);
}

int GameSelector::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: actCreate(); break;
        case 1: actRefresh(); break;
        case 2: actJoin(); break;
        case 3: checkResponse((*reinterpret_cast< ServerResponse(*)>(_a[1]))); break;
        case 4: statusChanged((*reinterpret_cast< ProtocolStatus(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 5;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
