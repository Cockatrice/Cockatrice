#include "server_abstractuserinterface.h"
#include <google/protobuf/descriptor.h>

void Server_AbstractUserInterface::sendProtocolItemByType(ServerMessage::MessageType type, const ::google::protobuf::Message &item)
{
	switch (type) {
		case ServerMessage::RESPONSE: sendProtocolItem(static_cast<const Response &>(item)); break;
		case ServerMessage::SESSION_EVENT: sendProtocolItem(static_cast<const SessionEvent &>(item)); break;
		case ServerMessage::GAME_EVENT_CONTAINER: sendProtocolItem(static_cast<const GameEventContainer &>(item)); break;
		case ServerMessage::ROOM_EVENT: sendProtocolItem(static_cast<const RoomEvent &>(item)); break;
	}
}

SessionEvent *Server_AbstractUserInterface::prepareSessionEvent(const ::google::protobuf::Message &sessionEvent)
{
	SessionEvent *event = new SessionEvent;
	event->GetReflection()->MutableMessage(event, sessionEvent.GetDescriptor()->FindExtensionByName("ext"))->CopyFrom(sessionEvent);
	return event;
}
