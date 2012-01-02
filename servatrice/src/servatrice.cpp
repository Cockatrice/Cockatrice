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
#include "server_logger.h"
#include "main.h"
#include "passwordhasher.h"
#include "pb/event_server_message.pb.h"
#include "pb/event_server_shutdown.pb.h"
#include "pb/event_connection_closed.pb.h"

void Servatrice_TcpServer::incomingConnection(int socketDescriptor)
{
	if (threaded) {
		ServerSocketThread *sst = new ServerSocketThread(socketDescriptor, server, this);
		sst->start();
	} else {
		QTcpSocket *socket = new QTcpSocket;
		socket->setSocketDescriptor(socketDescriptor);
		ServerSocketInterface *ssi = new ServerSocketInterface(server, socket);
		logger->logMessage(QString("incoming connection: %1").arg(socket->peerAddress().toString()), ssi);
	}
}

Servatrice::Servatrice(QSettings *_settings, QObject *parent)
	: Server(parent), dbMutex(QMutex::Recursive), settings(_settings), uptime(0), shutdownTimer(0)
{
	pingClock = new QTimer(this);
	connect(pingClock, SIGNAL(timeout()), this, SIGNAL(pingClockTimeout()));
	pingClock->start(1000);
	
	serverName = settings->value("server/name").toString();
	serverId = settings->value("server/id", 0).toInt();
	int statusUpdateTime = settings->value("server/statusupdate").toInt();
	statusUpdateClock = new QTimer(this);
	connect(statusUpdateClock, SIGNAL(timeout()), this, SLOT(statusUpdate()));
	if (statusUpdateTime != 0) {
		qDebug() << "Starting status update clock, interval " << statusUpdateTime << " ms";
		statusUpdateClock->start(statusUpdateTime);
	}
	
	threaded = settings->value("server/threaded", false).toInt();
	tcpServer = new Servatrice_TcpServer(this, threaded, this);
	int port = settings->value("server/port", 4747).toInt();
	qDebug() << "Starting server on port" << port;
	if (tcpServer->listen(QHostAddress::Any, port))
		qDebug() << "Server listening.";
	else
		qDebug() << "tcpServer->listen(): Error.";
	
	const QString authenticationMethodStr = settings->value("authentication/method").toString();
	if (authenticationMethodStr == "sql")
		authenticationMethod = AuthenticationSql;
	else
		authenticationMethod = AuthenticationNone;
	
	QString dbTypeStr = settings->value("database/type").toString();
	if (dbTypeStr == "mysql")
		databaseType = DatabaseMySql;
	else
		databaseType = DatabaseNone;
	dbPrefix = settings->value("database/prefix").toString();
	if (databaseType != DatabaseNone)
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
	QSqlDatabase::database().close();
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

bool Servatrice::checkSql()
{
	if (databaseType == DatabaseNone)
		return false;
	
	QMutexLocker locker(&dbMutex);
	if (!QSqlDatabase::database().exec("select 1").isActive())
		return openDatabase();
	return true;
}

bool Servatrice::execSqlQuery(QSqlQuery &query)
{
	if (query.exec())
		return true;
	qCritical() << "Database error:" << query.lastError().text();
	return false;
}

AuthenticationResult Servatrice::checkUserPassword(Server_ProtocolHandler *handler, const QString &user, const QString &password, QString &reasonStr)
{
	QMutexLocker locker(&dbMutex);
	const QString method = settings->value("authentication/method").toString();
	switch (authenticationMethod) {
	case AuthenticationNone: return UnknownUser;
	case AuthenticationSql: {
		if (!checkSql())
			return UnknownUser;
		
		QSqlQuery ipBanQuery;
		ipBanQuery.prepare("select time_to_sec(timediff(now(), date_add(b.time_from, interval b.minutes minute))) < 0, b.minutes <=> 0, b.visible_reason from " + dbPrefix + "_bans b where b.time_from = (select max(c.time_from) from " + dbPrefix + "_bans c where c.ip_address = :address) and b.ip_address = :address2");
		ipBanQuery.bindValue(":address", static_cast<ServerSocketInterface *>(handler)->getPeerAddress().toString());
		ipBanQuery.bindValue(":address2", static_cast<ServerSocketInterface *>(handler)->getPeerAddress().toString());
		if (!execSqlQuery(ipBanQuery)) {
			qDebug("Login denied: SQL error");
			return NotLoggedIn;
		}
		
		if (ipBanQuery.next())
			if (ipBanQuery.value(0).toInt() || ipBanQuery.value(1).toInt()) {
				reasonStr = ipBanQuery.value(2).toString();
				qDebug("Login denied: banned by address");
				return UserIsBanned;
			}
		
		QSqlQuery nameBanQuery;
		nameBanQuery.prepare("select time_to_sec(timediff(now(), date_add(b.time_from, interval b.minutes minute))) < 0, b.minutes <=> 0, b.visible_reason from " + dbPrefix + "_bans b where b.time_from = (select max(c.time_from) from " + dbPrefix + "_bans c where c.user_name = :name2) and b.user_name = :name1");
		nameBanQuery.bindValue(":name1", user);
		nameBanQuery.bindValue(":name2", user);
		if (!execSqlQuery(nameBanQuery)) {
			qDebug("Login denied: SQL error");
			return NotLoggedIn;
		}
		
		if (nameBanQuery.next())
			if (nameBanQuery.value(0).toInt() || nameBanQuery.value(1).toInt()) {
				reasonStr = nameBanQuery.value(2).toString();
				qDebug("Login denied: banned by name");
				return UserIsBanned;
			}
		
		QSqlQuery passwordQuery;
		passwordQuery.prepare("select password_sha512 from " + dbPrefix + "_users where name = :name and active = 1");
		passwordQuery.bindValue(":name", user);
		if (!execSqlQuery(passwordQuery)) {
			qDebug("Login denied: SQL error");
			return NotLoggedIn;
		}
		
		if (passwordQuery.next()) {
			const QString correctPassword = passwordQuery.value(0).toString();
			if (correctPassword == PasswordHasher::computeHash(password, correctPassword.left(16))) {
				qDebug("Login accepted: password right");
				return PasswordRight;
			} else {
				qDebug("Login denied: password wrong");
				return NotLoggedIn;
			}
		} else {
			qDebug("Login accepted: unknown user");
			return UnknownUser;
		}
	}
	}
	return UnknownUser;
}

bool Servatrice::userExists(const QString &user)
{
	if (authenticationMethod == AuthenticationSql) {
		QMutexLocker locker(&dbMutex);
		checkSql();
	
		QSqlQuery query;
		query.prepare("select 1 from " + dbPrefix + "_users where name = :name and active = 1");
		query.bindValue(":name", user);
		if (!execSqlQuery(query))
			return false;
		return query.next();
	}
	return false;
}

int Servatrice::getUserIdInDB(const QString &name)
{
	if (authenticationMethod == AuthenticationSql) {
		QMutexLocker locker(&dbMutex);
		QSqlQuery query;
		query.prepare("select id from " + dbPrefix + "_users where name = :name and active = 1");
		query.bindValue(":name", name);
		if (!execSqlQuery(query))
			return -1;
		if (!query.next())
			return -1;
		return query.value(0).toInt();
	}
	return -1;
}

bool Servatrice::isInBuddyList(const QString &whoseList, const QString &who)
{
	if (authenticationMethod == AuthenticationNone)
		return false;
	
	QMutexLocker locker(&dbMutex);
	if (!checkSql())
		return false;
	
	int id1 = getUserIdInDB(whoseList);
	int id2 = getUserIdInDB(who);
	
	QSqlQuery query;
	query.prepare("select 1 from " + dbPrefix + "_buddylist where id_user1 = :id_user1 and id_user2 = :id_user2");
	query.bindValue(":id_user1", id1);
	query.bindValue(":id_user2", id2);
	if (!execSqlQuery(query))
		return false;
	return query.next();
}

bool Servatrice::isInIgnoreList(const QString &whoseList, const QString &who)
{
	if (authenticationMethod == AuthenticationNone)
		return false;
	
	QMutexLocker locker(&dbMutex);
	if (!checkSql())
		return false;
	
	int id1 = getUserIdInDB(whoseList);
	int id2 = getUserIdInDB(who);
	
	QSqlQuery query;
	query.prepare("select 1 from " + dbPrefix + "_ignorelist where id_user1 = :id_user1 and id_user2 = :id_user2");
	query.bindValue(":id_user1", id1);
	query.bindValue(":id_user2", id2);
	if (!execSqlQuery(query))
		return false;
	return query.next();
}

ServerInfo_User Servatrice::evalUserQueryResult(const QSqlQuery &query, bool complete)
{
	ServerInfo_User result;
	
	result.set_name(query.value(0).toString().toStdString());
	
	const QString country = query.value(4).toString();
	if (!country.isEmpty())
		result.set_country(country.toStdString());
	
	if (complete) {
		const QByteArray avatarBmp = query.value(5).toByteArray();
		if (avatarBmp.size())
			result.set_avatar_bmp(avatarBmp.data(), avatarBmp.size());
	}
	
	const QString genderStr = query.value(3).toString();
	if (genderStr == "m")
		result.set_gender(ServerInfo_User::Male);
	else if (genderStr == "f")
		result.set_gender(ServerInfo_User::Female);
	
	const int is_admin = query.value(1).toInt();
	int userLevel = ServerInfo_User::IsUser | ServerInfo_User::IsRegistered;
	if (is_admin == 1)
		userLevel |= ServerInfo_User::IsAdmin | ServerInfo_User::IsModerator;
	else if (is_admin == 2)
		userLevel |= ServerInfo_User::IsModerator;
	result.set_user_level(userLevel);
	
	const QString realName = query.value(2).toString();
	if (!realName.isEmpty())
		result.set_real_name(realName.toStdString());
	
	return result;
}

ServerInfo_User Servatrice::getUserData(const QString &name)
{
	ServerInfo_User result;
	result.set_name(name.toStdString());
	result.set_user_level(ServerInfo_User::IsUser);
	
	if (authenticationMethod == AuthenticationSql) {
		QMutexLocker locker(&dbMutex);
		if (!checkSql())
			return result;
		
		QSqlQuery query;
		query.prepare("select name, admin, realname, gender, country, avatar_bmp from " + dbPrefix + "_users where name = :name and active = 1");
		query.bindValue(":name", name);
		if (!execSqlQuery(query))
			return result;
		
		if (query.next())
			return evalUserQueryResult(query, true);
		else
			return result;
	} else
		return result;
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

int Servatrice::startSession(const QString &userName, const QString &address)
{
	if (authenticationMethod == AuthenticationNone)
		return -1;
	
	QMutexLocker locker(&dbMutex);
	if (!checkSql())
		return -1;
	
	QSqlQuery query;
	query.prepare("insert into " + dbPrefix + "_sessions (user_name, ip_address, start_time) values(:user_name, :ip_address, NOW())");
	query.bindValue(":user_name", userName);
	query.bindValue(":ip_address", address);
	if (execSqlQuery(query))
		return query.lastInsertId().toInt();
	return -1;
}

void Servatrice::endSession(int sessionId)
{
	if (authenticationMethod == AuthenticationNone)
		return;
	
	QMutexLocker locker(&dbMutex);
	if (!checkSql())
		return;
	
	QSqlQuery query;
	query.prepare("update " + dbPrefix + "_sessions set end_time=NOW() where id = :id_session");
	query.bindValue(":id_session", sessionId);
	execSqlQuery(query);
}

QMap<QString, ServerInfo_User> Servatrice::getBuddyList(const QString &name)
{
	QMap<QString, ServerInfo_User> result;
	
	if (authenticationMethod == AuthenticationSql) {
		QMutexLocker locker(&dbMutex);
		checkSql();

		QSqlQuery query;
		query.prepare("select a.name, a.admin, a.realname, a.gender, a.country from " + dbPrefix + "_users a left join " + dbPrefix + "_buddylist b on a.id = b.id_user2 left join " + dbPrefix + "_users c on b.id_user1 = c.id where c.name = :name");
		query.bindValue(":name", name);
		if (!execSqlQuery(query))
			return result;
		
		while (query.next()) {
			const ServerInfo_User &temp = evalUserQueryResult(query, false);
			result.insert(QString::fromStdString(temp.name()), temp);
		}
	}
	return result;
}

QMap<QString, ServerInfo_User> Servatrice::getIgnoreList(const QString &name)
{
	QMap<QString, ServerInfo_User> result;
	
	if (authenticationMethod == AuthenticationSql) {
		QMutexLocker locker(&dbMutex);
		checkSql();

		QSqlQuery query;
		query.prepare("select a.name, a.admin, a.realname, a.gender, a.country from " + dbPrefix + "_users a left join " + dbPrefix + "_ignorelist b on a.id = b.id_user2 left join " + dbPrefix + "_users c on b.id_user1 = c.id where c.name = :name");
		query.bindValue(":name", name);
		if (!execSqlQuery(query))
			return result;
		
		while (query.next()) {
			ServerInfo_User temp = evalUserQueryResult(query, false);
			result.insert(QString::fromStdString(temp.name()), temp);
		}
	}
	return result;
}

void Servatrice::updateLoginMessage()
{
	QMutexLocker locker(&dbMutex);
	if (!checkSql())
		return;
	
	QSqlQuery query;
	query.prepare("select message from " + dbPrefix + "_servermessages where id_server = :id_server order by timest desc limit 1");
	query.bindValue(":id_server", serverId);
	if (execSqlQuery(query))
		if (query.next()) {
			loginMessage = query.value(0).toString();
			
			Event_ServerMessage event;
			event.set_message(loginMessage.toStdString());
			SessionEvent *se = Server_ProtocolHandler::prepareSessionEvent(event);
			QMapIterator<QString, Server_ProtocolHandler *> usersIterator(users);
			while (usersIterator.hasNext())
				usersIterator.next().value()->sendProtocolItem(*se);
			delete se;
		}
}

void Servatrice::statusUpdate()
{
	const int uc = getUsersCount(); // for correct mutex locking order
	const int gc = getGamesCount();
	
	uptime += statusUpdateClock->interval() / 1000;
	
	txBytesMutex.lock();
	quint64 tx = txBytes;
	txBytes = 0;
	txBytesMutex.unlock();
	rxBytesMutex.lock();
	quint64 rx = rxBytes;
	rxBytes = 0;
	rxBytesMutex.unlock();
	
	QMutexLocker locker(&dbMutex);
	if (!checkSql())
		return;
	
	QSqlQuery query;
	query.prepare("insert into " + dbPrefix + "_uptime (id_server, timest, uptime, users_count, games_count, tx_bytes, rx_bytes) values(:id, NOW(), :uptime, :users_count, :games_count, :tx, :rx)");
	query.bindValue(":id", serverId);
	query.bindValue(":uptime", uptime);
	query.bindValue(":users_count", uc);
	query.bindValue(":games_count", gc);
	query.bindValue(":tx", tx);
	query.bindValue(":rx", rx);
	execSqlQuery(query);
}

void Servatrice::scheduleShutdown(const QString &reason, int minutes)
{
	QMutexLocker locker(&serverMutex);

	shutdownReason = reason;
	shutdownMinutes = minutes + 1;
	if (minutes > 0) {
		shutdownTimer = new QTimer;
		connect(shutdownTimer, SIGNAL(timeout()), this, SLOT(shutdownTimeout()));
		shutdownTimer->start(60000);
	}
	shutdownTimeout();
}

void Servatrice::incTxBytes(quint64 num)
{
	txBytesMutex.lock();
	txBytes += num;
	txBytesMutex.unlock();
}

void Servatrice::incRxBytes(quint64 num)
{
	rxBytesMutex.lock();
	rxBytes += num;
	rxBytesMutex.unlock();
}

void Servatrice::shutdownTimeout()
{
	QMutexLocker locker(&serverMutex);
	
	--shutdownMinutes;
	
	SessionEvent *se;
	if (shutdownMinutes) {
		Event_ServerShutdown event;
		event.set_reason(shutdownReason.toStdString());
		event.set_minutes(shutdownMinutes);
		se = Server_ProtocolHandler::prepareSessionEvent(event);
	} else {
		Event_ConnectionClosed event;
		event.set_reason(Event_ConnectionClosed::SERVER_SHUTDOWN);
		se = Server_ProtocolHandler::prepareSessionEvent(event);
	}

	for (int i = 0; i < clients.size(); ++i)
		clients[i]->sendProtocolItem(*se);
	delete se;
	
	if (!shutdownMinutes)
		deleteLater();
}

const QString Servatrice::versionString = "Servatrice 0.20111113";
