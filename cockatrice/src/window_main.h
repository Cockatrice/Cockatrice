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
#ifndef WINDOW_H
#define WINDOW_H

#include <QList>
#include <QMainWindow>
#include <QMessageBox>
#include <QProcess>
#include <QSystemTrayIcon>
#include <QtNetwork>

#include "abstractclient.h"
#include "pb/response.pb.h"

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
    void actCheckServerUpdates();
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
    void actConnect();
    void actDisconnect();
    void actSinglePlayer();
    void actWatchReplay();
    void actDeckEditor();
    void actFullScreen(bool checked);
    void actRegister();
    void actSettings();
    void actExit();
    void actForgotPasswordRequest();
    void actAbout();
    void actTips();
    void actUpdate();
    void actViewLog();
    void forgotPasswordSuccess();
    void forgotPasswordError();
    void promptForgotPasswordReset();
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void promptForgotPasswordChallenge();
    void showWindowIfHidden();

    void cardUpdateError(QProcess::ProcessError err);
    void cardUpdateFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void refreshShortcuts();
    void cardDatabaseLoadingFailed();
    void cardDatabaseNewSetsFound(int numUnknownSets, QStringList unknownSetsNames);
    void cardDatabaseAllNewSetsEnabled();

    void actOpenCustomFolder();
    void actOpenCustomsetsFolder();
    void actAddCustomSet();

    void actManageSets();
    void actEditTokens();

    void alertForcedOracleRun(const QString &);

private:
    static const QString appName;
    static const QStringList fileNameFilters;
    void setClientStatusTitle();
    void retranslateUi();
    void createActions();
    void createMenus();

    void createTrayIcon();
    void createTrayActions();
    int getNextCustomSetPrefix(QDir dataDir);
    // TODO: add a preference item to choose updater name for other games
    inline QString getCardUpdaterBinaryName()
    {
        return "oracle";
    };

    QList<QMenu *> tabMenus;
    QMenu *cockatriceMenu, *dbMenu, *helpMenu, *trayIconMenu;
    QAction *aConnect, *aDisconnect, *aSinglePlayer, *aWatchReplay, *aDeckEditor, *aFullScreen, *aSettings, *aExit,
        *aAbout, *aTips, *aCheckCardUpdates, *aRegister, *aUpdate, *aViewLog, *closeAction;
    QAction *aManageSets, *aEditTokens, *aOpenCustomFolder, *aOpenCustomsetsFolder, *aAddCustomSet;
    TabSupervisor *tabSupervisor;
    WndSets *wndSets;
    RemoteClient *client;
    QThread *clientThread;
    LocalServer *localServer;
    bool bHasActivated;
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

protected:
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;
    QString extractInvalidUsernameMessage(QString &in);
};

class MainUpdateHelper : public QObject
{
    Q_OBJECT

signals:
    void newVersionDetected(QString);

public:
    explicit MainUpdateHelper() = default;
    ~MainUpdateHelper() override = default;
    void testForNewVersion();
};

#endif
