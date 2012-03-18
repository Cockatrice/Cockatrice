#ifndef SERVER_RESPONSE_CONTAINERS_H
#define SERVER_RESPONSE_CONTAINERS_H

#include <QPair>
#include "pb/server_message.pb.h"

namespace google { namespace protobuf { class Message; } }
class Server_Game;

class GameEventStorageItem {
public:
	enum EventRecipient { SendToPrivate = 0x01, SendToOthers = 0x02};
	Q_DECLARE_FLAGS(EventRecipients, EventRecipient)
private:
	GameEvent *event;
	EventRecipients recipients;
public:
	GameEventStorageItem(const ::google::protobuf::Message &_event, int _playerId, EventRecipients _recipients);
	~GameEventStorageItem();
	
	const GameEvent &getGameEvent() const { return *event; }
	EventRecipients getRecipients() const { return recipients; }
};
Q_DECLARE_OPERATORS_FOR_FLAGS(GameEventStorageItem::EventRecipients)

class GameEventStorage {
private:
	::google::protobuf::Message *gameEventContext;
	QList<GameEventStorageItem *> gameEventList;
	int privatePlayerId;
public:
	GameEventStorage();
	~GameEventStorage();
	
	void setGameEventContext(const ::google::protobuf::Message &_gameEventContext);
	::google::protobuf::Message *getGameEventContext() const { return gameEventContext; }
	const QList<GameEventStorageItem *> &getGameEventList() const { return gameEventList; }
	int getPrivatePlayerId() const { return privatePlayerId; }
	
	void enqueueGameEvent(const ::google::protobuf::Message &event, int playerId, GameEventStorageItem::EventRecipients recipients = GameEventStorageItem::SendToPrivate | GameEventStorageItem::SendToOthers, int _privatePlayerId = -1);
	void sendToGame(Server_Game *game);
};

class ResponseContainer {
private:
	int cmdId;
	::google::protobuf::Message *responseExtension;
	QList<QPair<ServerMessage::MessageType, ::google::protobuf::Message *> > preResponseQueue, postResponseQueue;
public:
	ResponseContainer(int _cmdId);
	~ResponseContainer();
	
	int getCmdId() const { return cmdId; }
	void setResponseExtension(::google::protobuf::Message *_responseExtension) { responseExtension = _responseExtension; }
	::google::protobuf::Message *getResponseExtension() const { return responseExtension; }
	void enqueuePreResponseItem(ServerMessage::MessageType type, ::google::protobuf::Message *item) { preResponseQueue.append(qMakePair(type, item)); }
	void enqueuePostResponseItem(ServerMessage::MessageType type, ::google::protobuf::Message *item) { postResponseQueue.append(qMakePair(type, item)); }
	const QList<QPair<ServerMessage::MessageType, ::google::protobuf::Message *> > &getPreResponseQueue() const { return preResponseQueue; }
	const QList<QPair<ServerMessage::MessageType, ::google::protobuf::Message *> > &getPostResponseQueue() const { return postResponseQueue; }
};

#endif
