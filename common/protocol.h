#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QString>
#include <QMap>
#include <QHash>
#include <QObject>
#include <QVariant>
#include "protocol_item_ids.h"
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

enum ItemId {
	ItemId_CommandContainer = ItemId_Other + 50,
	ItemId_GameEventContainer = ItemId_Other + 51,
	ItemId_Command_CreateGame = ItemId_Other + 99,
	ItemId_Command_DeckUpload = ItemId_Other + 100,
	ItemId_Command_DeckSelect = ItemId_Other + 101,
	ItemId_Command_SetSideboardPlan = ItemId_Other + 102,
	ItemId_Command_MoveCard = ItemId_Other + 103,
	ItemId_Event_ListRooms = ItemId_Other + 200,
	ItemId_Event_JoinRoom = ItemId_Other + 201,
	ItemId_Event_ListGames = ItemId_Other + 203,
	ItemId_Event_UserJoined = ItemId_Other + 204,
	ItemId_Event_GameStateChanged = ItemId_Other + 205,
	ItemId_Event_PlayerPropertiesChanged = ItemId_Other + 206,
	ItemId_Event_CreateArrows = ItemId_Other + 207,
	ItemId_Event_CreateCounters = ItemId_Other + 208,
	ItemId_Event_DrawCards = ItemId_Other + 209,
	ItemId_Event_RevealCards = ItemId_Other + 210,
	ItemId_Event_Join = ItemId_Other + 211,
	ItemId_Event_Ping = ItemId_Other + 212,
	ItemId_Event_AddToList = ItemId_Other + 213,
	ItemId_Response_ListUsers = ItemId_Other + 300,
	ItemId_Response_GetGamesOfUser = ItemId_Other + 301,
	ItemId_Response_GetUserInfo = ItemId_Other + 302,
	ItemId_Response_DeckList = ItemId_Other + 303,
	ItemId_Response_DeckDownload = ItemId_Other + 304,
	ItemId_Response_DeckUpload = ItemId_Other + 305,
	ItemId_Response_DumpZone = ItemId_Other + 306,
	ItemId_Response_JoinRoom = ItemId_Other + 307,
	ItemId_Response_Login = ItemId_Other + 308,
	ItemId_Invalid = ItemId_Other + 1000
};

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

class ProtocolItem_Invalid : public ProtocolItem {
public:
	ProtocolItem_Invalid() : ProtocolItem(QString(), QString()) { }
	int getItemId() const { return ItemId_Invalid; }
};

class TopLevelProtocolItem : public SerializableItem {
	Q_OBJECT
signals:
	void protocolItemReceived(ProtocolItem *item);
private:
	ProtocolItem *currentItem;
	bool readCurrentItem(QXmlStreamReader *xml);
public:
	TopLevelProtocolItem();
	bool readElement(QXmlStreamReader *xml);
	void writeElement(QXmlStreamWriter *xml);
	bool isEmpty() const { return false; }
};

// ----------------
// --- COMMANDS ---
// ----------------
/*
class Command : public ProtocolItem {
	Q_OBJECT
signals:
	void finished(ProtocolResponse *response);
	void finished(ResponseCode response);
private:
	QVariant extraData;
public:
	Command(const QString &_itemName = QString());
	void setExtraData(const QVariant &_extraData) { extraData = _extraData; }
	QVariant getExtraData() const { return extraData; }
	void processResponse(ProtocolResponse *response);
};
*/
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
class RoomCommand : public Command {
	Q_OBJECT
public:
	RoomCommand(const QString &_cmdName, int _roomId)
		: Command(_cmdName)
	{
		insertItem(new SerializableItem_Int("room_id", _roomId));
	}
	int getRoomId() const { return static_cast<SerializableItem_Int *>(itemMap.value("room_id"))->getData(); }
};

class GameCommand : public Command {
	Q_OBJECT
public:
	GameCommand(const QString &_cmdName, int _gameId)
		: Command(_cmdName)
	{
		insertItem(new SerializableItem_Int("game_id", _gameId));
	}
	int getGameId() const { return static_cast<SerializableItem_Int *>(itemMap.value("game_id"))->getData(); }
	void setGameId(int _gameId) { static_cast<SerializableItem_Int *>(itemMap.value("game_id"))->setData(_gameId); }
};

