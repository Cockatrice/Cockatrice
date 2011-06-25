#include <QLabel>
#include <QHBoxLayout>
#include <QSplitter>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QFileDialog>
#include "tab_game.h"
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
#include "protocol_items.h"
#include "dlg_load_remote_deck.h"
#include "abstractclient.h"
#include "carditem.h"
#include "arrowitem.h"
#include "main.h"
#include "settingscache.h"
#include "carddatabase.h"

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

DeckViewContainer::DeckViewContainer(AbstractClient *_client, TabGame *parent)
	: QWidget(parent), client(_client)
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
	
	Command_DeckSelect *cmd = new Command_DeckSelect(static_cast<TabGame *>(parent())->getGameId(), deck, -1);
	connect(cmd, SIGNAL(finished(ProtocolResponse *)), this, SLOT(deckSelectFinished(ProtocolResponse *)));
	client->sendCommand(cmd);
}

void DeckViewContainer::loadRemoteDeck()
{
	DlgLoadRemoteDeck dlg(client);
	if (dlg.exec()) {
		Command_DeckSelect *cmd = new Command_DeckSelect(static_cast<TabGame *>(parent())->getGameId(), 0, dlg.getDeckId());
		connect(cmd, SIGNAL(finished(ProtocolResponse *)), this, SLOT(deckSelectFinished(ProtocolResponse *)));
		client->sendCommand(cmd);
	}
}

void DeckViewContainer::deckSelectFinished(ProtocolResponse *r)
{
	Response_DeckDownload *resp = qobject_cast<Response_DeckDownload *>(r);
	if (!resp)
		return;
	
	db->cacheCardPixmaps(resp->getDeck()->getCardList());
	deckView->setDeck(new DeckList(resp->getDeck()));
	readyStartButton->setEnabled(true);
}

void DeckViewContainer::readyStart()
{
	client->sendCommand(new Command_ReadyStart(static_cast<TabGame *>(parent())->getGameId(), !readyStartButton->getReadyStart()));
}

