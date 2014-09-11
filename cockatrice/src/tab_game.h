#ifndef TAB_GAME_H
#define TAB_GAME_H

#include <QMap>
#include <QPushButton>
#include "tab.h"
#include "pb/serverinfo_game.pb.h"

class AbstractClient;
class CardDatabase;
class GameView;
class DeckView;
class GameScene;
class CardInfoWidget;
class MessageLogWidget;
class QTimer;
class QSplitter;
class QLabel;
class QLineEdit;
class QPushButton;
class QToolButton;
class QMenu;
class ZoneViewLayout;
class ZoneViewWidget;
class PhasesToolbar;
class PlayerListWidget;
class ReplayTimelineWidget;
class Response;
class GameEventContainer;
class GameEventContext;
class GameCommand;
class CommandContainer;
class Event_GameJoined;
class Event_GameStateChanged;
class Event_PlayerPropertiesChanged;
class Event_Join;
class Event_Leave;
class Event_GameHostChanged;
class Event_GameClosed;
class Event_GameStart;
class Event_SetActivePlayer;
class Event_SetActivePhase;
class Event_Ping;
class Event_GameSay;
class Event_Kicked;
class Player;
class CardZone;
class AbstractCardItem;
class CardItem;
class TabGame;
class DeckLoader;
class QVBoxLayout;
class QHBoxLayout;
class GameReplay;
class ServerInfo_User;
class PendingCommand;

class ToggleButton : public QPushButton {
    Q_OBJECT
private:
    bool state;
signals:
    void stateChanged();
public:
    ToggleButton(QWidget *parent = 0);
    bool getState() const { return state; }
    void setState(bool _state);
protected:
    void paintEvent(QPaintEvent *event);
};

class DeckViewContainer : public QWidget {
    Q_OBJECT
private:
    QPushButton *loadLocalButton, *loadRemoteButton;
    ToggleButton *readyStartButton, *sideboardLockButton;
    DeckView *deckView;
    int playerId;
private slots:
    void loadLocalDeck();
    void loadRemoteDeck();
    void readyStart();
    void deckSelectFinished(const Response &r);
    void sideboardPlanChanged();
    void sideboardLockButtonClicked();
    void updateSideboardLockButtonText();
signals:
    void newCardAdded(AbstractCardItem *card);
public:
    DeckViewContainer(int _playerId, TabGame *parent = 0);
    void retranslateUi();
    void setButtonsVisible(bool _visible);
    void setReadyStart(bool ready);
    void setSideboardLocked(bool locked);
    void setDeck(const DeckLoader &deck);
};

class TabGame : public Tab {
    Q_OBJECT
private:
    QTimer *gameTimer;
    int secondsElapsed;
    QList<AbstractClient *> clients;
    ServerInfo_Game gameInfo;
    QMap<int, QString> roomGameTypes;
    int hostId;
    int localPlayerId;
    bool spectator;
    QMap<int, Player *> players;
    QMap<int, ServerInfo_User> spectators;
    bool gameStateKnown;
    bool resuming;
    QStringList phasesList;
    int currentPhase;
    int activePlayer;
    CardItem *activeCard;
    bool gameClosed;
    
    // Replay related members
    GameReplay *replay;
    int currentReplayStep;
    QList<int> replayTimeline;
    ReplayTimelineWidget *timelineWidget;
    QToolButton *replayToStartButton, *replayStartButton, *replayPauseButton, *replayStopButton, *replayFastForwardButton, *replayToEndButton;
    
    QSplitter *splitter;
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
    QMenu *gameMenu;
    QMenu *phasesMenu;
    QAction *aGameInfo, *aConcede, *aLeaveGame, *aCloseReplay, *aNextPhase, *aNextTurn, *aRemoveLocalArrows;
    QList<QAction *> phaseActions;

    Player *addPlayer(int playerId, const ServerInfo_User &info);

    void startGame(bool resuming);
    void stopGame();
    void closeGame();

