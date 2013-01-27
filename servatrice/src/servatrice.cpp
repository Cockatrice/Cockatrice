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
#include <QSqlQuery>
#include <QSettings>
#include <QFile>
#include <QTimer>
#include <QDateTime>
#include <QDebug>
#include <iostream>
#include "servatrice.h"
#include "servatrice_database_interface.h"
#include "servatrice_connection_pool.h"
#include "server_room.h"
#include "serversocketinterface.h"
#include "isl_interface.h"
#include "server_logger.h"
#include "main.h"
#include "decklist.h"
#include "pb/event_server_message.pb.h"
#include "pb/event_server_shutdown.pb.h"
#include "pb/event_connection_closed.pb.h"

Servatrice_GameServer::Servatrice_GameServer(Servatrice *_server, int _numberPools, const QSqlDatabase &_sqlDatabase, QObject *parent)
	: QTcpServer(parent),
	  server(_server)
{
	if (_numberPools == 0) {
		server->setThreaded(false);
		Servatrice_DatabaseInterface *newDatabaseInterface = new Servatrice_DatabaseInterface(0, server);
		Servatrice_ConnectionPool *newPool = new Servatrice_ConnectionPool(newDatabaseInterface);
		
		server->addDatabaseInterface(thread(), newDatabaseInterface);
		newDatabaseInterface->initDatabase(_sqlDatabase);
		
		connectionPools.append(newPool);
	} else
	for (int i = 0; i < _numberPools; ++i) {
		Servatrice_DatabaseInterface *newDatabaseInterface = new Servatrice_DatabaseInterface(i, server);
		Servatrice_ConnectionPool *newPool = new Servatrice_ConnectionPool(newDatabaseInterface);
		
		QThread *newThread = new QThread;
		newThread->setObjectName("pool_" + QString::number(i));
		newPool->moveToThread(newThread);
		newDatabaseInterface->moveToThread(newThread);
		server->addDatabaseInterface(newThread, newDatabaseInterface);
		
		newThread->start();
		QMetaObject::invokeMethod(newDatabaseInterface, "initDatabase", Qt::BlockingQueuedConnection, Q_ARG(QSqlDatabase, _sqlDatabase));
		
		connectionPools.append(newPool);
	}
}

Servatrice_GameServer::~Servatrice_GameServer()
{
	for (int i = 0; i < connectionPools.size(); ++i) {
		logger->logMessage(QString("Closing pool %1...").arg(i));
		QThread *poolThread = connectionPools[i]->thread();
		connectionPools[i]->deleteLater(); // pool destructor calls thread()->quit()
		poolThread->wait();
	}
}

void Servatrice_GameServer::incomingConnection(int socketDescriptor)
{
	// Determine connection pool with smallest client count
	int minClientCount = -1;
	int poolIndex = -1;
	QStringList debugStr;
	for (int i = 0; i < connectionPools.size(); ++i) {
		const int clientCount = connectionPools[i]->getClientCount();
		if ((poolIndex == -1) || (clientCount < minClientCount)) {
			minClientCount = clientCount;
			poolIndex = i;
		}
		debugStr.append(QString::number(clientCount));
	}
	qDebug() << "Pool utilisation:" << debugStr;
	Servatrice_ConnectionPool *pool = connectionPools[poolIndex];
	
	ServerSocketInterface *ssi = new ServerSocketInterface(server, pool->getDatabaseInterface());
	ssi->moveToThread(pool->thread());
	pool->addClient();
	connect(ssi, SIGNAL(destroyed()), pool, SLOT(removeClient()));
	
	QMetaObject::invokeMethod(ssi, "initConnection", Qt::QueuedConnection, Q_ARG(int, socketDescriptor));
}

void Servatrice_IslServer::incomingConnection(int socketDescriptor)
{
	QThread *thread = new QThread;
	connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
	
	IslInterface *interface = new IslInterface(socketDescriptor, cert, privateKey, server);
	interface->moveToThread(thread);
	connect(interface, SIGNAL(destroyed()), thread, SLOT(quit()));
	
	thread->start();
	QMetaObject::invokeMethod(interface, "initServer", Qt::QueuedConnection);
}

Servatrice::Servatrice(QSettings *_settings, QObject *parent)
	: Server(true, parent), settings(_settings), uptime(0), shutdownTimer(0)
{
	qRegisterMetaType<QSqlDatabase>("QSqlDatabase");
}

