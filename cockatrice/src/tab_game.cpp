#include <QLabel>
#include <QHBoxLayout>
#include <QSplitter>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QFileDialog>
#include <QTimer>
#include <QToolButton>
#include <QDebug>
#include <QCompleter>
#include <QDockWidget>
#include <QWidget>
#include <QStackedWidget>

#include "dlg_creategame.h"
#include "tab_game.h"
#include "tab_supervisor.h"
#include "cardframe.h"
#include "playerlistwidget.h"
#include "messagelogwidget.h"
#include "phasestoolbar.h"
#include "gameview.h"
#include "gamescene.h"
#include "player.h"
#include "zoneviewzone.h"
#include "zoneviewwidget.h"
#include "deckview.h"
#include "deck_loader.h"
#include "dlg_load_remote_deck.h"
#include "abstractclient.h"
#include "carditem.h"
#include "arrowitem.h"
#include "main.h"
#include "settingscache.h"
#include "carddatabase.h"
#include "pictureloader.h"
#include "replay_timeline_widget.h"
#include "lineeditcompleter.h"
#include "window_sets.h"

#include <google/protobuf/descriptor.h>
#include "pending_command.h"
#include "pb/game_replay.pb.h"
#include "pb/command_concede.pb.h"
#include "pb/command_deck_select.pb.h"
#include "pb/command_ready_start.pb.h"
#include "pb/command_set_sideboard_plan.pb.h"
#include "pb/command_set_sideboard_lock.pb.h"
#include "pb/command_leave_game.pb.h"
#include "pb/command_game_say.pb.h"
#include "pb/command_set_active_phase.pb.h"
#include "pb/command_next_turn.pb.h"
#include "pb/command_delete_arrow.pb.h"
#include "pb/response_deck_download.pb.h"
#include "pb/game_event_container.pb.h"
#include "pb/event_game_joined.pb.h"
#include "pb/event_game_say.pb.h"
#include "pb/event_game_state_changed.pb.h"
#include "pb/event_player_properties_changed.pb.h"
#include "pb/event_join.pb.h"
#include "pb/event_leave.pb.h"
#include "pb/event_kicked.pb.h"
#include "pb/event_game_host_changed.pb.h"
#include "pb/event_game_closed.pb.h"
#include "pb/event_set_active_player.pb.h"
#include "pb/event_set_active_phase.pb.h"
#include "pb/context_deck_select.pb.h"
#include "pb/context_connection_state_changed.pb.h"
#include "pb/context_ping_changed.pb.h"
#include "get_pb_extension.h"

ToggleButton::ToggleButton(QWidget *parent)
        : QPushButton(parent), state(false)
{
}

void ToggleButton::paintEvent(QPaintEvent *event)
{
    QPushButton::paintEvent(event);

    QPainter painter(this);
    QPen pen;
    pen.setWidth(3);
    pen.setJoinStyle(Qt::MiterJoin);
    pen.setColor(state ? Qt::green : Qt::red);
    painter.setPen(pen);
    painter.drawRect(QRect(1, 1, width() - 3, height() - 3));
}

void ToggleButton::setState(bool _state)
{
    state = _state;
    emit stateChanged();
    update();
}

DeckViewContainer::DeckViewContainer(int _playerId, TabGame *parent)
        : QWidget(0), parentGame(parent), playerId(_playerId)
{
    loadLocalButton = new QPushButton;
    loadRemoteButton = new QPushButton;
    if (parentGame->getIsLocalGame())
        loadRemoteButton->setEnabled(false);
    readyStartButton = new ToggleButton;
    readyStartButton->setEnabled(false);
    sideboardLockButton = new ToggleButton;
    sideboardLockButton->setEnabled(false);

    connect(loadLocalButton, SIGNAL(clicked()), this, SLOT(loadLocalDeck()));
    connect(loadRemoteButton, SIGNAL(clicked()), this, SLOT(loadRemoteDeck()));
    connect(readyStartButton, SIGNAL(clicked()), this, SLOT(readyStart()));
    connect(sideboardLockButton, SIGNAL(clicked()), this, SLOT(sideboardLockButtonClicked()));
    connect(sideboardLockButton, SIGNAL(stateChanged()), this, SLOT(updateSideboardLockButtonText()));

    QHBoxLayout *buttonHBox = new QHBoxLayout;
    buttonHBox->addWidget(loadLocalButton);
    buttonHBox->addWidget(loadRemoteButton);
    buttonHBox->addWidget(readyStartButton);
    buttonHBox->addWidget(sideboardLockButton);
    buttonHBox->setContentsMargins(0, 0, 0, 0);
    buttonHBox->addStretch();
    deckView = new DeckView;
    connect(deckView, SIGNAL(newCardAdded(AbstractCardItem *)), this, SIGNAL(newCardAdded(AbstractCardItem *)));
    connect(deckView, SIGNAL(sideboardPlanChanged()), this, SLOT(sideboardPlanChanged()));

    QVBoxLayout *deckViewLayout = new QVBoxLayout;
    deckViewLayout->addLayout(buttonHBox);
    deckViewLayout->addWidget(deckView);
    deckViewLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(deckViewLayout);

    retranslateUi();
    connect(&settingsCache->shortcuts(), SIGNAL(shortCutchanged()),this,SLOT(refreshShortcuts()));
    refreshShortcuts();
}

void DeckViewContainer::retranslateUi()
{
    loadLocalButton->setText(tr("Load deck..."));
    loadRemoteButton->setText(tr("Load remote deck..."));
    readyStartButton->setText(tr("Ready to s&tart"));
    updateSideboardLockButtonText();
}

void DeckViewContainer::setButtonsVisible(bool _visible)
{
    loadLocalButton->setVisible(_visible);
    loadRemoteButton->setVisible(_visible);
    readyStartButton->setVisible(_visible);
    sideboardLockButton->setVisible(_visible);
}

void DeckViewContainer::updateSideboardLockButtonText()
{
    if (sideboardLockButton->getState())
        sideboardLockButton->setText(tr("S&ideboard unlocked"));
    else
        sideboardLockButton->setText(tr("S&ideboard locked"));
}

void DeckViewContainer::refreshShortcuts()
{
    loadLocalButton->setShortcut(settingsCache->shortcuts().getSingleShortcut("DeckViewContainer/loadLocalButton"));
    loadRemoteButton->setShortcut(settingsCache->shortcuts().getSingleShortcut("DeckViewContainer/loadRemoteButton"));
}

void TabGame::refreshShortcuts()
{
    for (int i = 0; i < phaseActions.size(); ++i) {
        QAction *temp = phaseActions.at(i);
        switch (i) {
            case 0: temp->setShortcuts(settingsCache->shortcuts().getShortcut("Player/phase0")); break;
            case 1: temp->setShortcuts(settingsCache->shortcuts().getShortcut("Player/phase1")); break;
            case 2: temp->setShortcuts(settingsCache->shortcuts().getShortcut("Player/phase2")); break;
            case 3: temp->setShortcuts(settingsCache->shortcuts().getShortcut("Player/phase3")); break;
            case 4: temp->setShortcuts(settingsCache->shortcuts().getShortcut("Player/phase4")); break;
            case 5: temp->setShortcuts(settingsCache->shortcuts().getShortcut("Player/phase5")); break;
            case 6: temp->setShortcuts(settingsCache->shortcuts().getShortcut("Player/phase6")); break;
            case 7: temp->setShortcuts(settingsCache->shortcuts().getShortcut("Player/phase7")); break;
            case 8: temp->setShortcuts(settingsCache->shortcuts().getShortcut("Player/phase8")); break;
            case 9: temp->setShortcuts(settingsCache->shortcuts().getShortcut("Player/phase9")); break;
            case 10:temp->setShortcuts(settingsCache->shortcuts().getShortcut("Player/phase10")); break;
            default: ;
        }
    }

    if (aNextPhase) {
        aNextPhase->setShortcuts(settingsCache->shortcuts().getShortcut("Player/aNextPhase"));
    }
    if (aNextTurn) {
        aNextTurn->setShortcuts(settingsCache->shortcuts().getShortcut("Player/aNextTurn"));
    }
    if (aRemoveLocalArrows) {
        aRemoveLocalArrows->setShortcuts(settingsCache->shortcuts().getShortcut("Player/aRemoveLocalArrows"));
    }
    if (aRotateViewCW) {
        aRotateViewCW->setShortcuts(settingsCache->shortcuts().getShortcut("Player/aRotateViewCW"));
    }
    if (aRotateViewCCW) {
        aRotateViewCCW->setShortcuts(settingsCache->shortcuts().getShortcut("Player/aRotateViewCCW"));
    }
    if (aConcede) {
        aConcede->setShortcuts(settingsCache->shortcuts().getShortcut("Player/aConcede"));
    }
    if (aLeaveGame) {
        aLeaveGame->setShortcuts(settingsCache->shortcuts().getShortcut("Player/aLeaveGame"));
    }
    if (aCloseReplay) {
        aCloseReplay->setShortcuts(settingsCache->shortcuts().getShortcut("Player/aCloseReplay"));
    }
    if (aResetLayout) {
        aResetLayout->setShortcuts(settingsCache->shortcuts().getShortcut("Player/aResetLayout"));
    }
}

