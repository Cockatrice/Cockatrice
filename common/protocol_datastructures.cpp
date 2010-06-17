#include "protocol_datastructures.h"
#include "decklist.h"
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

ServerInfo_ChatChannel::ServerInfo_ChatChannel(const QString &_name, const QString &_description, int _playerCount, bool _autoJoin)
	: SerializableItem_Map("chat_channel")
{
	insertItem(new SerializableItem_String("name", _name));
	insertItem(new SerializableItem_String("description", _description));
	insertItem(new SerializableItem_Int("player_count", _playerCount));
	insertItem(new SerializableItem_Bool("auto_join", _autoJoin));
}

ServerInfo_ChatUser::ServerInfo_ChatUser(const QString &_name)
	: SerializableItem_Map("chat_user")
{
	insertItem(new SerializableItem_String("name", _name));
}

ServerInfo_Game::ServerInfo_Game(int _gameId, const QString &_description, bool _hasPassword, int _playerCount, int _maxPlayers, const QString &_creatorName, bool _spectatorsAllowed, bool _spectatorsNeedPassword, int _spectatorCount)
	: SerializableItem_Map("game")
{
	insertItem(new SerializableItem_Int("game_id", _gameId));
	insertItem(new SerializableItem_String("description", _description));
	insertItem(new SerializableItem_Bool("has_password", _hasPassword));
	insertItem(new SerializableItem_Int("player_count", _playerCount));
	insertItem(new SerializableItem_Int("max_players", _maxPlayers));
	insertItem(new SerializableItem_String("creator_name", _creatorName));
	insertItem(new SerializableItem_Bool("spectators_allowed", _spectatorsAllowed));
	insertItem(new SerializableItem_Bool("spectators_need_password", _spectatorsNeedPassword));
	insertItem(new SerializableItem_Int("spectator_count", _spectatorCount));
}

ServerInfo_CardCounter::ServerInfo_CardCounter(int _id, int _value)
	: SerializableItem_Map("card_counter")
{
	insertItem(new SerializableItem_Int("id", _id));
	insertItem(new SerializableItem_Int("value", _value));
}

ServerInfo_Card::ServerInfo_Card(int _id, const QString &_name, int _x, int _y, bool _tapped, bool _attacking, const QString &_pt, const QString &_annotation, const QList<ServerInfo_CardCounter *> &_counters)
	: SerializableItem_Map("card")
{
	insertItem(new SerializableItem_Int("id", _id));
	insertItem(new SerializableItem_String("name", _name));
	insertItem(new SerializableItem_Int("x", _x));
	insertItem(new SerializableItem_Int("y", _y));
	insertItem(new SerializableItem_Bool("tapped", _tapped));
	insertItem(new SerializableItem_Bool("attacking", _attacking));
	insertItem(new SerializableItem_String("pt", _pt));
	insertItem(new SerializableItem_String("annotation", _annotation));
	
	for (int i = 0; i < _counters.size(); ++i)
		itemList.append(_counters[i]);
}

ServerInfo_Zone::ServerInfo_Zone(const QString &_name, ZoneType _type, bool _hasCoords, int _cardCount, const QList<ServerInfo_Card *> &_cardList)
	: SerializableItem_Map("zone")
{
	insertItem(new SerializableItem_String("name", _name));
	insertItem(new SerializableItem_String("zone_type", typeToString(_type)));
	insertItem(new SerializableItem_Bool("has_coords", _hasCoords));
	insertItem(new SerializableItem_Int("card_count", _cardCount));
	
	for (int i = 0; i < _cardList.size(); ++i)
		itemList.append(_cardList[i]);
}

ZoneType ServerInfo_Zone::typeFromString(const QString &type) const
{
	if (type == "private")
		return PrivateZone;
	else if (type == "hidden")
		return HiddenZone;
	return PublicZone;
}

QString ServerInfo_Zone::typeToString(ZoneType type) const
{
	switch (type) {
		case PrivateZone: return "private";
		case HiddenZone: return "hidden";
		default: return "public";
	}
}

QList<ServerInfo_Card *> ServerInfo_Zone::getCardList() const
{
	QList<ServerInfo_Card *> result;
	for (int i = 0; i < itemList.size(); ++i) {
		ServerInfo_Card *card = dynamic_cast<ServerInfo_Card *>(itemList[i]);
		if (card)
			result.append(card);
	}
	return result;
}

