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
#include <QSystemTrayIcon>
#include <QApplication>
#include <QtNetwork>
#include <QtConcurrent>

#include "main.h"
#include "window_main.h"
#include "dlg_connect.h"
#include "dlg_register.h"
#include "dlg_settings.h"
#include "dlg_update.h"
#include "dlg_viewlog.h"
#include "tab_supervisor.h"
#include "remoteclient.h"
#include "localserver.h"
#include "localserverinterface.h"
#include "localclient.h"
#include "settingscache.h"
#include "tab_game.h"
#include "version_string.h"
#include "update_checker.h"
#include "carddatabase.h"
#include "window_sets.h"
#include "dlg_edit_tokens.h"

#include "pb/game_replay.pb.h"
#include "pb/room_commands.pb.h"
#include "pb/event_connection_closed.pb.h"
#include "pb/event_server_shutdown.pb.h"

#define GITHUB_PAGES_URL "https://cockatrice.github.io"
#define GITHUB_CONTRIBUTORS_URL "https://github.com/Cockatrice/Cockatrice/graphs/contributors?type=c"
#define GITHUB_CONTRIBUTE_URL "https://github.com/Cockatrice/Cockatrice#cockatrice"
#define GITHUB_TRANSLATOR_RECOGNIZE_URL "https://github.com/Cockatrice/Cockatrice/wiki/Translators"
#define GITHUB_TRANSLATOR_FAQ_URL "https://github.com/Cockatrice/Cockatrice/wiki/Translation-FAQ"
#define GITHUB_ISSUES_URL "https://github.com/Cockatrice/Cockatrice/issues"
#define GITHUB_TROUBLESHOOTING_URL "https://github.com/Cockatrice/Cockatrice/wiki/Troubleshooting"
#define GITHUB_FAQ_URL "https://github.com/Cockatrice/Cockatrice/wiki/Frequently-Asked-Questions"

#define DOWNLOAD_URL "https://dl.bintray.com/cockatrice/Cockatrice/"

const QString MainWindow::appName = "Cockatrice";
const QStringList MainWindow::fileNameFilters = QStringList()
    << QObject::tr("Cockatrice card database (*.xml)")
    << QObject::tr("All files (*.*)");

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
        case Event_ConnectionClosed::USER_LIMIT_REACHED: reasonStr = tr("The server has reached its maximum user capacity, please check back later."); break;
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
        case Event_ConnectionClosed::LOGGEDINELSEWERE: reasonStr = tr("You have been logged out due to logging in at another location."); break;
        default: reasonStr = QString::fromStdString(event.reason_str());
    }
    QMessageBox::critical(this, tr("Connection closed"), tr("The server has terminated your connection.\nReason: %1").arg(reasonStr));
}

void MainWindow::processServerShutdownEvent(const Event_ServerShutdown &event)
{
    serverShutdownMessageBox.setInformativeText(tr("The server is going to be restarted in %n minute(s).\nAll running games will be lost.\nReason for shutdown: %1", "", event.minutes()).arg(QString::fromStdString(event.reason())));
    serverShutdownMessageBox.setIconPixmap(QPixmap("theme:cockatrice").scaled(64, 64));
    serverShutdownMessageBox.setText(tr("Scheduled server shutdown"));
    serverShutdownMessageBox.setWindowModality(Qt::ApplicationModal);
    serverShutdownMessageBox.setVisible(true);
}

void MainWindow::statusChanged(ClientStatus _status)
{
    setClientStatusTitle();
    switch (_status) {
        case StatusDisconnected:
            tabSupervisor->stop();
            aSinglePlayer->setEnabled(true);
            aConnect->setEnabled(true);
            aRegister->setEnabled(true);
            aDisconnect->setEnabled(false);
            break;
        case StatusLoggingIn:
            aSinglePlayer->setEnabled(false);
            aConnect->setEnabled(false);
            aRegister->setEnabled(false);
            aDisconnect->setEnabled(true);
            break;
        case StatusConnecting:
        case StatusRegistering:
        case StatusLoggedIn:
        default:
            break;
    }
}

void MainWindow::userInfoReceived(const ServerInfo_User &info)
{
    tabSupervisor->start(info);
}

void MainWindow::registerAccepted()
{
    QMessageBox::information(this, tr("Success"), tr("Registration accepted.\nWill now login."));
}

void MainWindow::registerAcceptedNeedsActivate()
{
    // nothing
}

