/****************************************************************************
** Meta object code from reading C++ file 'game.h'
**
** Created: Thu Jul 30 20:38:07 2009
**      by: The Qt Meta Object Compiler version 61 (Qt 4.5.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../src/game.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'game.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 61
#error "This file was generated using the moc from 4.5.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Game[] = {

 // content:
       2,       // revision
       0,       // classname
       0,    0, // classinfo
      41,   12, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors

 // signals: signature, parameters, type, tag, flags
       6,    5,    5,    5, 0x05,
      28,   23,    5,    5, 0x05,
      54,   47,    5,    5, 0x05,
      75,   47,    5,    5, 0x05,
     106,   98,    5,    5, 0x05,
     152,  141,    5,    5, 0x05,
     169,  141,    5,    5, 0x05,
     187,  141,    5,    5, 0x05,
     210,    5,    5,    5, 0x05,
     241,  225,    5,    5, 0x05,
     265,  141,    5,    5, 0x05,
     307,  285,    5,    5, 0x05,
     354,  336,    5,    5, 0x05,
     416,  375,    5,    5, 0x05,
     481,  461,    5,    5, 0x05,
     548,  513,    5,    5, 0x05,
     619,  592,    5,    5, 0x05,
     692,  654,    5,    5, 0x05,
     763,  731,    5,    5, 0x05,
     845,  803,    5,    5, 0x05,

 // slots: signature, parameters, type, tag, flags
     886,    5,    5,    5, 0x08,
     900,    5,    5,    5, 0x08,
     913,    5,    5,    5, 0x08,
     926,    5,    5,    5, 0x08,
     939,    5,    5,    5, 0x08,
     952,    5,    5,    5, 0x08,
     966,    5,    5,    5, 0x08,
     981,    5,    5,    5, 0x08,
     995,    5,    5,    5, 0x08,
    1014, 1012,    5,    5, 0x08,
    1035,    5,    5,    5, 0x08,
    1052,    5,    5,    5, 0x08,
    1062,    5,    5,    5, 0x08,
    1078,    5,    5,    5, 0x08,
    1097,    5,    5,    5, 0x08,
    1114,    5,    5,    5, 0x08,
    1134, 1130,    5,    5, 0x08,
    1172, 1161,    5,    5, 0x08,
    1213,    5,    5,    5, 0x08,
    1226,    5,    5,    5, 0x0a,
    1235,    5,    5,    5, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_Game[] = {
    "Game\0\0submitDecklist()\0name\0"
    "hoverCard(QString)\0player\0"
    "playerAdded(Player*)\0playerRemoved(Player*)\0"
    "players\0logPlayerListReceived(QStringList)\0"
    "playerName\0logJoin(QString)\0"
    "logLeave(QString)\0logReadyStart(QString)\0"
    "logGameStart()\0playerName,text\0"
    "logSay(QString,QString)\0logShuffle(QString)\0"
    "playerName,sides,roll\0"
    "logRollDice(QString,int,int)\0"
    "playerName,number\0logDraw(QString,int)\0"
    "playerName,cardName,startZone,targetZone\0"
    "logMoveCard(QString,QString,QString,QString)\0"
    "playerName,cardName\0logCreateToken(QString,QString)\0"
    "playerName,cardName,value,oldValue\0"
    "logSetCardCounters(QString,QString,int,int)\0"
    "playerName,cardName,tapped\0"
    "logSetTapped(QString,QString,bool)\0"
    "playerName,counterName,value,oldValue\0"
    "logSetCounter(QString,QString,int,int)\0"
    "playerName,cardName,doesntUntap\0"
    "logSetDoesntUntap(QString,QString,bool)\0"
    "playerName,zoneName,zoneOwner,numberCards\0"
    "logDumpZone(QString,QString,QString,int)\0"
    "actUntapAll()\0actIncLife()\0actDecLife()\0"
    "actSetLife()\0actShuffle()\0actDrawCard()\0"
    "actDrawCards()\0actRollDice()\0"
    "actCreateToken()\0p\0showCardMenu(QPoint)\0"
    "actDoesntUntap()\0actFlip()\0actAddCounter()\0"
    "actRemoveCounter()\0actSetCounters()\0"
    "actSayMessage()\0msg\0gameEvent(ServerEventData)\0"
    "playerList\0playerListReceived(QList<ServerPlayer*>)\0"
    "readyStart()\0actTap()\0actUntap()\0"
};

const QMetaObject Game::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Game,
      qt_meta_data_Game, 0 }
};

const QMetaObject *Game::metaObject() const
{
    return &staticMetaObject;
}

void *Game::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Game))
        return static_cast<void*>(const_cast< Game*>(this));
    return QObject::qt_metacast(_clname);
}

int Game::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: submitDecklist(); break;
        case 1: hoverCard((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: playerAdded((*reinterpret_cast< Player*(*)>(_a[1]))); break;
        case 3: playerRemoved((*reinterpret_cast< Player*(*)>(_a[1]))); break;
        case 4: logPlayerListReceived((*reinterpret_cast< QStringList(*)>(_a[1]))); break;
        case 5: logJoin((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 6: logLeave((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 7: logReadyStart((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 8: logGameStart(); break;
        case 9: logSay((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 10: logShuffle((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 11: logRollDice((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 12: logDraw((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 13: logMoveCard((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3])),(*reinterpret_cast< QString(*)>(_a[4]))); break;
        case 14: logCreateToken((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 15: logSetCardCounters((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< int(*)>(_a[4]))); break;
        case 16: logSetTapped((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 17: logSetCounter((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< int(*)>(_a[4]))); break;
        case 18: logSetDoesntUntap((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 19: logDumpZone((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3])),(*reinterpret_cast< int(*)>(_a[4]))); break;
        case 20: actUntapAll(); break;
        case 21: actIncLife(); break;
        case 22: actDecLife(); break;
        case 23: actSetLife(); break;
        case 24: actShuffle(); break;
        case 25: actDrawCard(); break;
        case 26: actDrawCards(); break;
        case 27: actRollDice(); break;
        case 28: actCreateToken(); break;
        case 29: showCardMenu((*reinterpret_cast< QPoint(*)>(_a[1]))); break;
        case 30: actDoesntUntap(); break;
        case 31: actFlip(); break;
        case 32: actAddCounter(); break;
        case 33: actRemoveCounter(); break;
        case 34: actSetCounters(); break;
        case 35: actSayMessage(); break;
        case 36: gameEvent((*reinterpret_cast< const ServerEventData(*)>(_a[1]))); break;
        case 37: playerListReceived((*reinterpret_cast< QList<ServerPlayer*>(*)>(_a[1]))); break;
        case 38: readyStart(); break;
        case 39: actTap(); break;
        case 40: actUntap(); break;
        default: ;
        }
        _id -= 41;
    }
    return _id;
}

// SIGNAL 0
void Game::submitDecklist()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void Game::hoverCard(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void Game::playerAdded(Player * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void Game::playerRemoved(Player * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void Game::logPlayerListReceived(QStringList _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void Game::logJoin(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void Game::logLeave(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void Game::logReadyStart(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void Game::logGameStart()
{
    QMetaObject::activate(this, &staticMetaObject, 8, 0);
}

// SIGNAL 9
void Game::logSay(QString _t1, QString _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 9, _a);
}

// SIGNAL 10
void Game::logShuffle(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 10, _a);
}

// SIGNAL 11
void Game::logRollDice(QString _t1, int _t2, int _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 11, _a);
}

// SIGNAL 12
void Game::logDraw(QString _t1, int _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 12, _a);
}

// SIGNAL 13
void Game::logMoveCard(QString _t1, QString _t2, QString _t3, QString _t4)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)), const_cast<void*>(reinterpret_cast<const void*>(&_t4)) };
    QMetaObject::activate(this, &staticMetaObject, 13, _a);
}

// SIGNAL 14
void Game::logCreateToken(QString _t1, QString _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 14, _a);
}

// SIGNAL 15
void Game::logSetCardCounters(QString _t1, QString _t2, int _t3, int _t4)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)), const_cast<void*>(reinterpret_cast<const void*>(&_t4)) };
    QMetaObject::activate(this, &staticMetaObject, 15, _a);
}

// SIGNAL 16
void Game::logSetTapped(QString _t1, QString _t2, bool _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 16, _a);
}

// SIGNAL 17
void Game::logSetCounter(QString _t1, QString _t2, int _t3, int _t4)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)), const_cast<void*>(reinterpret_cast<const void*>(&_t4)) };
    QMetaObject::activate(this, &staticMetaObject, 17, _a);
}

// SIGNAL 18
void Game::logSetDoesntUntap(QString _t1, QString _t2, bool _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 18, _a);
}

// SIGNAL 19
void Game::logDumpZone(QString _t1, QString _t2, QString _t3, int _t4)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)), const_cast<void*>(reinterpret_cast<const void*>(&_t4)) };
    QMetaObject::activate(this, &staticMetaObject, 19, _a);
}
QT_END_MOC_NAMESPACE
