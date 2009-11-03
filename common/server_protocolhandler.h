#ifndef SERVER_PROTOCOLHANDLER_H
#define SERVER_PROTOCOLHANDLER_H

#include <QObject>
#include <QPair>
#include "server.h"
#include "protocol.h"
#include "protocol_items.h"

class Server_Player;

class Server_ProtocolHandler : public QObject {
	Q_OBJECT
private:
	Server *server;
	QMap<int, QPair<Server_Game *, Server_Player *> > games;
	QMap<QString, Server_ChatChannel *> chatChannels;
	QString playerName;
	
	Server *getServer() const { return server; }
	QPair<Server_Game *, Server_Player *> getGame(int gameId) const;

	AuthenticationResult authState;
	bool acceptsGameListChanges;
	bool acceptsChatChannelListChanges;
	
	QList<ProtocolItem *> itemQueue;
	
	ProtocolResponse::ResponseCode cmdPing(Command_Ping *cmd);
	ProtocolResponse::ResponseCode cmdLogin(Command_Login *cmd);
	ProtocolResponse::ResponseCode cmdChatListChannels(Command_ChatListChannels *cmd);
	ProtocolResponse::ResponseCode cmdChatJoinChannel(Command_ChatJoinChannel *cmd);
	ProtocolResponse::ResponseCode cmdChatLeaveChannel(Command_ChatLeaveChannel *cmd, Server_ChatChannel *channel);
	ProtocolResponse::ResponseCode cmdChatSay(Command_ChatSay *cmd, Server_ChatChannel *channel);
	ProtocolResponse::ResponseCode cmdListGames(Command_ListGames *cmd);
	ProtocolResponse::ResponseCode cmdCreateGame(Command_CreateGame *cmd);
	ProtocolResponse::ResponseCode cmdJoinGame(Command_JoinGame *cmd);
	ProtocolResponse::ResponseCode cmdLeaveGame(Command_LeaveGame *cmd, Server_Game *game, Server_Player *player);
	ProtocolResponse::ResponseCode cmdSay(Command_Say *cmd, Server_Game *game, Server_Player *player);
	ProtocolResponse::ResponseCode cmdShuffle(Command_Shuffle *cmd, Server_Game *game, Server_Player *player);
	ProtocolResponse::ResponseCode cmdRollDie(Command_RollDie *cmd, Server_Game *game, Server_Player *player);
	ProtocolResponse::ResponseCode cmdDrawCards(Command_DrawCards *cmd, Server_Game *game, Server_Player *player);
	ProtocolResponse::ResponseCode cmdMoveCard(Command_MoveCard *cmd, Server_Game *game, Server_Player *player);
	ProtocolResponse::ResponseCode cmdCreateToken(Command_CreateToken *cmd, Server_Game *game, Server_Player *player);
	ProtocolResponse::ResponseCode cmdCreateArrow(Command_CreateArrow *cmd, Server_Game *game, Server_Player *player);
	ProtocolResponse::ResponseCode cmdDeleteArrow(Command_DeleteArrow *cmd, Server_Game *game, Server_Player *player);
	ProtocolResponse::ResponseCode cmdSetCardAttr(Command_SetCardAttr *cmd, Server_Game *game, Server_Player *player);
	ProtocolResponse::ResponseCode cmdReadyStart(Command_ReadyStart *cmd, Server_Game *game, Server_Player *player);
	ProtocolResponse::ResponseCode cmdIncCounter(Command_IncCounter *cmd, Server_Game *game, Server_Player *player);
	ProtocolResponse::ResponseCode cmdAddCounter(Command_AddCounter *cmd, Server_Game *game, Server_Player *player);
	ProtocolResponse::ResponseCode cmdSetCounter(Command_SetCounter *cmd, Server_Game *game, Server_Player *player);
	ProtocolResponse::ResponseCode cmdDelCounter(Command_DelCounter *cmd, Server_Game *game, Server_Player *player);
	ProtocolResponse::ResponseCode cmdNextTurn(Command_NextTurn *cmd, Server_Game *game, Server_Player *player);
	ProtocolResponse::ResponseCode cmdSetActivePhase(Command_SetActivePhase *cmd, Server_Game *game, Server_Player *player);
	ProtocolResponse::ResponseCode cmdDumpZone(Command_DumpZone *cmd, Server_Game *game, Server_Player *player);
	ProtocolResponse::ResponseCode cmdStopDumpZone(Command_StopDumpZone *cmd, Server_Game *game, Server_Player *player);
	ProtocolResponse::ResponseCode cmdDumpAll(Command_DumpAll *cmd, Server_Game *game, Server_Player *player);
	ProtocolResponse::ResponseCode cmdSubmitDeck(Command_SubmitDeck *cmd, Server_Game *game, Server_Player *player);
public:
	Server_ProtocolHandler(Server *_server, QObject *parent = 0);
	~Server_ProtocolHandler();
	
	bool getAcceptsGameListChanges() const { return acceptsGameListChanges; }
	bool getAcceptsChatChannelListChanges() const { return acceptsChatChannelListChanges; }
	const QString &getPlayerName() const { return playerName; }
	
	void processCommand(Command *command);
	virtual void sendProtocolItem(ProtocolItem *item, bool deleteItem = true) = 0;
	void enqueueProtocolItem(ProtocolItem *item);
};

#endif
