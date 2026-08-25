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

#include "metrics_registry.h"

#include <QDateTime>
#include <QHostAddress>
#include <QMetaType>
#include <QMutex>
#include <QReadWriteLock>
#include <QSqlDatabase>
#include <QSslCertificate>
#include <QSslKey>
#include <QTcpServer>
#include <QWebSocketServer>
#include <atomic>
#include <libcockatrice/protocol/pb/response_report_stats.pb.h>
#include <memory>
#include <server.h>
#include <utility>

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

class Servatrice_GameServer : public QTcpServer
{
    Q_OBJECT
private:
    Servatrice *server;
    QList<Servatrice_ConnectionPool *> connectionPools;

public:
    Servatrice_GameServer(Servatrice *_server,
                          int _numberPools,
                          const QSqlDatabase &_sqlDatabase,
                          QObject *parent = nullptr);
    ~Servatrice_GameServer() override;

protected:
    void incomingConnection(qintptr socketDescriptor) override;
    Servatrice_ConnectionPool *findLeastUsedConnectionPool();
};

class Servatrice_WebsocketGameServer : public QWebSocketServer
{
    Q_OBJECT
private:
    Servatrice *server;
    QList<Servatrice_ConnectionPool *> connectionPools;

public:
    Servatrice_WebsocketGameServer(Servatrice *_server,
                                   int _numberPools,
                                   const QSqlDatabase &_sqlDatabase,
                                   QObject *parent = nullptr);
    ~Servatrice_WebsocketGameServer() override;

protected:
    Servatrice_ConnectionPool *findLeastUsedConnectionPool();
protected slots:
    void onNewConnection();
};

class Servatrice_IslServer : public QTcpServer
{
    Q_OBJECT
private:
    Servatrice *server;
    QSslCertificate cert;
    QSslKey privateKey;

public:
    Servatrice_IslServer(Servatrice *_server,
                         const QSslCertificate &_cert,
                         QSslKey _privateKey,
                         QObject *parent = nullptr)
        : QTcpServer(parent), server(_server), cert(_cert), privateKey(std::move(_privateKey))
    {
    }

protected:
    void incomingConnection(qintptr socketDescriptor) override;
};

class ServerProperties
{
public:
    int id;
    QSslCertificate cert;
    QString hostname;
    QHostAddress address;
    int gamePort;
    int controlPort;

    ServerProperties(int _id,
                     const QSslCertificate &_cert,
                     QString _hostname,
                     const QHostAddress &_address,
                     int _gamePort,
                     int _controlPort)
        : id(_id), cert(_cert), hostname(std::move(_hostname)), address(_address), gamePort(_gamePort),
          controlPort(_controlPort)
    {
    }
};

class Servatrice : public Server
{
    Q_OBJECT
public:
    enum AuthenticationMethod
    {
        AuthenticationNone,
        AuthenticationSql,
        AuthenticationPassword
    };
private slots:
    void statusUpdate();
    void shutdownTimeout();

protected:
    void doSendIslMessage(const IslMessage &msg, int _serverId) override;

private:
    enum DatabaseType
    {
        DatabaseNone,
        DatabaseMySql
    };
    AuthenticationMethod authenticationMethod;
    DatabaseType databaseType;
    QTimer *pingClock, *statusUpdateClock;
    Servatrice_GameServer *gameServer;
    Servatrice_WebsocketGameServer *websocketGameServer;
    Servatrice_IslServer *islServer;
    mutable QMutex loginMessageMutex;
    QString loginMessage;
    mutable QMutex shutdownStateMutex;
    QString dbPrefix;
    QMap<QString, bool> serverRequiredFeatureList;
    QString officialWarnings;
    Servatrice_DatabaseInterface *servatriceDatabaseInterface;
    int serverId;
    int uptime;
    QMutex txBytesMutex, rxBytesMutex;
    quint64 txBytes, rxBytes;
    std::atomic<quint64> txBytesTotal{0}; ///< cumulative bytes sent since process start
    std::atomic<quint64> rxBytesTotal{0}; ///< cumulative bytes received since process start
    MetricsRegistry metricsRegistry;
    int metricsSlowCommandMs = 500;
    int metricsStallWarnMs = 2000;
    std::atomic<qint64> eventLoopStallsTotal{0}; ///< heartbeat overshoots past the warn threshold
    std::atomic<qint64> eventLoopLastStallMs{0}; ///< overshoot of the most recent stall
    std::atomic<qint64> eventLoopMaxStallMs{0};  ///< worst overshoot seen since process start

    QString shutdownReason;
    int shutdownMinutes;
    int nextShutdownMessageMinutes;
    QTimer *shutdownTimer;

