#include "protocol.h"
#include "protocol_commands.h"

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
	setParameter("faceDown", faceDown);
}
void Command_MoveCard::extractParameters()
{
	GameCommand::extractParameters();
	startZone = parameters["start_zone"];
	cardId = parameters["card_id"].toInt();
	targetZone = parameters["target_zone"];
	x = parameters["x"].toInt();
	y = parameters["y"].toInt();
	faceDown = (parameters["faceDown"] == "1");
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
void Command::initializeHash()
{
	commandHash.insert("ping", Command_Ping::newCommand);
	commandHash.insert("login", Command_Login::newCommand);
	commandHash.insert("chat_list_channels", Command_ChatListChannels::newCommand);
	commandHash.insert("chat_join_channel", Command_ChatJoinChannel::newCommand);
	commandHash.insert("chat_leave_channel", Command_ChatLeaveChannel::newCommand);
	commandHash.insert("chat_say", Command_ChatSay::newCommand);
	commandHash.insert("list_games", Command_ListGames::newCommand);
	commandHash.insert("create_game", Command_CreateGame::newCommand);
	commandHash.insert("join_game", Command_JoinGame::newCommand);
	commandHash.insert("leave_game", Command_LeaveGame::newCommand);
	commandHash.insert("say", Command_Say::newCommand);
	commandHash.insert("shuffle", Command_Shuffle::newCommand);
	commandHash.insert("roll_die", Command_RollDie::newCommand);
	commandHash.insert("draw_cards", Command_DrawCards::newCommand);
	commandHash.insert("move_card", Command_MoveCard::newCommand);
	commandHash.insert("create_token", Command_CreateToken::newCommand);
	commandHash.insert("create_arrow", Command_CreateArrow::newCommand);
	commandHash.insert("delete_arrow", Command_DeleteArrow::newCommand);
	commandHash.insert("set_card_attr", Command_SetCardAttr::newCommand);
	commandHash.insert("ready_start", Command_ReadyStart::newCommand);
	commandHash.insert("inc_counter", Command_IncCounter::newCommand);
	commandHash.insert("add_counter", Command_AddCounter::newCommand);
	commandHash.insert("set_counter", Command_SetCounter::newCommand);
	commandHash.insert("del_counter", Command_DelCounter::newCommand);
	commandHash.insert("next_turn", Command_NextTurn::newCommand);
	commandHash.insert("set_active_phase", Command_SetActivePhase::newCommand);
	commandHash.insert("dump_zone", Command_DumpZone::newCommand);
	commandHash.insert("stop_dump_zone", Command_StopDumpZone::newCommand);
	commandHash.insert("dump_all", Command_DumpAll::newCommand);
	commandHash.insert("submit_deck", Command_SubmitDeck::newCommand);
}
