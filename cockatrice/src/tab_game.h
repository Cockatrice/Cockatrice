#ifndef TAB_GAME_H
#define TAB_GAME_H

#include <QMap>
#include "tab.h"

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
class GameEventContainer;
class GameEventContext;
class GameCommand;
class CommandContainer;
class Event_GameStateChanged;
class Event_PlayerPropertiesChanged;
class Event_Join;
class Event_Leave;
class Event_GameClosed;
class Event_GameStart;
class Event_SetActivePlayer;
class Event_SetActivePhase;
class Event_Ping;
class Player;
class CardZone;
class AbstractCardItem;
class CardItem;

class TabGame : public Tab {
	Q_OBJECT
private:
	Client *client;
	int gameId;
	QString gameDescription;
	int localPlayerId;
	bool spectator;
	QMap<int, Player *> players;
	QMap<int, QString> spectators;
	bool started;
	bool resuming;
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
	QAction *playersSeparator;
	QMenu *playersMenu;
	QAction *aConcede, *aLeaveGame, *aNextPhase, *aNextTurn, *aRemoveLocalArrows;

	Player *addPlayer(int playerId, const QString &playerName);

	void startGame();
	void stopGame();

	void eventGameStateChanged(Event_GameStateChanged *event, GameEventContext *context);
	void eventPlayerPropertiesChanged(Event_PlayerPropertiesChanged *event, GameEventContext *context);
	void eventJoin(Event_Join *event, GameEventContext *context);
	void eventLeave(Event_Leave *event, GameEventContext *context);
	void eventGameClosed(Event_GameClosed *event, GameEventContext *context);
	Player *setActivePlayer(int id);
	void eventSetActivePlayer(Event_SetActivePlayer *event, GameEventContext *context);
	void setActivePhase(int phase);
	void eventSetActivePhase(Event_SetActivePhase *event, GameEventContext *context);
	void eventPing(Event_Ping *event, GameEventContext *context);
signals:
	void gameClosing(TabGame *tab);
private slots:
	void loadLocalDeck();
	void loadRemoteDeck();
	void readyStart();
	void deckSelectFinished(ProtocolResponse *r);
	void newCardAdded(AbstractCardItem *card);
	void sideboardPlanChanged();
	
	void actConcede();
	void actLeaveGame();
	void actRemoveLocalArrows();
	void actSay();
	void actNextPhase();
	void actNextTurn();
public:
	TabGame(Client *_client, int _gameId, const QString &_gameDescription, int _localPlayerId, bool _spectator, bool _resuming);
	~TabGame();
	void retranslateUi();
	const QMap<int, Player *> &getPlayers() const { return players; }
	int getGameId() const { return gameId; }
	QString getTabText() const { return tr("Game %1: %2").arg(gameId).arg(gameDescription); }

	void processGameEventContainer(GameEventContainer *cont);
public slots:
	void sendGameCommand(GameCommand *command);
	void sendCommandContainer(CommandContainer *cont);
};

#endif
