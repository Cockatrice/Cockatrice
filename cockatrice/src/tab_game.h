#ifndef TAB_GAME_H
#define TAB_GAME_H

#include <QMap>
#include <QPushButton>
#include "tab.h"

class AbstractClient;
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
class Event_Say;
class Player;
class CardZone;
class AbstractCardItem;
class CardItem;

class ReadyStartButton : public QPushButton {
	Q_OBJECT
private:
	bool readyStart;
public:
	ReadyStartButton(QWidget *parent = 0);
	bool getReadyStart() const { return readyStart; }
	void setReadyStart(bool _readyStart);
protected:
	void paintEvent(QPaintEvent *event);
};

class TabGame : public Tab {
	Q_OBJECT
private:
	AbstractClient *client;
	int gameId;
	QString gameDescription;
	int localPlayerId;
	bool spectator;
	bool spectatorsCanTalk, spectatorsSeeEverything;
	QMap<int, Player *> players;
	QMap<int, QString> spectators;
	bool started;
	bool resuming;
	int currentPhase;
	int activePlayer;

	QPushButton *loadLocalButton, *loadRemoteButton;
	ReadyStartButton *readyStartButton;
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

	void eventSpectatorSay(Event_Say *event, GameEventContext *context);
	void eventSpectatorLeave(Event_Leave *event, GameEventContext *context);
	
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
	TabGame(AbstractClient *_client, int _gameId, const QString &_gameDescription, int _localPlayerId, bool _spectator, bool _spectatorsCanTalk, bool _spectatorsSeeEverything, bool _resuming);
	~TabGame();
	void retranslateUi();
	const QMap<int, Player *> &getPlayers() const { return players; }
	CardItem *getCard(int playerId, const QString &zoneName, int cardId) const;
	int getGameId() const { return gameId; }
	QString getTabText() const { return tr("Game %1: %2").arg(gameId).arg(gameDescription); }
	bool getSpectator() const { return spectator; }
	bool getSpectatorsCanTalk() const { return spectatorsCanTalk; }
	bool getSpectatorsSeeEverything() const { return spectatorsSeeEverything; }
	Player *getActiveLocalPlayer() const;

	void processGameEventContainer(GameEventContainer *cont);
public slots:
	void sendGameCommand(GameCommand *command);
	void sendCommandContainer(CommandContainer *cont);
};

#endif
