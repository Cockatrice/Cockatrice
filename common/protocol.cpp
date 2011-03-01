#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include "protocol.h"
#include "protocol_items.h"
#include "decklist.h"

ProtocolItem::ProtocolItem(const QString &_itemType, const QString &_itemSubType)
	: SerializableItem_Map(_itemType, _itemSubType), receiverMayDelete(true)
{
}

void ProtocolItem::initializeHash()
{
	initializeHashAuto();
	
	registerSerializableItem("move_card_to_zone", MoveCardToZone::newItem);
	registerSerializableItem("room", ServerInfo_Room::newItem);
	registerSerializableItem("user", ServerInfo_User::newItem);
	registerSerializableItem("game", ServerInfo_Game::newItem);
	registerSerializableItem("game_type", ServerInfo_GameType::newItem);
	registerSerializableItem("card_counter", ServerInfo_CardCounter::newItem);
	registerSerializableItem("card", ServerInfo_Card::newItem);
	registerSerializableItem("zone", ServerInfo_Zone::newItem);
	registerSerializableItem("counter", ServerInfo_Counter::newItem);
	registerSerializableItem("arrow", ServerInfo_Arrow::newItem);
	registerSerializableItem("player_properties", ServerInfo_PlayerProperties::newItem);
	registerSerializableItem("player", ServerInfo_Player::newItem);
	registerSerializableItem("player_ping", ServerInfo_PlayerPing::newItem);
	registerSerializableItem("file", DeckList_File::newItem);
	registerSerializableItem("directory", DeckList_Directory::newItem);
	registerSerializableItem("card_to_move", CardToMove::newItem);
	registerSerializableItem("game_type_id", GameTypeId::newItem);
	
	registerSerializableItem("containercmd", CommandContainer::newItem);
	registerSerializableItem("containergame_event", GameEventContainer::newItem);
	
	registerSerializableItem("cmdcreate_game", Command_CreateGame::newItem);
	registerSerializableItem("cmddeck_upload", Command_DeckUpload::newItem);
	registerSerializableItem("cmddeck_select", Command_DeckSelect::newItem);
	registerSerializableItem("cmdset_sideboard_plan", Command_SetSideboardPlan::newItem);
	registerSerializableItem("cmdmove_card", Command_MoveCard::newItem);
	
	registerSerializableItem("resp", ProtocolResponse::newItem);
	ProtocolResponse::initializeHash();
	registerSerializableItem("respjoin_room", Response_JoinRoom::newItem);
	registerSerializableItem("resplist_users", Response_ListUsers::newItem);
	registerSerializableItem("respget_user_info", Response_GetUserInfo::newItem);
	registerSerializableItem("respdeck_list", Response_DeckList::newItem);
	registerSerializableItem("respdeck_download", Response_DeckDownload::newItem);
	registerSerializableItem("respdeck_upload", Response_DeckUpload::newItem);
	registerSerializableItem("respdump_zone", Response_DumpZone::newItem);
	registerSerializableItem("resplogin", Response_Login::newItem);
	
	registerSerializableItem("room_eventlist_games", Event_ListGames::newItem);
	registerSerializableItem("room_eventjoin_room", Event_JoinRoom::newItem);
	registerSerializableItem("generic_eventadd_to_list", Event_AddToList::newItem);
	registerSerializableItem("generic_eventuser_joined", Event_UserJoined::newItem);
	registerSerializableItem("generic_eventlist_rooms", Event_ListRooms::newItem);
	registerSerializableItem("game_eventjoin", Event_Join::newItem);
	registerSerializableItem("game_eventgame_state_changed", Event_GameStateChanged::newItem);
	registerSerializableItem("game_eventplayer_properties_changed", Event_PlayerPropertiesChanged::newItem);
	registerSerializableItem("game_eventcreate_arrows", Event_CreateArrows::newItem);
	registerSerializableItem("game_eventcreate_counters", Event_CreateCounters::newItem);
	registerSerializableItem("game_eventdraw_cards", Event_DrawCards::newItem);
	registerSerializableItem("game_eventreveal_cards", Event_RevealCards::newItem);
	registerSerializableItem("game_eventping", Event_Ping::newItem);
}

TopLevelProtocolItem::TopLevelProtocolItem()
	: SerializableItem(QString()), currentItem(0)
{
}

