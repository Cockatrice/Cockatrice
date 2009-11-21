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
#include "main.h"

TabGame::TabGame(Client *_client, int _gameId)
	: client(_client), gameId(_gameId)
{
	zoneLayout = new ZoneViewLayout;
	scene = new GameScene(zoneLayout, this);
	gameView = new GameView(scene);
	gameView->hide();
	
	deckView = new DeckView;
	
	DeckList *foo = new DeckList;
	foo->loadFromFile("/home/brukie/cockatrice/decks/adfb.cod", DeckList::CockatriceFormat);
	deckView->setDeck(foo);
//	deckView->hide();

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
	mainLayout->addWidget(deckView, 10);
	mainLayout->addLayout(verticalLayout);

	aCloseMostRecentZoneView = new QAction(this);
	connect(aCloseMostRecentZoneView, SIGNAL(triggered()), zoneLayout, SLOT(closeMostRecentZoneView()));
	addAction(aCloseMostRecentZoneView);
	
	connect(sayEdit, SIGNAL(returnPressed()), this, SLOT(actSay()));

//	connect(client, SIGNAL(maxPingTime(int, int)), pingWidget, SLOT(setPercentage(int, int)));

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
	sayLabel->setText(tr("&Say:"));
	cardInfo->retranslateUi();
//	if (game)
//		game->retranslateUi();
	zoneLayout->retranslateUi();
	aCloseMostRecentZoneView->setText(tr("Close most recent zone view"));
	aCloseMostRecentZoneView->setShortcut(tr("Esc"));
}

void TabGame::processGameEvent(GameEvent *event)
{
//	game->processGameEvent(event);
}
