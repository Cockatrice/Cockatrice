#ifndef PROTOCOL_DATASTRUCTURES_H
#define PROTOCOL_DATASTRUCTURES_H

#include <QString>
#include <QColor>
#include <QDateTime>

class QXmlStreamReader;
class QXmlStreamWriter;

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

class SerializableItem {
protected:
	SerializableItem *currentItem;
public:
	SerializableItem() : currentItem(0) { }
	virtual bool readElement(QXmlStreamReader *xml) = 0;
	virtual void writeElement(QXmlStreamWriter *xml) = 0;
};

class ServerChatChannelInfo {
private:
	QString name;
	QString description;
	int playerCount;
	bool autoJoin;
public:
	ServerChatChannelInfo(const QString &_name, const QString &_description, int _playerCount, bool _autoJoin)
		: name(_name), description(_description), playerCount(_playerCount), autoJoin(_autoJoin) { }
	QString getName() const { return name; }
	QString getDescription() const { return description; }
	int getPlayerCount() const { return playerCount; }
	bool getAutoJoin() const { return autoJoin; }
};

class ServerChatUserInfo {
private:
	QString name;
public:
	ServerChatUserInfo(const QString &_name)
		: name(_name) { }
	QString getName() const { return name; }
};

class ServerGameInfo {
private:
	int gameId;
	QString description;
	bool hasPassword;
	int playerCount;
	int maxPlayers;
	QString creatorName;
	bool spectatorsAllowed;
	int spectatorCount;
public:
	ServerGameInfo(int _gameId, const QString &_description, bool _hasPassword, int _playerCount, int _maxPlayers, const QString &_creatorName, bool _spectatorsAllowed, int _spectatorCount)
		: gameId(_gameId), description(_description), hasPassword(_hasPassword), playerCount(_playerCount), maxPlayers(_maxPlayers), creatorName(_creatorName), spectatorsAllowed(_spectatorsAllowed), spectatorCount(_spectatorCount) { }
	int getGameId() const { return gameId; }
	QString getDescription() const { return description; }
	bool getHasPassword() const { return hasPassword; }
	int getPlayerCount() const { return playerCount; }
	int getMaxPlayers() const { return maxPlayers; }
	QString getCreatorName() const { return creatorName; }
	bool getSpectatorsAllowed() const { return spectatorsAllowed; }
	int getSpectatorCount() const { return spectatorCount; }
};

class ServerInfo_Card : public SerializableItem {
private:
	int id;
	QString name;
	int x, y;
	int counters;
	bool tapped;
	bool attacking;
	QString annotation;
public:
	ServerInfo_Card(int _id = -1, const QString &_name = QString(), int _x = -1, int _y = -1, int _counters = -1, bool _tapped = false, bool _attacking = false, const QString &_annotation = QString())
		: id(_id), name(_name), x(_x), y(_y), counters(_counters), tapped(_tapped), attacking(_attacking), annotation(_annotation) { }
	int getId() const { return id; }
	QString getName() const { return name; }
	int getX() const { return x; }
	int getY() const { return y; }
	int getCounters() const { return counters; }
	bool getTapped() const { return tapped; }
	bool getAttacking() const { return attacking; }
	QString getAnnotation() const { return annotation; }
	bool readElement(QXmlStreamReader *xml);
	void writeElement(QXmlStreamWriter *xml);
};

class ServerInfo_Zone : public SerializableItem {
private:
	QString name;
	ZoneType type;
	bool hasCoords;
	int cardCount;
	QList<ServerInfo_Card *> cardList;
public:
	ServerInfo_Zone(const QString &_name = QString(), ZoneType _type = PrivateZone, bool _hasCoords = false, int _cardCount = -1, const QList<ServerInfo_Card *> &_cardList = QList<ServerInfo_Card *>())
		: name(_name), type(_type), hasCoords(_hasCoords), cardCount(_cardCount), cardList(_cardList) { }
	~ServerInfo_Zone();
	QString getName() const { return name; }
	ZoneType getType() const { return type; }
	bool getHasCoords() const { return hasCoords; }
	int getCardCount() const { return cardCount; }
	const QList<ServerInfo_Card *> &getCardList() const { return cardList; }
	void addCard(ServerInfo_Card *card) { cardList.append(card); ++cardCount; }
	bool readElement(QXmlStreamReader *xml);
	void writeElement(QXmlStreamWriter *xml);
};

