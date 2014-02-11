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

#include <QMainWindow>
#include "abstractclient.h"
#include "pb/response.pb.h"

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
    void loginError(Response::ResponseCode r, QString reasonStr, quint32 endTime);
    void socketError(const QString &errorStr);
    void protocolVersionMismatch(int localVersion, int remoteVersion);
    void userInfoReceived(const ServerInfo_User &userInfo);
    void localGameEnded();

    void actConnect();
    void actDisconnect();
    void actSinglePlayer();
    void actWatchReplay();
    void actDeckEditor();
    void actFullScreen(bool checked);
    void actSettings();
    void actExit();
    
    void actAbout();
private:
    static const QString appName;
    void setClientStatusTitle();
    void retranslateUi();
    void createActions();
    void createMenus();
    QList<QMenu *> tabMenus;
    QMenu *cockatriceMenu, *helpMenu;
    QAction *aConnect, *aDisconnect, *aSinglePlayer, *aWatchReplay, *aDeckEditor, *aFullScreen, *aSettings, *aExit,
        *aAbout;
    TabSupervisor *tabSupervisor;

    RemoteClient *client;
    QThread *clientThread;
    
    LocalServer *localServer;
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
protected:
    void closeEvent(QCloseEvent *event);
    void changeEvent(QEvent *event);
};

#endif
