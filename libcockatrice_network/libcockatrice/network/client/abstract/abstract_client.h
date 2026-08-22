/**
 * @file abstract_client.h
 * @ingroup Client
 */
//! \todo Document this file.

#ifndef ABSTRACTCLIENT_H
#define ABSTRACTCLIENT_H

#include "latency_tracker.h"

#include <QElapsedTimer>
#include <QLoggingCategory>
#include <QMutex>
#include <QVariant>
#include <libcockatrice/protocol/pb/response.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_user.pb.h>

inline Q_LOGGING_CATEGORY(AbstractClientLog, "abstract_client");

class PendingCommand;
class CommandContainer;
class RoomEvent;
class GameEventContainer;
class ServerMessage;
class Event_ServerIdentification;
class Event_AddToList;
class Event_RemoveFromList;
class Event_UserJoined;
class Event_UserLeft;
class Event_ServerMessage;
class Event_ListRooms;
class Event_GameJoined;
class Event_UserMessage;
class Event_NotifyUser;
class Event_ConnectionClosed;
class Event_ServerShutdown;
class Event_ReplayAdded;
class FeatureSet;

enum ClientStatus
{
    StatusDisconnected,
    StatusDisconnecting,
    StatusConnecting,
    StatusRegistering,
    StatusActivating,
    StatusLoggingIn,
    StatusLoggedIn,
    StatusRequestingForgotPassword,
    StatusSubmitForgotPasswordReset,
    StatusSubmitForgotPasswordChallenge,
    StatusGettingPasswordSalt,
};

class AbstractClient : public QObject
{
    Q_OBJECT
signals:
    void statusChanged(ClientStatus _status);
    void maxPingTime(int seconds, int maxSeconds);

    /**
     * @brief Aggregated round-trip statistics and a chronological snapshot of
     * the rolling window, emitted at most once per second.
     *
     * All values in the stats struct are in milliseconds; sampleCount is the
     * number of samples currently in the rolling window. The samples list is
     * ordered oldest first so graphs can redraw without polling the tracker
     * across threads. Emitted from the client thread. The connection to UI
     * objects is automatically queued across threads.
     */
    void pingStatsUpdated(const LatencyTracker::Stats &stats, const QList<int> &samplesMs);

    // Room events
    void roomEventReceived(const RoomEvent &event);
    // Game events
    void gameEventContainerReceived(const GameEventContainer &event);
    // Session events
    void serverIdentificationEventReceived(const Event_ServerIdentification &event);
    void connectionClosedEventReceived(const Event_ConnectionClosed &event);
    void serverShutdownEventReceived(const Event_ServerShutdown &event);
    void addToListEventReceived(const Event_AddToList &event);
    void removeFromListEventReceived(const Event_RemoveFromList &event);
    void userJoinedEventReceived(const Event_UserJoined &event);
    void userLeftEventReceived(const Event_UserLeft &event);
    void serverMessageEventReceived(const Event_ServerMessage &event);
    void listRoomsEventReceived(const Event_ListRooms &event);
    void gameJoinedEventReceived(const Event_GameJoined &event);
    void userMessageEventReceived(const Event_UserMessage &event);
    void notifyUserEventReceived(const Event_NotifyUser &event);
    void userInfoChanged(const ServerInfo_User &userInfo);
    void buddyListReceived(const QList<ServerInfo_User> &buddyList);
    void ignoreListReceived(const QList<ServerInfo_User> &ignoreList);
    void replayAddedEventReceived(const Event_ReplayAdded &event);
    void registerAccepted();
    void registerAcceptedNeedsActivate();
    void activateAccepted();

    void sigQueuePendingCommand(PendingCommand *pend);

private:
    int nextCmdId;
    mutable QMutex clientMutex;
    ClientStatus status;
    LatencyTracker latencyTracker;
    QElapsedTimer statsEmitClock;
    bool statsEmitClockStarted = false;

    void recordLatency(PendingCommand &pend);
private slots:
    void queuePendingCommand(PendingCommand *pend);
protected slots:
    void processProtocolItem(const ServerMessage &item);

protected:
    QMap<int, PendingCommand *> pendingCommands;
    QString userName, password, email, country, realName, token;
    bool serverSupportsPasswordHash;
    void setStatus(ClientStatus _status);
    int getNewCmdId()
    {
        return nextCmdId++;
    }
    virtual void sendCommandContainer(const CommandContainer &cont) = 0;

public:
    explicit AbstractClient(QObject *parent = nullptr);
    ~AbstractClient() override;

    ClientStatus getStatus() const
    {
        QMutexLocker locker(&clientMutex);
        return status;
    }
    void sendCommand(const CommandContainer &cont);
    void sendCommand(PendingCommand *pend);

    /**
     * @brief Drops all recorded round-trip samples and resets the stats
     * emission throttle, emitting zeroed stats so that UI listeners can
     * clear their display.
     *
     * Must be called from the client thread (as RemoteClient's disconnect
     * path does). The tracker is deliberately lock-free.
     */
    void clearLatencyStats();

    bool getServerSupportsPasswordHash() const
    {
        return serverSupportsPasswordHash;
    }
    const QString &getUserName() const
    {
        return userName;
    }

    /**
     * @brief Returns the server address configured for the current connection.
     *
     * May be empty for clients that have no server counterpart (e.g. local test clients).
     */
    virtual QString serverName() const
    {
        return {};
    }
    virtual quint16 serverPort() const
    {
        return 0;
    }

    static PendingCommand *prepareSessionCommand(const ::google::protobuf::Message &cmd);
    static PendingCommand *prepareRoomCommand(const ::google::protobuf::Message &cmd, int roomId);
    static PendingCommand *prepareModeratorCommand(const ::google::protobuf::Message &cmd);
    static PendingCommand *prepareAdminCommand(const ::google::protobuf::Message &cmd);

    QMap<QString, bool> clientFeatures;
};

#endif
