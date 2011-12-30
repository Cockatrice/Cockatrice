#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QString>
#include <QMap>
#include <QHash>
#include <QObject>
#include <QVariant>
#include <QPair>

#include <google/protobuf/message.h>
#include "pb/server_message.pb.h"

class DeckList;
class GameEvent;
class GameEventContainer;
class GameEventContext;

static const int protocolVersion = 13;

class GameEventStorageItem {
public:
	enum EventRecipient { SendToPrivate = 0x01, SendToOthers = 0x02};
	Q_DECLARE_FLAGS(EventRecipients, EventRecipient)
private:
	::google::protobuf::Message *event;
	int playerId;
	EventRecipients recipients;
public:
	GameEventStorageItem(const ::google::protobuf::Message &_event, int _playerId, EventRecipients _recipients)
		: event(_event.New()), playerId(_playerId), recipients(_recipients)
	{
		event->CopyFrom(_event);
	}
	~GameEventStorageItem()
	{
		delete event;
	}
	const ::google::protobuf::Message &getEvent() const { return *event; }
	int getPlayerId() const { return playerId; }
	EventRecipients getRecipients() const { return recipients; }
};
Q_DECLARE_OPERATORS_FOR_FLAGS(GameEventStorageItem::EventRecipients)

class GameEventStorage {
private:
	::google::protobuf::Message *gameEventContext;
	QList<GameEventStorageItem *> gameEventList;
	int privatePlayerId;
public:
	GameEventStorage()
		: gameEventContext(0)
	{
	}
	~GameEventStorage()
	{
		delete gameEventContext;
		for (int i = 0; i < gameEventList.size(); ++i)
			delete gameEventList[i];
	}
	
	void setGameEventContext(::google::protobuf::Message *_gameEventContext) { gameEventContext = _gameEventContext; }
	::google::protobuf::Message *getGameEventContext() const { return gameEventContext; }
	
	int getPrivatePlayerId() const { return privatePlayerId; }
	
	void enqueueGameEvent(const ::google::protobuf::Message &event, int playerId, GameEventStorageItem::EventRecipients recipients = GameEventStorageItem::SendToPrivate | GameEventStorageItem::SendToOthers, int _privatePlayerId = -1)
	{
		gameEventList.append(new GameEventStorageItem(event, playerId, recipients));
		if (_privatePlayerId != -1)
			privatePlayerId = _privatePlayerId;
	}
};

class ResponseContainer {
private:
	::google::protobuf::Message *responseExtension;
	QList<QPair<ServerMessage::MessageType, ::google::protobuf::Message *> > preResponseQueue, postResponseQueue;
public:
	ResponseContainer() : responseExtension(0) { }
	~ResponseContainer()
	{
		delete responseExtension;
		for (int i = 0; i < preResponseQueue.size(); ++i)
			delete preResponseQueue[i].second;
		for (int i = 0; i < postResponseQueue.size(); ++i)
			delete postResponseQueue[i].second;
	}
	void setResponseExtension(::google::protobuf::Message *_responseExtension) { responseExtension = _responseExtension; }
	::google::protobuf::Message *getResponseExtension() const { return responseExtension; }
	void enqueuePreResponseItem(ServerMessage::MessageType type, ::google::protobuf::Message *item) { preResponseQueue.append(qMakePair(type, item)); }
	void enqueuePostResponseItem(ServerMessage::MessageType type, ::google::protobuf::Message *item) { postResponseQueue.append(qMakePair(type, item)); }
	const QList<QPair<ServerMessage::MessageType, ::google::protobuf::Message *> > &getPreResponseQueue() const { return preResponseQueue; }
	const QList<QPair<ServerMessage::MessageType, ::google::protobuf::Message *> > &getPostResponseQueue() const { return postResponseQueue; }
};

/*
 * XXX
 * 
class Response_DeckList : public ProtocolResponse {
	Q_OBJECT
public:
	Response_DeckList(int _cmdId = -1, ResponseCode _responseCode = RespOk, DeckList_Directory *_root = 0);
	int getItemId() const { return ItemId_Response_DeckList; }
	static SerializableItem *newItem() { return new Response_DeckList; }
	DeckList_Directory *getRoot() const { return static_cast<DeckList_Directory *>(itemMap.value("directory")); }
};

class Response_DeckDownload : public ProtocolResponse {
	Q_OBJECT
public:
	Response_DeckDownload(int _cmdId = -1, ResponseCode _responseCode = RespOk, DeckList *_deck = 0);
	int getItemId() const { return ItemId_Response_DeckDownload; }
	static SerializableItem *newItem() { return new Response_DeckDownload; }
	DeckList *getDeck() const;
};

class Response_DeckUpload : public ProtocolResponse {
	Q_OBJECT
public:
	Response_DeckUpload(int _cmdId = -1, ResponseCode _responseCode = RespOk, DeckList_File *_file = 0);
	int getItemId() const { return ItemId_Response_DeckUpload; }
	static SerializableItem *newItem() { return new Response_DeckUpload; }
	DeckList_File *getFile() const { return static_cast<DeckList_File *>(itemMap.value("file")); }
};
*/
#endif
