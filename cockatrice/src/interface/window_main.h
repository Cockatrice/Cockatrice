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
/**
 * @file window_main.h
 * @ingroup Core
 */
//! \todo Document this file.
#ifndef WINDOW_H
#define WINDOW_H

#include "../client/lag_monitor.h"
#include "../client/network/update/client/release_channel.h"
#include "card_database_update/card_update_progress.h"
#include "connection_controller/remote_connection_controller.h"

#include <QByteArray>
#include <QDir>
#include <QElapsedTimer>
#include <QList>
#include <QLoggingCategory>
#include <QMainWindow>
#include <QMenu>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QtGlobal>
#include <libcockatrice/network/client/abstract/abstract_client.h>
#include <qtmetamacros.h>

class QAction;
class QWidget;
struct LocalGameOptions;

inline Q_LOGGING_CATEGORY(WindowMainLog, "window_main");
inline Q_LOGGING_CATEGORY(WindowMainStartupLog, "window_main.startup");
inline Q_LOGGING_CATEGORY(WindowMainStartupVersionLog, "window_main.startup.version");
inline Q_LOGGING_CATEGORY(WindowMainStartupShortcutsLog, "window_main.startup.shortcuts");
inline Q_LOGGING_CATEGORY(WindowMainStartupAutoconnectLog, "window_main.startup.autoconnect");
inline Q_LOGGING_CATEGORY(CardDatabaseUpdateLog, "card_database.update");

class DlgViewLog;
class GameReplay;
class LocalServer;
class LatencyStatusWidget;
class CardDatabaseUpdateStatusBar;
class RemoteClient;
class TabSupervisor;
class WndSets;
class DlgTipOfTheDay;
struct ContextConnectToServer;
class IntentUrlParser;

class MainWindow : public QMainWindow
{
    Q_OBJECT
signals:
    /** @brief Emitted after the card-database update subprocess exits. */
    void cardDatabaseUpdateFinished(bool success);

    /** @brief Emitted while the card-database update subprocess runs.
     *         @p progress.done/@p progress.total are byte counts for the first two stages
     *         and set indices for "import". */
    void cardDatabaseUpdateProgress(const CardUpdateProgress &progress);

public slots:
    void actCheckCardUpdates();
    void actCheckCardUpdatesBackground();
    void actCheckServerUpdates();
    void actCheckClientUpdates();
    void actConnect();
    void actExit();
    void handleCockatriceLink(const QString &url);
private slots:
    void updateTabMenu(const QList<QMenu *> &newMenuList);
    void statusChanged(ClientStatus _status);
    void localGameEnded();
    void pixmapCacheSizeChanged(int newSizeInMBs);
    void actDisconnect();
    void actSinglePlayer();
    void actWatchReplay();
    void actFullScreen(bool checked);
    void actSettings();
    void actAbout();
    void actTips();
    void actUpdate();
    void actViewLog();
    void actOpenSettingsFolder();
    void actShow();
    void showWindowIfHidden();
    void onUrlChainFinished(bool connected);

    void cardUpdateError(QProcess::ProcessError err);
    void cardUpdateFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void cardUpdateProgressOutput();
    void cardUpdateStandardError();
    void refreshShortcuts();
    void cardDatabaseLoadingFailed();
    void cardDatabaseNewSetsFound(int numUnknownSets, QStringList unknownSetsNames);
    void cardDatabaseAllNewSetsEnabled();

    void checkClientUpdatesFinished(bool needToUpdate, bool isCompatible, Release *release);
    void actCheckCommanderBracketDefinitionUpdates();

    void actOpenCustomFolder();
    void actOpenCustomsetsFolder();
    void actAddCustomSet();
    void actReloadCardDatabase();

    void actManageSets();
    void actEditTokens();

    void startupConfigCheck();
    void alertForcedOracleRun(const QString &version, bool isUpdate);

    void applyStartupDestination();
    void onStartupDestinationConnected(int destination, const ContextConnectToServer &serverContext);
    void startupDestinationFailed(const QString &reason);
    [[nodiscard]] bool startupDestinationConnectsToServer() const;

    void attemptStartupAutoConnect();

private:
    static const QString appName;
    static const QStringList fileNameFilters;
    void retranslateUi();
    void createActions();
    void createMenus();

    void createTrayIcon();
    int getNextCustomSetPrefix(QDir dataDir);

    void runFirstRunWizard(bool firstRun = false);

