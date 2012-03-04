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
#ifndef SERVATRICE_H
#define SERVATRICE_H

#include <QTcpServer>
#include <QMutex>
#include <QSslCertificate>
#include <QSslKey>
#include <QHostAddress>
#include "server.h"

class QSqlDatabase;
class QSettings;
class QSqlQuery;
class QTimer;

class GameReplay;
class Servatrice;
class ServerSocketInterface;
class NetworkServerInterface;

class Servatrice_GameServer : public QTcpServer {
	Q_OBJECT
private:
	Servatrice *server;
	bool threaded;
public:
	Servatrice_GameServer(Servatrice *_server, bool _threaded, QObject *parent = 0)
		: QTcpServer(parent), server(_server), threaded(_threaded) { }
protected:
	void incomingConnection(int socketDescriptor);
};

class Servatrice_NetworkServer : public QTcpServer {
	Q_OBJECT
private:
	Servatrice *server;
	QSslCertificate cert;
	QSslKey privateKey;
public:
	Servatrice_NetworkServer(Servatrice *_server, const QSslCertificate &_cert, const QSslKey &_privateKey, QObject *parent = 0)
		: QTcpServer(parent), server(_server), cert(_cert), privateKey(_privateKey) { }
protected:
	void incomingConnection(int socketDescriptor);
};

class ServerProperties {
public:
	int id;
	QSslCertificate cert;
	QString hostname;
	QHostAddress address;
	int gamePort;
	int controlPort;
	
	ServerProperties(int _id, const QSslCertificate &_cert, const QString &_hostname, const QHostAddress &_address, int _gamePort, int _controlPort)
		: id(_id), cert(_cert), hostname(_hostname), address(_address), gamePort(_gamePort), controlPort(_controlPort) { }
};

class Servatrice : public Server
{
	Q_OBJECT
private slots:
	void statusUpdate();
	void shutdownTimeout();
public:
	mutable QMutex dbMutex;
	Servatrice(QSettings *_settings, QObject *parent = 0);
	~Servatrice();
	bool openDatabase();
	bool checkSql();
	bool execSqlQuery(QSqlQuery &query);
	QString getServerName() const { return serverName; }
	QString getLoginMessage() const { return loginMessage; }
	bool getGameShouldPing() const { return true; }
	int getMaxGameInactivityTime() const { return maxGameInactivityTime; }
	int getMaxPlayerInactivityTime() const { return maxPlayerInactivityTime; }
	int getMaxUsersPerAddress() const { return maxUsersPerAddress; }
	int getMessageCountingInterval() const { return messageCountingInterval; }
	int getMaxMessageCountPerInterval() const { return maxMessageCountPerInterval; }
	int getMaxMessageSizePerInterval() const { return maxMessageSizePerInterval; }
	int getMaxGamesPerUser() const { return maxGamesPerUser; }
	bool getThreaded() const { return threaded; }
	QString getDbPrefix() const { return dbPrefix; }
	int getServerId() const { return serverId; }
	void updateLoginMessage();
	ServerInfo_User getUserData(const QString &name, bool withId = false);
	int getUsersWithAddress(const QHostAddress &address) const;
	QList<ServerSocketInterface *> getUsersWithAddressAsList(const QHostAddress &address) const;
	QMap<QString, ServerInfo_User> getBuddyList(const QString &name);
	QMap<QString, ServerInfo_User> getIgnoreList(const QString &name);
	bool isInBuddyList(const QString &whoseList, const QString &who);
	bool isInIgnoreList(const QString &whoseList, const QString &who);
	void scheduleShutdown(const QString &reason, int minutes);
	void incTxBytes(quint64 num);
	void incRxBytes(quint64 num);
	int getUserIdInDB(const QString &name);
	void storeGameInformation(int secondsElapsed, const QSet<QString> &allPlayersEver, const QSet<QString> &allSpectatorsEver, const QList<GameReplay *> &replays);
	
	void addNetworkServerInterface(NetworkServerInterface *interface);
	void removeNetworkServerInterface(NetworkServerInterface *interface);

	QList<ServerProperties> getServerList() const;
protected:
	int startSession(const QString &userName, const QString &address);
	void endSession(int sessionId);
	bool userExists(const QString &user);
	AuthenticationResult checkUserPassword(Server_ProtocolHandler *handler, const QString &user, const QString &password, QString &reasonStr);
	
	void clearSessionTables();
	void lockSessionTables();
	void unlockSessionTables();
	bool userSessionExists(const QString &userName);
private:
	enum AuthenticationMethod { AuthenticationNone, AuthenticationSql };
	enum DatabaseType { DatabaseNone, DatabaseMySql };
	AuthenticationMethod authenticationMethod;
	DatabaseType databaseType;
	QTimer *pingClock, *statusUpdateClock;
	Servatrice_GameServer *gameServer;
	Servatrice_NetworkServer *networkServer;
	QString serverName;
	QString loginMessage;
	QString dbPrefix;
	QSettings *settings;
	int serverId;
	bool threaded;
	int uptime;
	QMutex txBytesMutex, rxBytesMutex;
	quint64 txBytes, rxBytes;
	int maxGameInactivityTime, maxPlayerInactivityTime;
	int maxUsersPerAddress, messageCountingInterval, maxMessageCountPerInterval, maxMessageSizePerInterval, maxGamesPerUser;
	ServerInfo_User evalUserQueryResult(const QSqlQuery &query, bool complete, bool withId = false);
	
	QString shutdownReason;
	int shutdownMinutes;
	QTimer *shutdownTimer;
	
	mutable QMutex serverListMutex;
	QList<ServerProperties> serverList;
	void updateServerList();
	
	QList<NetworkServerInterface *> networkServerInterfaces;
};

#endif
