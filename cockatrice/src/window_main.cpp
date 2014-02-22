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
#include <QMenu>
#include <QAction>
#include <QCloseEvent>
#include <QMessageBox>
#include <QMenuBar>
#include <QPixmapCache>
#include <QInputDialog>
#include <QFile>
#include <QFileDialog>
#include <QThread>
#include <QDateTime>

#include "main.h"
#include "window_main.h"
#include "dlg_connect.h"
#include "dlg_settings.h"
#include "tab_supervisor.h"
#include "remoteclient.h"
#include "localserver.h"
#include "localserverinterface.h"
#include "localclient.h"
#include "settingscache.h"
#include "tab_game.h"

#include "version_string.h"

#include "pb/game_replay.pb.h"
#include "pb/room_commands.pb.h"
#include "pb/event_connection_closed.pb.h"
#include "pb/event_server_shutdown.pb.h"

const QString MainWindow::appName = "Cockatrice";

void MainWindow::updateTabMenu(const QList<QMenu *> &newMenuList)
{
    for (int i = 0; i < tabMenus.size(); ++i)
        menuBar()->removeAction(tabMenus[i]->menuAction());
    tabMenus = newMenuList;
    for (int i = 0; i < tabMenus.size(); ++i)
        menuBar()->insertMenu(helpMenu->menuAction(), tabMenus[i]);
}

void MainWindow::processConnectionClosedEvent(const Event_ConnectionClosed &event)
{
    client->disconnectFromServer();
    QString reasonStr;
    switch (event.reason()) {
        case Event_ConnectionClosed::TOO_MANY_CONNECTIONS: reasonStr = tr("There are too many concurrent connections from your address."); break;
        case Event_ConnectionClosed::BANNED: {
            reasonStr = tr("Banned by moderator");
            if (event.has_end_time())
                reasonStr.append("\n" + tr("Expected end time: %1").arg(QDateTime::fromTime_t(event.end_time()).toString()));
            else
                reasonStr.append("\n" + tr("This ban lasts indefinitely."));
            if (event.has_reason_str())
                reasonStr.append("\n\n" + QString::fromStdString(event.reason_str()));
            break;
        }
        case Event_ConnectionClosed::SERVER_SHUTDOWN: reasonStr = tr("Scheduled server shutdown."); break;
        case Event_ConnectionClosed::USERNAMEINVALID: reasonStr = tr("Invalid username."); break;
        default: reasonStr = QString::fromStdString(event.reason_str());
    }
    QMessageBox::critical(this, tr("Connection closed"), tr("The server has terminated your connection.\nReason: %1").arg(reasonStr));
}

void MainWindow::processServerShutdownEvent(const Event_ServerShutdown &event)
{
    QMessageBox::information(this, tr("Scheduled server shutdown"), tr("The server is going to be restarted in %n minute(s).\nAll running games will be lost.\nReason for shutdown: %1", "", event.minutes()).arg(QString::fromStdString(event.reason())));
}

void MainWindow::statusChanged(ClientStatus _status)
{
    setClientStatusTitle();
    switch (_status) {
        case StatusConnecting:
            break;
        case StatusDisconnected:
            tabSupervisor->stop();
            aSinglePlayer->setEnabled(true);
            aConnect->setEnabled(true);
            aDisconnect->setEnabled(false);
            break;
        case StatusLoggingIn:
            aSinglePlayer->setEnabled(false);
            aConnect->setEnabled(false);
            aDisconnect->setEnabled(true);
            break;
        case StatusLoggedIn:
            break;
        default:
            break;
    }
}

void MainWindow::userInfoReceived(const ServerInfo_User &info)
{
    tabSupervisor->start(info);
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
    
    aConnect->setEnabled(false);
    aSinglePlayer->setEnabled(false);
    
    localServer = new LocalServer(this);
    LocalServerInterface *mainLsi = localServer->newConnection();
    LocalClient *mainClient = new LocalClient(mainLsi, tr("Player %1").arg(1), this);
    QList<AbstractClient *> localClients;
    localClients.append(mainClient);
    
    for (int i = 0; i < numberPlayers - 1; ++i) {
        LocalServerInterface *slaveLsi = localServer->newConnection();
        LocalClient *slaveClient = new LocalClient(slaveLsi, tr("Player %1").arg(i + 2), this);
        localClients.append(slaveClient);
    }
    tabSupervisor->startLocal(localClients);
    
    Command_CreateGame createCommand;
    createCommand.set_max_players(numberPlayers);
    mainClient->sendCommand(mainClient->prepareRoomCommand(createCommand, 0));
}

