#include <QLabel>
#include <QHBoxLayout>
#include <QSplitter>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QFileDialog>
#include <QTimer>
#include <QToolButton>

#include "tab_game.h"
#include "tab_supervisor.h"
#include "cardinfowidget.h"
#include "playerlistwidget.h"
#include "messagelogwidget.h"
#include "phasestoolbar.h"
#include "gameview.h"
#include "gamescene.h"
#include "player.h"
#include "zoneviewzone.h"
#include "zoneviewwidget.h"
#include "deckview.h"
#include "decklist.h"
#include "dlg_load_remote_deck.h"
#include "abstractclient.h"
#include "carditem.h"
#include "arrowitem.h"
#include "main.h"
#include "settingscache.h"
#include "carddatabase.h"
#include "replay_timeline_widget.h"

#include <google/protobuf/descriptor.h>
#include "pending_command.h"
#include "pb/game_replay.pb.h"
#include "pb/command_concede.pb.h"
#include "pb/command_deck_select.pb.h"
#include "pb/command_ready_start.pb.h"
#include "pb/command_set_sideboard_plan.pb.h"
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

ReadyStartButton::ReadyStartButton(QWidget *parent)
	: QPushButton(parent), readyStart(false)
{
}

void ReadyStartButton::paintEvent(QPaintEvent *event)
{
	QPushButton::paintEvent(event);
	
	QPainter painter(this);
	if (readyStart)
		painter.setPen(QPen(Qt::green, 3));
	else
		painter.setPen(QPen(Qt::red, 3));
	painter.drawRect(1.5, 1.5, width() - 3, height() - 3);
}

void ReadyStartButton::setReadyStart(bool _readyStart)
{
	readyStart = _readyStart;
	update();
}

DeckViewContainer::DeckViewContainer(int _playerId, TabGame *parent)
	: QWidget(parent), playerId(_playerId)
{
	loadLocalButton = new QPushButton;
	loadRemoteButton = new QPushButton;
	readyStartButton = new ReadyStartButton;
	readyStartButton->setEnabled(false);
	
	connect(loadLocalButton, SIGNAL(clicked()), this, SLOT(loadLocalDeck()));
	connect(loadRemoteButton, SIGNAL(clicked()), this, SLOT(loadRemoteDeck()));
	connect(readyStartButton, SIGNAL(clicked()), this, SLOT(readyStart()));
	
	QHBoxLayout *buttonHBox = new QHBoxLayout;
	buttonHBox->addWidget(loadLocalButton);
	buttonHBox->addWidget(loadRemoteButton);
	buttonHBox->addWidget(readyStartButton);
	buttonHBox->addStretch();
	deckView = new DeckView;
	connect(deckView, SIGNAL(newCardAdded(AbstractCardItem *)), this, SIGNAL(newCardAdded(AbstractCardItem *)));
	connect(deckView, SIGNAL(sideboardPlanChanged()), this, SLOT(sideboardPlanChanged()));
	
	QVBoxLayout *deckViewLayout = new QVBoxLayout;
	deckViewLayout->addLayout(buttonHBox);
	deckViewLayout->addWidget(deckView);
	setLayout(deckViewLayout);
	
	retranslateUi();
}

void DeckViewContainer::retranslateUi()
{
	loadLocalButton->setText(tr("Load &local deck"));
	loadRemoteButton->setText(tr("Load d&eck from server"));
	readyStartButton->setText(tr("Ready to s&tart"));
}

void DeckViewContainer::setButtonsVisible(bool _visible)
{
	loadLocalButton->setVisible(_visible);
	loadRemoteButton->setVisible(_visible);
	readyStartButton->setVisible(_visible);
}

void DeckViewContainer::loadLocalDeck()
{
	QFileDialog dialog(this, tr("Load deck"));
	dialog.setDirectory(settingsCache->getDeckPath());
	dialog.setNameFilters(DeckList::fileNameFilters);
	if (!dialog.exec())
		return;

	QString fileName = dialog.selectedFiles().at(0);
	DeckList::FileFormat fmt = DeckList::getFormatFromNameFilter(dialog.selectedNameFilter());
	DeckList *deck = new DeckList;
	if (!deck->loadFromFile(fileName, fmt)) {
		delete deck;
		// Error message
		return;
	}
	
	Command_DeckSelect cmd;
	cmd.set_deck(deck->writeToString_Native().toStdString());
	PendingCommand *pend = static_cast<TabGame *>(parent())->prepareGameCommand(cmd);
	connect(pend, SIGNAL(finished(const Response &)), this, SLOT(deckSelectFinished(const Response &)));
	static_cast<TabGame *>(parent())->sendGameCommand(pend, playerId);
}

