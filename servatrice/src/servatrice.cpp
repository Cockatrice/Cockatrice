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
#include "protocol.h"

Servatrice::Servatrice(QObject *parent)
	: Server(parent), uptime(0)
{
	pingClock = new QTimer(this);
	connect(pingClock, SIGNAL(timeout()), this, SIGNAL(pingClockTimeout()));
	pingClock->start(1000);
	
	ProtocolItem::initializeHash();
	settings = new QSettings("servatrice.ini", QSettings::IniFormat, this);
	
	int statusUpdateTime = settings->value("server/statusupdate").toInt();
	statusUpdateClock = new QTimer(this);
	connect(statusUpdateClock, SIGNAL(timeout()), this, SLOT(statusUpdate()));
	if (statusUpdateTime != 0) {
		qDebug() << "Starting status update clock, interval " << statusUpdateTime << " ms";
		statusUpdateClock->start(statusUpdateTime);
	}
	
	tcpServer = new QTcpServer(this);
	connect(tcpServer, SIGNAL(newConnection()), this, SLOT(newConnection()));
	int port = settings->value("server/port", 4747).toInt();
	qDebug() << "Starting server on port" << port;
	tcpServer->listen(QHostAddress::Any, port);
	
	QString dbType = settings->value("database/type").toString();
	dbPrefix = settings->value("database/prefix").toString();
	if (dbType == "mysql")
		openDatabase();
	
	int size = settings->beginReadArray("rooms");
	for (int i = 0; i < size; ++i) {
	  	settings->setArrayIndex(i);
		Server_Room *newRoom = new Server_Room(
			i,
			settings->value("name").toString(),
			settings->value("description").toString(),
			settings->value("autojoin").toBool(),
			settings->value("joinmessage").toString(),
			this
		);
		addRoom(newRoom);
	}
	settings->endArray();
	
	updateLoginMessage();
	
	maxGameInactivityTime = settings->value("game/max_game_inactivity_time").toInt();
	maxPlayerInactivityTime = settings->value("game/max_player_inactivity_time").toInt();
}

Servatrice::~Servatrice()
{
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

void Servatrice::newConnection()
{
	QTcpSocket *socket = tcpServer->nextPendingConnection();
	ServerSocketInterface *ssi = new ServerSocketInterface(this, socket);
	addClient(ssi);
}

AuthenticationResult Servatrice::checkUserPassword(const QString &user, const QString &password)
{
	const QString method = settings->value("authentication/method").toString();
	if (method == "none")
		return UnknownUser;
	else if (method == "sql") {
		checkSql();
	
		QSqlQuery query;
		query.prepare("select password from " + dbPrefix + "_users where name = :name and active = 1");
		query.bindValue(":name", user);
		if (!execSqlQuery(query))
			return PasswordWrong;
		
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

ServerInfo_User *Servatrice::getUserData(const QString &name)
{
	const QString method = settings->value("authentication/method").toString();
	if (method == "sql") {
		checkSql();

		QSqlQuery query;
		query.prepare("select admin, realname, country, avatar_bmp from " + dbPrefix + "_users where name = :name and active = 1");
		query.bindValue(":name", name);
		if (!execSqlQuery(query))
			return new ServerInfo_User(name, ServerInfo_User::IsUser);
		
		if (query.next()) {
			bool is_admin = query.value(0).toInt();
			QString realName = query.value(1).toString();
			QString country = query.value(2).toString();
			QByteArray avatarBmp = query.value(3).toByteArray();
			
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
		} else
			return new ServerInfo_User(name, ServerInfo_User::IsUser);
	} else
		return new ServerInfo_User(name, ServerInfo_User::IsUser);
}

void Servatrice::updateLoginMessage()
{
	checkSql();
	QSqlQuery query;
	query.prepare("select message from " + dbPrefix + "_servermessages order by timest desc limit 1");
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
	uptime += statusUpdateClock->interval() / 1000;
	
	checkSql();
	
	QSqlQuery query;
	query.prepare("insert into " + dbPrefix + "_uptime (timest, uptime, users_count, games_count) values(NOW(), :uptime, :users_count, :games_count)");
	query.bindValue(":uptime", uptime);
	query.bindValue(":users_count", users.size());
	query.bindValue(":games_count", games.size());
	execSqlQuery(query);
}

const QString Servatrice::versionString = "Servatrice 0.20110124";
