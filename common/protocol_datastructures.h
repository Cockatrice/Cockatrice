#ifndef PROTOCOL_DATASTRUCTURES_H
#define PROTOCOL_DATASTRUCTURES_H

#include <QString>
#include <QColor>
#include <QDateTime>
#include "serializable_item.h"

class DeckList;

enum ResponseCode { RespNothing, RespOk, RespInvalidCommand, RespInvalidData, RespNameNotFound, RespLoginNeeded, RespContextError, RespWrongPassword, RespSpectatorsNotAllowed };

// PrivateZone: Contents of the zone are always visible to the owner,
// but not to anyone else.
// PublicZone: Contents of the zone are always visible to anyone.
// HiddenZone: Contents of the zone are never visible to anyone.
// However, the owner of the zone can issue a dump_zone command,
// setting beingLookedAt to true.
// Cards in a zone with the type HiddenZone are referenced by their
// list index, whereas cards in any other zone are referenced by their ids.
enum ZoneType { PrivateZone, PublicZone, HiddenZone };

class ServerInfo_ChatChannel : public SerializableItem_Map {
public:
	ServerInfo_ChatChannel(const QString &_name = QString(), const QString &_description = QString(), int _playerCount = -1, bool _autoJoin = false);
	static SerializableItem *newItem() { return new ServerInfo_ChatChannel; }
	QString getName() const { return static_cast<SerializableItem_String *>(itemMap.value("name"))->getData(); }
	QString getDescription() const { return static_cast<SerializableItem_String *>(itemMap.value("description"))->getData(); }
	int getPlayerCount() const { return static_cast<SerializableItem_Int *>(itemMap.value("player_count"))->getData(); }
	bool getAutoJoin() const { return static_cast<SerializableItem_Bool *>(itemMap.value("auto_join"))->getData(); }
};

class ServerInfo_ChatUser : public SerializableItem_Map {
public:
	ServerInfo_ChatUser(const QString &_name = QString());
	static SerializableItem *newItem() { return new ServerInfo_ChatUser; }
	QString getName() const { return static_cast<SerializableItem_String *>(itemMap.value("name"))->getData(); }
};

class ServerInfo_Game : public SerializableItem_Map {
public:
	ServerInfo_Game(int _gameId = -1, const QString &_description = QString(), bool _hasPassword = false, int _playerCount = -1, int _maxPlayers = -1, const QString &_creatorName = QString(), bool _spectatorsAllowed = false, int _spectatorCount = -1);
	static SerializableItem *newItem() { return new ServerInfo_Game; }
	int getGameId() const { return static_cast<SerializableItem_Int *>(itemMap.value("game_id"))->getData(); }
	QString getDescription() const { return static_cast<SerializableItem_String *>(itemMap.value("description"))->getData(); }
	bool getHasPassword() const { return static_cast<SerializableItem_Bool *>(itemMap.value("has_password"))->getData(); }
	int getPlayerCount() const { return static_cast<SerializableItem_Int *>(itemMap.value("player_count"))->getData(); }
	int getMaxPlayers() const { return static_cast<SerializableItem_Int *>(itemMap.value("max_players"))->getData(); }
	QString getCreatorName() const { return static_cast<SerializableItem_String *>(itemMap.value("creator_name"))->getData(); }
	bool getSpectatorsAllowed() const { return static_cast<SerializableItem_Bool *>(itemMap.value("spectators_allowed"))->getData(); }
	int getSpectatorCount() const { return static_cast<SerializableItem_Int *>(itemMap.value("spectator_count"))->getData(); }
};

class ServerInfo_Card : public SerializableItem_Map {
public:
	ServerInfo_Card(int _id = -1, const QString &_name = QString(), int _x = -1, int _y = -1, int _counters = -1, bool _tapped = false, bool _attacking = false, const QString &_annotation = QString());
	static SerializableItem *newItem() { return new ServerInfo_Card; }
	int getId() const { return static_cast<SerializableItem_Int *>(itemMap.value("id"))->getData(); }
	QString getName() const { return static_cast<SerializableItem_String *>(itemMap.value("name"))->getData(); }
	int getX() const { return static_cast<SerializableItem_Int *>(itemMap.value("x"))->getData(); }
	int getY() const { return static_cast<SerializableItem_Int *>(itemMap.value("y"))->getData(); }
	int getCounters() const { return static_cast<SerializableItem_Int *>(itemMap.value("counters"))->getData(); }
	bool getTapped() const { return static_cast<SerializableItem_Bool *>(itemMap.value("tapped"))->getData(); }
	bool getAttacking() const { return static_cast<SerializableItem_Bool *>(itemMap.value("attacking"))->getData(); }
	QString getAnnotation() const { return static_cast<SerializableItem_String *>(itemMap.value("annotation"))->getData(); }
};

