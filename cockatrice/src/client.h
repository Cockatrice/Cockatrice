#ifndef CLIENT_H
#define CLIENT_H

#include <QTcpSocket>
#include <QColor>
#include <QStringList>
#include <QHash>
#include "protocol_datastructures.h"

class QTimer;
class Command;
class QXmlStreamReader;
class QXmlStreamWriter;

class ProtocolItem;
class ProtocolResponse;
class TopLevelProtocolItem;
class ChatEvent;
class GameEvent;
class Event_ListGames;
class Event_ServerMessage;
class Event_ListChatChannels;
class Event_GameJoined;

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
	void maxPingTime(int seconds, int maxSeconds);
	void serverTimeout();
	void logSocketError(const QString &errorString);
	void serverError(ResponseCode resp);
	void protocolVersionMismatch(int clientVersion, int serverVersion);
	void protocolError();
	
	// Chat events
	void chatEventReceived(ChatEvent *event);
	// Game events
	void gameEventReceived(GameEvent *event);
	// Generic events
	void listGamesEventReceived(Event_ListGames *event);
	void serverMessageEventReceived(Event_ServerMessage *event);
	void listChatChannelsEventReceived(Event_ListChatChannels *event);
	void gameJoinedEventReceived(Event_GameJoined *event);
	
private slots:
	void slotConnected();
	void readData();
	void slotSocketError(QAbstractSocket::SocketError error);
	void ping();
	void loginResponse(ResponseCode response);
	void processProtocolItem(ProtocolItem *item);
private:
	static const int maxTimeout = 10;
	
	QTimer *timer;
	QMap<int, Command *> pendingCommands;
	QTcpSocket *socket;
	QXmlStreamReader *xmlReader;
	QXmlStreamWriter *xmlWriter;
	TopLevelProtocolItem *topLevelItem;
	ClientStatus status;
	QString userName, password;
	void setStatus(ClientStatus _status);
public:
	Client(QObject *parent = 0);
	~Client();
	ClientStatus getStatus() const { return status; }
	QString peerName() const { return socket->peerName(); }

	void connectToServer(const QString &hostname, unsigned int port, const QString &_userName, const QString &_password);
	void disconnectFromServer();
	void sendCommand(Command *cmd);
};

#endif
