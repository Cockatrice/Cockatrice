#ifndef CLIENT_H
#define CLIENT_H

#include "servereventdata.h"
#include "servergame.h"
#include "serverplayer.h"
#include "serverzone.h"
#include "serverzonecard.h"
#include <QTcpSocket>
#include <QColor>

class QTimer;

// Connection state.
// The protocol handler itself is stateless once the connection
// has been established.
enum ProtocolStatus { StatusDisconnected,
		      StatusConnecting,
		      StatusAwaitingWelcome,
		      StatusLoggingIn,
		      StatusIdle,
		      StatusPlaying };

enum ServerResponse {
	RespOk,
	RespPassword,
	RespErr
};

class PendingCommand : public QObject {
	Q_OBJECT
private:
	QString cmd;
	int msgid;
	int time;
	QString extraData;
signals:
	void finished(ServerResponse resp);
	void timeout();
public slots:
	void responseReceived(int _msgid, ServerResponse _resp);
	void checkTimeout();
public:
	int getMsgId() const { return msgid; }
	QString getCmd() const { return cmd; }
	const QString &getExtraData() const { return extraData; }
	void setExtraData(const QString &_extraData) { extraData = _extraData; }
	PendingCommand(const QString &_cmd, int _msgid, QObject *parent = 0);
};

class Client : public QObject {
	Q_OBJECT
signals:
	void statusChanged(ProtocolStatus _status);
	void welcomeMsgReceived(QStringList welcomeMsg);
	void gameListEvent(ServerGame *game);
	void playerListReceived(QList<ServerPlayer *> players);
	void zoneListReceived(int commandId, QList<ServerZone *> zones);
	void zoneDumpReceived(int commandId, QList<ServerZoneCard *> cards);
	void responseReceived(int msgid, ServerResponse resp);
	void playerIdReceived(int id, QString name);
	void gameEvent(const ServerEventData &msg);
	void chatEvent(const ChatEventData &msg);
	void serverTimeout();
	void logSocketError(const QString &errorString);
	void serverError(ServerResponse resp);
private slots:
	void slotConnected();
	void readLine();
	void timeout();
	void slotSocketError(QAbstractSocket::SocketError error);
	void ping();
	void removePendingCommand();
	void loginResponse(ServerResponse response);
	void enterGameResponse(ServerResponse response);
	void leaveGameResponse(ServerResponse response);
private:
	QTimer *timer;
	QList<PendingCommand *> PendingCommands;
	QTcpSocket *socket;
	ProtocolStatus status;
	QList<QStringList> msgbuf;
	QString playerName, password;
	unsigned int MsgId;
	void msg(const QString &s);
	PendingCommand *cmd(const QString &s);
	void setStatus(const ProtocolStatus _status);
public:
	Client(QObject *parent = 0);
	~Client();
	ProtocolStatus getStatus() const { return status; }
	QString peerName() const { return socket->peerName(); }

	void connectToServer(const QString &hostname, unsigned int port, const QString &_playerName, const QString &_password);
	void disconnectFromServer();
public slots:
	PendingCommand *chatListChannels();
	PendingCommand *chatJoinChannel(const QString &name);
	PendingCommand *chatLeaveChannel(const QString &name);
	PendingCommand *chatSay(const QString &name, const QString &s);
	PendingCommand *listGames();
	PendingCommand *listPlayers();
	PendingCommand *createGame(const QString &description, const QString &password, unsigned int maxPlayers);
	PendingCommand *joinGame(int gameId, const QString &password);
	PendingCommand *leaveGame();
	PendingCommand *login(const QString &name, const QString &pass);
	PendingCommand *say(const QString &s);
	PendingCommand *shuffle();
	PendingCommand *rollDice(unsigned int sides);
	PendingCommand *drawCards(unsigned int number);
	PendingCommand *moveCard(int cardid, const QString &startzone, const QString &targetzone, int x, int y = 0, bool faceDown = false);
	PendingCommand *createToken(const QString &zone, const QString &name, const QString &powtough, int x, int y);
	PendingCommand *setCardAttr(const QString &zone, int cardid, const QString &aname, const QString &avalue);
	PendingCommand *readyStart();
	PendingCommand *incCounter(const QString &counter, int delta);
	PendingCommand *addCounter(const QString &counter, QColor color, int value);
	PendingCommand *setCounter(const QString &counter, int value);
	PendingCommand *delCounter(const QString &counter);
	PendingCommand *nextTurn();
	PendingCommand *setActivePhase(int phase);
	PendingCommand *dumpZone(int player, const QString &zone, int numberCards);
	PendingCommand *stopDumpZone(int player, const QString &zone);
	void submitDeck(const QStringList &deck);
};

#endif
