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
#include <QSystemTrayIcon>
#include <QProcess>
#include <QMessageBox>
#include <QtNetwork>

#include "abstractclient.h"
#include "pb/response.pb.h"
#include "update_checker.h"

class TabSupervisor;
class RemoteClient;
class LocalClient;
class LocalServer;
class ServerInfo_User;
class QThread;

class MainWindow : public QMainWindow {
    Q_OBJECT
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
    
    void actAbout();
    void actUpdate();
    void actViewLog();

    void iconActivated(QSystemTrayIcon::ActivationReason reason);

    void showWindowIfHidden();

    void actCheckCardUpdates();
    void cardUpdateError(QProcess::ProcessError err);
    void cardUpdateFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void refreshShortcuts();
    void cardDatabaseLoadingFailed();
    void cardDatabaseNewSetsFound(int numUnknownSets, QStringList unknownSetsNames);
    void cardDatabaseAllNewSetsEnabled();

    void actOpenCustomFolder();
    void actOpenCustomsetsFolder();
    void actAddCustomSet();

    void actEditSets();
    void actEditTokens();
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
    inline QString getCardUpdaterBinaryName() { return "oracle"; };

    QList<QMenu *> tabMenus;
    QMenu *cockatriceMenu, *dbMenu, *helpMenu;
    QAction *aConnect, *aDisconnect, *aSinglePlayer, *aWatchReplay, *aDeckEditor, *aFullScreen, *aSettings, *aExit,
        *aAbout, *aCheckCardUpdates, *aRegister, *aUpdate, *aViewLog;
    QAction *aEditSets, *aEditTokens, *aOpenCustomFolder, *aOpenCustomsetsFolder, *aAddCustomSet;
    TabSupervisor *tabSupervisor;

    QMenu *trayIconMenu;

    QAction *closeAction;

    RemoteClient *client;
    QThread *clientThread;
    
    LocalServer *localServer;
    bool bHasActivated;

    QMessageBox serverShutdownMessageBox;
    QProcess * cardUpdateProcess;

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
protected:
    void closeEvent(QCloseEvent *event);
    void changeEvent(QEvent *event);
    QString extractInvalidUsernameMessage(QString & in);
};

#endif
