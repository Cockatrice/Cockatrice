/****************************************************************************
** Meta object code from reading C++ file 'zoneviewlayout.h'
**
** Created: Thu Jul 30 20:38:14 2009
**      by: The Qt Meta Object Compiler version 61 (Qt 4.5.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../src/zoneviewlayout.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'zoneviewlayout.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 61
#error "This file was generated using the moc from 4.5.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ZoneViewLayout[] = {

 // content:
       2,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   12, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors

 // signals: signature, parameters, type, tag, flags
      16,   15,   15,   15, 0x05,

 // slots: signature, parameters, type, tag, flags
      58,   30,   15,   15, 0x0a,
     110,   94,   15,   15, 0x2a,
     147,  142,   15,   15, 0x0a,
     175,  142,   15,   15, 0x0a,
     201,   15,   15,   15, 0x0a,
     227,   15,   15,   15, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_ZoneViewLayout[] = {
    "ZoneViewLayout\0\0sizeChanged()\0"
    "player,zoneName,numberCards\0"
    "toggleZoneView(Player*,QString,int)\0"
    "player,zoneName\0toggleZoneView(Player*,QString)\0"
    "item\0removeItem(ZoneViewWidget*)\0"
    "removeItem(ZoneViewZone*)\0"
    "closeMostRecentZoneView()\0clear()\0"
};

const QMetaObject ZoneViewLayout::staticMetaObject = {
    { &QGraphicsWidget::staticMetaObject, qt_meta_stringdata_ZoneViewLayout,
      qt_meta_data_ZoneViewLayout, 0 }
};

const QMetaObject *ZoneViewLayout::metaObject() const
{
    return &staticMetaObject;
}

void *ZoneViewLayout::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ZoneViewLayout))
        return static_cast<void*>(const_cast< ZoneViewLayout*>(this));
    return QGraphicsWidget::qt_metacast(_clname);
}

int ZoneViewLayout::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGraphicsWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: sizeChanged(); break;
        case 1: toggleZoneView((*reinterpret_cast< Player*(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 2: toggleZoneView((*reinterpret_cast< Player*(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 3: removeItem((*reinterpret_cast< ZoneViewWidget*(*)>(_a[1]))); break;
        case 4: removeItem((*reinterpret_cast< ZoneViewZone*(*)>(_a[1]))); break;
        case 5: closeMostRecentZoneView(); break;
        case 6: clear(); break;
        default: ;
        }
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void ZoneViewLayout::sizeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
QT_END_MOC_NAMESPACE
