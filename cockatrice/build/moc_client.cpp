/****************************************************************************
** Meta object code from reading C++ file 'client.h'
**
** Created: Thu Jul 30 20:38:04 2009
**      by: The Qt Meta Object Compiler version 61 (Qt 4.5.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../src/client.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'client.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 61
#error "This file was generated using the moc from 4.5.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_PendingCommand[] = {

 // content:
       2,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   12, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors

 // signals: signature, parameters, type, tag, flags
      21,   16,   15,   15, 0x05,

 // slots: signature, parameters, type, tag, flags
      59,   46,   15,   15, 0x0a,
      96,   15,   15,   15, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_PendingCommand[] = {
    "PendingCommand\0\0resp\0finished(ServerResponse)\0"
    "_msgid,_resp\0responseReceived(int,ServerResponse)\0"
    "checkTimeout()\0"
};

const QMetaObject PendingCommand::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_PendingCommand,
      qt_meta_data_PendingCommand, 0 }
};

const QMetaObject *PendingCommand::metaObject() const
{
    return &staticMetaObject;
}

void *PendingCommand::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_PendingCommand))
        return static_cast<void*>(const_cast< PendingCommand*>(this));
    return QObject::qt_metacast(_clname);
}

int PendingCommand::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: finished((*reinterpret_cast< ServerResponse(*)>(_a[1]))); break;
        case 1: responseReceived((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< ServerResponse(*)>(_a[2]))); break;
        case 2: checkTimeout(); break;
        default: ;
        }
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void PendingCommand::finished(ServerResponse _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_Client[] = {

 // content:
       2,       // revision
       0,       // classname
       0,    0, // classinfo
      22,   12, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors

 // signals: signature, parameters, type, tag, flags
      16,    8,    7,    7, 0x05,
      57,   46,    7,    7, 0x05,
      94,   89,    7,    7, 0x05,
     129,  121,    7,    7, 0x05,
     186,  170,    7,    7, 0x05,
     243,  227,    7,    7, 0x05,
     299,  288,    7,    7, 0x05,
     344,  336,    7,    7, 0x05,
     378,  374,    7,    7, 0x05,
     405,    7,    7,    7, 0x05,
     433,  421,    7,    7, 0x05,
     462,  457,    7,    7, 0x05,

 // slots: signature, parameters, type, tag, flags
     490,    7,    7,    7, 0x08,
     506,    7,    7,    7, 0x08,
     517,    7,    7,    7, 0x08,
     533,  527,    7,    7, 0x08,
     579,    7,    7,    7, 0x08,
     586,    7,    7,    7, 0x08,
     618,  609,    7,    7, 0x08,
     648,  609,    7,    7, 0x08,
     682,  609,    7,    7, 0x08,
     721,  716,    7,    7, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_Client[] = {
    "Client\0\0_status\0statusChanged(ProtocolStatus)\0"
    "welcomeMsg\0welcomeMsgReceived(QStringList)\0"
    "game\0gameListEvent(ServerGame*)\0players\0"
    "playerListReceived(QList<ServerPlayer*>)\0"
    "commandId,zones\0"
    "zoneListReceived(int,QList<ServerZone*>)\0"
    "commandId,cards\0"
    "zoneDumpReceived(int,QList<ServerZoneCard*>)\0"
    "msgid,resp\0responseReceived(int,ServerResponse)\0"
    "id,name\0playerIdReceived(int,QString)\0"
    "msg\0gameEvent(ServerEventData)\0"
    "serverTimeout()\0errorString\0"
    "logSocketError(QString)\0resp\0"
    "serverError(ServerResponse)\0slotConnected()\0"
    "readLine()\0timeout()\0error\0"
    "slotSocketError(QAbstractSocket::SocketError)\0"
    "ping()\0removePendingCommand()\0response\0"
    "loginResponse(ServerResponse)\0"
    "enterGameResponse(ServerResponse)\0"
    "leaveGameResponse(ServerResponse)\0"
    "deck\0submitDeck(QStringList)\0"
};

const QMetaObject Client::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Client,
      qt_meta_data_Client, 0 }
};

const QMetaObject *Client::metaObject() const
{
    return &staticMetaObject;
}

void *Client::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Client))
        return static_cast<void*>(const_cast< Client*>(this));
    return QObject::qt_metacast(_clname);
}

int Client::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: statusChanged((*reinterpret_cast< ProtocolStatus(*)>(_a[1]))); break;
        case 1: welcomeMsgReceived((*reinterpret_cast< QStringList(*)>(_a[1]))); break;
        case 2: gameListEvent((*reinterpret_cast< ServerGame*(*)>(_a[1]))); break;
        case 3: playerListReceived((*reinterpret_cast< QList<ServerPlayer*>(*)>(_a[1]))); break;
        case 4: zoneListReceived((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< QList<ServerZone*>(*)>(_a[2]))); break;
        case 5: zoneDumpReceived((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< QList<ServerZoneCard*>(*)>(_a[2]))); break;
        case 6: responseReceived((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< ServerResponse(*)>(_a[2]))); break;
        case 7: playerIdReceived((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 8: gameEvent((*reinterpret_cast< const ServerEventData(*)>(_a[1]))); break;
        case 9: serverTimeout(); break;
        case 10: logSocketError((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 11: serverError((*reinterpret_cast< ServerResponse(*)>(_a[1]))); break;
        case 12: slotConnected(); break;
        case 13: readLine(); break;
        case 14: timeout(); break;
        case 15: slotSocketError((*reinterpret_cast< QAbstractSocket::SocketError(*)>(_a[1]))); break;
        case 16: ping(); break;
        case 17: removePendingCommand(); break;
        case 18: loginResponse((*reinterpret_cast< ServerResponse(*)>(_a[1]))); break;
        case 19: enterGameResponse((*reinterpret_cast< ServerResponse(*)>(_a[1]))); break;
        case 20: leaveGameResponse((*reinterpret_cast< ServerResponse(*)>(_a[1]))); break;
        case 21: submitDeck((*reinterpret_cast< const QStringList(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 22;
    }
    return _id;
}

// SIGNAL 0
void Client::statusChanged(ProtocolStatus _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Client::welcomeMsgReceived(QStringList _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void Client::gameListEvent(ServerGame * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void Client::playerListReceived(QList<ServerPlayer*> _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void Client::zoneListReceived(int _t1, QList<ServerZone*> _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void Client::zoneDumpReceived(int _t1, QList<ServerZoneCard*> _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void Client::responseReceived(int _t1, ServerResponse _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void Client::playerIdReceived(int _t1, QString _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void Client::gameEvent(const ServerEventData & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}

// SIGNAL 9
void Client::serverTimeout()
{
    QMetaObject::activate(this, &staticMetaObject, 9, 0);
}

// SIGNAL 10
void Client::logSocketError(const QString & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 10, _a);
}

// SIGNAL 11
void Client::serverError(ServerResponse _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 11, _a);
}
QT_END_MOC_NAMESPACE
