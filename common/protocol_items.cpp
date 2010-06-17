#include "protocol.h"
#include "protocol_items.h"

Command_Ping::Command_Ping()
	: Command("ping")
{
}
Command_Login::Command_Login(const QString &_username, const QString &_password)
	: Command("login")
{
	insertItem(new SerializableItem_String("username", _username));
	insertItem(new SerializableItem_String("password", _password));
}
Command_DeckList::Command_DeckList()
	: Command("deck_list")
{
}
Command_DeckNewDir::Command_DeckNewDir(const QString &_path, const QString &_dirName)
	: Command("deck_new_dir")
{
	insertItem(new SerializableItem_String("path", _path));
	insertItem(new SerializableItem_String("dir_name", _dirName));
}
Command_DeckDelDir::Command_DeckDelDir(const QString &_path)
	: Command("deck_del_dir")
{
	insertItem(new SerializableItem_String("path", _path));
}
Command_DeckDel::Command_DeckDel(int _deckId)
	: Command("deck_del")
{
	insertItem(new SerializableItem_Int("deck_id", _deckId));
}
Command_DeckDownload::Command_DeckDownload(int _deckId)
	: Command("deck_download")
{
	insertItem(new SerializableItem_Int("deck_id", _deckId));
}
Command_ListChatChannels::Command_ListChatChannels()
	: Command("list_chat_channels")
{
}
Command_ChatJoinChannel::Command_ChatJoinChannel(const QString &_channel)
	: Command("chat_join_channel")
{
	insertItem(new SerializableItem_String("channel", _channel));
}
Command_ChatLeaveChannel::Command_ChatLeaveChannel(const QString &_channel)
	: ChatCommand("chat_leave_channel", _channel)
{
}
Command_ChatSay::Command_ChatSay(const QString &_channel, const QString &_message)
	: ChatCommand("chat_say", _channel)
{
	insertItem(new SerializableItem_String("message", _message));
}
Command_ListGames::Command_ListGames()
	: Command("list_games")
{
}
Command_CreateGame::Command_CreateGame(const QString &_description, const QString &_password, int _maxPlayers, bool _spectatorsAllowed, bool _spectatorsNeedPassword, bool _spectatorsCanTalk, bool _spectatorsSeeEverything)
	: Command("create_game")
{
	insertItem(new SerializableItem_String("description", _description));
	insertItem(new SerializableItem_String("password", _password));
	insertItem(new SerializableItem_Int("max_players", _maxPlayers));
	insertItem(new SerializableItem_Bool("spectators_allowed", _spectatorsAllowed));
	insertItem(new SerializableItem_Bool("spectators_need_password", _spectatorsNeedPassword));
	insertItem(new SerializableItem_Bool("spectators_can_talk", _spectatorsCanTalk));
	insertItem(new SerializableItem_Bool("spectators_see_everything", _spectatorsSeeEverything));
}
Command_JoinGame::Command_JoinGame(int _gameId, const QString &_password, bool _spectator)
	: Command("join_game")
{
	insertItem(new SerializableItem_Int("game_id", _gameId));
	insertItem(new SerializableItem_String("password", _password));
	insertItem(new SerializableItem_Bool("spectator", _spectator));
}
Command_LeaveGame::Command_LeaveGame(int _gameId)
	: GameCommand("leave_game", _gameId)
{
}
Command_Say::Command_Say(int _gameId, const QString &_message)
	: GameCommand("say", _gameId)
{
	insertItem(new SerializableItem_String("message", _message));
}
Command_Shuffle::Command_Shuffle(int _gameId)
	: GameCommand("shuffle", _gameId)
{
}
Command_Mulligan::Command_Mulligan(int _gameId)
	: GameCommand("mulligan", _gameId)
{
}
Command_RollDie::Command_RollDie(int _gameId, int _sides)
	: GameCommand("roll_die", _gameId)
{
	insertItem(new SerializableItem_Int("sides", _sides));
}
Command_DrawCards::Command_DrawCards(int _gameId, int _number)
	: GameCommand("draw_cards", _gameId)
{
	insertItem(new SerializableItem_Int("number", _number));
}
Command_MoveCard::Command_MoveCard(int _gameId, const QString &_startZone, int _cardId, const QString &_targetZone, int _x, int _y, bool _faceDown, bool _tapped)
	: GameCommand("move_card", _gameId)
{
	insertItem(new SerializableItem_String("start_zone", _startZone));
	insertItem(new SerializableItem_Int("card_id", _cardId));
	insertItem(new SerializableItem_String("target_zone", _targetZone));
	insertItem(new SerializableItem_Int("x", _x));
	insertItem(new SerializableItem_Int("y", _y));
	insertItem(new SerializableItem_Bool("face_down", _faceDown));
	insertItem(new SerializableItem_Bool("tapped", _tapped));
}
Command_CreateToken::Command_CreateToken(int _gameId, const QString &_zone, const QString &_cardName, const QString &_pt, int _x, int _y)
	: GameCommand("create_token", _gameId)
{
	insertItem(new SerializableItem_String("zone", _zone));
	insertItem(new SerializableItem_String("card_name", _cardName));
	insertItem(new SerializableItem_String("pt", _pt));
	insertItem(new SerializableItem_Int("x", _x));
	insertItem(new SerializableItem_Int("y", _y));
}
Command_CreateArrow::Command_CreateArrow(int _gameId, int _startPlayerId, const QString &_startZone, int _startCardId, int _targetPlayerId, const QString &_targetZone, int _targetCardId, const QColor &_color)
	: GameCommand("create_arrow", _gameId)
{
	insertItem(new SerializableItem_Int("start_player_id", _startPlayerId));
	insertItem(new SerializableItem_String("start_zone", _startZone));
	insertItem(new SerializableItem_Int("start_card_id", _startCardId));
	insertItem(new SerializableItem_Int("target_player_id", _targetPlayerId));
	insertItem(new SerializableItem_String("target_zone", _targetZone));
	insertItem(new SerializableItem_Int("target_card_id", _targetCardId));
	insertItem(new SerializableItem_Color("color", _color));
}
Command_DeleteArrow::Command_DeleteArrow(int _gameId, int _arrowId)
	: GameCommand("delete_arrow", _gameId)
{
	insertItem(new SerializableItem_Int("arrow_id", _arrowId));
}
Command_SetCardAttr::Command_SetCardAttr(int _gameId, const QString &_zone, int _cardId, const QString &_attrName, const QString &_attrValue)
	: GameCommand("set_card_attr", _gameId)
{
	insertItem(new SerializableItem_String("zone", _zone));
	insertItem(new SerializableItem_Int("card_id", _cardId));
	insertItem(new SerializableItem_String("attr_name", _attrName));
	insertItem(new SerializableItem_String("attr_value", _attrValue));
}
Command_SetCardCounter::Command_SetCardCounter(int _gameId, const QString &_zone, int _cardId, int _counterId, int _counterValue)
	: GameCommand("set_card_counter", _gameId)
{
	insertItem(new SerializableItem_String("zone", _zone));
	insertItem(new SerializableItem_Int("card_id", _cardId));
	insertItem(new SerializableItem_Int("counter_id", _counterId));
	insertItem(new SerializableItem_Int("counter_value", _counterValue));
}
Command_IncCardCounter::Command_IncCardCounter(int _gameId, const QString &_zone, int _cardId, int _counterId, int _counterDelta)
	: GameCommand("inc_card_counter", _gameId)
{
	insertItem(new SerializableItem_String("zone", _zone));
	insertItem(new SerializableItem_Int("card_id", _cardId));
	insertItem(new SerializableItem_Int("counter_id", _counterId));
	insertItem(new SerializableItem_Int("counter_delta", _counterDelta));
}
Command_ReadyStart::Command_ReadyStart(int _gameId, bool _ready)
	: GameCommand("ready_start", _gameId)
{
	insertItem(new SerializableItem_Bool("ready", _ready));
}
Command_Concede::Command_Concede(int _gameId)
	: GameCommand("concede", _gameId)
{
}
Command_IncCounter::Command_IncCounter(int _gameId, int _counterId, int _delta)
	: GameCommand("inc_counter", _gameId)
{
	insertItem(new SerializableItem_Int("counter_id", _counterId));
	insertItem(new SerializableItem_Int("delta", _delta));
}
Command_CreateCounter::Command_CreateCounter(int _gameId, const QString &_counterName, const QColor &_color, int _radius, int _value)
	: GameCommand("create_counter", _gameId)
{
	insertItem(new SerializableItem_String("counter_name", _counterName));
	insertItem(new SerializableItem_Color("color", _color));
	insertItem(new SerializableItem_Int("radius", _radius));
	insertItem(new SerializableItem_Int("value", _value));
}
Command_SetCounter::Command_SetCounter(int _gameId, int _counterId, int _value)
	: GameCommand("set_counter", _gameId)
{
	insertItem(new SerializableItem_Int("counter_id", _counterId));
	insertItem(new SerializableItem_Int("value", _value));
}
Command_DelCounter::Command_DelCounter(int _gameId, int _counterId)
	: GameCommand("del_counter", _gameId)
{
	insertItem(new SerializableItem_Int("counter_id", _counterId));
}
Command_NextTurn::Command_NextTurn(int _gameId)
	: GameCommand("next_turn", _gameId)
{
}
Command_SetActivePhase::Command_SetActivePhase(int _gameId, int _phase)
	: GameCommand("set_active_phase", _gameId)
{
	insertItem(new SerializableItem_Int("phase", _phase));
}
Command_DumpZone::Command_DumpZone(int _gameId, int _playerId, const QString &_zoneName, int _numberCards)
	: GameCommand("dump_zone", _gameId)
{
	insertItem(new SerializableItem_Int("player_id", _playerId));
	insertItem(new SerializableItem_String("zone_name", _zoneName));
	insertItem(new SerializableItem_Int("number_cards", _numberCards));
}
Command_StopDumpZone::Command_StopDumpZone(int _gameId, int _playerId, const QString &_zoneName)
	: GameCommand("stop_dump_zone", _gameId)
{
	insertItem(new SerializableItem_Int("player_id", _playerId));
	insertItem(new SerializableItem_String("zone_name", _zoneName));
}
Event_Say::Event_Say(int _playerId, const QString &_message)
	: GameEvent("say", _playerId)
{
	insertItem(new SerializableItem_String("message", _message));
}
Event_Leave::Event_Leave(int _playerId)
	: GameEvent("leave", _playerId)
{
}
Event_GameClosed::Event_GameClosed(int _playerId)
	: GameEvent("game_closed", _playerId)
{
}
Event_Shuffle::Event_Shuffle(int _playerId)
	: GameEvent("shuffle", _playerId)
{
}
Event_RollDie::Event_RollDie(int _playerId, int _sides, int _value)
	: GameEvent("roll_die", _playerId)
{
	insertItem(new SerializableItem_Int("sides", _sides));
	insertItem(new SerializableItem_Int("value", _value));
}
Event_MoveCard::Event_MoveCard(int _playerId, int _cardId, const QString &_cardName, const QString &_startZone, int _position, const QString &_targetZone, int _x, int _y, int _newCardId, bool _faceDown)
	: GameEvent("move_card", _playerId)
{
	insertItem(new SerializableItem_Int("card_id", _cardId));
	insertItem(new SerializableItem_String("card_name", _cardName));
	insertItem(new SerializableItem_String("start_zone", _startZone));
	insertItem(new SerializableItem_Int("position", _position));
	insertItem(new SerializableItem_String("target_zone", _targetZone));
	insertItem(new SerializableItem_Int("x", _x));
	insertItem(new SerializableItem_Int("y", _y));
	insertItem(new SerializableItem_Int("new_card_id", _newCardId));
	insertItem(new SerializableItem_Bool("face_down", _faceDown));
}
Event_CreateToken::Event_CreateToken(int _playerId, const QString &_zone, int _cardId, const QString &_cardName, const QString &_pt, int _x, int _y)
	: GameEvent("create_token", _playerId)
{
	insertItem(new SerializableItem_String("zone", _zone));
	insertItem(new SerializableItem_Int("card_id", _cardId));
	insertItem(new SerializableItem_String("card_name", _cardName));
	insertItem(new SerializableItem_String("pt", _pt));
	insertItem(new SerializableItem_Int("x", _x));
	insertItem(new SerializableItem_Int("y", _y));
}
Event_DeleteArrow::Event_DeleteArrow(int _playerId, int _arrowId)
	: GameEvent("delete_arrow", _playerId)
{
	insertItem(new SerializableItem_Int("arrow_id", _arrowId));
}
Event_SetCardAttr::Event_SetCardAttr(int _playerId, const QString &_zone, int _cardId, const QString &_attrName, const QString &_attrValue)
	: GameEvent("set_card_attr", _playerId)
{
	insertItem(new SerializableItem_String("zone", _zone));
	insertItem(new SerializableItem_Int("card_id", _cardId));
	insertItem(new SerializableItem_String("attr_name", _attrName));
	insertItem(new SerializableItem_String("attr_value", _attrValue));
}
Event_SetCardCounter::Event_SetCardCounter(int _playerId, const QString &_zone, int _cardId, int _counterId, int _counterValue)
	: GameEvent("set_card_counter", _playerId)
{
	insertItem(new SerializableItem_String("zone", _zone));
	insertItem(new SerializableItem_Int("card_id", _cardId));
	insertItem(new SerializableItem_Int("counter_id", _counterId));
	insertItem(new SerializableItem_Int("counter_value", _counterValue));
}
Event_SetCounter::Event_SetCounter(int _playerId, int _counterId, int _value)
	: GameEvent("set_counter", _playerId)
{
	insertItem(new SerializableItem_Int("counter_id", _counterId));
	insertItem(new SerializableItem_Int("value", _value));
}
Event_DelCounter::Event_DelCounter(int _playerId, int _counterId)
	: GameEvent("del_counter", _playerId)
{
	insertItem(new SerializableItem_Int("counter_id", _counterId));
}
Event_SetActivePlayer::Event_SetActivePlayer(int _playerId, int _activePlayerId)
	: GameEvent("set_active_player", _playerId)
{
	insertItem(new SerializableItem_Int("active_player_id", _activePlayerId));
}
Event_SetActivePhase::Event_SetActivePhase(int _playerId, int _phase)
	: GameEvent("set_active_phase", _playerId)
{
	insertItem(new SerializableItem_Int("phase", _phase));
}
Event_DumpZone::Event_DumpZone(int _playerId, int _zoneOwnerId, const QString &_zone, int _numberCards)
	: GameEvent("dump_zone", _playerId)
{
	insertItem(new SerializableItem_Int("zone_owner_id", _zoneOwnerId));
	insertItem(new SerializableItem_String("zone", _zone));
	insertItem(new SerializableItem_Int("number_cards", _numberCards));
}
Event_StopDumpZone::Event_StopDumpZone(int _playerId, int _zoneOwnerId, const QString &_zone)
	: GameEvent("stop_dump_zone", _playerId)
{
	insertItem(new SerializableItem_Int("zone_owner_id", _zoneOwnerId));
	insertItem(new SerializableItem_String("zone", _zone));
}
Event_ServerMessage::Event_ServerMessage(const QString &_message)
	: GenericEvent("server_message")
{
	insertItem(new SerializableItem_String("message", _message));
}
Event_GameJoined::Event_GameJoined(int _gameId, const QString &_gameDescription, int _playerId, bool _spectator, bool _spectatorsCanTalk, bool _spectatorsSeeEverything, bool _resuming)
	: GenericEvent("game_joined")
{
	insertItem(new SerializableItem_Int("game_id", _gameId));
	insertItem(new SerializableItem_String("game_description", _gameDescription));
	insertItem(new SerializableItem_Int("player_id", _playerId));
	insertItem(new SerializableItem_Bool("spectator", _spectator));
	insertItem(new SerializableItem_Bool("spectators_can_talk", _spectatorsCanTalk));
	insertItem(new SerializableItem_Bool("spectators_see_everything", _spectatorsSeeEverything));
	insertItem(new SerializableItem_Bool("resuming", _resuming));
}
Event_ChatJoinChannel::Event_ChatJoinChannel(const QString &_channel, const QString &_playerName)
	: ChatEvent("chat_join_channel", _channel)
{
	insertItem(new SerializableItem_String("player_name", _playerName));
}
Event_ChatLeaveChannel::Event_ChatLeaveChannel(const QString &_channel, const QString &_playerName)
	: ChatEvent("chat_leave_channel", _channel)
{
	insertItem(new SerializableItem_String("player_name", _playerName));
}
Event_ChatSay::Event_ChatSay(const QString &_channel, const QString &_playerName, const QString &_message)
	: ChatEvent("chat_say", _channel)
{
	insertItem(new SerializableItem_String("player_name", _playerName));
	insertItem(new SerializableItem_String("message", _message));
}
Context_ReadyStart::Context_ReadyStart()
	: GameEventContext("ready_start")
{
}
Context_Concede::Context_Concede()
	: GameEventContext("concede")
{
}
Context_DeckSelect::Context_DeckSelect(int _deckId)
	: GameEventContext("deck_select")
{
	insertItem(new SerializableItem_Int("deck_id", _deckId));
}
void ProtocolItem::initializeHashAuto()
{
	itemNameHash.insert("cmdping", Command_Ping::newItem);
	itemNameHash.insert("cmdlogin", Command_Login::newItem);
	itemNameHash.insert("cmddeck_list", Command_DeckList::newItem);
	itemNameHash.insert("cmddeck_new_dir", Command_DeckNewDir::newItem);
	itemNameHash.insert("cmddeck_del_dir", Command_DeckDelDir::newItem);
	itemNameHash.insert("cmddeck_del", Command_DeckDel::newItem);
	itemNameHash.insert("cmddeck_download", Command_DeckDownload::newItem);
	itemNameHash.insert("cmdlist_chat_channels", Command_ListChatChannels::newItem);
	itemNameHash.insert("cmdchat_join_channel", Command_ChatJoinChannel::newItem);
	itemNameHash.insert("cmdchat_leave_channel", Command_ChatLeaveChannel::newItem);
	itemNameHash.insert("cmdchat_say", Command_ChatSay::newItem);
	itemNameHash.insert("cmdlist_games", Command_ListGames::newItem);
	itemNameHash.insert("cmdcreate_game", Command_CreateGame::newItem);
	itemNameHash.insert("cmdjoin_game", Command_JoinGame::newItem);
	itemNameHash.insert("cmdleave_game", Command_LeaveGame::newItem);
	itemNameHash.insert("cmdsay", Command_Say::newItem);
	itemNameHash.insert("cmdshuffle", Command_Shuffle::newItem);
	itemNameHash.insert("cmdmulligan", Command_Mulligan::newItem);
	itemNameHash.insert("cmdroll_die", Command_RollDie::newItem);
	itemNameHash.insert("cmddraw_cards", Command_DrawCards::newItem);
	itemNameHash.insert("cmdmove_card", Command_MoveCard::newItem);
	itemNameHash.insert("cmdcreate_token", Command_CreateToken::newItem);
	itemNameHash.insert("cmdcreate_arrow", Command_CreateArrow::newItem);
	itemNameHash.insert("cmddelete_arrow", Command_DeleteArrow::newItem);
	itemNameHash.insert("cmdset_card_attr", Command_SetCardAttr::newItem);
	itemNameHash.insert("cmdset_card_counter", Command_SetCardCounter::newItem);
	itemNameHash.insert("cmdinc_card_counter", Command_IncCardCounter::newItem);
	itemNameHash.insert("cmdready_start", Command_ReadyStart::newItem);
	itemNameHash.insert("cmdconcede", Command_Concede::newItem);
	itemNameHash.insert("cmdinc_counter", Command_IncCounter::newItem);
	itemNameHash.insert("cmdcreate_counter", Command_CreateCounter::newItem);
	itemNameHash.insert("cmdset_counter", Command_SetCounter::newItem);
	itemNameHash.insert("cmddel_counter", Command_DelCounter::newItem);
	itemNameHash.insert("cmdnext_turn", Command_NextTurn::newItem);
	itemNameHash.insert("cmdset_active_phase", Command_SetActivePhase::newItem);
	itemNameHash.insert("cmddump_zone", Command_DumpZone::newItem);
	itemNameHash.insert("cmdstop_dump_zone", Command_StopDumpZone::newItem);
	itemNameHash.insert("game_eventsay", Event_Say::newItem);
	itemNameHash.insert("game_eventleave", Event_Leave::newItem);
	itemNameHash.insert("game_eventgame_closed", Event_GameClosed::newItem);
	itemNameHash.insert("game_eventshuffle", Event_Shuffle::newItem);
	itemNameHash.insert("game_eventroll_die", Event_RollDie::newItem);
	itemNameHash.insert("game_eventmove_card", Event_MoveCard::newItem);
	itemNameHash.insert("game_eventcreate_token", Event_CreateToken::newItem);
	itemNameHash.insert("game_eventdelete_arrow", Event_DeleteArrow::newItem);
	itemNameHash.insert("game_eventset_card_attr", Event_SetCardAttr::newItem);
	itemNameHash.insert("game_eventset_card_counter", Event_SetCardCounter::newItem);
	itemNameHash.insert("game_eventset_counter", Event_SetCounter::newItem);
	itemNameHash.insert("game_eventdel_counter", Event_DelCounter::newItem);
	itemNameHash.insert("game_eventset_active_player", Event_SetActivePlayer::newItem);
	itemNameHash.insert("game_eventset_active_phase", Event_SetActivePhase::newItem);
	itemNameHash.insert("game_eventdump_zone", Event_DumpZone::newItem);
	itemNameHash.insert("game_eventstop_dump_zone", Event_StopDumpZone::newItem);
	itemNameHash.insert("generic_eventserver_message", Event_ServerMessage::newItem);
	itemNameHash.insert("generic_eventgame_joined", Event_GameJoined::newItem);
	itemNameHash.insert("chat_eventchat_join_channel", Event_ChatJoinChannel::newItem);
	itemNameHash.insert("chat_eventchat_leave_channel", Event_ChatLeaveChannel::newItem);
	itemNameHash.insert("chat_eventchat_say", Event_ChatSay::newItem);
	itemNameHash.insert("game_event_contextready_start", Context_ReadyStart::newItem);
	itemNameHash.insert("game_event_contextconcede", Context_Concede::newItem);
	itemNameHash.insert("game_event_contextdeck_select", Context_DeckSelect::newItem);
}
