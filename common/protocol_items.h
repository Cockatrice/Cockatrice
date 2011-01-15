#ifndef PROTOCOL_ITEMS_H
#define PROTOCOL_ITEMS_H

#include "protocol.h"

class Command_Ping : public Command {
	Q_OBJECT
public:
	Command_Ping();
	static SerializableItem *newItem() { return new Command_Ping; }
	int getItemId() const { return ItemId_Command_Ping; }
};
class Command_Login : public Command {
	Q_OBJECT
public:
	Command_Login(const QString &_username = QString(), const QString &_password = QString());
	QString getUsername() const { return static_cast<SerializableItem_String *>(itemMap.value("username"))->getData(); };
	QString getPassword() const { return static_cast<SerializableItem_String *>(itemMap.value("password"))->getData(); };
	static SerializableItem *newItem() { return new Command_Login; }
	int getItemId() const { return ItemId_Command_Login; }
};
class Command_Message : public Command {
	Q_OBJECT
public:
	Command_Message(const QString &_userName = QString(), const QString &_text = QString());
	QString getUserName() const { return static_cast<SerializableItem_String *>(itemMap.value("user_name"))->getData(); };
	QString getText() const { return static_cast<SerializableItem_String *>(itemMap.value("text"))->getData(); };
	static SerializableItem *newItem() { return new Command_Message; }
	int getItemId() const { return ItemId_Command_Message; }
};
class Command_ListUsers : public Command {
	Q_OBJECT
public:
	Command_ListUsers();
	static SerializableItem *newItem() { return new Command_ListUsers; }
	int getItemId() const { return ItemId_Command_ListUsers; }
};
class Command_GetUserInfo : public Command {
	Q_OBJECT
public:
	Command_GetUserInfo(const QString &_userName = QString());
	QString getUserName() const { return static_cast<SerializableItem_String *>(itemMap.value("user_name"))->getData(); };
	static SerializableItem *newItem() { return new Command_GetUserInfo; }
	int getItemId() const { return ItemId_Command_GetUserInfo; }
};
class Command_DeckList : public Command {
	Q_OBJECT
public:
	Command_DeckList();
	static SerializableItem *newItem() { return new Command_DeckList; }
	int getItemId() const { return ItemId_Command_DeckList; }
};
class Command_DeckNewDir : public Command {
	Q_OBJECT
public:
	Command_DeckNewDir(const QString &_path = QString(), const QString &_dirName = QString());
	QString getPath() const { return static_cast<SerializableItem_String *>(itemMap.value("path"))->getData(); };
	QString getDirName() const { return static_cast<SerializableItem_String *>(itemMap.value("dir_name"))->getData(); };
	static SerializableItem *newItem() { return new Command_DeckNewDir; }
	int getItemId() const { return ItemId_Command_DeckNewDir; }
};
class Command_DeckDelDir : public Command {
	Q_OBJECT
public:
	Command_DeckDelDir(const QString &_path = QString());
	QString getPath() const { return static_cast<SerializableItem_String *>(itemMap.value("path"))->getData(); };
	static SerializableItem *newItem() { return new Command_DeckDelDir; }
	int getItemId() const { return ItemId_Command_DeckDelDir; }
};
class Command_DeckDel : public Command {
	Q_OBJECT
public:
	Command_DeckDel(int _deckId = -1);
	int getDeckId() const { return static_cast<SerializableItem_Int *>(itemMap.value("deck_id"))->getData(); };
	static SerializableItem *newItem() { return new Command_DeckDel; }
	int getItemId() const { return ItemId_Command_DeckDel; }
};
class Command_DeckDownload : public Command {
	Q_OBJECT
public:
	Command_DeckDownload(int _deckId = -1);
	int getDeckId() const { return static_cast<SerializableItem_Int *>(itemMap.value("deck_id"))->getData(); };
	static SerializableItem *newItem() { return new Command_DeckDownload; }
	int getItemId() const { return ItemId_Command_DeckDownload; }
};
class Command_ListRooms : public Command {
	Q_OBJECT
public:
	Command_ListRooms();
	static SerializableItem *newItem() { return new Command_ListRooms; }
	int getItemId() const { return ItemId_Command_ListRooms; }
};
class Command_JoinRoom : public Command {
	Q_OBJECT
public:
	Command_JoinRoom(int _roomId = -1);
	int getRoomId() const { return static_cast<SerializableItem_Int *>(itemMap.value("room_id"))->getData(); };
	static SerializableItem *newItem() { return new Command_JoinRoom; }
	int getItemId() const { return ItemId_Command_JoinRoom; }
};
class Command_LeaveRoom : public RoomCommand {
	Q_OBJECT
public:
	Command_LeaveRoom(int _roomId = -1);
	static SerializableItem *newItem() { return new Command_LeaveRoom; }
	int getItemId() const { return ItemId_Command_LeaveRoom; }
};
class Command_RoomSay : public RoomCommand {
	Q_OBJECT
public:
	Command_RoomSay(int _roomId = -1, const QString &_message = QString());
	QString getMessage() const { return static_cast<SerializableItem_String *>(itemMap.value("message"))->getData(); };
	static SerializableItem *newItem() { return new Command_RoomSay; }
	int getItemId() const { return ItemId_Command_RoomSay; }
};
class Command_CreateGame : public RoomCommand {
	Q_OBJECT
public:
	Command_CreateGame(int _roomId = -1, const QString &_description = QString(), const QString &_password = QString(), int _maxPlayers = -1, bool _spectatorsAllowed = false, bool _spectatorsNeedPassword = false, bool _spectatorsCanTalk = false, bool _spectatorsSeeEverything = false);
	QString getDescription() const { return static_cast<SerializableItem_String *>(itemMap.value("description"))->getData(); };
	QString getPassword() const { return static_cast<SerializableItem_String *>(itemMap.value("password"))->getData(); };
	int getMaxPlayers() const { return static_cast<SerializableItem_Int *>(itemMap.value("max_players"))->getData(); };
	bool getSpectatorsAllowed() const { return static_cast<SerializableItem_Bool *>(itemMap.value("spectators_allowed"))->getData(); };
	bool getSpectatorsNeedPassword() const { return static_cast<SerializableItem_Bool *>(itemMap.value("spectators_need_password"))->getData(); };
	bool getSpectatorsCanTalk() const { return static_cast<SerializableItem_Bool *>(itemMap.value("spectators_can_talk"))->getData(); };
	bool getSpectatorsSeeEverything() const { return static_cast<SerializableItem_Bool *>(itemMap.value("spectators_see_everything"))->getData(); };
	static SerializableItem *newItem() { return new Command_CreateGame; }
	int getItemId() const { return ItemId_Command_CreateGame; }
};
class Command_JoinGame : public RoomCommand {
	Q_OBJECT
public:
	Command_JoinGame(int _roomId = -1, int _gameId = -1, const QString &_password = QString(), bool _spectator = false);
	int getGameId() const { return static_cast<SerializableItem_Int *>(itemMap.value("game_id"))->getData(); };
	QString getPassword() const { return static_cast<SerializableItem_String *>(itemMap.value("password"))->getData(); };
	bool getSpectator() const { return static_cast<SerializableItem_Bool *>(itemMap.value("spectator"))->getData(); };
	static SerializableItem *newItem() { return new Command_JoinGame; }
	int getItemId() const { return ItemId_Command_JoinGame; }
};
class Command_LeaveGame : public GameCommand {
	Q_OBJECT
public:
	Command_LeaveGame(int _gameId = -1);
	static SerializableItem *newItem() { return new Command_LeaveGame; }
	int getItemId() const { return ItemId_Command_LeaveGame; }
};
class Command_Say : public GameCommand {
	Q_OBJECT
public:
	Command_Say(int _gameId = -1, const QString &_message = QString());
	QString getMessage() const { return static_cast<SerializableItem_String *>(itemMap.value("message"))->getData(); };
	static SerializableItem *newItem() { return new Command_Say; }
	int getItemId() const { return ItemId_Command_Say; }
};
class Command_Shuffle : public GameCommand {
	Q_OBJECT
public:
	Command_Shuffle(int _gameId = -1);
	static SerializableItem *newItem() { return new Command_Shuffle; }
	int getItemId() const { return ItemId_Command_Shuffle; }
};
class Command_Mulligan : public GameCommand {
	Q_OBJECT
public:
	Command_Mulligan(int _gameId = -1);
	static SerializableItem *newItem() { return new Command_Mulligan; }
	int getItemId() const { return ItemId_Command_Mulligan; }
};
class Command_RollDie : public GameCommand {
	Q_OBJECT
public:
	Command_RollDie(int _gameId = -1, int _sides = -1);
	int getSides() const { return static_cast<SerializableItem_Int *>(itemMap.value("sides"))->getData(); };
	static SerializableItem *newItem() { return new Command_RollDie; }
	int getItemId() const { return ItemId_Command_RollDie; }
};
class Command_DrawCards : public GameCommand {
	Q_OBJECT
public:
	Command_DrawCards(int _gameId = -1, int _number = -1);
	int getNumber() const { return static_cast<SerializableItem_Int *>(itemMap.value("number"))->getData(); };
	static SerializableItem *newItem() { return new Command_DrawCards; }
	int getItemId() const { return ItemId_Command_DrawCards; }
};
class Command_FlipCard : public GameCommand {
	Q_OBJECT
public:
	Command_FlipCard(int _gameId = -1, const QString &_zone = QString(), int _cardId = -1, bool _faceDown = false);
	QString getZone() const { return static_cast<SerializableItem_String *>(itemMap.value("zone"))->getData(); };
	int getCardId() const { return static_cast<SerializableItem_Int *>(itemMap.value("card_id"))->getData(); };
	bool getFaceDown() const { return static_cast<SerializableItem_Bool *>(itemMap.value("face_down"))->getData(); };
	static SerializableItem *newItem() { return new Command_FlipCard; }
	int getItemId() const { return ItemId_Command_FlipCard; }
};
class Command_AttachCard : public GameCommand {
	Q_OBJECT
public:
	Command_AttachCard(int _gameId = -1, const QString &_startZone = QString(), int _cardId = -1, int _targetPlayerId = -1, const QString &_targetZone = QString(), int _targetCardId = -1);
	QString getStartZone() const { return static_cast<SerializableItem_String *>(itemMap.value("start_zone"))->getData(); };
	int getCardId() const { return static_cast<SerializableItem_Int *>(itemMap.value("card_id"))->getData(); };
	int getTargetPlayerId() const { return static_cast<SerializableItem_Int *>(itemMap.value("target_player_id"))->getData(); };
	QString getTargetZone() const { return static_cast<SerializableItem_String *>(itemMap.value("target_zone"))->getData(); };
	int getTargetCardId() const { return static_cast<SerializableItem_Int *>(itemMap.value("target_card_id"))->getData(); };
	static SerializableItem *newItem() { return new Command_AttachCard; }
	int getItemId() const { return ItemId_Command_AttachCard; }
};
class Command_CreateToken : public GameCommand {
	Q_OBJECT
public:
	Command_CreateToken(int _gameId = -1, const QString &_zone = QString(), const QString &_cardName = QString(), const QString &_color = QString(), const QString &_pt = QString(), const QString &_annotation = QString(), bool _destroy = false, int _x = -1, int _y = -1);
	QString getZone() const { return static_cast<SerializableItem_String *>(itemMap.value("zone"))->getData(); };
	QString getCardName() const { return static_cast<SerializableItem_String *>(itemMap.value("card_name"))->getData(); };
	QString getColor() const { return static_cast<SerializableItem_String *>(itemMap.value("color"))->getData(); };
	QString getPt() const { return static_cast<SerializableItem_String *>(itemMap.value("pt"))->getData(); };
	QString getAnnotation() const { return static_cast<SerializableItem_String *>(itemMap.value("annotation"))->getData(); };
	bool getDestroy() const { return static_cast<SerializableItem_Bool *>(itemMap.value("destroy"))->getData(); };
	int getX() const { return static_cast<SerializableItem_Int *>(itemMap.value("x"))->getData(); };
	int getY() const { return static_cast<SerializableItem_Int *>(itemMap.value("y"))->getData(); };
	static SerializableItem *newItem() { return new Command_CreateToken; }
	int getItemId() const { return ItemId_Command_CreateToken; }
};
class Command_CreateArrow : public GameCommand {
	Q_OBJECT
public:
	Command_CreateArrow(int _gameId = -1, int _startPlayerId = -1, const QString &_startZone = QString(), int _startCardId = -1, int _targetPlayerId = -1, const QString &_targetZone = QString(), int _targetCardId = -1, const Color &_color = Color());
	int getStartPlayerId() const { return static_cast<SerializableItem_Int *>(itemMap.value("start_player_id"))->getData(); };
	QString getStartZone() const { return static_cast<SerializableItem_String *>(itemMap.value("start_zone"))->getData(); };
	int getStartCardId() const { return static_cast<SerializableItem_Int *>(itemMap.value("start_card_id"))->getData(); };
	int getTargetPlayerId() const { return static_cast<SerializableItem_Int *>(itemMap.value("target_player_id"))->getData(); };
	QString getTargetZone() const { return static_cast<SerializableItem_String *>(itemMap.value("target_zone"))->getData(); };
	int getTargetCardId() const { return static_cast<SerializableItem_Int *>(itemMap.value("target_card_id"))->getData(); };
	Color getColor() const { return static_cast<SerializableItem_Color *>(itemMap.value("color"))->getData(); };
	static SerializableItem *newItem() { return new Command_CreateArrow; }
	int getItemId() const { return ItemId_Command_CreateArrow; }
};
class Command_DeleteArrow : public GameCommand {
	Q_OBJECT
public:
	Command_DeleteArrow(int _gameId = -1, int _arrowId = -1);
	int getArrowId() const { return static_cast<SerializableItem_Int *>(itemMap.value("arrow_id"))->getData(); };
	static SerializableItem *newItem() { return new Command_DeleteArrow; }
	int getItemId() const { return ItemId_Command_DeleteArrow; }
};
class Command_SetCardAttr : public GameCommand {
	Q_OBJECT
public:
	Command_SetCardAttr(int _gameId = -1, const QString &_zone = QString(), int _cardId = -1, const QString &_attrName = QString(), const QString &_attrValue = QString());
	QString getZone() const { return static_cast<SerializableItem_String *>(itemMap.value("zone"))->getData(); };
	int getCardId() const { return static_cast<SerializableItem_Int *>(itemMap.value("card_id"))->getData(); };
	QString getAttrName() const { return static_cast<SerializableItem_String *>(itemMap.value("attr_name"))->getData(); };
	QString getAttrValue() const { return static_cast<SerializableItem_String *>(itemMap.value("attr_value"))->getData(); };
	static SerializableItem *newItem() { return new Command_SetCardAttr; }
	int getItemId() const { return ItemId_Command_SetCardAttr; }
};
class Command_SetCardCounter : public GameCommand {
	Q_OBJECT
public:
	Command_SetCardCounter(int _gameId = -1, const QString &_zone = QString(), int _cardId = -1, int _counterId = -1, int _counterValue = -1);
	QString getZone() const { return static_cast<SerializableItem_String *>(itemMap.value("zone"))->getData(); };
	int getCardId() const { return static_cast<SerializableItem_Int *>(itemMap.value("card_id"))->getData(); };
	int getCounterId() const { return static_cast<SerializableItem_Int *>(itemMap.value("counter_id"))->getData(); };
	int getCounterValue() const { return static_cast<SerializableItem_Int *>(itemMap.value("counter_value"))->getData(); };
	static SerializableItem *newItem() { return new Command_SetCardCounter; }
	int getItemId() const { return ItemId_Command_SetCardCounter; }
};
class Command_IncCardCounter : public GameCommand {
	Q_OBJECT
public:
	Command_IncCardCounter(int _gameId = -1, const QString &_zone = QString(), int _cardId = -1, int _counterId = -1, int _counterDelta = -1);
	QString getZone() const { return static_cast<SerializableItem_String *>(itemMap.value("zone"))->getData(); };
	int getCardId() const { return static_cast<SerializableItem_Int *>(itemMap.value("card_id"))->getData(); };
	int getCounterId() const { return static_cast<SerializableItem_Int *>(itemMap.value("counter_id"))->getData(); };
	int getCounterDelta() const { return static_cast<SerializableItem_Int *>(itemMap.value("counter_delta"))->getData(); };
	static SerializableItem *newItem() { return new Command_IncCardCounter; }
	int getItemId() const { return ItemId_Command_IncCardCounter; }
};
class Command_ReadyStart : public GameCommand {
	Q_OBJECT
public:
	Command_ReadyStart(int _gameId = -1, bool _ready = false);
	bool getReady() const { return static_cast<SerializableItem_Bool *>(itemMap.value("ready"))->getData(); };
	static SerializableItem *newItem() { return new Command_ReadyStart; }
	int getItemId() const { return ItemId_Command_ReadyStart; }
};
class Command_Concede : public GameCommand {
	Q_OBJECT
public:
	Command_Concede(int _gameId = -1);
	static SerializableItem *newItem() { return new Command_Concede; }
	int getItemId() const { return ItemId_Command_Concede; }
};
class Command_IncCounter : public GameCommand {
	Q_OBJECT
public:
	Command_IncCounter(int _gameId = -1, int _counterId = -1, int _delta = -1);
	int getCounterId() const { return static_cast<SerializableItem_Int *>(itemMap.value("counter_id"))->getData(); };
	int getDelta() const { return static_cast<SerializableItem_Int *>(itemMap.value("delta"))->getData(); };
	static SerializableItem *newItem() { return new Command_IncCounter; }
	int getItemId() const { return ItemId_Command_IncCounter; }
};
class Command_CreateCounter : public GameCommand {
	Q_OBJECT
public:
	Command_CreateCounter(int _gameId = -1, const QString &_counterName = QString(), const Color &_color = Color(), int _radius = -1, int _value = -1);
	QString getCounterName() const { return static_cast<SerializableItem_String *>(itemMap.value("counter_name"))->getData(); };
	Color getColor() const { return static_cast<SerializableItem_Color *>(itemMap.value("color"))->getData(); };
	int getRadius() const { return static_cast<SerializableItem_Int *>(itemMap.value("radius"))->getData(); };
	int getValue() const { return static_cast<SerializableItem_Int *>(itemMap.value("value"))->getData(); };
	static SerializableItem *newItem() { return new Command_CreateCounter; }
	int getItemId() const { return ItemId_Command_CreateCounter; }
};
class Command_SetCounter : public GameCommand {
	Q_OBJECT
public:
	Command_SetCounter(int _gameId = -1, int _counterId = -1, int _value = -1);
	int getCounterId() const { return static_cast<SerializableItem_Int *>(itemMap.value("counter_id"))->getData(); };
	int getValue() const { return static_cast<SerializableItem_Int *>(itemMap.value("value"))->getData(); };
	static SerializableItem *newItem() { return new Command_SetCounter; }
	int getItemId() const { return ItemId_Command_SetCounter; }
};
class Command_DelCounter : public GameCommand {
	Q_OBJECT
public:
	Command_DelCounter(int _gameId = -1, int _counterId = -1);
	int getCounterId() const { return static_cast<SerializableItem_Int *>(itemMap.value("counter_id"))->getData(); };
	static SerializableItem *newItem() { return new Command_DelCounter; }
	int getItemId() const { return ItemId_Command_DelCounter; }
};
class Command_NextTurn : public GameCommand {
	Q_OBJECT
public:
	Command_NextTurn(int _gameId = -1);
	static SerializableItem *newItem() { return new Command_NextTurn; }
	int getItemId() const { return ItemId_Command_NextTurn; }
};
class Command_SetActivePhase : public GameCommand {
	Q_OBJECT
public:
	Command_SetActivePhase(int _gameId = -1, int _phase = -1);
	int getPhase() const { return static_cast<SerializableItem_Int *>(itemMap.value("phase"))->getData(); };
	static SerializableItem *newItem() { return new Command_SetActivePhase; }
	int getItemId() const { return ItemId_Command_SetActivePhase; }
};
class Command_DumpZone : public GameCommand {
	Q_OBJECT
public:
	Command_DumpZone(int _gameId = -1, int _playerId = -1, const QString &_zoneName = QString(), int _numberCards = -1);
	int getPlayerId() const { return static_cast<SerializableItem_Int *>(itemMap.value("player_id"))->getData(); };
	QString getZoneName() const { return static_cast<SerializableItem_String *>(itemMap.value("zone_name"))->getData(); };
	int getNumberCards() const { return static_cast<SerializableItem_Int *>(itemMap.value("number_cards"))->getData(); };
	static SerializableItem *newItem() { return new Command_DumpZone; }
	int getItemId() const { return ItemId_Command_DumpZone; }
};
class Command_StopDumpZone : public GameCommand {
	Q_OBJECT
public:
	Command_StopDumpZone(int _gameId = -1, int _playerId = -1, const QString &_zoneName = QString());
	int getPlayerId() const { return static_cast<SerializableItem_Int *>(itemMap.value("player_id"))->getData(); };
	QString getZoneName() const { return static_cast<SerializableItem_String *>(itemMap.value("zone_name"))->getData(); };
	static SerializableItem *newItem() { return new Command_StopDumpZone; }
	int getItemId() const { return ItemId_Command_StopDumpZone; }
};
class Command_RevealCards : public GameCommand {
	Q_OBJECT
public:
	Command_RevealCards(int _gameId = -1, const QString &_zoneName = QString(), int _cardId = -1, int _playerId = -1);
	QString getZoneName() const { return static_cast<SerializableItem_String *>(itemMap.value("zone_name"))->getData(); };
	int getCardId() const { return static_cast<SerializableItem_Int *>(itemMap.value("card_id"))->getData(); };
	int getPlayerId() const { return static_cast<SerializableItem_Int *>(itemMap.value("player_id"))->getData(); };
	static SerializableItem *newItem() { return new Command_RevealCards; }
	int getItemId() const { return ItemId_Command_RevealCards; }
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
class Event_ServerMessage : public GenericEvent {
	Q_OBJECT
public:
	Event_ServerMessage(const QString &_message = QString());
	QString getMessage() const { return static_cast<SerializableItem_String *>(itemMap.value("message"))->getData(); };
	static SerializableItem *newItem() { return new Event_ServerMessage; }
	int getItemId() const { return ItemId_Event_ServerMessage; }
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
	Event_GameJoined(int _gameId = -1, const QString &_gameDescription = QString(), int _playerId = -1, bool _spectator = false, bool _spectatorsCanTalk = false, bool _spectatorsSeeEverything = false, bool _resuming = false);
	int getGameId() const { return static_cast<SerializableItem_Int *>(itemMap.value("game_id"))->getData(); };
	QString getGameDescription() const { return static_cast<SerializableItem_String *>(itemMap.value("game_description"))->getData(); };
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
	Context_DeckSelect(int _deckId = -1);
	int getDeckId() const { return static_cast<SerializableItem_Int *>(itemMap.value("deck_id"))->getData(); };
	static SerializableItem *newItem() { return new Context_DeckSelect; }
	int getItemId() const { return ItemId_Context_DeckSelect; }
};

#endif
