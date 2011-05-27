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
#include <QtSql>
#include <QSettings>
#include <QDebug>
#include <iostream>
#include "servatrice.h"
#include "server_room.h"
#include "serversocketinterface.h"
#include "serversocketthread.h"
#include "protocol.h"
#include "server_logger.h"
#include "main.h"

void Servatrice_TcpServer::incomingConnection(int socketDescriptor)
{
	if (threaded) {
		ServerSocketThread *sst = new ServerSocketThread(socketDescriptor, server, this);
		sst->start();
	} else {
		QTcpSocket *socket = new QTcpSocket;
		socket->setSocketDescriptor(socketDescriptor);
		logger->logMessage(QString("incoming connection: %1").arg(socket->peerAddress().toString()));
		
		new ServerSocketInterface(server, socket);
	}
}

Servatrice::Servatrice(QSettings *_settings, QObject *parent)
	: Server(parent), dbMutex(QMutex::Recursive), settings(_settings), uptime(0)
{
	pingClock = new QTimer(this);
	connect(pingClock, SIGNAL(timeout()), this, SIGNAL(pingClockTimeout()));
	pingClock->start(1000);
	
	banTimeoutClock = new QTimer(this);
	connect(banTimeoutClock, SIGNAL(timeout()), this, SLOT(updateBanTimer()));
	banTimeoutClock->start(60000);
	
	ProtocolItem::initializeHash();
	
	serverId = settings->value("server/id", 0).toInt();
	int statusUpdateTime = settings->value("server/statusupdate").toInt();
	statusUpdateClock = new QTimer(this);
	connect(statusUpdateClock, SIGNAL(timeout()), this, SLOT(statusUpdate()));
	if (statusUpdateTime != 0) {
		qDebug() << "Starting status update clock, interval " << statusUpdateTime << " ms";
		statusUpdateClock->start(statusUpdateTime);
	}
	
	bool threaded = settings->value("server/threaded", false).toInt();
	tcpServer = new Servatrice_TcpServer(this, threaded);
	int port = settings->value("server/port", 4747).toInt();
	qDebug() << "Starting server on port" << port;
	if (tcpServer->listen(QHostAddress::Any, port))
		qDebug() << "Server listening.";
	else
		qDebug() << "tcpServer->listen(): Error.";
	
	QString dbType = settings->value("database/type").toString();
	dbPrefix = settings->value("database/prefix").toString();
	if (dbType == "mysql")
		openDatabase();
	
	int size = settings->beginReadArray("rooms");
	for (int i = 0; i < size; ++i) {
	  	settings->setArrayIndex(i);
		
		QStringList gameTypes;
		int size2 = settings->beginReadArray("game_types");
		for (int j = 0; j < size2; ++j) {
			settings->setArrayIndex(j);
			gameTypes.append(settings->value("name").toString());
		}
		settings->endArray();
			
		Server_Room *newRoom = new Server_Room(
			i,
			settings->value("name").toString(),
			settings->value("description").toString(),
			settings->value("autojoin").toBool(),
			settings->value("joinmessage").toString(),
			gameTypes,
			this
		);
		addRoom(newRoom);
	}
	settings->endArray();
	
	updateLoginMessage();
	
	maxGameInactivityTime = settings->value("game/max_game_inactivity_time").toInt();
	maxPlayerInactivityTime = settings->value("game/max_player_inactivity_time").toInt();
	
	maxUsersPerAddress = settings->value("security/max_users_per_address").toInt();
	messageCountingInterval = settings->value("security/message_counting_interval").toInt();
	maxMessageCountPerInterval = settings->value("security/max_message_count_per_interval").toInt();
	maxMessageSizePerInterval = settings->value("security/max_message_size_per_interval").toInt();
	maxGamesPerUser = settings->value("security/max_games_per_user").toInt();
}

Servatrice::~Servatrice()
{
	prepareDestroy();
}

