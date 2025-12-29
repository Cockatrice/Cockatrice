#include "tab_game.h"

#include "../../../client/settings/cache_settings.h"
#include "../game/board/arrow_item.h"
#include "../game/board/card_item.h"
#include "../game/deckview/deck_view_container.h"
#include "../game/deckview/tabbed_deck_view_container.h"
#include "../game/game.h"
#include "../game/game_scene.h"
#include "../game/game_view.h"
#include "../game/log/message_log_widget.h"
#include "../game/phases_toolbar.h"
#include "../game/player/player.h"
#include "../game/player/player_list_widget.h"
#include "../game/replay.h"
#include "../interface/card_picture_loader/card_picture_loader.h"
#include "../interface/widgets/cards/card_info_frame_widget.h"
#include "../interface/widgets/dialogs/dlg_create_game.h"
#include "../interface/widgets/server/user/user_list_manager.h"
#include "../interface/widgets/utility/line_edit_completer.h"
#include "../interface/window_main.h"
#include "../main.h"
#include "tab_supervisor.h"

#include <QAction>
#include <QCompleter>
#include <QDebug>
#include <QDockWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QStackedWidget>
#include <QTimer>
#include <QWidget>
#include <libcockatrice/card/database/card_database.h>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/network/client/abstract/abstract_client.h>
#include <libcockatrice/protocol/pb/event_game_joined.pb.h>
#include <libcockatrice/protocol/pb/game_replay.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_player.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_user.pb.h>
#include <libcockatrice/utility/trice_limits.h>

TabGame::TabGame(TabSupervisor *_tabSupervisor, GameReplay *_replay)
    : Tab(_tabSupervisor), sayLabel(nullptr), sayEdit(nullptr)
{
    // THIS CTOR IS USED ON REPLAY
    game = new Replay(this, _replay);

    createCardInfoDock(true);
    createPlayerListDock(true);
    createMessageDock(true);
    createPlayAreaWidget(true);
    createDeckViewContainerWidget(true);
    createReplayDock(_replay);

    addDockWidget(Qt::RightDockWidgetArea, cardInfoDock);
    addDockWidget(Qt::RightDockWidgetArea, playerListDock);
    addDockWidget(Qt::RightDockWidgetArea, messageLayoutDock);
    addDockWidget(Qt::BottomDockWidgetArea, replayDock);

    mainWidget = new QStackedWidget(this);
    mainWidget->addWidget(deckViewContainerWidget);
    mainWidget->addWidget(gamePlayAreaWidget);
    setCentralWidget(mainWidget);

    createReplayMenuItems();
    createViewMenuItems();

    connectToGameState();
    connectToPlayerManager();
    connectToGameEventHandler();
    connectPlayerListToGameEventHandler();
    connectMessageLogToGameEventHandler();

    retranslateUi();
    connect(&SettingsCache::instance().shortcuts(), &ShortcutsSettings::shortCutChanged, this,
            &TabGame::refreshShortcuts);
    refreshShortcuts();
    messageLog->logReplayStarted(game->getGameMetaInfo()->gameId());

    QTimer::singleShot(0, this, &TabGame::loadLayout);
}

TabGame::TabGame(TabSupervisor *_tabSupervisor,
                 QList<AbstractClient *> &_clients,
                 const Event_GameJoined &event,
                 const QMap<int, QString> &_roomGameTypes)
    : Tab(_tabSupervisor), userListProxy(_tabSupervisor->getUserListManager())
{
    // THIS CTOR IS USED ON GAMES
    game = new Game(this, _clients, event, _roomGameTypes);

    createCardInfoDock();
    createPlayerListDock();
    createMessageDock();
    createPlayAreaWidget();
    createDeckViewContainerWidget();
    createReplayDock(nullptr);

    addDockWidget(Qt::RightDockWidgetArea, cardInfoDock);
    addDockWidget(Qt::RightDockWidgetArea, playerListDock);
    addDockWidget(Qt::RightDockWidgetArea, messageLayoutDock);
    replayDock->setHidden(true);

    mainWidget = new QStackedWidget(this);
    mainWidget->addWidget(deckViewContainerWidget);
    mainWidget->addWidget(gamePlayAreaWidget);
    mainWidget->setContentsMargins(0, 0, 0, 0);
    setCentralWidget(mainWidget);

    createMenuItems();
    createViewMenuItems();

    connectToGameState();
    connectToPlayerManager();
    connectToGameEventHandler();
    connectPlayerListToGameEventHandler();
    connectMessageLogToGameEventHandler();

    retranslateUi();
    connect(&SettingsCache::instance().shortcuts(), &ShortcutsSettings::shortCutChanged, this,
            &TabGame::refreshShortcuts);
    refreshShortcuts();

    // append game to rooms game list for others to see
    for (int i = game->getGameMetaInfo()->gameTypesSize() - 1; i >= 0; i--)
        gameTypes.append(game->getGameMetaInfo()->findRoomGameType(i));

    QTimer::singleShot(0, this, &TabGame::loadLayout);
}

void TabGame::connectToGameState()
{
    connect(game->getGameState(), &GameState::gameStarted, this, &TabGame::startGame);
    connect(game->getGameState(), &GameState::gameStopped, this, &TabGame::stopGame);
    connect(game->getGameState(), &GameState::activePhaseChanged, this, &TabGame::setActivePhase);
    connect(game->getGameState(), &GameState::activePlayerChanged, this, &TabGame::setActivePlayer);
}

void TabGame::connectToPlayerManager()
{
    connect(game->getPlayerManager(), &PlayerManager::playerAdded, this, &TabGame::addPlayer);
    connect(game->getPlayerManager(), &PlayerManager::playerRemoved, this, &TabGame::processPlayerLeave);
    // update menu text when player concedes so that "concede" gets updated to "unconcede"
    connect(game->getPlayerManager(), &PlayerManager::playerConceded, this, &TabGame::retranslateUi);
    connect(game->getPlayerManager(), &PlayerManager::playerUnconceded, this, &TabGame::retranslateUi);
}

void TabGame::connectToGameEventHandler()
{
    connect(this, &TabGame::gameLeft, game->getGameEventHandler(), &GameEventHandler::handleGameLeft);
    connect(game->getGameEventHandler(), &GameEventHandler::emitUserEvent, this, &TabGame::emitUserEvent);
    connect(game->getGameEventHandler(), &GameEventHandler::gameStopped, this, &TabGame::stopGame);
    connect(game->getGameEventHandler(), &GameEventHandler::gameClosed, this, &TabGame::closeGame);
    connect(game->getGameEventHandler(), &GameEventHandler::localPlayerReadyStateChanged, this,
            &TabGame::processLocalPlayerReadyStateChanged);
    connect(game->getGameEventHandler(), &GameEventHandler::localPlayerSideboardLocked, this,
            &TabGame::processLocalPlayerSideboardLocked);
    connect(game->getGameEventHandler(), &GameEventHandler::localPlayerDeckSelected, this,
            &TabGame::processLocalPlayerDeckSelect);
    connect(game->getGameEventHandler(), &GameEventHandler::remotePlayerDeckSelected, this,
            &TabGame::processRemotePlayerDeckSelect);
    connect(game->getGameEventHandler(), &GameEventHandler::remotePlayersDecksSelected, this,
            &TabGame::processMultipleRemotePlayerDeckSelect);
}

