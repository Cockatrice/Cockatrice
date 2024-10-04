/***************************************************************************
 *   Copyright (C) 2008 by Max-Wilhelm Bruker                              *
 *   brukie@gmx.net                                                        *
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
#include "window_main.h"

#include "carddatabase.h"
#include "dlg_connect.h"
#include "dlg_edit_tokens.h"
#include "dlg_forgotpasswordchallenge.h"
#include "dlg_forgotpasswordrequest.h"
#include "dlg_forgotpasswordreset.h"
#include "dlg_manage_sets.h"
#include "dlg_register.h"
#include "dlg_settings.h"
#include "dlg_tip_of_the_day.h"
#include "dlg_update.h"
#include "dlg_viewlog.h"
#include "gettextwithmax.h"
#include "localclient.h"
#include "localserver.h"
#include "localserverinterface.h"
#include "logger.h"
#include "main.h"
#include "pb/event_connection_closed.pb.h"
#include "pb/event_server_shutdown.pb.h"
#include "pb/game_replay.pb.h"
#include "pb/room_commands.pb.h"
#include "remoteclient.h"
#include "settingscache.h"
#include "tab_game.h"
#include "tab_supervisor.h"
#include "version_string.h"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QDateTime>
#include <QDesktopServices>
#include <QFile>
#include <QFileDialog>
#include <QInputDialog>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPixmapCache>
#include <QSystemTrayIcon>
#include <QThread>
#include <QTimer>
#include <QWindow>
#include <QtConcurrent>
#include <QtNetwork>

#define GITHUB_PAGES_URL "https://cockatrice.github.io"
#define GITHUB_CONTRIBUTORS_URL "https://github.com/Cockatrice/Cockatrice/graphs/contributors?type=c"
#define GITHUB_CONTRIBUTE_URL "https://github.com/Cockatrice/Cockatrice#cockatrice"
#define GITHUB_TRANSIFEX_TRANSLATORS_URL "https://github.com/Cockatrice/Cockatrice/wiki/Translator-Hall-of-Fame"
#define GITHUB_TRANSLATOR_FAQ_URL "https://github.com/Cockatrice/Cockatrice/wiki/Translation-FAQ"
#define GITHUB_ISSUES_URL "https://github.com/Cockatrice/Cockatrice/issues"
#define GITHUB_TROUBLESHOOTING_URL "https://github.com/Cockatrice/Cockatrice/wiki/Troubleshooting"
#define GITHUB_FAQ_URL "https://github.com/Cockatrice/Cockatrice/wiki/Frequently-Asked-Questions"

const QString MainWindow::appName = "Cockatrice";
const QStringList MainWindow::fileNameFilters = QStringList() << QObject::tr("Cockatrice card database (*.xml)")
                                                              << QObject::tr("All files (*.*)");

void MainWindow::updateTabMenu(const QList<QMenu *> &newMenuList)
{
    for (auto &tabMenu : tabMenus)
        menuBar()->removeAction(tabMenu->menuAction());
    tabMenus = newMenuList;
    for (auto &tabMenu : tabMenus)
        menuBar()->insertMenu(helpMenu->menuAction(), tabMenu);
}

void MainWindow::processConnectionClosedEvent(const Event_ConnectionClosed &event)
{
    client->disconnectFromServer();
    QString reasonStr;
    switch (event.reason()) {
        case Event_ConnectionClosed::USER_LIMIT_REACHED:
            reasonStr = tr("The server has reached its maximum user capacity, please check back later.");
            break;
        case Event_ConnectionClosed::TOO_MANY_CONNECTIONS:
            reasonStr = tr("There are too many concurrent connections from your address.");
            break;
        case Event_ConnectionClosed::BANNED: {
            reasonStr = tr("Banned by moderator");
            if (event.has_end_time())
                reasonStr.append(
                    "\n" + tr("Expected end time: %1").arg(QDateTime::fromSecsSinceEpoch(event.end_time()).toString()));
            else
                reasonStr.append("\n" + tr("This ban lasts indefinitely."));
            if (event.has_reason_str())
                reasonStr.append("\n\n" + QString::fromStdString(event.reason_str()));
            break;
        }
        case Event_ConnectionClosed::SERVER_SHUTDOWN:
            reasonStr = tr("Scheduled server shutdown.");
            break;
        case Event_ConnectionClosed::USERNAMEINVALID:
            reasonStr = tr("Invalid username.");
            break;
        case Event_ConnectionClosed::LOGGEDINELSEWERE:
            reasonStr = tr("You have been logged out due to logging in at another location.");
            break;
        default:
            reasonStr = QString::fromStdString(event.reason_str());
    }
    QMessageBox::critical(this, tr("Connection closed"),
                          tr("The server has terminated your connection.\nReason: %1").arg(reasonStr));
}

void MainWindow::processServerShutdownEvent(const Event_ServerShutdown &event)
{
    serverShutdownMessageBox.setInformativeText(tr("The server is going to be restarted in %n minute(s).\nAll running "
                                                   "games will be lost.\nReason for shutdown: %1",
                                                   "", event.minutes())
                                                    .arg(QString::fromStdString(event.reason())));
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
            aForgotPassword->setEnabled(true);
            break;
        case StatusLoggingIn:
            aSinglePlayer->setEnabled(false);
            aConnect->setEnabled(false);
            aRegister->setEnabled(false);
            aDisconnect->setEnabled(true);
            aForgotPassword->setEnabled(false);
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
    dlgConnect = new DlgConnect(this);
    connect(dlgConnect, SIGNAL(sigStartForgotPasswordRequest()), this, SLOT(actForgotPasswordRequest()));

    if (dlgConnect->exec()) {
        client->connectToServer(dlgConnect->getHost(), static_cast<unsigned int>(dlgConnect->getPort()),
                                dlgConnect->getPlayerName(), dlgConnect->getPassword());
    }
}

void MainWindow::actRegister()
{
    DlgRegister dlg(this);
    if (dlg.exec()) {
        client->registerToServer(dlg.getHost(), static_cast<unsigned int>(dlg.getPort()), dlg.getPlayerName(),
                                 dlg.getPassword(), dlg.getEmail(), dlg.getCountry(), dlg.getRealName());
    }
}

void MainWindow::actDisconnect()
{
    client->disconnectFromServer();
}

void MainWindow::actSinglePlayer()
{
    bool ok;
    int numberPlayers =
        QInputDialog::getInt(this, tr("Number of players"), tr("Please enter the number of players."), 1, 1, 8, 1, &ok);
    if (!ok)
        return;

    aConnect->setEnabled(false);
    aRegister->setEnabled(false);
    aForgotPassword->setEnabled(false);
    aSinglePlayer->setEnabled(false);

    localServer = new LocalServer(this);
    LocalServerInterface *mainLsi = localServer->newConnection();
    LocalClient *mainClient =
        new LocalClient(mainLsi, tr("Player %1").arg(1), SettingsCache::instance().getClientID(), this);
    QList<AbstractClient *> localClients;
    localClients.append(mainClient);

    for (int i = 0; i < numberPlayers - 1; ++i) {
        LocalServerInterface *slaveLsi = localServer->newConnection();
        LocalClient *slaveClient =
            new LocalClient(slaveLsi, tr("Player %1").arg(i + 2), SettingsCache::instance().getClientID(), this);
        localClients.append(slaveClient);
    }
    tabSupervisor->startLocal(localClients);

    Command_CreateGame createCommand;
    createCommand.set_max_players(static_cast<google::protobuf::uint32>(numberPlayers));
    mainClient->sendCommand(LocalClient::prepareRoomCommand(createCommand, 0));
}

void MainWindow::actWatchReplay()
{
    QFileDialog dlg(this, tr("Load replay"));
    dlg.setDirectory(SettingsCache::instance().getReplaysPath());
    dlg.setNameFilters(QStringList() << QObject::tr("Cockatrice replays (*.cor)"));
    if (!dlg.exec())
        return;

    QString fileName = dlg.selectedFiles().at(0);
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
        return;
    QByteArray buf = file.readAll();
    file.close();

    replay = new GameReplay;
    replay->ParseFromArray(buf.data(), buf.size());

    tabSupervisor->openReplay(replay);
}

void MainWindow::localGameEnded()
{
    delete localServer;
    localServer = nullptr;

    aConnect->setEnabled(true);
    aRegister->setEnabled(true);
    aForgotPassword->setEnabled(true);
    aSinglePlayer->setEnabled(true);
}

void MainWindow::actDeckEditor()
{
    tabSupervisor->addDeckEditorTab(nullptr);
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
    QMessageBox mb(
        QMessageBox::NoIcon, tr("About Cockatrice"),
        QString("<font size=\"8\"><b>Cockatrice</b></font> (" + QString::fromStdString(BUILD_ARCHITECTURE) + ")<br>" +
                tr("Version") + QString(" %1").arg(VERSION_STRING) + "<br><br><b><a href='" + GITHUB_PAGES_URL + "'>" +
                tr("Cockatrice Webpage") + "</a></b><br>" + "<br><b>" + tr("Project Manager:") +
                "</b><br>Zach Halpern<br><br>" + "<b>" + tr("Past Project Managers:") +
                "</b><br>Gavin Bisesi<br>Max-Wilhelm Bruker<br>Marcus Schütz<br><br>" + "<b>" + tr("Developers:") +
                "</b><br>" + "<a href='" + GITHUB_CONTRIBUTORS_URL + "'>" + tr("Our Developers") + "</a><br>" +
                "<a href='" + GITHUB_CONTRIBUTE_URL + "'>" + tr("Help Develop!") + "</a><br><br>" + "<b>" +
                tr("Translators:") + "</b><br>" + "<a href='" + GITHUB_TRANSIFEX_TRANSLATORS_URL + "'>" +
                tr("Our Translators") + "</a><br>" + "<a href='" + GITHUB_TRANSLATOR_FAQ_URL + "'>" +
                tr("Help Translate!") + "</a><br><br>" + "<b>" + tr("Support:") + "</b><br>" + "<a href='" +
                GITHUB_ISSUES_URL + "'>" + tr("Report an Issue") + "</a><br>" + "<a href='" +
                GITHUB_TROUBLESHOOTING_URL + "'>" + tr("Troubleshooting") + "</a><br>" + "<a href='" + GITHUB_FAQ_URL +
                "'>" + tr("F.A.Q.") + "</a><br>"),
        QMessageBox::Ok, this);
    mb.setIconPixmap(QPixmap("theme:cockatrice").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    mb.setTextInteractionFlags(Qt::TextBrowserInteraction);
    mb.exec();
}

void MainWindow::actTips()
{
    if (tip != nullptr) {
        delete tip;
        tip = nullptr;
    }
    tip = new DlgTipOfTheDay(this);
    if (tip->successfulInit) {
        tip->show();
    }
}

void MainWindow::actUpdate()
{
    DlgUpdate dlg(this);
    dlg.exec();
}

void MainWindow::actViewLog()
{
    if (logviewDialog == nullptr) {
        logviewDialog = new DlgViewLog(this);
    }

    logviewDialog->show();
    logviewDialog->raise();
    logviewDialog->activateWindow();
}

void MainWindow::serverTimeout()
{
    QMessageBox::critical(this, tr("Error"), tr("Server timeout"));
    actConnect();
}

void MainWindow::loginError(Response::ResponseCode r,
                            QString reasonStr,
                            quint32 endTime,
                            QList<QString> missingFeatures)
{
    switch (r) {
        case Response::RespClientUpdateRequired: {
            QString formattedMissingFeatures;
            formattedMissingFeatures = "Missing Features: ";
            for (int i = 0; i < missingFeatures.size(); ++i)
                formattedMissingFeatures.append(QString("\n     %1").arg(QChar(0x2022)) + " " +
                                                missingFeatures.value(i));

            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.setWindowTitle(tr("Failed Login"));
            msgBox.setText(tr("Your client seems to be missing features this server requires for connection.") +
                           "\n\n" + tr("To update your client, go to 'Help -> Check for Client Updates'."));
            msgBox.setDetailedText(formattedMissingFeatures);
            msgBox.exec();
            break;
        }
        case Response::RespWrongPassword:
            QMessageBox::critical(
                this, tr("Error"),
                tr("Incorrect username or password. Please check your authentication information and try again."));
            break;
        case Response::RespWouldOverwriteOldSession:
            QMessageBox::critical(this, tr("Error"),
                                  tr("There is already an active session using this user name.\nPlease close that "
                                     "session first and re-login."));
            break;
        case Response::RespUserIsBanned: {
            QString bannedStr;
            if (endTime)
                bannedStr = tr("You are banned until %1.").arg(QDateTime::fromSecsSinceEpoch(endTime).toString());
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
            if (QMessageBox::question(this, tr("Error"),
                                      tr("This server requires user registration. Do you want to register now?"),
                                      QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
                actRegister();
            }
            break;
        case Response::RespClientIdRequired:
            QMessageBox::critical(
                this, tr("Error"),
                tr("This server requires client IDs. Your client is either failing to generate an ID or you are "
                   "running a modified client.\nPlease close and reopen your client to try again."));
            break;
        case Response::RespContextError:
            QMessageBox::critical(this, tr("Error"),
                                  tr("An internal error has occurred, please close and reopen Cockatrice before trying "
                                     "again.\nIf the error persists, ensure you are running the latest version of the "
                                     "software and if needed contact the software developers."));
            break;
        case Response::RespAccountNotActivated: {
            bool ok = false;
            QString token = getTextWithMax(this, tr("Account activation"),
                                           tr("Your account has not been activated yet.\nYou need to provide "
                                              "the activation token received in the activation email."),
                                           QLineEdit::Normal, QString(), &ok);
            if (ok && !token.isEmpty()) {
                client->activateToServer(token);
                return;
            }
            client->disconnectFromServer();
            break;
        }
        case Response::RespServerFull: {
            QMessageBox::critical(this, tr("Server Full"),
                                  tr("The server has reached its maximum user capacity, please check back later."));
            break;
        }
        default:
            QMessageBox::critical(this, tr("Error"),
                                  tr("Unknown login error: %1").arg(static_cast<int>(r)) +
                                      tr("\nThis usually means that your client version is out of date, and the server "
                                         "sent a reply your client doesn't understand."));
            break;
    }
    actConnect();
}

QString MainWindow::extractInvalidUsernameMessage(QString &in)
{
    QString out = tr("Invalid username.") + "<br/>";
    QStringList rules = in.split(QChar('|'));
    if (rules.size() == 7 || rules.size() == 9) {
        out += tr("Your username must respect these rules:") + "<ul>";

        out += "<li>" + tr("is %1 - %2 characters long").arg(rules.at(0)).arg(rules.at(1)) + "</li>";
        out += "<li>" + tr("can %1 contain lowercase characters").arg((rules.at(2).toInt() > 0) ? "" : tr("NOT")) +
               "</li>";
        out += "<li>" + tr("can %1 contain uppercase characters").arg((rules.at(3).toInt() > 0) ? "" : tr("NOT")) +
               "</li>";
        out +=
            "<li>" + tr("can %1 contain numeric characters").arg((rules.at(4).toInt() > 0) ? "" : tr("NOT")) + "</li>";

        if (rules.at(6).size() > 0)
            out += "<li>" + tr("can contain the following punctuation: %1").arg(rules.at(6).toHtmlEscaped()) + "</li>";

        out += "<li>" +
               tr("first character can %1 be a punctuation mark").arg((rules.at(5).toInt() > 0) ? "" : tr("NOT")) +
               "</li>";

        if (rules.size() == 9) {
            if (rules.at(7).size() > 0) {
                QString words = rules.at(7).toHtmlEscaped();
                if (words.startsWith("\n")) {
                    out += tr("no unacceptable language as specified by these server rules:",
                              "note that the following lines will not be translated");
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
                    for (QString &line : words.split("\n", Qt::SkipEmptyParts)) {
#else
                    for (QString &line : words.split("\n", QString::SkipEmptyParts)) {
#endif
                        out += "<li>" + line + "</li>";
                    }
                } else {
                    out += "<li>" + tr("can not contain any of the following words: %1").arg(words) + "</li>";
                }
            }

            if (rules.at(8).size() > 0)
                out += "<li>" +
                       tr("can not match any of the following expressions: %1").arg(rules.at(8).toHtmlEscaped()) +
                       "</li>";
        }

        out += "</ul>";
    } else {
        out += tr("You may only use A-Z, a-z, 0-9, _, ., and - in your username.");
    }

    return out;
}

void MainWindow::registerError(Response::ResponseCode r, QString reasonStr, quint32 endTime)
{
    switch (r) {
        case Response::RespRegistrationDisabled:
            QMessageBox::critical(this, tr("Registration denied"),
                                  tr("Registration is currently disabled on this server"));
            break;
        case Response::RespUserAlreadyExists:
            QMessageBox::critical(this, tr("Registration denied"),
                                  tr("There is already an existing account with the same user name."));
            break;
        case Response::RespEmailRequiredToRegister:
            QMessageBox::critical(this, tr("Registration denied"),
                                  tr("It's mandatory to specify a valid email address when registering."));
            break;
        case Response::RespEmailBlackListed:
            if (reasonStr.isEmpty()) {
                reasonStr =
                    "The email address provider used during registration has been blocked from use on this server.";
            }
            QMessageBox::critical(this, tr("Registration denied"), reasonStr);
            break;
        case Response::RespTooManyRequests:
            QMessageBox::critical(
                this, tr("Registration denied"),
                tr("It appears you are attempting to register a new account on this server yet you already have an "
                   "account registered with the email provided. This server restricts the number of accounts a user "
                   "can register per address.  Please contact the server operator for further assistance or to obtain "
                   "your credential information."));
            break;
        case Response::RespPasswordTooShort:
            QMessageBox::critical(this, tr("Registration denied"), tr("Password too short."));
            break;
        case Response::RespUserIsBanned: {
            QString bannedStr;
            if (endTime)
                bannedStr = tr("You are banned until %1.").arg(QDateTime::fromSecsSinceEpoch(endTime).toString());
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
            QMessageBox::critical(this, tr("Error"),
                                  tr("Unknown registration error: %1").arg(static_cast<int>(r)) +
                                      tr("\nThis usually means that your client version is out of date, and the server "
                                         "sent a reply your client doesn't understand."));
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
        QMessageBox::critical(this, tr("Error"),
                              tr("You are trying to connect to an obsolete server. Please downgrade your Cockatrice "
                                 "version or connect to a suitable server.\nLocal version is %1, remote version is %2.")
                                  .arg(localVersion)
                                  .arg(remoteVersion));
    else
        QMessageBox::critical(this, tr("Error"),
                              tr("Your Cockatrice client is obsolete. Please update your Cockatrice version.\nLocal "
                                 "version is %1, remote version is %2.")
                                  .arg(localVersion)
                                  .arg(remoteVersion));
}

void MainWindow::setClientStatusTitle()
{
    switch (client->getStatus()) {
        case StatusConnecting:
            setWindowTitle(appName + " - " + tr("Connecting to %1...").arg(client->peerName()));
            break;
        case StatusRegistering:
            setWindowTitle(appName + " - " +
                           tr("Registering to %1 as %2...").arg(client->peerName()).arg(client->getUserName()));
            break;
        case StatusDisconnected:
            setWindowTitle(appName + " - " + tr("Disconnected"));
            break;
        case StatusLoggingIn:
            setWindowTitle(appName + " - " + tr("Connected, logging in at %1").arg(client->peerName()));
            break;
        case StatusLoggedIn:
            setWindowTitle(client->getUserName() + "@" + client->peerName());
            break;
        case StatusRequestingForgotPassword:
            setWindowTitle(
                appName + " - " +
                tr("Requesting forgotten password to %1 as %2...").arg(client->peerName()).arg(client->getUserName()));
            break;
        case StatusSubmitForgotPasswordChallenge:
            setWindowTitle(
                appName + " - " +
                tr("Requesting forgotten password to %1 as %2...").arg(client->peerName()).arg(client->getUserName()));
            break;
        case StatusSubmitForgotPasswordReset:
            setWindowTitle(
                appName + " - " +
                tr("Requesting forgotten password to %1 as %2...").arg(client->peerName()).arg(client->getUserName()));
            break;
        default:
            setWindowTitle(appName);
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
    aForgotPassword->setText(tr("&Restore password..."));
    aSettings->setText(tr("&Settings..."));
    aSettings->setIcon(QPixmap("theme:icons/settings"));
    aExit->setText(tr("&Exit"));

#if defined(__APPLE__) /* For OSX */
    cockatriceMenu->setTitle(tr("A&ctions"));
