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

#include "../client/latency_status_widget.h"
#include "../client/network/update/client/client_update_checker.h"
#include "../client/settings/cache_settings.h"
#include "../client/settings/shortcuts_settings.h"
#include "../interface/widgets/dialogs/dlg_edit_tokens.h"
#include "../interface/widgets/dialogs/dlg_local_game_options.h"
#include "../interface/widgets/dialogs/dlg_manage_sets.h"
#include "../interface/widgets/dialogs/dlg_settings.h"
#include "../interface/widgets/dialogs/dlg_startup_card_check.h"
#include "../interface/widgets/dialogs/dlg_tip_of_the_day.h"
#include "../interface/widgets/dialogs/dlg_update.h"
#include "../interface/widgets/dialogs/dlg_view_log.h"
#include "../interface/widgets/onboarding/first_run_wizard.h"
#include "../interface/widgets/settings_page/general_settings_page.h"
#include "../interface/widgets/tabs/tab_server.h"
#include "../interface/widgets/tabs/tab_supervisor.h"
#include "../main.h"
#include "abstract_client.h"
#include "card_database_update/card_database_update_status_bar.h"
#include "card_database_update/card_update_progress.h"
#include "intents/contexts/context_connect_to_server.h"
#include "intents/contexts/context_join_room.h"
#include "intents/intent.h"
#include "intents/intent_connect_to_server.h"
#include "intents/intent_login.h"
#include "intents/intent_open_server_room_by_name.h"
#include "intents/url_parser.h"
#include "latency_tracker.h"
#include "logger.h"
#include "pixel_map_generator.h"
#include "version_string.h"
#include "widgets/dialogs/dlg_connect.h"
#include "widgets/server/handle_public_servers.h"
#include "widgets/settings_page/abstract_settings_page.h"
#include "widgets/tabs/api/commander_spellbook/handle_commander_brackets.h"

#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QDesktopServices>
#include <QDialog>
#include <QDir>
#include <QEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFlags>
#include <QFutureWatcher>
#include <QIODevice>
#include <QLocalSocket>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMessageLogger>
#include <QObject>
#include <QPixmap>
#include <QPixmapCache>
#include <QPushButton>
#include <QSharedPointer>
#include <QStatusBar>
#include <QStringLiteral>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QWidget>
#include <QtConcurrentRun>
#include <QtGlobal>
#include <functional>
#include <google/protobuf/stubs/port.h>
#include <libcockatrice/card/database/card_database.h>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/network/client/local/local_client.h>
#include <libcockatrice/network/client/remote/remote_client.h>
#include <libcockatrice/network/server/local/local_server.h>
#include <libcockatrice/protocol/pb/game_replay.pb.h>
#include <libcockatrice/protocol/pb/room_commands.pb.h>
#include <libcockatrice/settings/cache_storage_settings.h>
#include <libcockatrice/settings/debug_settings.h>
#include <libcockatrice/settings/download_settings.h>
#include <libcockatrice/settings/interface_settings.h>
#include <libcockatrice/settings/layouts_settings.h>
#include <libcockatrice/settings/network_settings.h>
#include <libcockatrice/settings/paths_settings.h>
#include <libcockatrice/settings/personal_settings.h>
#include <libcockatrice/settings/servers_settings.h>
#include <libcockatrice/settings/tabs_settings.h>
#include <libcockatrice/settings/updates_settings.h>
#include <libcockatrice/utility/local_server_name.h>
#include <libcockatrice/utility/user_data_backup.h>
#include <memory>
#include <qlogging.h>
#include <qnamespace.h>
#include <tuple>
#include <utility>

class LocalServerInterface;

#define GITHUB_PAGES_URL "https://cockatrice.github.io"
#define GITHUB_CONTRIBUTORS_URL "https://github.com/Cockatrice/Cockatrice/graphs/contributors"
#define GITHUB_CONTRIBUTE_URL "https://github.com/Cockatrice/Cockatrice#"
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
    for (auto &tabMenu : tabMenus) {
        menuBar()->removeAction(tabMenu->menuAction());
    }
    tabMenus = newMenuList;
    for (auto &tabMenu : tabMenus) {
        menuBar()->insertMenu(helpMenu->menuAction(), tabMenu);
    }
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
    dlg.setDirectory(SettingsCache::instance().paths().getReplaysPath());
    dlg.setNameFilters(QStringList() << QObject::tr("Cockatrice replays (*.cor)"));
    if (!dlg.exec()) {
        return;
    }

    QString fileName = dlg.selectedFiles().at(0);
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }
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
    if (checked) {
        setWindowState(windowState() | Qt::WindowFullScreen);
    } else {
        setWindowState(windowState() & ~Qt::WindowFullScreen);
    }
}

