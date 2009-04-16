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
#include "servergame.h"
#include "serversocket.h"
#include "counter.h"
#include <QtSql>
#include <QSettings>

Server::Server(QObject *parent)
 : QTcpServer(parent), nextGameId(1)
{
	settings = new QSettings("servatrice.ini", QSettings::IniFormat, this);
}

Server::~Server()
{
}

bool Server::openDatabase()
{
	settings->beginGroup("database");
	QSqlDatabase sqldb = QSqlDatabase::addDatabase("QMYSQL");
	sqldb.setHostName(settings->value("hostname").toString());
	sqldb.setDatabaseName(settings->value("database").toString());
	sqldb.setUserName(settings->value("user").toString());
	sqldb.setPassword(settings->value("password").toString());
	settings->endGroup();
	
	return sqldb.open();
}

void Server::addGame(const QString description, const QString password, const int maxPlayers, ServerSocket *creator)
{
	ServerGame *newGame = new ServerGame(creator, nextGameId++, description, password, maxPlayers);
	games << newGame;
	connect(newGame, SIGNAL(gameClosing()), this, SLOT(gameClosing()));
	newGame->addPlayer(creator);
}

void Server::incomingConnection(int socketId)
{
	ServerSocket *socket = new ServerSocket(this);
	socket->setSocketDescriptor(socketId);
	connect(socket, SIGNAL(createGame(const QString, const QString, const int, ServerSocket *)), this, SLOT(addGame(const QString, const QString, const int, ServerSocket *)));
	connect(socket, SIGNAL(joinGame(int, ServerSocket *)), this, SLOT(addClientToGame(int, ServerSocket *)));
	socket->initConnection();
}

AuthenticationResult Server::checkUserPassword(const QString &user, const QString &password)
{
	if (!QSqlDatabase::database().isOpen())
		if (!openDatabase()) {
			qCritical(QString("Database error: %1").arg(QSqlDatabase::database().lastError().text()).toLatin1());
			return PasswordWrong;
		}

	QSqlQuery query;
	query.prepare("select password from users where name = :name");
	query.bindValue(":name", user);
	if (!query.exec()) {
		qCritical(QString("Database error: %1").arg(query.lastError().text()).toLatin1());
		return PasswordWrong;
	}
	if (query.next()) {
		if (query.value(0).toString() == password)
			return PasswordRight;
		else
			return PasswordWrong;
	} else
		return UnknownUser;
}

ServerGame *Server::getGame(int gameId)
{
	QListIterator<ServerGame *> i(games);
	while (i.hasNext()) {
		ServerGame *tmp = i.next();
		if ((tmp->gameId == gameId) && !tmp->getGameStarted())
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
		if ((!tmp->getGameStarted())
		 && (tmp->getPlayerCount() < tmp->maxPlayers))
			result.append(tmp);
	}
	return result;
}

bool Server::checkGamePassword(int gameId, const QString &password)
{
	ServerGame *tmp;
	if ((tmp = getGame(gameId))) {
		if ((!tmp->getGameStarted())
		 && (!tmp->password.compare(password, Qt::CaseSensitive))
		 && (tmp->getPlayerCount() < tmp->maxPlayers))
			return true;
	}
	return false;
}

void Server::addClientToGame(int gameId, ServerSocket *client)
{
	ServerGame *tmp = getGame(gameId);
	tmp->addPlayer(client);
}

void Server::gameClosing()
{
	qDebug("Server::gameClosing");
	ServerGame *g = qobject_cast<ServerGame *>(sender());
	games.removeAt(games.indexOf(g));
}
