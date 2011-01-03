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
class RoomEvent;
class GameEventContainer;
class Event_ListGames;
class Event_UserJoined;
class Event_UserLeft;
class Event_ServerMessage;
class Event_ListRooms;
class Event_GameJoined;
class Event_Message;

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
	
	// Room events
	void roomEventReceived(RoomEvent *event);
	// Game events
	void gameEventContainerReceived(GameEventContainer *event);
	// Generic events
	void userJoinedEventReceived(Event_UserJoined *event);
	void userLeftEventReceived(Event_UserLeft *event);
	void serverMessageEventReceived(Event_ServerMessage *event);
	void listRoomsEventReceived(Event_ListRooms *event);
	void gameJoinedEventReceived(Event_GameJoined *event);
	void messageEventReceived(Event_Message *event);
	void userInfoChanged(ServerInfo_User *userInfo);
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