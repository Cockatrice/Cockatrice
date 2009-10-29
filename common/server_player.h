#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QMap>

class ServerSocket;
class Server_Game;
class Server_CardZone;
class Server_Counter;
class Server_Arrow;

enum PlayerStatusEnum { StatusNormal, StatusSubmitDeck, StatusReadyStart, StatusPlaying };

class Server_Player : public QObject {
	Q_OBJECT
private:
	Server_Game *game;
	ServerSocket *socket;
	QMap<QString, Server_CardZone *> zones;
	QMap<int, Server_Counter *> counters;
	QMap<int, Server_Arrow *> arrows;
	int playerId;
	QString playerName;
	bool spectator;
	int nextCardId;
	void clearZones();
	PlayerStatusEnum PlayerStatus;
public:
	// Pfusch
	QList<QString> DeckList;
	QList<QString> SideboardList;
	// Pfusch Ende
	
	Server_Player(Server_Game *_game, int _playerId, const QString &_playerName, bool _spectator);
	void setSocket(ServerSocket *_socket) { socket = _socket; }
	
	void setStatus(PlayerStatusEnum _status) { PlayerStatus = _status; }
	void setPlayerId(int _id) { playerId = _id; }
	PlayerStatusEnum getStatus() { return PlayerStatus; }
	int getPlayerId() const { return playerId; }
	bool getSpectator() const { return spectator; }
	QString getPlayerName() const { return playerName; }
	const QMap<QString, Server_CardZone *> &getZones() const { return zones; }
	const QMap<int, Server_Counter *> &getCounters() const { return counters; }
	const QMap<int, Server_Arrow *> &getArrows() const { return arrows; }
	
	int newCardId();
	int newCounterId() const;
	int newArrowId() const;
	
	void addZone(Server_CardZone *zone);
	void addArrow(Server_Arrow *arrow);
	bool deleteArrow(int arrowId);
	void addCounter(Server_Counter *counter);
	bool deleteCounter(int counterId);
	
	void setupZones();

	void privateEvent(const QString &line);
	void publicEvent(const QString &line, Server_Player *player = 0);
};

#endif
