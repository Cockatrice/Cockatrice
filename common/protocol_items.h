#ifndef PROTOCOL_ITEMS_H
#define PROTOCOL_ITEMS_H

#include "protocol.h"

class Event_ConnectionStateChanged : public GameEvent {
	Q_OBJECT
public:
	Event_ConnectionStateChanged(int _playerId = -1, bool _connected = false);
	bool getConnected() const { return static_cast<SerializableItem_Bool *>(itemMap.value("connected"))->getData(); };
	static SerializableItem *newItem() { return new Event_ConnectionStateChanged; }
	int getItemId() const { return ItemId_Event_ConnectionStateChanged; }
};
class Event_Say : public GameEvent {
	Q_OBJECT
public:
	Event_Say(int _playerId = -1, const QString &_message = QString());
	QString getMessage() const { return static_cast<SerializableItem_String *>(itemMap.value("message"))->getData(); };
	static SerializableItem *newItem() { return new Event_Say; }
	int getItemId() const { return ItemId_Event_Say; }
};
class Event_Leave : public GameEvent {
	Q_OBJECT
public:
	Event_Leave(int _playerId = -1);
	static SerializableItem *newItem() { return new Event_Leave; }
	int getItemId() const { return ItemId_Event_Leave; }
};
class Event_GameClosed : public GameEvent {
	Q_OBJECT
public:
	Event_GameClosed(int _playerId = -1);
	static SerializableItem *newItem() { return new Event_GameClosed; }
	int getItemId() const { return ItemId_Event_GameClosed; }
};
class Event_GameHostChanged : public GameEvent {
	Q_OBJECT
public:
	Event_GameHostChanged(int _playerId = -1);
	static SerializableItem *newItem() { return new Event_GameHostChanged; }
	int getItemId() const { return ItemId_Event_GameHostChanged; }
};
class Event_Kicked : public GameEvent {
	Q_OBJECT
public:
	Event_Kicked(int _playerId = -1);
	static SerializableItem *newItem() { return new Event_Kicked; }
	int getItemId() const { return ItemId_Event_Kicked; }
};
class Event_Shuffle : public GameEvent {
	Q_OBJECT
public:
	Event_Shuffle(int _playerId = -1);
	static SerializableItem *newItem() { return new Event_Shuffle; }
	int getItemId() const { return ItemId_Event_Shuffle; }
};
class Event_RollDie : public GameEvent {
	Q_OBJECT
public:
	Event_RollDie(int _playerId = -1, int _sides = -1, int _value = -1);
	int getSides() const { return static_cast<SerializableItem_Int *>(itemMap.value("sides"))->getData(); };
	int getValue() const { return static_cast<SerializableItem_Int *>(itemMap.value("value"))->getData(); };
	static SerializableItem *newItem() { return new Event_RollDie; }
	int getItemId() const { return ItemId_Event_RollDie; }
};
class Event_MoveCard : public GameEvent {
	Q_OBJECT
public:
	Event_MoveCard(int _playerId = -1, int _cardId = -1, const QString &_cardName = QString(), const QString &_startZone = QString(), int _position = -1, int _targetPlayerId = -1, const QString &_targetZone = QString(), int _x = -1, int _y = -1, int _newCardId = -1, bool _faceDown = false);
	int getCardId() const { return static_cast<SerializableItem_Int *>(itemMap.value("card_id"))->getData(); };
	QString getCardName() const { return static_cast<SerializableItem_String *>(itemMap.value("card_name"))->getData(); };
	QString getStartZone() const { return static_cast<SerializableItem_String *>(itemMap.value("start_zone"))->getData(); };
	int getPosition() const { return static_cast<SerializableItem_Int *>(itemMap.value("position"))->getData(); };
	int getTargetPlayerId() const { return static_cast<SerializableItem_Int *>(itemMap.value("target_player_id"))->getData(); };
	QString getTargetZone() const { return static_cast<SerializableItem_String *>(itemMap.value("target_zone"))->getData(); };
	int getX() const { return static_cast<SerializableItem_Int *>(itemMap.value("x"))->getData(); };
	int getY() const { return static_cast<SerializableItem_Int *>(itemMap.value("y"))->getData(); };
	int getNewCardId() const { return static_cast<SerializableItem_Int *>(itemMap.value("new_card_id"))->getData(); };
	bool getFaceDown() const { return static_cast<SerializableItem_Bool *>(itemMap.value("face_down"))->getData(); };
	static SerializableItem *newItem() { return new Event_MoveCard; }
	int getItemId() const { return ItemId_Event_MoveCard; }
};
class Event_FlipCard : public GameEvent {
	Q_OBJECT
public:
	Event_FlipCard(int _playerId = -1, const QString &_zone = QString(), int _cardId = -1, const QString &_cardName = QString(), bool _faceDown = false);
	QString getZone() const { return static_cast<SerializableItem_String *>(itemMap.value("zone"))->getData(); };
	int getCardId() const { return static_cast<SerializableItem_Int *>(itemMap.value("card_id"))->getData(); };
	QString getCardName() const { return static_cast<SerializableItem_String *>(itemMap.value("card_name"))->getData(); };
	bool getFaceDown() const { return static_cast<SerializableItem_Bool *>(itemMap.value("face_down"))->getData(); };
	static SerializableItem *newItem() { return new Event_FlipCard; }
	int getItemId() const { return ItemId_Event_FlipCard; }
};
class Event_DestroyCard : public GameEvent {
	Q_OBJECT
public:
	Event_DestroyCard(int _playerId = -1, const QString &_zone = QString(), int _cardId = -1);
	QString getZone() const { return static_cast<SerializableItem_String *>(itemMap.value("zone"))->getData(); };
	int getCardId() const { return static_cast<SerializableItem_Int *>(itemMap.value("card_id"))->getData(); };
	static SerializableItem *newItem() { return new Event_DestroyCard; }
	int getItemId() const { return ItemId_Event_DestroyCard; }
};
class Event_AttachCard : public GameEvent {
	Q_OBJECT
public:
	Event_AttachCard(int _playerId = -1, const QString &_startZone = QString(), int _cardId = -1, int _targetPlayerId = -1, const QString &_targetZone = QString(), int _targetCardId = -1);
	QString getStartZone() const { return static_cast<SerializableItem_String *>(itemMap.value("start_zone"))->getData(); };
	int getCardId() const { return static_cast<SerializableItem_Int *>(itemMap.value("card_id"))->getData(); };
	int getTargetPlayerId() const { return static_cast<SerializableItem_Int *>(itemMap.value("target_player_id"))->getData(); };
	QString getTargetZone() const { return static_cast<SerializableItem_String *>(itemMap.value("target_zone"))->getData(); };
	int getTargetCardId() const { return static_cast<SerializableItem_Int *>(itemMap.value("target_card_id"))->getData(); };
	static SerializableItem *newItem() { return new Event_AttachCard; }
	int getItemId() const { return ItemId_Event_AttachCard; }
};
class Event_CreateToken : public GameEvent {
	Q_OBJECT
public:
	Event_CreateToken(int _playerId = -1, const QString &_zone = QString(), int _cardId = -1, const QString &_cardName = QString(), const QString &_color = QString(), const QString &_pt = QString(), const QString &_annotation = QString(), bool _destroyOnZoneChange = false, int _x = -1, int _y = -1);
	QString getZone() const { return static_cast<SerializableItem_String *>(itemMap.value("zone"))->getData(); };
	int getCardId() const { return static_cast<SerializableItem_Int *>(itemMap.value("card_id"))->getData(); };
	QString getCardName() const { return static_cast<SerializableItem_String *>(itemMap.value("card_name"))->getData(); };
	QString getColor() const { return static_cast<SerializableItem_String *>(itemMap.value("color"))->getData(); };
	QString getPt() const { return static_cast<SerializableItem_String *>(itemMap.value("pt"))->getData(); };
	QString getAnnotation() const { return static_cast<SerializableItem_String *>(itemMap.value("annotation"))->getData(); };
	bool getDestroyOnZoneChange() const { return static_cast<SerializableItem_Bool *>(itemMap.value("destroy_on_zone_change"))->getData(); };
	int getX() const { return static_cast<SerializableItem_Int *>(itemMap.value("x"))->getData(); };
	int getY() const { return static_cast<SerializableItem_Int *>(itemMap.value("y"))->getData(); };
	static SerializableItem *newItem() { return new Event_CreateToken; }
	int getItemId() const { return ItemId_Event_CreateToken; }
};
class Event_DeleteArrow : public GameEvent {
	Q_OBJECT
public:
	Event_DeleteArrow(int _playerId = -1, int _arrowId = -1);
	int getArrowId() const { return static_cast<SerializableItem_Int *>(itemMap.value("arrow_id"))->getData(); };
	static SerializableItem *newItem() { return new Event_DeleteArrow; }
	int getItemId() const { return ItemId_Event_DeleteArrow; }
};
class Event_SetCardAttr : public GameEvent {
	Q_OBJECT
public:
	Event_SetCardAttr(int _playerId = -1, const QString &_zone = QString(), int _cardId = -1, const QString &_attrName = QString(), const QString &_attrValue = QString());
	QString getZone() const { return static_cast<SerializableItem_String *>(itemMap.value("zone"))->getData(); };
	int getCardId() const { return static_cast<SerializableItem_Int *>(itemMap.value("card_id"))->getData(); };
	QString getAttrName() const { return static_cast<SerializableItem_String *>(itemMap.value("attr_name"))->getData(); };
	QString getAttrValue() const { return static_cast<SerializableItem_String *>(itemMap.value("attr_value"))->getData(); };
	static SerializableItem *newItem() { return new Event_SetCardAttr; }
	int getItemId() const { return ItemId_Event_SetCardAttr; }
};
class Event_SetCardCounter : public GameEvent {
	Q_OBJECT
public:
	Event_SetCardCounter(int _playerId = -1, const QString &_zone = QString(), int _cardId = -1, int _counterId = -1, int _counterValue = -1);
	QString getZone() const { return static_cast<SerializableItem_String *>(itemMap.value("zone"))->getData(); };
	int getCardId() const { return static_cast<SerializableItem_Int *>(itemMap.value("card_id"))->getData(); };
	int getCounterId() const { return static_cast<SerializableItem_Int *>(itemMap.value("counter_id"))->getData(); };
	int getCounterValue() const { return static_cast<SerializableItem_Int *>(itemMap.value("counter_value"))->getData(); };
	static SerializableItem *newItem() { return new Event_SetCardCounter; }
	int getItemId() const { return ItemId_Event_SetCardCounter; }
};
class Event_SetCounter : public GameEvent {
	Q_OBJECT
public:
	Event_SetCounter(int _playerId = -1, int _counterId = -1, int _value = -1);
	int getCounterId() const { return static_cast<SerializableItem_Int *>(itemMap.value("counter_id"))->getData(); };
	int getValue() const { return static_cast<SerializableItem_Int *>(itemMap.value("value"))->getData(); };
	static SerializableItem *newItem() { return new Event_SetCounter; }
	int getItemId() const { return ItemId_Event_SetCounter; }
};
class Event_DelCounter : public GameEvent {
	Q_OBJECT
public:
	Event_DelCounter(int _playerId = -1, int _counterId = -1);
	int getCounterId() const { return static_cast<SerializableItem_Int *>(itemMap.value("counter_id"))->getData(); };
	static SerializableItem *newItem() { return new Event_DelCounter; }
	int getItemId() const { return ItemId_Event_DelCounter; }
};
class Event_SetActivePlayer : public GameEvent {
	Q_OBJECT
public:
	Event_SetActivePlayer(int _playerId = -1, int _activePlayerId = -1);
	int getActivePlayerId() const { return static_cast<SerializableItem_Int *>(itemMap.value("active_player_id"))->getData(); };
	static SerializableItem *newItem() { return new Event_SetActivePlayer; }
	int getItemId() const { return ItemId_Event_SetActivePlayer; }
};
class Event_SetActivePhase : public GameEvent {
	Q_OBJECT
public:
	Event_SetActivePhase(int _playerId = -1, int _phase = -1);
	int getPhase() const { return static_cast<SerializableItem_Int *>(itemMap.value("phase"))->getData(); };
	static SerializableItem *newItem() { return new Event_SetActivePhase; }
	int getItemId() const { return ItemId_Event_SetActivePhase; }
};
class Event_DumpZone : public GameEvent {
	Q_OBJECT
public:
	Event_DumpZone(int _playerId = -1, int _zoneOwnerId = -1, const QString &_zone = QString(), int _numberCards = -1);
	int getZoneOwnerId() const { return static_cast<SerializableItem_Int *>(itemMap.value("zone_owner_id"))->getData(); };
	QString getZone() const { return static_cast<SerializableItem_String *>(itemMap.value("zone"))->getData(); };
	int getNumberCards() const { return static_cast<SerializableItem_Int *>(itemMap.value("number_cards"))->getData(); };
	static SerializableItem *newItem() { return new Event_DumpZone; }
	int getItemId() const { return ItemId_Event_DumpZone; }
};
class Event_StopDumpZone : public GameEvent {
	Q_OBJECT
public:
	Event_StopDumpZone(int _playerId = -1, int _zoneOwnerId = -1, const QString &_zone = QString());
	int getZoneOwnerId() const { return static_cast<SerializableItem_Int *>(itemMap.value("zone_owner_id"))->getData(); };
	QString getZone() const { return static_cast<SerializableItem_String *>(itemMap.value("zone"))->getData(); };
	static SerializableItem *newItem() { return new Event_StopDumpZone; }
	int getItemId() const { return ItemId_Event_StopDumpZone; }
};
class Event_RemoveFromList : public GenericEvent {
	Q_OBJECT
public:
	Event_RemoveFromList(const QString &_list = QString(), const QString &_userName = QString());
	QString getList() const { return static_cast<SerializableItem_String *>(itemMap.value("list"))->getData(); };
	QString getUserName() const { return static_cast<SerializableItem_String *>(itemMap.value("user_name"))->getData(); };
	static SerializableItem *newItem() { return new Event_RemoveFromList; }
	int getItemId() const { return ItemId_Event_RemoveFromList; }
};
class Event_ServerMessage : public GenericEvent {
	Q_OBJECT
public:
	Event_ServerMessage(const QString &_message = QString());
	QString getMessage() const { return static_cast<SerializableItem_String *>(itemMap.value("message"))->getData(); };
	static SerializableItem *newItem() { return new Event_ServerMessage; }
	int getItemId() const { return ItemId_Event_ServerMessage; }
};
class Event_ServerShutdown : public GenericEvent {
	Q_OBJECT
public:
	Event_ServerShutdown(const QString &_reason = QString(), int _minutes = -1);
	QString getReason() const { return static_cast<SerializableItem_String *>(itemMap.value("reason"))->getData(); };
	int getMinutes() const { return static_cast<SerializableItem_Int *>(itemMap.value("minutes"))->getData(); };
	static SerializableItem *newItem() { return new Event_ServerShutdown; }
	int getItemId() const { return ItemId_Event_ServerShutdown; }
};
class Event_ConnectionClosed : public GenericEvent {
	Q_OBJECT
public:
	Event_ConnectionClosed(const QString &_reason = QString());
	QString getReason() const { return static_cast<SerializableItem_String *>(itemMap.value("reason"))->getData(); };
	static SerializableItem *newItem() { return new Event_ConnectionClosed; }
	int getItemId() const { return ItemId_Event_ConnectionClosed; }
};
class Event_Message : public GenericEvent {
	Q_OBJECT
public:
	Event_Message(const QString &_senderName = QString(), const QString &_receiverName = QString(), const QString &_text = QString());
	QString getSenderName() const { return static_cast<SerializableItem_String *>(itemMap.value("sender_name"))->getData(); };
	QString getReceiverName() const { return static_cast<SerializableItem_String *>(itemMap.value("receiver_name"))->getData(); };
	QString getText() const { return static_cast<SerializableItem_String *>(itemMap.value("text"))->getData(); };
	static SerializableItem *newItem() { return new Event_Message; }
	int getItemId() const { return ItemId_Event_Message; }
};
class Event_GameJoined : public GenericEvent {
	Q_OBJECT
public:
	Event_GameJoined(int _gameId = -1, const QString &_gameDescription = QString(), int _hostId = -1, int _playerId = -1, bool _spectator = false, bool _spectatorsCanTalk = false, bool _spectatorsSeeEverything = false, bool _resuming = false);
	int getGameId() const { return static_cast<SerializableItem_Int *>(itemMap.value("game_id"))->getData(); };
	QString getGameDescription() const { return static_cast<SerializableItem_String *>(itemMap.value("game_description"))->getData(); };
	int getHostId() const { return static_cast<SerializableItem_Int *>(itemMap.value("host_id"))->getData(); };
	int getPlayerId() const { return static_cast<SerializableItem_Int *>(itemMap.value("player_id"))->getData(); };
	bool getSpectator() const { return static_cast<SerializableItem_Bool *>(itemMap.value("spectator"))->getData(); };
	bool getSpectatorsCanTalk() const { return static_cast<SerializableItem_Bool *>(itemMap.value("spectators_can_talk"))->getData(); };
	bool getSpectatorsSeeEverything() const { return static_cast<SerializableItem_Bool *>(itemMap.value("spectators_see_everything"))->getData(); };
	bool getResuming() const { return static_cast<SerializableItem_Bool *>(itemMap.value("resuming"))->getData(); };
	static SerializableItem *newItem() { return new Event_GameJoined; }
	int getItemId() const { return ItemId_Event_GameJoined; }
};
class Event_UserLeft : public GenericEvent {
	Q_OBJECT
public:
	Event_UserLeft(const QString &_userName = QString());
	QString getUserName() const { return static_cast<SerializableItem_String *>(itemMap.value("user_name"))->getData(); };
	static SerializableItem *newItem() { return new Event_UserLeft; }
	int getItemId() const { return ItemId_Event_UserLeft; }
};
class Event_LeaveRoom : public RoomEvent {
	Q_OBJECT
public:
	Event_LeaveRoom(int _roomId = -1, const QString &_playerName = QString());
	QString getPlayerName() const { return static_cast<SerializableItem_String *>(itemMap.value("player_name"))->getData(); };
	static SerializableItem *newItem() { return new Event_LeaveRoom; }
	int getItemId() const { return ItemId_Event_LeaveRoom; }
};
class Event_RoomSay : public RoomEvent {
	Q_OBJECT
public:
	Event_RoomSay(int _roomId = -1, const QString &_playerName = QString(), const QString &_message = QString());
	QString getPlayerName() const { return static_cast<SerializableItem_String *>(itemMap.value("player_name"))->getData(); };
	QString getMessage() const { return static_cast<SerializableItem_String *>(itemMap.value("message"))->getData(); };
	static SerializableItem *newItem() { return new Event_RoomSay; }
	int getItemId() const { return ItemId_Event_RoomSay; }
};
class Context_ReadyStart : public GameEventContext {
	Q_OBJECT
public:
	Context_ReadyStart();
	static SerializableItem *newItem() { return new Context_ReadyStart; }
	int getItemId() const { return ItemId_Context_ReadyStart; }
};
class Context_Concede : public GameEventContext {
	Q_OBJECT
public:
	Context_Concede();
	static SerializableItem *newItem() { return new Context_Concede; }
	int getItemId() const { return ItemId_Context_Concede; }
};
class Context_DeckSelect : public GameEventContext {
	Q_OBJECT
public:
	Context_DeckSelect(const QString &_deckHash = QString());
	QString getDeckHash() const { return static_cast<SerializableItem_String *>(itemMap.value("deck_hash"))->getData(); };
	static SerializableItem *newItem() { return new Context_DeckSelect; }
	int getItemId() const { return ItemId_Context_DeckSelect; }
};
class Context_UndoDraw : public GameEventContext {
	Q_OBJECT
public:
	Context_UndoDraw();
	static SerializableItem *newItem() { return new Context_UndoDraw; }
	int getItemId() const { return ItemId_Context_UndoDraw; }
};
class Context_MoveCard : public GameEventContext {
	Q_OBJECT
public:
	Context_MoveCard();
	static SerializableItem *newItem() { return new Context_MoveCard; }
	int getItemId() const { return ItemId_Context_MoveCard; }
};
class Context_Mulligan : public GameEventContext {
	Q_OBJECT
public:
	Context_Mulligan(int _number = -1);
	int getNumber() const { return static_cast<SerializableItem_Int *>(itemMap.value("number"))->getData(); };
	static SerializableItem *newItem() { return new Context_Mulligan; }
	int getItemId() const { return ItemId_Context_Mulligan; }
};

#endif