void MainWindow::actSettings()
{
    DlgSettings dlg(this);
    auto *generalPage = qobject_cast<GeneralSettingsPage *>(dlg.page(DlgSettings::GeneralPage));
    connect(generalPage, &GeneralSettingsPage::cardDatabaseUpdateRequested, this, &MainWindow::actCheckCardUpdates);
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
    mb.setIconPixmap(
        themePixmap(QStringLiteral("cockatrice")).scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
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
    aSettings->setIcon(themePixmap(QStringLiteral("icons/settings")));
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
    aFirstRunWizard->setText(tr("Re-run Onboarding Wizard..."));

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
    aStatusBar->setChecked(SettingsCache::instance().userInterface().getShowStatusBar());
    connect(aStatusBar, &QAction::triggered, &SettingsCache::instance().userInterface(),
            &InterfaceSettings::setShowStatusBar);
    aViewLog = new QAction(this);
    connect(aViewLog, &QAction::triggered, this, &MainWindow::actViewLog);
    aOpenSettingsFolder = new QAction(this);
    connect(aOpenSettingsFolder, &QAction::triggered, this, &MainWindow::actOpenSettingsFolder);
    aFirstRunWizard = new QAction(this);
    connect(aFirstRunWizard, &QAction::triggered, this, [this] { runFirstRunWizard(); });

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
    helpMenu->addSeparator();
    helpMenu->addAction(aFirstRunWizard);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), localServer(nullptr), bHasActivated(false), askedForDbUpdater(false),
      cardUpdateProcess(nullptr), logviewDialog(nullptr)
{
    connect(&SettingsCache::instance().cacheStorage(), &CacheStorageSettings::pixmapCacheSizeChanged, this,
            &MainWindow::pixmapCacheSizeChanged);
    pixmapCacheSizeChanged(SettingsCache::instance().cacheStorage().getPixmapCacheSize());

    connectionController = new ConnectionController(this, this);
    urlParser = new IntentUrlParser(this, this);
    connect(urlParser, &IntentUrlParser::urlChainFinished, this, &MainWindow::onUrlChainFinished);

    createActions();
    createMenus();

    connect(connectionController, &ConnectionController::windowTitleChanged, this, &MainWindow::setWindowTitle);
    connect(connectionController, &ConnectionController::statusChanged, this, &MainWindow::statusChanged);

    tabSupervisor = new TabSupervisor(connectionController->client(), tabsMenu, this);
    connect(tabSupervisor, &TabSupervisor::setMenu, this, &MainWindow::updateTabMenu);
    connect(tabSupervisor, &TabSupervisor::localGameEnded, this, &MainWindow::localGameEnded);
    connect(tabSupervisor, &TabSupervisor::showWindowIfHidden, this, &MainWindow::showWindowIfHidden);
    connect(tabSupervisor, &TabSupervisor::cockatriceLinkActivated, this, &MainWindow::handleCockatriceLink);
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
    connect(&SettingsCache::instance().userInterface(), &InterfaceSettings::showStatusBarChanged, this,
            [this](bool show) { statusBar()->setVisible(show); });
    statusBar()->setVisible(SettingsCache::instance().userInterface().getShowStatusBar());

    latencyStatus = new LatencyStatusWidget(this);
    statusBar()->addPermanentWidget(latencyStatus);

    connect(connectionController, &ConnectionController::pingStatsUpdated, latencyStatus,
            &LatencyStatusWidget::updateData);

    cardUpdateStatusBar = new CardDatabaseUpdateStatusBar(this);
    statusBar()->addPermanentWidget(cardUpdateStatusBar);
    connect(this, &MainWindow::cardDatabaseUpdateProgress, cardUpdateStatusBar,
            &CardDatabaseUpdateStatusBar::updateProgress);
    connect(this, &MainWindow::cardDatabaseUpdateFinished, cardUpdateStatusBar,
            &CardDatabaseUpdateStatusBar::updateFinished);

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
    QTimer::singleShot(0, this, &MainWindow::applyStartupDestination);
}

static bool isDeckFileSuffix(const QString &suffix)
{
    return suffix == QStringLiteral("cod") || suffix == QStringLiteral("dec") || suffix == QStringLiteral("dek") ||
           suffix == QStringLiteral("mwdeck");
}

static bool isInsideAnyDir(const QString &path, const QStringList &bases)
{
    for (const QString &base : bases) {
        if (PathsSettings::isInsideDir(path, base)) {
            return true;
        }
    }
    return false;
}

// Merges an at-risk offender into the backup source list: offenders already covered by a
// collected source are skipped, while an offender that is a parent of collected sources
// supersedes them.
static void mergeCoveringSource(QStringList &sources, const QString &offender)
{
    if (isInsideAnyDir(offender, sources)) {
        return;
    }
    for (int i = sources.size() - 1; i >= 0; --i) {
        if (PathsSettings::isInsideDir(sources.at(i), offender)) {
            sources.removeAt(i);
        }
    }
    sources.append(offender);
}

static bool hasLooseDeckFile(const QStringList &offenders)
{
    for (const QString &offender : offenders) {
        if (isDeckFileSuffix(QFileInfo(offender).suffix().toLower())) {
            return true;
        }
    }
    return false;
}

static QString resolveBackupRoot(const QString &appDir)
{
    QString backupRoot = UserDataBackup::defaultBackupRoot();
    if (PathsSettings::isInsideDir(backupRoot, appDir)) {
        backupRoot = QDir::temp().absoluteFilePath(QStringLiteral("CockatriceBackups"));
    }
    return backupRoot;
}

// True when a portable offender must stay out of the safety backup: replays (size),
// downloaded pictures (large and re-downloadable - everything under pics/ except the
// seeded CUSTOM dir) and the regenerable redirect/cache dir.
static bool isPortableBackupExcluded(const QString &offender)
{
    auto &paths = SettingsCache::instance().paths();
    const QString replaysPath = QDir::cleanPath(paths.getReplaysPath());
    const QString picsPath = QDir::cleanPath(paths.getPicsPath());
    const QString customPicsPath = QDir::cleanPath(paths.getCustomPicsPath());
    const QString redirectsPath = QDir::cleanPath(paths.getRedirectCachePath());
    const QString cacheDir = QDir::cleanPath(SettingsCache::instance().getCachePath());

    if (!replaysPath.isEmpty() && PathsSettings::isInsideDir(offender, replaysPath)) {
        return true;
    }
    if (!picsPath.isEmpty() && PathsSettings::isInsideDir(offender, picsPath) &&
        (customPicsPath.isEmpty() || !PathsSettings::isInsideDir(offender, customPicsPath))) {
        return true;
    }
    if (!redirectsPath.isEmpty() && PathsSettings::isInsideDir(offender, redirectsPath)) {
        return true;
    }
    if (!cacheDir.isEmpty() && PathsSettings::isInsideDir(offender, cacheDir)) {
        return true;
    }
    return false;
}

MainWindow::AtRiskPaths MainWindow::collectPortableAtRiskPaths(const QString &appDir)
{
    auto &paths = SettingsCache::instance().paths();
    AtRiskPaths atRisk;

    // Portable data lives inside the install directory by design, so there is
    // nothing to reconfigure. Back up the small irreplaceable data instead.
    // Downloaded pictures are deliberately excluded: they are large and can
    // always be re-downloaded (the installer's pre-uninstall backup drops
    // them for the same reason). Replays are excluded here for size; the
    // installer backup covers them during installer-driven upgrades, but a
    // manual run of the old uninstall.exe is not covered - hence the note
    // in the warning text below.
    const QString dataDir = SettingsCache::instance().getDataPath();
    for (const QString &sub :
         QStringList{QStringLiteral("decks"), QStringLiteral("customsets"), QStringLiteral("settings"),
                     QStringLiteral("filters"), QStringLiteral("themes"), QStringLiteral("sounds"),
                     QStringLiteral("pics/CUSTOM")}) {
        const QString candidate = QDir::cleanPath(QDir(dataDir).absoluteFilePath(sub));
        if (QFileInfo::exists(candidate)) {
            atRisk.sourcesToBackUp.append(candidate);
        }
    }

    // Card database files live in dataDir by default but are not in the list above. Seed them
    // explicitly so they do not depend on the offender merge below to be backed up.
    for (const QString &dbFile :
         QStringList{paths.getCardDatabasePath(), paths.getTokenDatabasePath(), paths.getSpoilerCardDatabasePath()}) {
        if (dbFile.isEmpty()) {
            continue;
        }
        const QString cleanedDbFile = QDir::cleanPath(dbFile);
        if (!QFileInfo::exists(cleanedDbFile) || !PathsSettings::isInsideDir(cleanedDbFile, appDir)) {
            continue;
        }
        // isInsideDir is equality-aware and case-insensitive on Windows/macOS,
        // so it also covers exact duplicates with different casing or separators.
        if (!isInsideAnyDir(cleanedDbFile, atRisk.sourcesToBackUp)) {
            atRisk.sourcesToBackUp.append(cleanedDbFile);
        }
    }

    // A portable user may also have pointed a custom path inside the install
    // dir. Offenders outside data/ (e.g. appDir/mydecks) would be wiped by the
    // old uninstaller but are not in the hardcoded list above - and neither
    // is a custom folder inside data/ itself (e.g. dataDir/mydecks), which the
    // old uninstaller deletes all the same. Cover any offender that is not
    // already inside a backed-up source; when an offender is a parent of
    // already-collected sources it supersedes them.
    for (const QString &offender : paths.pathsInsideDir(appDir)) {
        if (!QFileInfo::exists(offender)) {
            continue;
        }
        const QString cleanedOffender = QDir::cleanPath(offender);
        if (isPortableBackupExcluded(cleanedOffender)) {
            continue;
        }
        mergeCoveringSource(atRisk.sourcesToBackUp, cleanedOffender);
    }

    // A fresh portable install has no data on disk yet, but the user still
    // needs to know their data will live where the 3.1.0/3.1.1 uninstaller
    // deletes it - so fall through to the warning with an empty backup
    // instead of returning silently.
    if (atRisk.sourcesToBackUp.isEmpty()) {
        qCInfo(MainWindowLog) << "Portable install with no existing data files, warning without backup";
    }
    return atRisk;
}

QStringList MainWindow::collectInstalledAtRiskPaths(const QString &appDir)
{
    QStringList offenders = SettingsCache::instance().paths().pathsInsideDir(appDir);

    // Loose decks sitting directly in the program folder are wiped by the old uninstaller
    // just like configured paths. Mirror the installer backup set (DeckLoader extensions
    // minus *.txt) so a manual run of the old uninstall.exe is covered too.
    const QDir appDirHandle(appDir);
    const QStringList topLevelFiles = appDirHandle.entryList(QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot);
    for (const QString &fileName : topLevelFiles) {
        if (!isDeckFileSuffix(QFileInfo(fileName).suffix().toLower())) {
            continue;
        }
        const QString looseDeck = QDir::cleanPath(appDirHandle.absoluteFilePath(fileName));
        if (!isInsideAnyDir(looseDeck, offenders)) {
            offenders.append(looseDeck);
        }
    }
    return offenders;
}

MainWindow::AtRiskBackupOutcome MainWindow::backUpAtRiskPaths(const QStringList &sources, const QString &appDir)
{
    AtRiskBackupOutcome outcome;
    const QString backupRoot = resolveBackupRoot(appDir);

    // Throttle: dismissing the dialog without "Don't show again" must not create
    // a full duplicate on every launch. A recent session is reused only when it is
    // complete and its source manifest covers the current source set; otherwise a
    // fresh copy is taken so the dialog never claims an uncovered path is backed up.
    // The size estimate runs only when a fresh copy is actually going to happen,
    // keeping a synchronous tree walk off the reuse fast path at startup.
    // Search all recent sessions for a complete, covering one: a partial failure must not
    // pin the newest slot and force a full re-copy on every later launch.
    const QString reusableSession = UserDataBackup::findRecentUsableBackupSession(backupRoot, sources, 24);
    if (!reusableSession.isEmpty()) {
        outcome.sessionDir = reusableSession;
        outcome.locations = UserDataBackup::listSessionBackupLocations(reusableSession);
        outcome.reused = true;
        qCInfo(MainWindowLog) << "Reusing recent at-risk user data backup at" << outcome.sessionDir;
    } else if (!sources.isEmpty()) {
        // The copy runs synchronously on the UI thread at startup, so cap it: a
        // pictures folder pointed into the program dir can be gigabytes, and freezing
        // the client to duplicate it is worse than asking for a manual copy.
        // Portable sources are small by construction (downloaded pictures and replays
        // are excluded above); this guard bites for non-portable offenders.
        outcome.bytes = UserDataBackup::estimateBackupSize(sources);
        if (outcome.bytes >= UserDataBackup::MAX_AUTO_BACKUP_BYTES) {
            outcome.skippedDueToSize = true;
            qCWarning(MainWindowLog) << "At-risk user data backup skipped: estimated" << outcome.bytes
                                     << "bytes exceeds automatic cap";
        } else {
            // One timestamped session directory for all sources, so a second boundary
            // never splits a single startup check across two backup folders and
            // same-named sources cannot overwrite each other.
            outcome.complete =
                UserDataBackup::backupAtRiskPaths(sources, backupRoot, &outcome.locations, &outcome.sessionDir);
            UserDataBackup::pruneOldBackupSessions(backupRoot, 5);
            if (outcome.locations.isEmpty()) {
                // Fresh copy failed entirely: fall back to the newest covering session so the
                // dialog still points at something, flagged incomplete below.
                const QString fallbackSession =
                    UserDataBackup::findRecentCoveringBackupSession(backupRoot, sources, 24);
                if (!fallbackSession.isEmpty()) {
                    outcome.sessionDir = fallbackSession;
                    outcome.locations = UserDataBackup::listSessionBackupLocations(fallbackSession);
                    outcome.complete = false;
                }
            }
        }
    }
    for (const QString &backupLocation : outcome.locations) {
        qCInfo(MainWindowLog) << "Backed up at-risk user data to" << backupLocation;
    }
    if (!outcome.complete) {
        qCWarning(MainWindowLog) << "At-risk user data backup incomplete, session dir:" << outcome.sessionDir;
    }
    return outcome;
}

QString MainWindow::buildPortableAtRiskMessage(const AtRiskBackupOutcome &outcome, bool emptySources)
{
    const QString backupVerb = outcome.reused ? tr("is already available") : tr("has been created");
    const QString sizeInfo = tr("%1 MB (automatic backups are capped at %2 MB)")
                                 .arg(QString::number(outcome.bytes / 1048576.0, 'f', 1),
                                      QString::number(UserDataBackup::MAX_AUTO_BACKUP_BYTES / 1048576));
    if (!outcome.locations.isEmpty()) {
        QString message =
            tr("This portable install keeps its data inside the program directory, where the "
               "uninstallers shipped in Cockatrice 3.1.0/3.1.1 delete everything on uninstall or "
               "update.\n\nA safety backup of your decks, custom sets, settings, filters, themes, sounds and "
               "custom "
               "pictures %1 "
               "at:\n%2\n\nReplays and downloaded pictures are not included in this backup. Uninstalling "
               "with the old uninstaller still deletes the program "
               "directory, so keep your own copy of that backup until you have updated once.")
                .arg(backupVerb, outcome.locations.join(QLatin1Char('\n')));
        if (!outcome.complete) {
            message += tr("\n\nThe backup may be incomplete; please back up these folders manually as well.");
        }
        return message;
    }
    if (outcome.skippedDueToSize) {
        return tr("This portable install keeps its data inside the program directory, where the "
                  "uninstallers shipped in Cockatrice 3.1.0/3.1.1 delete everything on uninstall or "
                  "update.\n\nThe automatic safety backup was skipped because of its size: %1. "
                  "Please back up your data folder manually "
                  "before uninstalling or updating.\n\nReplays and downloaded pictures would not have been "
                  "included in that backup either.")
            .arg(sizeInfo);
    }
    if (emptySources) {
        return tr("This portable install keeps its data inside the program directory, where the "
                  "uninstallers shipped in Cockatrice 3.1.0/3.1.1 delete everything on uninstall or "
                  "update.\n\nNone of your data files exist on disk yet, so there was nothing to back up. "
                  "Files created there later are still at risk until you have updated once.");
    }
    return tr("This portable install keeps its data inside the program directory, where the "
              "uninstallers shipped in Cockatrice 3.1.0/3.1.1 delete everything on uninstall or "
              "update.\n\nThe automatic safety backup failed; please back up your data folder manually "
              "before uninstalling or updating.\n\nReplays and downloaded pictures would not have been "
              "included in that backup either.");
}

QString MainWindow::buildInstalledAtRiskMessage(const QStringList &offenders,
                                                const QStringList &sources,
                                                const AtRiskBackupOutcome &outcome)
{
    const QString backupVerb = outcome.reused ? tr("is already available") : tr("has been created");
    const QString sizeInfo = tr("%1 MB (automatic backups are capped at %2 MB)")
                                 .arg(QString::number(outcome.bytes / 1048576.0, 'f', 1),
                                      QString::number(UserDataBackup::MAX_AUTO_BACKUP_BYTES / 1048576));
    QString message = tr("These paths are configured inside the Cockatrice program directory:\n\n%1\n\n"
                         "Cockatrice keeps its own data outside the program directory by default. Files "
                         "stored inside it can be deleted by an update or by uninstalling Cockatrice. "
                         "Change them in the Settings dialog - most are on the General page - to keep "
                         "them safe.")
                          .arg(offenders.join(QLatin1Char('\n')));
    if (!outcome.locations.isEmpty()) {
        message += tr("\n\nA safety backup %1 at:\n%2").arg(backupVerb, outcome.locations.join(QLatin1Char('\n')));
        if (!outcome.complete) {
            message += tr("\n\nThe backup may be incomplete; please back up these folders manually as well.");
        }
        if (hasLooseDeckFile(offenders)) {
            message += tr("\n\nDeck files ending in .txt are not backed up - move those manually.");
        }
    } else if (outcome.skippedDueToSize) {
        message += tr("\n\nThe automatic safety backup was skipped because of its size: %1. "
                      "Please back up these folders manually.")
                       .arg(sizeInfo);
    } else if (sources.isEmpty()) {
        message += tr("\n\nNone of these paths exist on disk yet, so there was nothing to back up. "
                      "Files created there later are still at risk until the paths are moved.");
    } else {
        message += tr("\n\nThe automatic safety backup failed; please back up these folders manually.");
    }
    return message;
}

void MainWindow::showAtRiskDataWarning(const QString &title, const QString &message)
{
    QMessageBox box(QMessageBox::Warning, title, message, QMessageBox::Ok, this);
    box.setTextFormat(Qt::PlainText);
    auto *dontShowAgain = new QCheckBox(tr("Don't show this again"));
    box.setCheckBox(dontShowAgain);
    box.exec();
    if (dontShowAgain->isChecked()) {
        SettingsCache::instance().paths().setAppDirWarningAcknowledged(true);
    }
}

void MainWindow::warnAboutDataPathsInProgramDir()
{
    auto &paths = SettingsCache::instance().paths();
    if (paths.getAppDirWarningAcknowledged()) {
        return;
    }

    const QString appDir = QCoreApplication::applicationDirPath();
    const bool isPortable = SettingsCache::instance().getIsPortableBuild();

    // Sources to back up outside the install directory before warning. The uninstallers
    // shipped in 3.1.0/3.1.1 delete the whole install tree, so a safety copy is kept
    // even though the fixed uninstaller preserves foreign files - the destructive one
    // is already on disk on affected machines.
    AtRiskPaths atRisk;
    if (isPortable) {
        atRisk = collectPortableAtRiskPaths(appDir);
    } else {
        atRisk.offendersForDisplay = collectInstalledAtRiskPaths(appDir);
        if (atRisk.offendersForDisplay.isEmpty()) {
            return;
        }
        // Only existing paths can be copied; a configured-but-never-created path has
        // nothing to lose yet and must not turn the warning into a bogus "failed".
        for (const QString &existingOffender : atRisk.offendersForDisplay) {
            if (QFileInfo::exists(existingOffender)) {
                atRisk.sourcesToBackUp.append(existingOffender);
            }
        }
    }

    const AtRiskBackupOutcome outcome = backUpAtRiskPaths(atRisk.sourcesToBackUp, appDir);

    const QString title =
        isPortable ? tr("Portable data at risk on uninstall") : tr("Data paths inside the program directory");
    const QString message =
        isPortable ? buildPortableAtRiskMessage(outcome, atRisk.sourcesToBackUp.isEmpty())
                   : buildInstalledAtRiskMessage(atRisk.offendersForDisplay, atRisk.sourcesToBackUp, outcome);
    showAtRiskDataWarning(title, message);
}

void MainWindow::startupConfigCheck()
{
    warnAboutDataPathsInProgramDir();

    const bool isCleanInstall = SettingsCache::instance().network().getClientVersion() == CLIENT_INFO_NOT_SET;

    // The loader never runs checkUnknownSets() itself (see main.cpp): the
    // front-loaded parse runs before MainWindow exists, so
    // cardDatabaseNewSetsFound / cardDatabaseAllNewSetsEnabled would have no
    // receivers. MainWindow owns the check after startup.
    // On a clean install the onboarding wizard owns the first-run experience;
    // wait until it closes so the legacy "all sets enabled" welcome and the
    // Manage Sets dialog don't appear first.
    if (!isCleanInstall) {
        CardDatabaseManager::getInstance()->checkUnknownSets();
    }

    if (SettingsCache::instance().debug().getLocalGameOnStartup()) {
        LocalGameOptions options;
        options.numberPlayers = SettingsCache::instance().debug().getLocalGamePlayerCount();
        startLocalGame(options);
    }

    if (SettingsCache::instance().updates().getCheckUpdatesOnStartup()) {
        actCheckClientUpdates();
    }

    actCheckCommanderBracketDefinitionUpdates();

    if (isCleanInstall) {
        // no config found, 99% new clean install
        qCInfo(WindowMainStartupVersionLog)
            << "Startup: old client version empty, assuming first start after clean install";
        SettingsCache::instance().downloads().resetToDefaultURLs(); // populate the download urls
        SettingsCache::instance().network().setClientVersion(VERSION_STRING);
        actCheckServerUpdates();
        runFirstRunWizard(true);

        if (QString(VERSION_STRING).contains("custom", Qt::CaseInsensitive)) {
            SettingsCache::instance().updates().setCheckUpdatesOnStartup(false);
        } else if (QString(VERSION_STRING).contains("beta", Qt::CaseInsensitive)) {
            SettingsCache::instance().updates().setUpdateReleaseChannelIndex(1);
        }
    } else if (SettingsCache::instance().network().getClientVersion() != VERSION_STRING) {
        // config found, from another (presumably older) version
        qCInfo(WindowMainStartupVersionLog)
            << "Startup: old client version" << SettingsCache::instance().network().getClientVersion()
            << "differs, assuming first start after update";
        if (SettingsCache::instance().updates().getNotifyAboutNewVersion()) {
            alertForcedOracleRun(VERSION_STRING, true);
        }

        qCInfo(WindowMainStartupShortcutsLog) << "Migrating shortcuts after update detected.";
        SettingsCache::instance().shortcuts().migrateShortcuts();

        if (SettingsCache::instance().updates().getCheckUpdatesOnStartup()) {
            if (QString(VERSION_STRING).contains("custom", Qt::CaseInsensitive)) {
                qCInfo(WindowMainStartupShortcutsLog) << "Update has changed to custom version, disabling auto update";
                SettingsCache::instance().updates().setCheckUpdatesOnStartup(false);
            } else {
                int channel = 0;
                if (QString(VERSION_STRING).contains("beta", Qt::CaseInsensitive)) {
                    channel = 1;
                }
                if (SettingsCache::instance().getUpdateReleaseChannelIndex() != channel) {
                    qCInfo(WindowMainStartupShortcutsLog) << "Update has changed beta state, updating release channel.";
                    SettingsCache::instance().updates().setUpdateReleaseChannelIndex(channel);
                }
            }
        }

        SettingsCache::instance().network().setClientVersion(VERSION_STRING);
    } else {
        // previous config from this version found
        qCInfo(WindowMainStartupVersionLog) << "Startup: found config with current version";

        if (SettingsCache::instance().updates().getCardUpdateCheckRequired()) {
            if (SettingsCache::instance().updates().getStartupCardUpdateCheckPromptForUpdate()) {
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
                            SettingsCache::instance().updates().setStartupCardUpdateCheckPromptForUpdate(false);
                            SettingsCache::instance().updates().setStartupCardUpdateCheckAlwaysUpdate(true);
                            actCheckCardUpdatesBackground();
                            break;
                        case 3: // don't prompt again + don't run
                            SettingsCache::instance().updates().setStartupCardUpdateCheckPromptForUpdate(false);
                            SettingsCache::instance().updates().setStartupCardUpdateCheckAlwaysUpdate(false);
                            break;
                        default:
                            break;
                    }
                }
            } else if (SettingsCache::instance().updates().getStartupCardUpdateCheckAlwaysUpdate()) {
                actCheckCardUpdatesBackground();
            }
        }

        // Run the tips dialog only on subsequent startups.
        // On the first run after an install/update the startup is already crowded enough
        if (tip->successfulInit && SettingsCache::instance().personal().getShowTipsOnStartup() &&
            tip->newTipsAvailable) {
            tip->raise();
            tip->show();
        }
    }
}

