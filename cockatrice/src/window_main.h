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

class GameView;
class GameScene;
class Game;
class CardDatabase;
class Player;

class QTranslator;
class QVBoxLayout;
class CardInfoWidget;
class MessageLogWidget;
class QLabel;
class QLineEdit;
class QPushButton;
class ServerZoneCard;
class ZoneViewLayout;
class ZoneViewWidget;
class PhasesToolbar;
class GameSelector;
class ChatWidget;

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
	void actSettings();
	void actExit();
signals:
	void logConnecting(QString hostname);
	void logDisconnected();
private:
	void retranslateUi();
	void createActions();
	void createMenus();
	QMenu *gameMenu, *actionsMenu, *cardMenu;
	QAction *aConnect, *aDisconnect, *aRestartGame, *aLeaveGame, *aDeckEditor, *aFullScreen, *aSettings, *aExit;
	QAction *aCloseMostRecentZoneView;
	QVBoxLayout *viewLayout;

	CardInfoWidget *cardInfo;
	MessageLogWidget *messageLog;
	QLabel *sayLabel;
	QLineEdit *sayEdit;
	PhasesToolbar *phasesToolbar;
	GameSelector *gameSelector;
	ChatWidget *chatWidget;

	Client *client;
	GameScene *scene;
	GameView *view;
	Game *game;
	CardDatabase *db;
	ZoneViewLayout *zoneLayout;
	QTranslator *translator;
public:
	MainWindow(QTranslator *_translator, QWidget *parent = 0);
protected:
	void closeEvent(QCloseEvent *event);
	void changeEvent(QEvent *event);
};

#endif