#else
    cockatriceMenu->setTitle(tr("&Cockatrice"));
#endif

    dbMenu->setTitle(tr("C&ard Database"));
    aManageSets->setText(tr("&Manage sets..."));
    aEditTokens->setText(tr("Edit custom &tokens..."));
    aOpenCustomFolder->setText(tr("Open custom image folder"));
    aOpenCustomsetsFolder->setText(tr("Open custom sets folder"));
    aAddCustomSet->setText(tr("Add custom sets/cards"));

    helpMenu->setTitle(tr("&Help"));
    aAbout->setText(tr("&About Cockatrice"));
    aTips->setText(tr("&Tip of the Day"));
    aUpdate->setText(tr("Check for Client Updates"));
    aCheckCardUpdates->setText(tr("Check for Card Updates..."));
    aViewLog->setText(tr("View &Debug Log"));

    aShow->setText(tr("Show/Hide"));

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
    aForgotPassword = new QAction(this);
    connect(aForgotPassword, SIGNAL(triggered()), this, SLOT(actForgotPasswordRequest()));
    aSettings = new QAction(this);
    connect(aSettings, SIGNAL(triggered()), this, SLOT(actSettings()));
    aExit = new QAction(this);
    connect(aExit, SIGNAL(triggered()), this, SLOT(actExit()));

    aManageSets = new QAction(QString(), this);
    connect(aManageSets, SIGNAL(triggered()), this, SLOT(actManageSets()));
    aEditTokens = new QAction(QString(), this);
    connect(aEditTokens, SIGNAL(triggered()), this, SLOT(actEditTokens()));
    aOpenCustomFolder = new QAction(QString(), this);
    connect(aOpenCustomFolder, SIGNAL(triggered()), this, SLOT(actOpenCustomFolder()));
    aOpenCustomsetsFolder = new QAction(QString(), this);
    connect(aOpenCustomsetsFolder, SIGNAL(triggered()), this, SLOT(actOpenCustomsetsFolder()));
    aAddCustomSet = new QAction(QString(), this);
    connect(aAddCustomSet, SIGNAL(triggered()), this, SLOT(actAddCustomSet()));

    aAbout = new QAction(this);
    connect(aAbout, SIGNAL(triggered()), this, SLOT(actAbout()));
    aTips = new QAction(this);
    connect(aTips, SIGNAL(triggered()), this, SLOT(actTips()));
    aUpdate = new QAction(this);
    connect(aUpdate, SIGNAL(triggered()), this, SLOT(actUpdate()));
    aCheckCardUpdates = new QAction(this);
    connect(aCheckCardUpdates, SIGNAL(triggered()), this, SLOT(actCheckCardUpdates()));
    aViewLog = new QAction(this);
    connect(aViewLog, SIGNAL(triggered()), this, SLOT(actViewLog()));

    aShow = new QAction(this);
    connect(aShow, SIGNAL(triggered()), this, SLOT(actShow()));

