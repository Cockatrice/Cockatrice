#include "abstractclient.h"
#include "protocol.h"
#include "protocol_items.h"

#include "pending_command.h"
#include "pb/commands.pb.h"
#include <google/protobuf/descriptor.h>

AbstractClient::AbstractClient(QObject *parent)
	: QObject(parent), nextCmdId(0), status(StatusDisconnected)
{
}

AbstractClient::~AbstractClient()
{
}

void AbstractClient::processProtocolItem(ProtocolItem *item)
{
	ProtocolResponse *response = qobject_cast<ProtocolResponse *>(item);
	if (response) {
		const int cmdId = response->getCmdId();
		PendingCommand *pend = pendingCommands.value(cmdId, 0);
		if (!pend)
			return;
		
		pendingCommands.remove(cmdId);
		pend->processResponse(response);
		if (response->getReceiverMayDelete())
			delete response;
		pend->deleteLater();
		
		return;
	}
	
	GenericEvent *genericEvent = qobject_cast<GenericEvent *>(item);
	if (genericEvent) {
		switch (genericEvent->getItemId()) {
			case ItemId_Event_ConnectionClosed: emit connectionClosedEventReceived(static_cast<Event_ConnectionClosed *>(item)); break;
			case ItemId_Event_ServerShutdown: emit serverShutdownEventReceived(static_cast<Event_ServerShutdown *>(item)); break;
			case ItemId_Event_AddToList: emit addToListEventReceived(static_cast<Event_AddToList *>(item)); break;
			case ItemId_Event_RemoveFromList: emit removeFromListEventReceived(static_cast<Event_RemoveFromList *>(item)); break;
			case ItemId_Event_UserJoined: emit userJoinedEventReceived(static_cast<Event_UserJoined *>(item)); break;
			case ItemId_Event_UserLeft: emit userLeftEventReceived(static_cast<Event_UserLeft *>(item)); break;
			case ItemId_Event_ServerMessage: emit serverMessageEventReceived(static_cast<Event_ServerMessage *>(item)); break;
			case ItemId_Event_ListRooms: emit listRoomsEventReceived(static_cast<Event_ListRooms *>(item)); break;
			case ItemId_Event_GameJoined: emit gameJoinedEventReceived(static_cast<Event_GameJoined *>(item)); break;
			case ItemId_Event_Message: emit messageEventReceived(static_cast<Event_Message *>(item)); break;
		}
		if (genericEvent->getReceiverMayDelete())
			delete genericEvent;
		return;
	}

	GameEventContainer *gameEventContainer = qobject_cast<GameEventContainer *>(item);
	if (gameEventContainer) {
		emit gameEventContainerReceived(gameEventContainer);
		if (gameEventContainer->getReceiverMayDelete())
			delete gameEventContainer;
		return;
	}

	RoomEvent *roomEvent = qobject_cast<RoomEvent *>(item);
	if (roomEvent) {
		emit roomEventReceived(roomEvent);
		if (roomEvent->getReceiverMayDelete())
			delete roomEvent;
		return;
	}
}

void AbstractClient::setStatus(const ClientStatus _status)
{
	if (_status != status) {
		status = _status;
		emit statusChanged(_status);
	}
}

void AbstractClient::sendCommand(const CommandContainer &cont)
{
	sendCommand(new PendingCommand(cont));
}

void AbstractClient::sendCommand(PendingCommand *pend)
{
	const int cmdId = nextCmdId++;
	pendingCommands.insert(cmdId, pend);
	pend->getCommandContainer().set_cmd_id(cmdId);
	sendCommandContainer(pend->getCommandContainer());
}

PendingCommand *AbstractClient::prepareSessionCommand(const ::google::protobuf::Message &cmd)
{
	CommandContainer cont;
	SessionCommand *c = cont.add_session_command();
	c->GetReflection()->MutableMessage(c, cmd.GetDescriptor()->FindExtensionByName("ext"))->CopyFrom(cmd);
	return new PendingCommand(cont);
}

PendingCommand *AbstractClient::prepareRoomCommand(const ::google::protobuf::Message &cmd, int roomId)
{
	CommandContainer cont;
	RoomCommand *c = cont.add_room_command();
	cont.set_room_id(roomId);
	c->GetReflection()->MutableMessage(c, cmd.GetDescriptor()->FindExtensionByName("ext"))->CopyFrom(cmd);
	return new PendingCommand(cont);
}

PendingCommand *AbstractClient::prepareModeratorCommand(const ::google::protobuf::Message &cmd)
{
	CommandContainer cont;
	ModeratorCommand *c = cont.add_moderator_command();
	c->GetReflection()->MutableMessage(c, cmd.GetDescriptor()->FindExtensionByName("ext"))->CopyFrom(cmd);
	return new PendingCommand(cont);
}

PendingCommand *AbstractClient::prepareAdminCommand(const ::google::protobuf::Message &cmd)
{
	CommandContainer cont;
	AdminCommand *c = cont.add_admin_command();
	c->GetReflection()->MutableMessage(c, cmd.GetDescriptor()->FindExtensionByName("ext"))->CopyFrom(cmd);
	return new PendingCommand(cont);
}