void MainWindow::actWatchReplay()
{
    QFileDialog dlg(this, tr("Load replay"));
    dlg.setDirectory(settingsCache->getReplaysPath());
    dlg.setNameFilters(QStringList() << QObject::tr("Cockatrice replays (*.cor)"));
    if (!dlg.exec())
        return;
    
    QString fileName = dlg.selectedFiles().at(0);
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
        return;
    QByteArray buf = file.readAll();
    file.close();
    
    GameReplay *replay = new GameReplay;
    replay->ParseFromArray(buf.data(), buf.size());
    
    tabSupervisor->openReplay(replay);
}

void MainWindow::localGameEnded()
{
    delete localServer;
    localServer = 0;
    
    aConnect->setEnabled(true);
    aSinglePlayer->setEnabled(true);
}

void MainWindow::actDeckEditor()
{
    tabSupervisor->addDeckEditorTab(0);
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

void MainWindow::actAbout()
{
    QMessageBox::about(this, tr("About Cockatrice"), QString(
        "<font size=\"8\"><b>Cockatrice</b></font><br>"
        + tr("Version %1").arg(VERSION_STRING)
        + "<br><br><br><b>" + tr("Authors:") + "</b><br>Max-Wilhelm Bruker<br>Marcus Schütz<br><br>"
        + "<b>" + tr("Translators:") + "</b><br>"
        + tr("Spanish:") + " Víctor Martínez<br>"
        + tr("Portugese (Portugal):") + " Milton Gonçalves<br>"
        + tr("Portugese (Brazil):") + " Thiago Queiroz<br>"
        + tr("French:") + " Yannick Hammer, Arnaud Faes<br>"
        + tr("Japanese:") + " Nagase Task<br>"
        + tr("Russian:") + " Alexander Davidov<br>"
//        + tr("Czech:") + " Ondřej Trhoň<br>"
//        + tr("Slovak:") + " Ganjalf Rendy<br>"
        + tr("Italian:") + " Luigi Sciolla<br>"
        + tr("Swedish:") + " Jessica Dahl<br>"
    ));
}

void MainWindow::serverTimeout()
{
    QMessageBox::critical(this, tr("Error"), tr("Server timeout"));
}

void MainWindow::loginError(Response::ResponseCode r, QString reasonStr, quint32 endTime)
{
    switch (r) {
        case Response::RespWrongPassword:
            QMessageBox::critical(this, tr("Error"), tr("Invalid login data."));
            break;
        case Response::RespWouldOverwriteOldSession:
            QMessageBox::critical(this, tr("Error"), tr("There is already an active session using this user name.\nPlease close that session first and re-login."));
            break;
        case Response::RespUserIsBanned: {
            QString bannedStr;
            if (endTime)
                bannedStr = tr("You are banned until %1.").arg(QDateTime::fromTime_t(endTime).toString());
            else
                bannedStr = tr("You are banned indefinitely.");
            if (!reasonStr.isEmpty())
                bannedStr.append("\n\n" + reasonStr);
        
            QMessageBox::critical(this, tr("Error"), bannedStr);
            break;
        }
        case Response::RespUsernameInvalid:
            QMessageBox::critical(this, tr("Error"), tr("Invalid username."));
            break;
        default:
            QMessageBox::critical(this, tr("Error"), tr("Unknown login error: %1").arg(static_cast<int>(r)));
    }
}

void MainWindow::socketError(const QString &errorStr)
{
    QMessageBox::critical(this, tr("Error"), tr("Socket error: %1").arg(errorStr));
}

void MainWindow::protocolVersionMismatch(int localVersion, int remoteVersion)
{
    if (localVersion > remoteVersion)
        QMessageBox::critical(this, tr("Error"), tr("You are trying to connect to an obsolete server. Please downgrade your Cockatrice version or connect to a suitable server.\nLocal version is %1, remote version is %2.").arg(localVersion).arg(remoteVersion));
    else
        QMessageBox::critical(this, tr("Error"), tr("Your Cockatrice client is obsolete. Please update your Cockatrice version.\nLocal version is %1, remote version is %2.").arg(localVersion).arg(remoteVersion));
}

void MainWindow::setClientStatusTitle()
{
    switch (client->getStatus()) {
        case StatusConnecting: setWindowTitle(appName + " - " + tr("Connecting to %1...").arg(client->peerName())); break;
        case StatusDisconnected: setWindowTitle(appName + " - " + tr("Disconnected")); break;
        case StatusLoggingIn: setWindowTitle(appName + " - " + tr("Connected, logging in at %1").arg(client->peerName())); break;
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
    aWatchReplay->setText(tr("&Watch replay..."));
    aDeckEditor->setText(tr("&Deck editor"));
    aFullScreen->setText(tr("&Full screen"));
    aFullScreen->setShortcut(tr("Ctrl+F"));
    aSettings->setText(tr("&Settings..."));
    aExit->setText(tr("&Exit"));
    
    cockatriceMenu->setTitle(tr("&Cockatrice"));
    
    aAbout->setText(tr("&About Cockatrice"));
    helpMenu->setTitle(tr("&Help"));
    
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
    aWatchReplay = new QAction(this);
    connect(aWatchReplay, SIGNAL(triggered()), this, SLOT(actWatchReplay()));
    aDeckEditor = new QAction(this);
    connect(aDeckEditor, SIGNAL(triggered()), this, SLOT(actDeckEditor()));
    aFullScreen = new QAction(this);
    aFullScreen->setCheckable(true);
    connect(aFullScreen, SIGNAL(toggled(bool)), this, SLOT(actFullScreen(bool)));
    aSettings = new QAction(this);
    connect(aSettings, SIGNAL(triggered()), this, SLOT(actSettings()));
    aExit = new QAction(this);
    connect(aExit, SIGNAL(triggered()), this, SLOT(actExit()));
    
    aAbout = new QAction(this);
    connect(aAbout, SIGNAL(triggered()), this, SLOT(actAbout()));
}

void MainWindow::createMenus()
{
    cockatriceMenu = menuBar()->addMenu(QString());
    cockatriceMenu->addAction(aConnect);
    cockatriceMenu->addAction(aDisconnect);
    cockatriceMenu->addAction(aSinglePlayer);
    cockatriceMenu->addAction(aWatchReplay);
    cockatriceMenu->addSeparator();
    cockatriceMenu->addAction(aDeckEditor);
    cockatriceMenu->addSeparator();
    cockatriceMenu->addAction(aFullScreen);
    cockatriceMenu->addSeparator();
    cockatriceMenu->addAction(aSettings);
    cockatriceMenu->addSeparator();
    cockatriceMenu->addAction(aExit);
    
    helpMenu = menuBar()->addMenu(QString());
    helpMenu->addAction(aAbout);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), localServer(0)
{
    QPixmapCache::setCacheLimit(200000);

    client = new RemoteClient;
    connect(client, SIGNAL(connectionClosedEventReceived(const Event_ConnectionClosed &)), this, SLOT(processConnectionClosedEvent(const Event_ConnectionClosed &)));
    connect(client, SIGNAL(serverShutdownEventReceived(const Event_ServerShutdown &)), this, SLOT(processServerShutdownEvent(const Event_ServerShutdown &)));
    connect(client, SIGNAL(loginError(Response::ResponseCode, QString, quint32)), this, SLOT(loginError(Response::ResponseCode, QString, quint32)));
    connect(client, SIGNAL(socketError(const QString &)), this, SLOT(socketError(const QString &)));
    connect(client, SIGNAL(serverTimeout()), this, SLOT(serverTimeout()));
    connect(client, SIGNAL(statusChanged(ClientStatus)), this, SLOT(statusChanged(ClientStatus)));
    connect(client, SIGNAL(protocolVersionMismatch(int, int)), this, SLOT(protocolVersionMismatch(int, int)));
    connect(client, SIGNAL(userInfoChanged(const ServerInfo_User &)), this, SLOT(userInfoReceived(const ServerInfo_User &)), Qt::BlockingQueuedConnection);
    
    clientThread = new QThread(this);
    client->moveToThread(clientThread);
    clientThread->start();

    createActions();
    createMenus();
    
    tabSupervisor = new TabSupervisor(client);
    connect(tabSupervisor, SIGNAL(setMenu(QList<QMenu *>)), this, SLOT(updateTabMenu(QList<QMenu *>)));
    connect(tabSupervisor, SIGNAL(localGameEnded()), this, SLOT(localGameEnded()));
    tabSupervisor->addDeckEditorTab(0);    
    
    setCentralWidget(tabSupervisor);

    retranslateUi();
    
    resize(900, 700);
    restoreGeometry(settingsCache->getMainWindowGeometry());
    aFullScreen->setChecked(windowState() & Qt::WindowFullScreen);
}

MainWindow::~MainWindow()
{
    client->deleteLater();
    clientThread->wait();
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
    settingsCache->setMainWindowGeometry(saveGeometry());
    delete tabSupervisor;
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
    QMainWindow::changeEvent(event);
}