#if defined(__APPLE__) /* For OSX */
    aSettings->setMenuRole(QAction::PreferencesRole);
    aExit->setMenuRole(QAction::QuitRole);
    aAbout->setMenuRole(QAction::AboutRole);

    Q_UNUSED(QT_TRANSLATE_NOOP("QMenuBar", "Services"));
    Q_UNUSED(QT_TRANSLATE_NOOP("QMenuBar", "Hide %1"));
    Q_UNUSED(QT_TRANSLATE_NOOP("QMenuBar", "Hide Others"));
    Q_UNUSED(QT_TRANSLATE_NOOP("QMenuBar", "Show All"));
    Q_UNUSED(QT_TRANSLATE_NOOP("QMenuBar", "Preferences..."));
    Q_UNUSED(QT_TRANSLATE_NOOP("QMenuBar", "Quit %1"));
    Q_UNUSED(QT_TRANSLATE_NOOP("QMenuBar", "About %1"));
#endif
    // translate Qt's dialogs "default button text"; list taken from QPlatformTheme::defaultStandardButtonText()
    Q_UNUSED(QT_TRANSLATE_NOOP("QPlatformTheme", "OK"));
    Q_UNUSED(QT_TRANSLATE_NOOP("QPlatformTheme", "Save"));
    Q_UNUSED(QT_TRANSLATE_NOOP("QPlatformTheme", "Save All"));
    Q_UNUSED(QT_TRANSLATE_NOOP("QPlatformTheme", "Open"));
    Q_UNUSED(QT_TRANSLATE_NOOP("QPlatformTheme", "&Yes"));
    Q_UNUSED(QT_TRANSLATE_NOOP("QPlatformTheme", "Yes to &All"));
    Q_UNUSED(QT_TRANSLATE_NOOP("QPlatformTheme", "&No"));
    Q_UNUSED(QT_TRANSLATE_NOOP("QPlatformTheme", "N&o to All"));
    Q_UNUSED(QT_TRANSLATE_NOOP("QPlatformTheme", "Abort"));
    Q_UNUSED(QT_TRANSLATE_NOOP("QPlatformTheme", "Retry"));
    Q_UNUSED(QT_TRANSLATE_NOOP("QPlatformTheme", "Ignore"));
    Q_UNUSED(QT_TRANSLATE_NOOP("QPlatformTheme", "Close"));
    Q_UNUSED(QT_TRANSLATE_NOOP("QPlatformTheme", "Cancel"));
    Q_UNUSED(QT_TRANSLATE_NOOP("QPlatformTheme", "Discard"));
    Q_UNUSED(QT_TRANSLATE_NOOP("QPlatformTheme", "Help"));
    Q_UNUSED(QT_TRANSLATE_NOOP("QPlatformTheme", "Apply"));
    Q_UNUSED(QT_TRANSLATE_NOOP("QPlatformTheme", "Reset"));
    Q_UNUSED(QT_TRANSLATE_NOOP("QPlatformTheme", "Restore Defaults"));
}

