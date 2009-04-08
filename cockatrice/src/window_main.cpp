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
#include "dlg_games.h"
#include "window_deckeditor.h"
#include "cardinfowidget.h"
#include "messagelogwidget.h"

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
	connect(player, SIGNAL(addZoneView(Player *, QString, int)), zoneLayout, SLOT(addItem(Player *, QString, int)));
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
				delete game;
				game = 0;
			}
			aDisconnect->setEnabled(false);
			aGames->setEnabled(false);
			aRestartGame->setEnabled(false);
			aLeaveGame->setEnabled(false);
			emit logDisconnected();
			break;
		case StatusConnected:
			aDisconnect->setEnabled(true);
			aGames->setEnabled(true);
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

void MainWindow::actGames()
{
	DlgGames dlg(client, this);
	dlg.exec();
}

void MainWindow::actLeaveGame()
{
	client->leaveGame();
	delete game;
	game = 0;
	aRestartGame->setEnabled(false);
	aLeaveGame->setEnabled(false);
	aGames->setEnabled(true);
}

void MainWindow::actDeckEditor()
{
	WndDeckEditor *deckEditor = new WndDeckEditor(db, this);
	deckEditor->show();
}

void MainWindow::actExit()
{
	close();
}

void MainWindow::updateSceneSize()
{
	QRectF sr = scene->sceneRect();
	QSizeF zoneSize = zoneLayout->size();
	qDebug(QString("updateSceneSize: width=%1").arg(932 + zoneSize.width()).toLatin1());
	scene->setSceneRect(sr.x(), sr.y(), 932 + zoneSize.width(), sr.height());
	view->scaleToScene();
}

void MainWindow::textChanged(const QString &text)
{
	sayButton->setEnabled(!text.isEmpty());
}

// Knöpfe

void MainWindow::buttonSay()
{
	client->say(sayEdit->text());
	sayEdit->clear();
}

void MainWindow::playerIdReceived(int id, QString name)
{
	game = new Game(db, client, scene, actionsMenu, cardMenu, id, name);
	connect(game, SIGNAL(hoverCard(QString)), this, SLOT(hoverCard(QString)));
	connect(game, SIGNAL(playerAdded(Player *)), this, SLOT(playerAdded(Player *)));
	connect(game, SIGNAL(playerRemoved(Player *)), this, SLOT(playerRemoved(Player *)));
	connect(aRestartGame, SIGNAL(triggered()), game, SLOT(restartGameDialog()));
	playerAdded(game->getLocalPlayer());

	messageLog->connectToGame(game);
	aGames->setEnabled(false);
	aRestartGame->setEnabled(true);
	aLeaveGame->setEnabled(true);

	client->listPlayers();
}

void MainWindow::serverTimeout()
{
	QMessageBox::critical(this, tr("Error"), tr("Server timeout"));
}

void MainWindow::createActions()
{
	aConnect = new QAction(tr("&Connect..."), this);
	connect(aConnect, SIGNAL(triggered()), this, SLOT(actConnect()));
	aDisconnect = new QAction(tr("&Disconnect"), this);
	aDisconnect->setEnabled(false);
	connect(aDisconnect, SIGNAL(triggered()), this, SLOT(actDisconnect()));
	aGames = new QAction(tr("&Games..."), this);
	aGames->setEnabled(false);
	connect(aGames, SIGNAL(triggered()), this, SLOT(actGames()));
	aRestartGame = new QAction(tr("&Restart game..."), this);
	aRestartGame->setShortcut(tr("F2"));
	aRestartGame->setEnabled(false);
	aLeaveGame = new QAction(tr("&Leave game"), this);
	aLeaveGame->setEnabled(false);
	connect(aLeaveGame, SIGNAL(triggered()), this, SLOT(actLeaveGame()));
	aDeckEditor = new QAction(tr("&Deck editor"), this);
	connect(aDeckEditor, SIGNAL(triggered()), this, SLOT(actDeckEditor()));
	aExit = new QAction(tr("&Exit"), this);
	connect(aExit, SIGNAL(triggered()), this, SLOT(actExit()));
}

void MainWindow::createMenus()
{
	gameMenu = menuBar()->addMenu(tr("&Game"));
	gameMenu->addAction(aConnect);
	gameMenu->addAction(aDisconnect);
	gameMenu->addSeparator();
	gameMenu->addAction(aGames);
	gameMenu->addAction(aRestartGame);
	gameMenu->addAction(aLeaveGame);
	gameMenu->addSeparator();
	gameMenu->addAction(aDeckEditor);
	gameMenu->addSeparator();
	gameMenu->addAction(aExit);

	actionsMenu = menuBar()->addMenu(tr("&Actions"));

	cardMenu = menuBar()->addMenu(tr("&Card"));
}

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent), game(NULL)
{
//	setWindowState(windowState() | Qt::WindowFullScreen);

	QPixmapCache::setCacheLimit(200000);
	createActions();
	createMenus();

	db = new CardDatabase;
	int cardCount = db->loadFromFile("../cards.dat");
	qDebug(QString("%1 cards loaded").arg(cardCount).toLatin1());

	scene = new QGraphicsScene(0, 0, 932, 1020, this);
	view = new GameView(scene);

//	view->setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));

	zoneLayout = new ZoneViewLayout(db);
	zoneLayout->setPos(932, 0);
	scene->addItem(zoneLayout);
	connect(zoneLayout, SIGNAL(sizeChanged()), this, SLOT(updateSceneSize()));

	cardInfo = new CardInfoWidget(db);
	messageLog = new MessageLogWidget;
	sayEdit = new QLineEdit;
	sayButton = new QPushButton(tr("&Say"));
	sayButton->setEnabled(false);

	QHBoxLayout *hLayout = new QHBoxLayout;
	hLayout->addWidget(sayEdit);
	hLayout->addWidget(sayButton);

	QVBoxLayout *verticalLayout = new QVBoxLayout;
	verticalLayout->addWidget(cardInfo);
	verticalLayout->addWidget(messageLog);
	verticalLayout->addLayout(hLayout);

	QHBoxLayout *mainLayout = new QHBoxLayout;
	mainLayout->addWidget(view);
	mainLayout->setStretchFactor(view, 10);
	mainLayout->addLayout(verticalLayout);

	QWidget *centralWidget = new QWidget;
	centralWidget->setLayout(mainLayout);
	setCentralWidget(centralWidget);

	connect(sayEdit, SIGNAL(returnPressed()), sayButton, SLOT(click()));
	connect(sayEdit, SIGNAL(textChanged(const QString &)), this, SLOT(textChanged(const QString &)));
	connect(sayButton, SIGNAL(clicked()), this, SLOT(buttonSay()));

	client = new Client(this);
	connect(client, SIGNAL(serverTimeout()), this, SLOT(serverTimeout()));
	connect(client, SIGNAL(statusChanged(ProtocolStatus)), this, SLOT(statusChanged(ProtocolStatus)));
	connect(client, SIGNAL(playerIdReceived(int, QString)), this, SLOT(playerIdReceived(int, QString)));

	connect(this, SIGNAL(logConnecting(QString)), messageLog, SLOT(logConnecting(QString)));
	connect(client, SIGNAL(welcomeMsgReceived(const QStringList)), messageLog, SLOT(logConnected(const QStringList)));
	connect(this, SIGNAL(logDisconnected()), messageLog, SLOT(logDisconnected()));
	connect(client, SIGNAL(logSocketError(const QString &)), messageLog, SLOT(logSocketError(const QString &)));
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	delete game;
	
//	db->importOracle();
//	db->saveToFile("../cards.dat");
	delete db;
}
