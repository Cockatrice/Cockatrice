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

#include "window_main.h"
#include "dlg_connect.h"
#include "dlg_settings.h"
#include "window_deckeditor.h"
#include "tab_supervisor.h"
#include "remoteclient.h"
#include "localserver.h"
#include "localserverinterface.h"
#include "localclient.h"

const QString MainWindow::appName = "Cockatrice";

void MainWindow::updateTabMenu(QMenu *menu)
{
	if (tabMenu)
		menuBar()->removeAction(tabMenu->menuAction());
	tabMenu = menu;
	if (menu)
		menuBar()->addMenu(menu);
}

void MainWindow::statusChanged(ClientStatus _status)
{
	setClientStatusTitle();
	switch (_status) {
		case StatusConnecting:
			break;
		case StatusDisconnected:
			tabSupervisor->stop();
			aConnect->setEnabled(true);
			aDisconnect->setEnabled(false);
			break;
		case StatusLoggingIn:
			aConnect->setEnabled(false);
			aDisconnect->setEnabled(true);
			break;
		case StatusLoggedIn: {
			tabSupervisor->start(client);
			break;
		}
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

void MainWindow::actSinglePlayer()
{
	bool ok;
	int numberPlayers = QInputDialog::getInt(this, tr("Number of players"), tr("Please enter the number of players."), 2, 1, 8, 1, &ok);
	if (!ok)
		return;
	
	LocalServer *ls = new LocalServer(this);
	LocalServerInterface *mainLsi = ls->newConnection();
	LocalClient *mainClient = new LocalClient(mainLsi, tr("Player %1").arg(1), this);
	localClients.append(mainClient);
	
	for (int i = 0; i < numberPlayers - 1; ++i) {
		LocalServerInterface *slaveLsi = ls->newConnection();
		LocalClient *slaveClient = new LocalClient(slaveLsi, tr("Player %1").arg(i + 2), this);
		localClients.append(slaveClient);
	}
	tabSupervisor->startLocal(localClients);
	
	Command_CreateGame *createCommand = new Command_CreateGame(QString(), QString(), numberPlayers, false, false, false, false);
	mainClient->sendCommand(createCommand);
}

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

void MainWindow::serverTimeout()
{
	QMessageBox::critical(this, tr("Error"), tr("Server timeout"));
}

void MainWindow::serverError(ResponseCode r)
{
	switch (r) {
		case RespWrongPassword: QMessageBox::critical(this, tr("Error"), tr("Invalid login data.")); break;
		default: ;
	}
}

void MainWindow::socketError(const QString &errorStr)
{
	QMessageBox::critical(this, tr("Error"), tr("Socket error: %1").arg(errorStr));
}

void MainWindow::protocolVersionMismatch(int localVersion, int remoteVersion)
{
	QMessageBox::critical(this, tr("Error"), tr("Protocol version mismatch. Local version: %1, remote version: %2.").arg(localVersion).arg(remoteVersion));
}

void MainWindow::setClientStatusTitle()
{
	switch (client->getStatus()) {
		case StatusConnecting: setWindowTitle(appName + " - " + tr("Connecting to %1...").arg(client->peerName())); break;
		case StatusDisconnected: setWindowTitle(appName + " - " + tr("Disconnected")); break;
		case StatusLoggedIn: setWindowTitle(appName + " - " + tr("Logged in at %1").arg(client->peerName())); break;
		default: setWindowTitle(appName);
	}
}

void MainWindow::retranslateUi()
{
	setClientStatusTitle();
	
	aConnect->setText(tr("&Connect..."));
	aDisconnect->setText(tr("&Disconnect"));
	aSinglePlayer->setText(tr("Start &local game..."));
	aDeckEditor->setText(tr("&Deck editor"));
	aFullScreen->setText(tr("&Full screen"));
	aFullScreen->setShortcut(tr("Ctrl+F"));
	aSettings->setText(tr("&Settings..."));
	aExit->setText(tr("&Exit"));
	
	cockatriceMenu->setTitle(tr("&Cockatrice"));
	
	tabSupervisor->retranslateUi();
}

void MainWindow::createActions()
{
	aConnect = new QAction(this);
	connect(aConnect, SIGNAL(triggered()), this, SLOT(actConnect()));
	aDisconnect = new QAction(this);
	aDisconnect->setEnabled(false);
	connect(aDisconnect, SIGNAL(triggered()), this, SLOT(actDisconnect()));
	aSinglePlayer = new QAction(this);
	connect(aSinglePlayer, SIGNAL(triggered()), this, SLOT(actSinglePlayer()));
	aDeckEditor = new QAction(this);
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
	cockatriceMenu->addAction(aSinglePlayer);
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
	: QMainWindow(parent), tabMenu(0)
{
	QPixmapCache::setCacheLimit(200000);

	client = new RemoteClient(this);
	connect(client, SIGNAL(serverError(ResponseCode)), this, SLOT(serverError(ResponseCode)));
	connect(client, SIGNAL(socketError(const QString &)), this, SLOT(socketError(const QString &)));
	connect(client, SIGNAL(serverTimeout()), this, SLOT(serverTimeout()));
	connect(client, SIGNAL(statusChanged(ClientStatus)), this, SLOT(statusChanged(ClientStatus)));
	connect(client, SIGNAL(protocolVersionMismatch(int, int)), this, SLOT(protocolVersionMismatch(int, int)));

	tabSupervisor = new TabSupervisor;
	connect(tabSupervisor, SIGNAL(setMenu(QMenu *)), this, SLOT(updateTabMenu(QMenu *)));
	
	setCentralWidget(tabSupervisor);

	createActions();
	createMenus();
	
	retranslateUi();
	
	resize(900, 700);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	if (tabSupervisor->getGameCount()) {
		if (QMessageBox::question(this, tr("Are you sure?"), tr("There are still open games. Are you sure you want to quit?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No) {
			event->ignore();
			return;
		}
	}
	event->accept();
	delete tabSupervisor;
}

void MainWindow::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange)
		retranslateUi();
	QMainWindow::changeEvent(event);
}
