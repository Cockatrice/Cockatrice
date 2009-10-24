#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QMap>

class ServerSocket;
class ServerGame;
class PlayerZone;
class Counter;
class Arrow;

enum PlayerStatusEnum { StatusNormal, StatusSubmitDeck, StatusReadyStart, StatusPlaying };

class Player : public QObject {
	Q_OBJECT
private:
	ServerGame *game;
	ServerSocket *socket;
	QMap<QString, PlayerZone *> zones;
	QMap<int, Counter *> counters;
	QMap<int, Arrow *> arrows;
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
	
	Player(ServerGame *_game, int _playerId, const QString &_playerName, bool _spectator);
	void setSocket(ServerSocket *_socket) { socket = _socket; }
	
	void setStatus(PlayerStatusEnum _status) { PlayerStatus = _status; }
	void setPlayerId(int _id) { playerId = _id; }
	PlayerStatusEnum getStatus() { return PlayerStatus; }
	int getPlayerId() const { return playerId; }
	bool getSpectator() const { return spectator; }
	QString getPlayerName() const { return playerName; }
	const QMap<QString, PlayerZone *> &getZones() const { return zones; }
	const QMap<int, Counter *> &getCounters() const { return counters; }
	const QMap<int, Arrow *> &getArrows() const { return arrows; }
	
	int newCardId();
	int newCounterId() const;
	int newArrowId() const;
	
	void addZone(PlayerZone *zone);
	void addArrow(Arrow *arrow);
	bool deleteArrow(int arrowId);
	void addCounter(Counter *counter);
	bool deleteCounter(int counterId);
	
	void setupZones();

	void privateEvent(const QString &line);
	void publicEvent(const QString &line, Player *player = 0);
};

#endif
