#ifndef ABSTRACTCLIENT_H
#define ABSTRACTCLIENT_H

#include <QObject>
#include "protocol_datastructures.h"

class Command;
class CommandContainer;
class ProtocolItem;
class ProtocolResponse;
class TopLevelProtocolItem;
class CommandContainer;
class ChatEvent;
class GameEventContainer;
class Event_ListGames;
class Event_ServerMessage;
class Event_ListChatChannels;
class Event_GameJoined;

enum ClientStatus {
	StatusDisconnected,
	StatusDisconnecting,
	StatusConnecting,
	StatusAwaitingWelcome,
	StatusLoggingIn,
	StatusLoggedIn,
};

class AbstractClient : public QObject {
	Q_OBJECT
signals:
	void statusChanged(ClientStatus _status);
	void serverError(ResponseCode resp);
	
	// Chat events
	void chatEventReceived(ChatEvent *event);
	// Game events
	void gameEventContainerReceived(GameEventContainer *event);
	// Generic events
	void listGamesEventReceived(Event_ListGames *event);
	void serverMessageEventReceived(Event_ServerMessage *event);
	void listChatChannelsEventReceived(Event_ListChatChannels *event);
	void gameJoinedEventReceived(Event_GameJoined *event);
protected slots:
	void processProtocolItem(ProtocolItem *item);
protected:
	QMap<int, CommandContainer *> pendingCommands;
	ClientStatus status;
	QString userName, password;
	void setStatus(ClientStatus _status);
public:
	AbstractClient(QObject *parent = 0);
	~AbstractClient();
	
	ClientStatus getStatus() const { return status; }
	virtual void sendCommand(Command *cmd);
	virtual void sendCommandContainer(CommandContainer *cont) = 0;
};

#endif