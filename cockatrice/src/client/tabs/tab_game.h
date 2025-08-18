#ifndef TAB_GAME_H
#define TAB_GAME_H

#include "../../client/tearoff_menu.h"
#include "../../game/player/player.h"
#include "../replay_manager.h"
#include "../ui/widgets/visual_deck_storage/visual_deck_storage_widget.h"
#include "pb/event_leave.pb.h"
#include "pb/serverinfo_game.pb.h"
#include "tab.h"

#include <QCompleter>
#include <QLoggingCategory>
#include <QMap>

class TabbedDeckViewContainer;
inline Q_LOGGING_CATEGORY(TabGameLog, "tab_game");

class UserListProxy;
class DeckViewContainer;
class AbstractClient;
class CardDatabase;
class GameView;
class GameScene;
class ReplayManager;
class CardInfoFrameWidget;
class MessageLogWidget;
class QTimer;
class QSplitter;
class QLabel;
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
class Event_ReverseTurn;
class CardZone;
class AbstractCardItem;
class CardItem;
class DeckLoader;
class QVBoxLayout;
class QHBoxLayout;
class GameReplay;
class ServerInfo_User;
class PendingCommand;
class LineEditCompleter;
class QDockWidget;
class QStackedWidget;

class TabGame : public Tab
{
    Q_OBJECT
private:
    QTimer *gameTimer;
    int secondsElapsed;
    const UserListProxy *userListProxy;
    QList<AbstractClient *> clients;
    ServerInfo_Game gameInfo;
    QMap<int, QString> roomGameTypes;
    int hostId;
    int localPlayerId;
    const bool isLocalGame;
    bool spectator;
    bool judge;
    QMap<int, Player *> players;
    QMap<int, ServerInfo_User> spectators;
    bool gameStateKnown;
    bool resuming;
    QStringList phasesList;
    int currentPhase;
    int activePlayer;
    CardItem *activeCard;
    bool gameClosed;
    ReplayManager *replayManager;
    QStringList gameTypes;
    QCompleter *completer;
    QStringList autocompleteUserList;
    QStackedWidget *mainWidget;

    CardInfoFrameWidget *cardInfoFrameWidget;
    PlayerListWidget *playerListWidget;
    QLabel *timeElapsedLabel;
    MessageLogWidget *messageLog;
    QLabel *sayLabel;
    LineEditCompleter *sayEdit;
    PhasesToolbar *phasesToolbar;
    GameScene *scene;
    GameView *gameView;
    QMap<int, TabbedDeckViewContainer *> deckViewContainers;
    QVBoxLayout *deckViewContainerLayout;
    QWidget *gamePlayAreaWidget, *deckViewContainerWidget;
    QDockWidget *cardInfoDock, *messageLayoutDock, *playerListDock, *replayDock;
    QAction *playersSeparator;
    QMenu *gameMenu, *viewMenu, *cardInfoDockMenu, *messageLayoutDockMenu, *playerListDockMenu, *replayDockMenu;
    TearOffMenu *phasesMenu;
    QAction *aGameInfo, *aConcede, *aLeaveGame, *aCloseReplay, *aNextPhase, *aNextPhaseAction, *aNextTurn,
        *aReverseTurn, *aRemoveLocalArrows, *aRotateViewCW, *aRotateViewCCW, *aResetLayout, *aResetReplayLayout;
    QAction *aCardInfoDockVisible, *aCardInfoDockFloating, *aMessageLayoutDockVisible, *aMessageLayoutDockFloating,
        *aPlayerListDockVisible, *aPlayerListDockFloating, *aReplayDockVisible, *aReplayDockFloating;
    QAction *aFocusChat;
    QList<QAction *> phaseActions;
    QAction *aCardMenu;

    Player *addPlayer(int playerId, const ServerInfo_User &info);

    bool isMainPlayerConceded() const;

    void startGame(bool resuming);
    void stopGame();
    void closeGame();
    bool leaveGame();

    void eventSpectatorSay(const Event_GameSay &event, int eventPlayerId, const GameEventContext &context);
    void eventSpectatorLeave(const Event_Leave &event, int eventPlayerId, const GameEventContext &context);

