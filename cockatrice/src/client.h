#ifndef CLIENT_H
#define CLIENT_H

#include <QTcpSocket>
#include <QColor>
#include <QStringList>
#include <QHash>

class QTimer;
class Command;
class QXmlStreamReader;
class QXmlStreamWriter;
class ProtocolItem;

enum ClientStatus {
	StatusDisconnected,
	StatusConnecting,
	StatusAwaitingWelcome,
	StatusLoggingIn,
	StatusLoggedIn,
};

class Client : public QObject {
	Q_OBJECT
signals:
	void statusChanged(ClientStatus _status);
	void welcomeMsgReceived(QString welcomeMsg);
//	void gameListEvent(const ServerGame &game);
	void playerIdReceived(int id, QString name);
//	void gameEvent(const ServerEventData &msg);
//	void chatEvent(const ChatEventData &msg);
	void maxPingTime(int seconds, int maxSeconds);
	void serverTimeout();
	void logSocketError(const QString &errorString);
//	void serverError(ServerResponse resp);
	void protocolVersionMismatch();
	void protocolError();
private slots:
	void slotConnected();
	void readData();
	void slotSocketError(QAbstractSocket::SocketError error);
	void ping();
	void removePendingCommand();
//	void loginResponse(ServerResponse response);
//	void enterGameResponse(ServerResponse response);
//	void leaveGameResponse(ServerResponse response);
private:
	static const int maxTimeout = 10;
	
	QTimer *timer;
	QMap<int, Command *> pendingCommands;
	QTcpSocket *socket;
	QXmlStreamReader *xmlReader;
	QXmlStreamWriter *xmlWriter;
	ProtocolItem *currentItem;
	ClientStatus status;
	QString playerName, password;
	void setStatus(ClientStatus _status);
public:
	Client(QObject *parent = 0);
	~Client();
	ClientStatus getStatus() const { return status; }
	QString peerName() const { return socket->peerName(); }

	void connectToServer(const QString &hostname, unsigned int port, const QString &_playerName, const QString &_password);
	void disconnectFromServer();
public slots:

	void chatListChannels() { }
	void chatJoinChannel(const QString &name) { }
	void chatLeaveChannel(const QString &name) { }
	void chatSay(const QString &name, const QString &s) { }
	void listGames() { }
	void listPlayers() { }
	void createGame(const QString &description, const QString &password, unsigned int maxPlayers, bool spectatorsAllowed) { }
	void joinGame(int gameId, const QString &password, bool spectator) { }
	void leaveGame() { }
	void login(const QString &name, const QString &pass) { }
	void say(const QString &s) { }
	void shuffle() { }
	void rollDie(unsigned int sides) { }
	void drawCard() { return drawCards(1); }
	void drawCards(unsigned int number) { }
	void moveCard(int cardid, const QString &startzone, const QString &targetzone, int x, int y = 0, bool faceDown = false) { }
	void createToken(const QString &zone, const QString &name, const QString &powtough, int x, int y) { }
	void createArrow(int startPlayerId, const QString &startZone, int startCardId, int targetPlayerId, const QString &targetPlayerZone, int targetCardId, const QColor &color) { }
	void deleteArrow(int arrowId) { }
	void setCardAttr(const QString &zone, int cardid, const QString &aname, const QString &avalue) { }
	void readyStart() { }
	void incCounter(int counterId, int delta) { }
	void addCounter(const QString &counterName, QColor color, int radius, int value) { }
	void setCounter(int counterId, int value) { }
	void delCounter(int counterId) { }
	void nextTurn() { }
	void setActivePhase(int phase) { }
	void dumpZone(int player, const QString &zone, int numberCards) { }
	void stopDumpZone(int player, const QString &zone) { }
	void dumpAll() { }
	void submitDeck(const QStringList &deck) { }

};

#endif
