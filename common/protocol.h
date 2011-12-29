#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QString>
#include <QMap>
#include <QHash>
#include <QObject>
#include <QVariant>
#include "protocol_datastructures.h"

class QXmlStreamReader;
class QXmlStreamWriter;
class QXmlStreamAttributes;

class ProtocolResponse;
class DeckList;
class GameEvent;
class GameEventContainer;
class GameEventContext;
class MoveCardToZone;

class ProtocolItem : public SerializableItem_Map {
	Q_OBJECT
private:
	static void initializeHashAuto();
	bool receiverMayDelete;
public:
	static const int protocolVersion = 13;
	static void initializeHash();
	virtual int getItemId() const = 0;
	bool getReceiverMayDelete() const { return receiverMayDelete; }
	void setReceiverMayDelete(bool _receiverMayDelete) { receiverMayDelete = _receiverMayDelete; }
	ProtocolItem(const QString &_itemType, const QString &_itemSubType);
	bool isEmpty() const { return false; }
};

// ----------------
// --- COMMANDS ---
// ----------------

class BlaContainer : public ProtocolItem {
	Q_OBJECT
private:
	ProtocolResponse *resp;
	QList<ProtocolItem *> itemQueue;
	GameEventContext *gameEventContext;
	GameEventContainer *gameEventQueuePublic;
	GameEventContainer *gameEventQueueOmniscient;
	GameEventContainer *gameEventQueuePrivate;
	int privatePlayerId;
public:
	BlaContainer();
	int getItemId() const { return 102332456; }
	
	ProtocolResponse *getResponse() const { return resp; }
	void setResponse(ProtocolResponse *_resp);
	const QList<ProtocolItem *> &getItemQueue() const { return itemQueue; }
	void enqueueItem(ProtocolItem *item) { itemQueue.append(item); }
	GameEventContainer *getGameEventQueuePublic() const { return gameEventQueuePublic; }
	void enqueueGameEventPublic(GameEvent *event, int gameId, GameEventContext *context = 0);
	GameEventContainer *getGameEventQueueOmniscient() const { return gameEventQueueOmniscient; }
	void enqueueGameEventOmniscient(GameEvent *event, int gameId, GameEventContext *context = 0);
	GameEventContainer *getGameEventQueuePrivate() const { return gameEventQueuePrivate; }
	void enqueueGameEventPrivate(GameEvent *event, int gameId, int playerId = -1, GameEventContext *context = 0);
	int getPrivatePlayerId() const { return privatePlayerId; }
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
