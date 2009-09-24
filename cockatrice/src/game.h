#ifndef GAME_H
#define GAME_H

#include <QHash>
#include <QStringList>
#include "playerlist.h"
#include "client.h"

class GameScene;
class Player;
class ServerEventData;
class CardDatabase;
class DlgStartGame;
class CardItem;
class QMenuBar;

class Game : public QObject {
	Q_OBJECT
private:
	static const int phaseCount = 11;
	
	typedef void (Game::*CardMenuHandler)(CardItem *card);
	QHash<QAction *, CardMenuHandler> cardMenuHandlers;
	
	QMenu *gameMenu, *cardMenu, *moveMenu;
	QAction *aTap, *aUntap, *aDoesntUntap, *aFlip, *aAddCounter, *aRemoveCounter, *aSetCounters,
		*aMoveToTopLibrary, *aMoveToBottomLibrary, *aMoveToGraveyard, *aMoveToExile,
		*aNextPhase, *aNextTurn;
	DlgStartGame *dlgStartGame;

	CardDatabase *db;
	Client *client;
	GameScene *scene;
	QStringList spectatorList;
	PlayerList players;
	bool started;
	int currentPhase;
public slots:
	void actNextPhase();
	void actNextTurn();
private slots:
	void cardMenuAction();

	void showCardMenu(QPoint p);
	void actTap(CardItem *card);
	void actUntap(CardItem *card);
	void actDoesntUntap(CardItem *card);
	void actFlip(CardItem *card);
	void actAddCounter(CardItem *card);
	void actRemoveCounter(CardItem *card);
	void actSetCounters();
	void actMoveToTopLibrary(CardItem *card);
	void actMoveToBottomLibrary(CardItem *card);
	void actMoveToGraveyard(CardItem *card);
	void actMoveToExile(CardItem *card);

	void gameEvent(const ServerEventData &msg);
	
	void playerListReceived(QList<ServerPlayer> playerList);
	void cardListReceived(QList<ServerZoneCard> list);
	void zoneListReceived(QList<ServerZone> list);
	void counterListReceived(QList<ServerCounter> list);
	
	void readyStart();
signals:
	void submitDecklist();
	void hoverCard(QString name);
	void playerAdded(Player *player);
	void playerRemoved(Player *player);

	// Log events
	void logPlayerListReceived(QStringList players);
	void logJoin(Player *player);
	void logLeave(Player *player);
	void logGameClosed();
	void logJoinSpectator(QString playerName);
	void logLeaveSpectator(QString playerName);
	void logReadyStart(Player *player);
	void logGameStart();
	void logSay(Player *player, QString text);
	void logShuffle(Player *player);
	void logRollDie(Player *player, int sides, int roll);
	void logDraw(Player *player, int number);
	void logMoveCard(Player *player, QString cardName, CardZone *startZone, int oldX, CardZone *targetZone, int newX);
	void logCreateToken(Player *player, QString cardName);
	void logSetCardCounters(Player *player, QString cardName, int value, int oldValue);
	void logSetTapped(Player *player, QString cardName, bool tapped);
	void logSetCounter(Player *player, QString counterName, int value, int oldValue);
	void logSetDoesntUntap(Player *player, QString cardName, bool doesntUntap);
	void logDumpZone(Player *player, CardZone *zone, int numberCards);
	void logStopDumpZone(Player *player, CardZone *zone);
	void logSetActivePlayer(Player *player);
	void setActivePhase(int phase);
public:
	Game(CardDatabase *_db, Client *_client, GameScene *_scene, QMenuBar *menuBar, QObject *parent = 0);
	~Game();
	void retranslateUi();
	void restartGameDialog();
	void hoverCardEvent(CardItem *card);
	Player *addPlayer(int playerId, const QString &playerName, bool local);
	void queryGameState();
};

#endif
