#include <QList>
#include <QPair>
#include "server_abstractuserinterface.h"
#include "server_response_containers.h"
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

void Server_AbstractUserInterface::sendResponseContainer(const ResponseContainer &responseContainer, Response::ResponseCode responseCode)
{
	const QList<QPair<ServerMessage::MessageType, ::google::protobuf::Message *> > &preResponseQueue = responseContainer.getPreResponseQueue();
	for (int i = 0; i < preResponseQueue.size(); ++i)
		sendProtocolItemByType(preResponseQueue[i].first, *preResponseQueue[i].second);
	
	Response response;
	response.set_cmd_id(responseContainer.getCmdId());
	response.set_response_code(responseCode);
	::google::protobuf::Message *responseExtension = responseContainer.getResponseExtension();
	if (responseExtension)
		response.GetReflection()->MutableMessage(&response, responseExtension->GetDescriptor()->FindExtensionByName("ext"))->CopyFrom(*responseExtension);
	sendProtocolItem(response);
	
	const QList<QPair<ServerMessage::MessageType, ::google::protobuf::Message *> > &postResponseQueue = responseContainer.getPostResponseQueue();
	for (int i = 0; i < postResponseQueue.size(); ++i)
		sendProtocolItemByType(postResponseQueue[i].first, *postResponseQueue[i].second);
}
