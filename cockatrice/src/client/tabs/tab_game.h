#ifndef TAB_GAME_H
#define TAB_GAME_H

#include "../../client/tearoff_menu.h"
#include "../../game/game_event_handler.h"
#include "../../game/game_meta_info.h"
#include "../../game/game_state.h"
#include "../../game/player/player.h"
#include "../../game/player/player_manager.h"
#include "../replay_manager.h"
#include "../ui/widgets/visual_deck_storage/visual_deck_storage_widget.h"
#include "pb/event_leave.pb.h"
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
class DeckLoader;
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
    GameMetaInfo *gameMetaInfo;
    GameState *gameState;
    GameEventHandler *gameEventHandler;
    PlayerManager *playerManager;
    const UserListProxy *userListProxy;
    CardItem *activeCard;
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
    void playerAdded(Player *player);
    void playerRemoved(Player *player);
    void containerProcessingStarted(const GameEventContext &context);
    void containerProcessingDone();
    void openMessageDialog(const QString &userName, bool focus);
    void openDeckEditor(const DeckLoader *deck);
    void notIdle();

    void playerConceded();
    void playerUnconceded();
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
    void loadReplay(GameReplay *replay);
    TabGame(TabSupervisor *_tabSupervisor, GameReplay *replay);
    ~TabGame() override;
    void retranslateUi() override;
    void updatePlayerListDockTitle();
    bool closeRequest() override;

    GameMetaInfo *getGameMetaInfo()
    {
        return gameMetaInfo;
    }

    GameState *getGameState() const
    {
        return gameState;
    }

    GameEventHandler *getGameEventHandler() const
    {
        return gameEventHandler;
    }

    PlayerManager *getPlayerManager() const
    {
        return playerManager;
    }

    bool isHost() const;

    CardItem *getCard(int playerId, const QString &zoneName, int cardId) const;

    QString getTabText() const override;

    AbstractClient *getClientForPlayer(int playerId) const;

    void setActiveCard(CardItem *card);
    CardItem *getActiveCard() const
    {
        return activeCard;
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
