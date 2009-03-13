/***************************************************************************
 *   Copyright (C) 2008 by Max-Wilhelm Bruker   *
 *   brukie@laptop   *
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
#ifndef TESTSERVERGAME_H
#define TESTSERVERGAME_H

#include <QThread>
#include <QMutex>
#include <QStringList>
#include "testserversocket.h"
#include "testrandom.h"

class TestServerSocket;

class TestServerGame : public QObject {
	Q_OBJECT
private:
	QList<TestServerSocket *> players;
	bool gameStarted;
	int activePlayer;
	int activePhase;
public slots:
	void broadcastEvent(const QString &event, TestServerSocket *player);
public:
	QMutex *mutex;
	TestRandom *rnd;
	QString name;
	QString description;
	QString password;
	int maxPlayers;
	TestServerGame(QString _name, QString _description, QString _password, int _maxPlayers, QObject *parent = 0);
	~TestServerGame();
	bool getGameStarted();
	int getPlayerCount();
	QStringList getPlayerNames();
	TestServerSocket *getPlayer(int player_id);
	void addPlayer(TestServerSocket *player);
	void removePlayer(TestServerSocket *player);
	void startGameIfReady();
	void msg(const QString &s);
	int getActivePlayer() { return activePlayer; }
	int getActivePhase() { return activePhase; }
	void setActivePlayer(int _activePlayer);
	void setActivePhase(int _activePhase);
	
};

#endif