class ModeratorCommand : public Command {
	Q_OBJECT
public:
	ModeratorCommand(const QString &_cmdName)
		: Command(_cmdName)
	{
	}
};

class AdminCommand : public Command {
	Q_OBJECT
public:
	AdminCommand(const QString &_cmdName)
		: Command(_cmdName)
	{
	}
};

class Command_DeckUpload : public Command {
	Q_OBJECT
public:
	Command_DeckUpload(DeckList *_deck = 0, const QString &_path = QString());
	static SerializableItem *newItem() { return new Command_DeckUpload; }
	int getItemId() const { return ItemId_Command_DeckUpload; }
	DeckList *getDeck() const;
	QString getPath() const { return static_cast<SerializableItem_String *>(itemMap.value("path"))->getData(); }
};

class Command_DeckSelect : public GameCommand {
	Q_OBJECT
public:
	Command_DeckSelect(int _gameId = -1, DeckList *_deck = 0, int _deckId = -1);
	static SerializableItem *newItem() { return new Command_DeckSelect; }
	int getItemId() const { return ItemId_Command_DeckSelect; }
	DeckList *getDeck() const;
	int getDeckId() const { return static_cast<SerializableItem_Int *>(itemMap.value("deck_id"))->getData(); }
};

class Command_SetSideboardPlan : public GameCommand {
	Q_OBJECT
public:
	Command_SetSideboardPlan(int _gameId = -1, const QList<MoveCardToZone *> &_moveList = QList<MoveCardToZone *>());
	static SerializableItem *newItem() { return new Command_SetSideboardPlan; }
	int getItemId() const { return ItemId_Command_SetSideboardPlan; }
	QList<MoveCardToZone *> getMoveList() const;
};

class Command_MoveCard : public GameCommand {
	Q_OBJECT
public:
	Command_MoveCard(int _gameId = -1, const QString &_startZone = QString(), const QList<CardToMove *> &_cards = QList<CardToMove *>(), int _targetPlayerId = -1, const QString &_targetZone = QString(), int _x = -1, int _y = -1);
	QString getStartZone() const { return static_cast<SerializableItem_String *>(itemMap.value("start_zone"))->getData(); }
	QList<CardToMove *> getCards() const { return typecastItemList<CardToMove *>(); }
	int getTargetPlayerId() const { return static_cast<SerializableItem_Int *>(itemMap.value("target_player_id"))->getData(); }
	QString getTargetZone() const { return static_cast<SerializableItem_String *>(itemMap.value("target_zone"))->getData(); }
	int getX() const { return static_cast<SerializableItem_Int *>(itemMap.value("x"))->getData(); }
	int getY() const { return static_cast<SerializableItem_Int *>(itemMap.value("y"))->getData(); }
	static SerializableItem *newItem() { return new Command_MoveCard; }
	int getItemId() const { return ItemId_Command_MoveCard; }
};
*/
// -----------------
// --- RESPONSES ---
// -----------------

class ProtocolResponse : public ProtocolItem {
	Q_OBJECT
private:
	static QHash<QString, ResponseCode> responseHash;
public:
	ProtocolResponse(int _cmdId = -1, ResponseCode _responseCode = RespNothing, const QString &_itemName = QString());
	int getItemId() const { return ItemId_Other; }
	static void initializeHash();
	static SerializableItem *newItem() { return new ProtocolResponse; }
	int getCmdId() const { return static_cast<SerializableItem_Int *>(itemMap.value("cmd_id"))->getData(); }
	ResponseCode getResponseCode() const { return responseHash.value(static_cast<SerializableItem_String *>(itemMap.value("response_code"))->getData(), RespOk); }
};

class Response_JoinRoom : public ProtocolResponse {
	Q_OBJECT
public:
	Response_JoinRoom(int _cmdId = -1, ResponseCode _responseCode = RespOk, ServerInfo_Room *_roomInfo = 0);
	int getItemId() const { return ItemId_Response_JoinRoom; }
	static SerializableItem *newItem() { return new Response_JoinRoom; }
	ServerInfo_Room *getRoomInfo() const { return static_cast<ServerInfo_Room *>(itemMap.value("room")); }
};

