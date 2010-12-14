#ifndef PLAYER_H
#define PLAYER_H

#include "server_arrowtarget.h"
#include <QString>
#include <QList>
#include <QMap>
#include "protocol_datastructures.h"

class DeckList;
class Server_Game;
class Server_CardZone;
class Server_Counter;
class Server_Arrow;
class Server_Card;
class Server_ProtocolHandler;
class ProtocolItem;
class ServerInfo_User;
class ServerInfo_PlayerProperties;
class CommandContainer;

class Server_Player : public Server_ArrowTarget {
	Q_OBJECT
private:
	Server_Game *game;
	Server_ProtocolHandler *handler;
	ServerInfo_User *userInfo;
	DeckList *deck;
	QMap<QString, Server_CardZone *> zones;
	QMap<int, Server_Counter *> counters;
	QMap<int, Server_Arrow *> arrows;
	int playerId;
	bool spectator;
	int initialCards;
	int nextCardId;
	bool readyStart;
	bool conceded;
	int deckId;
public:
	Server_Player(Server_Game *_game, int _playerId, ServerInfo_User *_userInfo, bool _spectator, Server_ProtocolHandler *_handler);
	~Server_Player();
	Server_ProtocolHandler *getProtocolHandler() const { return handler; }
	void setProtocolHandler(Server_ProtocolHandler *_handler) { handler = _handler; }
	
	void setPlayerId(int _id) { playerId = _id; }
	int getInitialCards() const { return initialCards; }
	void setInitialCards(int _initialCards) { initialCards = _initialCards; }
	bool getReadyStart() const { return readyStart; }
	void setReadyStart(bool _readyStart) { readyStart = _readyStart; }
	int getPlayerId() const { return playerId; }
	bool getSpectator() const { return spectator; }
	bool getConceded() const { return conceded; }
	void setConceded(bool _conceded) { conceded = _conceded; }
	int getDeckId() const { return deckId; }
	ServerInfo_User *getUserInfo() const { return userInfo; }
	void setDeck(DeckList *_deck, int _deckId);
	DeckList *getDeck() const { return deck; }
	const QMap<QString, Server_CardZone *> &getZones() const { return zones; }
	const QMap<int, Server_Counter *> &getCounters() const { return counters; }
	const QMap<int, Server_Arrow *> &getArrows() const { return arrows; }

	ServerInfo_PlayerProperties *getProperties();
	
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

	ResponseCode moveCard(CommandContainer *cont, const QString &_startZone, int _cardId, const QString &_targetZone, int _x, int _y, bool _faceDown, bool _tapped);
	ResponseCode moveCard(CommandContainer *cont, Server_CardZone *startzone, int _cardId, Server_CardZone *targetzone, int x, int y, bool faceDown, bool tapped);
	void unattachCard(CommandContainer *cont, Server_Card *card);
	ResponseCode setCardAttrHelper(CommandContainer *cont, const QString &zone, int cardId, const QString &attrName, const QString &attrValue);

	void sendProtocolItem(ProtocolItem *item, bool deleteItem = true);
};

#endif