    void eventSpectatorSay(const Event_GameSay &event, int eventPlayerId, const GameEventContext &context);
    void eventSpectatorLeave(const Event_Leave &event, int eventPlayerId, const GameEventContext &context);
    
    void eventGameStateChanged(const Event_GameStateChanged &event, int eventPlayerId, const GameEventContext &context);
    void eventPlayerPropertiesChanged(const Event_PlayerPropertiesChanged &event, int eventPlayerId, const GameEventContext &context);
    void eventJoin(const Event_Join &event, int eventPlayerId, const GameEventContext &context);
    void eventLeave(const Event_Leave &event, int eventPlayerId, const GameEventContext &context);
    void eventKicked(const Event_Kicked &event, int eventPlayerId, const GameEventContext &context);
    void eventGameHostChanged(const Event_GameHostChanged &event, int eventPlayerId, const GameEventContext &context);
    void eventGameClosed(const Event_GameClosed &event, int eventPlayerId, const GameEventContext &context);
    Player *setActivePlayer(int id);
    void eventSetActivePlayer(const Event_SetActivePlayer &event, int eventPlayerId, const GameEventContext &context);
    void setActivePhase(int phase);
    void eventSetActivePhase(const Event_SetActivePhase &event, int eventPlayerId, const GameEventContext &context);
    void eventPing(const Event_Ping &event, int eventPlayerId, const GameEventContext &context);
signals:
    void gameClosing(TabGame *tab);
    void playerAdded(Player *player);
    void playerRemoved(Player *player);
    void containerProcessingStarted(const GameEventContext &context);
    void containerProcessingDone();
    void openMessageDialog(const QString &userName, bool focus);
    void openDeckEditor(const DeckLoader *deck);
private slots:
    void replayNextEvent();
    void replayFinished();
    void replayToStartButtonClicked();
    void replayStartButtonClicked();
    void replayPauseButtonClicked();
    void replayStopButtonClicked();
    void replayFastForwardButtonToggled(bool checked);
    void replayToEndButtonClicked();
    
    void incrementGameTime();
    void adminLockChanged(bool lock);
    void newCardAdded(AbstractCardItem *card);
    void updateCardMenu(AbstractCardItem *card);
    
    void actGameInfo();
    void actConcede();
    void actLeaveGame();
    void actRemoveLocalArrows();
    void actSay();
    void actPhaseAction();
    void actNextPhase();
    void actNextTurn();
public:
    TabGame(TabSupervisor *_tabSupervisor, QList<AbstractClient *> &_clients, const Event_GameJoined &event, const QMap<int, QString> &_roomGameTypes);
    TabGame(TabSupervisor *_tabSupervisor, GameReplay *replay);
    ~TabGame();
    void retranslateUi();
    void closeRequest();
    const QMap<int, Player *> &getPlayers() const { return players; }
    CardItem *getCard(int playerId, const QString &zoneName, int cardId) const;
    bool isHost() const { return hostId == localPlayerId; }
    int getGameId() const { return gameInfo.game_id(); }
    QString getTabText() const;
    bool getSpectator() const { return spectator; }
    bool getSpectatorsSeeEverything() const { return gameInfo.spectators_omniscient(); }
    Player *getActiveLocalPlayer() const;
    AbstractClient *getClientForPlayer(int playerId) const;
    
    void setActiveCard(CardItem *_card) { activeCard = _card; }
    CardItem *getActiveCard() const { return activeCard; }

    void processGameEventContainer(const GameEventContainer &cont, AbstractClient *client);
    PendingCommand *prepareGameCommand(const ::google::protobuf::Message &cmd);
    PendingCommand *prepareGameCommand(const QList< const ::google::protobuf::Message * > &cmdList);
public slots:
    void sendGameCommand(PendingCommand *pend, int playerId = -1);
    void sendGameCommand(const ::google::protobuf::Message &command, int playerId = -1);
};

#endif