class Response_ListUsers : public ProtocolResponse {
	Q_OBJECT
public:
	Response_ListUsers(int _cmdId = -1, ResponseCode _responseCode = RespOk, const QList<ServerInfo_User *> &_userList = QList<ServerInfo_User *>());
	int getItemId() const { return ItemId_Response_ListUsers; }
	static SerializableItem *newItem() { return new Response_ListUsers; }
	QList<ServerInfo_User *> getUserList() const { return typecastItemList<ServerInfo_User *>(); }
};

class Response_GetGamesOfUser : public ProtocolResponse {
	Q_OBJECT
private:
	QList<ServerInfo_Game *> gameList;
	QList<ServerInfo_Room *> roomList;
protected:
	void extractData();
public:
	Response_GetGamesOfUser(int _cmdId = -1, ResponseCode _responseCode = RespOk, const QList<ServerInfo_Room *> &_roomList = QList<ServerInfo_Room *>(), const QList<ServerInfo_Game *> &_gameList = QList<ServerInfo_Game *>());
	int getItemId() const { return ItemId_Response_GetGamesOfUser; }
	static SerializableItem *newItem() { return new Response_GetGamesOfUser; }
	QList<ServerInfo_Room *> getRoomList() const { return roomList; }
	QList<ServerInfo_Game *> getGameList() const { return gameList; }
};

class Response_GetUserInfo : public ProtocolResponse {
	Q_OBJECT
public:
	Response_GetUserInfo(int _cmdId = -1, ResponseCode _responseCode = RespOk, ServerInfo_User *_userInfo = 0);
	int getItemId() const { return ItemId_Response_GetUserInfo; }
	static SerializableItem *newItem() { return new Response_GetUserInfo; }
	ServerInfo_User *getUserInfo() const { return static_cast<ServerInfo_User *>(itemMap.value("user")); }
};

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

class Response_DumpZone : public ProtocolResponse {
	Q_OBJECT
public:
	Response_DumpZone(int _cmdId = -1, ResponseCode _responseCode = RespOk, ServerInfo_Zone *zone = 0);
	int getItemId() const { return ItemId_Response_DumpZone; }
	static SerializableItem *newItem() { return new Response_DumpZone; }
	ServerInfo_Zone *getZone() const { return static_cast<ServerInfo_Zone *>(itemMap.value("zone")); }
};

class Response_Login : public ProtocolResponse {
	Q_OBJECT
public:
	Response_Login(int _cmdId = -1, ResponseCode _responseCode = RespOk, ServerInfo_User *_userInfo = 0, const QList<ServerInfo_User *> &_buddyList = QList<ServerInfo_User *>(), const QList<ServerInfo_User *> &_ignoreList = QList<ServerInfo_User *>());
	int getItemId() const { return ItemId_Response_Login; }
	static SerializableItem *newItem() { return new Response_Login; }
	ServerInfo_User *getUserInfo() const { return static_cast<ServerInfo_User *>(itemMap.value("user")); }
	QList<ServerInfo_User *> getBuddyList() const { return static_cast<ServerInfo_UserList *>(itemMap.value("buddy_list"))->getUserList(); }
	QList<ServerInfo_User *> getIgnoreList() const { return static_cast<ServerInfo_UserList *>(itemMap.value("ignore_list"))->getUserList(); }
};

// --------------
// --- EVENTS ---
// --------------

class GenericEvent : public ProtocolItem {
	Q_OBJECT
public:
	GenericEvent(const QString &_eventName)
		: ProtocolItem("generic_event", _eventName) { }
};

class GameEvent : public ProtocolItem {
	Q_OBJECT
public:
	GameEvent(const QString &_eventName, int _playerId);
	int getPlayerId() const { return static_cast<SerializableItem_Int *>(itemMap.value("player_id"))->getData(); }
};

class GameEventContext : public ProtocolItem {
	Q_OBJECT
public:
	GameEventContext(const QString &_contextName);
};

