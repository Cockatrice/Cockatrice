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
#include "server.h"
#include "servergamethread.h"
#include "servergame.h"
#include "serversocket.h"
#include "counter.h"
#include <QtSql>

Server::Server(QObject *parent)
 : QTcpServer(parent)
{
}

Server::~Server()
{
}

bool Server::openDatabase()
{
	QSqlDatabase sqldb = QSqlDatabase::addDatabase("QMYSQL");
	sqldb.setHostName("localhost");
	sqldb.setDatabaseName("cockatrice");
	sqldb.setUserName("cockatrice");
	sqldb.setPassword("45CdX6rmd");
	return sqldb.open();
}

void Server::gameCreated(ServerGame *_game, ServerSocket *_creator)
{
	games << _game;
	_creator->moveToThread(_game->thread());
	_game->addPlayer(_creator);
}

void Server::addGame(const QString name, const QString description, const QString password, const int maxPlayers, ServerSocket *creator)
{
	ServerGameThread *newThread = new ServerGameThread(name, description, password, maxPlayers, creator);
	connect(newThread, SIGNAL(gameCreated(ServerGame *, ServerSocket *)), this, SLOT(gameCreated(ServerGame *, ServerSocket *)));
	connect(newThread, SIGNAL(finished()), this, SLOT(gameClosed()));
	newThread->start();
}

void Server::incomingConnection(int socketId)
{
	ServerSocket *socket = new ServerSocket(this);
	socket->setSocketDescriptor(socketId);
	connect(socket, SIGNAL(createGame(const QString, const QString, const QString, const int, ServerSocket *)), this, SLOT(addGame(const QString, const QString, const QString, const int, ServerSocket *)));
	connect(socket, SIGNAL(joinGame(const QString, ServerSocket *)), this, SLOT(addClientToGame(const QString, ServerSocket *)));
	socket->initConnection();
}

AuthenticationResult Server::checkUserPassword(const QString &user, const QString &password)
{
	QSqlQuery query;
	query.prepare("select password from users where name = :name");
	query.bindValue(":name", user);
	if (!query.exec()) {
		qCritical(QString("Database error: %1").arg(query.lastError().text()).toLatin1());
		exit(-1);
	}
	if (query.next()) {
		if (query.value(0).toString() == password)
			return PasswordRight;
		else
			return PasswordWrong;
	} else
		return UnknownUser;
}

ServerGame *Server::getGame(const QString &name)
{
	QListIterator<ServerGame *> i(games);
	while (i.hasNext()) {
		ServerGame *tmp = i.next();
		if ((!tmp->name.compare(name, Qt::CaseSensitive)) && !tmp->getGameStarted())
			return tmp;
	}
	return NULL;
}

QList<ServerGame *> Server::listOpenGames()
{
	QList<ServerGame *> result;
	QListIterator<ServerGame *> i(games);
	while (i.hasNext()) {
		ServerGame *tmp = i.next();
		tmp->mutex->lock();
		if ((!tmp->getGameStarted())
		 && (tmp->getPlayerCount() < tmp->maxPlayers))
			result.append(tmp);
		tmp->mutex->unlock();
	}
	return result;
}

bool Server::checkGamePassword(const QString &name, const QString &password)
{
	ServerGame *tmp;
	if ((tmp = getGame(name))) {
		QMutexLocker locker(tmp->mutex);
		if ((!tmp->getGameStarted())
		 && (!tmp->password.compare(password, Qt::CaseSensitive))
		 && (tmp->getPlayerCount() < tmp->maxPlayers))
			return true;
	}
	return false;
}

void Server::addClientToGame(const QString name, ServerSocket *client)
{
	ServerGame *tmp = getGame(name);
	client->moveToThread(tmp->thread());
	tmp->addPlayer(client);
}

void Server::gameClosed()
{
	qDebug("Server::gameClosed");
	ServerGameThread *t = qobject_cast<ServerGameThread *>(sender());
	games.removeAt(games.indexOf(t->getGame()));
	delete t;
}