void TabGame::connectMessageLogToGameEventHandler()
{
    connect(game->getGameEventHandler(), &GameEventHandler::gameFlooded, messageLog, &MessageLogWidget::logGameFlooded);
    connect(game->getGameEventHandler(), &GameEventHandler::containerProcessingStarted, messageLog,
            &MessageLogWidget::containerProcessingStarted);
    connect(game->getGameEventHandler(), &GameEventHandler::containerProcessingDone, messageLog,
            &MessageLogWidget::containerProcessingDone);
    connect(game->getGameEventHandler(), &GameEventHandler::setContextJudgeName, messageLog,
            &MessageLogWidget::setContextJudgeName);
    connect(game->getGameEventHandler(), &GameEventHandler::logSpectatorSay, messageLog,
            &MessageLogWidget::logSpectatorSay);

    connect(game->getGameEventHandler(), &GameEventHandler::logJoinPlayer, messageLog, &MessageLogWidget::logJoin);
    connect(game->getGameEventHandler(), &GameEventHandler::logJoinSpectator, messageLog,
            &MessageLogWidget::logJoinSpectator);
    connect(game->getGameEventHandler(), &GameEventHandler::logLeave, messageLog, &MessageLogWidget::logLeave);
    connect(game->getGameEventHandler(), &GameEventHandler::logKicked, messageLog, &MessageLogWidget::logKicked);
    connect(game->getGameEventHandler(), &GameEventHandler::logConnectionStateChanged, messageLog,
            &MessageLogWidget::logConnectionStateChanged);

    connect(game->getGameEventHandler(), &GameEventHandler::logDeckSelect, messageLog,
            &MessageLogWidget::logDeckSelect);
    connect(game->getGameEventHandler(), &GameEventHandler::logSideboardLockSet, messageLog,
            &MessageLogWidget::logSetSideboardLock);
    connect(game->getGameEventHandler(), &GameEventHandler::logReadyStart, messageLog,
            &MessageLogWidget::logReadyStart);
    connect(game->getGameEventHandler(), &GameEventHandler::logNotReadyStart, messageLog,
            &MessageLogWidget::logNotReadyStart);
    connect(game->getGameEventHandler(), &GameEventHandler::logGameStart, messageLog, &MessageLogWidget::logGameStart);

    connect(game->getGameEventHandler(), &GameEventHandler::logActivePlayer, messageLog,
            &MessageLogWidget::logSetActivePlayer);
    connect(game->getGameEventHandler(), &GameEventHandler::logActivePhaseChanged, messageLog,
            &MessageLogWidget::logSetActivePhase);

    connect(game->getGameEventHandler(), &GameEventHandler::logTurnReversed, messageLog,
            &MessageLogWidget::logReverseTurn);

    connect(game->getGameEventHandler(), &GameEventHandler::logConcede, messageLog, &MessageLogWidget::logConcede);
    connect(game->getGameEventHandler(), &GameEventHandler::logUnconcede, messageLog, &MessageLogWidget::logUnconcede);

    connect(game->getGameEventHandler(), &GameEventHandler::logGameClosed, messageLog,
            &MessageLogWidget::logGameClosed);
}

void TabGame::connectPlayerListToGameEventHandler()
{
    connect(game->getGameEventHandler(), &GameEventHandler::playerJoined, playerListWidget,
            &PlayerListWidget::addPlayer);
    connect(game->getGameEventHandler(), &GameEventHandler::playerLeft, playerListWidget,
            &PlayerListWidget::removePlayer);
    connect(game->getGameEventHandler(), &GameEventHandler::spectatorJoined, playerListWidget,
            &PlayerListWidget::addPlayer);
    connect(game->getGameEventHandler(), &GameEventHandler::spectatorLeft, playerListWidget,
            &PlayerListWidget::removePlayer);
    connect(game->getGameEventHandler(), &GameEventHandler::playerPropertiesChanged, playerListWidget,
            &PlayerListWidget::updatePlayerProperties);
}

void TabGame::addMentionTag(const QString &value)
{
    sayEdit->insert(value + " ");
    sayEdit->setFocus();
}

void TabGame::linkCardToChat(const QString &cardName)
{
    sayEdit->insert("[[" + cardName + "]] ");
    sayEdit->setFocus();
}

void TabGame::resetChatAndPhase()
{
    // reset chat log
    messageLog->clearChat();

    // reset phase markers
    game->getGameState()->setCurrentPhase(-1);
}

void TabGame::emitUserEvent()
{
    bool globalEvent =
        !game->getPlayerManager()->isSpectator() || SettingsCache::instance().getSpectatorNotificationsEnabled();
    emit userEvent(globalEvent);
    updatePlayerListDockTitle();
}

TabGame::~TabGame()
{
    delete replayManager->replay;
}

void TabGame::updatePlayerListDockTitle()
{
    QString tabText = " | " + (replayManager->replay ? tr("Replay") : tr("Game")) + " #" +
                      QString::number(game->getGameMetaInfo()->gameId());
    QString userCountInfo =
        QString(" %1/%2").arg(game->getPlayerManager()->getPlayerCount()).arg(game->getGameMetaInfo()->maxPlayers());
    playerListDock->setWindowTitle(tr("Player List") + userCountInfo +
                                   (playerListDock->isWindow() ? tabText : QString()));
}

void TabGame::retranslateUi()
{
    QString tabText = " | " + (replayManager->replay ? tr("Replay") : tr("Game")) + " #" +
                      QString::number(game->getGameMetaInfo()->gameId());

    updatePlayerListDockTitle();
    cardInfoDock->setWindowTitle(tr("Card Info") + (cardInfoDock->isWindow() ? tabText : QString()));
    messageLayoutDock->setWindowTitle(tr("Messages") + (messageLayoutDock->isWindow() ? tabText : QString()));
    if (replayDock)
        replayDock->setWindowTitle(tr("Replay Timeline") + (replayDock->isWindow() ? tabText : QString()));

    if (phasesMenu) {
        for (int i = 0; i < phaseActions.size(); ++i)
            phaseActions[i]->setText(phasesToolbar->getLongPhaseName(i));
        phasesMenu->setTitle(tr("&Phases"));
    }

    gameMenu->setTitle(tr("&Game"));
    if (aNextPhase) {
        aNextPhase->setText(tr("Next &phase"));
    }
    if (aNextPhaseAction) {
        aNextPhaseAction->setText(tr("Next phase with &action"));
    }
    if (aNextTurn) {
        aNextTurn->setText(tr("Next &turn"));
    }
    if (aReverseTurn) {
        aReverseTurn->setText(tr("Reverse turn order"));
    }
    if (aRemoveLocalArrows) {
        aRemoveLocalArrows->setText(tr("&Remove all local arrows"));
    }
    if (aRotateViewCW) {
        aRotateViewCW->setText(tr("Rotate View Cl&ockwise"));
    }
    if (aRotateViewCCW) {
        aRotateViewCCW->setText(tr("Rotate View Co&unterclockwise"));
    }
    if (aGameInfo)
        aGameInfo->setText(tr("Game &information"));
    if (aConcede) {
        if (game->getPlayerManager()->isMainPlayerConceded()) {
            aConcede->setText(tr("Un&concede"));
        } else {
            aConcede->setText(tr("&Concede"));
        }
    }
    if (aLeaveGame) {
        if (replayManager->replay) {
            aLeaveGame->setText(tr("C&lose replay"));
        } else {
            aLeaveGame->setText(tr("&Leave game"));
        }
    }
    if (aFocusChat) {
        aFocusChat->setText(tr("&Focus Chat"));
    }
    if (sayLabel) {
        sayLabel->setText(tr("&Say:"));
    }

    if (aCardMenu) {
        aCardMenu->setText(tr("Selected cards"));
    }

    viewMenu->setTitle(tr("&View"));
    cardInfoDockMenu->setTitle(tr("Card Info"));
    messageLayoutDockMenu->setTitle(tr("Messages"));
    playerListDockMenu->setTitle(tr("Player List"));

    aCardInfoDockVisible->setText(tr("Visible"));
    aCardInfoDockFloating->setText(tr("Floating"));

    aMessageLayoutDockVisible->setText(tr("Visible"));
    aMessageLayoutDockFloating->setText(tr("Floating"));

    aPlayerListDockVisible->setText(tr("Visible"));
    aPlayerListDockFloating->setText(tr("Floating"));

    if (replayDock) {
        replayDockMenu->setTitle(tr("Replay Timeline"));
        aReplayDockVisible->setText(tr("Visible"));
        aReplayDockFloating->setText(tr("Floating"));
    }

    aResetLayout->setText(tr("Reset layout"));

    cardInfoFrameWidget->retranslateUi();

    QMapIterator<int, Player *> i(game->getPlayerManager()->getPlayers());

    while (i.hasNext())
        i.next().value()->getGraphicsItem()->retranslateUi();
    QMapIterator<int, TabbedDeckViewContainer *> j(deckViewContainers);
    while (j.hasNext())
        j.next().value()->playerDeckView->retranslateUi();

    scene->retranslateUi();
}