void MainWindow::activateAccepted()
{
    QMessageBox::information(this, tr("Success"), tr("Account activation accepted.\nWill now login."));
}

// Actions

void MainWindow::actConnect()
{
    DlgConnect dlg(this);
    if (dlg.exec())
        client->connectToServer(dlg.getHost(), dlg.getPort(), dlg.getPlayerName(), dlg.getPassword());
}

void MainWindow::actRegister()
{
    DlgRegister dlg(this);
    if (dlg.exec())
    {
        client->registerToServer(
            dlg.getHost(),
            dlg.getPort(),
            dlg.getPlayerName(),
            dlg.getPassword(),
            dlg.getEmail(),
            dlg.getGender(),
            dlg.getCountry(),
            dlg.getRealName()
        );
    }
}

void MainWindow::actDisconnect()
{
    client->disconnectFromServer();
}

void MainWindow::actSinglePlayer()
{
    bool ok;
    int numberPlayers = QInputDialog::getInt(this, tr("Number of players"), tr("Please enter the number of players."), 1, 1, 8, 1, &ok);
    if (!ok)
        return;

    aConnect->setEnabled(false);
    aRegister->setEnabled(false);
    aSinglePlayer->setEnabled(false);

    localServer = new LocalServer(this);
    LocalServerInterface *mainLsi = localServer->newConnection();
    LocalClient *mainClient = new LocalClient(mainLsi, tr("Player %1").arg(1), settingsCache->getClientID(), this);
    QList<AbstractClient *> localClients;
    localClients.append(mainClient);

    for (int i = 0; i < numberPlayers - 1; ++i) {
        LocalServerInterface *slaveLsi = localServer->newConnection();
        LocalClient *slaveClient = new LocalClient(slaveLsi, tr("Player %1").arg(i + 2), settingsCache->getClientID(), this);
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
    aRegister->setEnabled(true);
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
        + "<br><br><b><a href='" + GITHUB_PAGES_URL + "'>" + tr("Cockatrice Webpage") + "</a></b><br>"
        + "<br><br><b>" + tr("Project Manager:") + "</b><br>Gavin Bisesi<br><br>"
        + "<b>" + tr("Past Project Managers:") + "</b><br>Max-Wilhelm Bruker<br>Marcus Schütz<br><br>"
        + "<b>" + tr("Developers:") + "</b><br>"
        + "<a href='" + GITHUB_CONTRIBUTORS_URL + "'>" + tr("Our Developers") + "</a><br>"
        + "<a href='" + GITHUB_CONTRIBUTE_URL + "'>" + tr("Help Develop!") + "</a><br><br>"
        + "<b>" + tr("Translators:") + "</b><br>"
        + "<a href='" + GITHUB_TRANSLATOR_RECOGNIZE_URL + "'>" + tr("Recognition Page") + "</a><br>"
        + "<a href='" + GITHUB_TRANSLATOR_FAQ_URL + "'>" + tr("Help Translate!") + "</a><br><br>"
        + "<b>" + tr("Support:") + "</b><br>"
        + "<a href='" + GITHUB_ISSUES_URL + "'>" + tr("Report an Issue") + "</a><br>"
        + "<a href='" + GITHUB_TROUBLESHOOTING_URL + "'>" + tr("Troubleshooting") + "</a><br>"
        + "<a href='" + GITHUB_FAQ_URL + "'>" + tr("F.A.Q.") + "</a><br>"
    ));
}

void MainWindow::actUpdate()
{
    DlgUpdate dlg(this);
    dlg.exec();
}

void MainWindow::actViewLog()
{
    DlgViewLog dlg(this);
    dlg.exec();
}

void MainWindow::serverTimeout()
{
    QMessageBox::critical(this, tr("Error"), tr("Server timeout"));
    actConnect();
}

