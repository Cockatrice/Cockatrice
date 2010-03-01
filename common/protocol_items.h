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
class Command_ListChatChannels : public Command {
	Q_OBJECT
public:
	Command_ListChatChannels();
	static SerializableItem *newItem() { return new Command_ListChatChannels; }
	int getItemId() const { return ItemId_Command_ListChatChannels; }
};
class Command_ChatJoinChannel : public Command {
	Q_OBJECT
public:
	Command_ChatJoinChannel(const QString &_channel = QString());
	QString getChannel() const { return static_cast<SerializableItem_String *>(itemMap.value("channel"))->getData(); };
	static SerializableItem *newItem() { return new Command_ChatJoinChannel; }
	int getItemId() const { return ItemId_Command_ChatJoinChannel; }
};
class Command_ChatLeaveChannel : public ChatCommand {
	Q_OBJECT
public:
	Command_ChatLeaveChannel(const QString &_channel = QString());
	static SerializableItem *newItem() { return new Command_ChatLeaveChannel; }
	int getItemId() const { return ItemId_Command_ChatLeaveChannel; }
};
class Command_ChatSay : public ChatCommand {
	Q_OBJECT
public:
	Command_ChatSay(const QString &_channel = QString(), const QString &_message = QString());
	QString getMessage() const { return static_cast<SerializableItem_String *>(itemMap.value("message"))->getData(); };
	static SerializableItem *newItem() { return new Command_ChatSay; }
	int getItemId() const { return ItemId_Command_ChatSay; }
};
class Command_ListGames : public Command {
	Q_OBJECT
public:
	Command_ListGames();
	static SerializableItem *newItem() { return new Command_ListGames; }
	int getItemId() const { return ItemId_Command_ListGames; }
};
class Command_CreateGame : public Command {
	Q_OBJECT
public:
	Command_CreateGame(const QString &_description = QString(), const QString &_password = QString(), int _maxPlayers = -1, bool _spectatorsAllowed = false);
	QString getDescription() const { return static_cast<SerializableItem_String *>(itemMap.value("description"))->getData(); };
	QString getPassword() const { return static_cast<SerializableItem_String *>(itemMap.value("password"))->getData(); };
	int getMaxPlayers() const { return static_cast<SerializableItem_Int *>(itemMap.value("max_players"))->getData(); };
	bool getSpectatorsAllowed() const { return static_cast<SerializableItem_Bool *>(itemMap.value("spectators_allowed"))->getData(); };
	static SerializableItem *newItem() { return new Command_CreateGame; }
	int getItemId() const { return ItemId_Command_CreateGame; }
};
class Command_JoinGame : public Command {
	Q_OBJECT
public:
	Command_JoinGame(int _gameId = -1, const QString &_password = QString(), bool _spectator = false);
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
class Command_MoveCard : public GameCommand {
	Q_OBJECT
public:
	Command_MoveCard(int _gameId = -1, const QString &_startZone = QString(), int _cardId = -1, const QString &_targetZone = QString(), int _x = -1, int _y = -1, bool _faceDown = false, bool _tapped = false);
	QString getStartZone() const { return static_cast<SerializableItem_String *>(itemMap.value("start_zone"))->getData(); };
	int getCardId() const { return static_cast<SerializableItem_Int *>(itemMap.value("card_id"))->getData(); };
	QString getTargetZone() const { return static_cast<SerializableItem_String *>(itemMap.value("target_zone"))->getData(); };
	int getX() const { return static_cast<SerializableItem_Int *>(itemMap.value("x"))->getData(); };
	int getY() const { return static_cast<SerializableItem_Int *>(itemMap.value("y"))->getData(); };
	bool getFaceDown() const { return static_cast<SerializableItem_Bool *>(itemMap.value("face_down"))->getData(); };
	bool getTapped() const { return static_cast<SerializableItem_Bool *>(itemMap.value("tapped"))->getData(); };
	static SerializableItem *newItem() { return new Command_MoveCard; }
	int getItemId() const { return ItemId_Command_MoveCard; }
};
class Command_CreateToken : public GameCommand {
	Q_OBJECT
public:
	Command_CreateToken(int _gameId = -1, const QString &_zone = QString(), const QString &_cardName = QString(), const QString &_pt = QString(), int _x = -1, int _y = -1);
	QString getZone() const { return static_cast<SerializableItem_String *>(itemMap.value("zone"))->getData(); };
	QString getCardName() const { return static_cast<SerializableItem_String *>(itemMap.value("card_name"))->getData(); };
	QString getPt() const { return static_cast<SerializableItem_String *>(itemMap.value("pt"))->getData(); };
	int getX() const { return static_cast<SerializableItem_Int *>(itemMap.value("x"))->getData(); };
	int getY() const { return static_cast<SerializableItem_Int *>(itemMap.value("y"))->getData(); };
	static SerializableItem *newItem() { return new Command_CreateToken; }
	int getItemId() const { return ItemId_Command_CreateToken; }
};
class Command_CreateArrow : public GameCommand {
	Q_OBJECT
public:
	Command_CreateArrow(int _gameId = -1, int _startPlayerId = -1, const QString &_startZone = QString(), int _startCardId = -1, int _targetPlayerId = -1, const QString &_targetZone = QString(), int _targetCardId = -1, const QColor &_color = QColor());
	int getStartPlayerId() const { return static_cast<SerializableItem_Int *>(itemMap.value("start_player_id"))->getData(); };
	QString getStartZone() const { return static_cast<SerializableItem_String *>(itemMap.value("start_zone"))->getData(); };
	int getStartCardId() const { return static_cast<SerializableItem_Int *>(itemMap.value("start_card_id"))->getData(); };
	int getTargetPlayerId() const { return static_cast<SerializableItem_Int *>(itemMap.value("target_player_id"))->getData(); };
	QString getTargetZone() const { return static_cast<SerializableItem_String *>(itemMap.value("target_zone"))->getData(); };
	int getTargetCardId() const { return static_cast<SerializableItem_Int *>(itemMap.value("target_card_id"))->getData(); };
	QColor getColor() const { return static_cast<SerializableItem_Color *>(itemMap.value("color"))->getData(); };
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
class Command_ReadyStart : public GameCommand {
	Q_OBJECT
public:
	Command_ReadyStart(int _gameId = -1);
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
	Command_CreateCounter(int _gameId = -1, const QString &_counterName = QString(), const QColor &_color = QColor(), int _radius = -1, int _value = -1);
	QString getCounterName() const { return static_cast<SerializableItem_String *>(itemMap.value("counter_name"))->getData(); };
	QColor getColor() const { return static_cast<SerializableItem_Color *>(itemMap.value("color"))->getData(); };
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
class Event_DeckSelect : public GameEvent {
	Q_OBJECT
public:
	Event_DeckSelect(int _playerId = -1, int _deckId = -1);
	int getDeckId() const { return static_cast<SerializableItem_Int *>(itemMap.value("deck_id"))->getData(); };
	static SerializableItem *newItem() { return new Event_DeckSelect; }
	int getItemId() const { return ItemId_Event_DeckSelect; }
};
class Event_GameClosed : public GameEvent {
	Q_OBJECT
public:
	Event_GameClosed(int _playerId = -1);
	static SerializableItem *newItem() { return new Event_GameClosed; }
	int getItemId() const { return ItemId_Event_GameClosed; }
};
class Event_ReadyStart : public GameEvent {
	Q_OBJECT
public:
	Event_ReadyStart(int _playerId = -1);
	static SerializableItem *newItem() { return new Event_ReadyStart; }
	int getItemId() const { return ItemId_Event_ReadyStart; }
};
class Event_Concede : public GameEvent {
	Q_OBJECT
public:
	Event_Concede(int _playerId = -1);
	static SerializableItem *newItem() { return new Event_Concede; }
	int getItemId() const { return ItemId_Event_Concede; }
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
	Event_MoveCard(int _playerId = -1, int _cardId = -1, const QString &_cardName = QString(), const QString &_startZone = QString(), int _position = -1, const QString &_targetZone = QString(), int _x = -1, int _y = -1, int _newCardId = -1, bool _faceDown = false);
	int getCardId() const { return static_cast<SerializableItem_Int *>(itemMap.value("card_id"))->getData(); };
	QString getCardName() const { return static_cast<SerializableItem_String *>(itemMap.value("card_name"))->getData(); };
	QString getStartZone() const { return static_cast<SerializableItem_String *>(itemMap.value("start_zone"))->getData(); };
	int getPosition() const { return static_cast<SerializableItem_Int *>(itemMap.value("position"))->getData(); };
	QString getTargetZone() const { return static_cast<SerializableItem_String *>(itemMap.value("target_zone"))->getData(); };
	int getX() const { return static_cast<SerializableItem_Int *>(itemMap.value("x"))->getData(); };
	int getY() const { return static_cast<SerializableItem_Int *>(itemMap.value("y"))->getData(); };
	int getNewCardId() const { return static_cast<SerializableItem_Int *>(itemMap.value("new_card_id"))->getData(); };
	bool getFaceDown() const { return static_cast<SerializableItem_Bool *>(itemMap.value("face_down"))->getData(); };
	static SerializableItem *newItem() { return new Event_MoveCard; }
	int getItemId() const { return ItemId_Event_MoveCard; }
};
class Event_CreateToken : public GameEvent {
	Q_OBJECT
public:
	Event_CreateToken(int _playerId = -1, const QString &_zone = QString(), int _cardId = -1, const QString &_cardName = QString(), const QString &_pt = QString(), int _x = -1, int _y = -1);
	QString getZone() const { return static_cast<SerializableItem_String *>(itemMap.value("zone"))->getData(); };
	int getCardId() const { return static_cast<SerializableItem_Int *>(itemMap.value("card_id"))->getData(); };
	QString getCardName() const { return static_cast<SerializableItem_String *>(itemMap.value("card_name"))->getData(); };
	QString getPt() const { return static_cast<SerializableItem_String *>(itemMap.value("pt"))->getData(); };
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
class Event_GameJoined : public GenericEvent {
	Q_OBJECT
public:
	Event_GameJoined(int _gameId = -1, const QString &_gameDescription = QString(), int _playerId = -1, bool _spectator = false, bool _resuming = false);
	int getGameId() const { return static_cast<SerializableItem_Int *>(itemMap.value("game_id"))->getData(); };
	QString getGameDescription() const { return static_cast<SerializableItem_String *>(itemMap.value("game_description"))->getData(); };
	int getPlayerId() const { return static_cast<SerializableItem_Int *>(itemMap.value("player_id"))->getData(); };
	bool getSpectator() const { return static_cast<SerializableItem_Bool *>(itemMap.value("spectator"))->getData(); };
	bool getResuming() const { return static_cast<SerializableItem_Bool *>(itemMap.value("resuming"))->getData(); };
	static SerializableItem *newItem() { return new Event_GameJoined; }
	int getItemId() const { return ItemId_Event_GameJoined; }
};
class Event_ChatJoinChannel : public ChatEvent {
	Q_OBJECT
public:
	Event_ChatJoinChannel(const QString &_channel = QString(), const QString &_playerName = QString());
	QString getPlayerName() const { return static_cast<SerializableItem_String *>(itemMap.value("player_name"))->getData(); };
	static SerializableItem *newItem() { return new Event_ChatJoinChannel; }
	int getItemId() const { return ItemId_Event_ChatJoinChannel; }
};
class Event_ChatLeaveChannel : public ChatEvent {
	Q_OBJECT
public:
	Event_ChatLeaveChannel(const QString &_channel = QString(), const QString &_playerName = QString());
	QString getPlayerName() const { return static_cast<SerializableItem_String *>(itemMap.value("player_name"))->getData(); };
	static SerializableItem *newItem() { return new Event_ChatLeaveChannel; }
	int getItemId() const { return ItemId_Event_ChatLeaveChannel; }
};
class Event_ChatSay : public ChatEvent {
	Q_OBJECT
public:
	Event_ChatSay(const QString &_channel = QString(), const QString &_playerName = QString(), const QString &_message = QString());
	QString getPlayerName() const { return static_cast<SerializableItem_String *>(itemMap.value("player_name"))->getData(); };
	QString getMessage() const { return static_cast<SerializableItem_String *>(itemMap.value("message"))->getData(); };
	static SerializableItem *newItem() { return new Event_ChatSay; }
	int getItemId() const { return ItemId_Event_ChatSay; }
};

#endif