void TabGame::refreshShortcuts()
{
    ShortcutsSettings &shortcuts = SettingsCache::instance().shortcuts();
    for (int i = 0; i < phaseActions.size(); ++i) {
        QAction *temp = phaseActions.at(i);
        switch (i) {
            case 0:
                temp->setShortcuts(shortcuts.getShortcut("Player/phase0"));
                break;
            case 1:
                temp->setShortcuts(shortcuts.getShortcut("Player/phase1"));
                break;
            case 2:
                temp->setShortcuts(shortcuts.getShortcut("Player/phase2"));
                break;
            case 3:
                temp->setShortcuts(shortcuts.getShortcut("Player/phase3"));
                break;
            case 4:
                temp->setShortcuts(shortcuts.getShortcut("Player/phase4"));
                break;
            case 5:
                temp->setShortcuts(shortcuts.getShortcut("Player/phase5"));
                break;
            case 6:
                temp->setShortcuts(shortcuts.getShortcut("Player/phase6"));
                break;
            case 7:
                temp->setShortcuts(shortcuts.getShortcut("Player/phase7"));
                break;
            case 8:
                temp->setShortcuts(shortcuts.getShortcut("Player/phase8"));
                break;
            case 9:
                temp->setShortcuts(shortcuts.getShortcut("Player/phase9"));
                break;
            case 10:
                temp->setShortcuts(shortcuts.getShortcut("Player/phase10"));
                break;
            default:;
        }
    }

    if (aNextPhase) {
        aNextPhase->setShortcuts(shortcuts.getShortcut("Player/aNextPhase"));
    }
    if (aNextPhaseAction) {
        aNextPhaseAction->setShortcuts(shortcuts.getShortcut("Player/aNextPhaseAction"));
    }
    if (aNextTurn) {
        aNextTurn->setShortcuts(shortcuts.getShortcut("Player/aNextTurn"));
    }
    if (aReverseTurn) {
        aReverseTurn->setShortcuts(shortcuts.getShortcut("Player/aReverseTurn"));
    }
    if (aRemoveLocalArrows) {
        aRemoveLocalArrows->setShortcuts(shortcuts.getShortcut("Player/aRemoveLocalArrows"));
    }
    if (aRotateViewCW) {
        aRotateViewCW->setShortcuts(shortcuts.getShortcut("Player/aRotateViewCW"));
    }
    if (aRotateViewCCW) {
        aRotateViewCCW->setShortcuts(shortcuts.getShortcut("Player/aRotateViewCCW"));
    }
    if (aConcede) {
        aConcede->setShortcuts(shortcuts.getShortcut("Player/aConcede"));
    }
    if (aLeaveGame) {
        aLeaveGame->setShortcuts(shortcuts.getShortcut("Player/aLeaveGame"));
    }
    if (aResetLayout) {
        aResetLayout->setShortcuts(shortcuts.getShortcut("Player/aResetLayout"));
    }
    if (aFocusChat) {
        aFocusChat->setShortcuts(shortcuts.getShortcut("Player/aFocusChat"));
    }
}

bool TabGame::closeRequest()
{
    if (!leaveGame()) {
        return false;
    }

    return close();
}

void TabGame::closeEvent(QCloseEvent *event)
{
    emit gameClosing(this);
    event->accept();
}

void TabGame::updateTimeElapsedLabel(const QString newTime)
{
    timeElapsedLabel->setText(newTime);
}

void TabGame::adminLockChanged(bool lock)
{
    bool v = !(game->getPlayerManager()->isSpectator() && !game->getGameMetaInfo()->spectatorsCanChat() && lock);
    sayLabel->setVisible(v);
    sayEdit->setVisible(v);
}

void TabGame::actGameInfo()
{
    DlgCreateGame dlg(game->getGameMetaInfo()->proto(), game->getGameMetaInfo()->getRoomGameTypes(), this);
    dlg.exec();
}

