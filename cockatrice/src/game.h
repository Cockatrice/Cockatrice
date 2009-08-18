#ifndef GAME_H
#define GAME_H

#include "playerlist.h"

class ServerPlayer;
class QGraphicsScene;
class Player;
class Client;
class ServerEventData;
class CardDatabase;
class DlgStartGame;
class CardItem;

class Game : public QObject {
	Q_OBJECT
private:
	QMenu *actionsMenu, *sayMenu, *cardMenu;
	QAction *aTap, *aUntap, *aDoesntUntap, *aFlip, *aAddCounter, *aRemoveCounter, *aSetCounters,
		*aUntapAll, *aDecLife, *aIncLife, *aSetLife, *aShuffle, *aDraw, *aDrawCards, *aRollDice, *aCreateToken;
	DlgStartGame *dlgStartGame;

	CardDatabase *db;
	Client *client;
	QGraphicsScene *scene;
	PlayerList players;
	Player *localPlayer;
	bool started;
	Player *addPlayer(int playerId, const QString &playerName, QPointF base, bool local);
	void initSayMenu();
private slots:
	void actUntapAll();
	void actIncLife();
	void actDecLife();
	void actSetLife();
	void actShuffle();
	void actDrawCard();
	void actDrawCards();
	void actRollDice();
	void actCreateToken();

	void showCardMenu(QPoint p);
	void actDoesntUntap();
	void actFlip();
	void actAddCounter();
	void actRemoveCounter();
	void actSetCounters();
	
	void actSayMessage();

	void gameEvent(const ServerEventData &msg);
	void playerListReceived(QList<ServerPlayer *> playerList);
	void readyStart();
public slots:
	void actTap();
	void actUntap();
signals:
	void submitDecklist();
	void hoverCard(QString name);
	void playerAdded(Player *player);
	void playerRemoved(Player *player);

	// Log events
	void logPlayerListReceived(QStringList players);
	void logJoin(Player *player);
	void logLeave(Player *player);
	void logReadyStart(Player *player);
	void logGameStart();
	void logSay(Player *player, QString text);
	void logShuffle(Player *player);
	void logRollDice(Player *player, int sides, int roll);
	void logDraw(Player *player, int number);
	void logMoveCard(Player *player, QString cardName, CardZone *startZone, int oldX, CardZone *targetZone, int newX);
	void logCreateToken(Player *player, QString cardName);
	void logSetCardCounters(Player *player, QString cardName, int value, int oldValue);
	void logSetTapped(Player *player, QString cardName, bool tapped);
	void logSetCounter(Player *player, QString counterName, int value, int oldValue);
	void logSetDoesntUntap(Player *player, QString cardName, bool doesntUntap);
	void logDumpZone(Player *player, QString zoneName, QString zoneOwner, int numberCards);
	void logStopDumpZone(Player *player, QString zoneName, QString zoneOwner);
	void logSetActivePlayer(Player *player);
	void setActivePhase(int phase);
public:
	Game(CardDatabase *_db, Client *_client, QGraphicsScene *_scene, QMenu *_actionsMenu, QMenu *_cardMenu, int playerId, const QString &playerName, QObject *parent = 0);
	~Game();
	Player *getLocalPlayer() const { return localPlayer; }
	void restartGameDialog();
	void hoverCardEvent(CardItem *card);
};

#endif
