#include <QtGui>
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
#include "deck_picturecacher.h"
#include "protocol_items.h"
#include "dlg_load_remote_deck.h"
#include "client.h"
#include "carditem.h"
#include "arrowitem.h"
#include "main.h"

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

TabGame::TabGame(Client *_client, int _gameId, const QString &_gameDescription, int _localPlayerId, bool _spectator, bool _spectatorsCanTalk, bool _spectatorsSeeEverything, bool _resuming)
	: Tab(), client(_client), gameId(_gameId), gameDescription(_gameDescription), localPlayerId(_localPlayerId), spectator(_spectator), spectatorsCanTalk(_spectatorsCanTalk), spectatorsSeeEverything(_spectatorsSeeEverything), started(false), resuming(_resuming), currentPhase(-1)
{
	scene = new GameScene(this);
	gameView = new GameView(scene);
	gameView->hide();
	
	loadLocalButton = new QPushButton;
	loadRemoteButton = new QPushButton;
	readyStartButton = new ReadyStartButton;
	readyStartButton->setEnabled(false);
	
	QHBoxLayout *buttonHBox = new QHBoxLayout;
	buttonHBox->addWidget(loadLocalButton);
	buttonHBox->addWidget(loadRemoteButton);
	buttonHBox->addWidget(readyStartButton);
	buttonHBox->addStretch();
	deckView = new DeckView;
	connect(deckView, SIGNAL(newCardAdded(AbstractCardItem *)), this, SLOT(newCardAdded(AbstractCardItem *)));
	connect(deckView, SIGNAL(sideboardPlanChanged()), this, SLOT(sideboardPlanChanged()));
	QVBoxLayout *deckViewLayout = new QVBoxLayout;
	deckViewLayout->addLayout(buttonHBox);
	deckViewLayout->addWidget(deckView);
	deckViewContainer = new QWidget;
	deckViewContainer->setLayout(deckViewLayout);

	cardInfo = new CardInfoWidget;
	playerListWidget = new PlayerListWidget;
	playerListWidget->setFocusPolicy(Qt::NoFocus);
	messageLog = new MessageLogWidget;
	sayLabel = new QLabel;
	sayEdit = new QLineEdit;
	sayLabel->setBuddy(sayEdit);

	QHBoxLayout *hLayout = new QHBoxLayout;
	hLayout->addWidget(sayLabel);
	hLayout->addWidget(sayEdit);

	phasesToolbar = new PhasesToolbar;
	phasesToolbar->hide();
	connect(phasesToolbar, SIGNAL(sendGameCommand(GameCommand *)), this, SLOT(sendGameCommand(GameCommand *)));
	
	QVBoxLayout *verticalLayout = new QVBoxLayout;
	verticalLayout->addWidget(cardInfo);
	verticalLayout->addWidget(playerListWidget, 1);
	verticalLayout->addWidget(messageLog, 3);
	verticalLayout->addLayout(hLayout);

	QHBoxLayout *mainLayout = new QHBoxLayout;
	mainLayout->addWidget(phasesToolbar);
	mainLayout->addWidget(gameView, 10);
	mainLayout->addWidget(deckViewContainer, 10);
	mainLayout->addLayout(verticalLayout);

	if (spectator) {
		if (!spectatorsCanTalk) {
			sayLabel->hide();
			sayEdit->hide();
		}
		loadLocalButton->hide();
		loadRemoteButton->hide();
		readyStartButton->hide();
	}

	connect(loadLocalButton, SIGNAL(clicked()), this, SLOT(loadLocalDeck()));
	connect(loadRemoteButton, SIGNAL(clicked()), this, SLOT(loadRemoteDeck()));
	connect(readyStartButton, SIGNAL(clicked()), this, SLOT(readyStart()));
	
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
	
	tabMenu = new QMenu(this);
	playersSeparator = tabMenu->addSeparator();
	tabMenu->addAction(aNextPhase);
	tabMenu->addAction(aNextTurn);
	tabMenu->addSeparator();
	tabMenu->addAction(aRemoveLocalArrows);
	tabMenu->addSeparator();
	tabMenu->addAction(aConcede);
	tabMenu->addAction(aLeaveGame);
	
	retranslateUi();
	setLayout(mainLayout);
	
	messageLog->logGameJoined(gameId);
}

