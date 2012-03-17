#ifndef SERVER_PROTOCOLHANDLER_H
#define SERVER_PROTOCOLHANDLER_H

#include <QObject>
#include <QPair>
#include "server.h"
#include "server_abstractuserinterface.h"
#include "pb/response.pb.h"
#include "pb/server_message.pb.h"

class Server_Player;
class Server_Card;
class ServerInfo_User;
class Server_Room;
class QTimer;
class DeckList;

class Message;
class ServerMessage;

class Response;
class SessionEvent;
class GameEventContainer;
class RoomEvent;
class GameEventStorage;
class ResponseContainer;

class CommandContainer;
class Command_Ping;
class Command_Login;
class Command_Message;
class Command_ListUsers;
class Command_GetGamesOfUser;
class Command_GetUserInfo;
class Command_AddToList;
class Command_RemoveFromList;
class Command_DeckList;
class Command_DeckNewDir;
class Command_DeckDelDir;
class Command_DeckDel;
class Command_DeckDownload;
class Command_DeckUpload;
class Command_ReplayList;
class Command_ReplayDownload;
class Command_ReplayModifyMatch;
class Command_ListRooms;
class Command_JoinRoom;
class Command_LeaveRoom;
class Command_RoomSay;
class Command_CreateGame;
class Command_JoinGame;

class Command_BanFromServer;
class Command_UpdateServerMessage;
class Command_ShutdownServer;

class Server_ProtocolHandler : public QObject, public Server_AbstractUserInterface {
	Q_OBJECT
protected:
	QMap<int, QPair<int, int> > games; // gameId -> (roomId, playerId)
	QMap<int, Server_Room *> rooms;

	AuthenticationResult authState;
	bool acceptsUserListChanges;
	bool acceptsRoomListChanges;
	
	int sessionId;
private:
	QMutex gameListMutex;
	
	QList<int> messageSizeOverTime, messageCountOverTime;
	int timeRunning, lastDataReceived;
	QTimer *pingClock;

	virtual void transmitProtocolItem(const ServerMessage &item) = 0;

	Response::ResponseCode cmdPing(const Command_Ping &cmd, ResponseContainer &rc);
	Response::ResponseCode cmdLogin(const Command_Login &cmd, ResponseContainer &rc);
	Response::ResponseCode cmdMessage(const Command_Message &cmd, ResponseContainer &rc);
	virtual Response::ResponseCode cmdAddToList(const Command_AddToList &cmd, ResponseContainer &rc) = 0;
	virtual Response::ResponseCode cmdRemoveFromList(const Command_RemoveFromList &cmd, ResponseContainer &rc) = 0;
	virtual Response::ResponseCode cmdDeckList(const Command_DeckList &cmd, ResponseContainer &rc) = 0;
	virtual Response::ResponseCode cmdDeckNewDir(const Command_DeckNewDir &cmd, ResponseContainer &rc) = 0;
	virtual Response::ResponseCode cmdDeckDelDir(const Command_DeckDelDir &cmd, ResponseContainer &rc) = 0;
	virtual Response::ResponseCode cmdDeckDel(const Command_DeckDel &cmd, ResponseContainer &rc) = 0;
	virtual Response::ResponseCode cmdDeckUpload(const Command_DeckUpload &cmd, ResponseContainer &rc) = 0;
	virtual Response::ResponseCode cmdDeckDownload(const Command_DeckDownload &cmd, ResponseContainer &rc) = 0;
	virtual Response::ResponseCode cmdReplayList(const Command_ReplayList &cmd, ResponseContainer &rc) = 0;
	virtual Response::ResponseCode cmdReplayDownload(const Command_ReplayDownload &cmd, ResponseContainer &rc) = 0;
	virtual Response::ResponseCode cmdReplayModifyMatch(const Command_ReplayModifyMatch &cmd, ResponseContainer &rc) = 0;
	Response::ResponseCode cmdGetGamesOfUser(const Command_GetGamesOfUser &cmd, ResponseContainer &rc);
	Response::ResponseCode cmdGetUserInfo(const Command_GetUserInfo &cmd, ResponseContainer &rc);
	Response::ResponseCode cmdListRooms(const Command_ListRooms &cmd, ResponseContainer &rc);
	Response::ResponseCode cmdJoinRoom(const Command_JoinRoom &cmd, ResponseContainer &rc);
	Response::ResponseCode cmdListUsers(const Command_ListUsers &cmd, ResponseContainer &rc);
	Response::ResponseCode cmdLeaveRoom(const Command_LeaveRoom &cmd, Server_Room *room, ResponseContainer &rc);
	Response::ResponseCode cmdRoomSay(const Command_RoomSay &cmd, Server_Room *room, ResponseContainer &rc);
	Response::ResponseCode cmdCreateGame(const Command_CreateGame &cmd, Server_Room *room, ResponseContainer &rc);
	Response::ResponseCode cmdJoinGame(const Command_JoinGame &cmd, Server_Room *room, ResponseContainer &rc);
	virtual Response::ResponseCode cmdBanFromServer(const Command_BanFromServer &cmd, ResponseContainer &rc) = 0;
	virtual Response::ResponseCode cmdShutdownServer(const Command_ShutdownServer &cmd, ResponseContainer &rc) = 0;
	virtual Response::ResponseCode cmdUpdateServerMessage(const Command_UpdateServerMessage &cmd, ResponseContainer &rc) = 0;
	
	Response::ResponseCode processSessionCommandContainer(const CommandContainer &cont, ResponseContainer &rc);
	Response::ResponseCode processRoomCommandContainer(const CommandContainer &cont, ResponseContainer &rc);
	Response::ResponseCode processGameCommandContainer(const CommandContainer &cont, ResponseContainer &rc);
	Response::ResponseCode processModeratorCommandContainer(const CommandContainer &cont, ResponseContainer &rc);
	Response::ResponseCode processAdminCommandContainer(const CommandContainer &cont, ResponseContainer &rc);
private slots:
	void pingClockTimeout();
signals:
	void logDebugMessage(const QString &message, Server_ProtocolHandler *session);
public slots:
	void prepareDestroy();
public:
	Server_ProtocolHandler(Server *_server, QObject *parent = 0);
	~Server_ProtocolHandler();
	void playerRemovedFromGame(Server_Game *game);
	
	bool getAcceptsUserListChanges() const { return acceptsUserListChanges; }
	bool getAcceptsRoomListChanges() const { return acceptsRoomListChanges; }
	virtual QString getAddress() const = 0;
	int getSessionId() const { return sessionId; }
	void setSessionId(int _sessionId) { sessionId = _sessionId; }

	int getLastCommandTime() const { return timeRunning - lastDataReceived; }
	void processCommandContainer(const CommandContainer &cont);
	
	void sendProtocolItem(const Response &item);
	void sendProtocolItem(const SessionEvent &item);
	void sendProtocolItem(const GameEventContainer &item);
	void sendProtocolItem(const RoomEvent &item);
};

#endif