void MainWindow::runFirstRunWizard(bool firstRun)
{
    auto *wizard = new FirstRunWizard(this);
    wizard->setAttribute(Qt::WA_DeleteOnClose);

    connect(wizard, &FirstRunWizard::cardDatabaseUpdateRequested, this, &MainWindow::actCheckCardUpdatesBackground);
    connect(wizard, &FirstRunWizard::manualCardDatabaseSetupRequested, this, &MainWindow::actCheckCardUpdates);
    connect(this, &MainWindow::cardDatabaseUpdateFinished, wizard, &FirstRunWizard::onCardDatabaseUpdateFinished);
    connect(this, &MainWindow::cardDatabaseUpdateProgress, wizard, &FirstRunWizard::onCardDatabaseUpdateProgress);
    connect(wizard, &FirstRunWizard::registerRequested, connectionController, &ConnectionController::registerToServer);
    connect(wizard, &FirstRunWizard::connectRequested, connectionController, &ConnectionController::connectToServer);

    if (firstRun) {
        // The onboarding wizard owns set handling for a clean install. Suppress
        // the legacy set dialogs while it's open and run checkUnknownSets() once
        // it closes so the sets end up enabled in the right order.
        firstRunWizardActive = true;
        connect(wizard, &QDialog::finished, this, [this] {
            QTimer::singleShot(0, this, [this] {
                CardDatabaseManager::getInstance()->checkUnknownSets();
                firstRunWizardActive = false;
            });
        });
    }

    wizard->setModal(true);
    wizard->show();
}