class GameEventContainer : public ProtocolItem {
	Q_OBJECT
private:
	QList<GameEvent *> eventList;
	GameEventContext *context;
protected:
	void extractData();
public:
	GameEventContainer(const QList<GameEvent *> &_eventList = QList<GameEvent *>(), int _gameId = -1, GameEventContext *_context = 0);
	static SerializableItem *newItem() { return new GameEventContainer; }
	int getItemId() const { return ItemId_GameEventContainer; }
	QList<GameEvent *> getEventList() const { return eventList; }
	GameEventContext *getContext() const { return context; }
	void setContext(GameEventContext *_context);
	void addGameEvent(GameEvent *event);
	static GameEventContainer *makeNew(GameEvent *event, int _gameId);

	int getGameId() const { return static_cast<SerializableItem_Int *>(itemMap.value("game_id"))->getData(); }
	void setGameId(int _gameId) { static_cast<SerializableItem_Int *>(itemMap.value("game_id"))->setData(_gameId); }
};

class RoomEvent : public ProtocolItem {
	Q_OBJECT
public:
	RoomEvent(const QString &_eventName, int _roomId);
	int getRoomId() const { return static_cast<SerializableItem_Int *>(itemMap.value("room_id"))->getData(); }
};

class Event_ListRooms : public GenericEvent {
	Q_OBJECT
public:
	Event_ListRooms(const QList<ServerInfo_Room *> &_roomList = QList<ServerInfo_Room *>());
	int getItemId() const { return ItemId_Event_ListRooms; }
	static SerializableItem *newItem() { return new Event_ListRooms; }
	QList<ServerInfo_Room *> getRoomList() const { return typecastItemList<ServerInfo_Room *>(); }
};

class Event_JoinRoom : public RoomEvent {
	Q_OBJECT
public:
	Event_JoinRoom(int _roomId = -1, ServerInfo_User *_info = 0);
	int getItemId() const { return ItemId_Event_JoinRoom; }
	static SerializableItem *newItem() { return new Event_JoinRoom; }
	ServerInfo_User *getUserInfo() const { return static_cast<ServerInfo_User *>(itemMap.value("user")); }
};

class Event_ListGames : public RoomEvent {
	Q_OBJECT
public:
	Event_ListGames(int _roomId = -1, const QList<ServerInfo_Game *> &_gameList = QList<ServerInfo_Game *>());
	int getItemId() const { return ItemId_Event_ListGames; }
	static SerializableItem *newItem() { return new Event_ListGames; }
	QList<ServerInfo_Game *> getGameList() const { return typecastItemList<ServerInfo_Game *>(); }
};

class Event_AddToList : public GenericEvent {
	Q_OBJECT
public:
	Event_AddToList(const QString &_list = QString(), ServerInfo_User *_userInfo = 0);
	int getItemId() const { return ItemId_Event_AddToList; }
	static SerializableItem *newItem() { return new Event_AddToList; }
	ServerInfo_User *getUserInfo() const { return static_cast<ServerInfo_User *>(itemMap.value("user")); }
	QString getList() const { return static_cast<SerializableItem_String *>(itemMap.value("list"))->getData(); }
};

class Event_UserJoined : public GenericEvent {
	Q_OBJECT
public:
	Event_UserJoined(ServerInfo_User *_userInfo = 0);
	int getItemId() const { return ItemId_Event_UserJoined; }
	static SerializableItem *newItem() { return new Event_UserJoined; }
	ServerInfo_User *getUserInfo() const { return static_cast<ServerInfo_User *>(itemMap.value("user")); }
};

class Event_Join : public GameEvent {
	Q_OBJECT
public:
	Event_Join(ServerInfo_PlayerProperties *player = 0);
	static SerializableItem *newItem() { return new Event_Join; }
	int getItemId() const { return ItemId_Event_Join; }
	ServerInfo_PlayerProperties *getPlayer() const { return static_cast<ServerInfo_PlayerProperties *>(itemMap.value("player_properties")); }
};

class Event_GameStateChanged : public GameEvent {
	Q_OBJECT
public:
	Event_GameStateChanged(bool _gameStarted = false, int _activePlayer = -1, int _activePhase = -1, const QList<ServerInfo_Player *> &_playerList = QList<ServerInfo_Player *>());
	static SerializableItem *newItem() { return new Event_GameStateChanged; }
	int getItemId() const { return ItemId_Event_GameStateChanged; }
	QList<ServerInfo_Player *> getPlayerList() const { return typecastItemList<ServerInfo_Player *>(); }
	bool getGameStarted() const { return static_cast<SerializableItem_Bool *>(itemMap.value("game_started"))->getData(); }
	int getActivePlayer() const { return static_cast<SerializableItem_Int *>(itemMap.value("active_player"))->getData(); }
	int getActivePhase() const { return static_cast<SerializableItem_Int *>(itemMap.value("active_phase"))->getData(); }
};