bool TopLevelProtocolItem::readCurrentItem(QXmlStreamReader *xml)
{
	if (currentItem) {
		if (currentItem->readElement(xml)) {
			emit protocolItemReceived(currentItem);
			currentItem = 0;
		}
		return true;
	} else
		return false;
}

bool TopLevelProtocolItem::readElement(QXmlStreamReader *xml)
{
	if (!readCurrentItem(xml) && (xml->isStartElement())) {
		QString childName = xml->name().toString();
		QString childSubType = xml->attributes().value("type").toString();
		
		currentItem = dynamic_cast<ProtocolItem *>(getNewItem(childName + childSubType));
		if (!currentItem)
			currentItem = new ProtocolItem_Invalid;
		
		readCurrentItem(xml);
	}
	return SerializableItem::readElement(xml);
}

void TopLevelProtocolItem::writeElement(QXmlStreamWriter * /*xml*/)
{
}

int CommandContainer::lastCmdId = 0;

Command::Command(const QString &_itemName)
	: ProtocolItem("cmd", _itemName)
{
}

void Command::processResponse(ProtocolResponse *response)
{
	emit finished(response);
	emit finished(response->getResponseCode());
}

CommandContainer::CommandContainer(const QList<Command *> &_commandList, int _cmdId)
	: ProtocolItem("container", "cmd"), ticks(0), resp(0), gameEventQueuePublic(0), gameEventQueueOmniscient(0), gameEventQueuePrivate(0), privatePlayerId(-1)
{
	if (_cmdId == -1)
		_cmdId = lastCmdId++;
	insertItem(new SerializableItem_Int("cmd_id", _cmdId));
	
	for (int i = 0; i < _commandList.size(); ++i)
		itemList.append(_commandList[i]);
}

void CommandContainer::processResponse(ProtocolResponse *response)
{
	emit finished(response);
	emit finished(response->getResponseCode());
	
	const QList<Command *> &cmdList = getCommandList();
	for (int i = 0; i < cmdList.size(); ++i)
		cmdList[i]->processResponse(response);
}

void CommandContainer::setResponse(ProtocolResponse *_resp)
{
	delete resp;
	resp = _resp;
}

void CommandContainer::enqueueGameEventPublic(GameEvent *event, int gameId, GameEventContext *context)
{
	if (!gameEventQueuePublic)
		gameEventQueuePublic = new GameEventContainer(QList<GameEvent *>(), gameId);
	gameEventQueuePublic->addGameEvent(event);
	if (context)
		gameEventQueuePublic->setContext(context);
}

void CommandContainer::enqueueGameEventOmniscient(GameEvent *event, int gameId, GameEventContext *context)
{
	if (!gameEventQueueOmniscient)
		gameEventQueueOmniscient = new GameEventContainer(QList<GameEvent *>(), gameId);
	gameEventQueueOmniscient->addGameEvent(event);
	if (context)
		gameEventQueueOmniscient->setContext(context);
}

void CommandContainer::enqueueGameEventPrivate(GameEvent *event, int gameId, int playerId, GameEventContext *context)
{
	if (!gameEventQueuePrivate)
		gameEventQueuePrivate = new GameEventContainer(QList<GameEvent *>(), gameId);
	gameEventQueuePrivate->addGameEvent(event);
	privatePlayerId = playerId;
	if (context)
		gameEventQueuePrivate->setContext(context);
}

Command_CreateGame::Command_CreateGame(int _roomId, const QString &_description, const QString &_password, int _maxPlayers, const QList<GameTypeId *> &_gameTypes, bool _onlyBuddies, bool _onlyRegistered, bool _spectatorsAllowed, bool _spectatorsNeedPassword, bool _spectatorsCanTalk, bool _spectatorsSeeEverything)
	: RoomCommand("create_game", _roomId)
{
	insertItem(new SerializableItem_String("description", _description));
	insertItem(new SerializableItem_String("password", _password));
	insertItem(new SerializableItem_Int("max_players", _maxPlayers));
	insertItem(new SerializableItem_Bool("only_buddies", _onlyBuddies));
	insertItem(new SerializableItem_Bool("only_registered", _onlyRegistered));
	insertItem(new SerializableItem_Bool("spectators_allowed", _spectatorsAllowed));
	insertItem(new SerializableItem_Bool("spectators_need_password", _spectatorsNeedPassword));
	insertItem(new SerializableItem_Bool("spectators_can_talk", _spectatorsCanTalk));
	insertItem(new SerializableItem_Bool("spectators_see_everything", _spectatorsSeeEverything));
	
	for (int i = 0; i < _gameTypes.size(); ++i)
		itemList.append(_gameTypes[i]);
}