bool Servatrice::openDatabase()
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
	
	std::cerr << "Opening database...";
	if (!sqldb.open()) {
		std::cerr << "error" << std::endl;
		return false;
	}
	std::cerr << "OK" << std::endl;
	
	if (!nextGameId) {
		QSqlQuery query;
		if (!query.exec("select max(id) from " + dbPrefix + "_games"))
			return false;
		if (!query.next())
			return false;
		nextGameId = query.value(0).toInt() + 1;
		qDebug() << "set nextGameId to " << nextGameId;
	}
	return true;
}

void Servatrice::checkSql()
{
	QMutexLocker locker(&dbMutex);
	if (!QSqlDatabase::database().exec("select 1").isActive())
		openDatabase();
}

bool Servatrice::execSqlQuery(QSqlQuery &query)
{
	if (query.exec())
		return true;
	qCritical() << "Database error:" << query.lastError().text();
	return false;
}

AuthenticationResult Servatrice::checkUserPassword(const QString &user, const QString &password)
{
	QMutexLocker locker(&dbMutex);
	const QString method = settings->value("authentication/method").toString();
	if (method == "none")
		return UnknownUser;
	else if (method == "sql") {
		checkSql();
	
		QSqlQuery query;
		query.prepare("select banned, password from " + dbPrefix + "_users where name = :name and active = 1");
		query.bindValue(":name", user);
		if (!execSqlQuery(query))
			return PasswordWrong;
		
		if (query.next()) {
			if (query.value(0).toInt())
				return PasswordWrong;
			if (query.value(1).toString() == password)
				return PasswordRight;
			else
				return PasswordWrong;
		} else
			return UnknownUser;
	} else
		return UnknownUser;
}

bool Servatrice::userExists(const QString &user)
{
	QMutexLocker locker(&dbMutex);
	const QString method = settings->value("authentication/method").toString();
	if (method == "sql") {
		checkSql();
	
		QSqlQuery query;
		query.prepare("select 1 from " + dbPrefix + "_users where name = :name");
		query.bindValue(":name", user);
		if (!execSqlQuery(query))
			return false;
		return query.next();
	} else return false;
}

ServerInfo_User *Servatrice::evalUserQueryResult(const QSqlQuery &query, bool complete)
{
	QString name = query.value(0).toString();
	bool is_admin = query.value(1).toInt();
	QString realName = query.value(2).toString();
	QString country = query.value(3).toString();
	QByteArray avatarBmp;
	if (complete)
		avatarBmp = query.value(4).toByteArray();
	
	int userLevel = ServerInfo_User::IsUser | ServerInfo_User::IsRegistered;
	if (is_admin)
		userLevel |= ServerInfo_User::IsAdmin;
	
	return new ServerInfo_User(
		name,
		userLevel,
		realName,
		country,
		avatarBmp
	);
}

ServerInfo_User *Servatrice::getUserData(const QString &name)
{
	QMutexLocker locker(&dbMutex);
	const QString method = settings->value("authentication/method").toString();
	if (method == "sql") {
		checkSql();

		QSqlQuery query;
		query.prepare("select name, admin, realname, country, avatar_bmp from " + dbPrefix + "_users where name = :name and active = 1");
		query.bindValue(":name", name);
		if (!execSqlQuery(query))
			return new ServerInfo_User(name, ServerInfo_User::IsUser);
		
		if (query.next())
			return evalUserQueryResult(query, true);
		else
			return new ServerInfo_User(name, ServerInfo_User::IsUser);
	} else
		return new ServerInfo_User(name, ServerInfo_User::IsUser);
}

int Servatrice::getUsersWithAddress(const QHostAddress &address) const
{
	QMutexLocker locker(&serverMutex);
	int result = 0;
	for (int i = 0; i < clients.size(); ++i)
		if (static_cast<ServerSocketInterface *>(clients[i])->getPeerAddress() == address)
			++result;
	return result;
}

