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
class TabGame;
class DeckList;
class QVBoxLayout;
class QHBoxLayout;
class ServerInfo_User;

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

class DeckViewContainer : public QWidget {
	Q_OBJECT
private:
	QPushButton *loadLocalButton, *loadRemoteButton;
	ReadyStartButton *readyStartButton;
	DeckView *deckView;
	AbstractClient *client;
private slots:
	void loadLocalDeck();
	void loadRemoteDeck();
	void readyStart();
	void deckSelectFinished(ProtocolResponse *r);
	void sideboardPlanChanged();
signals:
	void newCardAdded(AbstractCardItem *card);
public:
	DeckViewContainer(AbstractClient *_client, TabGame *parent = 0);
	void retranslateUi();
	void setButtonsVisible(bool _visible);
	void setReadyStart(bool ready);
	void setDeck(DeckList *deck);
};

class TabGame : public Tab {
	Q_OBJECT
private:
	QList<AbstractClient *> clients;
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

	CardInfoWidget *infoPopup;
	CardInfoWidget *cardInfo;
	PlayerListWidget *playerListWidget;
	QLabel *timeElapsedLabel;
	MessageLogWidget *messageLog;
	QLabel *sayLabel;
	QLineEdit *sayEdit;
	PhasesToolbar *phasesToolbar;
	GameScene *scene;
	GameView *gameView;
	QMap<int, DeckViewContainer *> deckViewContainers;
	QVBoxLayout *deckViewContainerLayout;
	QHBoxLayout *mainLayout;
	ZoneViewLayout *zoneLayout;
	QAction *playersSeparator;
	QMenu *playersMenu;
	QAction *aConcede, *aLeaveGame, *aNextPhase, *aNextTurn, *aRemoveLocalArrows;

	Player *addPlayer(int playerId, ServerInfo_User *info);

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
	void playerAdded(Player *player);
	void playerRemoved(Player *player);
private slots:
	void newCardAdded(AbstractCardItem *card);
	void showCardInfoPopup(const QPoint &pos, const QString &cardName);
	void deleteCardInfoPopup();
	
	void actConcede();
	void actLeaveGame();
	void actRemoveLocalArrows();
	void actSay();
	void actNextPhase();
	void actNextTurn();
public:
	TabGame(QList<AbstractClient *> &_clients, int _gameId, const QString &_gameDescription, int _localPlayerId, bool _spectator, bool _spectatorsCanTalk, bool _spectatorsSeeEverything, bool _resuming);
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

	void processGameEventContainer(GameEventContainer *cont, AbstractClient *client);
public slots:
	void sendGameCommand(GameCommand *command, int playerId = -1);
	void sendCommandContainer(CommandContainer *cont, int playerId = -1);
};

#endif
