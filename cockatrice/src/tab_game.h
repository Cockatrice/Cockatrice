#ifndef TAB_GAME_H
#define TAB_GAME_H

#include <QWidget>
#include <QMap>

class Client;
class CardDatabase;
class GameView;
class DeckView;
class GameScene;
class CardInfoWidget;
class MessageLogWidget;
class QLabel;
class QLineEdit;
class QPushButton;
class ZoneViewLayout;
class ZoneViewWidget;
class PhasesToolbar;
class ProtocolResponse;
class GameEvent;
class Event_GameJoined;
class Event_GameStart;
class Player;
class CardZone;

class TabGame : public QWidget {
	Q_OBJECT
private:
	Client *client;
	int gameId;
	int localPlayerId;
	bool spectator;
	QStringList spectatorList;
	QMap<int, Player *> players;
	bool started;
	int currentPhase;

	QPushButton *loadLocalButton, *loadRemoteButton, *readyStartButton;
	CardInfoWidget *cardInfo;
	MessageLogWidget *messageLog;
	QLabel *sayLabel;
	QLineEdit *sayEdit;
	PhasesToolbar *phasesToolbar;
	GameScene *scene;
	GameView *gameView;
	DeckView *deckView;
	QWidget *deckViewContainer;
	ZoneViewLayout *zoneLayout;
	QAction *aCloseMostRecentZoneView;

	Player *addPlayer(int playerId, const QString &playerName);

	void eventGameStart(Event_GameStart *event);
signals:
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
	void logCreateArrow(Player *player, Player *startPlayer, QString startCard, Player *targetPlayer, QString targetCard);
	void logSetCardCounters(Player *player, QString cardName, int value, int oldValue);
	void logSetTapped(Player *player, QString cardName, bool tapped);
	void logSetCounter(Player *player, QString counterName, int value, int oldValue);
	void logSetDoesntUntap(Player *player, QString cardName, bool doesntUntap);
	void logDumpZone(Player *player, CardZone *zone, int numberCards);
	void logStopDumpZone(Player *player, CardZone *zone);
	void logSetActivePlayer(Player *player);
	void setActivePhase(int phase);
private slots:
	void loadLocalDeck();
	void loadRemoteDeck();
	void readyStart();
	void deckSelectFinished(ProtocolResponse *r);
public:
	TabGame(Client *_client, int _gameId);
	void retranslateUi();

	void processGameEvent(GameEvent *event);
	void processGameJoinedEvent(Event_GameJoined *event);
};

#endif