QMap<QString, ServerInfo_User *> Servatrice::getBuddyList(const QString &name)
{
	QMutexLocker locker(&dbMutex);
	QMap<QString, ServerInfo_User *> result;
	
	const QString method = settings->value("authentication/method").toString();
	if (method == "sql") {
		checkSql();

		QSqlQuery query;
		query.prepare("select a.name, a.admin, a.realname, a.country from " + dbPrefix + "_users a left join " + dbPrefix + "_buddylist b on a.id = b.id_user2 left join " + dbPrefix + "_users c on b.id_user1 = c.id where c.name = :name");
		query.bindValue(":name", name);
		if (!execSqlQuery(query))
			return result;
		
		while (query.next()) {
			ServerInfo_User *temp = evalUserQueryResult(query, false);
			result.insert(temp->getName(), temp);
		}
	}
	return result;
}

QMap<QString, ServerInfo_User *> Servatrice::getIgnoreList(const QString &name)
{
	QMutexLocker locker(&dbMutex);
	QMap<QString, ServerInfo_User *> result;
	
	const QString method = settings->value("authentication/method").toString();
	if (method == "sql") {
		checkSql();

		QSqlQuery query;
		query.prepare("select a.name, a.admin, a.realname, a.country from " + dbPrefix + "_users a left join " + dbPrefix + "_ignorelist b on a.id = b.id_user2 left join " + dbPrefix + "_users c on b.id_user1 = c.id where c.name = :name");
		query.bindValue(":name", name);
		if (!execSqlQuery(query))
			return result;
		
		while (query.next()) {
			ServerInfo_User *temp = evalUserQueryResult(query, false);
			result.insert(temp->getName(), temp);
		}
	}
	return result;
}

bool Servatrice::getUserBanned(Server_ProtocolHandler *client, const QString &userName) const
{
	QMutexLocker locker(&serverMutex);
	QHostAddress address = static_cast<ServerSocketInterface *>(client)->getPeerAddress();
	for (int i = 0; i < addressBanList.size(); ++i)
		if (address == addressBanList[i].first)
			return true;
	for (int i = 0; i < nameBanList.size(); ++i)
		if (userName == nameBanList[i].first)
			return true;
	return false;
}

void Servatrice::updateBanTimer()
{
	QMutexLocker locker(&serverMutex);
	for (int i = 0; i < addressBanList.size(); )
		if (--(addressBanList[i].second) <= 0)
			addressBanList.removeAt(i);
		else
			++i;
	for (int i = 0; i < nameBanList.size(); )
		if (--(nameBanList[i].second) <= 0)
			nameBanList.removeAt(i);
		else
			++i;
}

void Servatrice::updateLoginMessage()
{
	QMutexLocker locker(&dbMutex);
	checkSql();
	QSqlQuery query;
	query.prepare("select message from " + dbPrefix + "_servermessages where id_server = :id_server order by timest desc limit 1");
	query.bindValue(":id_server", serverId);
	if (execSqlQuery(query))
		if (query.next()) {
			loginMessage = query.value(0).toString();
			
			Event_ServerMessage *event = new Event_ServerMessage(loginMessage);
			QMapIterator<QString, Server_ProtocolHandler *> usersIterator(users);
			while (usersIterator.hasNext()) {
				usersIterator.next().value()->sendProtocolItem(event, false);
			}
			delete event;
		}
}

void Servatrice::statusUpdate()
{
	QMutexLocker locker(&dbMutex);
	uptime += statusUpdateClock->interval() / 1000;
	
	checkSql();
	
	QSqlQuery query;
	query.prepare("insert into " + dbPrefix + "_uptime (id_server, timest, uptime, users_count, games_count) values(:id, NOW(), :uptime, :users_count, :games_count)");
	query.bindValue(":id", serverId);
	query.bindValue(":uptime", uptime);
	query.bindValue(":users_count", getUsersCount());
	query.bindValue(":games_count", getGamesCount());
	execSqlQuery(query);
}

const QString Servatrice::versionString = "Servatrice 0.20110527";