    mutable QMutex reportStatsMutex;
    QDateTime reportStatsTimestamp;
    std::shared_ptr<const Response_ReportStats> reportStatsCache;
    static constexpr int reportStatsCacheTtlSeconds = 60;

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
    QHostAddress getServerTCPHost() const;
    QHostAddress getServerWebSocketHost() const;

public slots:
    void scheduleShutdown(const QString &reason, int minutes);
    void updateLoginMessage();
    void setRequiredFeatures(const QString &featureList);

public:
    explicit Servatrice(QObject *parent = nullptr);
    ~Servatrice() override;
    bool initServer();
    QMap<QString, bool> getServerRequiredFeatureList() const override
    {
        return serverRequiredFeatureList;
    }
    QString getServerName() const;
    QString getLoginMessage() const override
    {
        QMutexLocker locker(&loginMessageMutex);
        return loginMessage;
    }
    SessionEvent *getLoginSessionEvent() const override;
    SessionEvent *makeShutdownEvent() const;
    QString getRequiredFeatures() const override;
    QString getAuthenticationMethodString() const;
    QString getDBTypeString() const;
    QString getDbPrefix() const
    {
        return dbPrefix;
    }
    QString getEmailBlackList() const;
    QString getEmailWhiteList() const;
    AuthenticationMethod getAuthenticationMethod() const
    {
        return authenticationMethod;
    }
    bool permitUnregisteredUsers() const override
    {
        return authenticationMethod != AuthenticationNone;
    }
    bool getGameShouldPing() const override
    {
        return true;
    }
    bool getClientIDRequiredEnabled() const override;
    bool getRegOnlyServerEnabled() const override;
    bool getMaxUserLimitEnabled() const override;
    bool getStoreReplaysEnabled() const override;
    bool getRegistrationEnabled() const;
    bool getRequireEmailForRegistrationEnabled() const;
    bool getRequireEmailActivationEnabled() const;
    bool getEnableLogQuery() const override;
    bool getEnableForgotPassword() const;
    bool getEnableForgotPasswordChallenge() const;
    bool getEnableAudit() const;
    bool getEnableRegistrationAudit() const;
    bool getEnableForgotPasswordAudit() const;
    int getMinPasswordLength() const;
    int getIdleClientTimeout() const override;
    int getServerID() const override;
    int getMaxGameInactivityTime() const override;
    int getMaxPlayerInactivityTime() const override;
    int getClientKeepAlive() const override;
    int getMaxUsersPerAddress() const;
    int getMessageCountingInterval() const override;
    int getMaxMessageCountPerInterval() const override;
    int getMaxMessageSizePerInterval() const override;
    int getMaxGamesPerUser() const override;
    int getCommandCountingInterval() const override;
    int getMaxCommandCountPerInterval() const override;
    int getMaxUserTotal() const override;
    bool permitCreateGameAsJudge() const override;
    int getMaxTcpUserLimit() const;
    int getMaxWebSocketUserLimit() const;
    int getUsersWithAddress(const QHostAddress &address) const;
    int getMaxAccountsPerEmail() const;
    int getForgotPasswordTokenLife() const;
    QList<AbstractServerSocketInterface *> getUsersWithAddressAsList(const QHostAddress &address) const;
    void incTxBytes(quint64 num);
    void incRxBytes(quint64 num);
    void addDatabaseInterface(QThread *thread, Servatrice_DatabaseInterface *databaseInterface);

    // Metrics (see [metrics] section in servatrice.ini.example)
    MetricsRegistry &getMetricsRegistry()
    {
        return metricsRegistry;
    }
    quint64 getTxBytesTotal() const
    {
        return txBytesTotal.load(std::memory_order_relaxed);
    }
    quint64 getRxBytesTotal() const
    {
        return rxBytesTotal.load(std::memory_order_relaxed);
    }
    int getUptimeSeconds() const
    {
        return uptime;
    }
    /**
     * Sums cards across all zones of all running games. Locks rooms and games
     * briefly per level, so scrape-time cost grows with live game count only.
     */
    qint64 getCardsInGamesTotal() const;
    int getMetricsSlowCommandMs() const
    {
        return metricsSlowCommandMs;
    }
    /// Heartbeat overshoot that counts as a stall. A value of 0 disables the watchdogs.
    int getMetricsStallWarnMs() const
    {
        return metricsStallWarnMs;
    }
    void observeGameStartDurationMs(qint64 elapsedMs) override;
    qint64 getEventLoopStallsTotal() const
    {
        return eventLoopStallsTotal.load(std::memory_order_relaxed);
    }
    qint64 getEventLoopLastStallMs() const
    {
        return eventLoopLastStallMs.load(std::memory_order_relaxed);
    }
    qint64 getEventLoopMaxStallMs() const
    {
        return eventLoopMaxStallMs.load(std::memory_order_relaxed);
    }
    /// Records one heartbeat overshoot and logs a single warning for it.
    void observeEventLoopStall(const QString &threadName, qint64 overshootMs);
    /**
     * Installs an EventLoopWatchdog in @p thread. Called once per socket pool
     * thread right after it starts.
     */
    void watchWorkerThread(QThread *thread);

    bool islConnectionExists(int _serverId) const;
    void addIslInterface(int _serverId, IslInterface *interface);
    void removeIslInterface(int _serverId);
    QReadWriteLock islLock;

    // The moderation queue statistics are shared between all connected moderators and
    // cached briefly to avoid re-running several full-table queries on every refresh.
    std::shared_ptr<const Response_ReportStats> getCachedReportStats() const;
    void cacheReportStats(const Response_ReportStats &stats);

    QList<ServerProperties> getServerList() const;
};

#endif
