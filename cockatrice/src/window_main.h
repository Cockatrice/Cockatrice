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

class QGraphicsScene;
class GameView;
class Game;
class CardDatabase;
class Player;

class QVBoxLayout;
class CardInfoWidget;
class MessageLogWidget;
class QLineEdit;
class QPushButton;
class ServerZoneCard;
class ZoneViewLayout;
class ZoneViewWidget;

class MainWindow : public QMainWindow {
	Q_OBJECT
private slots:
	void hoverCard(QString name);
	void playerAdded(Player *player);
	void playerRemoved(Player *player);
	void statusChanged(ProtocolStatus _status);
	void playerIdReceived(int id, QString name);
	void serverTimeout();

	void actSay();

	void actConnect();
	void actDisconnect();
	void actRestartGame();
	void actLeaveGame();
	void actDeckEditor();
	void actFullScreen(bool checked);
	void actExit();

	void updateSceneSize();
signals:
	void logConnecting(QString hostname);
	void logDisconnected();
private:
	void createActions();
	void createMenus();
	QMenu *gameMenu, *actionsMenu, *cardMenu;
	QAction *aConnect, *aDisconnect, *aRestartGame, *aLeaveGame, *aDeckEditor, *aFullScreen, *aExit;
	QAction *aCloseMostRecentZoneView;
	QVBoxLayout *viewLayout;

	CardInfoWidget *cardInfo;
	MessageLogWidget *messageLog;
	QLineEdit *sayEdit;

	Client *client;
	QGraphicsScene *scene;
	GameView *view;
	Game *game;
	CardDatabase *db;
	ZoneViewLayout *zoneLayout;
public:
	MainWindow(QWidget *parent = 0);
protected:
	void closeEvent(QCloseEvent *event);
};

#endif