Servatrice::~Servatrice()
{
	gameServer->close();
	prepareDestroy();
}

bool Servatrice::initServer()
{
	serverName = settings->value("server/name").toString();
	serverId = settings->value("server/id", 0).toInt();
	
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
	
	servatriceDatabaseInterface = new Servatrice_DatabaseInterface(-1, this);
	setDatabaseInterface(servatriceDatabaseInterface);
	
	if (databaseType != DatabaseNone) {
		settings->beginGroup("database");
		dbPrefix = settings->value("prefix").toString();
		servatriceDatabaseInterface->initDatabase("QMYSQL",
							  settings->value("hostname").toString(),
							  settings->value("database").toString(),
							  settings->value("user").toString(),
							  settings->value("password").toString());
		settings->endGroup();
		
		updateServerList();
		
		qDebug() << "Clearing previous sessions...";
		servatriceDatabaseInterface->clearSessionTables();
	}
	
	const QString roomMethod = settings->value("rooms/method").toString();
	if (roomMethod == "sql") {
		QSqlQuery query(servatriceDatabaseInterface->getDatabase());
		query.prepare("select id, name, descr, auto_join, join_message from " + dbPrefix + "_rooms order by id asc");
		servatriceDatabaseInterface->execSqlQuery(query);
		while (query.next()) {
			QSqlQuery query2(servatriceDatabaseInterface->getDatabase());
			query2.prepare("select name from " + dbPrefix + "_rooms_gametypes where id_room = :id_room");
			query2.bindValue(":id_room", query.value(0).toInt());
			servatriceDatabaseInterface->execSqlQuery(query2);
			QStringList gameTypes;
			while (query2.next())
				gameTypes.append(query2.value(0).toString());
			
			addRoom(new Server_Room(query.value(0).toInt(),
			                        query.value(1).toString(),
			                        query.value(2).toString(),
			                        query.value(3).toInt(),
			                        query.value(4).toString(),
			                        gameTypes,
			                        this
			));
		}
	} else {
		int size = settings->beginReadArray("rooms/roomlist");
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
	}
	
	updateLoginMessage();
	
	maxGameInactivityTime = settings->value("game/max_game_inactivity_time").toInt();
	maxPlayerInactivityTime = settings->value("game/max_player_inactivity_time").toInt();
	
	maxUsersPerAddress = settings->value("security/max_users_per_address").toInt();
	messageCountingInterval = settings->value("security/message_counting_interval").toInt();
	maxMessageCountPerInterval = settings->value("security/max_message_count_per_interval").toInt();
	maxMessageSizePerInterval = settings->value("security/max_message_size_per_interval").toInt();
	maxGamesPerUser = settings->value("security/max_games_per_user").toInt();

	try { if (settings->value("servernetwork/active", 0).toInt()) {
		qDebug() << "Connecting to ISL network.";
		const QString certFileName = settings->value("servernetwork/ssl_cert").toString();
		const QString keyFileName = settings->value("servernetwork/ssl_key").toString();
		qDebug() << "Loading certificate...";
		QFile certFile(certFileName);
		if (!certFile.open(QIODevice::ReadOnly))
			throw QString("Error opening certificate file: %1").arg(certFileName);
		QSslCertificate cert(&certFile);
		if (!cert.isValid())
			throw(QString("Invalid certificate."));
		qDebug() << "Loading private key...";
		QFile keyFile(keyFileName);
		if (!keyFile.open(QIODevice::ReadOnly))
			throw QString("Error opening private key file: %1").arg(keyFileName);
		QSslKey key(&keyFile, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey);
		if (key.isNull())
			throw QString("Invalid private key.");
		
		QMutableListIterator<ServerProperties> serverIterator(serverList);
		while (serverIterator.hasNext()) {
			const ServerProperties &prop = serverIterator.next();
			if (prop.cert == cert) {
				serverIterator.remove();
				continue;
			}
			
			QThread *thread = new QThread;
			thread->setObjectName("isl_" + QString::number(prop.id));
			connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
			
			IslInterface *interface = new IslInterface(prop.id, prop.hostname, prop.address.toString(), prop.controlPort, prop.cert, cert, key, this);
			interface->moveToThread(thread);
			connect(interface, SIGNAL(destroyed()), thread, SLOT(quit()));
			
			thread->start();
			QMetaObject::invokeMethod(interface, "initClient", Qt::BlockingQueuedConnection);
		}
			
		const int networkPort = settings->value("servernetwork/port", 14747).toInt();
		qDebug() << "Starting ISL server on port" << networkPort;
		
		islServer = new Servatrice_IslServer(this, cert, key, this);
		if (islServer->listen(QHostAddress::Any, networkPort))
			qDebug() << "ISL server listening.";
		else
			throw QString("islServer->listen()");
	} } catch (QString error) {
		qDebug() << "ERROR --" << error;
		return false;
	}
	
	pingClock = new QTimer(this);
	connect(pingClock, SIGNAL(timeout()), this, SIGNAL(pingClockTimeout()));
	pingClock->start(1000);
	
	int statusUpdateTime = settings->value("server/statusupdate").toInt();
	statusUpdateClock = new QTimer(this);
	connect(statusUpdateClock, SIGNAL(timeout()), this, SLOT(statusUpdate()));
	if (statusUpdateTime != 0) {
		qDebug() << "Starting status update clock, interval " << statusUpdateTime << " ms";
		statusUpdateClock->start(statusUpdateTime);
	}
	
	const int numberPools = settings->value("server/number_pools", 1).toInt();
	gameServer = new Servatrice_GameServer(this, numberPools, servatriceDatabaseInterface->getDatabase(), this);
	gameServer->setMaxPendingConnections(1000);
	const int gamePort = settings->value("server/port", 4747).toInt();
	qDebug() << "Starting server on port" << gamePort;
	if (gameServer->listen(QHostAddress::Any, gamePort))
		qDebug() << "Server listening.";
	else {
		qDebug() << "gameServer->listen(): Error.";
		return false;
	}
	return true;
}

