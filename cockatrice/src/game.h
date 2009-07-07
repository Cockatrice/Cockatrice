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

class Game : public QObject {
	Q_OBJECT
private:
	QMenu *actionsMenu, *sayMenu, *cardMenu;
	QAction *aTap, *aUntap, *aDoesntUntap, *aFlip, *aAddCounter, *aRemoveCounter, *aSetCounters,
		*aUntapAll, *aDecLife, *aIncLife, *aSetLife, *aShuffle, *aDraw, *aDrawCards, *aRollDice, *aCreateToken, *aEditMessages;
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
	void actEditMessages();

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
	void logJoin(QString playerName);
	void logLeave(QString playerName);
	void logReadyStart(QString playerName);
	void logGameStart();
	void logSay(QString playerName, QString text);
	void logShuffle(QString playerName);
	void logRollDice(QString playerName, int sides, int roll);
	void logDraw(QString playerName, int number);
	void logMoveCard(QString playerName, QString cardName, QString startZone, QString targetZone);
	void logCreateToken(QString playerName, QString cardName);
	void logSetCardCounters(QString playerName, QString cardName, int value, int oldValue);
	void logSetTapped(QString playerName, QString cardName, bool tapped);
	void logSetCounter(QString playerName, QString counterName, int value, int oldValue);
	void logSetDoesntUntap(QString playerName, QString cardName, bool doesntUntap);
	void logDumpZone(QString playerName, QString zoneName, QString zoneOwner, int numberCards);
public:
	Game(CardDatabase *_db, Client *_client, QGraphicsScene *_scene, QMenu *_actionsMenu, QMenu *_cardMenu, int playerId, const QString &playerName, QObject *parent = 0);
	~Game();
	Player *getLocalPlayer() const { return localPlayer; }
	void restartGameDialog();
};

#endif
