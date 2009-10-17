#ifndef CLIENT_H
#define CLIENT_H

#include <QTcpSocket>
#include <QColor>
#include <QStringList>
#include <QHash>

class QTimer;

// Connection state.
// The protocol handler itself is stateless once the connection
// has been established.
enum ProtocolStatus { StatusDisconnected,
		      StatusConnecting,
		      StatusAwaitingWelcome,
		      StatusLoggingIn,
		      StatusIdle,
		      StatusPlaying };

enum ServerResponse {
	RespOk,
	RespNameNotFound,
	RespLoginNeeded,
	RespSyntaxError,
	RespContextError,
	RespPasswordWrong,
	RespSpectatorsNotAllowed,
	RespInvalid
};

enum ServerEventType {
	eventInvalid,
	eventPlayerId,
	eventSay,
	eventJoin,
	eventLeave,
	eventGameClosed,
	eventReadyStart,
	eventSetupZones,
	eventGameStart,
	eventShuffle,
	eventRollDie,
	eventDraw,
	eventMoveCard,
	eventCreateToken,
	eventCreateArrow,
	eventSetCardAttr,
	eventAddCounter,
	eventSetCounter,
	eventDelCounter,
	eventSetActivePlayer,
	eventSetActivePhase,
	eventDumpZone,
	eventStopDumpZone
};

class ServerEventData {
private:
	static QHash<QString, ServerEventType> eventHash;
	
	bool IsPublic;
	int PlayerId;
	QString PlayerName;
	ServerEventType EventType;
	QStringList EventData;
public:
	ServerEventData(const QString &line);
	bool getPublic() const { return IsPublic; }
	int getPlayerId() const { return PlayerId; }
	const QString &getPlayerName() const { return PlayerName; }
	ServerEventType getEventType() const { return EventType; }
	const QStringList &getEventData() const { return EventData; }
};

enum ChatEventType {
	eventChatInvalid,
	eventChatListChannels,
	eventChatJoinChannel,
	eventChatListPlayers,
	eventChatLeaveChannel,
	eventChatSay,
	eventChatServerMessage
};

class ChatEventData {
private:
	static QHash<QString, ChatEventType> eventHash;
	
	ChatEventType eventType;
	QStringList eventData;
public:
	ChatEventData(const QString &line);
	ChatEventType getEventType() const { return eventType; }
	const QStringList &getEventData() const { return eventData; }
};

class ServerGame {
private:
	int gameId;
	QString creator;
	QString description;
	bool hasPassword;
	unsigned char playerCount;
	unsigned char maxPlayers;
	bool spectatorsAllowed;
	unsigned int spectatorsCount;
public:
	ServerGame(int _gameId = -1, const QString &_creator = QString(), const QString &_description = QString(), bool _hasPassword = false, unsigned char _playerCount = 0, unsigned char _maxPlayers = 0, bool _spectatorsAllowed = false, unsigned int _spectatorsCount = 0)
		: gameId(_gameId), creator(_creator), description(_description), hasPassword(_hasPassword), playerCount(_playerCount), maxPlayers(_maxPlayers), spectatorsAllowed(_spectatorsAllowed), spectatorsCount(_spectatorsCount) { }
	int getGameId() const { return gameId; }
	QString getCreator() const { return creator; }
	QString getDescription() const { return description; }
	bool getHasPassword() const { return hasPassword; }
	unsigned char getPlayerCount() const { return playerCount; }
	unsigned char getMaxPlayers() const { return maxPlayers; }
	bool getSpectatorsAllowed() const { return spectatorsAllowed; }
	unsigned int getSpectatorsCount() const { return spectatorsCount; }
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

class PendingCommand : public QObject {
	Q_OBJECT
private:
	int msgid;
	int time;
signals:
	void finished(ServerResponse resp);
public slots:
	virtual void responseReceived(ServerResponse resp);
public:
	PendingCommand(int _msgid = -1);
	int tick() { return ++time; }
	int getMsgId() const { return msgid; }
	void setMsgId(int _msgId) { msgid = _msgId; }
};

class PendingCommand_ChatJoinChannel : public PendingCommand {
	Q_OBJECT
private:
	QString channelName;
public:
	PendingCommand_ChatJoinChannel(const QString &_channelName)
		: channelName(_channelName) { }
	const QString &getChannelName() const { return channelName; }
};

class PendingCommand_ListPlayers : public PendingCommand {
	Q_OBJECT
private:
	QList<ServerPlayer> playerList;
signals:
	void playerListReceived(QList<ServerPlayer> _playerList);
public:
	void responseReceived(ServerResponse resp);
	void addPlayer(const ServerPlayer &player) { playerList.append(player); }
};

class PendingCommand_ListZones : public PendingCommand {
	Q_OBJECT
private:
	QList<ServerZone> zoneList;
	int playerId;
signals:
	void zoneListReceived(QList<ServerZone> _zoneList);
public:
	PendingCommand_ListZones(int _playerId)
		: playerId(_playerId) { }
	void responseReceived(ServerResponse resp);
	void addZone(const ServerZone &zone) { zoneList.append(zone); }
	int getPlayerId() const { return playerId; }
};

class PendingCommand_DumpZone : public PendingCommand {
	Q_OBJECT
private:
	QList<ServerZoneCard> cardList;
	int playerId;
	QString zoneName;
	int numberCards;
signals:
	void cardListReceived(QList<ServerZoneCard> _cardList);
public:
	PendingCommand_DumpZone(int _playerId, const QString &_zoneName, int _numberCards)
		: playerId(_playerId), zoneName(_zoneName), numberCards(_numberCards) { }
	void responseReceived(ServerResponse resp);
	void addCard(const ServerZoneCard &card) { cardList.append(card); }
	int getPlayerId() const { return playerId; }
	QString getZoneName() const { return zoneName; }
	int getNumberCards() const { return numberCards; }
};

class PendingCommand_ListCounters : public PendingCommand {
	Q_OBJECT
private:
	QList<ServerCounter> counterList;
	int playerId;
signals:
	void counterListReceived(QList<ServerCounter> _counterList);
public:
	PendingCommand_ListCounters(int _playerId)
		: playerId(_playerId) { }
	void responseReceived(ServerResponse resp);
	void addCounter(const ServerCounter &counter) { counterList.append(counter); }
	int getPlayerId() const { return playerId; }
};

class PendingCommand_DumpAll : public PendingCommand {
	Q_OBJECT
private:
	QList<ServerPlayer> playerList;
	QList<ServerZone> zoneList;
	QList<ServerZoneCard> cardList;
	QList<ServerCounter> counterList;
signals:
	void playerListReceived(QList<ServerPlayer> _playerList);
	void zoneListReceived(QList<ServerZone> _zoneList);
	void cardListReceived(QList<ServerZoneCard> _cardList);
	void counterListReceived(QList<ServerCounter> _counterList);
public:
	void responseReceived(ServerResponse resp);
	void addPlayer(const ServerPlayer &player) { playerList.append(player); }
	void addZone(const ServerZone &zone) { zoneList.append(zone); }
	void addCard(const ServerZoneCard &card) { cardList.append(card); }
	void addCounter(const ServerCounter &counter) { counterList.append(counter); }
};

class Client : public QObject {
	Q_OBJECT
signals:
	void statusChanged(ProtocolStatus _status);
	void welcomeMsgReceived(QString welcomeMsg);
	void gameListEvent(const ServerGame &game);
	void playerIdReceived(int id, QString name);
	void gameEvent(const ServerEventData &msg);
	void chatEvent(const ChatEventData &msg);
	void maxPingTime(int seconds, int maxSeconds);
	void serverTimeout();
	void logSocketError(const QString &errorString);
	void serverError(ServerResponse resp);
	void protocolVersionMismatch();
	void protocolError();
private slots:
	void slotConnected();
	void readLine();
	void slotSocketError(QAbstractSocket::SocketError error);
	void ping();
	void removePendingCommand();
	void loginResponse(ServerResponse response);
	void enterGameResponse(ServerResponse response);
	void leaveGameResponse(ServerResponse response);
private:
	static const int protocolVersion = 2;
	static const int maxTimeout = 10;
	
