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
#include "zoneviewlayout.h"
#include "deckview.h"
#include "decklist.h"
#include "deck_picturecacher.h"
#include "protocol_items.h"
#include "dlg_load_remote_deck.h"
#include "client.h"
#include "carditem.h"
#include "arrowitem.h"
#include "main.h"

TabGame::TabGame(Client *_client, int _gameId, int _localPlayerId, bool _spectator)
	: Tab(), client(_client), gameId(_gameId), localPlayerId(_localPlayerId), spectator(_spectator), started(false), currentPhase(-1)
{
	zoneLayout = new ZoneViewLayout;
	scene = new GameScene(zoneLayout, this);
	gameView = new GameView(scene);
	gameView->hide();
	
	loadLocalButton = new QPushButton;
	loadRemoteButton = new QPushButton;
	readyStartButton = new QPushButton;
	
	QHBoxLayout *buttonHBox = new QHBoxLayout;
	buttonHBox->addWidget(loadLocalButton);
	buttonHBox->addWidget(loadRemoteButton);
	buttonHBox->addWidget(readyStartButton);
	buttonHBox->addStretch();
	deckView = new DeckView;
	QVBoxLayout *deckViewLayout = new QVBoxLayout;
	deckViewLayout->addLayout(buttonHBox);
	deckViewLayout->addWidget(deckView);
	deckViewContainer = new QWidget;
	deckViewContainer->setLayout(deckViewLayout);

	cardInfo = new CardInfoWidget;
	playerListWidget = new PlayerListWidget;
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

	aCloseMostRecentZoneView = new QAction(this);
	connect(aCloseMostRecentZoneView, SIGNAL(triggered()), zoneLayout, SLOT(closeMostRecentZoneView()));
	addAction(aCloseMostRecentZoneView);
	
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
	aLeaveGame = new QAction(this);
	connect(aLeaveGame, SIGNAL(triggered()), this, SLOT(actLeaveGame()));
	
	tabMenu = new QMenu(this);
	playersSeparator = tabMenu->addSeparator();
	tabMenu->addAction(aNextPhase);
	tabMenu->addAction(aNextTurn);
	tabMenu->addSeparator();
	tabMenu->addAction(aRemoveLocalArrows);
	tabMenu->addSeparator();
	tabMenu->addAction(aLeaveGame);
	
	retranslateUi();
	setLayout(mainLayout);
	
	messageLog->logGameJoined(gameId);
}

TabGame::~TabGame()
{
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
	aLeaveGame->setText(tr("&Leave game"));
	
	loadLocalButton->setText(tr("Load &local deck"));
	loadRemoteButton->setText(tr("Load deck from &server"));
	readyStartButton->setText(tr("&Start game"));
	sayLabel->setText(tr("&Say:"));
	cardInfo->retranslateUi();
	zoneLayout->retranslateUi();
	aCloseMostRecentZoneView->setText(tr("Close most recent zone view"));
	aCloseMostRecentZoneView->setShortcut(tr("Esc"));

	QMapIterator<int, Player *> i(players);
	while (i.hasNext())
		i.next().value()->retranslateUi();
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

	connect(newPlayer, SIGNAL(newCardAdded(CardItem *)), this, SLOT(newCardAdded(CardItem *)));
	connect(newPlayer, SIGNAL(toggleZoneView(Player *, QString, int)), zoneLayout, SLOT(toggleZoneView(Player *, QString, int)));
	connect(newPlayer, SIGNAL(closeZoneView(ZoneViewZone *)), zoneLayout, SLOT(removeItem(ZoneViewZone *)));
	messageLog->connectToPlayer(newPlayer);

	tabMenu->insertMenu(playersSeparator, newPlayer->getPlayerMenu());
	
	players.insert(playerId, newPlayer);

	return newPlayer;
}

