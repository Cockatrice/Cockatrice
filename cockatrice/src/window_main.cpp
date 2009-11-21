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
#include "window_deckeditor.h"
#include "tab_supervisor.h"

PingWidget::PingWidget(QWidget *parent)
	: QWidget(parent)
{
	setPercentage(0, -1);
}

QSize PingWidget::sizeHint() const
{
	return QSize(15, 15);
}

void PingWidget::paintEvent(QPaintEvent */*event*/)
{
	QPainter painter(this);
	QRadialGradient g(QPointF((double) width() / 2, (double) height() / 2), qMin(width(), height()) / 2.0);
	g.setColorAt(0, color);
	g.setColorAt(1, Qt::transparent);
	painter.fillRect(0, 0, width(), height(), QBrush(g));
}

void PingWidget::setPercentage(int value, int max)
{
	if (max == -1)
		color = Qt::black;
	else
		color.setHsv(120 * (1.0 - ((double) value / max)), 255, 255);
	update();
}
/*
void MainWindow::playerAdded(Player *player)
{
	menuBar()->addMenu(player->getPlayerMenu());
	connect(player, SIGNAL(toggleZoneView(Player *, QString, int)), zoneLayout, SLOT(toggleZoneView(Player *, QString, int)));
	connect(player, SIGNAL(closeZoneView(ZoneViewZone *)), zoneLayout, SLOT(removeItem(ZoneViewZone *)));
}
*/
void MainWindow::statusChanged(ClientStatus _status)
{
	switch (_status) {
		case StatusConnecting:
			emit logConnecting(client->peerName());
			break;
		case StatusDisconnected:
			tabSupervisor->stop();
//			if (game) {
//				zoneLayout->clear();
//				delete game;
//				game = 0;
//			}
//			pingWidget->setPercentage(0, -1);
			aConnect->setEnabled(true);
			aDisconnect->setEnabled(false);
//			aRestartGame->setEnabled(false);
//			aLeaveGame->setEnabled(false);
//			phasesToolbar->setActivePhase(-1);
//			phasesToolbar->hide();
			emit logDisconnected();
			break;
		case StatusLoggingIn:
			emit logConnected();
			aConnect->setEnabled(false);
			aDisconnect->setEnabled(true);
			break;
		case StatusLoggedIn: {
			tabSupervisor->start(client);
	
/*			if (game) {
				zoneLayout->clear();
				delete game;
				game = 0;
			}
			aRestartGame->setEnabled(false);
			aLeaveGame->setEnabled(false);
			phasesToolbar->setActivePhase(-1);
			phasesToolbar->hide();
			
			view->hide();
			gameSelector->enableGameList();
			chatWidget->enableChat();
*/			break;
		}
//		case StatusPlaying: {
/*			chatWidget->disableChat();
			
			aRestartGame->setEnabled(true);
			aLeaveGame->setEnabled(true);
		
			
			phasesToolbar->show();
			view->show();
			break;
		}
*/		default:
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
/*
void MainWindow::actRestartGame()
{
	zoneLayout->clear();
	game->restartGameDialog();
}

void MainWindow::actLeaveGame()
{
	client->leaveGame();
}
*/
void MainWindow::actDeckEditor()
{
	WndDeckEditor *deckEditor = new WndDeckEditor(this);
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
	DlgSettings dlg(this);
	dlg.exec();
}

void MainWindow::actExit()
{
	close();
}
/*
void MainWindow::actSay()
{
	if (sayEdit->text().isEmpty())
		return;
	
	client->say(sayEdit->text());
	sayEdit->clear();
}
*/
void MainWindow::serverTimeout()
{
	QMessageBox::critical(this, tr("Error"), tr("Server timeout"));
}

void MainWindow::retranslateUi()
{
	setWindowTitle(tr("Cockatrice"));
	
	aConnect->setText(tr("&Connect..."));
	aDisconnect->setText(tr("&Disconnect"));
//	aRestartGame->setText(tr("&Restart game..."));
//	aRestartGame->setShortcut(tr("F2"));
//	aLeaveGame->setText(tr("&Leave game"));
	aDeckEditor->setText(tr("&Deck editor"));
	aFullScreen->setText(tr("&Full screen"));
	aFullScreen->setShortcut(tr("Ctrl+F"));
	aSettings->setText(tr("&Settings..."));
	aExit->setText(tr("&Exit"));
	
	cockatriceMenu->setTitle(tr("&Cockatrice"));
}

void MainWindow::createActions()
{
	aConnect = new QAction(this);
	connect(aConnect, SIGNAL(triggered()), this, SLOT(actConnect()));
	aDisconnect = new QAction(this);
	aDisconnect->setEnabled(false);
	connect(aDisconnect, SIGNAL(triggered()), this, SLOT(actDisconnect()));
/*	aRestartGame = new QAction(this);
	aRestartGame->setEnabled(false);
	connect(aRestartGame, SIGNAL(triggered()), this, SLOT(actRestartGame()));
	aLeaveGame = new QAction(this);
	aLeaveGame->setEnabled(false);
	connect(aLeaveGame, SIGNAL(triggered()), this, SLOT(actLeaveGame()));
*/	aDeckEditor = new QAction(this);
	connect(aDeckEditor, SIGNAL(triggered()), this, SLOT(actDeckEditor()));
	aFullScreen = new QAction(this);
	aFullScreen->setCheckable(true);
	connect(aFullScreen, SIGNAL(toggled(bool)), this, SLOT(actFullScreen(bool)));
	aSettings = new QAction(this);
	connect(aSettings, SIGNAL(triggered()), this, SLOT(actSettings()));
	aExit = new QAction(this);
	connect(aExit, SIGNAL(triggered()), this, SLOT(actExit()));
}

void MainWindow::createMenus()
{
	cockatriceMenu = menuBar()->addMenu(QString());
	cockatriceMenu->addAction(aConnect);
	cockatriceMenu->addAction(aDisconnect);
	cockatriceMenu->addSeparator();
	cockatriceMenu->addAction(aDeckEditor);
	cockatriceMenu->addSeparator();
	cockatriceMenu->addAction(aFullScreen);
	cockatriceMenu->addSeparator();
	cockatriceMenu->addAction(aSettings);
	cockatriceMenu->addSeparator();
	cockatriceMenu->addAction(aExit);
}

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	QPixmapCache::setCacheLimit(200000);

	client = new Client(this);
	tabSupervisor = new TabSupervisor;
	
	setCentralWidget(tabSupervisor);
/*
	
	connect(this, SIGNAL(logConnecting(QString)), messageLog, SLOT(logConnecting(QString)));
	connect(this, SIGNAL(logConnected()), messageLog, SLOT(logConnected()));
	connect(this, SIGNAL(logDisconnected()), messageLog, SLOT(logDisconnected()));
	connect(client, SIGNAL(logSocketError(const QString &)), messageLog, SLOT(logSocketError(const QString &)));
	connect(client, SIGNAL(serverError(ResponseCode)), messageLog, SLOT(logServerError(ResponseCode)));
	connect(client, SIGNAL(protocolVersionMismatch(int, int)), messageLog, SLOT(logProtocolVersionMismatch(int, int)));
	connect(client, SIGNAL(protocolError()), messageLog, SLOT(logProtocolError()));
*/
	connect(client, SIGNAL(serverTimeout()), this, SLOT(serverTimeout()));
	connect(client, SIGNAL(statusChanged(ClientStatus)), this, SLOT(statusChanged(ClientStatus)));

	createActions();
	createMenus();
	
	retranslateUi();
	
	resize(900, 700);
}

void MainWindow::closeEvent(QCloseEvent */*event*/)
{
	delete tabSupervisor;
}

void MainWindow::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange)
		retranslateUi();
	QMainWindow::changeEvent(event);
}
