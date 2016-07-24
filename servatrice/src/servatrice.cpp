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
#include <QFile>
#include <QTimer>
#include <QDateTime>
#include <QDebug>
#include <QString>
#include <iostream>
#include "servatrice.h"
#include "servatrice_database_interface.h"
#include "servatrice_connection_pool.h"
#include "server_room.h"
#include "settingscache.h"
#include "serversocketinterface.h"
#include "isl_interface.h"
#include "server_logger.h"
#include "main.h"
#include "decklist.h"
#include "smtpclient.h"
#include "pb/event_server_message.pb.h"
#include "pb/event_server_shutdown.pb.h"
#include "pb/event_connection_closed.pb.h"
#include "featureset.h"

Servatrice_GameServer::Servatrice_GameServer(Servatrice *_server, int _numberPools, const QSqlDatabase &_sqlDatabase, QObject *parent)
    : QTcpServer(parent),
      server(_server)
{
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

void Servatrice_GameServer::incomingConnection(qintptr socketDescriptor)
{
    Servatrice_ConnectionPool *pool = findLeastUsedConnectionPool();

    TcpServerSocketInterface *ssi = new TcpServerSocketInterface(server, pool->getDatabaseInterface());
    ssi->moveToThread(pool->thread());
    pool->addClient();
    connect(ssi, SIGNAL(destroyed()), pool, SLOT(removeClient()));

    QMetaObject::invokeMethod(ssi, "initConnection", Qt::QueuedConnection, Q_ARG(int, socketDescriptor));
}

Servatrice_ConnectionPool *Servatrice_GameServer::findLeastUsedConnectionPool()
{
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
    return connectionPools[poolIndex];
}

#if QT_VERSION > 0x050300
#define WEBSOCKET_POOL_NUMBER 999

Servatrice_WebsocketGameServer::Servatrice_WebsocketGameServer(Servatrice *_server, int /* _numberPools */, const QSqlDatabase &_sqlDatabase, QObject *parent)
    : QWebSocketServer("Servatrice", QWebSocketServer::NonSecureMode, parent),
      server(_server)
{
    // Qt limitation: websockets can't be moved to another thread
    Servatrice_DatabaseInterface *newDatabaseInterface = new Servatrice_DatabaseInterface(WEBSOCKET_POOL_NUMBER, server);
    Servatrice_ConnectionPool *newPool = new Servatrice_ConnectionPool(newDatabaseInterface);

    server->addDatabaseInterface(thread(), newDatabaseInterface);
    newDatabaseInterface->initDatabase(_sqlDatabase);

    connectionPools.append(newPool);

    connect(this, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
}

Servatrice_WebsocketGameServer::~Servatrice_WebsocketGameServer()
{
    for (int i = 0; i < connectionPools.size(); ++i) {
        logger->logMessage(QString("Closing websocket pool %1...").arg(i));
        QThread *poolThread = connectionPools[i]->thread();
        connectionPools[i]->deleteLater(); // pool destructor calls thread()->quit()
        poolThread->wait();
    }
}

void Servatrice_WebsocketGameServer::onNewConnection()
{
    Servatrice_ConnectionPool *pool = findLeastUsedConnectionPool();

    WebsocketServerSocketInterface *ssi = new WebsocketServerSocketInterface(server, pool->getDatabaseInterface());
//    ssi->moveToThread(pool->thread());
    pool->addClient();
    connect(ssi, SIGNAL(destroyed()), pool, SLOT(removeClient()));

    QMetaObject::invokeMethod(ssi, "initConnection", Qt::QueuedConnection, Q_ARG(void *, nextPendingConnection()));
}

Servatrice_ConnectionPool *Servatrice_WebsocketGameServer::findLeastUsedConnectionPool()
{
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
    return connectionPools[poolIndex];
}
#endif

void Servatrice_IslServer::incomingConnection(qintptr socketDescriptor)
{
    QThread *thread = new QThread;
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    IslInterface *interface = new IslInterface(socketDescriptor, cert, privateKey, server);
    interface->moveToThread(thread);
    connect(interface, SIGNAL(destroyed()), thread, SLOT(quit()));

    thread->start();
    QMetaObject::invokeMethod(interface, "initServer", Qt::QueuedConnection);
}

Servatrice::Servatrice(QObject *parent)
    : Server(parent), uptime(0), shutdownTimer(0), isFirstShutdownMessage(true)
{
    qRegisterMetaType<QSqlDatabase>("QSqlDatabase");
}

Servatrice::~Servatrice()
{
    gameServer->close();

    // clients live in other threads, we need to lock them 
    clientsLock.lockForRead();
    for (int i = 0; i < clients.size(); ++i)
        QMetaObject::invokeMethod(clients.at(i), "prepareDestroy", Qt::QueuedConnection);
    clientsLock.unlock();

    // client destruction is asynchronous, wait for all clients to be gone
    bool done = false;

    class SleeperThread : public QThread
    {
    public:
        static void msleep(unsigned long msecs) { QThread::usleep(msecs); }
    };

    do {
        SleeperThread::msleep(10);
        clientsLock.lockForRead();
        if (clients.isEmpty())
            done = true;
        clientsLock.unlock();
    } while (!done);

    prepareDestroy();
}

bool Servatrice::initServer()
{
    serverName = settingsCache->value("server/name", "My Cockatrice server").toString();
    serverId = settingsCache->value("server/id", 0).toInt();
    clientIdRequired = settingsCache->value("server/requireclientid",0).toBool();
    regServerOnly = settingsCache->value("authentication/regonly", 0).toBool();

    const QString authenticationMethodStr = settingsCache->value("authentication/method").toString();
    if (authenticationMethodStr == "sql") {
        qDebug() << "Authenticating method: sql";
        authenticationMethod = AuthenticationSql;
    } else if(authenticationMethodStr == "password") {
        qDebug() << "Authenticating method: password";
        authenticationMethod = AuthenticationPassword;
    } else {
        if (regServerOnly) {
            qDebug() << "Registration only server enabled but no authentication method defined: Error.";
            return false;
        }

        qDebug() << "Authenticating method: none";
        authenticationMethod = AuthenticationNone;
    }

    qDebug() << "Store Replays: " << settingsCache->value("game/store_replays", true).toBool();
    qDebug() << "Client ID Required: " << clientIdRequired;
    bool maxUserLimitEnabled = settingsCache->value("security/enable_max_user_limit", false).toBool();
    qDebug() << "Maximum user limit enabled: " << maxUserLimitEnabled;

    if (maxUserLimitEnabled){
        int maxUserLimit = settingsCache->value("security/max_users_total", 500).toInt();
        qDebug() << "Maximum total user limit: " << maxUserLimit;
        int maxTcpUserLimit = settingsCache->value("security/max_users_tcp", 500).toInt();
        qDebug() << "Maximum tcp user limit: " << maxTcpUserLimit;
        int maxWebsocketUserLimit = settingsCache->value("security/max_users_websocket", 500).toInt();
        qDebug() << "Maximum websocket user limit: " << maxWebsocketUserLimit;
    }

    bool registrationEnabled = settingsCache->value("registration/enabled", false).toBool();
    bool requireEmailForRegistration = settingsCache->value("registration/requireemail", true).toBool();
    bool requireEmailActivation = settingsCache->value("registration/requireemailactivation", true).toBool();

    qDebug() << "Accept registered users only: " << regServerOnly;
    qDebug() << "Registration enabled: " << registrationEnabled;
    if (registrationEnabled)
    {
        qDebug() << "Require email address to register: " << requireEmailForRegistration;
        qDebug() << "Require email activation via token: " << requireEmailActivation;
    }

    FeatureSet features;
    features.initalizeFeatureList(serverRequiredFeatureList);
    requiredFeatures = settingsCache->value("server/requiredfeatures","").toString();
    QStringList listReqFeatures = requiredFeatures.split(",", QString::SkipEmptyParts);
    if (!listReqFeatures.isEmpty())
        foreach(QString reqFeature, listReqFeatures)
            features.enableRequiredFeature(serverRequiredFeatureList,reqFeature);

    qDebug() << "Required client features: " << serverRequiredFeatureList;

    QString dbTypeStr = settingsCache->value("database/type").toString();
    if (dbTypeStr == "mysql")
        databaseType = DatabaseMySql;
    else
        databaseType = DatabaseNone;

    servatriceDatabaseInterface = new Servatrice_DatabaseInterface(-1, this);
    setDatabaseInterface(servatriceDatabaseInterface);

    if (databaseType != DatabaseNone) {
        settingsCache->beginGroup("database");
        dbPrefix = settingsCache->value("prefix").toString();
        bool dbOpened =
            servatriceDatabaseInterface->initDatabase("QMYSQL",
                 settingsCache->value("hostname").toString(),
                 settingsCache->value("database").toString(),
                 settingsCache->value("user").toString(),
                 settingsCache->value("password").toString());
        settingsCache->endGroup();
        if (!dbOpened) {
            qDebug() << "Failed to open database";
            return false;
        }

        updateServerList();

        qDebug() << "Clearing previous sessions...";
        servatriceDatabaseInterface->clearSessionTables();
    }

    const QString roomMethod = settingsCache->value("rooms/method").toString();
    if (roomMethod == "sql") {
        QSqlQuery *query = servatriceDatabaseInterface->prepareQuery("select id, name, descr, permissionlevel, auto_join, join_message, chat_history_size from {prefix}_rooms where id_server = :id_server order by id asc");
        query->bindValue(":id_server", serverId);
        servatriceDatabaseInterface->execSqlQuery(query);
        while (query->next()) {
            QSqlQuery *query2 = servatriceDatabaseInterface->prepareQuery("select name from {prefix}_rooms_gametypes where id_room = :id_room AND id_server = :id_server");
            query2->bindValue(":id_server", serverId);
            query2->bindValue(":id_room", query->value(0).toInt());
            servatriceDatabaseInterface->execSqlQuery(query2);
            QStringList gameTypes;
            while (query2->next())
                gameTypes.append(query2->value(0).toString());

            addRoom(new Server_Room(query->value(0).toInt(),
                                    query->value(6).toInt(),
                                    query->value(1).toString(),
                                    query->value(2).toString(),
                                    query->value(3).toString().toLower(),
                                    query->value(4).toInt(),
                                    query->value(5).toString(),
                                    gameTypes,
                                    this
            ));
        }
    } else {
        int size = settingsCache->beginReadArray("rooms/roomlist");
        for (int i = 0; i < size; ++i) {
            settingsCache->setArrayIndex(i);

            QStringList gameTypes;
            int size2 = settingsCache->beginReadArray("game_types");
                for (int j = 0; j < size2; ++j) {
                settingsCache->setArrayIndex(j);
                gameTypes.append(settingsCache->value("name").toString());
            }
            settingsCache->endArray();

            Server_Room *newRoom = new Server_Room(
                i,
                settingsCache->value("chathistorysize").toInt(),
                settingsCache->value("name").toString(),
                settingsCache->value("description").toString(),
                settingsCache->value("permissionlevel").toString().toLower(),
                settingsCache->value("autojoin").toBool(),
                settingsCache->value("joinmessage").toString(),
                gameTypes,
                this
            );
            addRoom(newRoom);
        }

        if(size==0)
        {
            // no room defined in config, add a dummy one
            Server_Room *newRoom = new Server_Room(
                0,
                100,
                "General room",
                "Play anything here.",
                "none",
                true,
                "",
                QStringList("Standard"),
                this
            );
            addRoom(newRoom);
        }

        settingsCache->endArray();
    }

    updateLoginMessage();

    maxGameInactivityTime = settingsCache->value("game/max_game_inactivity_time", 120).toInt();
    maxPlayerInactivityTime = settingsCache->value("server/max_player_inactivity_time", 15).toInt();
    pingClockInterval = settingsCache->value("server/clientkeepalive", 1).toInt();
    maxUsersPerAddress = settingsCache->value("security/max_users_per_address", 4).toInt();
    messageCountingInterval = settingsCache->value("security/message_counting_interval", 10).toInt();
    maxMessageCountPerInterval = settingsCache->value("security/max_message_count_per_interval", 15).toInt();
    maxMessageSizePerInterval = settingsCache->value("security/max_message_size_per_interval", 1000).toInt();
    maxGamesPerUser = settingsCache->value("security/max_games_per_user", 5).toInt();
    commandCountingInterval = settingsCache->value("game/command_counting_interval", 10).toInt();
    maxCommandCountPerInterval = settingsCache->value("game/max_command_count_per_interval", 20).toInt();

    try { if (settingsCache->value("servernetwork/active", 0).toInt()) {
        qDebug() << "Connecting to ISL network.";
        const QString certFileName = settingsCache->value("servernetwork/ssl_cert").toString();
        const QString keyFileName = settingsCache->value("servernetwork/ssl_key").toString();
        qDebug() << "Loading certificate...";
        QFile certFile(certFileName);
        if (!certFile.open(QIODevice::ReadOnly))
            throw QString("Error opening certificate file: %1").arg(certFileName);
        QSslCertificate cert(&certFile);

        const QDateTime currentTime = QDateTime::currentDateTime();
        if(currentTime < cert.effectiveDate() ||
            currentTime > cert.expiryDate() ||
            cert.isBlacklisted())
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

        const int networkPort = settingsCache->value("servernetwork/port", 14747).toInt();
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
    pingClock->start(pingClockInterval * 1000);

    int statusUpdateTime = settingsCache->value("server/statusupdate", 15000).toInt();
    statusUpdateClock = new QTimer(this);
    connect(statusUpdateClock, SIGNAL(timeout()), this, SLOT(statusUpdate()));
    if (statusUpdateTime != 0) {
        qDebug() << "Starting status update clock, interval " << statusUpdateTime << " ms";
        statusUpdateClock->start(statusUpdateTime);
    }

    // SOCKET SERVER
    const int numberPools = settingsCache->value("server/number_pools", 1).toInt();
    if(numberPools > 0)
    {
        gameServer = new Servatrice_GameServer(this, numberPools, servatriceDatabaseInterface->getDatabase(), this);
        gameServer->setMaxPendingConnections(1000);
        const int gamePort = settingsCache->value("server/port", 4747).toInt();
        qDebug() << "Starting server on port" << gamePort;
        if (gameServer->listen(QHostAddress::Any, gamePort))
            qDebug() << "Server listening.";
        else {
            qDebug() << "gameServer->listen(): Error:" << gameServer->errorString();
            return false;
        }
    }

#if QT_VERSION > 0x050300
    // WEBSOCKET SERVER
    const int wesocketNumberPools = settingsCache->value("server/websocket_number_pools", 1).toInt();
    if(wesocketNumberPools > 0)
    {
        websocketGameServer = new Servatrice_WebsocketGameServer(this, wesocketNumberPools, servatriceDatabaseInterface->getDatabase(), this);
        websocketGameServer->setMaxPendingConnections(1000);
        const int websocketGamePort = settingsCache->value("server/websocket_port", 4748).toInt();
        qDebug() << "Starting websocket server on port" << websocketGamePort;
        if (websocketGameServer->listen(QHostAddress::Any, websocketGamePort))
            qDebug() << "Websocket server listening.";
        else {
            qDebug() << "websocketGameServer->listen(): Error:" << websocketGameServer->errorString();
            return false;
        }
    }
#endif
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

    QSqlQuery *query = servatriceDatabaseInterface->prepareQuery("select id, ssl_cert, hostname, address, game_port, control_port from {prefix}_servers order by id asc");
    servatriceDatabaseInterface->execSqlQuery(query);
    while (query->next()) {
        ServerProperties prop(query->value(0).toInt(), QSslCertificate(query->value(1).toString().toUtf8()), query->value(2).toString(), QHostAddress(query->value(3).toString()), query->value(4).toInt(), query->value(5).toInt());
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
    if (static_cast<AbstractServerSocketInterface *>(clients[i])->getPeerAddress() == address)
        ++result;

    return result;
}

QList<AbstractServerSocketInterface *> Servatrice::getUsersWithAddressAsList(const QHostAddress &address) const
{
    QList<AbstractServerSocketInterface *> result;
    QReadLocker locker(&clientsLock);
    for (int i = 0; i < clients.size(); ++i)
        if (static_cast<AbstractServerSocketInterface *>(clients[i])->getPeerAddress() == address)
            result.append(static_cast<AbstractServerSocketInterface *>(clients[i]));
    return result;
}

void Servatrice::updateLoginMessage()
{
    if (!servatriceDatabaseInterface->checkSql())
        return;

    QSqlQuery *query = servatriceDatabaseInterface->prepareQuery("select message from {prefix}_servermessages where id_server = :id_server order by timest desc limit 1");
    query->bindValue(":id_server", serverId);
    if (servatriceDatabaseInterface->execSqlQuery(query))
        if (query->next()) {
            const QString newLoginMessage = query->value(0).toString();

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

    QSqlQuery *query = servatriceDatabaseInterface->prepareQuery("insert into {prefix}_uptime (id_server, timest, uptime, users_count, games_count, tx_bytes, rx_bytes) values(:id, NOW(), :uptime, :users_count, :games_count, :tx, :rx)");
    query->bindValue(":id", serverId);
    query->bindValue(":uptime", uptime);
    query->bindValue(":users_count", uc);
    query->bindValue(":games_count", gc);
    query->bindValue(":tx", tx);
    query->bindValue(":rx", rx);
    servatriceDatabaseInterface->execSqlQuery(query);

    // send activation emails
    bool registrationEnabled = settingsCache->value("registration/enabled", false).toBool();
    bool requireEmailActivation = settingsCache->value("registration/requireemailactivation", true).toBool();
    if (registrationEnabled && requireEmailActivation)
    {
        QSqlQuery *query = servatriceDatabaseInterface->prepareQuery("select a.name, b.email, b.token from {prefix}_activation_emails a left join {prefix}_users b on a.name = b.name");
        if (!servatriceDatabaseInterface->execSqlQuery(query))
            return;

        QSqlQuery *queryDelete = servatriceDatabaseInterface->prepareQuery("delete from {prefix}_activation_emails where name = :name");

        while (query->next()) {
            const QString userName = query->value(0).toString();
            const QString emailAddress = query->value(1).toString();
            const QString token = query->value(2).toString();

            if(smtpClient->enqueueActivationTokenMail(userName, emailAddress, token))
            {
                queryDelete->bindValue(":name", userName);
                servatriceDatabaseInterface->execSqlQuery(queryDelete);
            }
        }

        smtpClient->sendAllEmails();
    }
}

void Servatrice::scheduleShutdown(const QString &reason, int minutes)
{
    shutdownReason = reason;
    shutdownMinutes = minutes;
    nextShutdownMessageMinutes = shutdownMinutes;
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
    // Show every time counter cut in half & every minute for last 5 minutes
    if (shutdownMinutes <= 5 || shutdownMinutes == nextShutdownMessageMinutes) {
        if (shutdownMinutes == nextShutdownMessageMinutes)
            nextShutdownMessageMinutes = shutdownMinutes / 2;

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
    shutdownMinutes--;
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
