/****************************************************************************
** Meta object code from reading C++ file 'window_deckeditor.h'
**
** Created: Thu Jul 30 20:38:16 2009
**      by: The Qt Meta Object Compiler version 61 (Qt 4.5.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../src/window_deckeditor.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'window_deckeditor.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 61
#error "This file was generated using the moc from 4.5.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_WndDeckEditor[] = {

 // content:
       2,       // revision
       0,       // classname
       0,    0, // classinfo
      16,   12, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors

 // slots: signature, parameters, type, tag, flags
      20,   15,   14,   14, 0x08,
      40,   14,   14,   14, 0x08,
      74,   57,   14,   14, 0x08,
     118,   57,   14,   14, 0x08,
     170,  163,   14,   14, 0x08,
     192,   14,   14,   14, 0x08,
     205,   14,   14,   14, 0x08,
     224,   14,  219,   14, 0x08,
     238,   14,  219,   14, 0x08,
     254,   14,   14,   14, 0x08,
     269,   14,   14,   14, 0x08,
     283,   14,   14,   14, 0x08,
     296,   14,   14,   14, 0x08,
     320,   14,   14,   14, 0x08,
     336,   14,   14,   14, 0x08,
     351,   14,   14,   14, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_WndDeckEditor[] = {
    "WndDeckEditor\0\0name\0updateName(QString)\0"
    "updateComments()\0current,previous\0"
    "updateCardInfoLeft(QModelIndex,QModelIndex)\0"
    "updateCardInfoRight(QModelIndex,QModelIndex)\0"
    "search\0updateSearch(QString)\0actNewDeck()\0"
    "actLoadDeck()\0bool\0actSaveDeck()\0"
    "actSaveDeckAs()\0actPrintDeck()\0"
    "actEditSets()\0actAddCard()\0"
    "actAddCardToSideboard()\0actRemoveCard()\0"
    "actIncrement()\0actDecrement()\0"
};

const QMetaObject WndDeckEditor::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_WndDeckEditor,
      qt_meta_data_WndDeckEditor, 0 }
};

const QMetaObject *WndDeckEditor::metaObject() const
{
    return &staticMetaObject;
}

void *WndDeckEditor::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_WndDeckEditor))
        return static_cast<void*>(const_cast< WndDeckEditor*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int WndDeckEditor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: updateName((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: updateComments(); break;
        case 2: updateCardInfoLeft((*reinterpret_cast< const QModelIndex(*)>(_a[1])),(*reinterpret_cast< const QModelIndex(*)>(_a[2]))); break;
        case 3: updateCardInfoRight((*reinterpret_cast< const QModelIndex(*)>(_a[1])),(*reinterpret_cast< const QModelIndex(*)>(_a[2]))); break;
        case 4: updateSearch((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: actNewDeck(); break;
        case 6: actLoadDeck(); break;
        case 7: { bool _r = actSaveDeck();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 8: { bool _r = actSaveDeckAs();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        case 9: actPrintDeck(); break;
        case 10: actEditSets(); break;
        case 11: actAddCard(); break;
        case 12: actAddCardToSideboard(); break;
        case 13: actRemoveCard(); break;
        case 14: actIncrement(); break;
        case 15: actDecrement(); break;
        default: ;
        }
        _id -= 16;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