void Servatrice::addDatabaseInterface(QThread *thread, Servatrice_DatabaseInterface *databaseInterface)
{
	databaseInterfaces.insert(thread, databaseInterface);
}

void Servatrice::updateServerList()
{
	qDebug() << "Updating server list...";
	
	serverListMutex.lock();
	serverList.clear();
	
	QSqlQuery query(servatriceDatabaseInterface->getDatabase());
	query.prepare("select id, ssl_cert, hostname, address, game_port, control_port from " + dbPrefix + "_servers order by id asc");
	servatriceDatabaseInterface->execSqlQuery(query);
	while (query.next()) {
		ServerProperties prop(query.value(0).toInt(), QSslCertificate(query.value(1).toString().toAscii()), query.value(2).toString(), QHostAddress(query.value(3).toString()), query.value(4).toInt(), query.value(5).toInt());
		serverList.append(prop);
		qDebug() << QString("#%1 CERT=%2 NAME=%3 IP=%4:%5 CPORT=%6").arg(prop.id).arg(QString(prop.cert.digest().toHex())).arg(prop.hostname).arg(prop.address.toString()).arg(prop.gamePort).arg(prop.controlPort);
	}
	
	serverListMutex.unlock();
}

QList<ServerProperties> Servatrice::getServerList() const
{
	serverListMutex.lock();
	QList<ServerProperties> result = serverList;
	serverListMutex.unlock();
	
	return result;
}

int Servatrice::getUsersWithAddress(const QHostAddress &address) const
{
	int result = 0;
	QReadLocker locker(&clientsLock);
	for (int i = 0; i < clients.size(); ++i)
		if (static_cast<ServerSocketInterface *>(clients[i])->getPeerAddress() == address)
			++result;
	return result;
}

QList<ServerSocketInterface *> Servatrice::getUsersWithAddressAsList(const QHostAddress &address) const
{
	QList<ServerSocketInterface *> result;
	QReadLocker locker(&clientsLock);
	for (int i = 0; i < clients.size(); ++i)
		if (static_cast<ServerSocketInterface *>(clients[i])->getPeerAddress() == address)
			result.append(static_cast<ServerSocketInterface *>(clients[i]));
	return result;
}

