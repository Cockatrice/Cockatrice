#ifndef SERVER_PROTOCOLHANDLER_H
#define SERVER_PROTOCOLHANDLER_H

#include <QObject>
#include <QPair>
#include "server.h"
#include "server_abstractuserinterface.h"
#include "pb/response.pb.h"
#include "pb/server_message.pb.h"

class Features;
class Server_DatabaseInterface;
class Server_Player;
class ServerInfo_User;
class Server_Room;
class QTimer;
class FeatureSet;

class ServerMessage;
class Response;
class SessionEvent;
class GameEventContainer;
class RoomEvent;
class ResponseContainer;

class CommandContainer;
class SessionCommand;
class ModeratorCommand;
class AdminCommand;

class Command_Ping;
class Command_Login;
class Command_Register;
class Command_Message;
class Command_ListUsers;
class Command_GetGamesOfUser;
class Command_GetUserInfo;
class Command_ListRooms;
class Command_JoinRoom;
class Command_LeaveRoom;
class Command_RoomSay;
class Command_CreateGame;
class Command_JoinGame;

class Server_ProtocolHandler : public QObject, public Server_AbstractUserInterface {
    Q_OBJECT
protected:
    QMap<int, Server_Room *> rooms;
    
    bool deleted;
    Server_DatabaseInterface *databaseInterface;
    AuthenticationResult authState;
    bool acceptsUserListChanges;
    bool acceptsRoomListChanges;
    virtual void logDebugMessage(const QString & /* message */) { }
private:
    QList<int> messageSizeOverTime, messageCountOverTime, commandCountOverTime;
    int timeRunning, lastDataReceived;
    QTimer *pingClock;

    virtual void transmitProtocolItem(const ServerMessage &item) = 0;
    
    Response::ResponseCode cmdPing(const Command_Ping &cmd, ResponseContainer &rc);
    Response::ResponseCode cmdLogin(const Command_Login &cmd, ResponseContainer &rc);
    Response::ResponseCode cmdMessage(const Command_Message &cmd, ResponseContainer &rc);
    Response::ResponseCode cmdGetGamesOfUser(const Command_GetGamesOfUser &cmd, ResponseContainer &rc);
    Response::ResponseCode cmdGetUserInfo(const Command_GetUserInfo &cmd, ResponseContainer &rc);
    Response::ResponseCode cmdListRooms(const Command_ListRooms &cmd, ResponseContainer &rc);
    Response::ResponseCode cmdJoinRoom(const Command_JoinRoom &cmd, ResponseContainer &rc);
    Response::ResponseCode cmdListUsers(const Command_ListUsers &cmd, ResponseContainer &rc);
    Response::ResponseCode cmdLeaveRoom(const Command_LeaveRoom &cmd, Server_Room *room, ResponseContainer &rc);
    Response::ResponseCode cmdRoomSay(const Command_RoomSay &cmd, Server_Room *room, ResponseContainer &rc);
    Response::ResponseCode cmdCreateGame(const Command_CreateGame &cmd, Server_Room *room, ResponseContainer &rc);
    Response::ResponseCode cmdJoinGame(const Command_JoinGame &cmd, Server_Room *room, ResponseContainer &rc);
    
    Response::ResponseCode processSessionCommandContainer(const CommandContainer &cont, ResponseContainer &rc);
    virtual Response::ResponseCode processExtendedSessionCommand(int /* cmdType */, const SessionCommand & /* cmd */, ResponseContainer & /* rc */) { return Response::RespFunctionNotAllowed; }
    Response::ResponseCode processRoomCommandContainer(const CommandContainer &cont, ResponseContainer &rc);
    Response::ResponseCode processGameCommandContainer(const CommandContainer &cont, ResponseContainer &rc);
    Response::ResponseCode processModeratorCommandContainer(const CommandContainer &cont, ResponseContainer &rc);
    virtual Response::ResponseCode processExtendedModeratorCommand(int /* cmdType */, const ModeratorCommand & /* cmd */, ResponseContainer & /* rc */) { return Response::RespFunctionNotAllowed; }
    Response::ResponseCode processAdminCommandContainer(const CommandContainer &cont, ResponseContainer &rc);
    virtual Response::ResponseCode processExtendedAdminCommand(int /* cmdType */, const AdminCommand & /* cmd */, ResponseContainer & /* rc */) { return Response::RespFunctionNotAllowed; }
private slots:
    void pingClockTimeout();
public slots:
    void prepareDestroy();
public:
    Server_ProtocolHandler(Server *_server, Server_DatabaseInterface *_databaseInterface, QObject *parent = 0);
    ~Server_ProtocolHandler();
    
    bool getAcceptsUserListChanges() const { return acceptsUserListChanges; }
    bool getAcceptsRoomListChanges() const { return acceptsRoomListChanges; }
    virtual QString getAddress() const = 0;
    virtual QString getConnectionType() const = 0;
    Server_DatabaseInterface *getDatabaseInterface() const { return databaseInterface; }

    int getLastCommandTime() const { return timeRunning - lastDataReceived; }
    void processCommandContainer(const CommandContainer &cont);
    
    void sendProtocolItem(const Response &item);
    void sendProtocolItem(const SessionEvent &item);
    void sendProtocolItem(const GameEventContainer &item);
    void sendProtocolItem(const RoomEvent &item);

};

#endif