void MainWindow::createMenus()
{
    cockatriceMenu = menuBar()->addMenu(QString());
    cockatriceMenu->addAction(aConnect);
    cockatriceMenu->addAction(aDisconnect);
    cockatriceMenu->addAction(aRegister);
    cockatriceMenu->addAction(aForgotPassword);
    cockatriceMenu->addSeparator();
    cockatriceMenu->addAction(aSinglePlayer);
    cockatriceMenu->addAction(aWatchReplay);
    cockatriceMenu->addSeparator();
    cockatriceMenu->addAction(aDeckEditor);
    cockatriceMenu->addSeparator();
    cockatriceMenu->addAction(aFullScreen);
    cockatriceMenu->addSeparator();
    cockatriceMenu->addAction(aSettings);
    cockatriceMenu->addAction(aExit);

    dbMenu = menuBar()->addMenu(QString());
    dbMenu->addAction(aManageSets);
    dbMenu->addAction(aEditTokens);
    dbMenu->addSeparator();
    dbMenu->addAction(aOpenCustomFolder);
    dbMenu->addAction(aOpenCustomsetsFolder);
    dbMenu->addAction(aAddCustomSet);

    helpMenu = menuBar()->addMenu(QString());
    helpMenu->addAction(aAbout);
    helpMenu->addAction(aTips);
    helpMenu->addSeparator();
    helpMenu->addAction(aUpdate);
    helpMenu->addAction(aCheckCardUpdates);
    helpMenu->addSeparator();
    helpMenu->addAction(aViewLog);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), localServer(nullptr), bHasActivated(false), askedForDbUpdater(false),
      cardUpdateProcess(nullptr), logviewDialog(nullptr)
{
    connect(&SettingsCache::instance(), SIGNAL(pixmapCacheSizeChanged(int)), this, SLOT(pixmapCacheSizeChanged(int)));
    pixmapCacheSizeChanged(SettingsCache::instance().getPixmapCacheSize());

    client = new RemoteClient;
    connect(client, SIGNAL(connectionClosedEventReceived(const Event_ConnectionClosed &)), this,
            SLOT(processConnectionClosedEvent(const Event_ConnectionClosed &)));
    connect(client, SIGNAL(serverShutdownEventReceived(const Event_ServerShutdown &)), this,
            SLOT(processServerShutdownEvent(const Event_ServerShutdown &)));
    connect(client, SIGNAL(loginError(Response::ResponseCode, QString, quint32, QList<QString>)), this,
            SLOT(loginError(Response::ResponseCode, QString, quint32, QList<QString>)));
    connect(client, SIGNAL(socketError(const QString &)), this, SLOT(socketError(const QString &)));
    connect(client, SIGNAL(serverTimeout()), this, SLOT(serverTimeout()));
    connect(client, SIGNAL(statusChanged(ClientStatus)), this, SLOT(statusChanged(ClientStatus)));
    connect(client, SIGNAL(protocolVersionMismatch(int, int)), this, SLOT(protocolVersionMismatch(int, int)));
    connect(client, SIGNAL(userInfoChanged(const ServerInfo_User &)), this,
            SLOT(userInfoReceived(const ServerInfo_User &)), Qt::BlockingQueuedConnection);
    connect(client, SIGNAL(notifyUserAboutUpdate()), this, SLOT(notifyUserAboutUpdate()));
    connect(client, SIGNAL(registerAccepted()), this, SLOT(registerAccepted()));
    connect(client, SIGNAL(registerAcceptedNeedsActivate()), this, SLOT(registerAcceptedNeedsActivate()));
    connect(client, SIGNAL(registerError(Response::ResponseCode, QString, quint32)), this,
            SLOT(registerError(Response::ResponseCode, QString, quint32)));
    connect(client, SIGNAL(activateAccepted()), this, SLOT(activateAccepted()));
    connect(client, SIGNAL(activateError()), this, SLOT(activateError()));
    connect(client, SIGNAL(sigForgotPasswordSuccess()), this, SLOT(forgotPasswordSuccess()));
    connect(client, SIGNAL(sigForgotPasswordError()), this, SLOT(forgotPasswordError()));
    connect(client, SIGNAL(sigPromptForForgotPasswordReset()), this, SLOT(promptForgotPasswordReset()));
    connect(client, SIGNAL(sigPromptForForgotPasswordChallenge()), this, SLOT(promptForgotPasswordChallenge()));

    clientThread = new QThread(this);
    client->moveToThread(clientThread);
    clientThread->start();

    createActions();
    createMenus();

    tabSupervisor = new TabSupervisor(client);
    connect(tabSupervisor, SIGNAL(setMenu(QList<QMenu *>)), this, SLOT(updateTabMenu(QList<QMenu *>)));
    connect(tabSupervisor, SIGNAL(localGameEnded()), this, SLOT(localGameEnded()));
    connect(tabSupervisor, SIGNAL(showWindowIfHidden()), this, SLOT(showWindowIfHidden()));
    tabSupervisor->addDeckEditorTab(nullptr);

    setCentralWidget(tabSupervisor);

    retranslateUi();

    if (!restoreGeometry(SettingsCache::instance().getMainWindowGeometry())) {
        setWindowState(Qt::WindowMaximized);
    }
    aFullScreen->setChecked(static_cast<bool>(windowState() & Qt::WindowFullScreen));

    if (QSystemTrayIcon::isSystemTrayAvailable()) {
        createTrayIcon();
    }

    connect(&SettingsCache::instance().shortcuts(), SIGNAL(shortCutChanged()), this, SLOT(refreshShortcuts()));
    refreshShortcuts();

    connect(db, SIGNAL(cardDatabaseLoadingFailed()), this, SLOT(cardDatabaseLoadingFailed()));
    connect(db, SIGNAL(cardDatabaseNewSetsFound(int, QStringList)), this,
            SLOT(cardDatabaseNewSetsFound(int, QStringList)));
    connect(db, SIGNAL(cardDatabaseAllNewSetsEnabled()), this, SLOT(cardDatabaseAllNewSetsEnabled()));

    tip = new DlgTipOfTheDay();

    // run startup check async
    QTimer::singleShot(0, this, &MainWindow::startupConfigCheck);
}

