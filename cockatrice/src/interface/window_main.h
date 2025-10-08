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
 * @brief TODO: Document this.
 */
#ifndef WINDOW_H
#define WINDOW_H

#include "../../../libcockatrice_network/libcockatrice/client/abstract_client.h"

#include <QList>
#include <QLoggingCategory>
#include <QMainWindow>
#include <QMessageBox>
#include <QProcess>
#include <QSystemTrayIcon>
#include <QtNetwork>
#include <libcockatrice/protocol/pb/response.pb.h>

inline Q_LOGGING_CATEGORY(WindowMainLog, "window_main");
inline Q_LOGGING_CATEGORY(WindowMainStartupLog, "window_main.startup");
inline Q_LOGGING_CATEGORY(WindowMainStartupVersionLog, "window_main.startup.version");
inline Q_LOGGING_CATEGORY(WindowMainStartupShortcutsLog, "window_main.startup.shortcuts");
inline Q_LOGGING_CATEGORY(WindowMainStartupAutoconnectLog, "window_main.startup.autoconnect");

class Release;
class DlgConnect;
class DlgViewLog;
class GameReplay;
class HandlePublicServers;
class LocalClient;
class LocalServer;
class QThread;
class RemoteClient;
class ServerInfo_User;
class TabSupervisor;
class WndSets;
class DlgTipOfTheDay;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public slots:
    void actCheckCardUpdates();
    void actCheckCardUpdatesBackground();
    void actCheckServerUpdates();
    void actCheckClientUpdates();
    void actConnect();
    void actExit();
private slots:
    void updateTabMenu(const QList<QMenu *> &newMenuList);
    void statusChanged(ClientStatus _status);
    void processConnectionClosedEvent(const Event_ConnectionClosed &event);
    void processServerShutdownEvent(const Event_ServerShutdown &event);
    void serverTimeout();
    void loginError(Response::ResponseCode r, QString reasonStr, quint32 endTime, QList<QString> missingFeatures);
    void registerError(Response::ResponseCode r, QString reasonStr, quint32 endTime);
    void activateError();
    void socketError(const QString &errorStr);
    void protocolVersionMismatch(int localVersion, int remoteVersion);
    void userInfoReceived(const ServerInfo_User &userInfo);
    void registerAccepted();
    void registerAcceptedNeedsActivate();
    void activateAccepted();
    void localGameEnded();
    void pixmapCacheSizeChanged(int newSizeInMBs);
    void notifyUserAboutUpdate();
    void actDisconnect();
    void actSinglePlayer();
    void actWatchReplay();
    void actFullScreen(bool checked);
    void actRegister();
    void actSettings();
    void actForgotPasswordRequest();
    void actAbout();
    void actTips();
    void actUpdate();
    void actViewLog();
    void actOpenSettingsFolder();
    void forgotPasswordSuccess();
    void forgotPasswordError();
    void promptForgotPasswordReset();
    void actShow();
    void promptForgotPasswordChallenge();
    void showWindowIfHidden();

    void cardUpdateError(QProcess::ProcessError err);
    void cardUpdateFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void refreshShortcuts();
    void cardDatabaseLoadingFailed();
    void cardDatabaseNewSetsFound(int numUnknownSets, QStringList unknownSetsNames);
    void cardDatabaseAllNewSetsEnabled();

    void checkClientUpdatesFinished(bool needToUpdate, bool isCompatible, Release *release);

    void actOpenCustomFolder();
    void actOpenCustomsetsFolder();
    void actAddCustomSet();
    void actReloadCardDatabase();

    void actManageSets();
    void actEditTokens();

    void startupConfigCheck();
    void alertForcedOracleRun(const QString &version, bool isUpdate);

private:
    static const QString appName;
    static const QStringList fileNameFilters;
    void setClientStatusTitle();
    void retranslateUi();
    void createActions();
    void createMenus();

    void createTrayIcon();
    int getNextCustomSetPrefix(QDir dataDir);
    inline QString getCardUpdaterBinaryName()
    {
        return "oracle";
    };
    void createCardUpdateProcess(bool background = false);
    void exitCardDatabaseUpdate();

    void startLocalGame(int numberPlayers);

    QList<QMenu *> tabMenus;
    QMenu *cockatriceMenu, *dbMenu, *tabsMenu, *helpMenu, *trayIconMenu;
    QAction *aAbout, *aSettings, *aShow, *aExit;
    QAction *aConnect, *aDisconnect, *aRegister, *aForgotPassword, *aSinglePlayer, *aWatchReplay, *aFullScreen;
    QAction *aManageSets, *aEditTokens, *aOpenCustomFolder, *aOpenCustomsetsFolder, *aAddCustomSet,
        *aReloadCardDatabase;
    QAction *aTips, *aUpdate, *aCheckCardUpdates, *aCheckCardUpdatesBackground, *aStatusBar, *aViewLog,
        *aOpenSettingsFolder;

    TabSupervisor *tabSupervisor;
    WndSets *wndSets;
    RemoteClient *client;
    QThread *clientThread;
    LocalServer *localServer;
    bool bHasActivated, askedForDbUpdater;
    QMessageBox serverShutdownMessageBox;
    QProcess *cardUpdateProcess;
    DlgViewLog *logviewDialog;
    DlgConnect *dlgConnect;
    GameReplay *replay;
    DlgTipOfTheDay *tip;
    QUrl connectTo;

public:
    explicit MainWindow(QWidget *parent = nullptr);
    void setConnectTo(QString url)
    {
        connectTo = QUrl(QString("cockatrice://%1").arg(url));
    }
    ~MainWindow() override;

    TabSupervisor *getTabSupervisor() const
    {
        return tabSupervisor;
    }

protected:
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;
    QString extractInvalidUsernameMessage(QString &in);
};

#endif