    inline QString getCardUpdaterBinaryName()
    {
        return "oracle";
    }
    void createCardUpdateProcess(bool background = false);
    void exitCardDatabaseUpdate();
    /**
     * @brief Reloads the card database on a worker thread and runs checkUnknownSets() once the finished
     *        snapshot has been swapped in.
     *
     * The loader never runs checkUnknownSets() itself (see main.cpp); post-update loads that can introduce
     * new sets must own the check, and it has to run after the queued snapshot swap, not on the worker.
     */
    void reloadCardDatabaseAndCheckSets();
    /**
     * @brief Tries to bring the open card database updater window to the front.
     * @return true when the updater acknowledged the request
     */
    bool requestCardUpdateRaise();
    /** @brief Tells the user an update is running without blocking, honoring a hidden status bar. */
    void announceCardUpdateRunning();
    /** @brief Posts a tray notification when the status bar cannot show progress. */
    void notifyCardUpdateFinishedFromTray(bool success);

    void startLocalGame(const LocalGameOptions &options);

    QList<QMenu *> tabMenus;
    QMenu *cockatriceMenu, *dbMenu, *tabsMenu, *helpMenu, *trayIconMenu;
    QAction *aAbout, *aSettings, *aShow, *aExit;
    QAction *aConnect, *aDisconnect, *aRegister, *aForgotPassword, *aSinglePlayer, *aWatchReplay, *aFullScreen;
    QAction *aManageSets, *aEditTokens, *aOpenCustomFolder, *aOpenCustomsetsFolder, *aAddCustomSet,
        *aReloadCardDatabase;
    QAction *aTips, *aUpdate, *aCheckCardUpdates, *aCheckCardUpdatesBackground, *aFirstRunWizard, *aStatusBar,
        *aViewLog, *aOpenSettingsFolder;

    TabSupervisor *tabSupervisor;
    IntentUrlParser *urlParser;
    WndSets *wndSets;
    ConnectionController *connectionController;
    LocalServer *localServer;
    LagMonitor lagMonitor;                        ///< watches the main thread for event loop stalls
    LatencyStatusWidget *latencyStatus = nullptr; ///< status bar widget with live round-trip stats and history graph
    bool bHasActivated, askedForDbUpdater;
    bool bClosingDown = false; ///< guards closeEvent() against re-entrancy
    bool skipStartupAutoConnect = false;
    bool startupAutoConnectAttempted = false;
    bool firstRunWizardActive = false;
    QProcess *cardUpdateProcess;
    QByteArray cardUpdateOutputBuffer;
    QByteArray cardUpdateErrorOutput;  ///< diagnostic output collected from the updater's stderr
    QByteArray cardUpdateErrorPartial; ///< incomplete trailing stderr line, kept for line-based logging
    CardUpdateStage cardUpdateLoggedStage = CardUpdateStage::Unknown; ///< stage of the last logged progress update
    QElapsedTimer cardUpdateProgressLogTimer;                   ///< throttles progress logging to the running stage
    CardDatabaseUpdateStatusBar *cardUpdateStatusBar = nullptr; ///< status bar widget tracking the card database update
    DlgViewLog *logviewDialog;
    GameReplay *replay;
    DlgTipOfTheDay *tip;
    QUrl connectTo;

public:
    explicit MainWindow(QWidget *parent = nullptr);
    void setConnectTo(QString url)
    {
        connectTo = QUrl(QString("cockatrice://%1").arg(url));
    }
    // When set, the window's own startup connection (--connect or auto-connect
    // on first activation) is skipped. Used for activation launches: the intent
    // chain triggered by a cockatrice:// URL owns the connection, and letting
    // auto-connect race against it caused two connectToServer calls to tear
    // each other down. onUrlChainFinished() clears this and retries the startup
    // connection when the link's chain ended without connecting.
    void setSkipStartupAutoConnect(bool skip)
    {
        skipStartupAutoConnect = skip;
    }
    ~MainWindow() override;

    RemoteClient *getRemoteClient() const
    {
        return connectionController->client();
    }

    TabSupervisor *getTabSupervisor() const
    {
        return tabSupervisor;
    }

    /**
     * @brief Closes the window so an update installer can replace the running binaries.
     *        Returns true only if the shutdown actually ran (settings flushed, tabs shut down);
     *        false if the close was vetoed by the user or is already in progress.
     */
    bool closeForUpdate();

protected:
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;
};

#endif
