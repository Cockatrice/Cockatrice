#include <QtGui>
#include "tab_game.h"
#include "cardinfowidget.h"
#include "messagelogwidget.h"
#include "phasestoolbar.h"
#include "gameview.h"
#include "gamescene.h"
#include "player.h"
#include "game.h"
#include "zoneviewzone.h"
#include "zoneviewwidget.h"
#include "zoneviewlayout.h"
#include "deckview.h"
#include "decklist.h"
#include "deck_picturecacher.h"
#include "protocol_items.h"
#include "dlg_load_remote_deck.h"
#include "main.h"

TabGame::TabGame(Client *_client, int _gameId)
	: client(_client), gameId(_gameId), localPlayerId(-1)
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

	cardInfo = new CardInfoWidget(db);
	messageLog = new MessageLogWidget;
	sayLabel = new QLabel;
	sayEdit = new QLineEdit;
	sayLabel->setBuddy(sayEdit);
	
	QHBoxLayout *hLayout = new QHBoxLayout;
	hLayout->addWidget(sayLabel);
	hLayout->addWidget(sayEdit);

	phasesToolbar = new PhasesToolbar;
	phasesToolbar->hide();
	
	QVBoxLayout *verticalLayout = new QVBoxLayout;
	verticalLayout->addWidget(cardInfo);
	verticalLayout->addWidget(messageLog);
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

	connect(phasesToolbar, SIGNAL(signalSetPhase(int)), client, SLOT(setActivePhase(int)));
	connect(phasesToolbar, SIGNAL(signalNextTurn()), client, SLOT(nextTurn()));
	
/*	game = new Game(client, scene, menuBar(), this);
	connect(game, SIGNAL(hoverCard(QString)), cardInfo, SLOT(setCard(const QString &)));
	connect(game, SIGNAL(playerAdded(Player *)), this, SLOT(playerAdded(Player *)));
	connect(game, SIGNAL(playerRemoved(Player *)), scene, SLOT(removePlayer(Player *)));
	connect(game, SIGNAL(setActivePhase(int)), phasesToolbar, SLOT(setActivePhase(int)));
	connect(phasesToolbar, SIGNAL(signalDrawCard()), game, SLOT(activePlayerDrawCard()));
	connect(phasesToolbar, SIGNAL(signalUntapAll()), game, SLOT(activePlayerUntapAll()));
	messageLog->connectToGame(game);

	game->queryGameState();
*/
	retranslateUi();
	setLayout(mainLayout);
}

void TabGame::retranslateUi()
{
	loadLocalButton->setText(tr("Load &local deck"));
	loadRemoteButton->setText(tr("Load deck from &server"));
	readyStartButton->setText(tr("&Start game"));
	sayLabel->setText(tr("&Say:"));
	cardInfo->retranslateUi();
//	if (game)
//		game->retranslateUi();
	zoneLayout->retranslateUi();
	aCloseMostRecentZoneView->setText(tr("Close most recent zone view"));
	aCloseMostRecentZoneView->setShortcut(tr("Esc"));
}

Player *TabGame::addPlayer(int playerId, const QString &playerName)
{
	Player *newPlayer = new Player(playerName, playerId, playerId == localPlayerId, client, this);
	scene->addPlayer(newPlayer);

	messageLog->connectToPlayer(newPlayer);

	players.insert(playerId, newPlayer);
	emit playerAdded(newPlayer);

	return newPlayer;
}

void TabGame::processGameEvent(GameEvent *event)
{
	switch (event->getItemId()) {
		case ItemId_Event_GameStart: eventGameStart(qobject_cast<Event_GameStart *>(event)); break;
		default: qDebug() << "unhandled game event";
	}
}

void TabGame::processGameJoinedEvent(Event_GameJoined *event)
{
	localPlayerId = event->getPlayerId();
	spectator = event->getSpectator();

	const QList<ServerInfo_Player *> &plList = event->getPlayerList();
	for (int i = 0; i < plList.size(); ++i) {
		ServerInfo_Player *pl = plList[i];
		Player *newPlayer = addPlayer(pl->getPlayerId(), pl->getName());
		newPlayer->processPlayerInfo(pl);
	}
}

void TabGame::eventGameStart(Event_GameStart *event)
{
	deckViewContainer->hide();
	gameView->show();
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
	Command_DeckSelect *cmd = static_cast<Command_DeckSelect *>(sender());
	delete cmd->getDeck();
	
	Deck_PictureCacher::cachePictures(resp->getDeck(), this);
	deckView->setDeck(resp->getDeck());
}

void TabGame::readyStart()
{
	client->sendCommand(new Command_ReadyStart(gameId));
}
