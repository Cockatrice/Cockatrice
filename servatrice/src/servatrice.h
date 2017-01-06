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
#if QT_VERSION > 0x050300
  #include <QWebSocketServer>
#endif
#include <QMutex>
#include <QSslCertificate>
#include <QSslKey>
#include <QHostAddress>
#include <QReadWriteLock>
#include <QSqlDatabase>
#include <QMetaType>
#include "server.h"


Q_DECLARE_METATYPE(QSqlDatabase)

class QSqlQuery;
class QTimer;

class GameReplay;
class Servatrice;
class Servatrice_ConnectionPool;
class Servatrice_DatabaseInterface;
class AbstractServerSocketInterface;
class IslInterface;
class FeatureSet;

class Servatrice_GameServer : public QTcpServer {
    Q_OBJECT
private:
    Servatrice *server;
    QList<Servatrice_ConnectionPool *> connectionPools;
public:
    Servatrice_GameServer(Servatrice *_server, int _numberPools, const QSqlDatabase &_sqlDatabase, QObject *parent = 0);
    ~Servatrice_GameServer();
protected:
    void incomingConnection(qintptr socketDescriptor);
    Servatrice_ConnectionPool *findLeastUsedConnectionPool();
};

#if QT_VERSION > 0x050300
class Servatrice_WebsocketGameServer : public QWebSocketServer {
    Q_OBJECT
private:
    Servatrice *server;
    QList<Servatrice_ConnectionPool *> connectionPools;
public:
    Servatrice_WebsocketGameServer(Servatrice *_server, int _numberPools, const QSqlDatabase &_sqlDatabase, QObject *parent = 0);
    ~Servatrice_WebsocketGameServer();
protected:
    Servatrice_ConnectionPool *findLeastUsedConnectionPool();
protected slots:
    void onNewConnection();
};
#endif

class Servatrice_IslServer : public QTcpServer {
    Q_OBJECT
private:
    Servatrice *server;
    QSslCertificate cert;
    QSslKey privateKey;
public:
    Servatrice_IslServer(Servatrice *_server, const QSslCertificate &_cert, const QSslKey &_privateKey, QObject *parent = 0)
        : QTcpServer(parent), server(_server), cert(_cert), privateKey(_privateKey) { }
protected:
    void incomingConnection(qintptr socketDescriptor);
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
public:
    enum AuthenticationMethod { AuthenticationNone, AuthenticationSql, AuthenticationPassword };
private slots:
    void statusUpdate();
    void shutdownTimeout();
protected:
    void doSendIslMessage(const IslMessage &msg, int serverId);
private:
    enum DatabaseType { DatabaseNone, DatabaseMySql };
    AuthenticationMethod authenticationMethod;
    DatabaseType databaseType;
    QTimer *pingClock, *statusUpdateClock;
    Servatrice_GameServer *gameServer;
#if QT_VERSION > 0x050300
    Servatrice_WebsocketGameServer *websocketGameServer;
#endif
    Servatrice_IslServer *islServer;
    mutable QMutex loginMessageMutex;
    QString loginMessage;
    QString dbPrefix;
    QString requiredFeatures;
    QMap<QString, bool> serverRequiredFeatureList;
    QString officialWarnings;
    Servatrice_DatabaseInterface *servatriceDatabaseInterface;
    int serverId;
    int uptime;
    QMutex txBytesMutex, rxBytesMutex;
    quint64 txBytes, rxBytes;

    QString shutdownReason;
    int shutdownMinutes;
    int nextShutdownMessageMinutes;
    QTimer *shutdownTimer;
    bool isFirstShutdownMessage;

    mutable QMutex serverListMutex;
    QList<ServerProperties> serverList;
    void updateServerList();

    QMap<int, IslInterface *> islInterfaces;

    QString getDBPrefixString() const;
    QString getDBHostNameString() const;
    QString getDBDatabaseNameString() const;
    QString getDBUserNameString() const;
    QString getDBPasswordString() const;
    QString getRoomsMethodString() const;
    QString getISLNetworkSSLCertFile() const;
    QString getISLNetworkSSLKeyFile() const;
    
    int getServerStatusUpdateTime() const;
    int getNumberOfTCPPools() const;
    int getServerTCPPort() const;
    int getNumberOfWebSocketPools() const;
    int getServerWebSocketPort() const;
    int getISLNetworkPort() const;
    bool getISLNetworkEnabled() const;
    bool getEnableInternalSMTPClient() const;

public slots:
    void scheduleShutdown(const QString &reason, int minutes);
    void updateLoginMessage();
    void setRequiredFeatures(const QString featureList);
public:
    Servatrice(QObject *parent = 0);
    ~Servatrice();
    bool initServer();
    QMap<QString, bool> getServerRequiredFeatureList() const { return serverRequiredFeatureList; }
    QString getOfficialWarningsList() const { return officialWarnings; }
    QString getServerName() const;
    QString getLoginMessage() const { QMutexLocker locker(&loginMessageMutex); return loginMessage; }
    QString getRequiredFeatures() const;
    QString getAuthenticationMethodString() const;
    QString getDBTypeString() const;
    QString getDbPrefix() const { return dbPrefix; }
    AuthenticationMethod getAuthenticationMethod() const { return authenticationMethod; }
    bool permitUnregisteredUsers() const { return authenticationMethod != AuthenticationNone; }
    bool getGameShouldPing() const { return true; }
    bool getClientIDRequiredEnabled() const;
    bool getRegOnlyServerEnabled() const;
    bool getMaxUserLimitEnabled() const;
    bool getStoreReplaysEnabled() const;
    bool getRegistrationEnabled() const;
    bool getRequireEmailForRegistrationEnabled() const;
    bool getRequireEmailActivationEnabled() const;
    bool getEnableLogQuery() const;
    int getIdleClientTimeout() const;
    int getServerID() const;
    int getMaxGameInactivityTime() const;
    int getMaxPlayerInactivityTime() const;
    int getClientKeepAlive() const;
    int getMaxUsersPerAddress() const;
    int getMessageCountingInterval() const;
    int getMaxMessageCountPerInterval() const;
    int getMaxMessageSizePerInterval() const;
    int getMaxGamesPerUser() const;
    int getCommandCountingInterval() const;
    int getMaxCommandCountPerInterval() const;
    int getMaxUserTotal() const;
    int getMaxTcpUserLimit() const;
    int getMaxWebSocketUserLimit() const;
    int getUsersWithAddress(const QHostAddress &address) const;
    int getMaxAccountsPerEmail() const;
    QList<AbstractServerSocketInterface *> getUsersWithAddressAsList(const QHostAddress &address) const;
    void incTxBytes(quint64 num);
    void incRxBytes(quint64 num);
    void addDatabaseInterface(QThread *thread, Servatrice_DatabaseInterface *databaseInterface);

    bool islConnectionExists(int serverId) const;
    void addIslInterface(int serverId, IslInterface *interface);
    void removeIslInterface(int serverId);
    QReadWriteLock islLock;

    QList<ServerProperties> getServerList() const;
};

#endif