/**
 * Drives the server-based startup destinations (Server lobby, Server Room) through the intent
 * system: fetch saved credentials, connect to the configured server, then land on the Lobby or
 * join the configured room by name.
 */
void MainWindow::applyStartupDestination()
{
    // An explicit command-line connect takes precedence over the startup destination.
    if (!connectTo.isEmpty()) {
        return;
    }

    // A cockatrice:// link owns the startup connection while its chain runs;
    // connecting here would race (and tear down) the link's own connection.
    if (skipStartupAutoConnect) {
        return;
    }

    const int destination = SettingsCache::instance().tabs().getStartupTabIndex();
    if (destination != StartupTab::StartupTabServer && destination != StartupTab::StartupTabServerRoom) {
        return;
    }

    const QString host = SettingsCache::instance().tabs().getStartupServerHost();
    const QString port = SettingsCache::instance().tabs().getStartupServerPort();
    if (host.isEmpty() || port.isEmpty()) {
        qCWarning(WindowMainStartupLog) << "Startup destination needs a configured server";
        return;
    }

    auto serverContext = std::make_shared<ContextConnectToServer>();
    serverContext->hostname = host;
    serverContext->port = port;

    auto *credentials = new IntentGetLoginCredentials(serverContext.get());
    auto *connector = new IntentConnectToServer(getRemoteClient(), serverContext.get());

    connect(credentials, &Intent::finished, connector, &Intent::execute);
    connect(credentials, &Intent::failed, this, &MainWindow::startupDestinationFailed);
    connect(credentials, &Intent::cancelled, this, [this]() { startupDestinationFailed(tr("Sign-in cancelled")); });
    connect(connector, &Intent::finished, this,
            [this, destination, serverContext]() { onStartupDestinationConnected(destination, *serverContext); });
    connect(connector, &Intent::failed, this, &MainWindow::startupDestinationFailed);

    credentials->execute();
}

