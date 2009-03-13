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
#ifndef TESTSERVER_H
#define TESTSERVER_H

#include <QTcpServer>
#include "testservergamethread.h"
#include "testservergame.h"

class TestServerGame;
class TestServerSocket;

class TestServer : public QTcpServer
{
	Q_OBJECT
private slots:
	void addGame(const QString name, const QString description, const QString password, const int maxPlayers, TestServerSocket *creator);
	void addClientToGame(const QString name, TestServerSocket *client);
	void gameCreated(TestServerGame *_game, TestServerSocket *_creator);
	void gameClosed();
public:
	TestServer(QObject *parent = 0);
	~TestServer();
	bool checkGamePassword(const QString &name, const QString &password);
	QList<TestServerGame *> listOpenGames();
	TestServerGame *getGame(const QString &name);
private:
	void incomingConnection(int SocketId);
	QList<TestServerGame *> games;
};

#endif