class Event_PlayerPropertiesChanged : public GameEvent {
	Q_OBJECT
public:
	Event_PlayerPropertiesChanged(int _playerId = -1, ServerInfo_PlayerProperties *_properties = 0);
	static SerializableItem *newItem() { return new Event_PlayerPropertiesChanged; }
	int getItemId() const { return ItemId_Event_PlayerPropertiesChanged; }
	ServerInfo_PlayerProperties *getProperties() const { return static_cast<ServerInfo_PlayerProperties *>(itemMap.value("player_properties")); }
};

class Event_Ping : public GameEvent {
	Q_OBJECT
public:
	Event_Ping(int _secondsElapsed = -1, const QList<ServerInfo_PlayerPing *> &_pingList = QList<ServerInfo_PlayerPing *>());
	static SerializableItem *newItem() { return new Event_Ping; }
	int getItemId() const { return ItemId_Event_Ping; }
	int getSecondsElapsed() const { return static_cast<SerializableItem_Int *>(itemMap.value("seconds_elapsed"))->getData(); }
	QList<ServerInfo_PlayerPing *> getPingList() const { return typecastItemList<ServerInfo_PlayerPing *>(); }
};

class Event_CreateArrows : public GameEvent {
	Q_OBJECT
public:
	Event_CreateArrows(int _playerId = -1, const QList<ServerInfo_Arrow *> &_arrowList = QList<ServerInfo_Arrow *>());
	int getItemId() const { return ItemId_Event_CreateArrows; }
	static SerializableItem *newItem() { return new Event_CreateArrows; }
	QList<ServerInfo_Arrow *> getArrowList() const { return typecastItemList<ServerInfo_Arrow *>(); }
};

class Event_CreateCounters : public GameEvent {
	Q_OBJECT
public:
	Event_CreateCounters(int _playerId = -1, const QList<ServerInfo_Counter *> &_counterList = QList<ServerInfo_Counter *>());
	int getItemId() const { return ItemId_Event_CreateCounters; }
	static SerializableItem *newItem() { return new Event_CreateCounters; }
	QList<ServerInfo_Counter *> getCounterList() const { return typecastItemList<ServerInfo_Counter *>(); }
};

class Event_DrawCards : public GameEvent {
	Q_OBJECT
public:
	Event_DrawCards(int _playerId = -1, int numberCards = -1, const QList<ServerInfo_Card *> &_cardList = QList<ServerInfo_Card *>());
	int getItemId() const { return ItemId_Event_DrawCards; }
	static SerializableItem *newItem() { return new Event_DrawCards; }
	int getNumberCards() const { return static_cast<SerializableItem_Int *>(itemMap.value("number_cards"))->getData(); }
	QList<ServerInfo_Card *> getCardList() const { return typecastItemList<ServerInfo_Card *>(); }
};

class Event_RevealCards : public GameEvent {
	Q_OBJECT
public:
	Event_RevealCards(int _playerId = -1, const QString &_zoneName = QString(), int cardId = -1, int _otherPlayerId = -1, const QList<ServerInfo_Card *> &_cardList = QList<ServerInfo_Card *>());
	int getItemId() const { return ItemId_Event_RevealCards; }
	static SerializableItem *newItem() { return new Event_RevealCards; }
	QString getZoneName() const { return static_cast<SerializableItem_String *>(itemMap.value("zone_name"))->getData(); }
	int getCardId() const { return static_cast<SerializableItem_Int *>(itemMap.value("card_id"))->getData(); }
	int getOtherPlayerId() const { return static_cast<SerializableItem_Int *>(itemMap.value("other_player_id"))->getData(); }
	QList<ServerInfo_Card *> getCardList() const { return typecastItemList<ServerInfo_Card *>(); }
};

#endif
