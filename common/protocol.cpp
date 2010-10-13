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
	registerSerializableItem("chat_channel", ServerInfo_ChatChannel::newItem);
	registerSerializableItem("user", ServerInfo_User::newItem);
	registerSerializableItem("game", ServerInfo_Game::newItem);
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
	
	registerSerializableItem("containercmd", CommandContainer::newItem);
	registerSerializableItem("containergame_event", GameEventContainer::newItem);
	
	registerSerializableItem("cmddeck_upload", Command_DeckUpload::newItem);
	registerSerializableItem("cmddeck_select", Command_DeckSelect::newItem);
	registerSerializableItem("cmdset_sideboard_plan", Command_SetSideboardPlan::newItem);
	
	registerSerializableItem("resp", ProtocolResponse::newItem);
	ProtocolResponse::initializeHash();
	registerSerializableItem("resplist_users", Response_ListUsers::newItem);
	registerSerializableItem("respget_user_info", Response_GetUserInfo::newItem);
	registerSerializableItem("respdeck_list", Response_DeckList::newItem);
	registerSerializableItem("respdeck_download", Response_DeckDownload::newItem);
	registerSerializableItem("respdeck_upload", Response_DeckUpload::newItem);
	registerSerializableItem("respdump_zone", Response_DumpZone::newItem);
	
	registerSerializableItem("generic_eventlist_games", Event_ListGames::newItem);
	registerSerializableItem("generic_eventuser_joined", Event_UserJoined::newItem);
	registerSerializableItem("generic_eventlist_chat_channels", Event_ListChatChannels::newItem);
	registerSerializableItem("game_eventjoin", Event_Join::newItem);
	registerSerializableItem("game_eventgame_state_changed", Event_GameStateChanged::newItem);
	registerSerializableItem("game_eventplayer_properties_changed", Event_PlayerPropertiesChanged::newItem);
	registerSerializableItem("game_eventcreate_arrows", Event_CreateArrows::newItem);
	registerSerializableItem("game_eventcreate_counters", Event_CreateCounters::newItem);
	registerSerializableItem("game_eventdraw_cards", Event_DrawCards::newItem);
	registerSerializableItem("game_eventping", Event_Ping::newItem);
	registerSerializableItem("chat_eventchat_list_players", Event_ChatListPlayers::newItem);
	registerSerializableItem("chat_eventchat_join_channel", Event_ChatJoinChannel::newItem);
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
	: ProtocolItem("container", "cmd"), ticks(0), resp(0), gameEventQueuePublic(0), gameEventQueueOmniscient(0), gameEventQueuePrivate(0)
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

void CommandContainer::enqueueGameEventPublic(GameEvent *event, int gameId)
{
	if (!gameEventQueuePublic)
		gameEventQueuePublic = new GameEventContainer(QList<GameEvent *>(), gameId);
	gameEventQueuePublic->addGameEvent(event);
}

void CommandContainer::enqueueGameEventOmniscient(GameEvent *event, int gameId)
{
	if (!gameEventQueueOmniscient)
		gameEventQueueOmniscient = new GameEventContainer(QList<GameEvent *>(), gameId);
	gameEventQueueOmniscient->addGameEvent(event);
}

void CommandContainer::enqueueGameEventPrivate(GameEvent *event, int gameId)
{
	if (!gameEventQueuePrivate)
		gameEventQueuePrivate = new GameEventContainer(QList<GameEvent *>(), gameId);
	gameEventQueuePrivate->addGameEvent(event);
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
	responseHash.insert("invalid_command", RespInvalidCommand);
	responseHash.insert("name_not_found", RespNameNotFound);
	responseHash.insert("login_needed", RespLoginNeeded);
	responseHash.insert("function_not_allowed", RespFunctionNotAllowed);
	responseHash.insert("game_not_started", RespGameNotStarted);
	responseHash.insert("game_full", RespGameFull);
	responseHash.insert("context_error", RespContextError);
	responseHash.insert("wrong_password", RespWrongPassword);
	responseHash.insert("spectators_not_allowed", RespSpectatorsNotAllowed);
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

GameEvent::GameEvent(const QString &_eventName, int _playerId)
	: ProtocolItem("game_event", _eventName)
{
	insertItem(new SerializableItem_Int("player_id", _playerId));
}

GameEventContext::GameEventContext(const QString &_contextName)
	: ProtocolItem("game_event_context", _contextName)
{
}

ChatEvent::ChatEvent(const QString &_eventName, const QString &_channel)
	: ProtocolItem("chat_event", _eventName)
{
	insertItem(new SerializableItem_String("channel", _channel));
}

Event_ListChatChannels::Event_ListChatChannels(const QList<ServerInfo_ChatChannel *> &_channelList)
	: GenericEvent("list_chat_channels")
{
	for (int i = 0; i < _channelList.size(); ++i)
		itemList.append(_channelList[i]);
}

Event_ChatListPlayers::Event_ChatListPlayers(const QString &_channel, const QList<ServerInfo_User *> &_playerList)
	: ChatEvent("chat_list_players", _channel)
{
	for (int i = 0; i < _playerList.size(); ++i)
		itemList.append(_playerList[i]);
}

Event_ChatJoinChannel::Event_ChatJoinChannel(const QString &_channel, ServerInfo_User *_info)
	: ChatEvent("chat_join_channel", _channel)
{
	if (!_info)
		_info = new ServerInfo_User;
	insertItem(_info);
}

Event_ListGames::Event_ListGames(const QList<ServerInfo_Game *> &_gameList)
	: GenericEvent("list_games")
{
	for (int i = 0; i < _gameList.size(); ++i)
		itemList.append(_gameList[i]);
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

Event_Ping::Event_Ping(const QList<ServerInfo_PlayerPing *> &_pingList)
	: GameEvent("ping", -1)
{
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