void MainWindow::startupConfigCheck()
{
    if (SettingsCache::instance().getClientVersion() == CLIENT_INFO_NOT_SET) {
        // no config found, 99% new clean install
        qDebug() << "Startup: old client version empty, assuming first start after clean install";
        alertForcedOracleRun(VERSION_STRING, false);
        SettingsCache::instance().setClientVersion(VERSION_STRING);
    } else if (SettingsCache::instance().getClientVersion() != VERSION_STRING) {
        // config found, from another (presumably older) version
        qDebug() << "Startup: old client version" << SettingsCache::instance().getClientVersion()
                 << "differs, assuming first start after update";
        if (SettingsCache::instance().getNotifyAboutNewVersion()) {
            alertForcedOracleRun(VERSION_STRING, true);
        } else {
            const auto reloadOk0 = QtConcurrent::run([] { db->loadCardDatabases(); });
        }
        SettingsCache::instance().setClientVersion(VERSION_STRING);
    } else {
        // previous config from this version found
        qDebug() << "Startup: found config with current version";
        const auto reloadOk1 = QtConcurrent::run([] { db->loadCardDatabases(); });

        // Run the tips dialog only on subsequent startups.
        // On the first run after an install/update the startup is already crowded enough
        if (tip->successfulInit && SettingsCache::instance().getShowTipsOnStartup() && tip->newTipsAvailable) {
            tip->raise();
            tip->show();
        }
    }
}