void TabGame::actConcede()
{
    Player *player = game->getPlayerManager()->getActiveLocalPlayer(game->getGameState()->getActivePlayer());
    if (player == nullptr)
        return;
    if (!player->getConceded()) {
        if (QMessageBox::question(this, tr("Concede"), tr("Are you sure you want to concede this game?"),
                                  QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
            return;
        emit game->getPlayerManager()->activeLocalPlayerConceded();
        player->setConceded(true);
    } else {
        if (QMessageBox::question(this, tr("Unconcede"),
                                  tr("You have already conceded.  Do you want to return to this game?"),
                                  QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
            return;
        emit game->getPlayerManager()->activeLocalPlayerUnconceded();
        player->setConceded(false);
    }
}

/**
 * Confirms the leave game and sends the leave game command, if applicable.
 *
 * @return True if the leave game is confirmed
 */
bool TabGame::leaveGame()
{
    if (!game->getGameState()->isGameClosed()) {
        if (!game->getPlayerManager()->isSpectator()) {
            tabSupervisor->setCurrentWidget(this);
            if (QMessageBox::question(this, tr("Leave game"), tr("Are you sure you want to leave this game?"),
                                      QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
                return false;
        }

        if (!replayManager->replay)
            emit gameLeft();
    }
    return true;
}

void TabGame::actSay()
{
    if (completer->popup()->isVisible())
        return;

    if (sayEdit->text().startsWith("/card ")) {
        cardInfoFrameWidget->setCard(sayEdit->text().mid(6));
        sayEdit->clear();
        return;
    }

    if (!sayEdit->text().isEmpty()) {
        emit chatMessageSent(sayEdit->text());
        sayEdit->clear();
    }
}

void TabGame::addPlayerToAutoCompleteList(QString playerName)
{
    if (sayEdit && !autocompleteUserList.contains(playerName)) {
        autocompleteUserList << playerName;
        sayEdit->setCompletionList(autocompleteUserList);
    }
}

void TabGame::removePlayerFromAutoCompleteList(QString playerName)
{
    if (sayEdit && autocompleteUserList.removeOne(playerName)) {
        sayEdit->setCompletionList(autocompleteUserList);
    }
}

void TabGame::removeSpectator(int spectatorId, ServerInfo_User spectator)
{
    Q_UNUSED(spectator);
    QString playerName = "@" + game->getPlayerManager()->getSpectatorName(spectatorId);
    removePlayerFromAutoCompleteList(playerName);
}

void TabGame::actPhaseAction()
{
    int phase = phaseActions.indexOf(static_cast<QAction *>(sender()));
    emit phaseChanged(phase);
}

void TabGame::actNextPhase()
{
    int phase = game->getGameState()->getCurrentPhase();
    if (++phase >= phasesToolbar->phaseCount())
        phase = 0;

    emit phaseChanged(phase);
}

void TabGame::actNextPhaseAction()
{
    int phase = game->getGameState()->getCurrentPhase() + 1;
    if (phase >= phasesToolbar->phaseCount()) {
        phase = 0;
    }

    if (phase == 0) {
        emit turnAdvanced();
    } else {
        emit phaseChanged(phase);
    }

    phasesToolbar->triggerPhaseAction(phase);
}

void TabGame::actRemoveLocalArrows()
{
    QMapIterator<int, Player *> playerIterator(game->getPlayerManager()->getPlayers());
    while (playerIterator.hasNext()) {
        Player *player = playerIterator.next().value();
        if (!player->getPlayerInfo()->getLocal())
            continue;
        QMapIterator<int, ArrowItem *> arrowIterator(player->getArrows());
        while (arrowIterator.hasNext()) {
            ArrowItem *a = arrowIterator.next().value();
            emit arrowDeletionRequested(a->getId());
        }
    }
}

void TabGame::actRotateViewCW()
{
    scene->adjustPlayerRotation(-1);
}

void TabGame::actRotateViewCCW()
{
    scene->adjustPlayerRotation(1);
}

void TabGame::actCompleterChanged()
{
    SettingsCache::instance().getChatMentionCompleter() ? completer->setCompletionRole(2)
                                                        : completer->setCompletionRole(1);
}

void TabGame::notifyPlayerJoin(QString playerName)
{
    if (trayIcon) {
        QString gameId(QString::number(game->getGameMetaInfo()->gameId()));
        trayIcon->showMessage(tr("A player has joined game #%1").arg(gameId),
                              tr("%1 has joined the game").arg(playerName));
    }
}

void TabGame::notifyPlayerKicked()
{
    tabSupervisor->setCurrentIndex(tabSupervisor->indexOf(this));
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(getTabText());
    msgBox.setText(tr("You have been kicked out of the game."));
    msgBox.setIcon(QMessageBox::Information);
    msgBox.exec();
}

Player *TabGame::addPlayer(Player *newPlayer)
{
    QString newPlayerName = "@" + newPlayer->getPlayerInfo()->getName();
    addPlayerToAutoCompleteList(newPlayerName);

    scene->addPlayer(newPlayer);

    connect(newPlayer, &Player::newCardAdded, this, &TabGame::newCardAdded);
    connect(newPlayer->getPlayerMenu(), &PlayerMenu::cardMenuUpdated, this, &TabGame::setCardMenu);

    messageLog->connectToPlayerEventHandler(newPlayer->getPlayerEventHandler());

    if (game->getGameState()->getIsLocalGame() ||
        (game->getPlayerManager()->isLocalPlayer(newPlayer->getPlayerInfo()->getId()) &&
         !game->getPlayerManager()->isSpectator())) {
        if (game->getGameState()->getIsLocalGame()) {
            newPlayer->getPlayerInfo()->setLocal(true);
        }
        addLocalPlayer(newPlayer, newPlayer->getPlayerInfo()->getId());
    }

    gameMenu->insertMenu(playersSeparator, newPlayer->getPlayerMenu()->getPlayerMenu());

    createZoneForPlayer(newPlayer, newPlayer->getPlayerInfo()->getId());

    return newPlayer;
}

void TabGame::addLocalPlayer(Player *newPlayer, int playerId)
{
    if (game->getGameState()->getClients().size() == 1) {
        newPlayer->getPlayerMenu()->setShortcutsActive();
    }

    auto *deckView = new TabbedDeckViewContainer(playerId, this);
    connect(deckView->playerDeckView, &DeckViewContainer::newCardAdded, this, &TabGame::newCardAdded);
    deckViewContainers.insert(playerId, deckView);
    deckViewContainerLayout->addWidget(deckView);

    // auto load deck for player if that debug setting is enabled
    QString deckPath = SettingsCache::instance().debug().getDeckPathForPlayer(newPlayer->getPlayerInfo()->getName());
    if (!deckPath.isEmpty()) {
        QTimer::singleShot(0, this, [deckView, deckPath] {
            deckView->playerDeckView->loadDeckFromFile(deckPath);
            deckView->playerDeckView->readyAndUpdate();
        });
    }
}

void TabGame::processPlayerLeave(Player *leavingPlayer)
{
    QString playerName = "@" + leavingPlayer->getPlayerInfo()->getName();
    removePlayerFromAutoCompleteList(playerName);

    scene->removePlayer(leavingPlayer);

    // When we inserted the playerMenu into the gameMenu earlier, Qt wrapped the playerMenu into a QAction*, which lives
    // independently and does not get cleaned up when the source menu gets destroyed. We have to manually clean here.
    if (leavingPlayer->getPlayerMenu()) {
        QMenu *menu = leavingPlayer->getPlayerMenu()->getPlayerMenu();
        if (menu) {
            // Find and remove the QAction pointing to this menu
            QList<QAction *> actions = gameMenu->actions();
            for (QAction *act : actions) {
                if (act->menu() == menu) {
                    gameMenu->removeAction(act);
                    delete act; // deletes the QAction wrapper around the submenu
                    break;
                }
            }
        }
    }
}

void TabGame::processRemotePlayerDeckSelect(QString deckList, int playerId, QString playerName)
{
    DeckList loader;
    loader.loadFromString_Native(deckList);
    QMapIterator<int, TabbedDeckViewContainer *> i(deckViewContainers);
    while (i.hasNext()) {
        i.next();
        i.value()->addOpponentDeckView(loader, playerId, playerName);
    }
}

void TabGame::processMultipleRemotePlayerDeckSelect(QVector<QPair<int, QPair<QString, QString>>> playerIdDeckMap)
{
    for (const auto &entry : playerIdDeckMap) {
        int playerId = entry.first;
        QString playerName = entry.second.first;
        QString deckList = entry.second.second;

        processRemotePlayerDeckSelect(deckList, playerId, playerName);
    }
}

void TabGame::processLocalPlayerDeckSelect(Player *localPlayer, int playerId, ServerInfo_Player playerInfo)
{
    loadDeckForLocalPlayer(localPlayer, playerId, playerInfo);
    processLocalPlayerReady(playerId, playerInfo);
}

void TabGame::loadDeckForLocalPlayer(Player *localPlayer, int playerId, ServerInfo_Player playerInfo)
{
    TabbedDeckViewContainer *deckViewContainer = deckViewContainers.value(playerId);
    if (playerInfo.has_deck_list()) {
        DeckList deckList = DeckList(QString::fromStdString(playerInfo.deck_list()));
        CardPictureLoader::cacheCardPixmaps(CardDatabaseManager::query()->getCards(deckList.getCardRefList()));
        deckViewContainer->playerDeckView->setDeck(deckList);
        localPlayer->setDeck(deckList);
    }
}

void TabGame::processLocalPlayerReady(int playerId, ServerInfo_Player playerInfo)
{
    processLocalPlayerReadyStateChanged(playerId, playerInfo.properties().ready_start());
    processLocalPlayerSideboardLocked(playerId, playerInfo.properties().sideboard_locked());
}

void TabGame::processLocalPlayerSideboardLocked(int playerId, bool sideboardLocked)
{
    deckViewContainers.value(playerId)->playerDeckView->setSideboardLocked(sideboardLocked);
}

void TabGame::processLocalPlayerReadyStateChanged(int playerId, bool ready)
{
    deckViewContainers.value(playerId)->playerDeckView->setReadyStart(ready);
}

void TabGame::createZoneForPlayer(Player *newPlayer, int playerId)
{
    if (!game->getPlayerManager()->getSpectators().contains(playerId)) {

        // Loop for each player, the idea is to have one assigned zone for each non-spectator player
        for (int i = 1; i <= game->getPlayerManager()->getPlayerCount(); ++i) {
            bool aPlayerHasThisZone = false;
            for (auto &player : game->getPlayerManager()->getPlayers()) {
                if (player->getZoneId() == i) {
                    aPlayerHasThisZone = true;
                    break;
                }
            }
            if (!aPlayerHasThisZone) {
                newPlayer->setZoneId(i);
                break;
            }
        }
    }
}

void TabGame::startGame(bool _resuming)
{
    game->getGameState()->setCurrentPhase(-1);

    QMapIterator<int, TabbedDeckViewContainer *> i(deckViewContainers);
    while (i.hasNext()) {
        i.next();
        i.value()->playerDeckView->setReadyStart(false);
        i.value()->playerDeckView->setVisualDeckStorageExists(false);
        i.value()->hide();
    }

    mainWidget->setCurrentWidget(gamePlayAreaWidget);

    if (!_resuming) {
        QMapIterator<int, Player *> playerIterator(game->getPlayerManager()->getPlayers());
        while (playerIterator.hasNext())
            playerIterator.next().value()->setGameStarted();
    }

    playerListWidget->setGameStarted(true, game->getGameState()->isResuming());
    game->getGameMetaInfo()->setStarted(true);
    static_cast<GameScene *>(gameView->scene())->rearrange();

    if (aConcede != nullptr) {
        aConcede->setText(tr("&Concede"));
        aConcede->setEnabled(true);
    }
}

void TabGame::stopGame()
{
    QMapIterator<int, TabbedDeckViewContainer *> i(deckViewContainers);
    while (i.hasNext()) {
        i.next();
        i.value()->show();
    }

    mainWidget->setCurrentWidget(deckViewContainerWidget);

    playerListWidget->setActivePlayer(-1);
    playerListWidget->setGameStarted(false, false);

    scene->clearViews();

    if (aConcede != nullptr) {
        aConcede->setText(tr("&Concede"));
        aConcede->setEnabled(false);
    }
}

void TabGame::closeGame()
{
    gameMenu->clear();
    gameMenu->addAction(aLeaveGame);
}

Player *TabGame::setActivePlayer(int id)
{
    Player *player = game->getPlayerManager()->getPlayer(id);
    if (!player)
        return nullptr;

    playerListWidget->setActivePlayer(id);
    QMapIterator<int, Player *> i(game->getPlayerManager()->getPlayers());
    while (i.hasNext()) {
        i.next();
        if (i.value() == player) {
            i.value()->setActive(true);
            if (game->getGameState()->getClients().size() > 1) {
                i.value()->getPlayerMenu()->setShortcutsActive();
            }
        } else {
            i.value()->setActive(false);
            if (game->getGameState()->getClients().size() > 1) {
                i.value()->getPlayerMenu()->setShortcutsInactive();
            }
        }
    }
    game->getGameState()->setCurrentPhase(-1);
    emitUserEvent();
    return player;
}

void TabGame::setActivePhase(int phase)
{
    phasesToolbar->setActivePhase(phase);
}

void TabGame::newCardAdded(AbstractCardItem *card)
{
    connect(card, &AbstractCardItem::hovered, cardInfoFrameWidget,
            qOverload<AbstractCardItem *>(&CardInfoFrameWidget::setCard));
    connect(card, &AbstractCardItem::showCardInfoPopup, this, &TabGame::showCardInfoPopup);
    connect(card, SIGNAL(deleteCardInfoPopup(QString)), this, SLOT(deleteCardInfoPopup(QString)));
    connect(card, &AbstractCardItem::cardShiftClicked, this, &TabGame::linkCardToChat);
}

QString TabGame::getTabText() const
{
    QString gameTypeInfo;
    if (!gameTypes.empty()) {
        gameTypeInfo = gameTypes.at(0);
        if (gameTypes.size() > 1)
            gameTypeInfo.append("...");
    }

    QString gameDesc(game->getGameMetaInfo()->description());
    QString gameId(QString::number(game->getGameMetaInfo()->gameId()));

    QString tabText;
    if (replayManager->replay)
        tabText.append(tr("Replay") + " ");
    if (!gameTypeInfo.isEmpty())
        tabText.append(gameTypeInfo + " ");
    if (!gameDesc.isEmpty()) {
        if (gameDesc.length() >= 15)
            tabText.append("| " + gameDesc.left(15) + "... ");
        else
            tabText.append("| " + gameDesc + " ");
    }
    if (!tabText.isEmpty())
        tabText.append("| ");
    tabText.append("#" + gameId);

    return tabText;
}

/**
 * @param menu The menu to set. Pass in nullptr to set the menu to empty.
 */
void TabGame::setCardMenu(QMenu *menu)
{
    if (!aCardMenu) {
        return;
    }

    if (menu) {
        aCardMenu->setMenu(menu);
    } else {
        aCardMenu->setMenu(new QMenu);
    }
}

void TabGame::createMenuItems()
{
    aNextPhase = new QAction(this);
    connect(aNextPhase, &QAction::triggered, this, &TabGame::actNextPhase);
    connect(this, &TabGame::phaseChanged, game->getGameEventHandler(), &GameEventHandler::handleActivePhaseChanged);
    aNextPhaseAction = new QAction(this);
    connect(aNextPhaseAction, &QAction::triggered, this, &TabGame::actNextPhaseAction);
    connect(this, &TabGame::turnAdvanced, game->getGameEventHandler(), &GameEventHandler::handleNextTurn);
    aNextTurn = new QAction(this);
    connect(aNextTurn, &QAction::triggered, game->getGameEventHandler(), &GameEventHandler::handleNextTurn);
    aReverseTurn = new QAction(this);
    connect(aReverseTurn, &QAction::triggered, game->getGameEventHandler(), &GameEventHandler::handleReverseTurn);
    aRemoveLocalArrows = new QAction(this);
    connect(aRemoveLocalArrows, &QAction::triggered, this, &TabGame::actRemoveLocalArrows);
    connect(this, &TabGame::arrowDeletionRequested, game->getGameEventHandler(),
            &GameEventHandler::handleArrowDeletion);
    aRotateViewCW = new QAction(this);
    connect(aRotateViewCW, &QAction::triggered, this, &TabGame::actRotateViewCW);
    aRotateViewCCW = new QAction(this);
    connect(aRotateViewCCW, &QAction::triggered, this, &TabGame::actRotateViewCCW);
    aGameInfo = new QAction(this);
    connect(aGameInfo, &QAction::triggered, this, &TabGame::actGameInfo);
    aConcede = new QAction(this);
    connect(aConcede, &QAction::triggered, this, &TabGame::actConcede);
    if (!game->getGameMetaInfo()->started()) {
        aConcede->setEnabled(false);
    }
    connect(game->getPlayerManager(), &PlayerManager::activeLocalPlayerConceded, game->getGameEventHandler(),
            &GameEventHandler::handleActiveLocalPlayerConceded);
    connect(game->getPlayerManager(), &PlayerManager::activeLocalPlayerUnconceded, game->getGameEventHandler(),
            &GameEventHandler::handleActiveLocalPlayerUnconceded);
    aLeaveGame = new QAction(this);
    connect(aLeaveGame, &QAction::triggered, this, &TabGame::closeRequest);
    aFocusChat = new QAction(this);
    connect(aFocusChat, &QAction::triggered, sayEdit, qOverload<>(&LineEditCompleter::setFocus));

    phasesMenu = new TearOffMenu(this);

    for (int i = 0; i < phasesToolbar->phaseCount(); ++i) {
        auto *temp = new QAction(QString(), this);
        connect(temp, &QAction::triggered, this, &TabGame::actPhaseAction);
        phasesMenu->addAction(temp);
        phaseActions.append(temp);
    }

    phasesMenu->addSeparator();
    phasesMenu->addAction(aNextPhase);
    phasesMenu->addAction(aNextPhaseAction);

    gameMenu = new QMenu(this);
    playersSeparator = gameMenu->addSeparator();
    gameMenu->addMenu(phasesMenu);
    gameMenu->addAction(aNextTurn);
    gameMenu->addAction(aReverseTurn);
    gameMenu->addSeparator();
    gameMenu->addAction(aRemoveLocalArrows);
    gameMenu->addAction(aRotateViewCW);
    gameMenu->addAction(aRotateViewCCW);
    gameMenu->addSeparator();
    gameMenu->addAction(aGameInfo);
    gameMenu->addAction(aConcede);
    gameMenu->addAction(aFocusChat);
    gameMenu->addAction(aLeaveGame);

    gameMenu->addSeparator();

    aCardMenu = gameMenu->addMenu(new QMenu(this));

    addTabMenu(gameMenu);
}

void TabGame::createReplayMenuItems()
{
    aNextPhase = nullptr;
    aNextPhaseAction = nullptr;
    aNextTurn = nullptr;
    aReverseTurn = nullptr;
    aRemoveLocalArrows = nullptr;
    aRotateViewCW = nullptr;
    aRotateViewCCW = nullptr;
    aResetLayout = nullptr;
    aGameInfo = nullptr;
    aConcede = nullptr;
    aFocusChat = nullptr;
    aLeaveGame = new QAction(this);
    connect(aLeaveGame, &QAction::triggered, this, &TabGame::closeRequest);

    phasesMenu = nullptr;
    gameMenu = new QMenu(this);
    gameMenu->addAction(aLeaveGame);

    aCardMenu = nullptr;

    addTabMenu(gameMenu);
}

void TabGame::createViewMenuItems()
{
    viewMenu = new QMenu(this);

    cardInfoDockMenu = viewMenu->addMenu(QString());
    messageLayoutDockMenu = viewMenu->addMenu(QString());
    playerListDockMenu = viewMenu->addMenu(QString());

    aCardInfoDockVisible = cardInfoDockMenu->addAction(QString());
    aCardInfoDockVisible->setCheckable(true);
    connect(aCardInfoDockVisible, &QAction::triggered, this, &TabGame::dockVisibleTriggered);
    aCardInfoDockFloating = cardInfoDockMenu->addAction(QString());
    aCardInfoDockFloating->setCheckable(true);
    connect(aCardInfoDockFloating, &QAction::triggered, this, &TabGame::dockFloatingTriggered);

    aMessageLayoutDockVisible = messageLayoutDockMenu->addAction(QString());
    aMessageLayoutDockVisible->setCheckable(true);
    connect(aMessageLayoutDockVisible, &QAction::triggered, this, &TabGame::dockVisibleTriggered);
    aMessageLayoutDockFloating = messageLayoutDockMenu->addAction(QString());
    aMessageLayoutDockFloating->setCheckable(true);
    connect(aMessageLayoutDockFloating, &QAction::triggered, this, &TabGame::dockFloatingTriggered);

    aPlayerListDockVisible = playerListDockMenu->addAction(QString());
    aPlayerListDockVisible->setCheckable(true);
    connect(aPlayerListDockVisible, &QAction::triggered, this, &TabGame::dockVisibleTriggered);
    aPlayerListDockFloating = playerListDockMenu->addAction(QString());
    aPlayerListDockFloating->setCheckable(true);
    connect(aPlayerListDockFloating, &QAction::triggered, this, &TabGame::dockFloatingTriggered);

    if (replayDock) {
        replayDockMenu = viewMenu->addMenu(QString());

        aReplayDockVisible = replayDockMenu->addAction(QString());
        aReplayDockVisible->setCheckable(true);
        connect(aReplayDockVisible, &QAction::triggered, this, &TabGame::dockVisibleTriggered);
        aReplayDockFloating = replayDockMenu->addAction(QString());
        aReplayDockFloating->setCheckable(true);
        connect(aReplayDockFloating, &QAction::triggered, this, &TabGame::dockFloatingTriggered);
    }

    viewMenu->addSeparator();

    aResetLayout = viewMenu->addAction(QString());
    connect(aResetLayout, &QAction::triggered, this, &TabGame::actResetLayout);
    viewMenu->addAction(aResetLayout);

    addTabMenu(viewMenu);
}

void TabGame::loadLayout()
{
    LayoutsSettings &layouts = SettingsCache::instance().layouts();
    if (replayManager->replay) {
        restoreGeometry(layouts.getReplayPlayAreaGeometry());
        restoreState(layouts.getReplayPlayAreaLayoutState());

        cardInfoDock->setMinimumSize(layouts.getReplayCardInfoSize());
        cardInfoDock->setMaximumSize(layouts.getReplayCardInfoSize());
        messageLayoutDock->setMinimumSize(layouts.getReplayMessageLayoutSize());
        messageLayoutDock->setMaximumSize(layouts.getReplayMessageLayoutSize());
        playerListDock->setMinimumSize(layouts.getReplayPlayerListSize());
        playerListDock->setMaximumSize(layouts.getReplayPlayerListSize());
        replayDock->setMinimumSize(layouts.getReplayReplaySize());
        replayDock->setMaximumSize(layouts.getReplayReplaySize());
    } else {
        restoreGeometry(layouts.getGamePlayAreaGeometry());
        restoreState(layouts.getGamePlayAreaLayoutState());

        cardInfoDock->setMinimumSize(layouts.getGameCardInfoSize());
        cardInfoDock->setMaximumSize(layouts.getGameCardInfoSize());
        messageLayoutDock->setMinimumSize(layouts.getGameMessageLayoutSize());
        messageLayoutDock->setMaximumSize(layouts.getGameMessageLayoutSize());
        playerListDock->setMinimumSize(layouts.getGamePlayerListSize());
        playerListDock->setMaximumSize(layouts.getGamePlayerListSize());
    }

    aCardInfoDockVisible->setChecked(cardInfoDock->isVisible());
    aMessageLayoutDockVisible->setChecked(messageLayoutDock->isVisible());
    aPlayerListDockVisible->setChecked(playerListDock->isVisible());

    aCardInfoDockFloating->setEnabled(aCardInfoDockVisible->isChecked());
    aMessageLayoutDockFloating->setEnabled(aMessageLayoutDockVisible->isChecked());
    aPlayerListDockFloating->setEnabled(aPlayerListDockVisible->isChecked());

    aCardInfoDockFloating->setChecked(cardInfoDock->isFloating());
    aMessageLayoutDockFloating->setChecked(messageLayoutDock->isFloating());
    aPlayerListDockFloating->setChecked(playerListDock->isFloating());

    if (replayManager->replay) {
        aReplayDockVisible->setChecked(replayDock->isVisible());
        aReplayDockFloating->setEnabled(aReplayDockVisible->isChecked());
        aReplayDockFloating->setChecked(replayDock->isFloating());
    }

    QTimer::singleShot(100, this, &TabGame::freeDocksSize);
}

void TabGame::freeDocksSize()
{
    cardInfoDock->setMinimumSize(100, 100);
    cardInfoDock->setMaximumSize(5000, 5000);

    messageLayoutDock->setMinimumSize(100, 100);
    messageLayoutDock->setMaximumSize(5000, 5000);

    playerListDock->setMinimumSize(100, 100);
    playerListDock->setMaximumSize(5000, 5000);

    if (replayDock) {
        replayDock->setMinimumSize(100, 100);
        replayDock->setMaximumSize(5000, 5000);
    }
}

void TabGame::actResetLayout()
{
    cardInfoDock->setVisible(true);
    playerListDock->setVisible(true);
    messageLayoutDock->setVisible(true);

    cardInfoDock->setFloating(false);
    playerListDock->setFloating(false);
    messageLayoutDock->setFloating(false);

    aCardInfoDockVisible->setChecked(true);
    aPlayerListDockVisible->setChecked(true);
    aMessageLayoutDockVisible->setChecked(true);

    aCardInfoDockFloating->setChecked(false);
    aPlayerListDockFloating->setChecked(false);
    aMessageLayoutDockFloating->setChecked(false);

    addDockWidget(Qt::RightDockWidgetArea, cardInfoDock);
    addDockWidget(Qt::RightDockWidgetArea, playerListDock);
    addDockWidget(Qt::RightDockWidgetArea, messageLayoutDock);

    if (replayDock) {
        replayDock->setVisible(true);
        replayDock->setFloating(false);
        addDockWidget(Qt::BottomDockWidgetArea, replayDock);
        aReplayDockVisible->setChecked(true);
        aReplayDockFloating->setChecked(false);

        cardInfoDock->setMinimumSize(250, 360);
        cardInfoDock->setMaximumSize(250, 360);
        messageLayoutDock->setMinimumSize(250, 200);
        messageLayoutDock->setMaximumSize(250, 200);
        playerListDock->setMinimumSize(250, 50);
        playerListDock->setMaximumSize(250, 50);
        replayDock->setMinimumSize(900, 100);
        replayDock->setMaximumSize(900, 100);
    } else {
        cardInfoDock->setMinimumSize(250, 360);
        cardInfoDock->setMaximumSize(250, 360);
        messageLayoutDock->setMinimumSize(250, 250);
        messageLayoutDock->setMaximumSize(250, 250);
        playerListDock->setMinimumSize(250, 50);
        playerListDock->setMaximumSize(250, 50);
    }

    QTimer::singleShot(100, this, &TabGame::freeDocksSize);
}

void TabGame::createPlayAreaWidget(bool bReplay)
{
    phasesToolbar = new PhasesToolbar;
    if (!bReplay)
        connect(phasesToolbar, &PhasesToolbar::sendGameCommand, game->getGameEventHandler(),
                qOverload<const ::google::protobuf::Message &, int>(&GameEventHandler::sendGameCommand));
    scene = new GameScene(phasesToolbar, this);
    connect(game->getPlayerManager(), &PlayerManager::playerConceded, scene, &GameScene::rearrange);
    connect(game->getPlayerManager(), &PlayerManager::playerCountChanged, scene, &GameScene::rearrange);
    gameView = new GameView(scene);

    auto gamePlayAreaVBox = new QVBoxLayout;
    gamePlayAreaVBox->setContentsMargins(0, 0, 0, 0);
    gamePlayAreaVBox->addWidget(gameView);

    gamePlayAreaWidget = new QWidget;
    gamePlayAreaWidget->setObjectName("gamePlayAreaWidget");
    gamePlayAreaWidget->setLayout(gamePlayAreaVBox);
}

void TabGame::createReplayDock(GameReplay *replay)
{
    replayManager = new ReplayManager(this, replay);

    replayDock = new QDockWidget(this);
    replayDock->setObjectName("replayDock");
    replayDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable |
                            QDockWidget::DockWidgetMovable);
    replayDock->setWidget(replayManager);
    replayDock->setFloating(false);

    replayDock->installEventFilter(this);
    connect(replayDock, &QDockWidget::topLevelChanged, this, &TabGame::dockTopLevelChanged);
}

void TabGame::createDeckViewContainerWidget(bool bReplay)
{
    Q_UNUSED(bReplay);

    deckViewContainerWidget = new QWidget();
    deckViewContainerWidget->setObjectName("deckViewContainerWidget");
    deckViewContainerLayout = new QVBoxLayout;
    deckViewContainerLayout->setContentsMargins(0, 0, 0, 0);
    deckViewContainerWidget->setLayout(deckViewContainerLayout);
}

void TabGame::viewCardInfo(const CardRef &cardRef) const
{
    cardInfoFrameWidget->setCard(cardRef);
}

void TabGame::createCardInfoDock(bool bReplay)
{
    Q_UNUSED(bReplay);

    cardInfoFrameWidget = new CardInfoFrameWidget();
    auto cardHInfoLayout = new QHBoxLayout;
    auto cardVInfoLayout = new QVBoxLayout;
    cardVInfoLayout->setContentsMargins(0, 0, 0, 0);
    cardVInfoLayout->addWidget(cardInfoFrameWidget);
    cardVInfoLayout->addLayout(cardHInfoLayout);

    auto cardBoxLayoutWidget = new QWidget;
    cardBoxLayoutWidget->setLayout(cardVInfoLayout);

    cardInfoDock = new QDockWidget(this);
    cardInfoDock->setObjectName("cardInfoDock");
    cardInfoDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable |
                              QDockWidget::DockWidgetMovable);
    cardInfoDock->setWidget(cardBoxLayoutWidget);
    cardInfoDock->setFloating(false);

    cardInfoDock->installEventFilter(this);
    connect(cardInfoDock, &QDockWidget::topLevelChanged, this, &TabGame::dockTopLevelChanged);
}

void TabGame::createPlayerListDock(bool bReplay)
{
    if (bReplay) {
        playerListWidget = new PlayerListWidget(nullptr, nullptr, game);
    } else {
        playerListWidget = new PlayerListWidget(tabSupervisor, game->getGameState()->getClients().first(), game);
        connect(playerListWidget, SIGNAL(openMessageDialog(QString, bool)), this,
                SIGNAL(openMessageDialog(QString, bool)));
    }
    playerListWidget->setFocusPolicy(Qt::NoFocus);

    playerListDock = new QDockWidget(this);
    playerListDock->setObjectName("playerListDock");
    playerListDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable |
                                QDockWidget::DockWidgetMovable);
    playerListDock->setWidget(playerListWidget);
    playerListDock->setFloating(false);

    playerListDock->installEventFilter(this);
    connect(playerListDock, &QDockWidget::topLevelChanged, this, &TabGame::dockTopLevelChanged);
}

void TabGame::createMessageDock(bool bReplay)
{
    auto messageLogLayout = new QVBoxLayout;
    messageLogLayout->setContentsMargins(0, 0, 0, 0);

    // clock
    if (!bReplay) {
        timeElapsedLabel = new QLabel;
        timeElapsedLabel->setAlignment(Qt::AlignCenter);
        connect(game->getGameState(), &GameState::updateTimeElapsedLabel, this, &TabGame::updateTimeElapsedLabel);

        messageLogLayout->addWidget(timeElapsedLabel);
    }

    // message log
    messageLog = new MessageLogWidget(tabSupervisor, game);
    connect(messageLog, &MessageLogWidget::cardNameHovered, cardInfoFrameWidget,
            qOverload<const QString &>(&CardInfoFrameWidget::setCard));
    connect(messageLog, &MessageLogWidget::showCardInfoPopup, this, &TabGame::showCardInfoPopup);
    connect(messageLog, &MessageLogWidget::deleteCardInfoPopup, this, &TabGame::deleteCardInfoPopup);

    if (!bReplay) {
        connect(messageLog, &MessageLogWidget::openMessageDialog, this, &TabGame::openMessageDialog);
        connect(messageLog, &MessageLogWidget::addMentionTag, this, &TabGame::addMentionTag);
        connect(&SettingsCache::instance(), &SettingsCache::chatMentionCompleterChanged, this,
                &TabGame::actCompleterChanged);
    }

    messageLogLayout->addWidget(messageLog);

    // chat entry
    if (!bReplay) {
        sayLabel = new QLabel;
        sayEdit = new LineEditCompleter;
        sayEdit->setMaxLength(MAX_TEXT_LENGTH);
        sayLabel->setBuddy(sayEdit);
        connect(this, &TabGame::chatMessageSent, game->getGameEventHandler(), &GameEventHandler::handleChatMessageSent);
        completer = new QCompleter(autocompleteUserList, sayEdit);
        completer->setCaseSensitivity(Qt::CaseInsensitive);
        completer->setMaxVisibleItems(5);
        completer->setFilterMode(Qt::MatchStartsWith);

        sayEdit->setCompleter(completer);
        actCompleterChanged();

        if (game->getPlayerManager()->isSpectator()) {
            /* Spectators can only talk if:
             * (a) the game creator allows it
             * (b) the spectator is a moderator/administrator
             * (c) the spectator is a judge
             */
            bool isModOrJudge = !tabSupervisor->getAdminLocked() || game->getPlayerManager()->isJudge();
            if (!isModOrJudge && !game->getGameMetaInfo()->spectatorsCanChat()) {
                sayLabel->hide();
                sayEdit->hide();
            }
        }

        connect(tabSupervisor, &TabSupervisor::adminLockChanged, this, &TabGame::adminLockChanged);
        connect(sayEdit, &LineEditCompleter::returnPressed, this, &TabGame::actSay);

        auto sayHLayout = new QHBoxLayout;
        sayHLayout->addWidget(sayLabel);
        sayHLayout->addWidget(sayEdit);

        messageLogLayout->addLayout(sayHLayout);
    }

    // dock
    auto messageLogLayoutWidget = new QWidget;
    messageLogLayoutWidget->setLayout(messageLogLayout);

    messageLayoutDock = new QDockWidget(this);
    messageLayoutDock->setObjectName("messageLayoutDock");
    messageLayoutDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable |
                                   QDockWidget::DockWidgetMovable);
    messageLayoutDock->setWidget(messageLogLayoutWidget);
    messageLayoutDock->setFloating(false);

    messageLayoutDock->installEventFilter(this);
    connect(messageLayoutDock, &QDockWidget::topLevelChanged, this, &TabGame::dockTopLevelChanged);
}

void TabGame::hideEvent(QHideEvent *event)
{
    LayoutsSettings &layouts = SettingsCache::instance().layouts();
    if (replayManager->replay) {
        layouts.setReplayPlayAreaState(saveState());
        layouts.setReplayPlayAreaGeometry(saveGeometry());
        layouts.setReplayCardInfoSize(cardInfoDock->size());
        layouts.setReplayMessageLayoutSize(messageLayoutDock->size());
        layouts.setReplayPlayerListSize(playerListDock->size());
        layouts.setReplayReplaySize(replayDock->size());
    } else {
        layouts.setGamePlayAreaState(saveState());
        layouts.setGamePlayAreaGeometry(saveGeometry());
        layouts.setGameCardInfoSize(cardInfoDock->size());
        layouts.setGameMessageLayoutSize(messageLayoutDock->size());
        layouts.setGamePlayerListSize(playerListDock->size());
    }

    Tab::hideEvent(event);
}

// Method uses to sync docks state with menu items state
bool TabGame::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::Close) {
        if (o == cardInfoDock) {
            aCardInfoDockVisible->setChecked(false);
            aCardInfoDockFloating->setEnabled(false);
        } else if (o == messageLayoutDock) {
            aMessageLayoutDockVisible->setChecked(false);
            aMessageLayoutDockFloating->setEnabled(false);
        } else if (o == playerListDock) {
            aPlayerListDockVisible->setChecked(false);
            aPlayerListDockFloating->setEnabled(false);
        } else if (o == replayDock) {
            aReplayDockVisible->setChecked(false);
            aReplayDockFloating->setEnabled(false);
        }
    }

    return false;
}

