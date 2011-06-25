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
Command_Message::Command_Message(const QString &_userName, const QString &_text)
	: Command("message")
{
	insertItem(new SerializableItem_String("user_name", _userName));
	insertItem(new SerializableItem_String("text", _text));
}
Command_ListUsers::Command_ListUsers()
	: Command("list_users")
{
}
Command_GetUserInfo::Command_GetUserInfo(const QString &_userName)
	: Command("get_user_info")
{
	insertItem(new SerializableItem_String("user_name", _userName));
}
Command_AddToList::Command_AddToList(const QString &_list, const QString &_userName)
	: Command("add_to_list")
{
	insertItem(new SerializableItem_String("list", _list));
	insertItem(new SerializableItem_String("user_name", _userName));
}
Command_RemoveFromList::Command_RemoveFromList(const QString &_list, const QString &_userName)
	: Command("remove_from_list")
{
	insertItem(new SerializableItem_String("list", _list));
	insertItem(new SerializableItem_String("user_name", _userName));
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
Command_ListRooms::Command_ListRooms()
	: Command("list_rooms")
{
}
Command_JoinRoom::Command_JoinRoom(int _roomId)
	: Command("join_room")
{
	insertItem(new SerializableItem_Int("room_id", _roomId));
}
Command_LeaveRoom::Command_LeaveRoom(int _roomId)
	: RoomCommand("leave_room", _roomId)
{
}
Command_RoomSay::Command_RoomSay(int _roomId, const QString &_message)
	: RoomCommand("room_say", _roomId)
{
	insertItem(new SerializableItem_String("message", _message));
}
Command_JoinGame::Command_JoinGame(int _roomId, int _gameId, const QString &_password, bool _spectator)
	: RoomCommand("join_game", _roomId)
{
	insertItem(new SerializableItem_Int("game_id", _gameId));
	insertItem(new SerializableItem_String("password", _password));
	insertItem(new SerializableItem_Bool("spectator", _spectator));
}
Command_KickFromGame::Command_KickFromGame(int _gameId, int _playerId)
	: GameCommand("kick_from_game", _gameId)
{
	insertItem(new SerializableItem_Int("player_id", _playerId));
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
Command_UndoDraw::Command_UndoDraw(int _gameId)
	: GameCommand("undo_draw", _gameId)
{
}
Command_FlipCard::Command_FlipCard(int _gameId, const QString &_zone, int _cardId, bool _faceDown)
	: GameCommand("flip_card", _gameId)
{
	insertItem(new SerializableItem_String("zone", _zone));
	insertItem(new SerializableItem_Int("card_id", _cardId));
	insertItem(new SerializableItem_Bool("face_down", _faceDown));
}
Command_AttachCard::Command_AttachCard(int _gameId, const QString &_startZone, int _cardId, int _targetPlayerId, const QString &_targetZone, int _targetCardId)
	: GameCommand("attach_card", _gameId)
{
	insertItem(new SerializableItem_String("start_zone", _startZone));
	insertItem(new SerializableItem_Int("card_id", _cardId));
	insertItem(new SerializableItem_Int("target_player_id", _targetPlayerId));
	insertItem(new SerializableItem_String("target_zone", _targetZone));
	insertItem(new SerializableItem_Int("target_card_id", _targetCardId));
}
Command_CreateToken::Command_CreateToken(int _gameId, const QString &_zone, const QString &_cardName, const QString &_color, const QString &_pt, const QString &_annotation, bool _destroy, int _x, int _y)
	: GameCommand("create_token", _gameId)
{
	insertItem(new SerializableItem_String("zone", _zone));
	insertItem(new SerializableItem_String("card_name", _cardName));
	insertItem(new SerializableItem_String("color", _color));
	insertItem(new SerializableItem_String("pt", _pt));
	insertItem(new SerializableItem_String("annotation", _annotation));
	insertItem(new SerializableItem_Bool("destroy", _destroy));
	insertItem(new SerializableItem_Int("x", _x));
	insertItem(new SerializableItem_Int("y", _y));
}
Command_CreateArrow::Command_CreateArrow(int _gameId, int _startPlayerId, const QString &_startZone, int _startCardId, int _targetPlayerId, const QString &_targetZone, int _targetCardId, const Color &_color)
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
Command_CreateCounter::Command_CreateCounter(int _gameId, const QString &_counterName, const Color &_color, int _radius, int _value)
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
Command_RevealCards::Command_RevealCards(int _gameId, const QString &_zoneName, int _cardId, int _playerId)
	: GameCommand("reveal_cards", _gameId)
{
	insertItem(new SerializableItem_String("zone_name", _zoneName));
	insertItem(new SerializableItem_Int("card_id", _cardId));
	insertItem(new SerializableItem_Int("player_id", _playerId));
}
Event_ConnectionStateChanged::Event_ConnectionStateChanged(int _playerId, bool _connected)
	: GameEvent("connection_state_changed", _playerId)
{
	insertItem(new SerializableItem_Bool("connected", _connected));
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
Event_Kicked::Event_Kicked(int _playerId)
	: GameEvent("kicked", _playerId)
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
Event_MoveCard::Event_MoveCard(int _playerId, int _cardId, const QString &_cardName, const QString &_startZone, int _position, int _targetPlayerId, const QString &_targetZone, int _x, int _y, int _newCardId, bool _faceDown)
	: GameEvent("move_card", _playerId)
{
	insertItem(new SerializableItem_Int("card_id", _cardId));
	insertItem(new SerializableItem_String("card_name", _cardName));
	insertItem(new SerializableItem_String("start_zone", _startZone));
	insertItem(new SerializableItem_Int("position", _position));
	insertItem(new SerializableItem_Int("target_player_id", _targetPlayerId));
	insertItem(new SerializableItem_String("target_zone", _targetZone));
	insertItem(new SerializableItem_Int("x", _x));
	insertItem(new SerializableItem_Int("y", _y));
	insertItem(new SerializableItem_Int("new_card_id", _newCardId));
	insertItem(new SerializableItem_Bool("face_down", _faceDown));
}
Event_FlipCard::Event_FlipCard(int _playerId, const QString &_zone, int _cardId, const QString &_cardName, bool _faceDown)
	: GameEvent("flip_card", _playerId)
{
	insertItem(new SerializableItem_String("zone", _zone));
	insertItem(new SerializableItem_Int("card_id", _cardId));
	insertItem(new SerializableItem_String("card_name", _cardName));
	insertItem(new SerializableItem_Bool("face_down", _faceDown));
}
Event_DestroyCard::Event_DestroyCard(int _playerId, const QString &_zone, int _cardId)
	: GameEvent("destroy_card", _playerId)
{
	insertItem(new SerializableItem_String("zone", _zone));
	insertItem(new SerializableItem_Int("card_id", _cardId));
}
Event_AttachCard::Event_AttachCard(int _playerId, const QString &_startZone, int _cardId, int _targetPlayerId, const QString &_targetZone, int _targetCardId)
	: GameEvent("attach_card", _playerId)
{
	insertItem(new SerializableItem_String("start_zone", _startZone));
	insertItem(new SerializableItem_Int("card_id", _cardId));
	insertItem(new SerializableItem_Int("target_player_id", _targetPlayerId));
	insertItem(new SerializableItem_String("target_zone", _targetZone));
	insertItem(new SerializableItem_Int("target_card_id", _targetCardId));
}
Event_CreateToken::Event_CreateToken(int _playerId, const QString &_zone, int _cardId, const QString &_cardName, const QString &_color, const QString &_pt, const QString &_annotation, bool _destroyOnZoneChange, int _x, int _y)
	: GameEvent("create_token", _playerId)
{
	insertItem(new SerializableItem_String("zone", _zone));
	insertItem(new SerializableItem_Int("card_id", _cardId));
	insertItem(new SerializableItem_String("card_name", _cardName));
	insertItem(new SerializableItem_String("color", _color));
	insertItem(new SerializableItem_String("pt", _pt));
	insertItem(new SerializableItem_String("annotation", _annotation));
	insertItem(new SerializableItem_Bool("destroy_on_zone_change", _destroyOnZoneChange));
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
Event_RemoveFromList::Event_RemoveFromList(const QString &_list, const QString &_userName)
	: GenericEvent("remove_from_list")
{
	insertItem(new SerializableItem_String("list", _list));
	insertItem(new SerializableItem_String("user_name", _userName));
}
Event_ServerMessage::Event_ServerMessage(const QString &_message)
	: GenericEvent("server_message")
{
	insertItem(new SerializableItem_String("message", _message));
}
Event_ServerShutdown::Event_ServerShutdown(const QString &_reason, int _minutes)
	: GenericEvent("server_shutdown")
{
	insertItem(new SerializableItem_String("reason", _reason));
	insertItem(new SerializableItem_Int("minutes", _minutes));
}
Event_ConnectionClosed::Event_ConnectionClosed(const QString &_reason)
	: GenericEvent("connection_closed")
{
	insertItem(new SerializableItem_String("reason", _reason));
}
Event_Message::Event_Message(const QString &_senderName, const QString &_receiverName, const QString &_text)
	: GenericEvent("message")
{
	insertItem(new SerializableItem_String("sender_name", _senderName));
	insertItem(new SerializableItem_String("receiver_name", _receiverName));
	insertItem(new SerializableItem_String("text", _text));
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
Event_UserLeft::Event_UserLeft(const QString &_userName)
	: GenericEvent("user_left")
{
	insertItem(new SerializableItem_String("user_name", _userName));
}
Event_LeaveRoom::Event_LeaveRoom(int _roomId, const QString &_playerName)
	: RoomEvent("leave_room", _roomId)
{
	insertItem(new SerializableItem_String("player_name", _playerName));
}
Event_RoomSay::Event_RoomSay(int _roomId, const QString &_playerName, const QString &_message)
	: RoomEvent("room_say", _roomId)
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
Context_UndoDraw::Context_UndoDraw()
	: GameEventContext("undo_draw")
{
}
Context_MoveCard::Context_MoveCard()
	: GameEventContext("move_card")
{
}
Context_Mulligan::Context_Mulligan(int _number)
	: GameEventContext("mulligan")
{
	insertItem(new SerializableItem_Int("number", _number));
}
Command_UpdateServerMessage::Command_UpdateServerMessage()
	: AdminCommand("update_server_message")
{
}
Command_ShutdownServer::Command_ShutdownServer(const QString &_reason, int _minutes)
	: AdminCommand("shutdown_server")
{
	insertItem(new SerializableItem_String("reason", _reason));
	insertItem(new SerializableItem_Int("minutes", _minutes));
}
Command_BanFromServer::Command_BanFromServer(const QString &_userName, int _minutes, const QString &_reason)
	: ModeratorCommand("ban_from_server")
{
	insertItem(new SerializableItem_String("user_name", _userName));
	insertItem(new SerializableItem_Int("minutes", _minutes));
	insertItem(new SerializableItem_String("reason", _reason));
}
void ProtocolItem::initializeHashAuto()
{
	itemNameHash.insert("cmdping", Command_Ping::newItem);
	itemNameHash.insert("cmdlogin", Command_Login::newItem);
	itemNameHash.insert("cmdmessage", Command_Message::newItem);
	itemNameHash.insert("cmdlist_users", Command_ListUsers::newItem);
	itemNameHash.insert("cmdget_user_info", Command_GetUserInfo::newItem);
	itemNameHash.insert("cmdadd_to_list", Command_AddToList::newItem);
	itemNameHash.insert("cmdremove_from_list", Command_RemoveFromList::newItem);
	itemNameHash.insert("cmddeck_list", Command_DeckList::newItem);
	itemNameHash.insert("cmddeck_new_dir", Command_DeckNewDir::newItem);
	itemNameHash.insert("cmddeck_del_dir", Command_DeckDelDir::newItem);
	itemNameHash.insert("cmddeck_del", Command_DeckDel::newItem);
	itemNameHash.insert("cmddeck_download", Command_DeckDownload::newItem);
	itemNameHash.insert("cmdlist_rooms", Command_ListRooms::newItem);
	itemNameHash.insert("cmdjoin_room", Command_JoinRoom::newItem);
	itemNameHash.insert("cmdleave_room", Command_LeaveRoom::newItem);
	itemNameHash.insert("cmdroom_say", Command_RoomSay::newItem);
	itemNameHash.insert("cmdjoin_game", Command_JoinGame::newItem);
	itemNameHash.insert("cmdkick_from_game", Command_KickFromGame::newItem);
	itemNameHash.insert("cmdleave_game", Command_LeaveGame::newItem);
	itemNameHash.insert("cmdsay", Command_Say::newItem);
	itemNameHash.insert("cmdshuffle", Command_Shuffle::newItem);
	itemNameHash.insert("cmdmulligan", Command_Mulligan::newItem);
	itemNameHash.insert("cmdroll_die", Command_RollDie::newItem);
	itemNameHash.insert("cmddraw_cards", Command_DrawCards::newItem);
	itemNameHash.insert("cmdundo_draw", Command_UndoDraw::newItem);
	itemNameHash.insert("cmdflip_card", Command_FlipCard::newItem);
	itemNameHash.insert("cmdattach_card", Command_AttachCard::newItem);
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
	itemNameHash.insert("cmdreveal_cards", Command_RevealCards::newItem);
	itemNameHash.insert("game_eventconnection_state_changed", Event_ConnectionStateChanged::newItem);
	itemNameHash.insert("game_eventsay", Event_Say::newItem);
	itemNameHash.insert("game_eventleave", Event_Leave::newItem);
	itemNameHash.insert("game_eventgame_closed", Event_GameClosed::newItem);
	itemNameHash.insert("game_eventkicked", Event_Kicked::newItem);
	itemNameHash.insert("game_eventshuffle", Event_Shuffle::newItem);
	itemNameHash.insert("game_eventroll_die", Event_RollDie::newItem);
	itemNameHash.insert("game_eventmove_card", Event_MoveCard::newItem);
	itemNameHash.insert("game_eventflip_card", Event_FlipCard::newItem);
	itemNameHash.insert("game_eventdestroy_card", Event_DestroyCard::newItem);
	itemNameHash.insert("game_eventattach_card", Event_AttachCard::newItem);
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
	itemNameHash.insert("generic_eventremove_from_list", Event_RemoveFromList::newItem);
	itemNameHash.insert("generic_eventserver_message", Event_ServerMessage::newItem);
	itemNameHash.insert("generic_eventserver_shutdown", Event_ServerShutdown::newItem);
	itemNameHash.insert("generic_eventconnection_closed", Event_ConnectionClosed::newItem);
	itemNameHash.insert("generic_eventmessage", Event_Message::newItem);
	itemNameHash.insert("generic_eventgame_joined", Event_GameJoined::newItem);
	itemNameHash.insert("generic_eventuser_left", Event_UserLeft::newItem);
	itemNameHash.insert("room_eventleave_room", Event_LeaveRoom::newItem);
	itemNameHash.insert("room_eventroom_say", Event_RoomSay::newItem);
	itemNameHash.insert("game_event_contextready_start", Context_ReadyStart::newItem);
	itemNameHash.insert("game_event_contextconcede", Context_Concede::newItem);
	itemNameHash.insert("game_event_contextdeck_select", Context_DeckSelect::newItem);
	itemNameHash.insert("game_event_contextundo_draw", Context_UndoDraw::newItem);
	itemNameHash.insert("game_event_contextmove_card", Context_MoveCard::newItem);
	itemNameHash.insert("game_event_contextmulligan", Context_Mulligan::newItem);
	itemNameHash.insert("cmdupdate_server_message", Command_UpdateServerMessage::newItem);
	itemNameHash.insert("cmdshutdown_server", Command_ShutdownServer::newItem);
	itemNameHash.insert("cmdban_from_server", Command_BanFromServer::newItem);
}