void DeckViewContainer::loadLocalDeck()
{
    QFileDialog dialog(this, tr("Load deck"));
    dialog.setDirectory(settingsCache->getDeckPath());
    dialog.setNameFilters(DeckLoader::fileNameFilters);
    if (!dialog.exec())
        return;

    QString fileName = dialog.selectedFiles().at(0);
    DeckLoader::FileFormat fmt = DeckLoader::getFormatFromName(fileName);
    DeckLoader deck;
    if (!deck.loadFromFile(fileName, fmt)) {
        QMessageBox::critical(this, tr("Error"), tr("The selected file could not be loaded."));
        return;
    }

    Command_DeckSelect cmd;
    cmd.set_deck(deck.writeToString_Native().toStdString());
    PendingCommand *pend = parentGame->prepareGameCommand(cmd);
    connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this, SLOT(deckSelectFinished(const Response &)));
    parentGame->sendGameCommand(pend, playerId);
}

void DeckViewContainer::loadRemoteDeck()
{
    DlgLoadRemoteDeck dlg(parentGame->getClientForPlayer(playerId));
    if (dlg.exec()) {
        Command_DeckSelect cmd;
        cmd.set_deck_id(dlg.getDeckId());
        PendingCommand *pend = parentGame->prepareGameCommand(cmd);
        connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this, SLOT(deckSelectFinished(const Response &)));
        parentGame->sendGameCommand(pend, playerId);
    }
}

void DeckViewContainer::deckSelectFinished(const Response &r)
{
    const Response_DeckDownload &resp = r.GetExtension(Response_DeckDownload::ext);
    DeckLoader newDeck(QString::fromStdString(resp.deck()));
    PictureLoader::cacheCardPixmaps(db->getCards(newDeck.getCardList()));
    setDeck(newDeck);
}

void DeckViewContainer::readyStart()
{
    Command_ReadyStart cmd;
    cmd.set_ready(!readyStartButton->getState());
    parentGame->sendGameCommand(cmd, playerId);
}

void DeckViewContainer::sideboardLockButtonClicked()
{
    Command_SetSideboardLock cmd;
    cmd.set_locked(sideboardLockButton->getState());

    parentGame->sendGameCommand(cmd, playerId);
}

void DeckViewContainer::sideboardPlanChanged()
{
    Command_SetSideboardPlan cmd;
    const QList<MoveCard_ToZone> &newPlan = deckView->getSideboardPlan();
    for (int i = 0; i < newPlan.size(); ++i)
        cmd.add_move_list()->CopyFrom(newPlan[i]);
    parentGame->sendGameCommand(cmd, playerId);
}

void DeckViewContainer::setReadyStart(bool ready)
{
    readyStartButton->setState(ready);
    deckView->setLocked(ready || !sideboardLockButton->getState());
    sideboardLockButton->setEnabled(!readyStartButton->getState());
}

void DeckViewContainer::setSideboardLocked(bool locked)
{
    sideboardLockButton->setState(!locked);
    deckView->setLocked(readyStartButton->getState() || !sideboardLockButton->getState());
    if (locked)
        deckView->resetSideboardPlan();
}

void DeckViewContainer::setDeck(const DeckLoader &deck)
{
    deckView->setDeck(deck);
    readyStartButton->setEnabled(true);
    sideboardLockButton->setState(false);
    sideboardLockButton->setEnabled(true);
}

TabGame::TabGame(TabSupervisor *_tabSupervisor, GameReplay *_replay)
        : Tab(_tabSupervisor),
          secondsElapsed(0),
          hostId(-1),
          localPlayerId(-1),
          isLocalGame(_tabSupervisor->getIsLocalGame()),
          spectator(true),
          gameStateKnown(false),
          resuming(false),
          currentPhase(-1),
          activeCard(0),
          gameClosed(false),
          replay(_replay),
          currentReplayStep(0),
          sayLabel(0),
          sayEdit(0)
{
    // THIS CTOR IS USED ON REPLAY
    gameInfo.CopyFrom(replay->game_info());
    gameInfo.set_spectators_omniscient(true);

    // Create list: event number -> time [ms]
    // Distribute simultaneous events evenly across 1 second.
    unsigned int lastEventTimestamp = 0;
    const int eventCount = replay->event_list_size();
    for (int i = 0; i < eventCount; ++i) {
        int j = i + 1;
        while ((j < eventCount) && (replay->event_list(j).seconds_elapsed() == lastEventTimestamp))
            ++j;

        const int numberEventsThisSecond = j - i;
        for (int k = 0; k < numberEventsThisSecond; ++k)
            replayTimeline.append(replay->event_list(i + k).seconds_elapsed() * 1000 + (int) ((qreal) k / (qreal) numberEventsThisSecond * 1000));

        if (j < eventCount)
            lastEventTimestamp = replay->event_list(j).seconds_elapsed();
        i += numberEventsThisSecond - 1;
    }

    createCardInfoDock(true);
    createPlayerListDock(true);
    createMessageDock(true);
    createPlayAreaWidget(true);
    createDeckViewContainerWidget(true);
    createReplayDock();

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
    retranslateUi();
    connect(&settingsCache->shortcuts(), SIGNAL(shortCutchanged()),this,SLOT(refreshShortcuts()));
    refreshShortcuts();
    messageLog->logReplayStarted(gameInfo.game_id());

    this->installEventFilter(this);
    QTimer::singleShot(0, this, SLOT(loadLayout()));
}

TabGame::TabGame(TabSupervisor *_tabSupervisor, QList<AbstractClient *> &_clients, const Event_GameJoined &event, const QMap<int, QString> &_roomGameTypes)
        : Tab(_tabSupervisor),
          clients(_clients),
          gameInfo(event.game_info()),
          roomGameTypes(_roomGameTypes),
          hostId(event.host_id()),
          localPlayerId(event.player_id()),
          isLocalGame(_tabSupervisor->getIsLocalGame()),
          spectator(event.spectator()),
          gameStateKnown(false),
          resuming(event.resuming()),
          currentPhase(-1),
          activeCard(0),
          gameClosed(false),
          replay(0),
          replayDock(0)
{
    // THIS CTOR IS USED ON GAMES
    gameInfo.set_started(false);

    createCardInfoDock();
    createPlayerListDock();
    createMessageDock();
    createPlayAreaWidget();
    createDeckViewContainerWidget();

    addDockWidget(Qt::RightDockWidgetArea, cardInfoDock);
    addDockWidget(Qt::RightDockWidgetArea, playerListDock);
    addDockWidget(Qt::RightDockWidgetArea, messageLayoutDock);

    mainWidget = new QStackedWidget(this);
    mainWidget->addWidget(deckViewContainerWidget);
    mainWidget->addWidget(gamePlayAreaWidget);
    mainWidget->setContentsMargins(0,0,0,0);
    setCentralWidget(mainWidget);

    createMenuItems();
    createViewMenuItems();
    retranslateUi();
    connect(&settingsCache->shortcuts(), SIGNAL(shortCutchanged()),this,SLOT(refreshShortcuts()));
    refreshShortcuts();
    messageLog->logGameJoined(gameInfo.game_id());

    // append game to rooms game list for others to see
    for (int i = gameInfo.game_types_size() - 1; i >= 0; i--)
        gameTypes.append(roomGameTypes.find(gameInfo.game_types(i)).value());

    this->installEventFilter(this);
    QTimer::singleShot(0, this, SLOT(loadLayout()));
}