void Servatrice::updateLoginMessage()
{
	if (!servatriceDatabaseInterface->checkSql())
		return;
	
	QSqlQuery query(servatriceDatabaseInterface->getDatabase());
	query.prepare("select message from " + dbPrefix + "_servermessages where id_server = :id_server order by timest desc limit 1");
	query.bindValue(":id_server", serverId);
	if (servatriceDatabaseInterface->execSqlQuery(query))
		if (query.next()) {
			const QString newLoginMessage = query.value(0).toString();
			
			loginMessageMutex.lock();
			loginMessage = newLoginMessage;
			loginMessageMutex.unlock();
			
			Event_ServerMessage event;
			event.set_message(newLoginMessage.toStdString());
			SessionEvent *se = Server_ProtocolHandler::prepareSessionEvent(event);
			QMapIterator<QString, Server_ProtocolHandler *> usersIterator(users);
			while (usersIterator.hasNext())
				usersIterator.next().value()->sendProtocolItem(*se);
			delete se;
		}
}

void Servatrice::statusUpdate()
{
	if (!servatriceDatabaseInterface->checkSql())
		return;
	
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
	
	QSqlQuery query(servatriceDatabaseInterface->getDatabase());
	query.prepare("insert into " + dbPrefix + "_uptime (id_server, timest, uptime, users_count, games_count, tx_bytes, rx_bytes) values(:id, NOW(), :uptime, :users_count, :games_count, :tx, :rx)");
	query.bindValue(":id", serverId);
	query.bindValue(":uptime", uptime);
	query.bindValue(":users_count", uc);
	query.bindValue(":games_count", gc);
	query.bindValue(":tx", tx);
	query.bindValue(":rx", rx);
	servatriceDatabaseInterface->execSqlQuery(query);
}

void Servatrice::scheduleShutdown(const QString &reason, int minutes)
{
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
	
	clientsLock.lockForRead();
	for (int i = 0; i < clients.size(); ++i)
		clients[i]->sendProtocolItem(*se);
	clientsLock.unlock();
	delete se;
	
	if (!shutdownMinutes)
		deleteLater();
}

bool Servatrice::islConnectionExists(int serverId) const
{
	// Only call with islLock locked at least for reading
	
	return islInterfaces.contains(serverId);
}

void Servatrice::addIslInterface(int serverId, IslInterface *interface)
{
	// Only call with islLock locked for writing
	
	islInterfaces.insert(serverId, interface);
	connect(interface, SIGNAL(externalUserJoined(ServerInfo_User)), this, SLOT(externalUserJoined(ServerInfo_User)));
	connect(interface, SIGNAL(externalUserLeft(QString)), this, SLOT(externalUserLeft(QString)));
	connect(interface, SIGNAL(externalRoomUserJoined(int, ServerInfo_User)), this, SLOT(externalRoomUserJoined(int, ServerInfo_User)));
	connect(interface, SIGNAL(externalRoomUserLeft(int, QString)), this, SLOT(externalRoomUserLeft(int, QString)));
	connect(interface, SIGNAL(externalRoomSay(int, QString, QString)), this, SLOT(externalRoomSay(int, QString, QString)));
	connect(interface, SIGNAL(externalRoomGameListChanged(int, ServerInfo_Game)), this, SLOT(externalRoomGameListChanged(int, ServerInfo_Game)));
	connect(interface, SIGNAL(joinGameCommandReceived(Command_JoinGame, int, int, int, qint64)), this, SLOT(externalJoinGameCommandReceived(Command_JoinGame, int, int, int, qint64)));
	connect(interface, SIGNAL(gameCommandContainerReceived(CommandContainer, int, int, qint64)), this, SLOT(externalGameCommandContainerReceived(CommandContainer, int, int, qint64)));
	connect(interface, SIGNAL(responseReceived(Response, qint64)), this, SLOT(externalResponseReceived(Response, qint64)));
	connect(interface, SIGNAL(gameEventContainerReceived(GameEventContainer, qint64)), this, SLOT(externalGameEventContainerReceived(GameEventContainer, qint64)));
}

void Servatrice::removeIslInterface(int serverId)
{
	// Only call with islLock locked for writing
	
	// XXX we probably need to delete everything that belonged to it...
	islInterfaces.remove(serverId);
}

void Servatrice::doSendIslMessage(const IslMessage &msg, int serverId)
{
	QReadLocker locker(&islLock);
	
	if (serverId == -1) {
		QMapIterator<int, IslInterface *> islIterator(islInterfaces);
		while (islIterator.hasNext())
			islIterator.next().value()->transmitMessage(msg);
	} else {
		IslInterface *interface = islInterfaces.value(serverId);
		if (interface)
			interface->transmitMessage(msg);
	}
}