void DeckViewContainer::loadRemoteDeck()
{
	DlgLoadRemoteDeck dlg(static_cast<TabGame *>(parent())->getClientForPlayer(playerId));
	if (dlg.exec()) {
		Command_DeckSelect cmd;
		cmd.set_deck_id(dlg.getDeckId());
		PendingCommand *pend = static_cast<TabGame *>(parent())->prepareGameCommand(cmd);
		connect(pend, SIGNAL(finished(const Response &)), this, SLOT(deckSelectFinished(const Response &)));
		static_cast<TabGame *>(parent())->sendGameCommand(pend, playerId);
	}
}

void DeckViewContainer::deckSelectFinished(const Response &r)
{
	const Response_DeckDownload &resp = r.GetExtension(Response_DeckDownload::ext);
	DeckList *newDeck = new DeckList(QString::fromStdString(resp.deck()));
	db->cacheCardPixmaps(newDeck->getCardList());
	deckView->setDeck(newDeck);
	readyStartButton->setEnabled(true);
}

void DeckViewContainer::readyStart()
{
	Command_ReadyStart cmd;
	cmd.set_ready(!readyStartButton->getReadyStart());
	static_cast<TabGame *>(parent())->sendGameCommand(cmd, playerId);
}

void DeckViewContainer::sideboardPlanChanged()
{
	Command_SetSideboardPlan cmd;
	const QList<MoveCard_ToZone> &newPlan = deckView->getSideboardPlan();
	for (int i = 0; i < newPlan.size(); ++i)
		cmd.add_move_list()->CopyFrom(newPlan[i]);
	static_cast<TabGame *>(parent())->sendGameCommand(cmd, playerId);
}

void DeckViewContainer::setReadyStart(bool ready)
{
	readyStartButton->setReadyStart(ready);
	deckView->setLocked(ready);
}

void DeckViewContainer::setDeck(DeckList *deck)
{
	deckView->setDeck(deck);
	readyStartButton->setEnabled(true);
}

