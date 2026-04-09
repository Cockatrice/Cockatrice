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

#include "../client/network/update/client/client_update_checker.h"
#include "../client/network/update/client/release_channel.h"
#include "../client/settings/cache_settings.h"
#include "../interface/widgets/dialogs/dlg_edit_tokens.h"
#include "../interface/widgets/dialogs/dlg_local_game_options.h"
#include "../interface/widgets/dialogs/dlg_manage_sets.h"
#include "../interface/widgets/dialogs/dlg_settings.h"
#include "../interface/widgets/dialogs/dlg_startup_card_check.h"
#include "../interface/widgets/dialogs/dlg_tip_of_the_day.h"
#include "../interface/widgets/dialogs/dlg_update.h"
#include "../interface/widgets/dialogs/dlg_view_log.h"
#include "../interface/widgets/tabs/tab_game.h"
#include "../interface/widgets/tabs/tab_supervisor.h"
#include "../main.h"
#include "logger.h"
#include "version_string.h"
#include "widgets/dialogs/dlg_connect.h"
#include "widgets/server/handle_public_servers.h"
#include "widgets/utility/get_text_with_max.h"

#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QCloseEvent>
#include <QDateTime>
#include <QDesktopServices>
#include <QFile>
#include <QFileDialog>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPixmapCache>
#include <QStatusBar>
#include <QSystemTrayIcon>
#include <QThread>
#include <QTimer>
#include <QWindow>
#include <QtConcurrent>
#include <QtNetwork>
#include <libcockatrice/card/database/card_database.h>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/network/client/local/local_client.h>
#include <libcockatrice/network/client/remote/remote_client.h>
#include <libcockatrice/network/server/local/local_server.h>
#include <libcockatrice/network/server/local/local_server_interface.h>
#include <libcockatrice/protocol/pb/game_replay.pb.h>
#include <libcockatrice/protocol/pb/room_commands.pb.h>

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
inline Q_LOGGING_CATEGORY(MainWindowLog, "main_window");

/**
 * Replaces the tab-specific menus that are shown in the menuBar.
 *
 * @param newMenuList The tab-specific menus to show in the menuBar
 */
void MainWindow::updateTabMenu(const QList<QMenu *> &newMenuList)
{
    for (auto &tabMenu : tabMenus)
        menuBar()->removeAction(tabMenu->menuAction());
    tabMenus = newMenuList;
    for (auto &tabMenu : tabMenus)
        menuBar()->insertMenu(helpMenu->menuAction(), tabMenu);
}

