/****************************************************************************
** Meta object code from reading C++ file 'window_main.h'
**
** Created: Thu Jul 30 20:38:05 2009
**      by: The Qt Meta Object Compiler version 61 (Qt 4.5.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../src/window_main.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'window_main.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 61
#error "This file was generated using the moc from 4.5.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MainWindow[] = {

 // content:
       2,       // revision
       0,       // classname
       0,    0, // classinfo
      18,   12, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors

 // signals: signature, parameters, type, tag, flags
      21,   12,   11,   11, 0x05,
      44,   11,   11,   11, 0x05,

 // slots: signature, parameters, type, tag, flags
      67,   62,   11,   11, 0x08,
      93,   86,   11,   11, 0x08,
     114,   86,   11,   11, 0x08,
     145,  137,   11,   11, 0x08,
     183,  175,   11,   11, 0x08,
     213,   11,   11,   11, 0x08,
     229,   11,   11,   11, 0x08,
     238,   11,   11,   11, 0x08,
     251,   11,   11,   11, 0x08,
     267,   11,   11,   11, 0x08,
     284,   11,   11,   11, 0x08,
     299,   11,   11,   11, 0x08,
     323,  315,   11,   11, 0x08,
     343,   11,   11,   11, 0x08,
     357,   11,   11,   11, 0x08,
     367,   11,   11,   11, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_MainWindow[] = {
    "MainWindow\0\0hostname\0logConnecting(QString)\0"
    "logDisconnected()\0name\0hoverCard(QString)\0"
    "player\0playerAdded(Player*)\0"
    "playerRemoved(Player*)\0_status\0"
    "statusChanged(ProtocolStatus)\0id,name\0"
    "playerIdReceived(int,QString)\0"
    "serverTimeout()\0actSay()\0actConnect()\0"
    "actDisconnect()\0actRestartGame()\0"
    "actLeaveGame()\0actDeckEditor()\0checked\0"
    "actFullScreen(bool)\0actSettings()\0"
    "actExit()\0updateSceneSize()\0"
};

const QMetaObject MainWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_MainWindow,
      qt_meta_data_MainWindow, 0 }
};

const QMetaObject *MainWindow::metaObject() const
{
    return &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow))
        return static_cast<void*>(const_cast< MainWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: logConnecting((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: logDisconnected(); break;
        case 2: hoverCard((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 3: playerAdded((*reinterpret_cast< Player*(*)>(_a[1]))); break;
        case 4: playerRemoved((*reinterpret_cast< Player*(*)>(_a[1]))); break;
        case 5: statusChanged((*reinterpret_cast< ProtocolStatus(*)>(_a[1]))); break;
        case 6: playerIdReceived((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 7: serverTimeout(); break;
        case 8: actSay(); break;
        case 9: actConnect(); break;
        case 10: actDisconnect(); break;
        case 11: actRestartGame(); break;
        case 12: actLeaveGame(); break;
        case 13: actDeckEditor(); break;
        case 14: actFullScreen((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 15: actSettings(); break;
        case 16: actExit(); break;
        case 17: updateSceneSize(); break;
        default: ;
        }
        _id -= 18;
    }
    return _id;
}

// SIGNAL 0
void MainWindow::logConnecting(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void MainWindow::logDisconnected()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}
QT_END_MOC_NAMESPACE