TabGame::TabGame(GameReplay *_replay)
	: Tab(0),
	hostId(-1),
	localPlayerId(-1),
	spectator(true),
	spectatorsCanTalk(false),
	spectatorsSeeEverything(true),
	gameStateKnown(false),
	started(false),
	resuming(false),
	currentPhase(-1),
	replay(_replay),
	currentReplayStep(0)
{
	setAttribute(Qt::WA_DeleteOnClose);
	
	gameId = replay->game_info().game_id();
	gameDescription = QString::fromStdString(replay->game_info().description());
	
	// Create list: event number -> time [ms]
	// Distribute simultaneous events evenly across 1 second.
	int lastEventTimestamp = -1;
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
	
	phasesToolbar = new PhasesToolbar;
	phasesToolbar->hide();
	
	scene = new GameScene(phasesToolbar, this);
	gameView = new GameView(scene);
	gameView->hide();
	
	cardInfo = new CardInfoWidget(CardInfoWidget::ModeGameTab);
	playerListWidget = new PlayerListWidget(0, 0, this);
	playerListWidget->setFocusPolicy(Qt::NoFocus);
	
	timeElapsedLabel = new QLabel;
	timeElapsedLabel->setAlignment(Qt::AlignCenter);
	messageLog = new MessageLogWidget(QString(), false);
	connect(messageLog, SIGNAL(cardNameHovered(QString)), cardInfo, SLOT(setCard(QString)));
	connect(messageLog, SIGNAL(showCardInfoPopup(QPoint, QString)), this, SLOT(showCardInfoPopup(QPoint, QString)));
	connect(messageLog, SIGNAL(deleteCardInfoPopup(QString)), this, SLOT(deleteCardInfoPopup(QString)));
	sayLabel = 0;

	deckViewContainerLayout = new QVBoxLayout;

	QVBoxLayout *messageLogLayout = new QVBoxLayout;
	messageLogLayout->addWidget(timeElapsedLabel);
	messageLogLayout->addWidget(messageLog);
	
	QWidget *messageLogLayoutWidget = new QWidget;
	messageLogLayoutWidget->setLayout(messageLogLayout);
	
	timelineWidget = new ReplayTimelineWidget;
	timelineWidget->setTimeline(replayTimeline);
	connect(timelineWidget, SIGNAL(processNextEvent()), this, SLOT(replayNextEvent()));
	connect(timelineWidget, SIGNAL(replayFinished()), this, SLOT(replayFinished()));
	
	replayToStartButton = new QToolButton;
	replayToStartButton->setIconSize(QSize(32, 32));
	replayToStartButton->setIcon(QIcon(":/resources/replay_tostart.svg"));
	connect(replayToStartButton, SIGNAL(clicked()), this, SLOT(replayToStartButtonClicked()));
	replayStartButton = new QToolButton;
	replayStartButton->setIconSize(QSize(32, 32));
	replayStartButton->setIcon(QIcon(":/resources/replay_start.svg"));
	connect(replayStartButton, SIGNAL(clicked()), this, SLOT(replayStartButtonClicked()));
	replayPauseButton = new QToolButton;
	replayPauseButton->setIconSize(QSize(32, 32));
	replayPauseButton->setEnabled(false);
	replayPauseButton->setIcon(QIcon(":/resources/replay_pause.svg"));
	connect(replayPauseButton, SIGNAL(clicked()), this, SLOT(replayPauseButtonClicked()));
	replayStopButton = new QToolButton;
	replayStopButton->setIconSize(QSize(32, 32));
	replayStopButton->setEnabled(false);
	replayStopButton->setIcon(QIcon(":/resources/replay_stop.svg"));
	connect(replayStopButton, SIGNAL(clicked()), this, SLOT(replayStopButtonClicked()));
	replayFastForwardButton = new QToolButton;
	replayFastForwardButton->setIconSize(QSize(32, 32));
	replayFastForwardButton->setEnabled(false);
	replayFastForwardButton->setIcon(QIcon(":/resources/replay_fastforward.svg"));
	replayFastForwardButton->setCheckable(true);
	connect(replayFastForwardButton, SIGNAL(toggled(bool)), this, SLOT(replayFastForwardButtonToggled(bool)));
	replayToEndButton = new QToolButton;
	replayToEndButton->setIconSize(QSize(32, 32));
	replayToEndButton->setIcon(QIcon(":/resources/replay_toend.svg"));
	connect(replayStopButton, SIGNAL(clicked()), this, SLOT(replayToEndButtonClicked()));
	
	splitter = new QSplitter(Qt::Vertical);
	splitter->addWidget(cardInfo);
	splitter->addWidget(playerListWidget);
	splitter->addWidget(messageLogLayoutWidget);

	mainLayout = new QHBoxLayout;
	mainLayout->addWidget(gameView, 10);
	mainLayout->addLayout(deckViewContainerLayout, 10);
	mainLayout->addWidget(splitter);
	
	QHBoxLayout *replayControlLayout = new QHBoxLayout;
	replayControlLayout->addWidget(timelineWidget, 10);
	replayControlLayout->addWidget(replayToStartButton);
	replayControlLayout->addWidget(replayStartButton);
	replayControlLayout->addWidget(replayPauseButton);
	replayControlLayout->addWidget(replayStopButton);
	replayControlLayout->addWidget(replayFastForwardButton);
	replayControlLayout->addWidget(replayToEndButton);
	
	QVBoxLayout *superMainLayout = new QVBoxLayout;
	superMainLayout->addLayout(mainLayout);
	superMainLayout->addLayout(replayControlLayout);
	
	aNextPhase = 0;
	aNextTurn = 0;
	aRemoveLocalArrows = 0;
	aConcede = 0;
	aLeaveGame = 0;
	aCloseReplay = new QAction(this);
	connect(aCloseReplay, SIGNAL(triggered()), this, SLOT(actLeaveGame()));
	
	phasesMenu = 0;
	tabMenu = new QMenu(this);
	tabMenu->addAction(aCloseReplay);
	
	retranslateUi();
	setLayout(superMainLayout);

	splitter->restoreState(settingsCache->getTabGameSplitterSizes());
	
	messageLog->logReplayStarted(gameId);
}

