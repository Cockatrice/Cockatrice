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
class QMenu;
class ZoneViewLayout;
class ZoneViewWidget;
class PhasesToolbar;
class PlayerListWidget;
class ProtocolResponse;
class GameEvent;
class GameCommand;
class Event_GameStart;
class Event_GameStateChanged;
class Event_Join;
class Event_Leave;
class Event_GameClosed;
class Event_GameStart;
class Event_SetActivePlayer;
class Event_SetActivePhase;
class Player;
class CardZone;
class CardItem;

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
	PlayerListWidget *playerListWidget;
	MessageLogWidget *messageLog;
	QLabel *sayLabel;
	QLineEdit *sayEdit;
	PhasesToolbar *phasesToolbar;
	GameScene *scene;
	GameView *gameView;
	DeckView *deckView;
	QWidget *deckViewContainer;
	ZoneViewLayout *zoneLayout;
	QAction *aCloseMostRecentZoneView,
		*aNextPhase, *aNextTurn, *aRemoveLocalArrows;
	QMenu *gameMenu;

	Player *addPlayer(int playerId, const QString &playerName);

	void eventGameStart(Event_GameStart *event);
	void eventGameStateChanged(Event_GameStateChanged *event);
	void eventJoin(Event_Join *event);
	void eventLeave(Event_Leave *event);
	void eventGameClosed(Event_GameClosed *event);
	void eventSetActivePlayer(Event_SetActivePlayer *event);
	void eventSetActivePhase(Event_SetActivePhase *event);
signals:
	// -- XXX --
	void playerAdded(Player *player);
	void playerRemoved(Player *player);
	// -- XXX --
private slots:
	void loadLocalDeck();
	void loadRemoteDeck();
	void readyStart();
	void deckSelectFinished(ProtocolResponse *r);
	void newCardAdded(CardItem *card);
	
	void actRemoveLocalArrows();
	void actSay();
	void actNextPhase();
	void actNextTurn();
public:
	TabGame(Client *_client, int _gameId, int _localPlayerId, bool _spectator);
	void retranslateUi();
	const QMap<int, Player *> &getPlayers() const { return players; }

	void processGameEvent(GameEvent *event);
public slots:
	void sendGameCommand(GameCommand *command);
};

#endif
