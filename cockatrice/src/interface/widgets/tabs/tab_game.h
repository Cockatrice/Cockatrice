/**
 * @file tab_game.h
 * @ingroup Tabs
 * @ingroup GameWidgets
 * @ingroup Lobby
 * @brief TODO: Document this.
 */

#ifndef TAB_GAME_H
#define TAB_GAME_H

#include "../game/abstract_game.h"
#include "../game/log/message_log_widget.h"
#include "../game/player/player.h"
#include "../interface/widgets/menus/tearoff_menu.h"
#include "../interface/widgets/replay/replay_manager.h"
#include "tab.h"

#include <QCompleter>
#include <QLoggingCategory>
#include <QMap>

class ServerInfo_PlayerProperties;
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
class CardZone;
class AbstractCardItem;
class CardItem;
class QVBoxLayout;
class QHBoxLayout;
class GameReplay;
class LineEditCompleter;
class QDockWidget;
class QStackedWidget;

class TabGame : public Tab
{
    Q_OBJECT
private:
    AbstractGame *game;
    const UserListProxy *userListProxy;
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
    QAction *aGameInfo, *aConcede, *aLeaveGame, *aNextPhase, *aNextPhaseAction, *aNextTurn, *aReverseTurn,
        *aRemoveLocalArrows, *aRotateViewCW, *aRotateViewCCW, *aResetLayout, *aResetReplayLayout;
    QAction *aCardInfoDockVisible, *aCardInfoDockFloating, *aMessageLayoutDockVisible, *aMessageLayoutDockFloating,
        *aPlayerListDockVisible, *aPlayerListDockFloating, *aReplayDockVisible, *aReplayDockFloating;
    QAction *aFocusChat;
    QList<QAction *> phaseActions;
    QAction *aCardMenu;

    Player *addPlayer(Player *newPlayer);
    void addLocalPlayer(Player *newPlayer, int playerId);
    void processRemotePlayerDeckSelect(QString deckList, int playerId, QString playerName);
    void processMultipleRemotePlayerDeckSelect(QVector<QPair<int, QPair<QString, QString>>> playerIdDeckMap);
    void processLocalPlayerDeckSelect(Player *localPlayer, int playerId, ServerInfo_Player playerInfo);
    void loadDeckForLocalPlayer(Player *localPlayer, int playerId, ServerInfo_Player playerInfo);
    void processLocalPlayerReady(int playerId, ServerInfo_Player playerInfo);
    void createZoneForPlayer(Player *newPlayer, int playerId);

    void startGame(bool resuming);
    void stopGame();
    void closeGame();
    bool leaveGame();

    Player *setActivePlayer(int id);
    void setActivePhase(int phase);
    void createMenuItems();
    void createReplayMenuItems();
    void createViewMenuItems();
    void createCardInfoDock(bool bReplay = false);
    void createPlayerListDock(bool bReplay = false);
    void createMessageDock(bool bReplay = false);
    void createPlayAreaWidget(bool bReplay = false);
    void createDeckViewContainerWidget(bool bReplay = false);
    void createReplayDock(GameReplay *replay);
signals:
    void gameClosing(TabGame *tab);
    void containerProcessingStarted(const GameEventContext &context);
    void containerProcessingDone();
    void openMessageDialog(const QString &userName, bool focus);
    void openDeckEditor(const LoadedDeck &deck);
    void notIdle();

    void phaseChanged(int phase);
    void gameLeft();
    void chatMessageSent(QString chatMessage);
    void turnAdvanced();
    void arrowDeletionRequested(int arrowId);

private slots:
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

    void addMentionTag(const QString &value);
    void linkCardToChat(const QString &cardName);

    void refreshShortcuts();

    void loadLayout();
    void actCompleterChanged();
    void notifyPlayerJoin(QString playerName);
    void notifyPlayerKicked();
    void processPlayerLeave(Player *leavingPlayer);
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
    void connectToGameState();
    void connectToPlayerManager();
    void connectToGameEventHandler();
    void connectMessageLogToGameEventHandler();
    void connectPlayerListToGameEventHandler();
    TabGame(TabSupervisor *_tabSupervisor, GameReplay *replay);
    ~TabGame() override;
    void retranslateUi() override;
    void updatePlayerListDockTitle();
    bool closeRequest() override;

    [[nodiscard]] QString getTabText() const override;

    [[nodiscard]] AbstractGame *getGame() const
    {
        return game;
    }

public slots:
    void viewCardInfo(const CardRef &cardRef = {}) const;
    void resetChatAndPhase();
    void updateTimeElapsedLabel(QString newTime);
    void addPlayerToAutoCompleteList(QString playerName);
    void removePlayerFromAutoCompleteList(QString playerName);
    void removeSpectator(int spectatorId, ServerInfo_User spectator);
    void processLocalPlayerSideboardLocked(int playerId, bool sideboardLocked);
    void processLocalPlayerReadyStateChanged(int playerId, bool ready);
    void emitUserEvent();
};

#endif
