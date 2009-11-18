#ifndef PROTOCOL_DATASTRUCTURES_H
#define PROTOCOL_DATASTRUCTURES_H

#include <QString>
#include <QColor>

enum ResponseCode { RespNothing, RespOk, RespInvalidCommand, RespInvalidData, RespNameNotFound, RespLoginNeeded, RespContextError, RespWrongPassword, RespSpectatorsNotAllowed };

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

class ServerPlayerInfo {
private:
	QString name;
public:
	ServerPlayerInfo(const QString &_name)
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

class ServerPlayer {
private:
	int PlayerId;
	QString name;
	bool local;
public:
	ServerPlayer(int _PlayerId, const QString &_name, bool _local)
		: PlayerId(_PlayerId), name(_name), local(_local) { }
	int getPlayerId() const { return PlayerId; }
	QString getName() const { return name; }
	bool getLocal() const { return local; }
};

class ServerZoneCard {
private:
	int playerId;
	QString zoneName;
	int id;
	QString name;
	int x, y;
	int counters;
	bool tapped;
	bool attacking;
	QString annotation;
public:
	ServerZoneCard(int _playerId, const QString &_zoneName, int _id, const QString &_name, int _x, int _y, int _counters, bool _tapped, bool _attacking, const QString &_annotation)
		: playerId(_playerId), zoneName(_zoneName), id(_id), name(_name), x(_x), y(_y), counters(_counters), tapped(_tapped), attacking(_attacking), annotation(_annotation) { }
	int getPlayerId() const { return playerId; }
	QString getZoneName() const { return zoneName; }
	int getId() const { return id; }
	QString getName() const { return name; }
	int getX() const { return x; }
	int getY() const { return y; }
	int getCounters() const { return counters; }
	bool getTapped() const { return tapped; }
	bool getAttacking() const { return attacking; }
	QString getAnnotation() const { return annotation; }
};

class ServerZone {
public:
	enum ZoneType { PrivateZone, PublicZone, HiddenZone };
private:
	int playerId;
	QString name;
	ZoneType type;
	bool hasCoords;
	int cardCount;
public:
	ServerZone(int _playerId, const QString &_name, ZoneType _type, bool _hasCoords, int _cardCount)
		: playerId(_playerId), name(_name), type(_type), hasCoords(_hasCoords), cardCount(_cardCount) { }
	int getPlayerId() const { return playerId; }
	QString getName() const { return name; }
	ZoneType getType() const { return type; }
	bool getHasCoords() const { return hasCoords; }
	int getCardCount() const { return cardCount; }
};

class ServerCounter {
private:
	int playerId;
	int id;
	QString name;
	QColor color;
	int radius;
	int count;
public:
	ServerCounter(int _playerId, int _id, const QString &_name, QColor _color, int _radius, int _count)
		: playerId(_playerId), id(_id), name(_name), color(_color), radius(_radius), count(_count) { }
	int getPlayerId() const { return playerId; }
	int getId() const { return id; }
	QString getName() const { return name; }
	QColor getColor() const { return color; }
	int getRadius() const { return radius; }
	int getCount() const { return count; }
};

class ServerArrow {
private:
	int id;
	int playerId;
	int startPlayerId;
	QString startZone;
	int startCardId;
	int targetPlayerId;
	QString targetZone;
	int targetCardId;
	QColor color;
public:
	ServerArrow(int _playerId, int _id, int _startPlayerId, const QString &_startZone, int _startCardId, int _targetPlayerId, const QString &_targetZone, int _targetCardId, const QColor &_color)
		: id(_id), playerId(_playerId), startPlayerId(_startPlayerId), startZone(_startZone), startCardId(_startCardId), targetPlayerId(_targetPlayerId), targetZone(_targetZone), targetCardId(_targetCardId), color(_color) { }
	int getId() const { return id; }
	int getPlayerId() const { return playerId; }
	int getStartPlayerId() const { return startPlayerId; }
	QString getStartZone() const { return startZone; }
	int getStartCardId() const { return startCardId; }
	int getTargetPlayerId() const { return targetPlayerId; }
	QString getTargetZone() const { return targetZone; }
	int getTargetCardId() const { return targetCardId; }
	QColor getColor() const { return color; }
};

#endif