	QTimer *timer;
	QMap<int, PendingCommand *> pendingCommands;
	QTcpSocket *socket;
	ProtocolStatus status;
	QString playerName, password;
	unsigned int MsgId;
	void msg(const QString &s);
	PendingCommand *cmd(const QString &s, PendingCommand *_pc = 0);
	void setStatus(const ProtocolStatus _status);
public:
	Client(QObject *parent = 0);
	~Client();
	ProtocolStatus getStatus() const { return status; }
	QString peerName() const { return socket->peerName(); }

	void connectToServer(const QString &hostname, unsigned int port, const QString &_playerName, const QString &_password);
	void disconnectFromServer();
public slots:
	PendingCommand *chatListChannels();
	PendingCommand_ChatJoinChannel *chatJoinChannel(const QString &name);
	PendingCommand *chatLeaveChannel(const QString &name);
	PendingCommand *chatSay(const QString &name, const QString &s);
	PendingCommand *listGames();
	PendingCommand_ListPlayers *listPlayers();
	PendingCommand *createGame(const QString &description, const QString &password, unsigned int maxPlayers, bool spectatorsAllowed);
	PendingCommand *joinGame(int gameId, const QString &password, bool spectator);
	PendingCommand *leaveGame();
	PendingCommand *login(const QString &name, const QString &pass);
	PendingCommand *say(const QString &s);
	PendingCommand *shuffle();
	PendingCommand *rollDie(unsigned int sides);
	PendingCommand *drawCard() { return drawCards(1); }
	PendingCommand *drawCards(unsigned int number);
	PendingCommand *moveCard(int cardid, const QString &startzone, const QString &targetzone, int x, int y = 0, bool faceDown = false);
	PendingCommand *createToken(const QString &zone, const QString &name, const QString &powtough, int x, int y);
	PendingCommand *createArrow(int startPlayerId, const QString &startZone, int startCardId, int targetPlayerId, const QString &targetPlayerZone, int targetCardId);
	PendingCommand *setCardAttr(const QString &zone, int cardid, const QString &aname, const QString &avalue);
	PendingCommand *readyStart();
	PendingCommand *incCounter(int counterId, int delta);
	PendingCommand *addCounter(const QString &counterName, QColor color, int radius, int value);
	PendingCommand *setCounter(int counterId, int value);
	PendingCommand *delCounter(int counterId);
	PendingCommand_ListCounters *listCounters(int playerId);
	PendingCommand *nextTurn();
	PendingCommand *setActivePhase(int phase);
	PendingCommand_ListZones *listZones(int playerId);
	PendingCommand_DumpZone *dumpZone(int player, const QString &zone, int numberCards);
	PendingCommand *stopDumpZone(int player, const QString &zone);
	PendingCommand_DumpAll *dumpAll();
	void submitDeck(const QStringList &deck);
};

#endif