void MainWindow::alertForcedOracleRun(const QString &version, bool isUpdate)
{
    if (isUpdate) {
        QMessageBox::information(this, tr("New Version"),
                                 tr("Congratulations on updating to Cockatrice %1!\n"
                                    "Oracle will now launch to update your card database.")
                                     .arg(version));
    } else {
        QMessageBox::information(this, tr("Cockatrice installed"),
                                 tr("Congratulations on installing Cockatrice %1!\n"
                                    "Oracle will now launch to install the initial card database.")
                                     .arg(version));
    }

    actCheckCardUpdates();
    actCheckServerUpdates();
}

MainWindow::~MainWindow()
{
    if (tip != nullptr) {
        delete tip;
        tip = nullptr;
    }
    if (trayIcon) {
        trayIcon->hide();
        trayIcon->deleteLater();
    }

    client->deleteLater();
    clientThread->wait();
}

void MainWindow::createTrayIcon()
{
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(aShow);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(aSettings);
    trayIconMenu->addAction(aCheckCardUpdates);
    trayIconMenu->addAction(aAbout);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(aExit);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->setIcon(QPixmap("theme:cockatrice"));
    trayIcon->show();
}

void MainWindow::actShow()
{
    // wait 50 msec before actually checking the active window, this is because the trayicon menu will actually take
    // focus and we have to wait for the focus to come back to the application
    QTimer::singleShot(50, this, [this]() {
        if (isActiveWindow()) {
            showMinimized();
        } else {
            showNormal();
            activateWindow();
        }
    });
}