class ServerInfo_Zone : public SerializableItem_Map {
private:
	ZoneType typeFromString(const QString &type) const;
	QString typeToString(ZoneType type) const;
public:
	ServerInfo_Zone(const QString &_name = QString(), ZoneType _type = PrivateZone, bool _hasCoords = false, int _cardCount = -1, const QList<ServerInfo_Card *> &_cardList = QList<ServerInfo_Card *>());
	static SerializableItem *newItem() { return new ServerInfo_Zone; }
	QString getName() const { return static_cast<SerializableItem_String *>(itemMap.value("name"))->getData(); }
	ZoneType getType() const { return typeFromString(static_cast<SerializableItem_String *>(itemMap.value("type"))->getData()); }
	bool getHasCoords() const { return static_cast<SerializableItem_Bool *>(itemMap.value("has_coords"))->getData(); }
	int getCardCount() const { return static_cast<SerializableItem_Int *>(itemMap.value("card_count"))->getData(); }
	QList<ServerInfo_Card *> getCardList() const;
};

class ServerInfo_Counter : public SerializableItem_Map {
public:
	ServerInfo_Counter(int _id = -1, const QString &_name = QString(), const QColor &_color = QColor(), int _radius = -1, int _count = -1);
	static SerializableItem *newItem() { return new ServerInfo_Counter; }
	int getId() const { return static_cast<SerializableItem_Int *>(itemMap.value("id"))->getData(); }
	QString getName() const { return static_cast<SerializableItem_String *>(itemMap.value("name"))->getData(); }
	QColor getColor() const { return static_cast<SerializableItem_Color *>(itemMap.value("color"))->getData(); }
	int getRadius() const { return static_cast<SerializableItem_Int *>(itemMap.value("radius"))->getData(); }
	int getCount() const { return static_cast<SerializableItem_Int *>(itemMap.value("count"))->getData(); }
};

class ServerInfo_Arrow : public SerializableItem_Map {
public:
	ServerInfo_Arrow(int _id = -1, int _startPlayerId = -1, const QString &_startZone = QString(), int _startCardId = -1, int _targetPlayerId = -1, const QString &_targetZone = QString(), int _targetCardId = -1, const QColor &_color = QColor());
	static SerializableItem *newItem() { return new ServerInfo_Arrow; }
	int getId() const { return static_cast<SerializableItem_Int *>(itemMap.value("id"))->getData(); }
	int getStartPlayerId() const { return static_cast<SerializableItem_Int *>(itemMap.value("start_player_id"))->getData(); }
	QString getStartZone() const { return static_cast<SerializableItem_String *>(itemMap.value("start_zone"))->getData(); }
	int getStartCardId() const { return static_cast<SerializableItem_Int *>(itemMap.value("start_card_id"))->getData(); }
	int getTargetPlayerId() const { return static_cast<SerializableItem_Int *>(itemMap.value("target_player_id"))->getData(); }
	QString getTargetZone() const { return static_cast<SerializableItem_String *>(itemMap.value("target_zone"))->getData(); }
	int getTargetCardId() const { return static_cast<SerializableItem_Int *>(itemMap.value("target_card_id"))->getData(); }
	QColor getColor() const { return static_cast<SerializableItem_Color *>(itemMap.value("color"))->getData(); }
};

class ServerInfo_Player : public SerializableItem_Map {
private:
	QList<ServerInfo_Zone *> zoneList;
	QList<ServerInfo_Counter *> counterList;
	QList<ServerInfo_Arrow *> arrowList;
protected:
	void extractData();
public:
	ServerInfo_Player(int _playerId = -1, const QString &_name = QString(), bool _spectator = false, DeckList *_deck = 0, const QList<ServerInfo_Zone *> &_zoneList = QList<ServerInfo_Zone *>(), const QList<ServerInfo_Counter *> &_counterList = QList<ServerInfo_Counter *>(), const QList<ServerInfo_Arrow *> &_arrowList = QList<ServerInfo_Arrow *>());
	static SerializableItem *newItem() { return new ServerInfo_Player; }
	int getPlayerId() const { return static_cast<SerializableItem_Int *>(itemMap.value("player_id"))->getData(); }
	QString getName() const { return static_cast<SerializableItem_String *>(itemMap.value("name"))->getData(); }
	bool getSpectator() const { return static_cast<SerializableItem_Bool *>(itemMap.value("spectator"))->getData(); }
	DeckList *getDeck() const;
	const QList<ServerInfo_Zone *> &getZoneList() const { return zoneList; }
	const QList<ServerInfo_Counter *> &getCounterList() const { return counterList; }
	const QList<ServerInfo_Arrow *> &getArrowList() const { return arrowList; }
};

class DeckList_TreeItem : public SerializableItem_Map {
public:
	DeckList_TreeItem(const QString &_itemType, const QString &_name, int _id);
	QString getName() const { return static_cast<SerializableItem_String *>(itemMap.value("name"))->getData(); }
	int getId() const { return static_cast<SerializableItem_Int *>(itemMap.value("id"))->getData(); }
};
class DeckList_File : public DeckList_TreeItem {
public:
	DeckList_File(const QString &_name = QString(), int _id = -1, QDateTime _uploadTime = QDateTime());
	static SerializableItem *newItem() { return new DeckList_File; }
	QDateTime getUploadTime() const { return static_cast<SerializableItem_DateTime *>(itemMap.value("upload_time"))->getData(); }
};
class DeckList_Directory : public DeckList_TreeItem {
public:
	DeckList_Directory(const QString &_name = QString(), int _id = 0);
	static SerializableItem *newItem() { return new DeckList_Directory; }
	QList<DeckList_TreeItem *> getTreeItems() const { return typecastItemList<DeckList_TreeItem *>(); }
};

#endif
