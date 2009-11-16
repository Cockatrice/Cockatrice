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
#include "main.h"

TabGame::TabGame(Client *_client, int _gameId)
	: client(_client), gameId(_gameId)
{
	zoneLayout = new ZoneViewLayout;
	scene = new GameScene(zoneLayout, this);
	view = new GameView(scene);

	cardInfo = new CardInfoWidget(db);
	messageLog = new MessageLogWidget;
	sayLabel = new QLabel;
	sayEdit = new QLineEdit;
	sayLabel->setBuddy(sayEdit);
	
	QHBoxLayout *hLayout = new QHBoxLayout;
	hLayout->addWidget(sayLabel);
	hLayout->addWidget(sayEdit);

	phasesToolbar = new PhasesToolbar;
	
	QVBoxLayout *verticalLayout = new QVBoxLayout;
	verticalLayout->addWidget(cardInfo);
	verticalLayout->addWidget(messageLog);
	verticalLayout->addLayout(hLayout);

	QHBoxLayout *mainLayout = new QHBoxLayout;
	mainLayout->addWidget(phasesToolbar);
	mainLayout->addWidget(view, 10);
	mainLayout->addLayout(verticalLayout);

	setLayout(mainLayout);

	connect(sayEdit, SIGNAL(returnPressed()), this, SLOT(actSay()));

//	connect(client, SIGNAL(maxPingTime(int, int)), pingWidget, SLOT(setPercentage(int, int)));

	connect(phasesToolbar, SIGNAL(signalSetPhase(int)), client, SLOT(setActivePhase(int)));
	connect(phasesToolbar, SIGNAL(signalNextTurn()), client, SLOT(nextTurn()));
	
}

void TabGame::processGameEvent(GameEvent *event)
{
}
