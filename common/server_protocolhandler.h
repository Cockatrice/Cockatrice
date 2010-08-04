#ifndef SERVER_PROTOCOLHANDLER_H
#define SERVER_PROTOCOLHANDLER_H

#include <QObject>
#include <QPair>
#include "server.h"
#include "protocol.h"
#include "protocol_items.h"

class Server_Player;
class Server_Card;
class QTimer;

class Server_ProtocolHandler : public QObject {
	Q_OBJECT
protected:
	Server *server;
	QMap<int, QPair<Server_Game *, Server_Player *> > games;
	QMap<QString, Server_ChatChannel *> chatChannels;
	QString playerName;

	Server *getServer() const { return server; }
	QPair<Server_Game *, Server_Player *> getGame(int gameId) const;

	AuthenticationResult authState;
	bool acceptsGameListChanges;
	bool acceptsChatChannelListChanges;
	
private:
	QList<ProtocolItem *> itemQueue;
	QDateTime lastCommandTime;
	QTimer *pingClock;

	virtual DeckList *getDeckFromDatabase(int deckId) = 0;

	ResponseCode cmdPing(Command_Ping *cmd, CommandContainer *cont);
	ResponseCode cmdLogin(Command_Login *cmd, CommandContainer *cont);
	virtual ResponseCode cmdDeckList(Command_DeckList *cmd, CommandContainer *cont) = 0;
	virtual ResponseCode cmdDeckNewDir(Command_DeckNewDir *cmd, CommandContainer *cont) = 0;
	virtual ResponseCode cmdDeckDelDir(Command_DeckDelDir *cmd, CommandContainer *cont) = 0;
	virtual ResponseCode cmdDeckDel(Command_DeckDel *cmd, CommandContainer *cont) = 0;
	virtual ResponseCode cmdDeckUpload(Command_DeckUpload *cmd, CommandContainer *cont) = 0;
	virtual ResponseCode cmdDeckDownload(Command_DeckDownload *cmd, CommandContainer *cont) = 0;
	ResponseCode cmdListChatChannels(Command_ListChatChannels *cmd, CommandContainer *cont);
	ResponseCode cmdChatJoinChannel(Command_ChatJoinChannel *cmd, CommandContainer *cont);
	ResponseCode cmdChatLeaveChannel(Command_ChatLeaveChannel *cmd, CommandContainer *cont, Server_ChatChannel *channel);
	ResponseCode cmdChatSay(Command_ChatSay *cmd, CommandContainer *cont, Server_ChatChannel *channel);
	ResponseCode cmdListGames(Command_ListGames *cmd, CommandContainer *cont);
	ResponseCode cmdCreateGame(Command_CreateGame *cmd, CommandContainer *cont);
	ResponseCode cmdJoinGame(Command_JoinGame *cmd, CommandContainer *cont);
	ResponseCode cmdLeaveGame(Command_LeaveGame *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player);
	ResponseCode cmdConcede(Command_Concede *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player);
	ResponseCode cmdReadyStart(Command_ReadyStart *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player);
	ResponseCode cmdDeckSelect(Command_DeckSelect *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player);
	ResponseCode cmdSetSideboardPlan(Command_SetSideboardPlan *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player);
	ResponseCode cmdSay(Command_Say *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player);
	ResponseCode cmdShuffle(Command_Shuffle *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player);
	ResponseCode cmdMulligan(Command_Mulligan *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player);
	ResponseCode cmdRollDie(Command_RollDie *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player);
	// XXX Maybe the following function and others belong into Server_Player
	ResponseCode drawCards(Server_Game *game, Server_Player *player, CommandContainer *cont, int number);
	ResponseCode cmdDrawCards(Command_DrawCards *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player);
	ResponseCode moveCard(Server_Game *game, Server_Player *player, CommandContainer *cont, const QString &_startZone, int _cardId, const QString &_targetZone, int _x, int _y, bool _faceDown, bool _tapped);
	ResponseCode cmdMoveCard(Command_MoveCard *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player);
	void unattachCard(Server_Game *game, Server_Player *player, CommandContainer *cont, Server_Card *card);
	ResponseCode cmdAttachCard(Command_AttachCard *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player);
	ResponseCode cmdCreateToken(Command_CreateToken *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player);
	ResponseCode cmdCreateArrow(Command_CreateArrow *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player);
	ResponseCode cmdDeleteArrow(Command_DeleteArrow *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player);
	ResponseCode setCardAttrHelper(CommandContainer *cont, Server_Game *game, Server_Player *player, const QString &zone, int cardId, const QString &attrName, const QString &attrValue);
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
	
	ResponseCode processCommandHelper(Command *command, CommandContainer *cont);
private slots:
	void pingClockTimeout();
public:
	Server_ProtocolHandler(Server *_server, QObject *parent = 0);
	~Server_ProtocolHandler();
	void playerRemovedFromGame(Server_Game *game);
	
	bool getAcceptsGameListChanges() const { return acceptsGameListChanges; }
	bool getAcceptsChatChannelListChanges() const { return acceptsChatChannelListChanges; }
	const QString &getPlayerName() const { return playerName; }
	const QDateTime &getLastCommandTime() const { return lastCommandTime; }

	void processCommandContainer(CommandContainer *cont);
	virtual bool sendProtocolItem(ProtocolItem *item, bool deleteItem = true) = 0;
	void enqueueProtocolItem(ProtocolItem *item);
};

#endif