void TabGame::addMentionTag(QString value) {
    sayEdit->insert(value + " ");
    sayEdit->setFocus();
}

void TabGame::emitUserEvent() {
    bool globalEvent = !spectator || settingsCache->getSpectatorNotificationsEnabled();
    emit userEvent(globalEvent);
}

TabGame::~TabGame()
{
    if(replay)
        delete replay;

    QMapIterator<int, Player *> i(players);
    while (i.hasNext())
        delete i.next().value();
    players.clear();

    emit gameClosing(this);
}

void TabGame::retranslateUi()
{
    QString tabText = " | " + (replay ? tr("Replay") : tr("Game")) + " #" + QString::number(gameInfo.game_id());

    cardInfoDock->setWindowTitle(tr("Card Info") + (cardInfoDock->isWindow() ? tabText : QString()));
    playerListDock->setWindowTitle(tr("Player List") + (playerListDock->isWindow() ? tabText : QString()));
    messageLayoutDock->setWindowTitle(tr("Messages") + (messageLayoutDock->isWindow() ? tabText : QString()));
    if(replayDock)
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
    if (aNextTurn) {
        aNextTurn->setText(tr("Next &turn"));
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
        aConcede->setText(tr("&Concede"));
    }
    if (aLeaveGame) {
        aLeaveGame->setText(tr("&Leave game"));
    }
    if (aCloseReplay) {
        aCloseReplay->setText(tr("C&lose replay"));
    }
    if (sayLabel){
        sayLabel->setText(tr("&Say:"));
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

    if(replayDock)
    {
        replayDockMenu->setTitle(tr("Replay Timeline"));
        aReplayDockVisible->setText(tr("Visible"));
        aReplayDockFloating->setText(tr("Floating"));
    }

    aResetLayout->setText(tr("Reset layout"));

    cardInfo->retranslateUi();

    QMapIterator<int, Player *> i(players);
    while (i.hasNext())
        i.next().value()->retranslateUi();
    QMapIterator<int, DeckViewContainer *> j(deckViewContainers);
    while (j.hasNext())
        j.next().value()->retranslateUi();

    scene->retranslateUi();
}

void TabGame::closeRequest()
{
    actLeaveGame();
}

void TabGame::replayNextEvent()
{
    processGameEventContainer(replay->event_list(timelineWidget->getCurrentEvent()), 0);
}

void TabGame::replayFinished()
{
    replayStartButton->setEnabled(true);
    replayPauseButton->setEnabled(false);
    replayFastForwardButton->setEnabled(false);
}


void TabGame::replayStartButtonClicked()
{
    replayStartButton->setEnabled(false);
    replayPauseButton->setEnabled(true);
    replayFastForwardButton->setEnabled(true);

    timelineWidget->startReplay();
}

void TabGame::replayPauseButtonClicked()
{
    replayStartButton->setEnabled(true);
    replayPauseButton->setEnabled(false);
    replayFastForwardButton->setEnabled(false);

    timelineWidget->stopReplay();
}

void TabGame::replayFastForwardButtonToggled(bool checked)
{
    timelineWidget->setTimeScaleFactor(checked ? 10.0 : 1.0);
}

void TabGame::incrementGameTime()
{
    int seconds = ++secondsElapsed;
    int minutes = seconds / 60;
    seconds -= minutes * 60;
    int hours = minutes / 60;
    minutes -= hours * 60;

    timeElapsedLabel->setText(QString::number(hours).rightJustified(2, '0') + ":" + QString::number(minutes).rightJustified(2, '0') + ":" + QString::number(seconds).rightJustified(2, '0'));
}

void TabGame::adminLockChanged(bool lock)
{
    bool v = !(spectator && !gameInfo.spectators_can_chat() && lock);
    sayLabel->setVisible(v);
    sayEdit->setVisible(v);
}

void TabGame::actGameInfo()
{
    DlgCreateGame dlg(gameInfo, roomGameTypes);
    dlg.exec();
}

void TabGame::actConcede()
{
    if (QMessageBox::question(this, tr("Concede"), tr("Are you sure you want to concede this game?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
        return;

    sendGameCommand(Command_Concede());
}

void TabGame::actLeaveGame()
{
    if (!gameClosed) {
        if (!spectator)
        if (QMessageBox::question(this, tr("Leave game"), tr("Are you sure you want to leave this game?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
            return;

        if (!replay)
            sendGameCommand(Command_LeaveGame());
    }
    scene->clearViews();
    deleteLater();
}

void TabGame::actSay()
{
    if (completer->popup()->isVisible())
        return;

    if (!sayEdit->text().isEmpty()) {
        Command_GameSay cmd;
        cmd.set_message(sayEdit->text().toStdString());
        sendGameCommand(cmd);
        sayEdit->clear();
    }
}

void TabGame::actPhaseAction()
{
    int phase = phaseActions.indexOf(static_cast<QAction *>(sender()));
    Command_SetActivePhase cmd;
    cmd.set_phase(phase);
    sendGameCommand(cmd);
}

void TabGame::actNextPhase()
{
    int phase = currentPhase;
    if (++phase >= phasesToolbar->phaseCount())
        phase = 0;
    Command_SetActivePhase cmd;
    cmd.set_phase(phase);
    sendGameCommand(cmd);
}

void TabGame::actNextTurn()
{
    sendGameCommand(Command_NextTurn());
}

void TabGame::actRemoveLocalArrows()
{
    QMapIterator<int, Player *> playerIterator(players);
    while (playerIterator.hasNext()) {
        Player *player = playerIterator.next().value();
        if (!player->getLocal())
            continue;
        QMapIterator<int, ArrowItem *> arrowIterator(player->getArrows());
        while (arrowIterator.hasNext()) {
            ArrowItem *a = arrowIterator.next().value();
            Command_DeleteArrow cmd;
            cmd.set_arrow_id(a->getId());
            sendGameCommand(cmd);
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
    settingsCache->getChatMentionCompleter() ? completer->setCompletionRole(2) : completer->setCompletionRole(1);
}

Player *TabGame::addPlayer(int playerId, const ServerInfo_User &info)
{
    bool local = ((clients.size() > 1) || (playerId == localPlayerId));
    Player *newPlayer = new Player(info, playerId, local, this);
    connect(newPlayer, SIGNAL(openDeckEditor(const DeckLoader *)), this, SIGNAL(openDeckEditor(const DeckLoader *)));
    QString newPlayerName = "@" + newPlayer->getName();
    if (sayEdit && !autocompleteUserList.contains(newPlayerName)){
        autocompleteUserList << newPlayerName;
        sayEdit->setCompletionList(autocompleteUserList);
    }
    scene->addPlayer(newPlayer);

    connect(newPlayer, SIGNAL(newCardAdded(AbstractCardItem *)), this, SLOT(newCardAdded(AbstractCardItem *)));
    messageLog->connectToPlayer(newPlayer);

    if (local && !spectator) {
        if (clients.size() == 1)
            newPlayer->setShortcutsActive();

        DeckViewContainer *deckView = new DeckViewContainer(playerId, this);
        connect(deckView, SIGNAL(newCardAdded(AbstractCardItem *)), this, SLOT(newCardAdded(AbstractCardItem *)));
        deckViewContainers.insert(playerId, deckView);
        deckViewContainerLayout->addWidget(deckView);
    }

    gameMenu->insertMenu(playersSeparator, newPlayer->getPlayerMenu());

    players.insert(playerId, newPlayer);
    emit playerAdded(newPlayer);
    return newPlayer;
}

void TabGame::processGameEventContainer(const GameEventContainer &cont, AbstractClient *client)
{
    const GameEventContext &context = cont.context();
    messageLog->containerProcessingStarted(context);
    const int eventListSize = cont.event_list_size();
    for (int i = 0; i < eventListSize; ++i) {
        const GameEvent &event = cont.event_list(i);
        const int playerId = event.player_id();
        const GameEvent::GameEventType eventType = static_cast<GameEvent::GameEventType>(getPbExtension(event));
        if (spectators.contains(playerId)) {
            switch (eventType) {
                case GameEvent::GAME_SAY: eventSpectatorSay(event.GetExtension(Event_GameSay::ext), playerId, context); break;
                case GameEvent::LEAVE: eventSpectatorLeave(event.GetExtension(Event_Leave::ext), playerId, context); break;
                default: {
                    qDebug() << "unhandled spectator game event";
                    break;
                }
            }
        } else {
            if ((clients.size() > 1) && (playerId != -1))
            if (clients.at(playerId) != client)
                continue;

            switch (eventType) {
                case GameEvent::GAME_STATE_CHANGED: eventGameStateChanged(event.GetExtension(Event_GameStateChanged::ext), playerId, context); break;
                case GameEvent::PLAYER_PROPERTIES_CHANGED: eventPlayerPropertiesChanged(event.GetExtension(Event_PlayerPropertiesChanged::ext), playerId, context); break;
                case GameEvent::JOIN: eventJoin(event.GetExtension(Event_Join::ext), playerId, context); break;
                case GameEvent::LEAVE: eventLeave(event.GetExtension(Event_Leave::ext), playerId, context); break;
                case GameEvent::KICKED: eventKicked(event.GetExtension(Event_Kicked::ext), playerId, context); break;
                case GameEvent::GAME_HOST_CHANGED: eventGameHostChanged(event.GetExtension(Event_GameHostChanged::ext), playerId, context); break;
                case GameEvent::GAME_CLOSED: eventGameClosed(event.GetExtension(Event_GameClosed::ext), playerId, context); break;
                case GameEvent::SET_ACTIVE_PLAYER: eventSetActivePlayer(event.GetExtension(Event_SetActivePlayer::ext), playerId, context); break;
                case GameEvent::SET_ACTIVE_PHASE: eventSetActivePhase(event.GetExtension(Event_SetActivePhase::ext), playerId, context); break;

                default: {
                    Player *player = players.value(playerId, 0);
                    if (!player) {
                        qDebug() << "unhandled game event: invalid player id";
                        break;
                    }
                    player->processGameEvent(eventType, event, context);
                    emitUserEvent();
                }
            }
        }
    }
    messageLog->containerProcessingDone();
}

AbstractClient *TabGame::getClientForPlayer(int playerId) const
{
    if (clients.size() > 1) {
        if (playerId == -1)
            playerId = getActiveLocalPlayer()->getId();

        return clients.at(playerId);
    } else if (clients.isEmpty())
        return 0;
    else
        return clients.first();
}

void TabGame::sendGameCommand(PendingCommand *pend, int playerId)
{
    AbstractClient *client = getClientForPlayer(playerId);
    if (!client)
        return;

    connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this, SLOT(commandFinished(const Response &)));
    client->sendCommand(pend);
}

void TabGame::sendGameCommand(const google::protobuf::Message &command, int playerId)
{
    AbstractClient *client = getClientForPlayer(playerId);
    if (!client)
        return;

    PendingCommand *pend = prepareGameCommand(command);
    connect(pend, SIGNAL(finished(Response, CommandContainer, QVariant)), this, SLOT(commandFinished(const Response &)));
    client->sendCommand(pend);
}

void TabGame::commandFinished(const Response &response)
{
    if (response.response_code() == Response::RespChatFlood)
        messageLog->appendMessage(tr("You are flooding the game. Please wait a couple of seconds."));
}

PendingCommand *TabGame::prepareGameCommand(const ::google::protobuf::Message &cmd)
{
    CommandContainer cont;
    cont.set_game_id(gameInfo.game_id());
    GameCommand *c = cont.add_game_command();
    c->GetReflection()->MutableMessage(c, cmd.GetDescriptor()->FindExtensionByName("ext"))->CopyFrom(cmd);
    return new PendingCommand(cont);
}

PendingCommand *TabGame::prepareGameCommand(const QList< const ::google::protobuf::Message * > &cmdList)
{
    CommandContainer cont;
    cont.set_game_id(gameInfo.game_id());
    for (int i = 0; i < cmdList.size(); ++i) {
        GameCommand *c = cont.add_game_command();
        c->GetReflection()->MutableMessage(c, cmdList[i]->GetDescriptor()->FindExtensionByName("ext"))->CopyFrom(*cmdList[i]);
        delete cmdList[i];
    }
    return new PendingCommand(cont);
}

void TabGame::startGame(bool resuming)
{
    currentPhase = -1;

    QMapIterator<int, DeckViewContainer *> i(deckViewContainers);
    while (i.hasNext()) {
        i.next();
        i.value()->setReadyStart(false);
        i.value()->hide();
    }

    mainWidget->setCurrentWidget(gamePlayAreaWidget);

    if (!resuming) {
        QMapIterator<int, Player *> playerIterator(players);
        while (playerIterator.hasNext())
            playerIterator.next().value()->setGameStarted();
    }

    playerListWidget->setGameStarted(true, resuming);
    gameInfo.set_started(true);
    static_cast<GameScene *>(gameView->scene())->rearrange();
    if(sayEdit && players.size() > 1)
        sayEdit->setFocus();
}

void TabGame::stopGame()
{
    currentPhase = -1;
    activePlayer = -1;

    QMapIterator<int, DeckViewContainer *> i(deckViewContainers);
    while (i.hasNext()) {
        i.next();
        i.value()->show();
    }

    mainWidget->setCurrentWidget(deckViewContainerWidget);

    playerListWidget->setActivePlayer(-1);
    playerListWidget->setGameStarted(false, false);
    gameInfo.set_started(false);
}

void TabGame::closeGame()
{
    gameInfo.set_started(false);
    gameClosed = true;

    gameMenu->clear();
    gameMenu->addAction(aLeaveGame);
}

void TabGame::eventSpectatorSay(const Event_GameSay &event, int eventPlayerId, const GameEventContext & /*context*/)
{
    const ServerInfo_User &userInfo = spectators.value(eventPlayerId);
    messageLog->logSpectatorSay(QString::fromStdString(userInfo.name()), UserLevelFlags(userInfo.user_level()), QString::fromStdString(userInfo.privlevel()), QString::fromStdString(event.message()));
}

void TabGame::eventSpectatorLeave(const Event_Leave & /*event*/, int eventPlayerId, const GameEventContext & /*context*/)
{
    QString playerName = "@" + QString::fromStdString(spectators.value(eventPlayerId).name());
    if (sayEdit && autocompleteUserList.removeOne(playerName))
        sayEdit->setCompletionList(autocompleteUserList);
    messageLog->logLeaveSpectator(QString::fromStdString(spectators.value(eventPlayerId).name()));
    playerListWidget->removePlayer(eventPlayerId);
    spectators.remove(eventPlayerId);

    emitUserEvent();
}

void TabGame::eventGameStateChanged(const Event_GameStateChanged &event, int /*eventPlayerId*/, const GameEventContext & /*context*/)
{
    const int playerListSize = event.player_list_size();
    for (int i = 0; i < playerListSize; ++i) {
        const ServerInfo_Player &playerInfo = event.player_list(i);
        const ServerInfo_PlayerProperties &prop = playerInfo.properties();
        const int playerId = prop.player_id();
        QString playerName = "@" + QString::fromStdString(prop.user_info().name());
        if (sayEdit && !autocompleteUserList.contains(playerName)){
            autocompleteUserList << playerName;
            sayEdit->setCompletionList(autocompleteUserList);
        }
        if (prop.spectator()) {
            if (!spectators.contains(playerId)) {
                spectators.insert(playerId, prop.user_info());
                playerListWidget->addPlayer(prop);
            }
        } else {
            Player *player = players.value(playerId, 0);
            if (!player) {
                player = addPlayer(playerId, prop.user_info());
                playerListWidget->addPlayer(prop);
            }
            player->processPlayerInfo(playerInfo);
            if (player->getLocal()) {
                DeckViewContainer *deckViewContainer = deckViewContainers.value(playerId);
                if (playerInfo.has_deck_list()) {
                    DeckLoader newDeck(QString::fromStdString(playerInfo.deck_list()));
                    PictureLoader::cacheCardPixmaps(db->getCards(newDeck.getCardList()));
                    deckViewContainer->setDeck(newDeck);
                    player->setDeck(newDeck);
                }
                deckViewContainer->setReadyStart(prop.ready_start());
                deckViewContainer->setSideboardLocked(prop.sideboard_locked());
            }
        }
    }
    for (int i = 0; i < playerListSize; ++i) {
        const ServerInfo_Player &playerInfo = event.player_list(i);
        const ServerInfo_PlayerProperties &prop = playerInfo.properties();
        if (!prop.spectator()) {
            Player *player = players.value(prop.player_id(), 0);
            if (!player)
                continue;
            player->processCardAttachment(playerInfo);
        }
    }

    secondsElapsed = event.seconds_elapsed();

    if (event.game_started() && !gameInfo.started()) {
        startGame(!gameStateKnown);
        if (gameStateKnown)
            messageLog->logGameStart();
        setActivePlayer(event.active_player_id());
        setActivePhase(event.active_phase());
    } else if (!event.game_started() && gameInfo.started()) {
        stopGame();
        scene->clearViews();
    }
    gameStateKnown = true;
    emitUserEvent();
}

void TabGame::eventPlayerPropertiesChanged(const Event_PlayerPropertiesChanged &event, int eventPlayerId, const GameEventContext &context)
{
    Player *player = players.value(eventPlayerId, 0);
    if (!player)
        return;
    const ServerInfo_PlayerProperties &prop = event.player_properties();
    playerListWidget->updatePlayerProperties(prop, eventPlayerId);

    const GameEventContext::ContextType contextType = static_cast<const GameEventContext::ContextType>(getPbExtension(context));
    switch (contextType) {
        case GameEventContext::READY_START: {
            bool ready = prop.ready_start();
            if (player->getLocal())
                deckViewContainers.value(player->getId())->setReadyStart(ready);
            if (ready)
                messageLog->logReadyStart(player);
            else
                messageLog->logNotReadyStart(player);
            break;
        }
        case GameEventContext::CONCEDE: {
            messageLog->logConcede(player);
            player->setConceded(true);

            QMapIterator<int, Player *> playerIterator(players);
            while (playerIterator.hasNext())
                playerIterator.next().value()->updateZones();

            break;
        }
        case GameEventContext::DECK_SELECT: {
            Context_DeckSelect deckSelect = context.GetExtension(Context_DeckSelect::ext);
            messageLog->logDeckSelect(player,
                                      QString::fromStdString(deckSelect.deck_hash()),
                                      deckSelect.sideboard_size());
            break;
        }
        case GameEventContext::SET_SIDEBOARD_LOCK: {
            if (player->getLocal())
                deckViewContainers.value(player->getId())->setSideboardLocked(prop.sideboard_locked());
            messageLog->logSetSideboardLock(player, prop.sideboard_locked());
            break;
        }
        case GameEventContext::CONNECTION_STATE_CHANGED: {
            messageLog->logConnectionStateChanged(player, prop.ping_seconds() != -1);
            break;
        }
        default: ;
    }
}

void TabGame::eventJoin(const Event_Join &event, int /*eventPlayerId*/, const GameEventContext & /*context*/)
{
    const ServerInfo_PlayerProperties &playerInfo = event.player_properties();
    const int playerId = playerInfo.player_id();
    QString playerName = QString::fromStdString(playerInfo.user_info().name());
    if (sayEdit && !autocompleteUserList.contains("@" + playerName)){
        autocompleteUserList << "@" + playerName;
        sayEdit->setCompletionList(autocompleteUserList);
    }

    if (players.contains(playerId))
        return;

    if (playerInfo.spectator()) {
        spectators.insert(playerId, playerInfo.user_info());
        messageLog->logJoinSpectator(playerName);
    } else {
        Player *newPlayer = addPlayer(playerId, playerInfo.user_info());
        messageLog->logJoin(newPlayer);
    }
    playerListWidget->addPlayer(playerInfo);
    emitUserEvent();
}

void TabGame::eventLeave(const Event_Leave & /*event*/, int eventPlayerId, const GameEventContext & /*context*/)
{
    Player *player = players.value(eventPlayerId, 0);
    if (!player)
        return;

    QString playerName = "@" + player->getName();
    if(sayEdit && autocompleteUserList.removeOne(playerName))
        sayEdit->setCompletionList(autocompleteUserList);

    messageLog->logLeave(player);
    playerListWidget->removePlayer(eventPlayerId);
    players.remove(eventPlayerId);
    emit playerRemoved(player);
    player->clear();
    player->deleteLater();

    // Rearrange all remaining zones so that attachment relationship updates take place
    QMapIterator<int, Player *> playerIterator(players);
    while (playerIterator.hasNext())
        playerIterator.next().value()->updateZones();

    emitUserEvent();
}

void TabGame::eventKicked(const Event_Kicked & /*event*/, int /*eventPlayerId*/, const GameEventContext & /*context*/)
{
    closeGame();
    tabSupervisor->setCurrentIndex(tabSupervisor->indexOf(this));
    messageLog->logKicked();

    QMessageBox msgBox(this);
    msgBox.setWindowTitle(getTabText());
    msgBox.setText(tr("You have been kicked out of the game."));
    msgBox.setIcon(QMessageBox::Information);
    msgBox.exec();

    emitUserEvent();
}

void TabGame::eventGameHostChanged(const Event_GameHostChanged & /*event*/, int eventPlayerId, const GameEventContext & /*context*/)
{
    hostId = eventPlayerId;
}

void TabGame::eventGameClosed(const Event_GameClosed & /*event*/, int /*eventPlayerId*/, const GameEventContext & /*context*/)
{
    closeGame();
    messageLog->logGameClosed();
    emitUserEvent();
}

Player *TabGame::setActivePlayer(int id)
{
    Player *player = players.value(id, 0);
    if (!player)
        return 0;
    activePlayer = id;
    playerListWidget->setActivePlayer(id);
    QMapIterator<int, Player *> i(players);
    while (i.hasNext()) {
        i.next();
        if (i.value() == player) {
            i.value()->setActive(true);
            if (clients.size() > 1)
                i.value()->setShortcutsActive();
        } else {
            i.value()->setActive(false);
            if (clients.size() > 1)
                i.value()->setShortcutsInactive();
        }
    }
    currentPhase = -1;
    emitUserEvent();
    return player;
}

void TabGame::eventSetActivePlayer(const Event_SetActivePlayer &event, int /*eventPlayerId*/, const GameEventContext & /*context*/)
{
    Player *player = setActivePlayer(event.active_player_id());
    if (!player)
        return;
    messageLog->logSetActivePlayer(player);
    emitUserEvent();
}

void TabGame::setActivePhase(int phase)
{
    if (currentPhase != phase) {
        currentPhase = phase;
        phasesToolbar->setActivePhase(phase);
    }
}

void TabGame::eventSetActivePhase(const Event_SetActivePhase &event, int /*eventPlayerId*/, const GameEventContext & /*context*/)
{
    const int phase = event.phase();
    if (currentPhase != phase)
        messageLog->logSetActivePhase(phase);
    setActivePhase(phase);
    emitUserEvent();
}

void TabGame::newCardAdded(AbstractCardItem *card)
{
    connect(card, SIGNAL(hovered(AbstractCardItem *)), cardInfo, SLOT(setCard(AbstractCardItem *)));
    connect(card, SIGNAL(showCardInfoPopup(QPoint, QString)), this, SLOT(showCardInfoPopup(QPoint, QString)));
    connect(card, SIGNAL(deleteCardInfoPopup(QString)), this, SLOT(deleteCardInfoPopup(QString)));
    connect(card, SIGNAL(updateCardMenu(AbstractCardItem *)), this, SLOT(updateCardMenu(AbstractCardItem *)));
}

CardItem *TabGame::getCard(int playerId, const QString &zoneName, int cardId) const
{
    Player *player = players.value(playerId, 0);
    if (!player)
        return 0;

    CardZone *zone = player->getZones().value(zoneName, 0);
    if (!zone)
        return 0;

    return zone->getCard(cardId, QString());
}

QString TabGame::getTabText() const
{
    QString gameTypeInfo;
    if (gameTypes.size() != 0) {
        gameTypeInfo = gameTypes.at(0);
        if (gameTypes.size() > 1)
            gameTypeInfo.append("...");
    }

    QString gameDesc(gameInfo.description().c_str());
    QString gameId(QString::number(gameInfo.game_id()));

    QString tabText;
    if (replay)
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

Player *TabGame::getActiveLocalPlayer() const
{
    Player *active = players.value(activePlayer, 0);
    if (active)
    if (active->getLocal())
        return active;

    QMapIterator<int, Player *> playerIterator(players);
    while (playerIterator.hasNext()) {
        Player *temp = playerIterator.next().value();
        if (temp->getLocal())
            return temp;
    }

    return 0;
}

void TabGame::updateCardMenu(AbstractCardItem *card)
{
    Player *p;
    if ((clients.size() > 1) || !players.contains(localPlayerId))
        p = card->getOwner();
    else
        p = players.value(localPlayerId);
    p->updateCardMenu(static_cast<CardItem *>(card));
}

void TabGame::createMenuItems()
{
    aNextPhase = new QAction(this);
    connect(aNextPhase, SIGNAL(triggered()), this, SLOT(actNextPhase()));
    aNextTurn = new QAction(this);
    connect(aNextTurn, SIGNAL(triggered()), this, SLOT(actNextTurn()));
    aRemoveLocalArrows = new QAction(this);
    connect(aRemoveLocalArrows, SIGNAL(triggered()), this, SLOT(actRemoveLocalArrows()));
    aRotateViewCW = new QAction(this);
    connect(aRotateViewCW, SIGNAL(triggered()), this, SLOT(actRotateViewCW()));
    aRotateViewCCW = new QAction(this);
    connect(aRotateViewCCW, SIGNAL(triggered()), this, SLOT(actRotateViewCCW()));
    aGameInfo = new QAction(this);
    connect(aGameInfo, SIGNAL(triggered()), this, SLOT(actGameInfo()));
    aConcede = new QAction(this);
    connect(aConcede, SIGNAL(triggered()), this, SLOT(actConcede()));
    aLeaveGame = new QAction(this);
    connect(aLeaveGame, SIGNAL(triggered()), this, SLOT(actLeaveGame()));
    aCloseReplay = 0;

    phasesMenu = new QMenu(this);
    for (int i = 0; i < phasesToolbar->phaseCount(); ++i) {
        QAction *temp = new QAction(QString(), this);
        connect(temp, SIGNAL(triggered()), this, SLOT(actPhaseAction()));
        phasesMenu->addAction(temp);
        phaseActions.append(temp);
    }

    phasesMenu->addSeparator();
    phasesMenu->addAction(aNextPhase);

    gameMenu = new QMenu(this);
    playersSeparator = gameMenu->addSeparator();
    gameMenu->addMenu(phasesMenu);
    gameMenu->addAction(aNextTurn);
    gameMenu->addSeparator();
    gameMenu->addAction(aRemoveLocalArrows);
    gameMenu->addAction(aRotateViewCW);
    gameMenu->addAction(aRotateViewCCW);
    gameMenu->addSeparator();
    gameMenu->addAction(aGameInfo);
    gameMenu->addAction(aConcede);
    gameMenu->addAction(aLeaveGame);
    addTabMenu(gameMenu);
}

void TabGame::createReplayMenuItems()
{
    aNextPhase = 0;
    aNextTurn = 0;
    aRemoveLocalArrows = 0;
    aRotateViewCW = 0;
    aRotateViewCCW = 0;
    aResetLayout = 0;
    aGameInfo = 0;
    aConcede = 0;
    aLeaveGame = 0;
    aCloseReplay = new QAction(this);
    connect(aCloseReplay, SIGNAL(triggered()), this, SLOT(actLeaveGame()));

    phasesMenu = 0;
    gameMenu = new QMenu(this);
    gameMenu->addAction(aCloseReplay);
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
    connect(aCardInfoDockVisible,SIGNAL(triggered()),this,SLOT(dockVisibleTriggered()));
    aCardInfoDockFloating = cardInfoDockMenu->addAction(QString());
    aCardInfoDockFloating->setCheckable(true);
    connect(aCardInfoDockFloating,SIGNAL(triggered()),this,SLOT(dockFloatingTriggered()));

    aMessageLayoutDockVisible = messageLayoutDockMenu->addAction(QString());
    aMessageLayoutDockVisible->setCheckable(true);
    connect(aMessageLayoutDockVisible,SIGNAL(triggered()),this,SLOT(dockVisibleTriggered()));
    aMessageLayoutDockFloating = messageLayoutDockMenu->addAction(QString());
    aMessageLayoutDockFloating->setCheckable(true);
    connect(aMessageLayoutDockFloating,SIGNAL(triggered()),this,SLOT(dockFloatingTriggered()));

    aPlayerListDockVisible = playerListDockMenu->addAction(QString());
    aPlayerListDockVisible->setCheckable(true);
    connect(aPlayerListDockVisible,SIGNAL(triggered()),this,SLOT(dockVisibleTriggered()));
    aPlayerListDockFloating = playerListDockMenu->addAction(QString());
    aPlayerListDockFloating->setCheckable(true);
    connect(aPlayerListDockFloating,SIGNAL(triggered()),this,SLOT(dockFloatingTriggered()));

    if(replayDock)
    {
        replayDockMenu = viewMenu->addMenu(QString());

        aReplayDockVisible = replayDockMenu->addAction(QString());
        aReplayDockVisible->setCheckable(true);
        connect(aReplayDockVisible,SIGNAL(triggered()),this,SLOT(dockVisibleTriggered()));
        aReplayDockFloating = replayDockMenu->addAction(QString());
        aReplayDockFloating->setCheckable(true);
        connect(aReplayDockFloating,SIGNAL(triggered()),this,SLOT(dockFloatingTriggered()));
    }

    viewMenu->addSeparator();

    aResetLayout = viewMenu->addAction(QString());
    connect(aResetLayout,SIGNAL(triggered()),this,SLOT(actResetLayout()));
    viewMenu->addAction(aResetLayout);

    addTabMenu(viewMenu);
}

void TabGame::loadLayout()
{
    if(replayDock)
    {
        restoreGeometry(settingsCache->layouts().getReplayPlayAreaGeometry());
        restoreState(settingsCache->layouts().getReplayPlayAreaLayoutState());

        cardInfoDock->setMinimumSize(settingsCache->layouts().getReplayCardInfoSize());
        cardInfoDock->setMaximumSize(settingsCache->layouts().getReplayCardInfoSize());
        messageLayoutDock->setMinimumSize(settingsCache->layouts().getReplayMessageLayoutSize());
        messageLayoutDock->setMaximumSize(settingsCache->layouts().getReplayMessageLayoutSize());
        playerListDock->setMinimumSize(settingsCache->layouts().getReplayPlayerListSize());
        playerListDock->setMaximumSize(settingsCache->layouts().getReplayPlayerListSize());
        replayDock->setMinimumSize(settingsCache->layouts().getReplayReplaySize());
        replayDock->setMaximumSize(settingsCache->layouts().getReplayReplaySize());
    } else {
        restoreGeometry(settingsCache->layouts().getGamePlayAreaGeometry());
        restoreState(settingsCache->layouts().getGamePlayAreaLayoutState());

        cardInfoDock->setMinimumSize(settingsCache->layouts().getGameCardInfoSize());
        cardInfoDock->setMaximumSize(settingsCache->layouts().getGameCardInfoSize());
        messageLayoutDock->setMinimumSize(settingsCache->layouts().getGameMessageLayoutSize());
        messageLayoutDock->setMaximumSize(settingsCache->layouts().getGameMessageLayoutSize());
        playerListDock->setMinimumSize(settingsCache->layouts().getGamePlayerListSize());
        playerListDock->setMaximumSize(settingsCache->layouts().getGamePlayerListSize());
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

    if(replayDock)
    {
        aReplayDockVisible->setChecked(replayDock->isVisible());
        aReplayDockFloating->setEnabled(aReplayDockVisible->isChecked());
        aReplayDockFloating->setChecked(replayDock->isFloating());
    }

    QTimer::singleShot(100, this, SLOT(freeDocksSize()));
}

void TabGame::freeDocksSize()
{
    cardInfoDock->setMinimumSize(100, 100);
    cardInfoDock->setMaximumSize(5000, 5000);

    messageLayoutDock->setMinimumSize(100, 100);
    messageLayoutDock->setMaximumSize(5000, 5000);

    playerListDock->setMinimumSize(100,100);
    playerListDock->setMaximumSize(5000,5000);

    if(replayDock)
    {
        replayDock->setMinimumSize(100,100);
        replayDock->setMaximumSize(5000,5000);
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

    if(replayDock)
    {
        replayDock->setVisible(true);
        replayDock->setFloating(false);
        addDockWidget(Qt::BottomDockWidgetArea, replayDock);
        aReplayDockVisible->setChecked(true);
        aReplayDockFloating->setChecked(false);

        cardInfoDock->setMinimumSize(250,360);
        cardInfoDock->setMaximumSize(250,360);
        messageLayoutDock->setMinimumSize(250,200);
        messageLayoutDock->setMaximumSize(250,200);
        playerListDock->setMinimumSize(250,50);
        playerListDock->setMaximumSize(250,50);
        replayDock->setMinimumSize(900,100);
        replayDock->setMaximumSize(900,100);
    } else {
        cardInfoDock->setMinimumSize(250,360);
        cardInfoDock->setMaximumSize(250,360);
        messageLayoutDock->setMinimumSize(250,250);
        messageLayoutDock->setMaximumSize(250,250);
        playerListDock->setMinimumSize(250,50);
        playerListDock->setMaximumSize(250,50);
    }

    QTimer::singleShot(100, this, SLOT(freeDocksSize()));
}

void TabGame::createPlayAreaWidget(bool bReplay)
{
    phasesToolbar = new PhasesToolbar;
    if(!bReplay)
        connect(phasesToolbar, SIGNAL(sendGameCommand(const ::google::protobuf::Message &, int)), this, SLOT(sendGameCommand(const ::google::protobuf::Message &, int)));
    scene = new GameScene(phasesToolbar, this);
    gameView = new GameView(scene);

    gamePlayAreaVBox = new QVBoxLayout;
    gamePlayAreaVBox->setContentsMargins(0,0,0,0);
    gamePlayAreaVBox->addWidget(gameView);

    gamePlayAreaWidget = new QWidget;
    gamePlayAreaWidget->setObjectName("gamePlayAreaWidget");
    gamePlayAreaWidget->setLayout(gamePlayAreaVBox);
}

void TabGame::createReplayDock()
{
    timelineWidget = new ReplayTimelineWidget;
    timelineWidget->setTimeline(replayTimeline);
    connect(timelineWidget, SIGNAL(processNextEvent()), this, SLOT(replayNextEvent()));
    connect(timelineWidget, SIGNAL(replayFinished()), this, SLOT(replayFinished()));

    replayStartButton = new QToolButton;
    replayStartButton->setIconSize(QSize(32, 32));
    replayStartButton->setIcon(QPixmap("theme:replay/start"));
    connect(replayStartButton, SIGNAL(clicked()), this, SLOT(replayStartButtonClicked()));
    replayPauseButton = new QToolButton;
    replayPauseButton->setIconSize(QSize(32, 32));
    replayPauseButton->setEnabled(false);
    replayPauseButton->setIcon(QPixmap("theme:replay/pause"));
    connect(replayPauseButton, SIGNAL(clicked()), this, SLOT(replayPauseButtonClicked()));
    replayFastForwardButton = new QToolButton;
    replayFastForwardButton->setIconSize(QSize(32, 32));
    replayFastForwardButton->setEnabled(false);
    replayFastForwardButton->setIcon(QPixmap("theme:replay/fastforward"));
    replayFastForwardButton->setCheckable(true);
    connect(replayFastForwardButton, SIGNAL(toggled(bool)), this, SLOT(replayFastForwardButtonToggled(bool)));

    replayControlLayout = new QHBoxLayout;
    replayControlLayout->addWidget(timelineWidget, 10);
    replayControlLayout->addWidget(replayStartButton);
    replayControlLayout->addWidget(replayPauseButton);
    replayControlLayout->addWidget(replayFastForwardButton);

    replayControlWidget = new QWidget();
    replayControlWidget->setObjectName("replayControlWidget");
    replayControlWidget->setLayout(replayControlLayout);

    replayDock = new QDockWidget(this);
    replayDock->setObjectName("replayDock");
    replayDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
    replayDock->setWidget(replayControlWidget);
    replayDock->setFloating(false);

    replayDock->installEventFilter(this);
    connect(replayDock, SIGNAL(topLevelChanged(bool)), this, SLOT(dockTopLevelChanged(bool)));
}

void TabGame::createDeckViewContainerWidget(bool bReplay)
{
    Q_UNUSED(bReplay);

    deckViewContainerWidget = new QWidget();
    deckViewContainerWidget->setObjectName("deckViewContainerWidget");
    deckViewContainerLayout = new QVBoxLayout;
    deckViewContainerLayout->setContentsMargins(0,0,0,0);
    deckViewContainerWidget->setLayout(deckViewContainerLayout);
}

void TabGame::createCardInfoDock(bool bReplay)
{
    Q_UNUSED(bReplay);

    cardInfo = new CardFrame();
    cardHInfoLayout = new QHBoxLayout;
    cardVInfoLayout = new QVBoxLayout;
    cardVInfoLayout->setContentsMargins(0, 0, 0, 0);
    cardVInfoLayout->addWidget(cardInfo);
    cardVInfoLayout->addLayout(cardHInfoLayout);

    cardBoxLayoutWidget = new QWidget;
    cardBoxLayoutWidget->setLayout(cardVInfoLayout);

    cardInfoDock = new QDockWidget(this);
    cardInfoDock->setObjectName("cardInfoDock");
    cardInfoDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
    cardInfoDock->setWidget(cardBoxLayoutWidget);
    cardInfoDock->setFloating(false);

    cardInfoDock->installEventFilter(this);
    connect(cardInfoDock, SIGNAL(topLevelChanged(bool)), this, SLOT(dockTopLevelChanged(bool)));
}

void TabGame::createPlayerListDock(bool bReplay)
{
    if(bReplay)
    {
        playerListWidget = new PlayerListWidget(0, 0, this);
    } else {
        playerListWidget = new PlayerListWidget(tabSupervisor, clients.first(), this);
        connect(playerListWidget, SIGNAL(openMessageDialog(QString, bool)), this, SIGNAL(openMessageDialog(QString, bool)));
    }
    playerListWidget->setFocusPolicy(Qt::NoFocus);

    playerListDock = new QDockWidget(this);
    playerListDock->setObjectName("playerListDock");
    playerListDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
    playerListDock->setWidget(playerListWidget);
    playerListDock->setFloating(false);

    playerListDock->installEventFilter(this);
    connect(playerListDock, SIGNAL(topLevelChanged(bool)), this, SLOT(dockTopLevelChanged(bool)));
}

void TabGame::createMessageDock(bool bReplay)
{
    messageLog = new MessageLogWidget(tabSupervisor, this);
    connect(messageLog, SIGNAL(cardNameHovered(QString)), cardInfo, SLOT(setCard(QString)));
    connect(messageLog, SIGNAL(showCardInfoPopup(QPoint, QString)), this, SLOT(showCardInfoPopup(QPoint, QString)));
    connect(messageLog, SIGNAL(deleteCardInfoPopup(QString)), this, SLOT(deleteCardInfoPopup(QString)));

    if(!bReplay)
    {
        connect(messageLog, SIGNAL(openMessageDialog(QString, bool)), this, SIGNAL(openMessageDialog(QString, bool)));
        connect(messageLog, SIGNAL(addMentionTag(QString)), this, SLOT(addMentionTag(QString)));
        connect(settingsCache, SIGNAL(chatMentionCompleterChanged()), this, SLOT(actCompleterChanged()));

        timeElapsedLabel = new QLabel;
        timeElapsedLabel->setAlignment(Qt::AlignCenter);
        gameTimer = new QTimer(this);
        gameTimer->setInterval(1000);
        connect(gameTimer, SIGNAL(timeout()), this, SLOT(incrementGameTime()));
        gameTimer->start();

        sayLabel = new QLabel;
        sayEdit = new LineEditCompleter;
        sayLabel->setBuddy(sayEdit);
        completer = new QCompleter(autocompleteUserList, sayEdit);
        completer->setCaseSensitivity(Qt::CaseInsensitive);
        completer->setMaxVisibleItems(5);
        completer->setFilterMode(Qt::MatchStartsWith);

        sayEdit->setCompleter(completer);
        actCompleterChanged();

        if (spectator && !gameInfo.spectators_can_chat() && tabSupervisor->getAdminLocked()) {
            sayLabel->hide();
            sayEdit->hide();
        }
        connect(tabSupervisor, SIGNAL(adminLockChanged(bool)), this, SLOT(adminLockChanged(bool)));
        connect(sayEdit, SIGNAL(returnPressed()), this, SLOT(actSay()));

        sayHLayout = new QHBoxLayout;
        sayHLayout->addWidget(sayLabel);
        sayHLayout->addWidget(sayEdit);

    }

    messageLogLayout = new QVBoxLayout;
    messageLogLayout->setContentsMargins(0, 0, 0, 0);
    if(!bReplay)
        messageLogLayout->addWidget(timeElapsedLabel);
    messageLogLayout->addWidget(messageLog);
    if(!bReplay)
        messageLogLayout->addLayout(sayHLayout);

    messageLogLayoutWidget = new QWidget;
    messageLogLayoutWidget->setLayout(messageLogLayout);

    messageLayoutDock = new QDockWidget(this);
    messageLayoutDock->setObjectName("messageLayoutDock");
    messageLayoutDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
    messageLayoutDock->setWidget(messageLogLayoutWidget);
    messageLayoutDock->setFloating(false);

    messageLayoutDock->installEventFilter(this);
    connect(messageLayoutDock, SIGNAL(topLevelChanged(bool)), this, SLOT(dockTopLevelChanged(bool)));
}

// Method uses to sync docks state with menu items state
bool TabGame::eventFilter(QObject * o, QEvent * e)
{
    if(e->type() == QEvent::Close)
    {
        if(o == cardInfoDock)
        {
            aCardInfoDockVisible->setChecked(false);
            aCardInfoDockFloating->setEnabled(false);
        } else if(o == messageLayoutDock) {
            aMessageLayoutDockVisible->setChecked(false);
            aMessageLayoutDockFloating->setEnabled(false);
        } else if(o == playerListDock) {
            aPlayerListDockVisible->setChecked(false);
            aPlayerListDockFloating->setEnabled(false);
        } else if(o == replayDock) {
            aReplayDockVisible->setChecked(false);
            aReplayDockFloating->setEnabled(false);
        }
    }

    if( o == this && e->type() == QEvent::Hide){
        if(replay)
        {
            settingsCache->layouts().setReplayPlayAreaState(saveState());
            settingsCache->layouts().setReplayPlayAreaGeometry(saveGeometry());
            settingsCache->layouts().setReplayCardInfoSize(cardInfoDock->size());
            settingsCache->layouts().setReplayMessageLayoutSize(messageLayoutDock->size());
            settingsCache->layouts().setReplayPlayerListSize(playerListDock->size());
            settingsCache->layouts().setReplayReplaySize(replayDock->size());
        } else {
            settingsCache->layouts().setGamePlayAreaState(saveState());
            settingsCache->layouts().setGamePlayAreaGeometry(saveGeometry());
            settingsCache->layouts().setGameCardInfoSize(cardInfoDock->size());
            settingsCache->layouts().setGameMessageLayoutSize(messageLayoutDock->size());
            settingsCache->layouts().setGamePlayerListSize(playerListDock->size());
        }
    }
    return false;
}

void TabGame::dockVisibleTriggered()
{
    QObject *o = sender();
    if(o == aCardInfoDockVisible)
    {
        cardInfoDock->setVisible(aCardInfoDockVisible->isChecked());
        aCardInfoDockFloating->setEnabled(aCardInfoDockVisible->isChecked());
        return;
    }

    if(o == aMessageLayoutDockVisible)
    {
        messageLayoutDock->setVisible(aMessageLayoutDockVisible->isChecked());
        aMessageLayoutDockFloating->setEnabled(aMessageLayoutDockVisible->isChecked());
        return;
    }

    if(o == aPlayerListDockVisible)
    {
        playerListDock->setVisible(aPlayerListDockVisible->isChecked());
        aPlayerListDockFloating->setEnabled(aPlayerListDockVisible->isChecked());
        return;
    }

    if(o == aReplayDockVisible)
    {
        replayDock->setVisible(aReplayDockVisible->isChecked());
        aReplayDockFloating->setEnabled(aReplayDockVisible->isChecked());
        return;
    }
}

void TabGame::dockFloatingTriggered()
{
    QObject *o = sender();
    if(o == aCardInfoDockFloating)
    {
        cardInfoDock->setFloating(aCardInfoDockFloating->isChecked());
        return;
    }

    if(o == aMessageLayoutDockFloating)
    {
        messageLayoutDock->setFloating(aMessageLayoutDockFloating->isChecked());
        return;
    }

    if(o == aPlayerListDockFloating)
    {
        playerListDock->setFloating(aPlayerListDockFloating->isChecked());
        return;
    }

    if(o == aReplayDockFloating)
    {
        replayDock->setFloating(aReplayDockFloating->isChecked());
        return;
    }
}

void TabGame::dockTopLevelChanged(bool topLevel)
{
    retranslateUi();

    QObject *o = sender();
    if(o == cardInfoDock)
    {
        aCardInfoDockFloating->setChecked(topLevel);
        return;
    }

    if(o == messageLayoutDock)
    {
        aMessageLayoutDockFloating->setChecked(topLevel);
        return;
    }

    if(o == playerListDock)
    {
        aPlayerListDockFloating->setChecked(topLevel);
        return;
    }

    if(o == replayDock)
    {
        aReplayDockFloating->setChecked(topLevel);
        return;
    }
}