void TabGame::dockVisibleTriggered()
{
    QObject *o = sender();
    if (o == aCardInfoDockVisible) {
        cardInfoDock->setVisible(aCardInfoDockVisible->isChecked());
        aCardInfoDockFloating->setEnabled(aCardInfoDockVisible->isChecked());
        return;
    }

    if (o == aMessageLayoutDockVisible) {
        messageLayoutDock->setVisible(aMessageLayoutDockVisible->isChecked());
        aMessageLayoutDockFloating->setEnabled(aMessageLayoutDockVisible->isChecked());
        return;
    }

    if (o == aPlayerListDockVisible) {
        playerListDock->setVisible(aPlayerListDockVisible->isChecked());
        aPlayerListDockFloating->setEnabled(aPlayerListDockVisible->isChecked());
        return;
    }

    if (o == aReplayDockVisible) {
        replayDock->setVisible(aReplayDockVisible->isChecked());
        aReplayDockFloating->setEnabled(aReplayDockVisible->isChecked());
        return;
    }
}

void TabGame::dockFloatingTriggered()
{
    QObject *o = sender();
    if (o == aCardInfoDockFloating) {
        cardInfoDock->setFloating(aCardInfoDockFloating->isChecked());
        return;
    }

    if (o == aMessageLayoutDockFloating) {
        messageLayoutDock->setFloating(aMessageLayoutDockFloating->isChecked());
        return;
    }

    if (o == aPlayerListDockFloating) {
        playerListDock->setFloating(aPlayerListDockFloating->isChecked());
        return;
    }

    if (o == aReplayDockFloating) {
        replayDock->setFloating(aReplayDockFloating->isChecked());
        return;
    }
}

void TabGame::dockTopLevelChanged(bool topLevel)
{
    retranslateUi();

    QObject *o = sender();
    if (o == cardInfoDock) {
        aCardInfoDockFloating->setChecked(topLevel);
        return;
    }

    if (o == messageLayoutDock) {
        aMessageLayoutDockFloating->setChecked(topLevel);
        return;
    }

    if (o == playerListDock) {
        aPlayerListDockFloating->setChecked(topLevel);
        return;
    }

    if (o == replayDock) {
        aReplayDockFloating->setChecked(topLevel);
        return;
    }
}
