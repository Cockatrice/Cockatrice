/***************************************************************************
 *   Copyright (C) 2008 by Max-Wilhelm Bruker   *
 *   brukie@gmx.net   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <QtGui>
//#include <QtOpenGL>

#include "window_main.h"
#include "dlg_connect.h"
#include "dlg_settings.h"
#include "gameselector.h"
#include "window_deckeditor.h"
#include "cardinfowidget.h"
#include "messagelogwidget.h"
#include "phasestoolbar.h"
#include "gameview.h"
#include "player.h"
#include "game.h"
#include "carddatabase.h"
#include "zoneviewzone.h"
#include "zoneviewwidget.h"
#include "zoneviewlayout.h"

void MainWindow::hoverCard(QString name)
{
	cardInfo->setCard(name);
}

void MainWindow::playerAdded(Player *player)
{
	menuBar()->addMenu(player->getPlayerMenu());
	connect(player, SIGNAL(toggleZoneView(Player *, QString, int)), zoneLayout, SLOT(toggleZoneView(Player *, QString, int)));
	connect(player, SIGNAL(closeZoneView(ZoneViewZone *)), zoneLayout, SLOT(removeItem(ZoneViewZone *)));
}

void MainWindow::playerRemoved(Player *player)
{
	menuBar()->removeAction(player->getPlayerMenu()->menuAction());
}

void MainWindow::statusChanged(ProtocolStatus _status)
{
	switch (_status) {
		case StatusConnecting:
			emit logConnecting(client->peerName());
			break;
		case StatusDisconnected:
			if (game) {
				zoneLayout->clear();
				delete game;
				game = 0;
			}
			aDisconnect->setEnabled(false);
			aRestartGame->setEnabled(false);
			aLeaveGame->setEnabled(false);
			emit logDisconnected();
			break;
		case StatusLoggingIn:
			aDisconnect->setEnabled(true);
			break;
		case StatusIdle: {
			if (game) {
				zoneLayout->clear();
				delete game;
				game = 0;
			}
			aRestartGame->setEnabled(false);
			aLeaveGame->setEnabled(false);
			
			GameSelector *gameSelector = new GameSelector(client);
			viewLayout->insertWidget(0, gameSelector);
		}
		case StatusPlaying:
			break;
		default:
			break;
	}
}

// Actions

void MainWindow::actConnect()
{
	DlgConnect dlg(this);
	if (dlg.exec())
		client->connectToServer(dlg.getHost(), dlg.getPort(), dlg.getPlayerName(), dlg.getPassword());
}

void MainWindow::actDisconnect()
{
	client->disconnectFromServer();
}

void MainWindow::actRestartGame()
{
	zoneLayout->clear();
	game->restartGameDialog();
}

void MainWindow::actLeaveGame()
{
	client->leaveGame();
}

void MainWindow::actDeckEditor()
{
	WndDeckEditor *deckEditor = new WndDeckEditor(db, this);
	deckEditor->show();
}

void MainWindow::actFullScreen(bool checked)
{
	if (checked)
		setWindowState(windowState() | Qt::WindowFullScreen);
	else
		setWindowState(windowState() & ~Qt::WindowFullScreen);
}

void MainWindow::actSettings()
{
	DlgSettings dlg(db, translator, this);
	dlg.exec();
}

void MainWindow::actExit()
{
	close();
}

void MainWindow::updateSceneSize()
{
	QRectF sr = scene->sceneRect();
	QSizeF zoneSize = zoneLayout->size();
	qDebug(QString("updateSceneSize: width=%1").arg(1096 + zoneSize.width()).toLatin1());
	scene->setSceneRect(sr.x(), sr.y(), 1096 + zoneSize.width(), sr.height());
	view->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

void MainWindow::actSay()
{
	if (sayEdit->text().isEmpty())
		return;
	
	client->say(sayEdit->text());
	sayEdit->clear();
}

void MainWindow::playerIdReceived(int id, QString name)
{
	game = new Game(db, client, scene, actionsMenu, cardMenu, id, name, this);
	connect(game, SIGNAL(hoverCard(QString)), this, SLOT(hoverCard(QString)));
	connect(game, SIGNAL(playerAdded(Player *)), this, SLOT(playerAdded(Player *)));
	connect(game, SIGNAL(playerRemoved(Player *)), this, SLOT(playerRemoved(Player *)));
	playerAdded(game->getLocalPlayer());

	messageLog->connectToGame(game);
	aRestartGame->setEnabled(true);
	aLeaveGame->setEnabled(true);

	client->listPlayers();
}

void MainWindow::serverTimeout()
{
	QMessageBox::critical(this, tr("Error"), tr("Server timeout"));
}

void MainWindow::retranslateUi()
{
	aConnect->setText(tr("&Connect..."));
	aDisconnect->setText(tr("&Disconnect"));
	aRestartGame->setText(tr("&Restart game..."));
	aRestartGame->setShortcut(tr("F2"));
	aLeaveGame->setText(tr("&Leave game"));
	aDeckEditor->setText(tr("&Deck editor"));
	aFullScreen->setText(tr("&Full screen"));
	aFullScreen->setShortcut(tr("Ctrl+F"));
	aSettings->setText(tr("&Settings..."));
	aExit->setText(tr("&Exit"));
	aCloseMostRecentZoneView->setText(tr("Close most recent zone view"));
	aCloseMostRecentZoneView->setShortcut(tr("Esc"));
	
	gameMenu->setTitle(tr("&Game"));
	actionsMenu->setTitle(tr("&Actions"));
	cardMenu->setTitle(tr("&Card"));
	
	sayLabel->setText(tr("&Say:"));
	
	cardInfo->retranslateUi();
}

void MainWindow::createActions()
{
	aConnect = new QAction(this);
	connect(aConnect, SIGNAL(triggered()), this, SLOT(actConnect()));
	aDisconnect = new QAction(this);
	aDisconnect->setEnabled(false);
	connect(aDisconnect, SIGNAL(triggered()), this, SLOT(actDisconnect()));
	aRestartGame = new QAction(this);
	aRestartGame->setEnabled(false);
	connect(aRestartGame, SIGNAL(triggered()), this, SLOT(actRestartGame()));
	aLeaveGame = new QAction(this);
	aLeaveGame->setEnabled(false);
	connect(aLeaveGame, SIGNAL(triggered()), this, SLOT(actLeaveGame()));
	aDeckEditor = new QAction(this);
	connect(aDeckEditor, SIGNAL(triggered()), this, SLOT(actDeckEditor()));
	aFullScreen = new QAction(this);
	aFullScreen->setCheckable(true);
	connect(aFullScreen, SIGNAL(toggled(bool)), this, SLOT(actFullScreen(bool)));
	aSettings = new QAction(this);
	connect(aSettings, SIGNAL(triggered()), this, SLOT(actSettings()));
	aExit = new QAction(this);
	connect(aExit, SIGNAL(triggered()), this, SLOT(actExit()));

	aCloseMostRecentZoneView = new QAction(this);
	connect(aCloseMostRecentZoneView, SIGNAL(triggered()), zoneLayout, SLOT(closeMostRecentZoneView()));
	addAction(aCloseMostRecentZoneView);
}

void MainWindow::createMenus()
{
	gameMenu = menuBar()->addMenu(QString());
	gameMenu->addAction(aConnect);
	gameMenu->addAction(aDisconnect);
	gameMenu->addSeparator();
	gameMenu->addAction(aRestartGame);
	gameMenu->addAction(aLeaveGame);
	gameMenu->addSeparator();
	gameMenu->addAction(aDeckEditor);
	gameMenu->addSeparator();
	gameMenu->addAction(aFullScreen);
	gameMenu->addSeparator();
	gameMenu->addAction(aSettings);
	gameMenu->addSeparator();
	gameMenu->addAction(aExit);

	actionsMenu = menuBar()->addMenu(QString());

	cardMenu = menuBar()->addMenu(QString());
}

MainWindow::MainWindow(QTranslator *_translator, QWidget *parent)
	: QMainWindow(parent), game(NULL), translator(_translator)
{
	QPixmapCache::setCacheLimit(200000);

	db = new CardDatabase(this);
	
	scene = new QGraphicsScene(0, 0, 1096, 1160, this);
	view = new GameView(scene);

//	view->setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));

	zoneLayout = new ZoneViewLayout(db);
	zoneLayout->setPos(1096, 0);
	scene->addItem(zoneLayout);
	connect(zoneLayout, SIGNAL(sizeChanged()), this, SLOT(updateSceneSize()));

	cardInfo = new CardInfoWidget(db);
	messageLog = new MessageLogWidget;
	sayLabel = new QLabel;
	sayEdit = new QLineEdit;
	sayLabel->setBuddy(sayEdit);

	QHBoxLayout *hLayout = new QHBoxLayout;
	hLayout->addWidget(sayLabel);
	hLayout->addWidget(sayEdit);

	QVBoxLayout *verticalLayout = new QVBoxLayout;
	verticalLayout->addWidget(cardInfo);
	verticalLayout->addWidget(messageLog);
	verticalLayout->addLayout(hLayout);

	viewLayout = new QVBoxLayout;
	viewLayout->addWidget(view);

	PhasesToolbar *phasesToolbar = new PhasesToolbar;

	QHBoxLayout *mainLayout = new QHBoxLayout;
	mainLayout->addWidget(phasesToolbar);
	mainLayout->addLayout(viewLayout, 10);
	mainLayout->addLayout(verticalLayout);

	QWidget *centralWidget = new QWidget;
	centralWidget->setLayout(mainLayout);
	setCentralWidget(centralWidget);

	connect(sayEdit, SIGNAL(returnPressed()), this, SLOT(actSay()));

	client = new Client(this);
	connect(client, SIGNAL(serverTimeout()), this, SLOT(serverTimeout()));
	connect(client, SIGNAL(statusChanged(ProtocolStatus)), this, SLOT(statusChanged(ProtocolStatus)));
	connect(client, SIGNAL(playerIdReceived(int, QString)), this, SLOT(playerIdReceived(int, QString)));

	connect(this, SIGNAL(logConnecting(QString)), messageLog, SLOT(logConnecting(QString)));
	connect(client, SIGNAL(welcomeMsgReceived(const QStringList)), messageLog, SLOT(logConnected(const QStringList)));
	connect(this, SIGNAL(logDisconnected()), messageLog, SLOT(logDisconnected()));
	connect(client, SIGNAL(logSocketError(const QString &)), messageLog, SLOT(logSocketError(const QString &)));
	connect(client, SIGNAL(serverError(ServerResponse)), messageLog, SLOT(logServerError(ServerResponse)));

	createActions();
	createMenus();
	
	retranslateUi();
}

void MainWindow::closeEvent(QCloseEvent */*event*/)
{
	delete game;
}

void MainWindow::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange)
		retranslateUi();
	QMainWindow::changeEvent(event);
}