TabGame::TabGame(TabSupervisor *_tabSupervisor, QList<AbstractClient *> &_clients, const Event_GameJoined &event)
	: Tab(_tabSupervisor),
	clients(_clients),
	gameId(event.game_id()),
	gameDescription(QString::fromStdString(event.game_description())),
	hostId(event.host_id()),
	localPlayerId(event.player_id()),
	spectator(event.spectator()),
	spectatorsCanTalk(event.spectators_can_talk()),
	spectatorsSeeEverything(event.spectators_see_everything()),
	gameStateKnown(true),
	started(false),
	resuming(event.resuming()),
	currentPhase(-1),
	replay(0)
{
	gameTimer = new QTimer(this);
	gameTimer->setInterval(1000);
	connect(gameTimer, SIGNAL(timeout()), this, SLOT(incrementGameTime()));
	gameTimer->start();
	
	phasesToolbar = new PhasesToolbar;
	phasesToolbar->hide();
	connect(phasesToolbar, SIGNAL(sendGameCommand(const ::google::protobuf::Message &, int)), this, SLOT(sendGameCommand(const ::google::protobuf::Message &, int)));
	
	scene = new GameScene(phasesToolbar, this);
	gameView = new GameView(scene);
	gameView->hide();
	
	cardInfo = new CardInfoWidget(CardInfoWidget::ModeGameTab);
	playerListWidget = new PlayerListWidget(tabSupervisor, clients.first(), this);
	playerListWidget->setFocusPolicy(Qt::NoFocus);
	connect(playerListWidget, SIGNAL(openMessageDialog(QString, bool)), this, SIGNAL(openMessageDialog(QString, bool)));
	
	timeElapsedLabel = new QLabel;
	timeElapsedLabel->setAlignment(Qt::AlignCenter);
	messageLog = new MessageLogWidget(QString::fromStdString(tabSupervisor->getUserInfo()->name()), tabSupervisor->getUserInfo()->gender() == ServerInfo_User::Female);
	connect(messageLog, SIGNAL(cardNameHovered(QString)), cardInfo, SLOT(setCard(QString)));
	connect(messageLog, SIGNAL(showCardInfoPopup(QPoint, QString)), this, SLOT(showCardInfoPopup(QPoint, QString)));
	connect(messageLog, SIGNAL(deleteCardInfoPopup(QString)), this, SLOT(deleteCardInfoPopup(QString)));
	sayLabel = new QLabel;
	sayEdit = new QLineEdit;
	sayLabel->setBuddy(sayEdit);

	QHBoxLayout *hLayout = new QHBoxLayout;
	hLayout->addWidget(sayLabel);
	hLayout->addWidget(sayEdit);
	
	deckViewContainerLayout = new QVBoxLayout;

	QVBoxLayout *messageLogLayout = new QVBoxLayout;
	messageLogLayout->addWidget(timeElapsedLabel);
	messageLogLayout->addWidget(messageLog);
	messageLogLayout->addLayout(hLayout);
	
	QWidget *messageLogLayoutWidget = new QWidget;
	messageLogLayoutWidget->setLayout(messageLogLayout);
	
	splitter = new QSplitter(Qt::Vertical);
	splitter->addWidget(cardInfo);
	splitter->addWidget(playerListWidget);
	splitter->addWidget(messageLogLayoutWidget);

	mainLayout = new QHBoxLayout;
	mainLayout->addWidget(gameView, 10);
	mainLayout->addLayout(deckViewContainerLayout, 10);
	mainLayout->addWidget(splitter);
	
	if (spectator && !spectatorsCanTalk && tabSupervisor->getAdminLocked()) {
		sayLabel->hide();
		sayEdit->hide();
	}
	connect(tabSupervisor, SIGNAL(adminLockChanged(bool)), this, SLOT(adminLockChanged(bool)));
	connect(sayEdit, SIGNAL(returnPressed()), this, SLOT(actSay()));

	// Menu actions
	aNextPhase = new QAction(this);
	connect(aNextPhase, SIGNAL(triggered()), this, SLOT(actNextPhase()));
	aNextTurn = new QAction(this);
	connect(aNextTurn, SIGNAL(triggered()), this, SLOT(actNextTurn()));
	aRemoveLocalArrows = new QAction(this);
	connect(aRemoveLocalArrows, SIGNAL(triggered()), this, SLOT(actRemoveLocalArrows()));
	aConcede = new QAction(this);
	connect(aConcede, SIGNAL(triggered()), this, SLOT(actConcede()));
	aLeaveGame = new QAction(this);
	connect(aLeaveGame, SIGNAL(triggered()), this, SLOT(actLeaveGame()));
	aCloseReplay = 0;
	
	phasesMenu = new QMenu(this);
	for (int i = 0; i < phasesToolbar->phaseCount(); ++i) {
		QAction *temp = new QAction(QString(), this);
		connect(temp, SIGNAL(triggered()), this, SLOT(actPhaseAction()));
		switch (i) {
			case 0: temp->setShortcut(tr("F5")); break;
			case 2: temp->setShortcut(tr("F6")); break;
			case 3: temp->setShortcut(tr("F7")); break;
			case 4: temp->setShortcut(tr("F8")); break;
			case 9: temp->setShortcut(tr("F9")); break;
			case 10: temp->setShortcut(tr("F10")); break;
			default: ;
		}
		phasesMenu->addAction(temp);
		phaseActions.append(temp);
	}
	phasesMenu->addSeparator();
	phasesMenu->addAction(aNextPhase);
	
	tabMenu = new QMenu(this);
	playersSeparator = tabMenu->addSeparator();
	tabMenu->addMenu(phasesMenu);
	tabMenu->addAction(aNextTurn);
	tabMenu->addSeparator();
	tabMenu->addAction(aRemoveLocalArrows);
	tabMenu->addSeparator();
	tabMenu->addAction(aConcede);
	tabMenu->addAction(aLeaveGame);
	
	retranslateUi();
	setLayout(mainLayout);

	splitter->restoreState(settingsCache->getTabGameSplitterSizes());
	
	messageLog->logGameJoined(gameId);
}

