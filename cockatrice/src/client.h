#ifndef CLIENT_H
#define CLIENT_H

#include "servereventdata.h"
#include "serverresponse.h"
#include "servergame.h"
#include "serverplayer.h"
#include "serverzone.h"
#include "serverzonecard.h"
#include "pendingcommand.h"
#include <QTcpSocket>

class QTimer;

// Connection state.
// The protocol handler itself is stateless once the connection
// has been established.
enum ProtocolStatus { StatusDisconnected,
		      StatusConnecting,
		      StatusAwaitingWelcome,
		      StatusConnected };

class Client : public QObject {
	Q_OBJECT
signals:
	void statusChanged(ProtocolStatus _status);
	void welcomeMsgReceived(QStringList welcomeMsg);
	void gameListReceived(QList<ServerGame *> games);
	void playerListReceived(QList<ServerPlayer *> players);
	void zoneListReceived(int commandId, QList<ServerZone *> zones);
	void zoneDumpReceived(int commandId, QList<ServerZoneCard *> cards);
	void responseReceived(ServerResponse *response);
	void playerIdReceived(int id, QString name);
	void gameEvent(ServerEventData *msg);
	void serverTimeout();
	void logSocketError(const QString &errorString);

private slots:
	void slotConnected();
	void readLine();
	void checkTimeout();
	void slotSocketError(QAbstractSocket::SocketError error);

private:
	QTimer *timer;
	QList<PendingCommand *> PendingCommands;
	QTcpSocket *socket;
	ProtocolStatus status;
	QList<QStringList> msgbuf;
	QString PlayerName;
	unsigned int MsgId;
	void msg(const QString &s);
	int cmd(const QString &s);
	void setStatus(const ProtocolStatus _status);
public:
	Client(QObject *parent = 0);
	~Client();
	ProtocolStatus getStatus() { return status; }
	QString peerName() const { return socket->peerName(); }

	void connectToServer(const QString &hostname, unsigned int port, const QString &playername);
	void disconnectFromServer();
	int listGames();
	int listPlayers();
	int createGame(const QString &name, const QString &description, const QString &password, unsigned int maxPlayers);
	int joinGame(const QString &name, const QString &password);
	int leaveGame();
	int setName(const QString &name);
	int say(const QString &s);
	int shuffle();
	int rollDice(unsigned int sides);
	int drawCards(unsigned int number);
	int moveCard(int cardid, const QString &startzone, const QString &targetzone, int x, int y = 0);
	int createToken(const QString &zone, const QString &name, const QString &powtough, int x, int y);
	int setCardAttr(const QString &zone, int cardid, const QString &aname, const QString &avalue);
	int submitDeck(const QStringList &deck);
	int readyStart();
	int incCounter(const QString &counter, int delta);
	int setCounter(const QString &counter, int value);
	int delCounter(const QString &counter);
	int setActivePlayer(int player);
	int setActivePhase(int phase);
	int dumpZone(int player, const QString &zone, int numberCards);
};

#endif