void MainWindow::statusChanged(ClientStatus _status)
{
    connectionController->refreshWindowTitle();
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

// Actions

void MainWindow::actConnect()
{
    connectionController->connectToServer();
}

void MainWindow::actDisconnect()
{
    connectionController->disconnectFromServer();
}

void MainWindow::actSinglePlayer()
{
    DlgLocalGameOptions dlg(this);
    if (dlg.exec() != QDialog::Accepted) {
        return;
    }

    startLocalGame(dlg.getOptions());
}

void MainWindow::startLocalGame(const LocalGameOptions &options)
{
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

    for (int i = 0; i < options.numberPlayers - 1; ++i) {
        LocalServerInterface *slaveLsi = localServer->newConnection();
        LocalClient *slaveClient =
            new LocalClient(slaveLsi, tr("Player %1").arg(i + 2), SettingsCache::instance().getClientID(), this);
        localClients.append(slaveClient);
    }
    tabSupervisor->startLocal(localClients);

    Command_CreateGame createCommand;
    createCommand.set_max_players(static_cast<google::protobuf::uint32>(options.numberPlayers));
    createCommand.set_starting_life_total(options.startingLifeTotal);
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
    if (replay->ParseFromArray(buf.data(), buf.size())) {
        tabSupervisor->openReplay(replay);
    } else {
        qCWarning(MainWindowLog) << "failed to parse replay!";
    }
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

void MainWindow::actOpenSettingsFolder()
{
    QString dir = SettingsCache::instance().getSettingsPath();
    QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
}

void MainWindow::retranslateUi()
{
    connectionController->refreshWindowTitle();

    aConnect->setText(tr("&Connect..."));
    aDisconnect->setText(tr("&Disconnect"));
    aSinglePlayer->setText(tr("Start &local game..."));
    aWatchReplay->setText(tr("&Watch replay..."));
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
    aReloadCardDatabase->setText(tr("Reload card database"));

    tabsMenu->setTitle(tr("Tabs"));

    helpMenu->setTitle(tr("&Help"));
    aAbout->setText(tr("&About Cockatrice"));
    aTips->setText(tr("&Tip of the Day"));
    aUpdate->setText(tr("Check for Client Updates"));
    aCheckCardUpdates->setText(tr("Check for Card Updates..."));
    aCheckCardUpdatesBackground->setText(tr("Check for Card Updates (Automatic)"));
    aStatusBar->setText(tr("Show Status Bar"));
    aViewLog->setText(tr("View &Debug Log"));
    aOpenSettingsFolder->setText(tr("Open Settings Folder"));

    aShow->setText(tr("Show/Hide"));

    tabSupervisor->retranslateUi();
}

void MainWindow::createActions()
{
    aConnect = new QAction(this);
    connect(aConnect, &QAction::triggered, this, &MainWindow::actConnect);
    aDisconnect = new QAction(this);
    aDisconnect->setEnabled(false);
    connect(aDisconnect, &QAction::triggered, this, &MainWindow::actDisconnect);
    aSinglePlayer = new QAction(this);
    connect(aSinglePlayer, &QAction::triggered, this, &MainWindow::actSinglePlayer);
    aWatchReplay = new QAction(this);
    connect(aWatchReplay, &QAction::triggered, this, &MainWindow::actWatchReplay);
    aFullScreen = new QAction(this);
    aFullScreen->setCheckable(true);
    connect(aFullScreen, &QAction::toggled, this, &MainWindow::actFullScreen);
    aRegister = new QAction(this);
    connect(aRegister, &QAction::triggered, connectionController, &ConnectionController::registerToServer);
    aForgotPassword = new QAction(this);
    connect(aForgotPassword, &QAction::triggered, connectionController, &ConnectionController::forgotPasswordRequest);
    aSettings = new QAction(this);
    connect(aSettings, &QAction::triggered, this, &MainWindow::actSettings);
    aExit = new QAction(this);
    connect(aExit, &QAction::triggered, this, &MainWindow::actExit);

    aManageSets = new QAction(QString(), this);
    connect(aManageSets, &QAction::triggered, this, &MainWindow::actManageSets);
    aEditTokens = new QAction(QString(), this);
    connect(aEditTokens, &QAction::triggered, this, &MainWindow::actEditTokens);
    aOpenCustomFolder = new QAction(QString(), this);
    connect(aOpenCustomFolder, &QAction::triggered, this, &MainWindow::actOpenCustomFolder);
    aOpenCustomsetsFolder = new QAction(QString(), this);
    connect(aOpenCustomsetsFolder, &QAction::triggered, this, &MainWindow::actOpenCustomsetsFolder);
    aAddCustomSet = new QAction(QString(), this);
    connect(aAddCustomSet, &QAction::triggered, this, &MainWindow::actAddCustomSet);
    aReloadCardDatabase = new QAction(QString(), this);
    connect(aReloadCardDatabase, &QAction::triggered, this, &MainWindow::actReloadCardDatabase);

    aAbout = new QAction(this);
    connect(aAbout, &QAction::triggered, this, &MainWindow::actAbout);
    aTips = new QAction(this);
    connect(aTips, &QAction::triggered, this, &MainWindow::actTips);
    aUpdate = new QAction(this);
    connect(aUpdate, &QAction::triggered, this, &MainWindow::actUpdate);
    aCheckCardUpdates = new QAction(this);
    connect(aCheckCardUpdates, &QAction::triggered, this, &MainWindow::actCheckCardUpdates);
    aCheckCardUpdatesBackground = new QAction(this);
    connect(aCheckCardUpdatesBackground, &QAction::triggered, this, &MainWindow::actCheckCardUpdatesBackground);
    aStatusBar = new QAction(this);
    aStatusBar->setCheckable(true);
    aStatusBar->setChecked(SettingsCache::instance().getShowStatusBar());
    connect(aStatusBar, &QAction::triggered, &SettingsCache::instance(), &SettingsCache::setShowStatusBar);
    aViewLog = new QAction(this);
    connect(aViewLog, &QAction::triggered, this, &MainWindow::actViewLog);
    aOpenSettingsFolder = new QAction(this);
    connect(aOpenSettingsFolder, &QAction::triggered, this, &MainWindow::actOpenSettingsFolder);

    aShow = new QAction(this);
    connect(aShow, &QAction::triggered, this, &MainWindow::actShow);

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
    dbMenu->addSeparator();
    dbMenu->addAction(aReloadCardDatabase);

    tabsMenu = menuBar()->addMenu(QString());

    helpMenu = menuBar()->addMenu(QString());
    helpMenu->addAction(aAbout);
    helpMenu->addAction(aTips);
    helpMenu->addSeparator();
    helpMenu->addAction(aUpdate);
    helpMenu->addAction(aCheckCardUpdates);
    helpMenu->addAction(aCheckCardUpdatesBackground);
    helpMenu->addSeparator();
    helpMenu->addAction(aStatusBar);
    helpMenu->addAction(aViewLog);
    helpMenu->addAction(aOpenSettingsFolder);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), localServer(nullptr), bHasActivated(false), askedForDbUpdater(false),
      cardUpdateProcess(nullptr), logviewDialog(nullptr)
{
    connect(&SettingsCache::instance(), &SettingsCache::pixmapCacheSizeChanged, this,
            &MainWindow::pixmapCacheSizeChanged);
    pixmapCacheSizeChanged(SettingsCache::instance().getPixmapCacheSize());

    connectionController = new ConnectionController(this, this);

    createActions();
    createMenus();

    connect(connectionController, &ConnectionController::windowTitleChanged, this, &MainWindow::setWindowTitle);
    connect(connectionController, &ConnectionController::statusChanged, this, &MainWindow::statusChanged);

    tabSupervisor = new TabSupervisor(connectionController->client(), tabsMenu, this);
    connect(tabSupervisor, &TabSupervisor::setMenu, this, &MainWindow::updateTabMenu);
    connect(tabSupervisor, &TabSupervisor::localGameEnded, this, &MainWindow::localGameEnded);
    connect(tabSupervisor, &TabSupervisor::showWindowIfHidden, this, &MainWindow::showWindowIfHidden);
    connect(connectionController, &ConnectionController::tabSupervisorStartRequested, tabSupervisor,
            &TabSupervisor::start);
    connect(connectionController, &ConnectionController::tabSupervisorStopRequested, tabSupervisor,
            &TabSupervisor::stop);
    tabSupervisor->initStartupTabs();

    setCentralWidget(tabSupervisor);

    retranslateUi();

    if (!restoreGeometry(SettingsCache::instance().layouts().getMainWindowGeometry())) {
        setWindowState(Qt::WindowMaximized);
    }
    aFullScreen->setChecked(static_cast<bool>(windowState() & Qt::WindowFullScreen));

    if (QSystemTrayIcon::isSystemTrayAvailable()) {
        createTrayIcon();
    }

    // status bar
    connect(&SettingsCache::instance(), &SettingsCache::showStatusBarChanged, this,
            [this](bool show) { statusBar()->setVisible(show); });
    statusBar()->setVisible(SettingsCache::instance().getShowStatusBar());

    connect(&SettingsCache::instance().shortcuts(), &ShortcutsSettings::shortCutChanged, this,
            &MainWindow::refreshShortcuts);
    refreshShortcuts();

    connect(CardDatabaseManager::getInstance(), &CardDatabase::cardDatabaseLoadingFailed, this,
            &MainWindow::cardDatabaseLoadingFailed);
    connect(CardDatabaseManager::getInstance(), &CardDatabase::cardDatabaseNewSetsFound, this,
            &MainWindow::cardDatabaseNewSetsFound);
    connect(CardDatabaseManager::getInstance(), &CardDatabase::cardDatabaseAllNewSetsEnabled, this,
            &MainWindow::cardDatabaseAllNewSetsEnabled);

    tip = new DlgTipOfTheDay();

    // run startup check async
    QTimer::singleShot(0, this, &MainWindow::startupConfigCheck);
}

void MainWindow::startupConfigCheck()
{
    if (SettingsCache::instance().debug().getLocalGameOnStartup()) {
        LocalGameOptions options;
        options.numberPlayers = SettingsCache::instance().debug().getLocalGamePlayerCount();
        startLocalGame(options);
    }

    if (SettingsCache::instance().getCheckUpdatesOnStartup()) {
        actCheckClientUpdates();
    }

    if (SettingsCache::instance().getClientVersion() == CLIENT_INFO_NOT_SET) {
        // no config found, 99% new clean install
        qCInfo(WindowMainStartupVersionLog)
            << "Startup: old client version empty, assuming first start after clean install";
        alertForcedOracleRun(VERSION_STRING, false);
        SettingsCache::instance().downloads().resetToDefaultURLs(); // populate the download urls
        SettingsCache::instance().setClientVersion(VERSION_STRING);
    } else if (SettingsCache::instance().getClientVersion() != VERSION_STRING) {
        // config found, from another (presumably older) version
        qCInfo(WindowMainStartupVersionLog)
            << "Startup: old client version" << SettingsCache::instance().getClientVersion()
            << "differs, assuming first start after update";
        if (SettingsCache::instance().getNotifyAboutNewVersion()) {
            alertForcedOracleRun(VERSION_STRING, true);
        } else {
            const auto reloadOk0 = QtConcurrent::run([] { CardDatabaseManager::getInstance()->loadCardDatabases(); });
        }

        qCInfo(WindowMainStartupShortcutsLog) << "Migrating shortcuts after update detected.";
        SettingsCache::instance().shortcuts().migrateShortcuts();

        if (SettingsCache::instance().getCheckUpdatesOnStartup()) {
            if (QString(VERSION_STRING).contains("custom", Qt::CaseInsensitive)) {
                qCInfo(WindowMainStartupShortcutsLog) << "Update has changed to custom version, disabling auto update";
                SettingsCache::instance().setCheckUpdatesOnStartup(Qt::Unchecked);
            } else {
                int channel = 0;
                if (QString(VERSION_STRING).contains("beta", Qt::CaseInsensitive)) {
                    channel = 1;
                }
                if (SettingsCache::instance().getUpdateReleaseChannelIndex() != channel) {
                    qCInfo(WindowMainStartupShortcutsLog) << "Update has changed beta state, updating release channel.";
                    SettingsCache::instance().setUpdateReleaseChannelIndex(channel);
                }
            }
        }

        SettingsCache::instance().setClientVersion(VERSION_STRING);
    } else {
        // previous config from this version found
        qCInfo(WindowMainStartupVersionLog) << "Startup: found config with current version";

        if (SettingsCache::instance().getCardUpdateCheckRequired()) {
            if (SettingsCache::instance().getStartupCardUpdateCheckPromptForUpdate()) {
                auto startupCardCheckDialog = new DlgStartupCardCheck(this);

                if (startupCardCheckDialog->exec() == QDialog::Accepted) {
                    switch (startupCardCheckDialog->group->checkedId()) {
                        case 0: // foreground
                            actCheckCardUpdates();
                            break;
                        case 1: // background
                            actCheckCardUpdatesBackground();
                            break;
                        case 2: // background + always
                            SettingsCache::instance().setStartupCardUpdateCheckPromptForUpdate(false);
                            SettingsCache::instance().setStartupCardUpdateCheckAlwaysUpdate(true);
                            actCheckCardUpdatesBackground();
                            break;
                        case 3: // don't prompt again + don't run
                            SettingsCache::instance().setStartupCardUpdateCheckPromptForUpdate(false);
                            SettingsCache::instance().setStartupCardUpdateCheckAlwaysUpdate(false);
                            break;
                        default:
                            break;
                    }
                }
            } else if (SettingsCache::instance().getStartupCardUpdateCheckAlwaysUpdate()) {
                actCheckCardUpdatesBackground();
            }
        }

        const auto reloadOk1 = QtConcurrent::run([] { CardDatabaseManager::getInstance()->loadCardDatabases(); });

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

    if (cardUpdateProcess) {
        cardUpdateProcess->disconnect(this);
        cardUpdateProcess->terminate();
        cardUpdateProcess->waitForFinished(1000);
        cardUpdateProcess = nullptr;
    }
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

void MainWindow::closeEvent(QCloseEvent *event)
{
    // workaround Qt bug where closeEvent gets called twice
    static bool bClosingDown = false;
    if (bClosingDown)
        return;
    bClosingDown = true;

    if (!tabSupervisor->close()) {
        event->ignore();
        bClosingDown = false;
        return;
    }
    tip->close();

    event->accept();
    SettingsCache::instance().layouts().setMainWindowGeometry(saveGeometry());
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
                qCInfo(WindowMainStartupAutoconnectLog) << "Command line connect to " << connectTo;
                connectionController->connectToServerDirect(connectTo.host(), connectTo.port(), connectTo.userName(),
                                                            connectTo.password());
            } else if (SettingsCache::instance().servers().getAutoConnect() &&
                       !SettingsCache::instance().debug().getLocalGameOnStartup()) {
                qCInfo(WindowMainStartupAutoconnectLog) << "Attempting auto-connect...";
                DlgConnect dlg(this);
                connectionController->connectToServerDirect(dlg.getHost(), static_cast<unsigned int>(dlg.getPort()),
                                                            dlg.getPlayerName(), dlg.getPassword());
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
    QMessageBox msgBox(this);
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
        CardDatabaseManager::getInstance()->enableAllUnknownSets();
        const auto reloadOk1 = QtConcurrent::run([] { CardDatabaseManager::getInstance()->loadCardDatabases(); });
    } else if (msgBox.clickedButton() == noButton) {
        CardDatabaseManager::getInstance()->markAllSetsAsKnown();
    } else if (msgBox.clickedButton() == settingsButton) {
        CardDatabaseManager::getInstance()->markAllSetsAsKnown();
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
    createCardUpdateProcess();
}

void MainWindow::actCheckCardUpdatesBackground()
{
    createCardUpdateProcess(true);
}

void MainWindow::createCardUpdateProcess(bool background)
{
    if (cardUpdateProcess) {
        QMessageBox::information(this, tr("Information"), tr("A card database update is already running."));
        return;
    }

    cardUpdateProcess = new QProcess(this);

    connect(cardUpdateProcess, &QProcess::errorOccurred, this, &MainWindow::cardUpdateError);

    connect(cardUpdateProcess, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
            &MainWindow::cardUpdateFinished);

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

    if (!background) {
        cardUpdateProcess->start(updaterCmd, QStringList());
    } else {
        cardUpdateProcess->start(updaterCmd, QStringList("-b"));
        statusBar()->showMessage(tr("Card database update running."));
    }
}

void MainWindow::exitCardDatabaseUpdate()
{
    cardUpdateProcess->deleteLater();
    cardUpdateProcess = nullptr;
    statusBar()->clearMessage();

    const auto reloadOk1 = QtConcurrent::run([] { CardDatabaseManager::getInstance()->loadCardDatabases(); });
}

void MainWindow::cardUpdateError(QProcess::ProcessError err)
{
    QString error;
    switch (err) {
        case QProcess::FailedToStart:
            error = tr("Failed to start. The file might be missing, or permissions might be incorrect.");
            break;
        case QProcess::Crashed:
            error = tr("The process crashed some time after starting successfully.");
            error += "\n\nError output:\n" + cardUpdateProcess->readAllStandardError();
            break;
        case QProcess::Timedout:
            error = tr("Timed out. The process took too long to respond. The last waitFor...() function timed out.");
            break;
        case QProcess::WriteError:
            error = tr("An error occurred when attempting to write to the process. For example, the process may "
                       "not be running, or it may have closed its input channel.");
            break;
        case QProcess::ReadError:
            error = tr("An error occurred when attempting to read from the process. For example, the process may "
                       "not be running.");
            break;
        case QProcess::UnknownError:
        default:
            error = tr("Unknown error occurred.");
            break;
    }

    exitCardDatabaseUpdate();
    QMessageBox::warning(this, tr("Error"), tr("The card database updater exited with an error:\n%1").arg(error));
}

void MainWindow::cardUpdateFinished(int, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::NormalExit) {
        SettingsCache::instance().setLastCardUpdateCheck(QDateTime::currentDateTime().date());
    }
    exitCardDatabaseUpdate();
}

void MainWindow::actCheckServerUpdates()
{
    auto hps = new HandlePublicServers(this);
    hps->downloadPublicServers();
    connect(hps, &HandlePublicServers::sigPublicServersDownloadedSuccessfully, [=]() { hps->deleteLater(); });
}

void MainWindow::actCheckClientUpdates()
{
    auto checker = new ClientUpdateChecker(this);
    connect(checker, &ClientUpdateChecker::finishedCheck, this, &MainWindow::checkClientUpdatesFinished);
    checker->check();
}

void MainWindow::checkClientUpdatesFinished(bool needToUpdate, bool /* isCompatible */, Release * /* release */)
{
    if (needToUpdate) {
        DlgUpdate dlg(this);
        dlg.exec();
    }
}

void MainWindow::refreshShortcuts()
{
    ShortcutsSettings &shortcuts = SettingsCache::instance().shortcuts();
    aConnect->setShortcuts(shortcuts.getShortcut("MainWindow/aConnect"));
    aDisconnect->setShortcuts(shortcuts.getShortcut("MainWindow/aDisconnect"));
    aSinglePlayer->setShortcuts(shortcuts.getShortcut("MainWindow/aSinglePlayer"));
    aWatchReplay->setShortcuts(shortcuts.getShortcut("MainWindow/aWatchReplay"));
    aFullScreen->setShortcuts(shortcuts.getShortcut("MainWindow/aFullScreen"));
    aRegister->setShortcuts(shortcuts.getShortcut("MainWindow/aRegister"));
    aSettings->setShortcuts(shortcuts.getShortcut("MainWindow/aSettings"));
    aExit->setShortcuts(shortcuts.getShortcut("MainWindow/aExit"));
    aManageSets->setShortcuts(shortcuts.getShortcut("MainWindow/aManageSets"));
    aEditTokens->setShortcuts(shortcuts.getShortcut("MainWindow/aEditTokens"));
    aOpenCustomFolder->setShortcuts(shortcuts.getShortcut("MainWindow/aOpenCustomFolder"));
    aCheckCardUpdates->setShortcuts(shortcuts.getShortcut("MainWindow/aCheckCardUpdates"));
    aStatusBar->setShortcuts(shortcuts.getShortcut("MainWindow/aStatusBar"));
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
        const auto reloadOk1 = QtConcurrent::run([] { CardDatabaseManager::getInstance()->loadCardDatabases(); });
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

void MainWindow::actReloadCardDatabase()
{
    const auto reloadOk1 = QtConcurrent::run([] {
        CardDatabaseManager::getInstance()->loadCardDatabases();
        SettingsCache::instance().downloads().sync();
    });
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
    CardDatabaseManager::getInstance()->saveCustomTokensToFile();
}