void DeckViewContainer::sideboardPlanChanged()
{
	QList<MoveCardToZone *> newPlan = deckView->getSideboardPlan();
	client->sendCommand(new Command_SetSideboardPlan(static_cast<TabGame *>(parent())->getGameId(), newPlan));
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

TabGame::TabGame(TabSupervisor *_tabSupervisor, QList<AbstractClient *> &_clients, int _gameId, const QString &_gameDescription, int _localPlayerId, const QString &_userName, bool _spectator, bool _spectatorsCanTalk, bool _spectatorsSeeEverything, bool _resuming)
	: Tab(_tabSupervisor), clients(_clients), gameId(_gameId), gameDescription(_gameDescription), localPlayerId(_localPlayerId), spectator(_spectator), spectatorsCanTalk(_spectatorsCanTalk), spectatorsSeeEverything(_spectatorsSeeEverything), started(false), resuming(_resuming), currentPhase(-1)
{
	phasesToolbar = new PhasesToolbar;
	phasesToolbar->hide();
	connect(phasesToolbar, SIGNAL(sendGameCommand(GameCommand *, int)), this, SLOT(sendGameCommand(GameCommand *, int)));
	
	scene = new GameScene(phasesToolbar, this);
	gameView = new GameView(scene);
	gameView->hide();
	
	cardInfo = new CardInfoWidget(CardInfoWidget::ModeGameTab);
	playerListWidget = new PlayerListWidget(tabSupervisor, clients.first(), this, true);
	playerListWidget->setFocusPolicy(Qt::NoFocus);
	connect(playerListWidget, SIGNAL(openMessageDialog(QString, bool)), this, SIGNAL(openMessageDialog(QString, bool)));
	
	timeElapsedLabel = new QLabel;
	timeElapsedLabel->setAlignment(Qt::AlignCenter);
	messageLog = new MessageLogWidget(_userName);
	connect(messageLog, SIGNAL(cardNameHovered(QString)), cardInfo, SLOT(setCard(QString)));
	connect(messageLog, SIGNAL(showCardInfoPopup(QPoint, QString)), this, SLOT(showCardInfoPopup(QPoint, QString)));
	connect(messageLog, SIGNAL(deleteCardInfoPopup()), this, SLOT(deleteCardInfoPopup()));
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

	if (spectator && !spectatorsCanTalk) {
		sayLabel->hide();
		sayEdit->hide();
	}

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
	for (int i = 0; i < phaseActions.size(); ++i)
		phaseActions[i]->setText(phasesToolbar->getLongPhaseName(i));
	phasesMenu->setTitle(tr("&Phases"));
	
	tabMenu->setTitle(tr("&Game"));
	aNextPhase->setText(tr("Next &phase"));
	aNextPhase->setShortcut(tr("Ctrl+Space"));
	aNextTurn->setText(tr("Next &turn"));
	aNextTurn->setShortcuts(QList<QKeySequence>() << QKeySequence(tr("Ctrl+Return")) << QKeySequence(tr("Ctrl+Enter")));
	aRemoveLocalArrows->setText(tr("&Remove all local arrows"));
	aRemoveLocalArrows->setShortcut(tr("Ctrl+R"));
	aConcede->setText(tr("&Concede"));
	aConcede->setShortcut(tr("F2"));
	aLeaveGame->setText(tr("&Leave game"));
	aLeaveGame->setShortcut(tr("Ctrl+Q"));
	
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

void TabGame::actConcede()
{
	if (QMessageBox::question(this, tr("Concede"), tr("Are you sure you want to concede this game?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
		return;

	sendGameCommand(new Command_Concede);
}

void TabGame::actLeaveGame()
{
	if (!spectator)
		if (QMessageBox::question(this, tr("Leave game"), tr("Are you sure you want to leave this game?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
			return;

	sendGameCommand(new Command_LeaveGame);
	deleteLater();
}

void TabGame::actSay()
{
	if (!sayEdit->text().isEmpty()) {
		sendGameCommand(new Command_Say(-1, sayEdit->text()));
		sayEdit->clear();
	}
}

void TabGame::actPhaseAction()
{
	int phase = phaseActions.indexOf(static_cast<QAction *>(sender()));
	emit sendGameCommand(new Command_SetActivePhase(-1, phase), -1);
}

void TabGame::actNextPhase()
{
	int phase = currentPhase;
	if (++phase >= phasesToolbar->phaseCount())
		phase = 0;
	sendGameCommand(new Command_SetActivePhase(-1, phase));
}

void TabGame::actNextTurn()
{
	sendGameCommand(new Command_NextTurn);
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
			sendGameCommand(new Command_DeleteArrow(-1, a->getId()));
		}
	}
}

Player *TabGame::addPlayer(int playerId, ServerInfo_User *info)
{
	bool local = ((clients.size() > 1) || (playerId == localPlayerId));
	Player *newPlayer = new Player(info, playerId, local, this);
	scene->addPlayer(newPlayer);

	connect(newPlayer, SIGNAL(newCardAdded(AbstractCardItem *)), this, SLOT(newCardAdded(AbstractCardItem *)));
	messageLog->connectToPlayer(newPlayer);
	
	if (local && !spectator) {
		AbstractClient *client;
		if (clients.size() > 1)
			client = clients.at(playerId);
		else {
			client = clients.first();
			newPlayer->setShortcutsActive();
		}
		DeckViewContainer *deckView = new DeckViewContainer(client, this);
		connect(deckView, SIGNAL(newCardAdded(AbstractCardItem *)), this, SLOT(newCardAdded(AbstractCardItem *)));
		deckViewContainers.insert(playerId, deckView);
		deckViewContainerLayout->addWidget(deckView);
	}

	tabMenu->insertMenu(playersSeparator, newPlayer->getPlayerMenu());
	
	players.insert(playerId, newPlayer);
	emit playerAdded(newPlayer);

	return newPlayer;
}

void TabGame::processGameEventContainer(GameEventContainer *cont, AbstractClient *client)
{
	const QList<GameEvent *> &eventList = cont->getEventList();
	GameEventContext *context = cont->getContext();
	messageLog->containerProcessingStarted(context);
	for (int i = 0; i < eventList.size(); ++i) {
		GameEvent *event = eventList[i];
		
		if (spectators.contains(event->getPlayerId())) {
			switch (event->getItemId()) {
				case ItemId_Event_Say: eventSpectatorSay(static_cast<Event_Say *>(event), context); break;
				case ItemId_Event_Leave: eventSpectatorLeave(static_cast<Event_Leave *>(event), context); break;
				default: {
					qDebug() << "unhandled spectator game event";
					break;
				}
			}
		} else {
			if ((clients.size() > 1) && (event->getPlayerId() != -1))
				if (clients.at(event->getPlayerId()) != client)
					continue;
				
			switch (event->getItemId()) {
				case ItemId_Event_GameStateChanged: eventGameStateChanged(static_cast<Event_GameStateChanged *>(event), context); break;
				case ItemId_Event_PlayerPropertiesChanged: eventPlayerPropertiesChanged(static_cast<Event_PlayerPropertiesChanged *>(event), context); break;
				case ItemId_Event_Join: eventJoin(static_cast<Event_Join *>(event), context); break;
				case ItemId_Event_Leave: eventLeave(static_cast<Event_Leave *>(event), context); break;
				case ItemId_Event_Kicked: eventKicked(static_cast<Event_Kicked *>(event), context); break;
				case ItemId_Event_GameClosed: eventGameClosed(static_cast<Event_GameClosed *>(event), context); break;
				case ItemId_Event_SetActivePlayer: eventSetActivePlayer(static_cast<Event_SetActivePlayer *>(event), context); break;
				case ItemId_Event_SetActivePhase: eventSetActivePhase(static_cast<Event_SetActivePhase *>(event), context); break;
				case ItemId_Event_Ping: eventPing(static_cast<Event_Ping *>(event), context); break;
		
				default: {
					Player *player = players.value(event->getPlayerId(), 0);
					if (!player) {
						qDebug() << "unhandled game event: invalid player id";
						break;
					}
					player->processGameEvent(event, context);
					emit userEvent();
				}
			}
		}
	}
	messageLog->containerProcessingDone();
}

void TabGame::sendGameCommand(GameCommand *command, int playerId)
{
	command->setGameId(gameId);
	AbstractClient *client;
	if (clients.size() > 1) {
		if (playerId == -1)
			playerId = getActiveLocalPlayer()->getId();

		client = clients.at(playerId);
	} else
		client = clients.first();
	client->sendCommand(command);
}

void TabGame::sendCommandContainer(CommandContainer *cont, int playerId)
{
	const QList<Command *> &cmdList = cont->getCommandList();
	for (int i = 0; i < cmdList.size(); ++i) {
		GameCommand *cmd = qobject_cast<GameCommand *>(cmdList[i]);
		if (cmd)
			cmd->setGameId(gameId);
	}

	AbstractClient *client;
	if (clients.size() > 1) {
		if (playerId == -1)
			playerId = getActiveLocalPlayer()->getId();
		client = clients.at(playerId);
	} else
		client = clients.first();
	client->sendCommandContainer(cont);
}

void TabGame::startGame()
{
	currentPhase = -1;

	QMapIterator<int, DeckViewContainer *> i(deckViewContainers);
	while (i.hasNext()) {
		i.next();
		i.value()->setReadyStart(false);
		i.value()->hide();
	}
	mainLayout->removeItem(deckViewContainerLayout);
	
	QMapIterator<int, Player *> playerIterator(players);
	while (playerIterator.hasNext())
		playerIterator.next().value()->setConceded(false);
	
	playerListWidget->setGameStarted(true);
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
	playerListWidget->setGameStarted(false);
	started = false;
	gameView->hide();
	phasesToolbar->hide();
}

void TabGame::eventSpectatorSay(Event_Say *event, GameEventContext * /*context*/)
{
	messageLog->logSpectatorSay(spectators.value(event->getPlayerId()), event->getMessage());
}

void TabGame::eventSpectatorLeave(Event_Leave *event, GameEventContext * /*context*/)
{
	int playerId = event->getPlayerId();
	messageLog->logLeaveSpectator(spectators.value(playerId));
	playerListWidget->removePlayer(playerId);
	spectators.remove(playerId);
	
	emit userEvent();
}

void TabGame::eventGameStateChanged(Event_GameStateChanged *event, GameEventContext * /*context*/)
{
	const QList<ServerInfo_Player *> &plList = event->getPlayerList();
	for (int i = 0; i < plList.size(); ++i) {
		ServerInfo_Player *pl = plList[i];
		ServerInfo_PlayerProperties *prop = pl->getProperties();
		if (prop->getSpectator()) {
			if (!spectators.contains(prop->getPlayerId())) {
				spectators.insert(prop->getPlayerId(), prop->getUserInfo()->getName());
				playerListWidget->addPlayer(prop);
			}
		} else {
			Player *player = players.value(prop->getPlayerId(), 0);
			if (!player) {
				player = addPlayer(prop->getPlayerId(), prop->getUserInfo());
				playerListWidget->addPlayer(prop);
			}
			player->processPlayerInfo(pl);
			if (player->getLocal() && !pl->getDeck()->isEmpty()) {
				db->cacheCardPixmaps(pl->getDeck()->getCardList());
				deckViewContainers.value(player->getId())->setDeck(new DeckList(pl->getDeck()));
			}
		}
	}
	for (int i = 0; i < plList.size(); ++i) {
		ServerInfo_Player *pl = plList[i];
		ServerInfo_PlayerProperties *prop = pl->getProperties();
		if (!prop->getSpectator()) {
			Player *player = players.value(prop->getPlayerId(), 0);
			if (!player)
				continue;
			player->processCardAttachment(pl);
		}
	}
	if (event->getGameStarted() && !started) {
		startGame();
		if (!resuming)
			messageLog->logGameStart();
		setActivePlayer(event->getActivePlayer());
		setActivePhase(event->getActivePhase());
	} else if (!event->getGameStarted() && started) {
		stopGame();
		scene->clearViews();
	}
	emit userEvent();
}

void TabGame::eventPlayerPropertiesChanged(Event_PlayerPropertiesChanged *event, GameEventContext *context)
{
	Player *player = players.value(event->getProperties()->getPlayerId(), 0);
	if (!player)
		return;
	playerListWidget->updatePlayerProperties(event->getProperties());
	if (context) switch (context->getItemId()) {
		case ItemId_Context_ReadyStart: {
			bool ready = event->getProperties()->getReadyStart();
			if (player->getLocal())
				deckViewContainers.value(player->getId())->setReadyStart(ready);
			if (ready)
				messageLog->logReadyStart(player);
			else
				messageLog->logNotReadyStart(player);
			break;
		}
		case ItemId_Context_Concede: {
			messageLog->logConcede(player);
			player->setConceded(true);
			
			QMapIterator<int, Player *> playerIterator(players);
			while (playerIterator.hasNext())
				playerIterator.next().value()->updateZones();
			
			break;
		}
		case ItemId_Context_DeckSelect: messageLog->logDeckSelect(player, static_cast<Context_DeckSelect *>(context)->getDeckId()); break;
		default: ;
	}
}

void TabGame::eventJoin(Event_Join *event, GameEventContext * /*context*/)
{
	ServerInfo_PlayerProperties *playerInfo = event->getPlayer();
	if (players.contains(playerInfo->getPlayerId()))
		return;
	if (playerInfo->getSpectator()) {
		spectators.insert(playerInfo->getPlayerId(), playerInfo->getUserInfo()->getName());
		messageLog->logJoinSpectator(playerInfo->getUserInfo()->getName());
		playerListWidget->addPlayer(playerInfo);
	} else {
		Player *newPlayer = addPlayer(playerInfo->getPlayerId(), playerInfo->getUserInfo());
		messageLog->logJoin(newPlayer);
		playerListWidget->addPlayer(playerInfo);
	}
	emit userEvent();
}

void TabGame::eventLeave(Event_Leave *event, GameEventContext * /*context*/)
{
	int playerId = event->getPlayerId();

	Player *player = players.value(playerId, 0);
	if (!player)
		return;
	
	messageLog->logLeave(player);
	playerListWidget->removePlayer(playerId);
	players.remove(playerId);
	emit playerRemoved(player);
	player->clear();
	player->deleteLater();
	
	// Rearrange all remaining zones so that attachment relationship updates take place
	QMapIterator<int, Player *> playerIterator(players);
	while (playerIterator.hasNext())
		playerIterator.next().value()->updateZones();
	
	emit userEvent();
}

void TabGame::eventKicked(Event_Kicked * /*event*/, GameEventContext * /*context*/)
{
	emit userEvent();
	QMessageBox::critical(this, tr("Kicked"), tr("You have been kicked out of the game."));
	deleteLater();
}

void TabGame::eventGameClosed(Event_GameClosed * /*event*/, GameEventContext * /*context*/)
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

void TabGame::eventSetActivePlayer(Event_SetActivePlayer *event, GameEventContext * /*context*/)
{
	Player *player = setActivePlayer(event->getActivePlayerId());
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

void TabGame::eventSetActivePhase(Event_SetActivePhase *event, GameEventContext * /*context*/)
{
	int phase = event->getPhase();
	if (currentPhase != phase)
		messageLog->logSetActivePhase(phase);
	setActivePhase(phase);
	emit userEvent();
}

void TabGame::eventPing(Event_Ping *event, GameEventContext * /*context*/)
{
	const QList<ServerInfo_PlayerPing *> &pingList = event->getPingList();
	for (int i = 0; i < pingList.size(); ++i)
		playerListWidget->updatePing(pingList[i]->getPlayerId(), pingList[i]->getPingTime());
	
	int seconds = event->getSecondsElapsed();
	int minutes = seconds / 60;
	seconds -= minutes * 60;
	int hours = minutes / 60;
	minutes -= hours * 60;
	timeElapsedLabel->setText(QString::number(hours).rightJustified(2, '0') + ":" + QString::number(minutes).rightJustified(2, '0') + ":" + QString::number(seconds).rightJustified(2, '0'));
}

void TabGame::newCardAdded(AbstractCardItem *card)
{
	connect(card, SIGNAL(hovered(AbstractCardItem *)), cardInfo, SLOT(setCard(AbstractCardItem *)));
	connect(card, SIGNAL(showCardInfoPopup(QPoint, QString)), this, SLOT(showCardInfoPopup(QPoint, QString)));
	connect(card, SIGNAL(deleteCardInfoPopup()), this, SLOT(deleteCardInfoPopup()));
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