Command_DeckUpload::Command_DeckUpload(DeckList *_deck, const QString &_path)
	: Command("deck_upload")
{
	insertItem(new SerializableItem_String("path", _path));
	if (!_deck)
		_deck = new DeckList;
	insertItem(_deck);
}

DeckList *Command_DeckUpload::getDeck() const
{
	return static_cast<DeckList *>(itemMap.value("cockatrice_deck"));
}

Command_DeckSelect::Command_DeckSelect(int _gameId, DeckList *_deck, int _deckId)
	: GameCommand("deck_select", _gameId)
{
	insertItem(new SerializableItem_Int("deck_id", _deckId));
	if (!_deck)
		_deck = new DeckList;
	insertItem(_deck);
}

DeckList *Command_DeckSelect::getDeck() const
{
	return static_cast<DeckList *>(itemMap.value("cockatrice_deck"));
}

Command_SetSideboardPlan::Command_SetSideboardPlan(int _gameId, const QList<MoveCardToZone *> &_moveList)
	: GameCommand("set_sideboard_plan", _gameId)
{
	for (int i = 0; i < _moveList.size(); ++i)
		itemList.append(_moveList[i]);
}

QList<MoveCardToZone *> Command_SetSideboardPlan::getMoveList() const
{
	return typecastItemList<MoveCardToZone *>();
}

Command_MoveCard::Command_MoveCard(int _gameId, const QString &_startZone, const QList<CardToMove *> &_cards, int _targetPlayerId, const QString &_targetZone, int _x, int _y, bool _faceDown)
	: GameCommand("move_card", _gameId)
{
	insertItem(new SerializableItem_String("start_zone", _startZone));
	insertItem(new SerializableItem_Int("target_player_id", _targetPlayerId));
	insertItem(new SerializableItem_String("target_zone", _targetZone));
	insertItem(new SerializableItem_Int("x", _x));
	insertItem(new SerializableItem_Int("y", _y));
	insertItem(new SerializableItem_Bool("face_down", _faceDown));

	for (int i = 0; i < _cards.size(); ++i)
		itemList.append(_cards[i]);
}

QHash<QString, ResponseCode> ProtocolResponse::responseHash;

ProtocolResponse::ProtocolResponse(int _cmdId, ResponseCode _responseCode, const QString &_itemName)
	: ProtocolItem("resp", _itemName)
{
	insertItem(new SerializableItem_Int("cmd_id", _cmdId));
	insertItem(new SerializableItem_String("response_code", responseHash.key(_responseCode)));
}

void ProtocolResponse::initializeHash()
{
	responseHash.insert(QString(), RespNothing);
	responseHash.insert("ok", RespOk);
	responseHash.insert("internal_error", RespInternalError);
	responseHash.insert("invalid_command", RespInvalidCommand);
	responseHash.insert("name_not_found", RespNameNotFound);
	responseHash.insert("login_needed", RespLoginNeeded);
	responseHash.insert("function_not_allowed", RespFunctionNotAllowed);
	responseHash.insert("game_not_started", RespGameNotStarted);
	responseHash.insert("game_full", RespGameFull);
	responseHash.insert("context_error", RespContextError);
	responseHash.insert("wrong_password", RespWrongPassword);
	responseHash.insert("spectators_not_allowed", RespSpectatorsNotAllowed);
	responseHash.insert("only_buddies", RespOnlyBuddies);
	responseHash.insert("user_level_too_low", RespUserLevelTooLow);
}

Response_JoinRoom::Response_JoinRoom(int _cmdId, ResponseCode _responseCode, ServerInfo_Room *_roomInfo)
	: ProtocolResponse(_cmdId, _responseCode, "join_room")
{
	if (!_roomInfo)
		_roomInfo = new ServerInfo_Room;
	insertItem(_roomInfo);
}

Response_ListUsers::Response_ListUsers(int _cmdId, ResponseCode _responseCode, const QList<ServerInfo_User *> &_userList)
	: ProtocolResponse(_cmdId, _responseCode, "list_users")
{
	for (int i = 0; i < _userList.size(); ++i)
		itemList.append(_userList[i]);
}

