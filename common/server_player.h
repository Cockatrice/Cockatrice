#ifndef PLAYER_H
#define PLAYER_H

#include "server_arrowtarget.h"
#include <QString>
#include <QList>
#include <QMap>
#include <QMutex>

#include "pb/response.pb.h"
#include "pb/card_attributes.pb.h"

class DeckList;
class Server_Game;
class Server_CardZone;
class Server_Counter;
class Server_Arrow;
class Server_Card;
class Server_ProtocolHandler;
class ServerInfo_User;
class ServerInfo_PlayerProperties;
class CommandContainer;
class CardToMove;
class GameEventContainer;
class GameEventStorage;

class Server_Player : public Server_ArrowTarget {
	Q_OBJECT
private:
	class MoveCardCompareFunctor;
	Server_Game *game;
	Server_ProtocolHandler *handler;
	ServerInfo_User *userInfo;
	DeckList *deck;
	QMap<QString, Server_CardZone *> zones;
	QMap<int, Server_Counter *> counters;
	QMap<int, Server_Arrow *> arrows;
	QList<int> lastDrawList;
	int pingTime;
	int playerId;
	bool spectator;
	int initialCards;
	int nextCardId;
	bool readyStart;
	bool conceded;
public:
	mutable QMutex playerMutex;
	Server_Player(Server_Game *_game, int _playerId, const ServerInfo_User &_userInfo, bool _spectator, Server_ProtocolHandler *_handler);
	~Server_Player();
	void prepareDestroy();
	Server_ProtocolHandler *getProtocolHandler() const { return handler; }
	void setProtocolHandler(Server_ProtocolHandler *_handler);
	
	void setPlayerId(int _id) { playerId = _id; }
	int getInitialCards() const { return initialCards; }
	void setInitialCards(int _initialCards) { initialCards = _initialCards; }
	bool getReadyStart() const { return readyStart; }
	void setReadyStart(bool _readyStart) { readyStart = _readyStart; }
	int getPlayerId() const { return playerId; }
	bool getSpectator() const { return spectator; }
	bool getConceded() const { return conceded; }
	void setConceded(bool _conceded) { conceded = _conceded; }
	ServerInfo_User *getUserInfo() const { return userInfo; }
	void setDeck(DeckList *_deck);
	DeckList *getDeck() const { return deck; }
	Server_Game *getGame() const { return game; }
	const QMap<QString, Server_CardZone *> &getZones() const { return zones; }
	const QMap<int, Server_Counter *> &getCounters() const { return counters; }
	const QMap<int, Server_Arrow *> &getArrows() const { return arrows; }
	
	int getPingTime() const { return pingTime; }
	void setPingTime(int _pingTime) { pingTime = _pingTime; }
	ServerInfo_PlayerProperties getProperties(bool withUserInfo);
	
	int newCardId();
	int newCounterId() const;
	int newArrowId() const;
	
	void addZone(Server_CardZone *zone);
	void addArrow(Server_Arrow *arrow);
	bool deleteArrow(int arrowId);
	void addCounter(Server_Counter *counter);
	bool deleteCounter(int counterId);
	
	void clearZones();
	void setupZones();

	Response::ResponseCode drawCards(GameEventStorage &ges, int number);
	Response::ResponseCode undoDraw(GameEventStorage &ges);
	Response::ResponseCode moveCard(GameEventStorage &ges, const QString &_startZone, const QList<const CardToMove *> &_cards, int _targetPlayer, const QString &_targetZone, int _x, int _y);
	Response::ResponseCode moveCard(GameEventStorage &ges, Server_CardZone *startzone, const QList<const CardToMove *> &_cards, Server_CardZone *targetzone, int x, int y, bool fixFreeSpaces = true, bool undoingDraw = false);
	void unattachCard(GameEventStorage &ges, Server_Card *card);
	Response::ResponseCode setCardAttrHelper(GameEventStorage &ges, const QString &zone, int cardId, CardAttribute attribute, const QString &attrValue);

	void sendGameEvent(const GameEventContainer &event);
};

#endif
