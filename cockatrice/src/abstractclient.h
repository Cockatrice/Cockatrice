#ifndef ABSTRACTCLIENT_H
#define ABSTRACTCLIENT_H

#include <QObject>
#include <QVariant>
#include <QMutex>
#include "pb/response.pb.h"
#include "pb/serverinfo_user.pb.h"

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

enum ClientStatus {
    StatusDisconnected,
    StatusDisconnecting,
    StatusConnecting,
    StatusRegistering,
    StatusActivating,
    StatusLoggingIn,
    StatusLoggedIn,
};

class AbstractClient : public QObject {
    Q_OBJECT
signals:
    void statusChanged(ClientStatus _status);
    
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
private slots:
    void queuePendingCommand(PendingCommand *pend);
protected slots:
    void processProtocolItem(const ServerMessage &item);
protected:
    QMap<int, PendingCommand *> pendingCommands;
    QString userName, password, email, country, realName, token;
    int gender;
    void setStatus(ClientStatus _status);
    int getNewCmdId() { return nextCmdId++; }
    virtual void sendCommandContainer(const CommandContainer &cont) = 0;
public:
    AbstractClient(QObject *parent = 0);
    ~AbstractClient();
    
    ClientStatus getStatus() const { QMutexLocker locker(&clientMutex); return status; }
    void sendCommand(const CommandContainer &cont);
    void sendCommand(PendingCommand *pend);

    const QString getUserName() {return userName;}
    
    static PendingCommand *prepareSessionCommand(const ::google::protobuf::Message &cmd);
    static PendingCommand *prepareRoomCommand(const ::google::protobuf::Message &cmd, int roomId);
    static PendingCommand *prepareModeratorCommand(const ::google::protobuf::Message &cmd);
    static PendingCommand *prepareAdminCommand(const ::google::protobuf::Message &cmd);

    QMap<QString, bool> clientFeatures;
};

#endif
