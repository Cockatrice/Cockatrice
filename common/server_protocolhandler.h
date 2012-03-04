#ifndef SERVER_PROTOCOLHANDLER_H
#define SERVER_PROTOCOLHANDLER_H

#include <QObject>
#include <QPair>
#include "server.h"
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

class Command_KickFromGame;
class Command_LeaveGame;
class Command_GameSay;
class Command_Shuffle;
class Command_Mulligan;
class Command_RollDie;
class Command_DrawCards;
class Command_UndoDraw;
class Command_FlipCard;
class Command_AttachCard;
class Command_CreateToken;
class Command_CreateArrow;
class Command_DeleteArrow;
class Command_SetCardAttr;
class Command_SetCardCounter;
class Command_IncCardCounter;
class Command_ReadyStart;
class Command_Concede;
class Command_IncCounter;
class Command_CreateCounter;
class Command_SetCounter;
class Command_DelCounter;
class Command_NextTurn;
class Command_SetActivePhase;
class Command_DumpZone;
class Command_StopDumpZone;
class Command_RevealCards;
class Command_MoveCard;
class Command_SetSideboardPlan;
class Command_DeckSelect;

class Command_BanFromServer;
class Command_UpdateServerMessage;
class Command_ShutdownServer;

class Server_ProtocolHandler : public QObject {
	Q_OBJECT
protected:
	Server *server;
	QMap<int, QPair<Server_Game *, Server_Player *> > games;
	QMap<int, Server_Room *> rooms;

	Server *getServer() const { return server; }
	QPair<Server_Game *, Server_Player *> getGame(int gameId) const;

	AuthenticationResult authState;
	bool acceptsUserListChanges;
	bool acceptsRoomListChanges;
	ServerInfo_User *userInfo;
	
	void prepareDestroy();
	int sessionId;
private:
	QString thisUserName;
	QList<int> messageSizeOverTime, messageCountOverTime;
	int timeRunning, lastDataReceived;
	QTimer *pingClock;

	virtual void transmitProtocolItem(const ServerMessage &item) = 0;

