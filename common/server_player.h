#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QMap>

class DeckList;
class Server_Game;
class Server_CardZone;
class Server_Counter;
class Server_Arrow;
class Server_ProtocolHandler;
class ProtocolItem;

class Server_Player : public QObject {
	Q_OBJECT
private:
	Server_Game *game;
	Server_ProtocolHandler *handler;
	DeckList *deck;
	QMap<QString, Server_CardZone *> zones;
	QMap<int, Server_Counter *> counters;
	QMap<int, Server_Arrow *> arrows;
	int playerId;
	QString playerName;
	bool spectator;
	int nextCardId;
	void clearZones();
	bool readyStart;
public:
	Server_Player(Server_Game *_game, int _playerId, const QString &_playerName, bool _spectator, Server_ProtocolHandler *_handler);
	void setProtocolHandler(Server_ProtocolHandler *_handler) { handler = _handler; }
	
	void setPlayerId(int _id) { playerId = _id; }
	bool getReadyStart() const { return readyStart; }
	void setReadyStart(bool _readyStart) { readyStart = _readyStart; }
	int getPlayerId() const { return playerId; }
	bool getSpectator() const { return spectator; }
	QString getPlayerName() const { return playerName; }
	void setDeck(DeckList *_deck);
	DeckList *getDeck() const { return deck; }
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

	void sendProtocolItem(ProtocolItem *item, bool deleteItem = true);
};

#endif