void MainWindow::promptForgotPasswordChallenge()
{
    DlgForgotPasswordChallenge dlg(this);
    if (dlg.exec())
        client->submitForgotPasswordChallengeToServer(dlg.getHost(), static_cast<unsigned int>(dlg.getPort()),
                                                      dlg.getPlayerName(), dlg.getEmail());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // workaround Qt bug where closeEvent gets called twice
    static bool bClosingDown = false;
    if (bClosingDown)
        return;
    bClosingDown = true;

    if (!tabSupervisor->closeRequest()) {
        event->ignore();
        bClosingDown = false;
        return;
    }
    tip->close();

    event->accept();
    SettingsCache::instance().setMainWindowGeometry(saveGeometry());
    tabSupervisor->deleteLater();
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
    else if (event->type() == QEvent::ActivationChange) {
        if (isActiveWindow() && !bHasActivated) {
            bHasActivated = true;
            if (!connectTo.isEmpty()) {
                qDebug() << "Command line connect to " << connectTo;
                client->connectToServer(connectTo.host(), connectTo.port(), connectTo.userName(), connectTo.password());
            } else if (SettingsCache::instance().servers().getAutoConnect()) {
                qDebug() << "Attempting auto-connect...";
                DlgConnect dlg(this);
                client->connectToServer(dlg.getHost(), static_cast<unsigned int>(dlg.getPort()), dlg.getPlayerName(),
                                        dlg.getPassword());
            }
        }
    }

    QMainWindow::changeEvent(event);
}

void MainWindow::pixmapCacheSizeChanged(int newSizeInMBs)
{
    // qDebug() << "Setting pixmap cache size to " << value << " MBs";
    // translate MBs to KBs
    QPixmapCache::setCacheLimit(newSizeInMBs * 1024);
}

void MainWindow::showWindowIfHidden()
{
    // keep the previous window state
    setWindowState(windowState() & ~Qt::WindowMinimized);
    show();
}

void MainWindow::cardDatabaseLoadingFailed()
{
    if (askedForDbUpdater) {
        return;
    }
    askedForDbUpdater = true;
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
    msgBox.setText(tr("%n new set(s) found in the card database\n"
                      "Set code(s): %1\n"
                      "Do you want to enable it/them?",
                      "", numUnknownSets)
                       .arg(unknownSetsNames.join(", ")));

    QPushButton *yesButton = msgBox.addButton(tr("Yes"), QMessageBox::YesRole);
    QPushButton *noButton = msgBox.addButton(tr("No"), QMessageBox::NoRole);
    QPushButton *settingsButton = msgBox.addButton(tr("View sets"), QMessageBox::ActionRole);
    msgBox.setDefaultButton(yesButton);

    msgBox.exec();

    if (msgBox.clickedButton() == yesButton) {
        db->enableAllUnknownSets();
        const auto reloadOk1 = QtConcurrent::run([] { db->loadCardDatabases(); });
    } else if (msgBox.clickedButton() == noButton) {
        db->markAllSetsAsKnown();
    } else if (msgBox.clickedButton() == settingsButton) {
        db->markAllSetsAsKnown();
        actManageSets();
    }
}

void MainWindow::cardDatabaseAllNewSetsEnabled()
{
    QMessageBox::information(
        this, tr("Welcome"),
        tr("Hi! It seems like you're running this version of Cockatrice for the first time.\nAll the sets in the card "
           "database have been enabled.\nRead more about changing the set order or disabling specific sets and "
           "consequent effects in the \"Manage Sets\" dialog."));
    actManageSets();
}

/* CARD UPDATER */
void MainWindow::actCheckCardUpdates()
{
    if (cardUpdateProcess) {
        QMessageBox::information(this, tr("Information"), tr("A card database update is already running."));
        return;
    }

    cardUpdateProcess = new QProcess(this);

    connect(cardUpdateProcess, &QProcess::errorOccurred, this, &MainWindow::cardUpdateError);

    connect(cardUpdateProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this,
            SLOT(cardUpdateFinished(int, QProcess::ExitStatus)));

    // full "run the update" command; leave empty if not present
    QString updaterCmd;
    QString binaryName;
    QDir dir = QDir(QApplication::applicationDirPath());

#if defined(Q_OS_MAC)
    /*
     * bypass app translocation: quarantined application will be started from a temporary directory eg.
     * /private/var/folders/tk/qx76cyb50jn5dvj7rrgfscz40000gn/T/AppTranslocation/A0CBBD5A-9264-4106-8547-36B84DB161E2/d/oracle/
     */
    if (dir.absolutePath().startsWith("/private/var/folders")) {
        dir.setPath("/Applications/");
    } else {
        // exit from the Cockatrice application bundle
        dir.cdUp();
        dir.cdUp();
        dir.cdUp();
    }

    binaryName = getCardUpdaterBinaryName();

    dir.cd(binaryName + ".app");
    dir.cd("Contents");
    dir.cd("MacOS");
#elif defined(Q_OS_WIN)
    binaryName = getCardUpdaterBinaryName() + ".exe";
#else
    binaryName = getCardUpdaterBinaryName();
#endif

    if (dir.exists(binaryName)) {
        updaterCmd = dir.absoluteFilePath(binaryName);
    } else { // try and find the directory oracle is stored in the build directory
        QDir findLocalDir(dir);
        findLocalDir.cdUp();
        findLocalDir.cd(getCardUpdaterBinaryName());
        if (findLocalDir.exists(binaryName)) {
            dir = findLocalDir;
            updaterCmd = dir.absoluteFilePath(binaryName);
        }
    }

    if (updaterCmd.isEmpty()) {
        QMessageBox::warning(this, tr("Error"),
                             tr("Unable to run the card database updater: ") + dir.absoluteFilePath(binaryName));
        exitCardDatabaseUpdate();
        return;
    }

    cardUpdateProcess->start(updaterCmd, QStringList());
}

void MainWindow::exitCardDatabaseUpdate()
{
    cardUpdateProcess->deleteLater();
    cardUpdateProcess = nullptr;

    const auto reloadOk1 = QtConcurrent::run([] { db->loadCardDatabases(); });
}

