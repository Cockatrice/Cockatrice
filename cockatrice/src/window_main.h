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
#include "client.h"

class TabSupervisor;

class MainWindow : public QMainWindow {
	Q_OBJECT
private slots:
//	void playerAdded(Player *player);
	void statusChanged(ClientStatus _status);
	void serverTimeout();

	void actConnect();
	void actDisconnect();
	void actDeckEditor();
	void actFullScreen(bool checked);
	void actSettings();
	void actExit();
signals:
	void logConnecting(QString hostname);
	void logConnected();
	void logDisconnected();
private:
	void retranslateUi();
	void createActions();
	void createMenus();
	QMenu *cockatriceMenu;
	QAction *aConnect, *aDisconnect, *aDeckEditor, *aFullScreen, *aSettings, *aExit;
	TabSupervisor *tabSupervisor;

	Client *client;
public:
	MainWindow(QWidget *parent = 0);
protected:
	void closeEvent(QCloseEvent *event);
	void changeEvent(QEvent *event);
};

#endif