TabGame::~TabGame()
{
	delete replay;
	settingsCache->setTabGameSplitterSizes(splitter->saveState());

	QMapIterator<int, Player *> i(players);
	while (i.hasNext())
		delete i.next().value();
	players.clear();
	
	delete deckViewContainerLayout;
	
	emit gameClosing(this);
}

void TabGame::retranslateUi()
{
	if (phasesMenu) {
		for (int i = 0; i < phaseActions.size(); ++i)
			phaseActions[i]->setText(phasesToolbar->getLongPhaseName(i));
		phasesMenu->setTitle(tr("&Phases"));
	}
	
	tabMenu->setTitle(tr("&Game"));
	if (aNextPhase) {
		aNextPhase->setText(tr("Next &phase"));
		aNextPhase->setShortcut(tr("Ctrl+Space"));
	}
	if (aNextTurn) {
		aNextTurn->setText(tr("Next &turn"));
		aNextTurn->setShortcuts(QList<QKeySequence>() << QKeySequence(tr("Ctrl+Return")) << QKeySequence(tr("Ctrl+Enter")));
	}
	if (aRemoveLocalArrows) {
		aRemoveLocalArrows->setText(tr("&Remove all local arrows"));
		aRemoveLocalArrows->setShortcut(tr("Ctrl+R"));
	}
	if (aConcede) {
		aConcede->setText(tr("&Concede"));
		aConcede->setShortcut(tr("F2"));
	}
	if (aLeaveGame) {
		aLeaveGame->setText(tr("&Leave game"));
		aLeaveGame->setShortcut(tr("Ctrl+Q"));
	}
	if (aCloseReplay) {
		aCloseReplay->setText(tr("C&lose replay"));
		aCloseReplay->setShortcut(tr("Ctrl+Q"));
	}
	
	if (sayLabel)
		sayLabel->setText(tr("&Say:"));
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
	replayStopButton->setEnabled(false);
	replayFastForwardButton->setEnabled(false);
}

void TabGame::replayToStartButtonClicked()
{
	// XXX
}

