#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QString>
#include <QMap>
#include <QHash>
#include <QObject>
#include <QDebug>
#include "protocol_item_ids.h"

class QXmlStreamReader;
class QXmlStreamWriter;
class QXmlStreamAttributes;

enum ItemId {
	ItemId_Event_ChatListChannels = ItemId_Other + 1,
	ItemId_Event_ChatListPlayers = ItemId_Other + 2,
	ItemId_Event_ListGames = ItemId_Other + 3
};

class ProtocolItem : public QObject {
	Q_OBJECT
private:
	QString currentElementText;
protected:
	typedef ProtocolItem *(*NewItemFunction)();
	static QHash<QString, NewItemFunction> itemNameHash;
	
	QString itemName;
	QMap<QString, QString> parameters;
	void setParameter(const QString &name, const QString &value) { parameters[name] = value; }
	void setParameter(const QString &name, bool value) { parameters[name] = (value ? "1" : "0"); }
	void setParameter(const QString &name, int value) { parameters[name] = QString::number(value); }
	virtual void extractParameters() { }
	virtual QString getItemType() const = 0;
	
	virtual bool readElement(QXmlStreamReader * /*xml*/) { return false; }
	virtual void writeElement(QXmlStreamWriter * /*xml*/) { }
private:
	static void initializeHashAuto();
public:
	static const int protocolVersion = 4;
	virtual int getItemId() const = 0;
	ProtocolItem(const QString &_itemName);
	static void initializeHash();
	static ProtocolItem *getNewItem(const QString &name);
	bool read(QXmlStreamReader *xml);
	void write(QXmlStreamWriter *xml);
};

class Command : public ProtocolItem {
	Q_OBJECT
private:
	int cmdId;
	static int lastCmdId;
protected:
	QString getItemType() const { return "cmd"; }
	void extractParameters();
public:
	Command(const QString &_itemName = QString(), int _cmdId = -1);
	int getCmdId() const { return cmdId; }
};

class InvalidCommand : public Command {
	Q_OBJECT
public:
	InvalidCommand() : Command() { }
	int getItemId() const { return ItemId_Other; }
};

class ChatCommand : public Command {
	Q_OBJECT
private:
	QString channel;
protected:
	void extractParameters()
	{
		channel = parameters["channel"];
	}
public:
	ChatCommand(const QString &_cmdName, const QString &_channel)
		: Command(_cmdName), channel(_channel)
	{
		setParameter("channel", channel);
	}
	QString getChannel() const { return channel; }
};

class GameCommand : public Command {
	Q_OBJECT
private:
	int gameId;
protected:
	void extractParameters()
	{
		gameId = parameters["game_id"].toInt();
	}
public:
	GameCommand(const QString &_cmdName, int _gameId)
		: Command(_cmdName), gameId(_gameId)
	{
		setParameter("game_id", gameId);
	}
	int getGameId() const { return gameId; }
};

class ProtocolResponse : public ProtocolItem {
	Q_OBJECT
public:
	enum ResponseCode { RespNothing, RespOk, RespInvalidCommand, RespNameNotFound, RespLoginNeeded, RespContextError, RespWrongPassword, RespSpectatorsNotAllowed };
private:
	int cmdId;
	ResponseCode responseCode;
	static QHash<QString, ResponseCode> responseHash;
protected:
	QString getItemType() const { return "resp"; }
	void extractParameters();
public:
	ProtocolResponse(int _cmdId = -1, ResponseCode _responseCode = RespOk);
	int getItemId() const { return ItemId_Other; }
	static void initializeHash();
	static ProtocolItem *newItem() { return new ProtocolResponse; }
};

class GenericEvent : public ProtocolItem {
	Q_OBJECT
protected:
	QString getItemType() const { return "generic_event"; }
public:
	GenericEvent(const QString &_eventName);
};

class GameEvent : public ProtocolItem {
	Q_OBJECT
private:
	int gameId;
	int playerId;
protected:
	QString getItemType() const { return "game_event"; }
	void extractParameters();
public:
	GameEvent(const QString &_eventName, int _gameId, int _playerId);
	int getGameId() const { return gameId; }
	int getPlayerId() const { return playerId; }
	void setGameId(int _gameId) { gameId = _gameId; }
};

class ChatEvent : public ProtocolItem {
	Q_OBJECT
private:
	QString channel;
protected:
	QString getItemType() const { return "chat_event"; }
	void extractParameters();
public:
	ChatEvent(const QString &_eventName, const QString &_channel);
};

class Event_ChatListChannels : public GenericEvent {
	Q_OBJECT
public:
	class ChannelInfo {
	private:
		QString name;
		QString description;
		int playerCount;
		bool autoJoin;
	public:
		ChannelInfo(const QString &_name, const QString &_description, int _playerCount, bool _autoJoin)
			: name(_name), description(_description), playerCount(_playerCount), autoJoin(_autoJoin) { }
		QString getName() const { return name; }
		QString getDescription() const { return description; }
		int getPlayerCount() const { return playerCount; }
		bool getAutoJoin() const { return autoJoin; }
	};
private:
	QList<ChannelInfo> channelList;
public:
	Event_ChatListChannels() : GenericEvent("chat_list_channels") { }
	int getItemId() const { return ItemId_Event_ChatListChannels; }
	void addChannel(const QString &_name, const QString &_description, int _playerCount, bool _autoJoin)
	{
		channelList.append(ChannelInfo(_name, _description, _playerCount, _autoJoin));
	}
	const QList<ChannelInfo> &getChannelList() const { return channelList; }

	bool readElement(QXmlStreamReader *xml);
	void writeElement(QXmlStreamWriter *xml);
};

class Event_ChatListPlayers : public ChatEvent {
	Q_OBJECT
public:
	class PlayerInfo {
	private:
		QString name;
	public:
		PlayerInfo(const QString &_name)
			: name(_name) { }
		QString getName() const { return name; }
	};
private:
	QList<PlayerInfo> playerList;
public:
	Event_ChatListPlayers(const QString &_channel) : ChatEvent("chat_list_players", _channel) { }
	int getItemId() const { return ItemId_Event_ChatListPlayers; }
	void addPlayer(const QString &_name)
	{
		playerList.append(PlayerInfo(_name));
	}
	const QList<PlayerInfo> &getPlayerList() const { return playerList; }

	bool readElement(QXmlStreamReader *xml);
	void writeElement(QXmlStreamWriter *xml);
};

class Event_ListGames : public GenericEvent {
	Q_OBJECT
public:
	class GameInfo {
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
		GameInfo(int _gameId, const QString &_description, bool _hasPassword, int _playerCount, int _maxPlayers, const QString &_creatorName, bool _spectatorsAllowed, int _spectatorCount)
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
private:
	QList<GameInfo> gameList;
public:
	Event_ListGames() : GenericEvent("list_games") { }
	int getItemId() const { return ItemId_Event_ListGames; }
	void addGame(int _gameId, const QString &_description, bool _hasPassword, int _playerCount, int _maxPlayers, const QString &_creatorName, bool _spectatorsAllowed, int _spectatorCount)
	{
		gameList.append(GameInfo(_gameId, _description, _hasPassword, _playerCount, _maxPlayers, _creatorName, _spectatorsAllowed, _spectatorCount));
	}
	const QList<GameInfo> &getGameList() const { return gameList; }

	bool readElement(QXmlStreamReader *xml);
	void writeElement(QXmlStreamWriter *xml);
};

#endif
