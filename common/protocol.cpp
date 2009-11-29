#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>
#include "protocol.h"
#include "protocol_items.h"
#include "decklist.h"

ProtocolItem::ProtocolItem(const QString &_itemType, const QString &_itemSubType)
	: SerializableItem_Map(_itemType, _itemSubType)
{
}

void ProtocolItem::initializeHash()
{
	initializeHashAuto();
	
	registerSerializableItem("chat_channel", ServerInfo_ChatChannel::newItem);
	registerSerializableItem("chat_user", ServerInfo_ChatUser::newItem);
	registerSerializableItem("game", ServerInfo_Game::newItem);
	registerSerializableItem("card", ServerInfo_Card::newItem);
	registerSerializableItem("zone", ServerInfo_Zone::newItem);
	registerSerializableItem("counter", ServerInfo_Counter::newItem);
	registerSerializableItem("arrow", ServerInfo_Arrow::newItem);
	registerSerializableItem("player", ServerInfo_Player::newItem);
	registerSerializableItem("file", DeckList_File::newItem);
	registerSerializableItem("directory", DeckList_Directory::newItem);
	
	registerSerializableItem("cmddeck_upload", Command_DeckUpload::newItem);
	registerSerializableItem("cmddeck_select", Command_DeckSelect::newItem);
	
	registerSerializableItem("resp", ProtocolResponse::newItem);
	ProtocolResponse::initializeHash();
	registerSerializableItem("respdeck_list", Response_DeckList::newItem);
	registerSerializableItem("respdeck_download", Response_DeckDownload::newItem);
	registerSerializableItem("respdeck_upload", Response_DeckUpload::newItem);
	registerSerializableItem("respdump_zone", Response_DumpZone::newItem);
	
	registerSerializableItem("generic_eventlist_games", Event_ListGames::newItem);
	registerSerializableItem("generic_eventlist_chat_channels", Event_ListChatChannels::newItem);
	registerSerializableItem("game_eventgame_state_changed", Event_GameStateChanged::newItem);
	registerSerializableItem("game_eventcreate_arrows", Event_CreateArrows::newItem);
	registerSerializableItem("game_eventcreate_counters", Event_CreateCounters::newItem);
	registerSerializableItem("game_eventdraw_cards", Event_DrawCards::newItem);
	registerSerializableItem("chat_eventchat_list_players", Event_ChatListPlayers::newItem);
}

TopLevelProtocolItem::TopLevelProtocolItem()
	: SerializableItem(QString()), currentItem(0)
{
}

bool TopLevelProtocolItem::readCurrentItem(QXmlStreamReader *xml)
{
	if (currentItem) {
		if (currentItem->read(xml)) {
			emit protocolItemReceived(currentItem);
			currentItem = 0;
		}
		return true;
	} else
		return false;
}

void TopLevelProtocolItem::readElement(QXmlStreamReader *xml)
{
	if (!readCurrentItem(xml) && (xml->isStartElement())) {
		QString childName = xml->name().toString();
		QString childSubType = xml->attributes().value("type").toString();
		
		currentItem = dynamic_cast<ProtocolItem *>(getNewItem(childName + childSubType));
		if (!currentItem)
			currentItem = new ProtocolItem_Invalid;
		
		readCurrentItem(xml);
	}
}

void TopLevelProtocolItem::writeElement(QXmlStreamWriter * /*xml*/)
{
}

int Command::lastCmdId = 0;

Command::Command(const QString &_itemName, int _cmdId)
	: ProtocolItem("cmd", _itemName), ticks(0)
{
	if (_cmdId == -1)
		_cmdId = lastCmdId++;
	insertItem(new SerializableItem_Int("cmd_id", _cmdId));
}

void Command::processResponse(ProtocolResponse *response)
{
	emit finished(response);
	emit finished(response->getResponseCode());
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

QHash<QString, ResponseCode> ProtocolResponse::responseHash;

ProtocolResponse::ProtocolResponse(int _cmdId, ResponseCode _responseCode, const QString &_itemName)
	: ProtocolItem("resp", _itemName)
{
	insertItem(new SerializableItem_Int("cmd_id", _cmdId));
	insertItem(new SerializableItem_String("response_code", responseHash.key(_responseCode)));
}

void ProtocolResponse::initializeHash()
{
	responseHash.insert("ok", RespOk);
	responseHash.insert("invalid_command", RespInvalidCommand);
	responseHash.insert("name_not_found", RespNameNotFound);
	responseHash.insert("login_needed", RespLoginNeeded);
	responseHash.insert("context_error", RespContextError);
	responseHash.insert("wrong_password", RespWrongPassword);
	responseHash.insert("spectators_not_allowed", RespSpectatorsNotAllowed);
}

Response_DeckList::Response_DeckList(int _cmdId, ResponseCode _responseCode, DeckList_Directory *_root)
	: ProtocolResponse(_cmdId, _responseCode, "deck_list")
{
	if (!_root)
		_root = new DeckList_Directory;
	insertItem(_root);
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

GameEvent::GameEvent(const QString &_eventName, int _gameId, int _playerId)
	: ProtocolItem("game_event", _eventName)
{
	insertItem(new SerializableItem_Int("game_id", _gameId));
	insertItem(new SerializableItem_Int("player_id", _playerId));
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

Event_ChatListPlayers::Event_ChatListPlayers(const QString &_channel, const QList<ServerInfo_ChatUser *> &_playerList)
	: ChatEvent("chat_list_players", _channel)
{
	for (int i = 0; i < _playerList.size(); ++i)
		itemList.append(_playerList[i]);
}

Event_ListGames::Event_ListGames(const QList<ServerInfo_Game *> &_gameList)
	: GenericEvent("list_games")
{
	for (int i = 0; i < _gameList.size(); ++i)
		itemList.append(_gameList[i]);
}

Event_GameStateChanged::Event_GameStateChanged(int _gameId, const QList<ServerInfo_Player *> &_playerList)
	: GameEvent("game_state_changed", _gameId, -1)
{
	for (int i = 0; i < _playerList.size(); ++i)
		itemList.append(_playerList[i]);
}

Event_CreateArrows::Event_CreateArrows(int _gameId, int _playerId, const QList<ServerInfo_Arrow *> &_arrowList)
	: GameEvent("create_arrows", _gameId, _playerId)
{
	for (int i = 0; i < _arrowList.size(); ++i)
		itemList.append(_arrowList[i]);
}

Event_CreateCounters::Event_CreateCounters(int _gameId, int _playerId, const QList<ServerInfo_Counter *> &_counterList)
	: GameEvent("create_counters", _gameId, _playerId)
{
	for (int i = 0; i < _counterList.size(); ++i)
		itemList.append(_counterList[i]);
}

Event_DrawCards::Event_DrawCards(int _gameId, int _playerId, int _numberCards, const QList<ServerInfo_Card *> &_cardList)
	: GameEvent("draw_cards", _gameId, _playerId)
{
	insertItem(new SerializableItem_Int("number_cards", _numberCards));
	for (int i = 0; i < _cardList.size(); ++i)
		itemList.append(_cardList[i]);
}
