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
#include "rng_qt.h"
#include "chatchannel.h"
#include <QtSql>
#include <QSettings>

Server::Server(QObject *parent)
	: QTcpServer(parent), nextGameId(0)
{
	rng = new RNG_Qt(this);
	
	settings = new QSettings("servatrice.ini", QSettings::IniFormat, this);

	QString dbType = settings->value("database/type").toString();
	if (dbType == "mysql")
		openDatabase();
	
	int size = settings->beginReadArray("chatchannels");
	for (int i = 0; i < size; ++i) {
	  	settings->setArrayIndex(i);
		chatChannelList << new ChatChannel(settings->value("name").toString(),
						   settings->value("description").toString(),
						   settings->value("autojoin").toBool(),
						   settings->value("joinmessage").toStringList());
	}
	settings->endArray();
	
	for (int i = 0; i < chatChannelList.size(); ++i)
	  	connect(chatChannelList[i], SIGNAL(channelInfoChanged()), this, SLOT(broadcastChannelUpdate()));
	
	loginMessage = settings->value("messages/login").toStringList();
}

Server::~Server()
{
}

bool Server::openDatabase()
{
	if (!QSqlDatabase::connectionNames().isEmpty())
		QSqlDatabase::removeDatabase(QSqlDatabase::database().connectionNames().at(0));
	
	settings->beginGroup("database");
	QSqlDatabase sqldb = QSqlDatabase::addDatabase("QMYSQL");
	sqldb.setHostName(settings->value("hostname").toString());
	sqldb.setDatabaseName(settings->value("database").toString());
	sqldb.setUserName(settings->value("user").toString());
	sqldb.setPassword(settings->value("password").toString());
	settings->endGroup();
	
	if (!sqldb.open())
		return false;
	
	if (!nextGameId) {
		QSqlQuery query;
		if (!query.exec("select max(id) from games"))
			return false;
		if (!query.next())
			return false;
		nextGameId = query.value(0).toInt() + 1;
		qDebug(QString("set nextGameId to %1").arg(nextGameId).toLatin1());
	}
	return true;
}

void Server::addGame(const QString description, const QString password, int maxPlayers, bool spectatorsAllowed, ServerSocket *creator)
{
	ServerGame *newGame = new ServerGame(creator, nextGameId++, description, password, maxPlayers, spectatorsAllowed, this);
	games.insert(newGame->getGameId(), newGame);
	connect(newGame, SIGNAL(gameClosing()), this, SLOT(gameClosing()));
	newGame->addPlayer(creator, false);
	
	broadcastGameListUpdate(newGame);
}

void Server::incomingConnection(int socketId)
{
	ServerSocket *socket = new ServerSocket(this);
	socket->setSocketDescriptor(socketId);
	connect(socket, SIGNAL(createGame(const QString, const QString, int, bool, ServerSocket *)), this, SLOT(addGame(const QString, const QString, int, bool, ServerSocket *)));
	socket->initConnection();
	players << socket;
}

AuthenticationResult Server::checkUserPassword(const QString &user, const QString &password)
{
	const QString method = settings->value("authentication/method").toString();
	if (method == "none")
		return UnknownUser;
	else if (method == "sql") {
		if (!QSqlDatabase::database().exec("select 1").isActive())
			openDatabase();
	
		QSqlQuery query;
		query.prepare("select password from players where name = :name");
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
	} else
		return UnknownUser;
}

ServerGame *Server::getGame(int gameId) const
{
	return games.value(gameId);
}

void Server::broadcastGameListUpdate(ServerGame *game)
{
	qDebug(QString("broadcastGameListUpdate() to %1 players").arg(players.size()).toLatin1());
	QString line = game->getGameListLine();
	for (int i = 0; i < players.size(); i++)
		if (players[i]->getAcceptsGameListChanges())
			players[i]->msg(line);
}

void Server::broadcastChannelUpdate()
{
	QString line = qobject_cast<ChatChannel *>(sender())->getChannelListLine();
	for (int i = 0; i < players.size(); ++i)
	  	if (players[i]->getAcceptsChatChannelListChanges())
		  	players[i]->msg(line);
}

void Server::gameClosing()
{
	qDebug("Server::gameClosing");
	games.remove(games.key(static_cast<ServerGame *>(sender())));
}

void Server::removePlayer(ServerSocket *player)
{
	players.removeAt(players.indexOf(player));
	qDebug(QString("Server::removePlayer: %1 players left").arg(players.size()).toLatin1());
}