void MainWindow::loginError(Response::ResponseCode r, QString reasonStr, quint32 endTime, QList<QString> missingFeatures)
{
    switch (r) {
        case Response::RespClientUpdateRequired: {
            QString formatedMissingFeatures;
            formatedMissingFeatures = "Missing Features: ";
            for (int i = 0; i < missingFeatures.size(); ++i)
                formatedMissingFeatures.append(QString("\n     %1").arg(QChar(0x2022)) + " " + missingFeatures.value(i)   );

            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.setWindowTitle(tr("Failed Login"));
            msgBox.setText(tr("Your client does not support features that the server requires, please update your client and try again."));
            msgBox.setDetailedText(formatedMissingFeatures);
            msgBox.exec();
            break;
        }
        case Response::RespWrongPassword:
            QMessageBox::critical(this, tr("Error"), tr("Incorrect username or password. Please check your authentication information and try again."));
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
        case Response::RespUsernameInvalid: {
            QMessageBox::critical(this, tr("Error"), extractInvalidUsernameMessage(reasonStr));
            break;
        }
        case Response::RespRegistrationRequired:
            if (QMessageBox::question(this, tr("Error"), tr("This server requires user registration. Do you want to register now?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
                actRegister();
            }
            break;
        case Response::RespClientIdRequired:
            QMessageBox::critical(this, tr("Error"), tr("This server requires client ID's. Your client is either failing to generate an ID or you are running a modified client.\nPlease close and reopen your client to try again."));
            break;
        case Response::RespContextError:
            QMessageBox::critical(this, tr("Error"), tr("An internal error has occurred, please try closing and reopening your client and try again. If the error persists try updating your client to the most recent build and if need be contact your software provider."));
            break;
        case Response::RespAccountNotActivated: {
            bool ok = false;
            QString token = QInputDialog::getText(this, tr("Account activation"), tr("Your account has not been activated yet.\nYou need to provide the activation token received in the activation email"), QLineEdit::Normal, QString(), &ok);
            if(ok && !token.isEmpty())
            {
                client->activateToServer(token);
                return;
            }
            client->disconnectFromServer();
            break;
        }
        default:
            QMessageBox::critical(this, tr("Error"), tr("Unknown login error: %1").arg(static_cast<int>(r)) + tr("\nThis usually means that your client version is out of date, and the server sent a reply your client doesn't understand."));
            break;
    }
    actConnect();
}

QString MainWindow::extractInvalidUsernameMessage(QString & in)
{
    QString out = tr("Invalid username.") + "<br/>";
    QStringList rules = in.split(QChar('|'));
    if (rules.size() == 7)
    {
        out += tr("Your username must respect these rules:") + "<br><ul>";

        out += "<li>" + tr("is %1 - %2 characters long").arg(rules.at(0)).arg(rules.at(1)) + "</li>";
        out += "<li>" + tr("can %1 contain lowercase characters").arg((rules.at(2).toInt() > 0) ? "" : tr("NOT")) + "</li>";
        out += "<li>" + tr("can %1 contain uppercase characters").arg((rules.at(3).toInt() > 0) ? "" : tr("NOT")) + "</li>";
        out += "<li>" + tr("can %1 contain numeric characters").arg((rules.at(4).toInt() > 0) ? "" : tr("NOT")) + "</li>";

        if (rules.at(6).size() > 0)
            out += "<li>" + tr("can contain the following punctuation: %1").arg(rules.at(6).toHtmlEscaped()) + "</li>";

        out += "<li>" + tr("first character can %1 be a punctuation mark").arg((rules.at(5).toInt() > 0) ? "" : tr("NOT")) + "</li>";
        out += "</ul>";
    }
    else
    {
        out += tr("You may only use A-Z, a-z, 0-9, _, ., and - in your username.");
    }

    return out;
}

void MainWindow::registerError(Response::ResponseCode r, QString reasonStr, quint32 endTime)
{
    switch (r) {
        case Response::RespRegistrationDisabled:
            QMessageBox::critical(this, tr("Registration denied"), tr("Registration is currently disabled on this server"));
            break;
        case Response::RespUserAlreadyExists:
            QMessageBox::critical(this, tr("Registration denied"), tr("There is already an existing account with the same user name."));
            break;
        case Response::RespEmailRequiredToRegister:
            QMessageBox::critical(this, tr("Registration denied"), tr("It's mandatory to specify a valid email address when registering."));
            break;
        case Response::RespTooManyRequests:
            QMessageBox::critical(this, tr("Registration denied"), tr("Too many registration attempts from your IP address."));
            break;
        case Response::RespPasswordTooShort:
            QMessageBox::critical(this, tr("Registration denied"), tr("Password too short."));
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
        case Response::RespUsernameInvalid: {
            QMessageBox::critical(this, tr("Error"), extractInvalidUsernameMessage(reasonStr));
            break;
        }
        case Response::RespRegistrationFailed:
            QMessageBox::critical(this, tr("Error"), tr("Registration failed for a technical problem on the server."));
            break;
        default:
            QMessageBox::critical(this, tr("Error"), tr("Unknown registration error: %1").arg(static_cast<int>(r)) + tr("\nThis usually means that your client version is out of date, and the server sent a reply your client doesn't understand."));
    }
    actRegister();
}

void MainWindow::activateError()
{
    QMessageBox::critical(this, tr("Error"), tr("Account activation failed"));
    client->disconnectFromServer();
    actConnect();
}

void MainWindow::socketError(const QString &errorStr)
{
    QMessageBox::critical(this, tr("Error"), tr("Socket error: %1").arg(errorStr));
    actConnect();
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
        case StatusRegistering: setWindowTitle(appName + " - " + tr("Registering to %1 as %2...").arg(client->peerName()).arg(client->getUserName())); break;
        case StatusDisconnected: setWindowTitle(appName + " - " + tr("Disconnected")); break;
        case StatusLoggingIn: setWindowTitle(appName + " - " + tr("Connected, logging in at %1").arg(client->peerName())); break;
        case StatusLoggedIn: setWindowTitle(client->getUserName() + "@" + client->peerName()); break;
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
    aRegister->setText(tr("&Register to server..."));
    aSettings->setText(tr("&Settings..."));
    aSettings->setIcon(QPixmap("theme:icons/settings"));
    aExit->setText(tr("&Exit"));

#if defined(__APPLE__)  /* For OSX */
    cockatriceMenu->setTitle(tr("A&ctions"));
#else
    cockatriceMenu->setTitle(tr("&Cockatrice"));
#endif

    dbMenu->setTitle(tr("C&ard Database"));
    aOpenCustomFolder->setText(tr("Open custom image folder"));
    aOpenCustomsetsFolder->setText(tr("Open custom sets folder"));
    aAddCustomSet->setText(tr("Add custom sets/cards"));    
    aEditSets->setText(tr("&Edit sets..."));
    aEditTokens->setText(tr("Edit &tokens..."));

    aAbout->setText(tr("&About Cockatrice"));
    aUpdate->setText(tr("&Update Cockatrice"));
    aViewLog->setText(tr("View &debug log"));
    helpMenu->setTitle(tr("&Help"));
    aCheckCardUpdates->setText(tr("Check for card updates..."));
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
    aRegister = new QAction(this);
    connect(aRegister, SIGNAL(triggered()), this, SLOT(actRegister()));
    aSettings = new QAction(this);
    connect(aSettings, SIGNAL(triggered()), this, SLOT(actSettings()));
    aExit = new QAction(this);
    connect(aExit, SIGNAL(triggered()), this, SLOT(actExit()));

    aAbout = new QAction(this);
    connect(aAbout, SIGNAL(triggered()), this, SLOT(actAbout()));
    aUpdate = new QAction(this);
    connect(aUpdate, SIGNAL(triggered()), this, SLOT(actUpdate()));
    aViewLog = new QAction(this);
    connect(aViewLog, SIGNAL(triggered()), this, SLOT(actViewLog()));

    aCheckCardUpdates = new QAction(this);
    connect(aCheckCardUpdates, SIGNAL(triggered()), this, SLOT(actCheckCardUpdates()));

    aOpenCustomsetsFolder = new QAction(QString(), this);
    connect(aOpenCustomsetsFolder, SIGNAL(triggered()), this, SLOT(actOpenCustomsetsFolder()));

    aOpenCustomFolder = new QAction(QString(), this);
    connect(aOpenCustomFolder, SIGNAL(triggered()), this, SLOT(actOpenCustomFolder()));

    aAddCustomSet = new QAction(QString(), this);
    connect(aAddCustomSet, SIGNAL(triggered()), this, SLOT(actAddCustomSet()));

    aEditSets = new QAction(QString(), this);
    connect(aEditSets, SIGNAL(triggered()), this, SLOT(actEditSets()));

    aEditTokens = new QAction(QString(), this);
    connect(aEditTokens, SIGNAL(triggered()), this, SLOT(actEditTokens()));

#if defined(__APPLE__)  /* For OSX */
    aSettings->setMenuRole(QAction::PreferencesRole);
    aExit->setMenuRole(QAction::QuitRole);
    aAbout->setMenuRole(QAction::AboutRole);

    char const * foo; // avoid "warning: expression result unused" under clang
    foo = QT_TRANSLATE_NOOP("QMenuBar","Services");
    foo = QT_TRANSLATE_NOOP("QMenuBar","Hide %1");
    foo = QT_TRANSLATE_NOOP("QMenuBar","Hide Others");
    foo = QT_TRANSLATE_NOOP("QMenuBar","Show All");
    foo = QT_TRANSLATE_NOOP("QMenuBar","Preferences...");
    foo = QT_TRANSLATE_NOOP("QMenuBar","Quit %1");
    foo = QT_TRANSLATE_NOOP("QMenuBar","About %1");
#endif
}

void MainWindow::createMenus()
{
    cockatriceMenu = menuBar()->addMenu(QString());
    cockatriceMenu->addAction(aConnect);
    cockatriceMenu->addAction(aDisconnect);
    cockatriceMenu->addAction(aRegister);
    cockatriceMenu->addSeparator();
    cockatriceMenu->addAction(aSinglePlayer);
    cockatriceMenu->addAction(aWatchReplay);
    cockatriceMenu->addSeparator();
    cockatriceMenu->addAction(aDeckEditor);
    cockatriceMenu->addSeparator();
    cockatriceMenu->addAction(aFullScreen);
    cockatriceMenu->addSeparator();
    cockatriceMenu->addAction(aSettings);
    cockatriceMenu->addAction(aCheckCardUpdates);
    cockatriceMenu->addSeparator();
    cockatriceMenu->addAction(aExit);

    dbMenu = menuBar()->addMenu(QString());
    dbMenu->addAction(aEditSets);
    dbMenu->addAction(aEditTokens);
    dbMenu->addSeparator();
#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
    dbMenu->addAction(aOpenCustomFolder);
    dbMenu->addAction(aOpenCustomsetsFolder);
#endif
    dbMenu->addAction(aAddCustomSet);

    helpMenu = menuBar()->addMenu(QString());
    helpMenu->addAction(aAbout);
    helpMenu->addAction(aUpdate);
    helpMenu->addAction(aViewLog);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), localServer(0), bHasActivated(false), cardUpdateProcess(0)
{
    connect(settingsCache, SIGNAL(pixmapCacheSizeChanged(int)), this, SLOT(pixmapCacheSizeChanged(int)));
    pixmapCacheSizeChanged(settingsCache->getPixmapCacheSize());

    client = new RemoteClient;
    connect(client, SIGNAL(connectionClosedEventReceived(const Event_ConnectionClosed &)), this, SLOT(processConnectionClosedEvent(const Event_ConnectionClosed &)));
    connect(client, SIGNAL(serverShutdownEventReceived(const Event_ServerShutdown &)), this, SLOT(processServerShutdownEvent(const Event_ServerShutdown &)));
    connect(client, SIGNAL(loginError(Response::ResponseCode, QString, quint32, QList<QString>)), this, SLOT(loginError(Response::ResponseCode, QString, quint32, QList<QString>)));
    connect(client, SIGNAL(socketError(const QString &)), this, SLOT(socketError(const QString &)));
    connect(client, SIGNAL(serverTimeout()), this, SLOT(serverTimeout()));
    connect(client, SIGNAL(statusChanged(ClientStatus)), this, SLOT(statusChanged(ClientStatus)));
    connect(client, SIGNAL(protocolVersionMismatch(int, int)), this, SLOT(protocolVersionMismatch(int, int)));
    connect(client, SIGNAL(userInfoChanged(const ServerInfo_User &)), this, SLOT(userInfoReceived(const ServerInfo_User &)), Qt::BlockingQueuedConnection);
    connect(client, SIGNAL(notifyUserAboutUpdate()), this, SLOT(notifyUserAboutUpdate()));
    connect(client, SIGNAL(registerAccepted()), this, SLOT(registerAccepted()));
    connect(client, SIGNAL(registerAcceptedNeedsActivate()), this, SLOT(registerAcceptedNeedsActivate()));
    connect(client, SIGNAL(registerError(Response::ResponseCode, QString, quint32)), this, SLOT(registerError(Response::ResponseCode, QString, quint32)));
    connect(client, SIGNAL(activateAccepted()), this, SLOT(activateAccepted()));
    connect(client, SIGNAL(activateError()), this, SLOT(activateError()));

    clientThread = new QThread(this);
    client->moveToThread(clientThread);
    clientThread->start();

    createActions();
    createMenus();

    tabSupervisor = new TabSupervisor(client);
    connect(tabSupervisor, SIGNAL(setMenu(QList<QMenu *>)), this, SLOT(updateTabMenu(QList<QMenu *>)));
    connect(tabSupervisor, SIGNAL(localGameEnded()), this, SLOT(localGameEnded()));
    connect(tabSupervisor, SIGNAL(showWindowIfHidden()), this, SLOT(showWindowIfHidden()));
    tabSupervisor->addDeckEditorTab(0);

    setCentralWidget(tabSupervisor);

    retranslateUi();

    resize(900, 700);
    restoreGeometry(settingsCache->getMainWindowGeometry());
    aFullScreen->setChecked(windowState() & Qt::WindowFullScreen);

    if (QSystemTrayIcon::isSystemTrayAvailable()) {
        createTrayActions();
        createTrayIcon();
    }

    connect(&settingsCache->shortcuts(), SIGNAL(shortCutchanged()),this,SLOT(refreshShortcuts()));
    refreshShortcuts();

    connect(db, SIGNAL(cardDatabaseLoadingFailed()), this, SLOT(cardDatabaseLoadingFailed()));
    connect(db, SIGNAL(cardDatabaseNewSetsFound(int, QStringList)), this, SLOT(cardDatabaseNewSetsFound(int, QStringList)));
    connect(db, SIGNAL(cardDatabaseAllNewSetsEnabled()), this, SLOT(cardDatabaseAllNewSetsEnabled()));
    QtConcurrent::run(db, &CardDatabase::loadCardDatabases);
}

MainWindow::~MainWindow()
{
    trayIcon->hide();
    trayIcon->deleteLater();
    client->deleteLater();
    clientThread->wait();
}

void MainWindow::createTrayIcon() {
    QMenu *trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(closeAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->setIcon(QPixmap("theme:cockatrice"));
    trayIcon->show();

    connect(trayIcon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,
        SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::DoubleClick) {
        if (windowState() != Qt::WindowMinimized && windowState() != Qt::WindowMinimized + Qt::WindowMaximized)
            showMinimized();
        else {
            showNormal();
            QApplication::setActiveWindow(this);
        }
    }
}


void MainWindow::createTrayActions() {
    closeAction = new QAction(tr("&Exit"), this);
    connect(closeAction, SIGNAL(triggered()), this, SLOT(close()));
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    // workaround Qt bug where closeEvent gets called twice
    static bool bClosingDown=false;
    if(bClosingDown)
        return;
    bClosingDown=true;

    if (!tabSupervisor->closeRequest())
    {
        event->ignore();
        bClosingDown=false;
        return;
    }

    event->accept();
    settingsCache->setMainWindowGeometry(saveGeometry());
    tabSupervisor->deleteLater();
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
    else if(event->type() == QEvent::ActivationChange) {
        if(isActiveWindow() && !bHasActivated){
            bHasActivated = true;
            if(settingsCache->servers().getAutoConnect()) {
                qDebug() << "Attempting auto-connect...";
                 DlgConnect dlg(this);
                 client->connectToServer(dlg.getHost(), dlg.getPort(), dlg.getPlayerName(), dlg.getPassword());
            }
        }
    }

    QMainWindow::changeEvent(event);
}

void MainWindow::pixmapCacheSizeChanged(int newSizeInMBs)
{
    //qDebug() << "Setting pixmap cache size to " << value << " MBs";
    // translate MBs to KBs
    QPixmapCache::setCacheLimit(newSizeInMBs * 1024);
}

void MainWindow::showWindowIfHidden() {
    // keep the previous window state
    setWindowState(windowState() & ~Qt::WindowMinimized);
    show();
}

void MainWindow::cardDatabaseLoadingFailed()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(tr("Card database"));
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText(tr("Cockatrice is unable to load the card database.\n"
        "Do you want to update your card database now?\n"
        "If unsure or first time user, choose \"Yes\""));

    QPushButton *yesButton = msgBox.addButton(tr("Yes"), QMessageBox::YesRole);
    msgBox.addButton(tr("No"), QMessageBox::NoRole);
    QPushButton *settingsButton = msgBox.addButton(tr("Open settings"), QMessageBox::ActionRole);
    msgBox.setDefaultButton(yesButton);

    msgBox.exec();

    if (msgBox.clickedButton() == yesButton) {
        actCheckCardUpdates();
    } else if (msgBox.clickedButton() == settingsButton) {
        actSettings();
    }
}

void MainWindow::cardDatabaseNewSetsFound(int numUnknownSets, QStringList unknownSetsNames)
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(tr("New sets found"));
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText(
        tr("%1 new set(s) found in the card database\n"
        "Set code(s): %2\n"
        "Do you want to enable it/them?"
        ).arg(numUnknownSets).arg(unknownSetsNames.join(", ")));

    QPushButton *yesButton = msgBox.addButton(tr("Yes"), QMessageBox::YesRole);
    QPushButton *noButton = msgBox.addButton(tr("No"), QMessageBox::NoRole);
    QPushButton *settingsButton = msgBox.addButton(tr("View sets"), QMessageBox::ActionRole);
    msgBox.setDefaultButton(yesButton);

    msgBox.exec();

    if (msgBox.clickedButton() == yesButton) {
        db->enableAllUnknownSets();
    } else if (msgBox.clickedButton() == noButton) {
        db->markAllSetsAsKnown();
    } else if (msgBox.clickedButton() == settingsButton) {
        db->markAllSetsAsKnown();
        actEditSets();
    }
}

void MainWindow::cardDatabaseAllNewSetsEnabled()
{
    QMessageBox::information(this, tr("Welcome"), tr("Hi! It seems like you're running this version of Cockatrice for the first time.\nAll the sets in the card database have been enabled.\nRead more about changing the set order or disabling specific sets and consequent effects in the \"Edit Sets\" window."));
    actEditSets();
}

/* CARD UPDATER */

void MainWindow::actCheckCardUpdates()
{
    if(cardUpdateProcess)
    {
        QMessageBox::information(this, tr("Information"), tr("A card database update is already running."));
        return;
    }

    cardUpdateProcess = new QProcess(this);
    connect(cardUpdateProcess, SIGNAL(error(QProcess::ProcessError)), this, SLOT(cardUpdateError(QProcess::ProcessError)));
    connect(cardUpdateProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(cardUpdateFinished(int, QProcess::ExitStatus)));

    // full "run the update" command; leave empty if not present
    QString updaterCmd;
    QString binaryName;
    QDir dir = QDir(QApplication::applicationDirPath());

#if defined(Q_OS_MAC)
    binaryName = getCardUpdaterBinaryName();

    // exit from the application bundle
    dir.cdUp();
    dir.cdUp();
    dir.cdUp();
    dir.cd(binaryName + ".app");
    dir.cd("Contents");
    dir.cd("MacOS");
#elif defined(Q_OS_WIN)
    binaryName = getCardUpdaterBinaryName() + ".exe";
#else
    binaryName = getCardUpdaterBinaryName();
#endif

    if(dir.exists(binaryName))
        updaterCmd = dir.absoluteFilePath(binaryName);

    if(updaterCmd.isEmpty())
    {
        QMessageBox::warning(this, tr("Error"), tr("Unable to run the card database updater: ") + dir.absoluteFilePath(binaryName));
        return;
    }

    cardUpdateProcess->start("\"" + updaterCmd + "\"");
}

void MainWindow::cardUpdateError(QProcess::ProcessError err)
{
    QString error;
    switch(err)
    {
        case QProcess::FailedToStart:
            error = tr("failed to start.");
            break;
        case QProcess::Crashed:
            error = tr("crashed.");
            break;
        case QProcess::Timedout:
            error = tr("timed out.");
            break;
        case QProcess::WriteError:
            error = tr("write error.");
            break;
        case QProcess::ReadError:
            error = tr("read error.");
            break;
        case QProcess::UnknownError:
        default:
            error = tr("unknown error.");
            break;
    }

    cardUpdateProcess->deleteLater();
    cardUpdateProcess = 0;

    QMessageBox::warning(this, tr("Error"), tr("The card database updater exited with an error: %1").arg(error));
}

void MainWindow::cardUpdateFinished(int, QProcess::ExitStatus)
{
    cardUpdateProcess->deleteLater();
    cardUpdateProcess = 0;

    QMessageBox::information(this, tr("Information"), tr("Update completed successfully.\nCockatrice will now reload the card database."));

    QtConcurrent::run(db, &CardDatabase::loadCardDatabases);
}

void MainWindow::refreshShortcuts()
{
    aConnect->setShortcuts(settingsCache->shortcuts().getShortcut("MainWindow/aConnect"));
    aDisconnect->setShortcuts(settingsCache->shortcuts().getShortcut("MainWindow/aDisconnect"));
    aSinglePlayer->setShortcuts(settingsCache->shortcuts().getShortcut("MainWindow/aSinglePlayer"));
    aWatchReplay->setShortcuts(settingsCache->shortcuts().getShortcut("MainWindow/aWatchReplay"));
    aDeckEditor->setShortcuts(settingsCache->shortcuts().getShortcut("MainWindow/aDeckEditor"));
    aFullScreen->setShortcuts(settingsCache->shortcuts().getShortcut("MainWindow/aFullScreen"));
    aRegister->setShortcuts(settingsCache->shortcuts().getShortcut("MainWindow/aRegister"));
    aSettings->setShortcuts(settingsCache->shortcuts().getShortcut("MainWindow/aSettings"));
    aExit->setShortcuts(settingsCache->shortcuts().getShortcut("MainWindow/aExit"));
    aCheckCardUpdates->setShortcuts(settingsCache->shortcuts().getShortcut("MainWindow/aCheckCardUpdates"));
    aOpenCustomFolder->setShortcuts(settingsCache->shortcuts().getShortcut("MainWindow/aOpenCustomFolder"));
    aEditSets->setShortcuts(settingsCache->shortcuts().getShortcut("MainWindow/aEditSets"));
    aEditTokens->setShortcuts(settingsCache->shortcuts().getShortcut("MainWindow/aEditTokens"));
}

void MainWindow::notifyUserAboutUpdate()
{
    QMessageBox::information(this, tr("Information"), tr("Your client appears to be missing features that the server supports.\nThis usually means that your client version is out of date, please check to see if there is a new client available for download."));
}

void MainWindow::actOpenCustomFolder()
{
    QString dir = settingsCache->getCustomPicsPath();
#if defined(Q_OS_MAC)
    QStringList scriptArgs;
    scriptArgs << QLatin1String("-e");
    scriptArgs << QString::fromLatin1("tell application \"Finder\" to open POSIX file \"%1\"").arg(dir);
    scriptArgs << QLatin1String("-e");
    scriptArgs << QLatin1String("tell application \"Finder\" to activate");

    QProcess::execute("/usr/bin/osascript", scriptArgs);
#elif defined(Q_OS_WIN)
    QStringList args;
    args << QDir::toNativeSeparators(dir);
    QProcess::startDetached("explorer", args);
#endif
}

void MainWindow::actOpenCustomsetsFolder()
{
    QString dir = settingsCache->getCustomCardDatabasePath();

#if defined(Q_OS_MAC)
    QStringList scriptArgs;
    scriptArgs << QLatin1String("-e");
    scriptArgs << QString::fromLatin1("tell application \"Finder\" to open POSIX file \"%1\"").arg(dir);
    scriptArgs << QLatin1String("-e");
    scriptArgs << QLatin1String("tell application \"Finder\" to activate");

    QProcess::execute("/usr/bin/osascript", scriptArgs);
#elif defined(Q_OS_WIN)
    QStringList args;
    args << QDir::toNativeSeparators(dir);
    QProcess::startDetached("explorer", args);
#endif
}

void MainWindow::actAddCustomSet()
{
    QFileDialog dialog(this, tr("Load sets/cards"), QDir::homePath());
    dialog.setNameFilters(MainWindow::fileNameFilters);
    if (!dialog.exec())
        return;

    QString fileName = dialog.selectedFiles().at(0);

    if (!QFile::exists(fileName)) {
        QMessageBox::warning(this, tr("Load sets/cards"), tr("Selected file cannot be found."));
        return;
    }

    QDir dir = settingsCache->getCustomCardDatabasePath();
    int nextPrefix = getNextCustomSetPrefix(dir);

    bool res = QFile::copy(
        fileName, dir.absolutePath() + "/" + (nextPrefix > 9 ? "" : "0") +
        QString::number(nextPrefix) + "." + QFileInfo(fileName).fileName()
    );

    if (res) {
        QMessageBox::information(this, tr("Load sets/cards"), tr("The new sets/cards have been added successfully.\nCockatrice will now reload the card database."));
        QtConcurrent::run(db, &CardDatabase::loadCardDatabases);
    } else {
        QMessageBox::warning(this, tr("Load sets/cards"), tr("Sets/cards failed to import."));
    }
}

int MainWindow::getNextCustomSetPrefix(QDir dataDir) {
    QStringList files = dataDir.entryList();
    int maxIndex = 0;

    QStringList::const_iterator filesIterator;
    for (filesIterator = files.constBegin(); filesIterator != files.constEnd(); ++filesIterator) {
        int fileIndex = (*filesIterator).split(".").at(0).toInt();
        if (fileIndex > maxIndex)
            maxIndex = fileIndex;
    }

    return maxIndex + 1;
}

void MainWindow::actEditSets()
{
    WndSets *w = new WndSets;
    w->setWindowModality(Qt::WindowModal);
    w->show();
}

void MainWindow::actEditTokens()
{
    DlgEditTokens dlg;
    dlg.exec();
    db->saveCustomTokensToFile();
}