void TabGame::replayStartButtonClicked()
{
	replayStartButton->setEnabled(false);
	replayPauseButton->setEnabled(true);
	replayStopButton->setEnabled(true);
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

void TabGame::replayStopButtonClicked()
{
	replayStartButton->setEnabled(true);
	replayPauseButton->setEnabled(false);
	replayStopButton->setEnabled(false);
	replayFastForwardButton->setEnabled(false);
	
	timelineWidget->stopReplay();
	// XXX to start
}

void TabGame::replayFastForwardButtonToggled(bool checked)
{
	timelineWidget->setTimeScaleFactor(checked ? 10.0 : 1.0);
}

void TabGame::replayToEndButtonClicked()
{
	// XXX
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
	bool v = !(spectator && !spectatorsCanTalk && lock);
	sayLabel->setVisible(v);
	sayEdit->setVisible(v);
}

void TabGame::actConcede()
{
	if (QMessageBox::question(this, tr("Concede"), tr("Are you sure you want to concede this game?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
		return;

	sendGameCommand(Command_Concede());
}

void TabGame::actLeaveGame()
{
	if (!spectator)
		if (QMessageBox::question(this, tr("Leave game"), tr("Are you sure you want to leave this game?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
			return;
	
	if (!replay)
		sendGameCommand(Command_LeaveGame());
	deleteLater();
}

void TabGame::actSay()
{
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

Player *TabGame::addPlayer(int playerId, const ServerInfo_User &info)
{
	bool local = ((clients.size() > 1) || (playerId == localPlayerId));
	Player *newPlayer = new Player(info, playerId, local, this);
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

	tabMenu->insertMenu(playersSeparator, newPlayer->getPlayerMenu());
	
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
					emit userEvent();
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
	} else
		return clients.first();
}

void TabGame::sendGameCommand(PendingCommand *pend, int playerId)
{
	getClientForPlayer(playerId)->sendCommand(pend);
}

void TabGame::sendGameCommand(const google::protobuf::Message &command, int playerId)
{
	AbstractClient *client = getClientForPlayer(playerId);
	client->sendCommand(prepareGameCommand(command));
}

PendingCommand *TabGame::prepareGameCommand(const ::google::protobuf::Message &cmd)
{
	CommandContainer cont;
	cont.set_game_id(gameId);
	GameCommand *c = cont.add_game_command();
	c->GetReflection()->MutableMessage(c, cmd.GetDescriptor()->FindExtensionByName("ext"))->CopyFrom(cmd);
	return new PendingCommand(cont);
}

PendingCommand *TabGame::prepareGameCommand(const QList< const ::google::protobuf::Message * > &cmdList)
{
	CommandContainer cont;
	cont.set_game_id(gameId);
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
	mainLayout->removeItem(deckViewContainerLayout);
	
	if (!resuming) {
		QMapIterator<int, Player *> playerIterator(players);
		while (playerIterator.hasNext())
			playerIterator.next().value()->setConceded(false);
	}

	playerListWidget->setGameStarted(true, resuming);
	started = true;
	static_cast<GameScene *>(gameView->scene())->rearrange();
	gameView->show();
	phasesToolbar->show();
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
	mainLayout->insertLayout(1, deckViewContainerLayout, 10);

	playerListWidget->setActivePlayer(-1);
	playerListWidget->setGameStarted(false, false);
	started = false;
	gameView->hide();
	phasesToolbar->hide();
}

void TabGame::eventSpectatorSay(const Event_GameSay &event, int eventPlayerId, const GameEventContext & /*context*/)
{
	messageLog->logSpectatorSay(spectators.value(eventPlayerId), QString::fromStdString(event.message()));
}

void TabGame::eventSpectatorLeave(const Event_Leave & /*event*/, int eventPlayerId, const GameEventContext & /*context*/)
{
	messageLog->logLeaveSpectator(spectators.value(eventPlayerId));
	playerListWidget->removePlayer(eventPlayerId);
	spectators.remove(eventPlayerId);
	
	emit userEvent();
}

void TabGame::eventGameStateChanged(const Event_GameStateChanged &event, int /*eventPlayerId*/, const GameEventContext & /*context*/)
{
	const int playerListSize = event.player_list_size();
	for (int i = 0; i < playerListSize; ++i) {
		const ServerInfo_Player &playerInfo = event.player_list(i);
		const ServerInfo_PlayerProperties &prop = playerInfo.properties();
		const int playerId = prop.player_id();
		if (prop.spectator()) {
			if (!spectators.contains(playerId)) {
				spectators.insert(playerId, QString::fromStdString(prop.user_info().name()));
				playerListWidget->addPlayer(prop);
			}
		} else {
			Player *player = players.value(playerId, 0);
			if (!player) {
				player = addPlayer(playerId, prop.user_info());
				playerListWidget->addPlayer(prop);
			}
			player->processPlayerInfo(playerInfo);
			if (player->getLocal() && playerInfo.has_deck_list()) {
				DeckList *newDeck = new DeckList(QString::fromStdString(playerInfo.deck_list()));
				db->cacheCardPixmaps(newDeck->getCardList());
				deckViewContainers.value(playerId)->setDeck(newDeck);
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
	
	if (event.game_started() && !started) {
		startGame(!gameStateKnown);
		if (gameStateKnown)
			messageLog->logGameStart();
		setActivePlayer(event.active_player_id());
		setActivePhase(event.active_phase());
	} else if (!event.game_started() && started) {
		stopGame();
		scene->clearViews();
	}
	gameStateKnown = true;
	emit userEvent();
}

void TabGame::eventPlayerPropertiesChanged(const Event_PlayerPropertiesChanged &event, int eventPlayerId, const GameEventContext &context)
{
	Player *player = players.value(eventPlayerId, 0);
	if (!player)
		return;
	playerListWidget->updatePlayerProperties(event.player_properties(), eventPlayerId);
	
	const GameEventContext::ContextType contextType = static_cast<const GameEventContext::ContextType>(getPbExtension(context));
	switch (contextType) {
		case GameEventContext::READY_START: {
			bool ready = event.player_properties().ready_start();
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
			messageLog->logDeckSelect(player, QString::fromStdString(context.GetExtension(Context_DeckSelect::ext).deck_hash()));
			break;
		}
		case GameEventContext::CONNECTION_STATE_CHANGED: {
			messageLog->logConnectionStateChanged(player, event.player_properties().ping_seconds() != -1);
			break;
		}
		default: ;
	}
}

void TabGame::eventJoin(const Event_Join &event, int /*eventPlayerId*/, const GameEventContext & /*context*/)
{
	const ServerInfo_PlayerProperties &playerInfo = event.player_properties();
	const int playerId = playerInfo.player_id();
	if (players.contains(playerId))
		return;
	if (playerInfo.spectator()) {
		spectators.insert(playerId, QString::fromStdString(playerInfo.user_info().name()));
		messageLog->logJoinSpectator(QString::fromStdString(playerInfo.user_info().name()));
	} else {
		Player *newPlayer = addPlayer(playerId, playerInfo.user_info());
		messageLog->logJoin(newPlayer);
	}
	playerListWidget->addPlayer(playerInfo);
	emit userEvent();
}

void TabGame::eventLeave(const Event_Leave & /*event*/, int eventPlayerId, const GameEventContext & /*context*/)
{
	Player *player = players.value(eventPlayerId, 0);
	if (!player)
		return;
	
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
	
	emit userEvent();
}

void TabGame::eventKicked(const Event_Kicked & /*event*/, int /*eventPlayerId*/, const GameEventContext & /*context*/)
{
	emit userEvent();
	QMessageBox::critical(this, tr("Kicked"), tr("You have been kicked out of the game."));
	deleteLater();
}

void TabGame::eventGameHostChanged(const Event_GameHostChanged & /*event*/, int eventPlayerId, const GameEventContext & /*context*/)
{
	hostId = eventPlayerId;
}

void TabGame::eventGameClosed(const Event_GameClosed & /*event*/, int /*eventPlayerId*/, const GameEventContext & /*context*/)
{
	started = false;
	messageLog->logGameClosed();
	emit userEvent();
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
	emit userEvent();
	return player;
}

void TabGame::eventSetActivePlayer(const Event_SetActivePlayer &event, int /*eventPlayerId*/, const GameEventContext & /*context*/)
{
	Player *player = setActivePlayer(event.active_player_id());
	if (!player)
		return;
	messageLog->logSetActivePlayer(player);
	emit userEvent();
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
	emit userEvent();
}

void TabGame::newCardAdded(AbstractCardItem *card)
{
	connect(card, SIGNAL(hovered(AbstractCardItem *)), cardInfo, SLOT(setCard(AbstractCardItem *)));
	connect(card, SIGNAL(showCardInfoPopup(QPoint, QString)), this, SLOT(showCardInfoPopup(QPoint, QString)));
	connect(card, SIGNAL(deleteCardInfoPopup(QString)), this, SLOT(deleteCardInfoPopup(QString)));
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
	if (replay)
		return tr("Replay %1: %2").arg(gameId).arg(gameDescription);
	else
		return tr("Game %1: %2").arg(gameId).arg(gameDescription);
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
