#include "protocol.h"
#include "protocol_items.h"

Command_Ping::Command_Ping()
	: Command("ping")
{
}
Command_Login::Command_Login(const QString &_username, const QString &_password)
	: Command("login"), username(_username), password(_password)
{
	setParameter("username", username);
	setParameter("password", password);
}
void Command_Login::extractParameters()
{
	Command::extractParameters();
	username = parameters["username"];
	password = parameters["password"];
}
Command_ChatListChannels::Command_ChatListChannels()
	: Command("chat_list_channels")
{
}
Command_ChatJoinChannel::Command_ChatJoinChannel(const QString &_channel)
	: Command("chat_join_channel"), channel(_channel)
{
	setParameter("channel", channel);
}
void Command_ChatJoinChannel::extractParameters()
{
	Command::extractParameters();
	channel = parameters["channel"];
}
Command_ChatLeaveChannel::Command_ChatLeaveChannel(const QString &_channel)
	: ChatCommand("chat_leave_channel", _channel)
{
}
Command_ChatSay::Command_ChatSay(const QString &_channel, const QString &_message)
	: ChatCommand("chat_say", _channel), message(_message)
{
	setParameter("message", message);
}
void Command_ChatSay::extractParameters()
{
	ChatCommand::extractParameters();
	message = parameters["message"];
}
Command_ListGames::Command_ListGames()
	: Command("list_games")
{
}
Command_CreateGame::Command_CreateGame(const QString &_description, const QString &_password, int _maxPlayers, bool _spectatorsAllowed)
	: Command("create_game"), description(_description), password(_password), maxPlayers(_maxPlayers), spectatorsAllowed(_spectatorsAllowed)
{
	setParameter("description", description);
	setParameter("password", password);
	setParameter("max_players", maxPlayers);
	setParameter("spectators_allowed", spectatorsAllowed);
}
void Command_CreateGame::extractParameters()
{
	Command::extractParameters();
	description = parameters["description"];
	password = parameters["password"];
	maxPlayers = parameters["max_players"].toInt();
	spectatorsAllowed = (parameters["spectators_allowed"] == "1");
}
Command_JoinGame::Command_JoinGame(int _gameId, const QString &_password, bool _spectator)
	: Command("join_game"), gameId(_gameId), password(_password), spectator(_spectator)
{
	setParameter("game_id", gameId);
	setParameter("password", password);
	setParameter("spectator", spectator);
}
void Command_JoinGame::extractParameters()
{
	Command::extractParameters();
	gameId = parameters["game_id"].toInt();
	password = parameters["password"];
	spectator = (parameters["spectator"] == "1");
}
Command_LeaveGame::Command_LeaveGame(int _gameId)
	: GameCommand("leave_game", _gameId)
{
}
Command_Say::Command_Say(int _gameId, const QString &_message)
	: GameCommand("say", _gameId), message(_message)
{
	setParameter("message", message);
}
void Command_Say::extractParameters()
{
	GameCommand::extractParameters();
	message = parameters["message"];
}
Command_Shuffle::Command_Shuffle(int _gameId)
	: GameCommand("shuffle", _gameId)
{
}
Command_RollDie::Command_RollDie(int _gameId, int _sides)
	: GameCommand("roll_die", _gameId), sides(_sides)
{
	setParameter("sides", sides);
}
void Command_RollDie::extractParameters()
{
	GameCommand::extractParameters();
	sides = parameters["sides"].toInt();
}
Command_DrawCards::Command_DrawCards(int _gameId, int _number)
	: GameCommand("draw_cards", _gameId), number(_number)
{
	setParameter("number", number);
}
void Command_DrawCards::extractParameters()
{
	GameCommand::extractParameters();
	number = parameters["number"].toInt();
}
Command_MoveCard::Command_MoveCard(int _gameId, const QString &_startZone, int _cardId, const QString &_targetZone, int _x, int _y, bool _faceDown)
	: GameCommand("move_card", _gameId), startZone(_startZone), cardId(_cardId), targetZone(_targetZone), x(_x), y(_y), faceDown(_faceDown)
{
	setParameter("start_zone", startZone);
	setParameter("card_id", cardId);
	setParameter("target_zone", targetZone);
	setParameter("x", x);
	setParameter("y", y);
	setParameter("face_down", faceDown);
}
void Command_MoveCard::extractParameters()
{
	GameCommand::extractParameters();
	startZone = parameters["start_zone"];
	cardId = parameters["card_id"].toInt();
	targetZone = parameters["target_zone"];
	x = parameters["x"].toInt();
	y = parameters["y"].toInt();
	faceDown = (parameters["face_down"] == "1");
}
Command_CreateToken::Command_CreateToken(int _gameId, const QString &_zone, const QString &_name, const QString &_pt, int _x, int _y)
	: GameCommand("create_token", _gameId), zone(_zone), name(_name), pt(_pt), x(_x), y(_y)
{
	setParameter("zone", zone);
	setParameter("name", name);
	setParameter("pt", pt);
	setParameter("x", x);
	setParameter("y", y);
}
void Command_CreateToken::extractParameters()
{
	GameCommand::extractParameters();
	zone = parameters["zone"];
	name = parameters["name"];
	pt = parameters["pt"];
	x = parameters["x"].toInt();
	y = parameters["y"].toInt();
}
Command_CreateArrow::Command_CreateArrow(int _gameId, int _startPlayerId, const QString &_startZone, int _startCardId, int _targetPlayerId, const QString &_targetPlayerZone, int _targetCardId, int _color)
	: GameCommand("create_arrow", _gameId), startPlayerId(_startPlayerId), startZone(_startZone), startCardId(_startCardId), targetPlayerId(_targetPlayerId), targetPlayerZone(_targetPlayerZone), targetCardId(_targetCardId), color(_color)
{
	setParameter("start_player_id", startPlayerId);
	setParameter("start_zone", startZone);
	setParameter("start_card_id", startCardId);
	setParameter("target_player_id", targetPlayerId);
	setParameter("target_player_zone", targetPlayerZone);
	setParameter("target_card_id", targetCardId);
	setParameter("color", color);
}
void Command_CreateArrow::extractParameters()
{
	GameCommand::extractParameters();
	startPlayerId = parameters["start_player_id"].toInt();
	startZone = parameters["start_zone"];
	startCardId = parameters["start_card_id"].toInt();
	targetPlayerId = parameters["target_player_id"].toInt();
	targetPlayerZone = parameters["target_player_zone"];
	targetCardId = parameters["target_card_id"].toInt();
	color = parameters["color"].toInt();
}
Command_DeleteArrow::Command_DeleteArrow(int _gameId, int _arrowId)
	: GameCommand("delete_arrow", _gameId), arrowId(_arrowId)
{
	setParameter("arrow_id", arrowId);
}
void Command_DeleteArrow::extractParameters()
{
	GameCommand::extractParameters();
	arrowId = parameters["arrow_id"].toInt();
}
Command_SetCardAttr::Command_SetCardAttr(int _gameId, const QString &_zone, int _cardId, const QString &_attrName, const QString &_attrValue)
	: GameCommand("set_card_attr", _gameId), zone(_zone), cardId(_cardId), attrName(_attrName), attrValue(_attrValue)
{
	setParameter("zone", zone);
	setParameter("card_id", cardId);
	setParameter("attr_name", attrName);
	setParameter("attr_value", attrValue);
}
void Command_SetCardAttr::extractParameters()
{
	GameCommand::extractParameters();
	zone = parameters["zone"];
	cardId = parameters["card_id"].toInt();
	attrName = parameters["attr_name"];
	attrValue = parameters["attr_value"];
}
Command_ReadyStart::Command_ReadyStart(int _gameId)
	: GameCommand("ready_start", _gameId)
{
}
Command_IncCounter::Command_IncCounter(int _gameId, int _counterId, int _delta)
	: GameCommand("inc_counter", _gameId), counterId(_counterId), delta(_delta)
{
	setParameter("counter_id", counterId);
	setParameter("delta", delta);
}
void Command_IncCounter::extractParameters()
{
	GameCommand::extractParameters();
	counterId = parameters["counter_id"].toInt();
	delta = parameters["delta"].toInt();
}
Command_AddCounter::Command_AddCounter(int _gameId, const QString &_counterName, int _color, int _radius, int _value)
	: GameCommand("add_counter", _gameId), counterName(_counterName), color(_color), radius(_radius), value(_value)
{
	setParameter("counter_name", counterName);
	setParameter("color", color);
	setParameter("radius", radius);
	setParameter("value", value);
}
void Command_AddCounter::extractParameters()
{
	GameCommand::extractParameters();
	counterName = parameters["counter_name"];
	color = parameters["color"].toInt();
	radius = parameters["radius"].toInt();
	value = parameters["value"].toInt();
}
Command_SetCounter::Command_SetCounter(int _gameId, int _counterId, int _value)
	: GameCommand("set_counter", _gameId), counterId(_counterId), value(_value)
{
	setParameter("counter_id", counterId);
	setParameter("value", value);
}
void Command_SetCounter::extractParameters()
{
	GameCommand::extractParameters();
	counterId = parameters["counter_id"].toInt();
	value = parameters["value"].toInt();
}
Command_DelCounter::Command_DelCounter(int _gameId, int _counterId)
	: GameCommand("del_counter", _gameId), counterId(_counterId)
{
	setParameter("counter_id", counterId);
}
void Command_DelCounter::extractParameters()
{
	GameCommand::extractParameters();
	counterId = parameters["counter_id"].toInt();
}
Command_NextTurn::Command_NextTurn(int _gameId)
	: GameCommand("next_turn", _gameId)
{
}
Command_SetActivePhase::Command_SetActivePhase(int _gameId, int _phase)
	: GameCommand("set_active_phase", _gameId), phase(_phase)
{
	setParameter("phase", phase);
}
void Command_SetActivePhase::extractParameters()
{
	GameCommand::extractParameters();
	phase = parameters["phase"].toInt();
}
Command_DumpZone::Command_DumpZone(int _gameId, int _playerId, const QString &_zoneName, int _numberCards)
	: GameCommand("dump_zone", _gameId), playerId(_playerId), zoneName(_zoneName), numberCards(_numberCards)
{
	setParameter("player_id", playerId);
	setParameter("zone_name", zoneName);
	setParameter("number_cards", numberCards);
}
void Command_DumpZone::extractParameters()
{
	GameCommand::extractParameters();
	playerId = parameters["player_id"].toInt();
	zoneName = parameters["zone_name"];
	numberCards = parameters["number_cards"].toInt();
}
Command_StopDumpZone::Command_StopDumpZone(int _gameId, int _playerId, const QString &_zoneName)
	: GameCommand("stop_dump_zone", _gameId), playerId(_playerId), zoneName(_zoneName)
{
	setParameter("player_id", playerId);
	setParameter("zone_name", zoneName);
}
void Command_StopDumpZone::extractParameters()
{
	GameCommand::extractParameters();
	playerId = parameters["player_id"].toInt();
	zoneName = parameters["zone_name"];
}
Command_DumpAll::Command_DumpAll(int _gameId)
	: GameCommand("dump_all", _gameId)
{
}
Command_SubmitDeck::Command_SubmitDeck(int _gameId)
	: GameCommand("submit_deck", _gameId)
{
}
Event_Say::Event_Say(int _gameId, int _playerId, const QString &_message)
	: GameEvent("say", _gameId, _playerId), message(_message)
{
	setParameter("message", message);
}
void Event_Say::extractParameters()
{
	GameEvent::extractParameters();
	message = parameters["message"];
}
Event_Join::Event_Join(int _gameId, int _playerId, const QString &_playerName, bool _spectator)
	: GameEvent("join", _gameId, _playerId), playerName(_playerName), spectator(_spectator)
{
	setParameter("player_name", playerName);
	setParameter("spectator", spectator);
}
void Event_Join::extractParameters()
{
	GameEvent::extractParameters();
	playerName = parameters["player_name"];
	spectator = (parameters["spectator"] == "1");
}
Event_Leave::Event_Leave(int _gameId, int _playerId)
	: GameEvent("leave", _gameId, _playerId)
{
}
Event_GameClosed::Event_GameClosed(int _gameId, int _playerId)
	: GameEvent("game_closed", _gameId, _playerId)
{
}
Event_ReadyStart::Event_ReadyStart(int _gameId, int _playerId)
	: GameEvent("ready_start", _gameId, _playerId)
{
}
Event_SetupZones::Event_SetupZones(int _gameId, int _playerId, int _deckSize, int _sbSize)
	: GameEvent("setup_zones", _gameId, _playerId), deckSize(_deckSize), sbSize(_sbSize)
{
	setParameter("deck_size", deckSize);
	setParameter("sb_size", sbSize);
}
void Event_SetupZones::extractParameters()
{
	GameEvent::extractParameters();
	deckSize = parameters["deck_size"].toInt();
	sbSize = parameters["sb_size"].toInt();
}
Event_GameStart::Event_GameStart(int _gameId, int _playerId)
	: GameEvent("game_start", _gameId, _playerId)
{
}
Event_Shuffle::Event_Shuffle(int _gameId, int _playerId)
	: GameEvent("shuffle", _gameId, _playerId)
{
}
Event_RollDie::Event_RollDie(int _gameId, int _playerId, int _sides, int _value)
	: GameEvent("roll_die", _gameId, _playerId), sides(_sides), value(_value)
{
	setParameter("sides", sides);
	setParameter("value", value);
}
void Event_RollDie::extractParameters()
{
	GameEvent::extractParameters();
	sides = parameters["sides"].toInt();
	value = parameters["value"].toInt();
}
Event_MoveCard::Event_MoveCard(int _gameId, int _playerId, int _cardId, const QString &_cardName, const QString &_startZone, int _position, const QString &_targetZone, int _x, int _y, bool _faceDown)
	: GameEvent("move_card", _gameId, _playerId), cardId(_cardId), cardName(_cardName), startZone(_startZone), position(_position), targetZone(_targetZone), x(_x), y(_y), faceDown(_faceDown)
{
	setParameter("card_id", cardId);
	setParameter("card_name", cardName);
	setParameter("start_zone", startZone);
	setParameter("position", position);
	setParameter("target_zone", targetZone);
	setParameter("x", x);
	setParameter("y", y);
	setParameter("face_down", faceDown);
}
void Event_MoveCard::extractParameters()
{
	GameEvent::extractParameters();
	cardId = parameters["card_id"].toInt();
	cardName = parameters["card_name"];
	startZone = parameters["start_zone"];
	position = parameters["position"].toInt();
	targetZone = parameters["target_zone"];
	x = parameters["x"].toInt();
	y = parameters["y"].toInt();
	faceDown = (parameters["face_down"] == "1");
}
Event_CreateToken::Event_CreateToken(int _gameId, int _playerId, const QString &_zone, int _cardId, const QString &_cardName, const QString &_pt, int _x, int _y)
	: GameEvent("create_token", _gameId, _playerId), zone(_zone), cardId(_cardId), cardName(_cardName), pt(_pt), x(_x), y(_y)
{
	setParameter("zone", zone);
	setParameter("card_id", cardId);
	setParameter("card_name", cardName);
	setParameter("pt", pt);
	setParameter("x", x);
	setParameter("y", y);
}
void Event_CreateToken::extractParameters()
{
	GameEvent::extractParameters();
	zone = parameters["zone"];
	cardId = parameters["card_id"].toInt();
	cardName = parameters["card_name"];
	pt = parameters["pt"];
	x = parameters["x"].toInt();
	y = parameters["y"].toInt();
}
Event_CreateArrow::Event_CreateArrow(int _gameId, int _playerId, int _arrowId, int _startPlayerId, const QString &_startZone, int _startCardId, int _targetPlayerId, const QString &_targetZone, int _targetCardId, int _color)
	: GameEvent("create_arrow", _gameId, _playerId), arrowId(_arrowId), startPlayerId(_startPlayerId), startZone(_startZone), startCardId(_startCardId), targetPlayerId(_targetPlayerId), targetZone(_targetZone), targetCardId(_targetCardId), color(_color)
{
	setParameter("arrow_id", arrowId);
	setParameter("start_player_id", startPlayerId);
	setParameter("start_zone", startZone);
	setParameter("start_card_id", startCardId);
	setParameter("target_player_id", targetPlayerId);
	setParameter("target_zone", targetZone);
	setParameter("target_card_id", targetCardId);
	setParameter("color", color);
}
void Event_CreateArrow::extractParameters()
{
	GameEvent::extractParameters();
	arrowId = parameters["arrow_id"].toInt();
	startPlayerId = parameters["start_player_id"].toInt();
	startZone = parameters["start_zone"];
	startCardId = parameters["start_card_id"].toInt();
	targetPlayerId = parameters["target_player_id"].toInt();
	targetZone = parameters["target_zone"];
	targetCardId = parameters["target_card_id"].toInt();
	color = parameters["color"].toInt();
}
Event_DeleteArrow::Event_DeleteArrow(int _gameId, int _playerId, int _arrowId)
	: GameEvent("delete_arrow", _gameId, _playerId), arrowId(_arrowId)
{
	setParameter("arrow_id", arrowId);
}
void Event_DeleteArrow::extractParameters()
{
	GameEvent::extractParameters();
	arrowId = parameters["arrow_id"].toInt();
}
Event_SetCardAttr::Event_SetCardAttr(int _gameId, int _playerId, const QString &_zone, int _cardId, const QString &_attrName, const QString &_attrValue)
	: GameEvent("set_card_attr", _gameId, _playerId), zone(_zone), cardId(_cardId), attrName(_attrName), attrValue(_attrValue)
{
	setParameter("zone", zone);
	setParameter("card_id", cardId);
	setParameter("attr_name", attrName);
	setParameter("attr_value", attrValue);
}
void Event_SetCardAttr::extractParameters()
{
	GameEvent::extractParameters();
	zone = parameters["zone"];
	cardId = parameters["card_id"].toInt();
	attrName = parameters["attr_name"];
	attrValue = parameters["attr_value"];
}
Event_AddCounter::Event_AddCounter(int _gameId, int _playerId, int _counterId, const QString &_counterName, int _color, int _radius, int _value)
	: GameEvent("add_counter", _gameId, _playerId), counterId(_counterId), counterName(_counterName), color(_color), radius(_radius), value(_value)
{
	setParameter("counter_id", counterId);
	setParameter("counter_name", counterName);
	setParameter("color", color);
	setParameter("radius", radius);
	setParameter("value", value);
}
void Event_AddCounter::extractParameters()
{
	GameEvent::extractParameters();
	counterId = parameters["counter_id"].toInt();
	counterName = parameters["counter_name"];
	color = parameters["color"].toInt();
	radius = parameters["radius"].toInt();
	value = parameters["value"].toInt();
}
Event_SetCounter::Event_SetCounter(int _gameId, int _playerId, int _counterId, int _value)
	: GameEvent("set_counter", _gameId, _playerId), counterId(_counterId), value(_value)
{
	setParameter("counter_id", counterId);
	setParameter("value", value);
}
void Event_SetCounter::extractParameters()
{
	GameEvent::extractParameters();
	counterId = parameters["counter_id"].toInt();
	value = parameters["value"].toInt();
}
Event_DelCounter::Event_DelCounter(int _gameId, int _playerId, int _counterId)
	: GameEvent("del_counter", _gameId, _playerId), counterId(_counterId)
{
	setParameter("counter_id", counterId);
}
void Event_DelCounter::extractParameters()
{
	GameEvent::extractParameters();
	counterId = parameters["counter_id"].toInt();
}
Event_SetActivePlayer::Event_SetActivePlayer(int _gameId, int _playerId, int _activePlayerId)
	: GameEvent("set_active_player", _gameId, _playerId), activePlayerId(_activePlayerId)
{
	setParameter("active_player_id", activePlayerId);
}
void Event_SetActivePlayer::extractParameters()
{
	GameEvent::extractParameters();
	activePlayerId = parameters["active_player_id"].toInt();
}
Event_SetActivePhase::Event_SetActivePhase(int _gameId, int _playerId, int _phase)
	: GameEvent("set_active_phase", _gameId, _playerId), phase(_phase)
{
	setParameter("phase", phase);
}
void Event_SetActivePhase::extractParameters()
{
	GameEvent::extractParameters();
	phase = parameters["phase"].toInt();
}
Event_DumpZone::Event_DumpZone(int _gameId, int _playerId, int _zoneOwnerId, const QString &_zone, int _numberCards)
	: GameEvent("dump_zone", _gameId, _playerId), zoneOwnerId(_zoneOwnerId), zone(_zone), numberCards(_numberCards)
{
	setParameter("zone_owner_id", zoneOwnerId);
	setParameter("zone", zone);
	setParameter("number_cards", numberCards);
}
void Event_DumpZone::extractParameters()
{
	GameEvent::extractParameters();
	zoneOwnerId = parameters["zone_owner_id"].toInt();
	zone = parameters["zone"];
	numberCards = parameters["number_cards"].toInt();
}
Event_StopDumpZone::Event_StopDumpZone(int _gameId, int _playerId, int _zoneOwnerId, const QString &_zone)
	: GameEvent("stop_dump_zone", _gameId, _playerId), zoneOwnerId(_zoneOwnerId), zone(_zone)
{
	setParameter("zone_owner_id", zoneOwnerId);
	setParameter("zone", zone);
}
void Event_StopDumpZone::extractParameters()
{
	GameEvent::extractParameters();
	zoneOwnerId = parameters["zone_owner_id"].toInt();
	zone = parameters["zone"];
}
void ProtocolItem::initializeHashAuto()
{
	itemNameHash.insert("cmdping", Command_Ping::newItem);
	itemNameHash.insert("cmdlogin", Command_Login::newItem);
	itemNameHash.insert("cmdchat_list_channels", Command_ChatListChannels::newItem);
	itemNameHash.insert("cmdchat_join_channel", Command_ChatJoinChannel::newItem);
	itemNameHash.insert("cmdchat_leave_channel", Command_ChatLeaveChannel::newItem);
	itemNameHash.insert("cmdchat_say", Command_ChatSay::newItem);
	itemNameHash.insert("cmdlist_games", Command_ListGames::newItem);
	itemNameHash.insert("cmdcreate_game", Command_CreateGame::newItem);
	itemNameHash.insert("cmdjoin_game", Command_JoinGame::newItem);
	itemNameHash.insert("cmdleave_game", Command_LeaveGame::newItem);
	itemNameHash.insert("cmdsay", Command_Say::newItem);
	itemNameHash.insert("cmdshuffle", Command_Shuffle::newItem);
	itemNameHash.insert("cmdroll_die", Command_RollDie::newItem);
	itemNameHash.insert("cmddraw_cards", Command_DrawCards::newItem);
	itemNameHash.insert("cmdmove_card", Command_MoveCard::newItem);
	itemNameHash.insert("cmdcreate_token", Command_CreateToken::newItem);
	itemNameHash.insert("cmdcreate_arrow", Command_CreateArrow::newItem);
	itemNameHash.insert("cmddelete_arrow", Command_DeleteArrow::newItem);
	itemNameHash.insert("cmdset_card_attr", Command_SetCardAttr::newItem);
	itemNameHash.insert("cmdready_start", Command_ReadyStart::newItem);
	itemNameHash.insert("cmdinc_counter", Command_IncCounter::newItem);
	itemNameHash.insert("cmdadd_counter", Command_AddCounter::newItem);
	itemNameHash.insert("cmdset_counter", Command_SetCounter::newItem);
	itemNameHash.insert("cmddel_counter", Command_DelCounter::newItem);
	itemNameHash.insert("cmdnext_turn", Command_NextTurn::newItem);
	itemNameHash.insert("cmdset_active_phase", Command_SetActivePhase::newItem);
	itemNameHash.insert("cmddump_zone", Command_DumpZone::newItem);
	itemNameHash.insert("cmdstop_dump_zone", Command_StopDumpZone::newItem);
	itemNameHash.insert("cmddump_all", Command_DumpAll::newItem);
	itemNameHash.insert("cmdsubmit_deck", Command_SubmitDeck::newItem);
	itemNameHash.insert("game_eventsay", Event_Say::newItem);
	itemNameHash.insert("game_eventjoin", Event_Join::newItem);
	itemNameHash.insert("game_eventleave", Event_Leave::newItem);
	itemNameHash.insert("game_eventgame_closed", Event_GameClosed::newItem);
	itemNameHash.insert("game_eventready_start", Event_ReadyStart::newItem);
	itemNameHash.insert("game_eventsetup_zones", Event_SetupZones::newItem);
	itemNameHash.insert("game_eventgame_start", Event_GameStart::newItem);
	itemNameHash.insert("game_eventshuffle", Event_Shuffle::newItem);
	itemNameHash.insert("game_eventroll_die", Event_RollDie::newItem);
	itemNameHash.insert("game_eventmove_card", Event_MoveCard::newItem);
	itemNameHash.insert("game_eventcreate_token", Event_CreateToken::newItem);
	itemNameHash.insert("game_eventcreate_arrow", Event_CreateArrow::newItem);
	itemNameHash.insert("game_eventdelete_arrow", Event_DeleteArrow::newItem);
	itemNameHash.insert("game_eventset_card_attr", Event_SetCardAttr::newItem);
	itemNameHash.insert("game_eventadd_counter", Event_AddCounter::newItem);
	itemNameHash.insert("game_eventset_counter", Event_SetCounter::newItem);
	itemNameHash.insert("game_eventdel_counter", Event_DelCounter::newItem);
	itemNameHash.insert("game_eventset_active_player", Event_SetActivePlayer::newItem);
	itemNameHash.insert("game_eventset_active_phase", Event_SetActivePhase::newItem);
	itemNameHash.insert("game_eventdump_zone", Event_DumpZone::newItem);
	itemNameHash.insert("game_eventstop_dump_zone", Event_StopDumpZone::newItem);
}
