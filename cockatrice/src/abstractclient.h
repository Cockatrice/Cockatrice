#ifndef ABSTRACTCLIENT_H
#define ABSTRACTCLIENT_H

#include <QObject>
#include <QVariant>
#include <google/protobuf/message.h>
#include "protocol_datastructures.h"

class PendingCommand;
class CommandContainer;
class ProtocolItem;
class ProtocolResponse;
class TopLevelProtocolItem;
class CommandContainer;
class RoomEvent;
class GameEventContainer;
class Event_AddToList;
class Event_RemoveFromList;
class Event_UserJoined;
class Event_UserLeft;
class Event_ServerMessage;
class Event_ListRooms;
class Event_GameJoined;
class Event_Message;
class Event_ConnectionClosed;
class Event_ServerShutdown;

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
	void connectionClosedEventReceived(Event_ConnectionClosed *event);
	void serverShutdownEventReceived(Event_ServerShutdown *event);
	void addToListEventReceived(Event_AddToList *event);
	void removeFromListEventReceived(Event_RemoveFromList *event);
	void userJoinedEventReceived(Event_UserJoined *event);
	void userLeftEventReceived(Event_UserLeft *event);
	void serverMessageEventReceived(Event_ServerMessage *event);
	void listRoomsEventReceived(Event_ListRooms *event);
	void gameJoinedEventReceived(Event_GameJoined *event);
	void messageEventReceived(Event_Message *event);
	void userInfoChanged(ServerInfo_User *userInfo);
	void buddyListReceived(const QList<ServerInfo_User *> &buddyList);
	void ignoreListReceived(const QList<ServerInfo_User *> &ignoreList);
private:
	int nextCmdId;
protected slots:
	void processProtocolItem(ProtocolItem *item);
protected:
	QMap<int, PendingCommand *> pendingCommands;
	ClientStatus status;
	QString userName, password;
	void setStatus(ClientStatus _status);
	virtual void sendCommandContainer(const CommandContainer &cont) = 0;
public:
	AbstractClient(QObject *parent = 0);
	~AbstractClient();
	
	ClientStatus getStatus() const { return status; }
	void sendCommand(const CommandContainer &cont);
	void sendCommand(PendingCommand *pend);
	
	PendingCommand *prepareSessionCommand(const ::google::protobuf::Message &cmd);
	PendingCommand *prepareModeratorCommand(const ::google::protobuf::Message &cmd);
	PendingCommand *prepareAdminCommand(const ::google::protobuf::Message &cmd);
};

#endif