void MainWindow::cardUpdateError(QProcess::ProcessError err)
{
    QString error;
    switch (err) {
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

    exitCardDatabaseUpdate();
    QMessageBox::warning(this, tr("Error"), tr("The card database updater exited with an error: %1").arg(error));
}

void MainWindow::cardUpdateFinished(int, QProcess::ExitStatus)
{
    exitCardDatabaseUpdate();
}

void MainWindow::actCheckServerUpdates()
{
    auto hps = new HandlePublicServers(this);
    hps->downloadPublicServers();
    connect(hps, &HandlePublicServers::sigPublicServersDownloadedSuccessfully, [=]() { hps->deleteLater(); });
}

void MainWindow::refreshShortcuts()
{
    ShortcutsSettings &shortcuts = SettingsCache::instance().shortcuts();
    aConnect->setShortcuts(shortcuts.getShortcut("MainWindow/aConnect"));
    aDisconnect->setShortcuts(shortcuts.getShortcut("MainWindow/aDisconnect"));
    aSinglePlayer->setShortcuts(shortcuts.getShortcut("MainWindow/aSinglePlayer"));
    aWatchReplay->setShortcuts(shortcuts.getShortcut("MainWindow/aWatchReplay"));
    aDeckEditor->setShortcuts(shortcuts.getShortcut("MainWindow/aDeckEditor"));
    aFullScreen->setShortcuts(shortcuts.getShortcut("MainWindow/aFullScreen"));
    aRegister->setShortcuts(shortcuts.getShortcut("MainWindow/aRegister"));
    aSettings->setShortcuts(shortcuts.getShortcut("MainWindow/aSettings"));
    aExit->setShortcuts(shortcuts.getShortcut("MainWindow/aExit"));
    aManageSets->setShortcuts(shortcuts.getShortcut("MainWindow/aManageSets"));
    aEditTokens->setShortcuts(shortcuts.getShortcut("MainWindow/aEditTokens"));
    aOpenCustomFolder->setShortcuts(shortcuts.getShortcut("MainWindow/aOpenCustomFolder"));
    aCheckCardUpdates->setShortcuts(shortcuts.getShortcut("MainWindow/aCheckCardUpdates"));
}

void MainWindow::notifyUserAboutUpdate()
{
    QMessageBox::information(
        this, tr("Information"),
        tr("This server supports additional features that your client doesn't have.\nThis is most likely not a "
           "problem, but this message might mean there is a new version of Cockatrice available or this server is "
           "running a custom or pre-release version.\n\nTo update your client, go to Help -> Check for Updates."));
}

void MainWindow::actOpenCustomFolder()
{
    QString dir = SettingsCache::instance().getCustomPicsPath();
    QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
}

void MainWindow::actOpenCustomsetsFolder()
{
    QString dir = SettingsCache::instance().getCustomCardDatabasePath();
    QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
}

void MainWindow::actAddCustomSet()
{
    QFileDialog dialog(this, tr("Load sets/cards"), QDir::homePath());
    dialog.setNameFilters(MainWindow::fileNameFilters);
    if (!dialog.exec()) {
        return;
    }

    QString fullFilePath = dialog.selectedFiles().at(0);

    if (!QFile::exists(fullFilePath)) {
        QMessageBox::warning(this, tr("Load sets/cards"), tr("Selected file cannot be found."));
        return;
    }

    if (QFileInfo(fullFilePath).suffix() != "xml") // fileName = *.xml
    {
        QMessageBox::warning(this, tr("Load sets/cards"), tr("You can only import XML databases at this time."));
        return;
    }

    QDir dir = SettingsCache::instance().getCustomCardDatabasePath();
    int nextPrefix = getNextCustomSetPrefix(dir);

    bool res;

    QString fileName = QFileInfo(fullFilePath).fileName();
    if (fileName.compare("spoiler.xml", Qt::CaseInsensitive) == 0) {
        /*
         * If the file being added is "spoiler.xml"
         * then we'll want to overwrite the old version
         * and replace it with the new one
         */
        if (QFile::exists(dir.absolutePath() + "/spoiler.xml")) {
            QFile::remove(dir.absolutePath() + "/spoiler.xml");
        }

        res = QFile::copy(fullFilePath, dir.absolutePath() + "/spoiler.xml");
    } else {
        res = QFile::copy(fullFilePath, dir.absolutePath() + "/" + (nextPrefix > 9 ? "" : "0") +
                                            QString::number(nextPrefix) + "." + fileName);
    }

    if (res) {
        QMessageBox::information(
            this, tr("Load sets/cards"),
            tr("The new sets/cards have been added successfully.\nCockatrice will now reload the card database."));
        const auto reloadOk1 = QtConcurrent::run([] { db->loadCardDatabases(); });
    } else {
        QMessageBox::warning(this, tr("Load sets/cards"), tr("Sets/cards failed to import."));
    }
}

int MainWindow::getNextCustomSetPrefix(QDir dataDir)
{
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

void MainWindow::actManageSets()
{
    wndSets = new WndSets(this);
    wndSets->show();
}

void MainWindow::actEditTokens()
{
    DlgEditTokens dlg(this);
    dlg.exec();
    db->saveCustomTokensToFile();
}

void MainWindow::actForgotPasswordRequest()
{
    DlgForgotPasswordRequest dlg(this);
    if (dlg.exec())
        client->requestForgotPasswordToServer(dlg.getHost(), static_cast<unsigned int>(dlg.getPort()),
                                              dlg.getPlayerName());
}

void MainWindow::forgotPasswordSuccess()
{
    QMessageBox::information(
        this, tr("Reset Password"),
        tr("Your password has been reset successfully, you can now log in using the new credentials."));
    SettingsCache::instance().servers().setFPHostName("");
    SettingsCache::instance().servers().setFPPort("");
    SettingsCache::instance().servers().setFPPlayerName("");
}

void MainWindow::forgotPasswordError()
{
    QMessageBox::warning(
        this, tr("Reset Password"),
        tr("Failed to reset user account password, please contact the server operator to reset your password."));
    SettingsCache::instance().servers().setFPHostName("");
    SettingsCache::instance().servers().setFPPort("");
    SettingsCache::instance().servers().setFPPlayerName("");
}

void MainWindow::promptForgotPasswordReset()
{
    QMessageBox::information(this, tr("Reset Password"),
                             tr("Activation request received, please check your email for an activation token."));
    DlgForgotPasswordReset dlg(this);
    if (dlg.exec()) {
        client->submitForgotPasswordResetToServer(dlg.getHost(), static_cast<unsigned int>(dlg.getPort()),
                                                  dlg.getPlayerName(), dlg.getToken(), dlg.getPassword());
    }
}
