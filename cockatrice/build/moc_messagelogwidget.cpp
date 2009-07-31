/****************************************************************************
** Meta object code from reading C++ file 'messagelogwidget.h'
**
** Created: Thu Jul 30 20:38:12 2009
**      by: The Qt Meta Object Compiler version 61 (Qt 4.5.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../src/messagelogwidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'messagelogwidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 61
#error "This file was generated using the moc from 4.5.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MessageLogWidget[] = {

 // content:
       2,       // revision
       0,       // classname
       0,    0, // classinfo
      21,   12, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors

 // slots: signature, parameters, type, tag, flags
      27,   18,   17,   17, 0x0a,
      61,   50,   17,   17, 0x0a,
      87,   17,   17,   17, 0x0a,
     117,  105,   17,   17, 0x0a,
     150,  141,   17,   17, 0x0a,
     189,  181,   17,   17, 0x08,
     235,  224,   17,   17, 0x08,
     252,  224,   17,   17, 0x08,
     270,  224,   17,   17, 0x08,
     293,   17,   17,   17, 0x08,
     327,  308,   17,   17, 0x08,
     351,  224,   17,   17, 0x08,
     393,  371,   17,   17, 0x08,
     440,  422,   17,   17, 0x08,
     502,  461,   17,   17, 0x08,
     567,  547,   17,   17, 0x08,
     634,  599,   17,   17, 0x08,
     705,  678,   17,   17, 0x08,
     778,  740,   17,   17, 0x08,
     849,  817,   17,   17, 0x08,
     931,  889,   17,   17, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_MessageLogWidget[] = {
    "MessageLogWidget\0\0hostname\0"
    "logConnecting(QString)\0WelcomeMsg\0"
    "logConnected(QStringList)\0logDisconnected()\0"
    "errorString\0logSocketError(QString)\0"
    "response\0logServerError(ServerResponse)\0"
    "players\0logPlayerListReceived(QStringList)\0"
    "playerName\0logJoin(QString)\0"
    "logLeave(QString)\0logReadyStart(QString)\0"
    "logGameStart()\0playerName,message\0"
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
};

const QMetaObject MessageLogWidget::staticMetaObject = {
    { &QTextEdit::staticMetaObject, qt_meta_stringdata_MessageLogWidget,
      qt_meta_data_MessageLogWidget, 0 }
};

const QMetaObject *MessageLogWidget::metaObject() const
{
    return &staticMetaObject;
}

void *MessageLogWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MessageLogWidget))
        return static_cast<void*>(const_cast< MessageLogWidget*>(this));
    return QTextEdit::qt_metacast(_clname);
}

int MessageLogWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTextEdit::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: logConnecting((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: logConnected((*reinterpret_cast< const QStringList(*)>(_a[1]))); break;
        case 2: logDisconnected(); break;
        case 3: logSocketError((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: logServerError((*reinterpret_cast< ServerResponse(*)>(_a[1]))); break;
        case 5: logPlayerListReceived((*reinterpret_cast< QStringList(*)>(_a[1]))); break;
        case 6: logJoin((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 7: logLeave((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 8: logReadyStart((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 9: logGameStart(); break;
        case 10: logSay((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 11: logShuffle((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 12: logRollDice((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 13: logDraw((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 14: logMoveCard((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3])),(*reinterpret_cast< QString(*)>(_a[4]))); break;
        case 15: logCreateToken((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 16: logSetCardCounters((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< int(*)>(_a[4]))); break;
        case 17: logSetTapped((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 18: logSetCounter((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< int(*)>(_a[4]))); break;
        case 19: logSetDoesntUntap((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 20: logDumpZone((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3])),(*reinterpret_cast< int(*)>(_a[4]))); break;
        default: ;
        }
        _id -= 21;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