	virtual DeckList *getDeckFromDatabase(int deckId) = 0;

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
	Response::ResponseCode cmdLeaveGame(const Command_LeaveGame &cmd, Server_Game *game, Server_Player *player, ResponseContainer &rc, GameEventStorage &ges);
	Response::ResponseCode cmdKickFromGame(const Command_KickFromGame &cmd, Server_Game *game, Server_Player *player, ResponseContainer &rc, GameEventStorage &ges);
	Response::ResponseCode cmdConcede(const Command_Concede &cmd, Server_Game *game, Server_Player *player, ResponseContainer &rc, GameEventStorage &ges);
	Response::ResponseCode cmdReadyStart(const Command_ReadyStart &cmd, Server_Game *game, Server_Player *player, ResponseContainer &rc, GameEventStorage &ges);
	Response::ResponseCode cmdDeckSelect(const Command_DeckSelect &cmd, Server_Game *game, Server_Player *player, ResponseContainer &rc, GameEventStorage &ges);
	Response::ResponseCode cmdSetSideboardPlan(const Command_SetSideboardPlan &cmd, Server_Game *game, Server_Player *player, ResponseContainer &rc, GameEventStorage &ges);
	Response::ResponseCode cmdGameSay(const Command_GameSay &cmd, Server_Game *game, Server_Player *player, ResponseContainer &rc, GameEventStorage &ges);
	Response::ResponseCode cmdShuffle(const Command_Shuffle &cmd, Server_Game *game, Server_Player *player, ResponseContainer &rc, GameEventStorage &ges);
	Response::ResponseCode cmdMulligan(const Command_Mulligan &cmd, Server_Game *game, Server_Player *player, ResponseContainer &rc, GameEventStorage &ges);
	Response::ResponseCode cmdRollDie(const Command_RollDie &cmd, Server_Game *game, Server_Player *player, ResponseContainer &rc, GameEventStorage &ges);
	Response::ResponseCode cmdDrawCards(const Command_DrawCards &cmd, Server_Game *game, Server_Player *player, ResponseContainer &rc, GameEventStorage &ges);
	Response::ResponseCode cmdUndoDraw(const Command_UndoDraw &cmd, Server_Game *game, Server_Player *player, ResponseContainer &rc, GameEventStorage &ges);
	Response::ResponseCode cmdMoveCard(const Command_MoveCard &cmd, Server_Game *game, Server_Player *player, ResponseContainer &rc, GameEventStorage &ges);
	Response::ResponseCode cmdFlipCard(const Command_FlipCard &cmd, Server_Game *game, Server_Player *player, ResponseContainer &rc, GameEventStorage &ges);
	Response::ResponseCode cmdAttachCard(const Command_AttachCard &cmd, Server_Game *game, Server_Player *player, ResponseContainer &rc, GameEventStorage &ges);
	Response::ResponseCode cmdCreateToken(const Command_CreateToken &cmd, Server_Game *game, Server_Player *player, ResponseContainer &rc, GameEventStorage &ges);
	Response::ResponseCode cmdCreateArrow(const Command_CreateArrow &cmd, Server_Game *game, Server_Player *player, ResponseContainer &rc, GameEventStorage &ges);
	Response::ResponseCode cmdDeleteArrow(const Command_DeleteArrow &cmd, Server_Game *game, Server_Player *player, ResponseContainer &rc, GameEventStorage &ges);
	Response::ResponseCode cmdSetCardAttr(const Command_SetCardAttr &cmd, Server_Game *game, Server_Player *player, ResponseContainer &rc, GameEventStorage &ges);
	Response::ResponseCode cmdSetCardCounter(const Command_SetCardCounter &cmd, Server_Game *game, Server_Player *player, ResponseContainer &rc, GameEventStorage &ges);
	Response::ResponseCode cmdIncCardCounter(const Command_IncCardCounter &cmd, Server_Game *game, Server_Player *player, ResponseContainer &rc, GameEventStorage &ges);
	Response::ResponseCode cmdIncCounter(const Command_IncCounter &cmd, Server_Game *game, Server_Player *player, ResponseContainer &rc, GameEventStorage &ges);
	Response::ResponseCode cmdCreateCounter(const Command_CreateCounter &cmd, Server_Game *game, Server_Player *player, ResponseContainer &rc, GameEventStorage &ges);
	Response::ResponseCode cmdSetCounter(const Command_SetCounter &cmd, Server_Game *game, Server_Player *player, ResponseContainer &rc, GameEventStorage &ges);
	Response::ResponseCode cmdDelCounter(const Command_DelCounter &cmd, Server_Game *game, Server_Player *player, ResponseContainer &rc, GameEventStorage &ges);
	Response::ResponseCode cmdNextTurn(const Command_NextTurn &cmd, Server_Game *game, Server_Player *player, ResponseContainer &rc, GameEventStorage &ges);
	Response::ResponseCode cmdSetActivePhase(const Command_SetActivePhase &cmd, Server_Game *game, Server_Player *player, ResponseContainer &rc, GameEventStorage &ges);
	Response::ResponseCode cmdDumpZone(const Command_DumpZone &cmd, Server_Game *game, Server_Player *player, ResponseContainer &rc, GameEventStorage &ges);
	Response::ResponseCode cmdStopDumpZone(const Command_StopDumpZone &cmd, Server_Game *game, Server_Player *player, ResponseContainer &rc, GameEventStorage &ges);
	Response::ResponseCode cmdRevealCards(const Command_RevealCards &cmd, Server_Game *game, Server_Player *player, ResponseContainer &rc, GameEventStorage &ges);
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
public:
	QMutex gameListMutex;
	
	Server_ProtocolHandler(Server *_server, QObject *parent = 0);
	~Server_ProtocolHandler();
	void playerRemovedFromGame(Server_Game *game);
	
	bool getAcceptsUserListChanges() const { return acceptsUserListChanges; }
	bool getAcceptsRoomListChanges() const { return acceptsRoomListChanges; }
	ServerInfo_User *getUserInfo() const { return userInfo; }
	ServerInfo_User copyUserInfo(bool complete, bool moderatorInfo = false) const;
	const QString &getUserName() const { return thisUserName; }
	virtual QString getAddress() const = 0;
	void setUserInfo(const ServerInfo_User &_userInfo);
	int getSessionId() const { return sessionId; }
	void setSessionId(int _sessionId) { sessionId = _sessionId; }

	int getLastCommandTime() const { return timeRunning - lastDataReceived; }
	void processCommandContainer(const CommandContainer &cont);
	
	void sendProtocolItem(const Response &item);
	void sendProtocolItem(const SessionEvent &item);
	void sendProtocolItem(const GameEventContainer &item);
	void sendProtocolItem(const RoomEvent &item);
	void sendProtocolItem(ServerMessage::MessageType type, const ::google::protobuf::Message &item);
	
	static SessionEvent *prepareSessionEvent(const ::google::protobuf::Message &sessionEvent);
};

#endif