class ServerInfo_Counter : public SerializableItem {
private:
	int id;
	QString name;
	QColor color;
	int radius;
	int count;
public:
	ServerInfo_Counter(int _id = -1, const QString &_name = QString(), const QColor &_color = QColor(), int _radius = -1, int _count = -1)
		: id(_id), name(_name), color(_color), radius(_radius), count(_count) { }
	int getId() const { return id; }
	QString getName() const { return name; }
	QColor getColor() const { return color; }
	int getRadius() const { return radius; }
	int getCount() const { return count; }
	bool readElement(QXmlStreamReader *xml);
	void writeElement(QXmlStreamWriter *xml);
};

class ServerInfo_Arrow : public SerializableItem {
private:
	int id;
	int startPlayerId;
	QString startZone;
	int startCardId;
	int targetPlayerId;
	QString targetZone;
	int targetCardId;
	QColor color;
public:
	ServerInfo_Arrow(int _id = -1, int _startPlayerId = -1, const QString &_startZone = QString(), int _startCardId = -1, int _targetPlayerId = -1, const QString &_targetZone = QString(), int _targetCardId = -1, const QColor &_color = QColor())
		: id(_id), startPlayerId(_startPlayerId), startZone(_startZone), startCardId(_startCardId), targetPlayerId(_targetPlayerId), targetZone(_targetZone), targetCardId(_targetCardId), color(_color) { }
	int getId() const { return id; }
	int getStartPlayerId() const { return startPlayerId; }
	QString getStartZone() const { return startZone; }
	int getStartCardId() const { return startCardId; }
	int getTargetPlayerId() const { return targetPlayerId; }
	QString getTargetZone() const { return targetZone; }
	int getTargetCardId() const { return targetCardId; }
	QColor getColor() const { return color; }
	bool readElement(QXmlStreamReader *xml);
	void writeElement(QXmlStreamWriter *xml);
};

class ServerInfo_Player : public SerializableItem {
private:
	int playerId;
	QString name;
	QList<ServerInfo_Zone *> zoneList;
	QList<ServerInfo_Counter *> counterList;
	QList<ServerInfo_Arrow *> arrowList;
public:
	ServerInfo_Player(int _playerId = -1, const QString &_name = QString(), const QList<ServerInfo_Zone *> &_zoneList = QList<ServerInfo_Zone *>(), const QList<ServerInfo_Counter *> &_counterList = QList<ServerInfo_Counter *>(), const QList<ServerInfo_Arrow *> &_arrowList = QList<ServerInfo_Arrow *>())
		: playerId(_playerId), name(_name), zoneList(_zoneList), counterList(_counterList), arrowList(_arrowList) { }
	~ServerInfo_Player();
	int getPlayerId() const { return playerId; }
	QString getName() const { return name; }
	const QList<ServerInfo_Zone *> &getZoneList() const { return zoneList; }
	const QList<ServerInfo_Counter *> &getCounterList() const { return counterList; }
	const QList<ServerInfo_Arrow *> &getArrowList() const { return arrowList; }
	void addZone(ServerInfo_Zone *zone) { zoneList.append(zone); }
	void addCounter(ServerInfo_Counter *counter) { counterList.append(counter); }
	void addArrow(ServerInfo_Arrow *arrow) { arrowList.append(arrow); }
	bool readElement(QXmlStreamReader *xml);
	void writeElement(QXmlStreamWriter *xml);
};

class DeckList_TreeItem : public SerializableItem {
protected:
	QString name;
	int id;
public:
	DeckList_TreeItem(const QString &_name, int _id) : name(_name), id(_id) { }
	QString getName() const { return name; }
	int getId() const { return id; }
};
class DeckList_File : public DeckList_TreeItem {
private:
	QDateTime uploadTime;
public:
	DeckList_File(const QString &_name, int _id, QDateTime _uploadTime) : DeckList_TreeItem(_name, _id), uploadTime(_uploadTime) { }
	bool readElement(QXmlStreamReader *xml);
	void writeElement(QXmlStreamWriter *xml);
	QDateTime getUploadTime() const { return uploadTime; }
};
class DeckList_Directory : public DeckList_TreeItem, public QList<DeckList_TreeItem *> {
public:
	DeckList_Directory(const QString &_name = QString(), int _id = 0) : DeckList_TreeItem(_name, _id) { }
	~DeckList_Directory();
	bool readElement(QXmlStreamReader *xml);
	void writeElement(QXmlStreamWriter *xml);
};

#endif