    void eventGameStateChanged(const Event_GameStateChanged &event, int eventPlayerId, const GameEventContext &context);
    void eventPlayerPropertiesChanged(const Event_PlayerPropertiesChanged &event,
                                      int eventPlayerId,
                                      const GameEventContext &context);
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
    void eventReverseTurn(const Event_ReverseTurn &event, int eventPlayerId, const GameEventContext & /*context*/);
    void emitUserEvent();
    void createMenuItems();
    void createReplayMenuItems();
    void createViewMenuItems();
    void createCardInfoDock(bool bReplay = false);
    void createPlayerListDock(bool bReplay = false);
    void createMessageDock(bool bReplay = false);
    void createPlayAreaWidget(bool bReplay = false);
    void createDeckViewContainerWidget(bool bReplay = false);
    void createReplayDock(GameReplay *replay);
    QString getLeaveReason(Event_Leave::LeaveReason reason);
signals:
    void gameClosing(TabGame *tab);
    void playerAdded(Player *player);
    void playerRemoved(Player *player);
    void containerProcessingStarted(const GameEventContext &context);
    void containerProcessingDone();
    void openMessageDialog(const QString &userName, bool focus);
    void openDeckEditor(const DeckLoader *deck);
    void notIdle();

private slots:
    void incrementGameTime();
    void adminLockChanged(bool lock);
    void newCardAdded(AbstractCardItem *card);
    void setCardMenu(QMenu *menu);

    void actGameInfo();
    void actConcede();
    void actRemoveLocalArrows();
    void actRotateViewCW();
    void actRotateViewCCW();
    void actSay();
    void actPhaseAction();
    void actNextPhase();
    void actNextPhaseAction();
    void actNextTurn();
    void actReverseTurn();

    void addMentionTag(const QString &value);
    void linkCardToChat(const QString &cardName);
    void commandFinished(const Response &response);

    void refreshShortcuts();

    void loadLayout();
    void actCompleterChanged();
    void actResetLayout();
    void freeDocksSize();

    void hideEvent(QHideEvent *event) override;
    bool eventFilter(QObject *o, QEvent *e) override;
    void dockVisibleTriggered();
    void dockFloatingTriggered();
    void dockTopLevelChanged(bool topLevel);

protected slots:
    void closeEvent(QCloseEvent *event) override;

public:
    TabGame(TabSupervisor *_tabSupervisor,
            QList<AbstractClient *> &_clients,
            const Event_GameJoined &event,
            const QMap<int, QString> &_roomGameTypes);
    void loadReplay(GameReplay *replay);
    TabGame(TabSupervisor *_tabSupervisor, GameReplay *replay);
    ~TabGame() override;
    void retranslateUi() override;
    void updatePlayerListDockTitle();
    bool closeRequest() override;
    const QMap<int, Player *> &getPlayers() const
    {
        return players;
    }
    CardItem *getCard(int playerId, const QString &zoneName, int cardId) const;
    bool isHost() const
    {
        return hostId == localPlayerId;
    }
    bool getIsLocalGame() const
    {
        return isLocalGame;
    }
    int getGameId() const
    {
        return gameInfo.game_id();
    }
    QString getTabText() const override;
    bool isSpectator() const
    {
        return spectator;
    }
    bool isSpectatorsOmniscient() const
    {
        return gameInfo.spectators_omniscient();
    }
    Player *getActiveLocalPlayer() const;
    AbstractClient *getClientForPlayer(int playerId) const;

    void setActiveCard(CardItem *card);
    CardItem *getActiveCard() const
    {
        return activeCard;
    }

    void processGameEventContainer(const GameEventContainer &cont,
                                   AbstractClient *client,
                                   Player::EventProcessingOptions options);
    PendingCommand *prepareGameCommand(const ::google::protobuf::Message &cmd);
    PendingCommand *prepareGameCommand(const QList<const ::google::protobuf::Message *> &cmdList);
public slots:
    void sendGameCommand(PendingCommand *pend, int playerId = -1);
    void sendGameCommand(const ::google::protobuf::Message &command, int playerId = -1);
    void viewCardInfo(const CardRef &cardRef = {}) const;
    void resetChatAndPhase();
};

#endif