Response_DeckList::Response_DeckList(int _cmdId, ResponseCode _responseCode, DeckList_Directory *_root)
	: ProtocolResponse(_cmdId, _responseCode, "deck_list")
{
	if (!_root)
		_root = new DeckList_Directory;
	insertItem(_root);
}

Response_GetUserInfo::Response_GetUserInfo(int _cmdId, ResponseCode _responseCode, ServerInfo_User *_user)
	: ProtocolResponse(_cmdId, _responseCode, "get_user_info")
{
	if (!_user)
		_user = new ServerInfo_User;
	insertItem(_user);
}

Response_DeckDownload::Response_DeckDownload(int _cmdId, ResponseCode _responseCode, DeckList *_deck)
	: ProtocolResponse(_cmdId, _responseCode, "deck_download")
{
	if (!_deck)
		_deck = new DeckList;
	insertItem(_deck);
}

DeckList *Response_DeckDownload::getDeck() const
{
	return static_cast<DeckList *>(itemMap.value("cockatrice_deck"));
}

Response_DeckUpload::Response_DeckUpload(int _cmdId, ResponseCode _responseCode, DeckList_File *_file)
	: ProtocolResponse(_cmdId, _responseCode, "deck_upload")
{
	if (!_file)
		_file = new DeckList_File;
	insertItem(_file);
}

Response_DumpZone::Response_DumpZone(int _cmdId, ResponseCode _responseCode, ServerInfo_Zone *_zone)
	: ProtocolResponse(_cmdId, _responseCode, "dump_zone")
{
	if (!_zone)
		_zone = new ServerInfo_Zone;
	insertItem(_zone);
}

Response_Login::Response_Login(int _cmdId, ResponseCode _responseCode, ServerInfo_User *_userInfo, const QList<ServerInfo_User *> &_buddyList, const QList<ServerInfo_User *> &_ignoreList)
	: ProtocolResponse(_cmdId, _responseCode, "login")
{
	if (!_userInfo)
		_userInfo = new ServerInfo_User;
	insertItem(_userInfo);
	
	insertItem(new ServerInfo_UserList("buddy_list", _buddyList));
	insertItem(new ServerInfo_UserList("ignore_list", _ignoreList));
}

GameEvent::GameEvent(const QString &_eventName, int _playerId)
	: ProtocolItem("game_event", _eventName)
{
	insertItem(new SerializableItem_Int("player_id", _playerId));
}

GameEventContext::GameEventContext(const QString &_contextName)
	: ProtocolItem("game_event_context", _contextName)
{
}

RoomEvent::RoomEvent(const QString &_eventName, int _roomId)
	: ProtocolItem("room_event", _eventName)
{
	insertItem(new SerializableItem_Int("room_id", _roomId));
}

Event_ListRooms::Event_ListRooms(const QList<ServerInfo_Room *> &_roomList)
	: GenericEvent("list_rooms")
{
	for (int i = 0; i < _roomList.size(); ++i)
		itemList.append(_roomList[i]);
}

Event_JoinRoom::Event_JoinRoom(int _roomId, ServerInfo_User *_info)
	: RoomEvent("join_room", _roomId)
{
	if (!_info)
		_info = new ServerInfo_User;
	insertItem(_info);
}

Event_ListGames::Event_ListGames(int _roomId, const QList<ServerInfo_Game *> &_gameList)
	: RoomEvent("list_games", _roomId)
{
	for (int i = 0; i < _gameList.size(); ++i)
		itemList.append(_gameList[i]);
}

Event_AddToList::Event_AddToList(const QString &_list, ServerInfo_User *_userInfo)
	: GenericEvent("add_to_list")
{
	if (!_userInfo)
		_userInfo = new ServerInfo_User;
	insertItem(_userInfo);
	insertItem(new SerializableItem_String("list", _list));
}

Event_UserJoined::Event_UserJoined(ServerInfo_User *_userInfo)
	: GenericEvent("user_joined")
{
	if (!_userInfo)
		_userInfo = new ServerInfo_User;
	insertItem(_userInfo);
}

Event_Join::Event_Join(ServerInfo_PlayerProperties *player)
	: GameEvent("join", -1)
{
	if (!player)
		player = new ServerInfo_PlayerProperties;
	insertItem(player);
}