void MainWindow::onStartupDestinationConnected(int destination, const ContextConnectToServer &serverContext)
{
    // The server tab must exist: it is what requests the room list.
    if (!tabSupervisor->getTabServer()) {
        tabSupervisor->openTabServer();
    }

    if (destination == StartupTab::StartupTabServerRoom) {
        auto roomContext = std::make_unique<ContextJoinRoom>();
        roomContext->serverContext = serverContext;
        auto *roomIntent = new IntentOpenServerRoomByName(tabSupervisor, getRemoteClient(), std::move(roomContext),
                                                          SettingsCache::instance().tabs().getStartupRoomName());
        roomIntent->setParent(this);
        connect(roomIntent, &Intent::failed, this, &MainWindow::startupDestinationFailed);
        roomIntent->execute();
        return;
    }

    if (tabSupervisor->getTabServer()) {
        tabSupervisor->setCurrentWidget(tabSupervisor->getTabServer());
    } else {
        qCWarning(WindowMainStartupLog) << "Startup destination: server tab could not be opened";
    }
}

void MainWindow::startupDestinationFailed(const QString &reason)
{
    qCWarning(WindowMainStartupLog) << "Startup destination failed:" << reason;
}

bool MainWindow::startupDestinationConnectsToServer() const
{
    const int destination = SettingsCache::instance().tabs().getStartupTabIndex();
    return destination == StartupTab::StartupTabServer || destination == StartupTab::StartupTabServerRoom;
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
    actCheckCommanderBracketDefinitionUpdates();
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
    trayIcon->setIcon(themePixmap(QStringLiteral("cockatrice")));
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
    if (bClosingDown) {
        return;
    }
    bClosingDown = true;

    if (cardUpdateProcess && cardUpdateProcess->state() != QProcess::NotRunning) {
        const QString runningFor = cardUpdateStatusBar ? cardUpdateStatusBar->statusText() : QString();
        const QString message = runningFor.isEmpty()
                                    ? tr("A card database update is still running. Quitting now will cancel it.\n"
                                         "Are you sure you want to quit?")
                                    : tr("A card database update is still running: %1.\nQuitting now will cancel it.\n"
                                         "Are you sure you want to quit?")
                                          .arg(runningFor);
        if (QMessageBox::question(this, tr("Are you sure?"), message, QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::No) == QMessageBox::No) {
            event->ignore();
            bClosingDown = false;
            return;
        }
    }

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

bool MainWindow::closeForUpdate()
{
    // A shutdown is already being handled (e.g. this is reached from a nested event loop while
    // closeEvent() is blocked on a user prompt). close() would hit the re-entrancy guard and return
    // true without any shutdown having happened, so report faithfully instead: the caller must keep
    // the process alive until the user resolves whatever is blocking the close.
    if (bClosingDown) {
        return false;
    }
    return close();
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    } else if (event->type() == QEvent::ActivationChange) {
        if (isActiveWindow() && !bHasActivated) {
            bHasActivated = true;
            attemptStartupAutoConnect();
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

void MainWindow::handleCockatriceLink(const QString &url)
{
    urlParser->handle(url);
}

void MainWindow::attemptStartupAutoConnect()
{
    if (startupAutoConnectAttempted || skipStartupAutoConnect) {
        return;
    }
    startupAutoConnectAttempted = true;

    if (!connectTo.isEmpty()) {
        qCInfo(WindowMainStartupAutoconnectLog) << "Command line connect to " << connectTo;
        connectionController->connectToServerDirect(connectTo.host(), connectTo.port(), connectTo.userName(),
                                                    connectTo.password());
    } else if (SettingsCache::instance().servers().getAutoConnect() &&
               !SettingsCache::instance().debug().getLocalGameOnStartup() && !startupDestinationConnectsToServer()) {
        qCInfo(WindowMainStartupAutoconnectLog) << "Attempting auto-connect...";
        DlgConnect dlg(this);
        connectionController->connectToServerDirect(dlg.getHost(), static_cast<unsigned int>(dlg.getPort()),
                                                    dlg.getPlayerName(), dlg.getPassword());
    }
}

void MainWindow::onUrlChainFinished(bool connected)
{
    // A cockatrice:// link owns the startup connection while it runs. When its
    // chain ended without connecting (declined, invalid, offline), fall back to
    // the startup connection so the activation launch still behaves like a
    // normal launch.
    if (connected) {
        // The launch link connected, so the startup fallback has served its
        // purpose: drop the skip so a later mid-session link that ends declined
        // or offline cannot silently fire auto-connect or applyStartupDestination
        // again.
        skipStartupAutoConnect = false;
        return;
    }

    if (!skipStartupAutoConnect || getRemoteClient()->getStatus() != StatusDisconnected) {
        return;
    }

    if (startupDestinationConnectsToServer()) {
        // Users whose startup tab is a Server / Server Room connect through the
        // startup destination, not through auto-connect; retry that instead.
        qCInfo(WindowMainStartupAutoconnectLog) << "URL chain ended without a connection; retrying startup destination";
        skipStartupAutoConnect = false;
        applyStartupDestination();
        return;
    }

    qCInfo(WindowMainStartupAutoconnectLog) << "URL chain ended without a connection; retrying startup connect";
    skipStartupAutoConnect = false;
    attemptStartupAutoConnect();
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
    if (firstRunWizardActive || SettingsCache::instance().updates().getAlwaysEnableNewSets()) {
        CardDatabaseManager::getInstance()->enableAllUnknownSets();
        const auto reloadOk1 =
            QtConcurrent::run([] { CardDatabaseManager::getInstance()->reloadCardDatabasesAndNotify(); });
        return;
    }

    QMessageBox msgBox(this);
    msgBox.setWindowTitle(tr("New sets found"));
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText(tr("%n new set(s) found in the card database\n"
                      "Set code(s): %1\n"
                      "Do you want to enable it/them?",
                      "", numUnknownSets)
                       .arg(unknownSetsNames.join(", ")));

    QPushButton *yesButton = msgBox.addButton(tr("Yes"), QMessageBox::YesRole);
    QPushButton *yesAlwaysButton = msgBox.addButton(tr("Yes, always enable"), QMessageBox::YesRole);
    QPushButton *noButton = msgBox.addButton(tr("No"), QMessageBox::NoRole);
    QPushButton *settingsButton = msgBox.addButton(tr("View sets"), QMessageBox::ActionRole);
    msgBox.setDefaultButton(yesButton);

    msgBox.exec();

    if (msgBox.clickedButton() == yesButton) {
        CardDatabaseManager::getInstance()->enableAllUnknownSets();
        const auto reloadOk1 =
            QtConcurrent::run([] { CardDatabaseManager::getInstance()->reloadCardDatabasesAndNotify(); });
    } else if (msgBox.clickedButton() == yesAlwaysButton) {
        CardDatabaseManager::getInstance()->enableAllUnknownSets();
        const auto reloadOk1 =
            QtConcurrent::run([] { CardDatabaseManager::getInstance()->reloadCardDatabasesAndNotify(); });
        SettingsCache::instance().updates().setAlwaysEnableNewSets(true);
    } else if (msgBox.clickedButton() == noButton) {
        CardDatabaseManager::getInstance()->markAllSetsAsKnown();
    } else if (msgBox.clickedButton() == settingsButton) {
        CardDatabaseManager::getInstance()->markAllSetsAsKnown();
        actManageSets();
    }
}

void MainWindow::cardDatabaseAllNewSetsEnabled()
{
    auto *cardDatabase = CardDatabaseManager::getInstance();

    // With no card data there is nothing to have enabled and nothing to reparse.
    if (cardDatabase->getCardList().isEmpty()) {
        return;
    }

    // checkUnknownSets() enabled every set after the database had already been
    // parsed under the (empty) first-run enablement, so the parser dropped every
    // printing of every set. Reparse now that the sets are enabled, otherwise the
    // deck editor's card database display stays empty until the next launch.
    auto *watcher = new QFutureWatcher<void>(this);
    connect(watcher, &QFutureWatcher<void>::finished, this, [this, watcher] {
        watcher->deleteLater();
        if (firstRunWizardActive) {
            // The onboarding wizard owns the first-run messaging on a clean install.
            return;
        }
        QMessageBox::information(
            this, tr("Welcome"),
            tr("Hi! It seems like you're running this version of Cockatrice for the first time.\nAll the sets in the "
               "card "
               "database have been enabled.\nRead more about changing the set order or disabling specific sets and "
               "consequent effects in the \"Manage Sets\" dialog."));
        // The reload above replaces the set instances, so Manage Sets must open
        // only after the finished snapshot has been swapped in.
        actManageSets();
    });
    watcher->setFuture(QtConcurrent::run([cardDatabase] { cardDatabase->reloadCardDatabasesAndNotify(); }));
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
        qCDebug(CardDatabaseUpdateLog) << "A card database update is already running; asking it to come to the front";
        if (requestCardUpdateRaise()) {
            return;
        }
        if (background) {
            // A hidden run has nothing to raise: keep the user informed without blocking.
            announceCardUpdateRunning();
            return;
        }
        QMessageBox::information(this, tr("Information"),
                                 tr("The card database updater is already open.\n"
                                    "Look for the Oracle window - it may be behind this one."));
        return;
    }

    cardUpdateProcess = new QProcess(this);

    connect(cardUpdateProcess, &QProcess::errorOccurred, this, &MainWindow::cardUpdateError);
    connect(cardUpdateProcess, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
            &MainWindow::cardUpdateFinished);
    connect(cardUpdateProcess, &QProcess::readyReadStandardOutput, this, &MainWindow::cardUpdateProgressOutput);
    connect(cardUpdateProcess, &QProcess::readyReadStandardError, this, &MainWindow::cardUpdateStandardError);

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
    } else { // try and find the directory Oracle is stored in the build directory
        QDir findLocalDir(dir);
        findLocalDir.cdUp();
        findLocalDir.cd(getCardUpdaterBinaryName());
        if (findLocalDir.exists(binaryName)) {
            dir = findLocalDir;
            updaterCmd = dir.absoluteFilePath(binaryName);
        }
    }

    if (updaterCmd.isEmpty()) {
        qCWarning(CardDatabaseUpdateLog) << "Unable to locate the card database updater; looked for"
                                         << dir.absoluteFilePath(binaryName);
        QMessageBox::warning(this, tr("Error"),
                             tr("Unable to run the card database updater: ") + dir.absoluteFilePath(binaryName));
        exitCardDatabaseUpdate();
        emit cardDatabaseUpdateFinished(false);
        return;
    }

    cardUpdateOutputBuffer.clear();
    cardUpdateErrorOutput.clear();
    cardUpdateErrorPartial.clear();
    cardUpdateLoggedStage = CardUpdateStage::Unknown;
    cardUpdateProgressLogTimer.start();

    qCInfo(CardDatabaseUpdateLog) << "Starting the card database updater" << updaterCmd << "background:" << background;
    cardUpdateStatusBar->updateStarted();

    if (!background) {
        cardUpdateProcess->start(updaterCmd, QStringList());
    } else {
        cardUpdateProcess->start(updaterCmd, QStringList("-b"));
        statusBar()->showMessage(tr("Card database update running."));
        if (trayIcon && SettingsCache::instance().updates().getCardDatabaseUpdateTrayNotification()) {
            trayIcon->showMessage(tr("Card database update started"),
                                  tr("Cockatrice is updating the card database in the background."));
        }
    }
}

void MainWindow::cardUpdateProgressOutput()
{
    if (!cardUpdateProcess) {
        return;
    }
    cardUpdateOutputBuffer.append(cardUpdateProcess->readAllStandardOutput());
    while (true) {
        const int newline = cardUpdateOutputBuffer.indexOf('\n');
        if (newline < 0) {
            break;
        }
        const QByteArray line = cardUpdateOutputBuffer.left(newline);
        cardUpdateOutputBuffer.remove(0, newline + 1);
        // Protocol emitted by `oracle`: "PROGRESS <stage> <done> <total>"
        const std::optional<CardUpdateProgress> progress = CardUpdateProgress::fromProtocolLine(line);
        if (!progress.has_value()) {
            continue;
        }
        // Log stage changes and, while a stage runs, one line per second at most, so the
        // debug log shows liveness without flooding it.
        if (progress->stage != cardUpdateLoggedStage || cardUpdateProgressLogTimer.elapsed() >= 1000) {
            qCDebug(CardDatabaseUpdateLog)
                << "Card database update" << progress->stageToken() << progress->done << "/" << progress->total;
            cardUpdateLoggedStage = progress->stage;
            cardUpdateProgressLogTimer.restart();
        }
        emit cardDatabaseUpdateProgress(progress.value());
    }
}

void MainWindow::cardUpdateStandardError()
{
    if (!cardUpdateProcess) {
        return;
    }
    const QByteArray chunk = cardUpdateProcess->readAllStandardError();
    if (chunk.isEmpty()) {
        return;
    }
    cardUpdateErrorOutput.append(chunk);
    cardUpdateErrorPartial.append(chunk);
    int newline;
    while ((newline = cardUpdateErrorPartial.indexOf('\n')) >= 0) {
        const QByteArray line = cardUpdateErrorPartial.left(newline).trimmed();
        cardUpdateErrorPartial.remove(0, newline + 1);
        if (!line.isEmpty()) {
            qCWarning(CardDatabaseUpdateLog) << "Card database updater:" << line;
        }
    }
}

void MainWindow::exitCardDatabaseUpdate()
{
    if (!cardUpdateProcess) {
        return;
    }
    cardUpdateProcess->deleteLater();
    cardUpdateProcess = nullptr;
    statusBar()->clearMessage();

    // This can be the first successful load of the session (e.g. the onboarding
    // download), so it must own the set check for that load.
    reloadCardDatabaseAndCheckSets();
}

void MainWindow::reloadCardDatabaseAndCheckSets()
{
    auto *watcher = new QFutureWatcher<void>(this);
    connect(watcher, &QFutureWatcher<void>::finished, this, [watcher] {
        watcher->deleteLater();
        CardDatabaseManager::getInstance()->checkUnknownSets();
    });
    watcher->setFuture(QtConcurrent::run([] { CardDatabaseManager::getInstance()->loadCardDatabases(); }));
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

    if (!cardUpdateErrorOutput.isEmpty()) {
        error += "\n\n" + tr("Error output:") + "\n" + QString::fromUtf8(cardUpdateErrorOutput);
        cardUpdateErrorOutput.clear();
    }

    qCWarning(CardDatabaseUpdateLog) << "The card database updater exited with an error:" << error;
    exitCardDatabaseUpdate();
    QMessageBox::warning(this, tr("Error"), tr("The card database updater exited with an error:\n%1").arg(error));
    emit cardDatabaseUpdateFinished(false);
}

void MainWindow::cardUpdateFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (!cardUpdateProcess) {
        return;
    }
    cardUpdateProgressOutput(); // drain any progress lines not yet parsed

    const bool success = (exitStatus == QProcess::NormalExit) && (exitCode == 0);
    if (exitStatus == QProcess::NormalExit) {
        SettingsCache::instance().updates().setLastCardUpdateCheck(QDateTime::currentDateTime().date());
    }
    qCInfo(CardDatabaseUpdateLog) << "The card database updater finished with exit code" << exitCode << "exit status"
                                  << static_cast<int>(exitStatus) << "success:" << success;
    exitCardDatabaseUpdate();
    notifyCardUpdateFinishedFromTray(success);
    emit cardDatabaseUpdateFinished(success);
}