ServerInfo_Counter::ServerInfo_Counter(int _id, const QString &_name, const QColor &_color, int _radius, int _count)
	: SerializableItem_Map("counter")
{
	insertItem(new SerializableItem_Int("id", _id));
	insertItem(new SerializableItem_String("name", _name));
	insertItem(new SerializableItem_Color("color", _color));
	insertItem(new SerializableItem_Int("radius", _radius));
	insertItem(new SerializableItem_Int("count", _count));
}

ServerInfo_Arrow::ServerInfo_Arrow(int _id, int _startPlayerId, const QString &_startZone, int _startCardId, int _targetPlayerId, const QString &_targetZone, int _targetCardId, const QColor &_color)
	: SerializableItem_Map("arrow")
{
	insertItem(new SerializableItem_Int("id", _id));
	insertItem(new SerializableItem_Int("start_player_id", _startPlayerId));
	insertItem(new SerializableItem_String("start_zone", _startZone));
	insertItem(new SerializableItem_Int("start_card_id", _startCardId));
	insertItem(new SerializableItem_Int("target_player_id", _targetPlayerId));
	insertItem(new SerializableItem_String("target_zone", _targetZone));
	insertItem(new SerializableItem_Int("target_card_id", _targetCardId));
	insertItem(new SerializableItem_Color("color", _color));
}

ServerInfo_PlayerProperties::ServerInfo_PlayerProperties(int _playerId, const QString &_name, bool _spectator, bool _conceded, bool _readyStart, int _deckId)
	: SerializableItem_Map("player_properties")
{
	insertItem(new SerializableItem_Int("player_id", _playerId));
	insertItem(new SerializableItem_String("name", _name));
	insertItem(new SerializableItem_Bool("spectator", _spectator));
	insertItem(new SerializableItem_Bool("conceded", _conceded));
	insertItem(new SerializableItem_Bool("ready_start", _readyStart));
	insertItem(new SerializableItem_Int("deck_id", _deckId));
}

ServerInfo_Player::ServerInfo_Player(ServerInfo_PlayerProperties *_properties, DeckList *_deck, const QList<ServerInfo_Zone *> &_zoneList, const QList<ServerInfo_Counter *> &_counterList, const QList<ServerInfo_Arrow *> &_arrowList)
	: SerializableItem_Map("player"), zoneList(_zoneList), counterList(_counterList), arrowList(_arrowList)
{
	if (!_properties)
		insertItem(new ServerInfo_PlayerProperties);
	else
		insertItem(_properties);

	if (!_deck)
		insertItem(new DeckList);
	else
		insertItem(new DeckList(_deck));
	
	for (int i = 0; i < _zoneList.size(); ++i)
		itemList.append(_zoneList[i]);
	for (int i = 0; i < _counterList.size(); ++i)
		itemList.append(_counterList[i]);
	for (int i = 0; i < _arrowList.size(); ++i)
		itemList.append(_arrowList[i]);
}

void ServerInfo_Player::extractData()
{
	for (int i = 0; i < itemList.size(); ++i) {
		ServerInfo_Zone *zone = dynamic_cast<ServerInfo_Zone *>(itemList[i]);
		ServerInfo_Counter *counter = dynamic_cast<ServerInfo_Counter *>(itemList[i]);
		ServerInfo_Arrow *arrow = dynamic_cast<ServerInfo_Arrow *>(itemList[i]);
		if (zone)
			zoneList.append(zone);
		else if (counter)
			counterList.append(counter);
		else if (arrow)
			arrowList.append(arrow);
	}
}

DeckList *ServerInfo_Player::getDeck() const
{
	return static_cast<DeckList *>(itemMap.value("cockatrice_deck"));
}

ServerInfo_PlayerPing::ServerInfo_PlayerPing(int _playerId, int _pingTime)
	: SerializableItem_Map("player_ping")
{
	insertItem(new SerializableItem_Int("player_id", _playerId));
	insertItem(new SerializableItem_Int("ping_time", _pingTime));
}

DeckList_TreeItem::DeckList_TreeItem(const QString &_itemType, const QString &_name, int _id)
	: SerializableItem_Map(_itemType)
{
	insertItem(new SerializableItem_String("name", _name));
	insertItem(new SerializableItem_Int("id", _id));
}

DeckList_File::DeckList_File(const QString &_name, int _id, QDateTime _uploadTime)
	: DeckList_TreeItem("file", _name, _id)
{
	insertItem(new SerializableItem_DateTime("upload_time", _uploadTime));
}

DeckList_Directory::DeckList_Directory(const QString &_name, int _id)
	: DeckList_TreeItem("directory", _name, _id)
{
}
