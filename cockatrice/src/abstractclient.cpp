#include "abstractclient.h"

#include "pending_command.h"
#include "pb/commands.pb.h"
#include "pb/server_message.pb.h"
#include "pb/event_server_identification.pb.h"
#include "pb/event_server_message.pb.h"
#include "pb/event_server_shutdown.pb.h"
#include "pb/event_connection_closed.pb.h"
#include "pb/event_user_message.pb.h"
#include "pb/event_list_rooms.pb.h"
#include "pb/event_add_to_list.pb.h"
#include "pb/event_remove_from_list.pb.h"
#include "pb/event_user_joined.pb.h"
#include "pb/event_user_left.pb.h"
#include "pb/event_game_joined.pb.h"
#include "pb/event_replay_added.pb.h"
#include "get_pb_extension.h"
#include <google/protobuf/descriptor.h>

AbstractClient::AbstractClient(QObject *parent)
	: QObject(parent), nextCmdId(0), status(StatusDisconnected)
{
}

AbstractClient::~AbstractClient()
{
}

void AbstractClient::processProtocolItem(const ServerMessage &item)
{
	switch (item.message_type()) {
		case ServerMessage::RESPONSE: {
			const Response &response = item.response();
			const int cmdId = response.cmd_id();
			PendingCommand *pend = pendingCommands.value(cmdId, 0);
			if (!pend)
				return;
			
			pendingCommands.remove(cmdId);
			pend->processResponse(response);
			pend->deleteLater();
			break;
		}
		case ServerMessage::SESSION_EVENT: {
			const SessionEvent &event = item.session_event();
			switch ((SessionEvent::SessionEventType) getPbExtension(event)) {
				case SessionEvent::SERVER_IDENTIFICATION: emit serverIdentificationEventReceived(event.GetExtension(Event_ServerIdentification::ext)); break;
				case SessionEvent::SERVER_MESSAGE: emit serverMessageEventReceived(event.GetExtension(Event_ServerMessage::ext)); break;
				case SessionEvent::SERVER_SHUTDOWN: emit serverShutdownEventReceived(event.GetExtension(Event_ServerShutdown::ext)); break;
				case SessionEvent::CONNECTION_CLOSED: emit connectionClosedEventReceived(event.GetExtension(Event_ConnectionClosed::ext)); break;
				case SessionEvent::USER_MESSAGE: emit userMessageEventReceived(event.GetExtension(Event_UserMessage::ext)); break;
				case SessionEvent::LIST_ROOMS: emit listRoomsEventReceived(event.GetExtension(Event_ListRooms::ext)); break;
				case SessionEvent::ADD_TO_LIST: emit addToListEventReceived(event.GetExtension(Event_AddToList::ext)); break;
				case SessionEvent::REMOVE_FROM_LIST: emit removeFromListEventReceived(event.GetExtension(Event_RemoveFromList::ext)); break;
				case SessionEvent::USER_JOINED: emit userJoinedEventReceived(event.GetExtension(Event_UserJoined::ext)); break;
				case SessionEvent::USER_LEFT: emit userLeftEventReceived(event.GetExtension(Event_UserLeft::ext)); break;
				case SessionEvent::GAME_JOINED: emit gameJoinedEventReceived(event.GetExtension(Event_GameJoined::ext)); break;
				case SessionEvent::REPLAY_ADDED: emit replayAddedEventReceived(event.GetExtension(Event_ReplayAdded::ext)); break;
			}
			break;
		}
		case ServerMessage::GAME_EVENT_CONTAINER: {
			emit gameEventContainerReceived(item.game_event_container());
			break;
		}
		case ServerMessage::ROOM_EVENT: {
			emit roomEventReceived(item.room_event());
			break;
		}
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