TabGame::~TabGame()
{
	QMapIterator<int, Player *> i(players);
	while (i.hasNext())
		delete i.next().value();
	players.clear();
	
	emit gameClosing(this);
}

void TabGame::retranslateUi()
{
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
	
	loadLocalButton->setText(tr("Load &local deck"));
	loadRemoteButton->setText(tr("Load d&eck from server"));
	readyStartButton->setText(tr("Ready to s&tart"));
	sayLabel->setText(tr("&Say:"));
	cardInfo->retranslateUi();

	QMapIterator<int, Player *> i(players);
	while (i.hasNext())
		i.next().value()->retranslateUi();
	
	scene->retranslateUi();
}

void TabGame::actConcede()
{
	if (QMessageBox::question(this, tr("Concede"), tr("Are you sure you want to concede this game?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
		return;

	sendGameCommand(new Command_Concede);
}

void TabGame::actLeaveGame()
{
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

Player *TabGame::addPlayer(int playerId, const QString &playerName)
{
	Player *newPlayer = new Player(playerName, playerId, playerId == localPlayerId, client, this);
	scene->addPlayer(newPlayer);

	connect(newPlayer, SIGNAL(newCardAdded(AbstractCardItem *)), this, SLOT(newCardAdded(AbstractCardItem *)));
	messageLog->connectToPlayer(newPlayer);

	tabMenu->insertMenu(playersSeparator, newPlayer->getPlayerMenu());
	
	players.insert(playerId, newPlayer);

	return newPlayer;
}

void TabGame::processGameEventContainer(GameEventContainer *cont)
{
	const QList<GameEvent *> &eventList = cont->getEventList();
	GameEventContext *context = cont->getContext();
	for (int i = 0; i < eventList.size(); ++i) {
		GameEvent *event = eventList[i];
		
		if (spectators.contains(event->getPlayerId())) {
			switch (event->getItemId()) {
				case ItemId_Event_Say: eventSpectatorSay(qobject_cast<Event_Say *>(event), context); break;
				case ItemId_Event_Leave: eventSpectatorLeave(qobject_cast<Event_Leave *>(event), context); break;
				default: {
					qDebug() << "unhandled spectator game event";
					break;
				}
			}
		} else switch (event->getItemId()) {
			case ItemId_Event_GameStateChanged: eventGameStateChanged(qobject_cast<Event_GameStateChanged *>(event), context); break;
			case ItemId_Event_PlayerPropertiesChanged: eventPlayerPropertiesChanged(qobject_cast<Event_PlayerPropertiesChanged *>(event), context); break;
			case ItemId_Event_Join: eventJoin(qobject_cast<Event_Join *>(event), context); break;
			case ItemId_Event_Leave: eventLeave(qobject_cast<Event_Leave *>(event), context); break;
			case ItemId_Event_GameClosed: eventGameClosed(qobject_cast<Event_GameClosed *>(event), context); break;
			case ItemId_Event_SetActivePlayer: eventSetActivePlayer(qobject_cast<Event_SetActivePlayer *>(event), context); break;
			case ItemId_Event_SetActivePhase: eventSetActivePhase(qobject_cast<Event_SetActivePhase *>(event), context); break;
			case ItemId_Event_Ping: eventPing(qobject_cast<Event_Ping *>(event), context); break;
	
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

void TabGame::sendGameCommand(GameCommand *command)
{
	command->setGameId(gameId);
	client->sendCommand(command);
}

void TabGame::sendCommandContainer(CommandContainer *cont)
{
	const QList<Command *> &cmdList = cont->getCommandList();
	for (int i = 0; i < cmdList.size(); ++i) {
		GameCommand *cmd = qobject_cast<GameCommand *>(cmdList[i]);
		if (cmd)
			cmd->setGameId(gameId);
	}
	client->sendCommandContainer(cont);
}

void TabGame::startGame()
{
	currentPhase = -1;

	readyStartButton->setReadyStart(false);
	playerListWidget->setGameStarted(true);
	started = true;
	deckViewContainer->hide();
	gameView->show();
	phasesToolbar->show();
}

void TabGame::stopGame()
{
	currentPhase = -1;

	playerListWidget->setActivePlayer(-1);
	playerListWidget->setGameStarted(false);
	started = false;
	gameView->hide();
	phasesToolbar->hide();
	deckViewContainer->show();
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
				spectators.insert(prop->getPlayerId(), prop->getName());
				playerListWidget->addPlayer(prop);
			}
		} else {
			Player *player = players.value(prop->getPlayerId(), 0);
			if (!player) {
				player = addPlayer(prop->getPlayerId(), prop->getName());
				playerListWidget->addPlayer(prop);
			}
			player->processPlayerInfo(pl);
			if (player->getLocal() && pl->getDeck()) {
				Deck_PictureCacher::cachePictures(pl->getDeck(), this);
				deckView->setDeck(new DeckList(pl->getDeck()));
				readyStartButton->setEnabled(true);
			}
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
			if (player->getLocal()) {
				readyStartButton->setReadyStart(ready);
				deckView->setLocked(ready);
			}
			if (ready)
				messageLog->logReadyStart(player);
			else
				messageLog->logNotReadyStart(player);
			break;
		}
		case ItemId_Context_Concede: messageLog->logConcede(player); break;
		case ItemId_Context_DeckSelect: messageLog->logDeckSelect(player, static_cast<Context_DeckSelect *>(context)->getDeckId()); break;
		default: ;
	}
}

void TabGame::eventJoin(Event_Join *event, GameEventContext * /*context*/)
{
	ServerInfo_PlayerProperties *playerInfo = event->getPlayer();
	if (playerInfo->getSpectator()) {
		spectators.insert(playerInfo->getPlayerId(), playerInfo->getName());
		messageLog->logJoinSpectator(playerInfo->getName());
		playerListWidget->addPlayer(playerInfo);
	} else {
		Player *newPlayer = addPlayer(playerInfo->getPlayerId(), playerInfo->getName());
		messageLog->logJoin(newPlayer);
		playerListWidget->addPlayer(playerInfo);
	}
	emit userEvent();
}

void TabGame::eventLeave(Event_Leave *event, GameEventContext * /*context*/)
{
	int playerId = event->getPlayerId();

	Player *player = players.value(playerId, 0);
	if (player) {
		messageLog->logLeave(player);
		playerListWidget->removePlayer(playerId);
		players.remove(playerId);
		delete player;
	}
	emit userEvent();
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
	playerListWidget->setActivePlayer(id);
	QMapIterator<int, Player *> i(players);
	while (i.hasNext()) {
		i.next();
		i.value()->setActive(i.value() == player);
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
}

void TabGame::loadLocalDeck()
{
	QFileDialog dialog(this, tr("Load deck"));
	QSettings settings;
	dialog.setDirectory(settings.value("paths/decks").toString());
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
	
	Command_DeckSelect *cmd = new Command_DeckSelect(gameId, deck, -1);
	connect(cmd, SIGNAL(finished(ProtocolResponse *)), this, SLOT(deckSelectFinished(ProtocolResponse *)));
	client->sendCommand(cmd);
}

void TabGame::loadRemoteDeck()
{
	DlgLoadRemoteDeck dlg(client);
	if (dlg.exec()) {
		Command_DeckSelect *cmd = new Command_DeckSelect(gameId, 0, dlg.getDeckId());
		connect(cmd, SIGNAL(finished(ProtocolResponse *)), this, SLOT(deckSelectFinished(ProtocolResponse *)));
		client->sendCommand(cmd);
	}
}

void TabGame::deckSelectFinished(ProtocolResponse *r)
{
	Response_DeckDownload *resp = qobject_cast<Response_DeckDownload *>(r);
	if (!resp)
		return;
	
	Deck_PictureCacher::cachePictures(resp->getDeck(), this);
	deckView->setDeck(new DeckList(resp->getDeck()));
	readyStartButton->setEnabled(true);
}

void TabGame::readyStart()
{
	client->sendCommand(new Command_ReadyStart(gameId, !readyStartButton->getReadyStart()));
}

void TabGame::newCardAdded(AbstractCardItem *card)
{
	connect(card, SIGNAL(hovered(AbstractCardItem *)), cardInfo, SLOT(setCard(AbstractCardItem *)));
}

void TabGame::sideboardPlanChanged()
{
	QList<MoveCardToZone *> newPlan = deckView->getSideboardPlan();
	client->sendCommand(new Command_SetSideboardPlan(gameId, newPlan));
}
