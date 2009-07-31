/****************************************************************************
** Meta object code from reading C++ file 'decklist.h'
**
** Created: Thu Jul 30 20:38:17 2009
**      by: The Qt Meta Object Compiler version 61 (Qt 4.5.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../src/decklist.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'decklist.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 61
#error "This file was generated using the moc from 4.5.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_DeckList[] = {

 // content:
       2,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   12, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors

 // signals: signature, parameters, type, tag, flags
      10,    9,    9,    9, 0x05,

 // slots: signature, parameters, type, tag, flags
      29,   23,    9,    9, 0x0a,
      46,    9,    9,    9, 0x2a,
      66,   56,    9,    9, 0x0a,
      87,    9,    9,    9, 0x2a,

       0        // eod
};

static const char qt_meta_stringdata_DeckList[] = {
    "DeckList\0\0deckLoaded()\0_name\0"
    "setName(QString)\0setName()\0_comments\0"
    "setComments(QString)\0setComments()\0"
};

const QMetaObject DeckList::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_DeckList,
      qt_meta_data_DeckList, 0 }
};

const QMetaObject *DeckList::metaObject() const
{
    return &staticMetaObject;
}

void *DeckList::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DeckList))
        return static_cast<void*>(const_cast< DeckList*>(this));
    return QObject::qt_metacast(_clname);
}

int DeckList::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: deckLoaded(); break;
        case 1: setName((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: setName(); break;
        case 3: setComments((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: setComments(); break;
        default: ;
        }
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void DeckList::deckLoaded()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
QT_END_MOC_NAMESPACE
