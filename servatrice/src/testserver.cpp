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
#include "testserver.h"

TestServer::TestServer(QObject *parent)
 : QTcpServer(parent)
{
}

TestServer::~TestServer()
{
}

void TestServer::gameCreated(TestServerGame *_game, TestServerSocket *_creator)
{
	games << _game;
	_creator->moveToThread(_game->thread());
	_game->addPlayer(_creator);
}

void TestServer::addGame(const QString name, const QString description, const QString password, const int maxPlayers, TestServerSocket *creator)
{
	TestServerGameThread *newThread = new TestServerGameThread(name, description, password, maxPlayers, creator);
	connect(newThread, SIGNAL(gameCreated(TestServerGame *, TestServerSocket *)), this, SLOT(gameCreated(TestServerGame *, TestServerSocket *)));
	connect(newThread, SIGNAL(finished()), this, SLOT(gameClosed()));
	newThread->start();
}

void TestServer::incomingConnection(int socketId)
{
	TestServerSocket *socket = new TestServerSocket(this);
	socket->setSocketDescriptor(socketId);
	connect(socket, SIGNAL(createGame(const QString, const QString, const QString, const int, TestServerSocket *)), this, SLOT(addGame(const QString, const QString, const QString, const int, TestServerSocket *)));
	connect(socket, SIGNAL(joinGame(const QString, TestServerSocket *)), this, SLOT(addClientToGame(const QString, TestServerSocket *)));
	socket->initConnection();
}

TestServerGame *TestServer::getGame(const QString &name)
{
	QListIterator<TestServerGame *> i(games);
	while (i.hasNext()) {
		TestServerGame *tmp = i.next();
		if ((!tmp->name.compare(name, Qt::CaseSensitive)) && !tmp->getGameStarted())
			return tmp;
	}
	return NULL;
}

QList<TestServerGame *> TestServer::listOpenGames()
{
	QList<TestServerGame *> result;
	QListIterator<TestServerGame *> i(games);
	while (i.hasNext()) {
		TestServerGame *tmp = i.next();
		tmp->mutex->lock();
		if ((!tmp->getGameStarted())
		 && (tmp->getPlayerCount() < tmp->maxPlayers))
			result.append(tmp);
		tmp->mutex->unlock();
	}
	return result;
}

bool TestServer::checkGamePassword(const QString &name, const QString &password)
{
	TestServerGame *tmp;
	if ((tmp = getGame(name))) {
		QMutexLocker locker(tmp->mutex);
		if ((!tmp->getGameStarted())
		 && (!tmp->password.compare(password, Qt::CaseSensitive))
		 && (tmp->getPlayerCount() < tmp->maxPlayers))
			return true;
	}
	return false;
}

void TestServer::addClientToGame(const QString name, TestServerSocket *client)
{
	TestServerGame *tmp = getGame(name);
	client->moveToThread(tmp->thread());
	tmp->addPlayer(client);
}

void TestServer::gameClosed()
{
	qDebug("TestServer::gameClosed");
	TestServerGameThread *t = qobject_cast<TestServerGameThread *>(sender());
	games.removeAt(games.indexOf(t->getGame()));
	delete t;
}
