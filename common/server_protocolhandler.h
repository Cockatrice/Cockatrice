#ifndef SERVER_PROTOCOLHANDLER_H
#define SERVER_PROTOCOLHANDLER_H

#include <QObject>
#include <QPair>
#include "server.h"
#include "protocol.h"
#include "protocol_items.h"

class Server_Player;
class Server_Card;
class ServerInfo_User;
class Server_Room;
class QTimer;

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
	
private:
	QList<ProtocolItem *> itemQueue;
	QDateTime lastCommandTime;
	QTimer *pingClock;

	virtual DeckList *getDeckFromDatabase(int deckId) = 0;

	ResponseCode cmdPing(Command_Ping *cmd, CommandContainer *cont);
	ResponseCode cmdLogin(Command_Login *cmd, CommandContainer *cont);
	ResponseCode cmdMessage(Command_Message *cmd, CommandContainer *cont);
	virtual ResponseCode cmdDeckList(Command_DeckList *cmd, CommandContainer *cont) = 0;
	virtual ResponseCode cmdDeckNewDir(Command_DeckNewDir *cmd, CommandContainer *cont) = 0;
	virtual ResponseCode cmdDeckDelDir(Command_DeckDelDir *cmd, CommandContainer *cont) = 0;
	virtual ResponseCode cmdDeckDel(Command_DeckDel *cmd, CommandContainer *cont) = 0;
	virtual ResponseCode cmdDeckUpload(Command_DeckUpload *cmd, CommandContainer *cont) = 0;
	virtual ResponseCode cmdDeckDownload(Command_DeckDownload *cmd, CommandContainer *cont) = 0;
	ResponseCode cmdGetUserInfo(Command_GetUserInfo *cmd, CommandContainer *cont);
	ResponseCode cmdListRooms(Command_ListRooms *cmd, CommandContainer *cont);
	ResponseCode cmdJoinRoom(Command_JoinRoom *cmd, CommandContainer *cont);
	ResponseCode cmdLeaveRoom(Command_LeaveRoom *cmd, CommandContainer *cont, Server_Room *room);
	ResponseCode cmdRoomSay(Command_RoomSay *cmd, CommandContainer *cont, Server_Room *room);
	ResponseCode cmdListUsers(Command_ListUsers *cmd, CommandContainer *cont);
	ResponseCode cmdCreateGame(Command_CreateGame *cmd, CommandContainer *cont, Server_Room *room);
	ResponseCode cmdJoinGame(Command_JoinGame *cmd, CommandContainer *cont, Server_Room *room);
	ResponseCode cmdLeaveGame(Command_LeaveGame *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player);
	ResponseCode cmdConcede(Command_Concede *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player);
	ResponseCode cmdReadyStart(Command_ReadyStart *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player);
	ResponseCode cmdDeckSelect(Command_DeckSelect *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player);
	ResponseCode cmdSetSideboardPlan(Command_SetSideboardPlan *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player);
	ResponseCode cmdSay(Command_Say *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player);
	ResponseCode cmdShuffle(Command_Shuffle *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player);
	ResponseCode cmdMulligan(Command_Mulligan *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player);
	ResponseCode cmdRollDie(Command_RollDie *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player);
	ResponseCode cmdDrawCards(Command_DrawCards *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player);
	ResponseCode cmdUndoDraw(Command_UndoDraw *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player);
	ResponseCode cmdMoveCard(Command_MoveCard *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player);
	ResponseCode cmdFlipCard(Command_FlipCard *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player);
	ResponseCode cmdAttachCard(Command_AttachCard *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player);
	ResponseCode cmdCreateToken(Command_CreateToken *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player);
	ResponseCode cmdCreateArrow(Command_CreateArrow *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player);
	ResponseCode cmdDeleteArrow(Command_DeleteArrow *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player);
	ResponseCode cmdSetCardAttr(Command_SetCardAttr *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player);
	ResponseCode cmdSetCardCounter(Command_SetCardCounter *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player);
	ResponseCode cmdIncCardCounter(Command_IncCardCounter *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player);
	ResponseCode cmdIncCounter(Command_IncCounter *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player);
	ResponseCode cmdCreateCounter(Command_CreateCounter *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player);
	ResponseCode cmdSetCounter(Command_SetCounter *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player);
	ResponseCode cmdDelCounter(Command_DelCounter *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player);
	ResponseCode cmdNextTurn(Command_NextTurn *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player);
	ResponseCode cmdSetActivePhase(Command_SetActivePhase *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player);
	ResponseCode cmdDumpZone(Command_DumpZone *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player);
	ResponseCode cmdStopDumpZone(Command_StopDumpZone *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player);
	ResponseCode cmdRevealCards(Command_RevealCards *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player);
	virtual ResponseCode cmdUpdateServerMessage(Command_UpdateServerMessage *cmd, CommandContainer *cont) = 0;
	
	ResponseCode processCommandHelper(Command *command, CommandContainer *cont);
private slots:
	void pingClockTimeout();
public:
	Server_ProtocolHandler(Server *_server, QObject *parent = 0);
	~Server_ProtocolHandler();
	void playerRemovedFromGame(Server_Game *game);
	
	bool getAcceptsUserListChanges() const { return acceptsUserListChanges; }
	bool getAcceptsRoomListChanges() const { return acceptsRoomListChanges; }
	ServerInfo_User *getUserInfo() const { return userInfo; }
	void setUserInfo(ServerInfo_User *_userInfo) { userInfo = _userInfo; }
	const QDateTime &getLastCommandTime() const { return lastCommandTime; }

	void processCommandContainer(CommandContainer *cont);
	virtual void sendProtocolItem(ProtocolItem *item, bool deleteItem = true) = 0;
	void enqueueProtocolItem(ProtocolItem *item);
};

#endif