void TabGame::processGameEvent(GameEvent *event)
{
	switch (event->getItemId()) {
		case ItemId_Event_GameStart: eventGameStart(qobject_cast<Event_GameStart *>(event)); break;
		case ItemId_Event_GameStateChanged: eventGameStateChanged(qobject_cast<Event_GameStateChanged *>(event)); break;
		case ItemId_Event_Join: eventJoin(qobject_cast<Event_Join *>(event)); break;
		case ItemId_Event_Leave: eventLeave(qobject_cast<Event_Leave *>(event)); break;
		case ItemId_Event_GameClosed: eventGameClosed(qobject_cast<Event_GameClosed *>(event)); break;
		case ItemId_Event_SetActivePlayer: eventSetActivePlayer(qobject_cast<Event_SetActivePlayer *>(event)); break;
		case ItemId_Event_SetActivePhase: eventSetActivePhase(qobject_cast<Event_SetActivePhase *>(event)); break;
		default: {
			Player *player = players.value(event->getPlayerId(), 0);
			if (!player) {
				qDebug() << "unhandled game event: invalid player id";
				break;
			}
			player->processGameEvent(event);
		}
	}
}

void TabGame::sendGameCommand(GameCommand *command)
{
	command->setGameId(gameId);
	client->sendCommand(command);
}

void TabGame::startGame()
{
	currentPhase = -1;

	started = true;
	deckViewContainer->hide();
	gameView->show();
	phasesToolbar->show();
}

void TabGame::eventGameStart(Event_GameStart * /*event*/)
{
	startGame();
	messageLog->logGameStart();

	QMapIterator<int, Player *> i(players);
	while (i.hasNext())
		i.next().value()->prepareForGame();
}

void TabGame::eventGameStateChanged(Event_GameStateChanged *event)
{
	const QList<ServerInfo_Player *> &plList = event->getPlayerList();
	for (int i = 0; i < plList.size(); ++i) {
		ServerInfo_Player *pl = plList[i];
		Player *player = players.value(pl->getPlayerId(), 0);
		if (!player) {
			player = addPlayer(pl->getPlayerId(), pl->getName());
			playerListWidget->addPlayer(pl);
		}
		player->processPlayerInfo(pl);
	}
	if (event->getGameStarted() && !started) {
		startGame();
		setActivePlayer(event->getActivePlayer());
		setActivePhase(event->getActivePhase());
	}
}

void TabGame::eventJoin(Event_Join *event)
{
	ServerInfo_Player *playerInfo = event->getPlayer();
	if (playerInfo->getSpectator()) {
		spectatorList.append(playerInfo->getName());
		messageLog->logJoinSpectator(playerInfo->getName());
	} else {
		Player *newPlayer = addPlayer(playerInfo->getPlayerId(), playerInfo->getName());
		messageLog->logJoin(newPlayer);
		playerListWidget->addPlayer(playerInfo);
	}
}

void TabGame::eventLeave(Event_Leave *event)
{
	int playerId = event->getPlayerId();
	Player *player = players.value(playerId, 0);
	if (!player)
		return;
	
	messageLog->logLeave(player);
	playerListWidget->removePlayer(playerId);
}

void TabGame::eventGameClosed(Event_GameClosed * /*event*/)
{
	started = false;
	messageLog->logGameClosed();
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
	return player;
}

void TabGame::eventSetActivePlayer(Event_SetActivePlayer *event)
{
	Player *player = setActivePlayer(event->getActivePlayerId());
	if (!player)
		return;
	messageLog->logSetActivePlayer(player);
}

void TabGame::setActivePhase(int phase)
{
	if (currentPhase != phase) {
		currentPhase = phase;
		phasesToolbar->setActivePhase(phase);
	}
}

void TabGame::eventSetActivePhase(Event_SetActivePhase *event)
{
	int phase = event->getPhase();
	if (currentPhase != phase)
		messageLog->logSetActivePhase(phase);
	setActivePhase(phase);
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
}

void TabGame::readyStart()
{
	client->sendCommand(new Command_ReadyStart(gameId));
}

void TabGame::newCardAdded(CardItem *card)
{
	connect(card, SIGNAL(hovered(CardItem *)), cardInfo, SLOT(setCard(CardItem *)));
}