GameEventContainer::GameEventContainer(const QList<GameEvent *> &_eventList, int _gameId, GameEventContext *_context)
	: ProtocolItem("container", "game_event")
{
	insertItem(new SerializableItem_Int("game_id", _gameId));
	
	context = _context;
	if (_context)
		itemList.append(_context);

	eventList = _eventList;
	for (int i = 0; i < _eventList.size(); ++i)
		itemList.append(_eventList[i]);
}

void GameEventContainer::extractData()
{
	for (int i = 0; i < itemList.size(); ++i) {
		GameEvent *_event = dynamic_cast<GameEvent *>(itemList[i]);
		GameEventContext *_context = dynamic_cast<GameEventContext *>(itemList[i]);
		if (_event)
			eventList.append(_event);
		else if (_context)
			context = _context;
	}
}

void GameEventContainer::setContext(GameEventContext *_context)
{
	for (int i = 0; i < itemList.size(); ++i) {
		GameEventContext *temp = qobject_cast<GameEventContext *>(itemList[i]);
		if (temp) {
			delete temp;
			itemList.removeAt(i);
			break;
		}
	}
	itemList.append(_context);
	context = _context;
}

void GameEventContainer::addGameEvent(GameEvent *event)
{
	appendItem(event);
	eventList.append(event);
}

GameEventContainer *GameEventContainer::makeNew(GameEvent *event, int _gameId)
{
	return new GameEventContainer(QList<GameEvent *>() << event, _gameId);
}

Event_GameStateChanged::Event_GameStateChanged(bool _gameStarted, int _activePlayer, int _activePhase, const QList<ServerInfo_Player *> &_playerList)
	: GameEvent("game_state_changed", -1)
{
	insertItem(new SerializableItem_Bool("game_started", _gameStarted));
	insertItem(new SerializableItem_Int("active_player", _activePlayer));
	insertItem(new SerializableItem_Int("active_phase", _activePhase));
	for (int i = 0; i < _playerList.size(); ++i)
		itemList.append(_playerList[i]);
}

Event_PlayerPropertiesChanged::Event_PlayerPropertiesChanged(int _playerId, ServerInfo_PlayerProperties *_properties)
	: GameEvent("player_properties_changed", _playerId)
{
	if (!_properties)
		_properties = new ServerInfo_PlayerProperties;
	insertItem(_properties);
}

Event_Ping::Event_Ping(int _secondsElapsed, const QList<ServerInfo_PlayerPing *> &_pingList)
	: GameEvent("ping", -1)
{
	insertItem(new SerializableItem_Int("seconds_elapsed", _secondsElapsed));
	for (int i = 0; i < _pingList.size(); ++i)
		itemList.append(_pingList[i]);
}

Event_CreateArrows::Event_CreateArrows(int _playerId, const QList<ServerInfo_Arrow *> &_arrowList)
	: GameEvent("create_arrows", _playerId)
{
	for (int i = 0; i < _arrowList.size(); ++i)
		itemList.append(_arrowList[i]);
}

Event_CreateCounters::Event_CreateCounters(int _playerId, const QList<ServerInfo_Counter *> &_counterList)
	: GameEvent("create_counters", _playerId)
{
	for (int i = 0; i < _counterList.size(); ++i)
		itemList.append(_counterList[i]);
}

Event_DrawCards::Event_DrawCards(int _playerId, int _numberCards, const QList<ServerInfo_Card *> &_cardList)
	: GameEvent("draw_cards", _playerId)
{
	insertItem(new SerializableItem_Int("number_cards", _numberCards));
	for (int i = 0; i < _cardList.size(); ++i)
		itemList.append(_cardList[i]);
}

Event_RevealCards::Event_RevealCards(int _playerId, const QString &_zoneName, int _cardId, int _otherPlayerId, const QList<ServerInfo_Card *> &_cardList)
	: GameEvent("reveal_cards", _playerId)
{
	insertItem(new SerializableItem_String("zone_name", _zoneName));
	insertItem(new SerializableItem_Int("card_id", _cardId));
	insertItem(new SerializableItem_Int("other_player_id", _otherPlayerId));
	for (int i = 0; i < _cardList.size(); ++i)
		itemList.append(_cardList[i]);
}