bool MainWindow::requestCardUpdateRaise()
{
    const QString serverName = scopedLocalServerName(QStringLiteral("CockatriceOracleRaise"));
    QLocalSocket socket;
    socket.connectToServer(serverName);
    if (!socket.waitForConnected(250)) {
        return false;
    }
    socket.write("RAISE\n");
    const bool acked = socket.waitForBytesWritten(250);
    socket.disconnectFromServer();
    return acked;
}

void MainWindow::announceCardUpdateRunning()
{
    if (SettingsCache::instance().userInterface().getShowStatusBar()) {
        statusBar()->showMessage(tr("Card database update running."));
    } else if (trayIcon && SettingsCache::instance().updates().getCardDatabaseUpdateTrayNotification()) {
        trayIcon->showMessage(tr("Card database update running"),
                              tr("Cockatrice is updating the card database in the background."));
    }
}

void MainWindow::notifyCardUpdateFinishedFromTray(bool success)
{
    if (SettingsCache::instance().userInterface().getShowStatusBar() || !trayIcon ||
        !SettingsCache::instance().updates().getCardDatabaseUpdateTrayNotification()) {
        return;
    }
    trayIcon->showMessage(tr("Card database update finished"),
                          success ? tr("The card database is up to date.") : tr("The card database update failed."));
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

void MainWindow::actCheckCommanderBracketDefinitionUpdates()
{
    auto *handler = new HandleCommanderBrackets(this);

    connect(handler, &HandleCommanderBrackets::sigBracketDefinitionsDownloaded, this,
            []() { qDebug() << "Bracket definitions loaded"; });

    handler->downloadBracketDefinitions();
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
    QString dir = SettingsCache::instance().paths().getCustomPicsPath();
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
        // Imported files can introduce sets the config has never seen.
        reloadCardDatabaseAndCheckSets();
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
        if (fileIndex > maxIndex) {
            maxIndex = fileIndex;
        }
    }

    return maxIndex + 1;
}

void MainWindow::actReloadCardDatabase()
{
    const auto reloadOk1 = QtConcurrent::run([] {
        CardDatabaseManager::getInstance()->reloadCardDatabasesAndNotify();
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
