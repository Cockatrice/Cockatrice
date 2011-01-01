#include <QDebug>
#include "rng_abstract.h"
#include "server_protocolhandler.h"
#include "protocol.h"
#include "protocol_items.h"
#include "server_chatchannel.h"
#include "server_card.h"
#include "server_arrow.h"
#include "server_cardzone.h"
#include "server_counter.h"
#include "server_game.h"
#include "server_player.h"
#include "decklist.h"
#include <QDateTime>

Server_ProtocolHandler::Server_ProtocolHandler(Server *_server, QObject *parent)
	: QObject(parent), server(_server), authState(PasswordWrong), acceptsGameListChanges(false), acceptsUserListChanges(false), acceptsChatChannelListChanges(false), userInfo(0), lastCommandTime(QDateTime::currentDateTime())
{
	connect(server, SIGNAL(pingClockTimeout()), this, SLOT(pingClockTimeout()));
}

Server_ProtocolHandler::~Server_ProtocolHandler()
{
	// The socket has to be removed from the server's list before it is removed from the game's list
	// so it will not receive the game update event.
	server->removeClient(this);

	QMapIterator<int, QPair<Server_Game *, Server_Player *> > gameIterator(games);
	while (gameIterator.hasNext()) {
		gameIterator.next();
		Server_Game *g = gameIterator.value().first;
		Server_Player *p = gameIterator.value().second;
		
		if (authState == UnknownUser)
			g->removePlayer(p);
		else
			p->setProtocolHandler(0);
	}

	QMapIterator<QString, Server_ChatChannel *> chatChannelIterator(chatChannels);
	while (chatChannelIterator.hasNext())
		chatChannelIterator.next().value()->removeClient(this);
	
	delete userInfo;
}

void Server_ProtocolHandler::playerRemovedFromGame(Server_Game *game)
{
	qDebug() << "Server_ProtocolHandler::playerRemovedFromGame(): gameId =" << game->getGameId();
	games.remove(game->getGameId());
}

ResponseCode Server_ProtocolHandler::processCommandHelper(Command *command, CommandContainer *cont)
{
	lastCommandTime = QDateTime::currentDateTime();

	ChatCommand *chatCommand = qobject_cast<ChatCommand *>(command);
	GameCommand *gameCommand = qobject_cast<GameCommand *>(command);
	if (chatCommand) {
		qDebug() << "received ChatCommand: channel =" << chatCommand->getChannel();
		if (authState == PasswordWrong)
			return RespLoginNeeded;
	
		Server_ChatChannel *channel = chatChannels.value(chatCommand->getChannel(), 0);
		if (!channel)
			return RespNameNotFound;
		
		switch (command->getItemId()) {
			case ItemId_Command_ChatLeaveChannel: return cmdChatLeaveChannel(qobject_cast<Command_ChatLeaveChannel *>(command), cont, channel);
			case ItemId_Command_ChatSay: return cmdChatSay(qobject_cast<Command_ChatSay *>(command), cont, channel);
		}
	} else if (gameCommand) {
		qDebug() << "received GameCommand: game =" << gameCommand->getGameId();
		if (authState == PasswordWrong)
			return RespLoginNeeded;
	
		if (!games.contains(gameCommand->getGameId())) {
			qDebug() << "invalid game";
			return RespNameNotFound;
		}
		QPair<Server_Game *, Server_Player *> gamePair = games.value(gameCommand->getGameId());
		Server_Game *game = gamePair.first;
		Server_Player *player = gamePair.second;
		
		switch (command->getItemId()) {
			case ItemId_Command_DeckSelect: return cmdDeckSelect(qobject_cast<Command_DeckSelect *>(command), cont, game, player);
			case ItemId_Command_SetSideboardPlan: return cmdSetSideboardPlan(qobject_cast<Command_SetSideboardPlan *>(command), cont, game, player);
			case ItemId_Command_LeaveGame: return cmdLeaveGame(qobject_cast<Command_LeaveGame *>(command), cont, game, player);
			case ItemId_Command_ReadyStart: return cmdReadyStart(qobject_cast<Command_ReadyStart *>(command), cont, game, player);
			case ItemId_Command_Concede: return cmdConcede(qobject_cast<Command_Concede *>(command), cont, game, player);
			case ItemId_Command_Say: return cmdSay(qobject_cast<Command_Say *>(command), cont, game, player);
			case ItemId_Command_Shuffle: return cmdShuffle(qobject_cast<Command_Shuffle *>(command), cont, game, player);
			case ItemId_Command_Mulligan: return cmdMulligan(qobject_cast<Command_Mulligan *>(command), cont, game, player);
			case ItemId_Command_RollDie: return cmdRollDie(qobject_cast<Command_RollDie *>(command), cont, game, player);
			case ItemId_Command_DrawCards: return cmdDrawCards(qobject_cast<Command_DrawCards *>(command), cont, game, player);
			case ItemId_Command_MoveCard: return cmdMoveCard(qobject_cast<Command_MoveCard *>(command), cont, game, player);
			case ItemId_Command_FlipCard: return cmdFlipCard(qobject_cast<Command_FlipCard *>(command), cont, game, player);
			case ItemId_Command_AttachCard: return cmdAttachCard(qobject_cast<Command_AttachCard *>(command), cont, game, player);
			case ItemId_Command_CreateToken: return cmdCreateToken(qobject_cast<Command_CreateToken *>(command), cont, game, player);
			case ItemId_Command_CreateArrow: return cmdCreateArrow(qobject_cast<Command_CreateArrow *>(command), cont, game, player);
			case ItemId_Command_DeleteArrow: return cmdDeleteArrow(qobject_cast<Command_DeleteArrow *>(command), cont, game, player);
			case ItemId_Command_SetCardAttr: return cmdSetCardAttr(qobject_cast<Command_SetCardAttr *>(command), cont, game, player);
			case ItemId_Command_SetCardCounter: return cmdSetCardCounter(qobject_cast<Command_SetCardCounter *>(command), cont, game, player);
			case ItemId_Command_IncCardCounter: return cmdIncCardCounter(qobject_cast<Command_IncCardCounter *>(command), cont, game, player);
			case ItemId_Command_IncCounter: return cmdIncCounter(qobject_cast<Command_IncCounter *>(command), cont, game, player);
			case ItemId_Command_CreateCounter: return cmdCreateCounter(qobject_cast<Command_CreateCounter *>(command), cont, game, player);
			case ItemId_Command_SetCounter: return cmdSetCounter(qobject_cast<Command_SetCounter *>(command), cont, game, player);
			case ItemId_Command_DelCounter: return cmdDelCounter(qobject_cast<Command_DelCounter *>(command), cont, game, player);
			case ItemId_Command_NextTurn: return cmdNextTurn(qobject_cast<Command_NextTurn *>(command), cont, game, player);
			case ItemId_Command_SetActivePhase: return cmdSetActivePhase(qobject_cast<Command_SetActivePhase *>(command), cont, game, player);
			case ItemId_Command_DumpZone: return cmdDumpZone(qobject_cast<Command_DumpZone *>(command), cont, game, player);
			case ItemId_Command_StopDumpZone: return cmdStopDumpZone(qobject_cast<Command_StopDumpZone *>(command), cont, game, player);
			case ItemId_Command_RevealCards: return cmdRevealCards(qobject_cast<Command_RevealCards *>(command), cont, game, player);
		}
	} else {
		qDebug() << "received generic Command";
		switch (command->getItemId()) {
			case ItemId_Command_Ping: return cmdPing(qobject_cast<Command_Ping *>(command), cont);
			case ItemId_Command_Login: return cmdLogin(qobject_cast<Command_Login *>(command), cont);
			case ItemId_Command_Message: return cmdMessage(qobject_cast<Command_Message *>(command), cont);
			case ItemId_Command_DeckList: return cmdDeckList(qobject_cast<Command_DeckList *>(command), cont);
			case ItemId_Command_DeckNewDir: return cmdDeckNewDir(qobject_cast<Command_DeckNewDir *>(command), cont);
			case ItemId_Command_DeckDelDir: return cmdDeckDelDir(qobject_cast<Command_DeckDelDir *>(command), cont);
			case ItemId_Command_DeckDel: return cmdDeckDel(qobject_cast<Command_DeckDel *>(command), cont);
			case ItemId_Command_DeckUpload: return cmdDeckUpload(qobject_cast<Command_DeckUpload *>(command), cont);
			case ItemId_Command_DeckDownload: return cmdDeckDownload(qobject_cast<Command_DeckDownload *>(command), cont);
			case ItemId_Command_GetUserInfo: return cmdGetUserInfo(qobject_cast<Command_GetUserInfo *>(command), cont);
			case ItemId_Command_ListChatChannels: return cmdListChatChannels(qobject_cast<Command_ListChatChannels *>(command), cont);
			case ItemId_Command_ChatJoinChannel: return cmdChatJoinChannel(qobject_cast<Command_ChatJoinChannel *>(command), cont);
			case ItemId_Command_ListUsers: return cmdListUsers(qobject_cast<Command_ListUsers *>(command), cont);
			case ItemId_Command_ListGames: return cmdListGames(qobject_cast<Command_ListGames *>(command), cont);
			case ItemId_Command_CreateGame: return cmdCreateGame(qobject_cast<Command_CreateGame *>(command), cont);
			case ItemId_Command_JoinGame: return cmdJoinGame(qobject_cast<Command_JoinGame *>(command), cont);
		}
	}
	return RespInvalidCommand;
}

void Server_ProtocolHandler::processCommandContainer(CommandContainer *cont)
{
	const QList<Command *> &cmdList = cont->getCommandList();
	ResponseCode finalResponseCode = RespOk;
	for (int i = 0; i < cmdList.size(); ++i) {
		ResponseCode resp = processCommandHelper(cmdList[i], cont);
		if ((resp != RespOk) && (resp != RespNothing))
			finalResponseCode = resp;
	}
	
	ProtocolResponse *pr = cont->getResponse();
	if (!pr)
		pr = new ProtocolResponse(cont->getCmdId(), finalResponseCode);

	GameEventContainer *gQPublic = cont->getGameEventQueuePublic();
	if (gQPublic) {
		Server_Game *game = games.value(gQPublic->getGameId()).first;
		Server_Player *player = games.value(gQPublic->getGameId()).second;
		GameEventContainer *gQPrivate = cont->getGameEventQueuePrivate();
		GameEventContainer *gQOmniscient = cont->getGameEventQueueOmniscient();
		if (gQPrivate) {
			int privatePlayerId = cont->getPrivatePlayerId();
			Server_Player *privatePlayer;
			if (privatePlayerId == -1)
				privatePlayer = player;
			else
				privatePlayer = game->getPlayer(privatePlayerId);
			if (gQOmniscient) {
				game->sendGameEventContainer(gQPublic, privatePlayer, true);
				game->sendGameEventContainerOmniscient(gQOmniscient, privatePlayer);
			} else
				game->sendGameEventContainer(gQPublic, privatePlayer);
			privatePlayer->sendProtocolItem(gQPrivate);
		} else
			game->sendGameEventContainer(gQPublic);
	}
	
	const QList<ProtocolItem *> &iQ = cont->getItemQueue();
	for (int i = 0; i < iQ.size(); ++i)
		sendProtocolItem(iQ[i]);
	
	sendProtocolItem(pr);
	
	while (!itemQueue.isEmpty())
		sendProtocolItem(itemQueue.takeFirst());

	if (cont->getReceiverMayDelete())
		delete cont;
}

void Server_ProtocolHandler::pingClockTimeout()
{
	if (lastCommandTime.secsTo(QDateTime::currentDateTime()) > server->getMaxPlayerInactivityTime())
		deleteLater();
}

void Server_ProtocolHandler::enqueueProtocolItem(ProtocolItem *item)
{
	itemQueue.append(item);
}

QPair<Server_Game *, Server_Player *> Server_ProtocolHandler::getGame(int gameId) const
{
	if (games.contains(gameId))
		return games.value(gameId);
	return QPair<Server_Game *, Server_Player *>(0, 0);
}

ResponseCode Server_ProtocolHandler::cmdPing(Command_Ping * /*cmd*/, CommandContainer * /*cont*/)
{
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdLogin(Command_Login *cmd, CommandContainer *cont)
{
	QString userName = cmd->getUsername().simplified();
	if (userName.isEmpty() || (userInfo != 0))
		return RespContextError;
	authState = server->loginUser(this, userName, cmd->getPassword());
	if (authState == PasswordWrong)
		return RespWrongPassword;

	enqueueProtocolItem(new Event_ServerMessage(server->getLoginMessage()));

	if (authState == PasswordRight) {
		// This might not scale very well. Use an extra QMap if it becomes a problem.
		const QList<Server_Game *> &serverGames = server->getGames();
		for (int i = 0; i < serverGames.size(); ++i) {
			const QList<Server_Player *> &gamePlayers = serverGames[i]->getPlayers().values();
			for (int j = 0; j < gamePlayers.size(); ++j)
				if (gamePlayers[j]->getUserInfo()->getName() == userInfo->getName()) {
					gamePlayers[j]->setProtocolHandler(this);
					games.insert(serverGames[i]->getGameId(), QPair<Server_Game *, Server_Player *>(serverGames[i], gamePlayers[j]));
					
					enqueueProtocolItem(new Event_GameJoined(serverGames[i]->getGameId(), serverGames[i]->getDescription(), gamePlayers[j]->getPlayerId(), gamePlayers[j]->getSpectator(), serverGames[i]->getSpectatorsCanTalk(), serverGames[i]->getSpectatorsSeeEverything(), true));
					enqueueProtocolItem(GameEventContainer::makeNew(new Event_GameStateChanged(serverGames[i]->getGameStarted(), serverGames[i]->getActivePlayer(), serverGames[i]->getActivePhase(), serverGames[i]->getGameState(gamePlayers[j])), serverGames[i]->getGameId()));
				}
		}
	}

	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdMessage(Command_Message *cmd, CommandContainer *cont)
{
	if (authState == PasswordWrong)
		return RespLoginNeeded;
	
	QString receiver = cmd->getUserName();
	Server_ProtocolHandler *userHandler = server->getUsers().value(receiver);
	if (!userHandler)
		return RespNameNotFound;
	
	cont->enqueueItem(new Event_Message(userInfo->getName(), receiver, cmd->getText()));
	userHandler->sendProtocolItem(new Event_Message(userInfo->getName(), receiver, cmd->getText()));
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdGetUserInfo(Command_GetUserInfo *cmd, CommandContainer *cont)
{
	if (authState == PasswordWrong)
		return RespLoginNeeded;
	
	ServerInfo_User *result;
	if (cmd->getUserName().isEmpty())
		result = new ServerInfo_User(userInfo);
	else {
		Server_ProtocolHandler *handler = server->getUsers().value(cmd->getUserName());
		if (!handler)
			return RespNameNotFound;
		result = handler->getUserInfo();
	}
		
	cont->setResponse(new Response_GetUserInfo(cont->getCmdId(), RespOk, result));
	return RespNothing;
}

ResponseCode Server_ProtocolHandler::cmdListChatChannels(Command_ListChatChannels * /*cmd*/, CommandContainer *cont)
{
	if (authState == PasswordWrong)
		return RespLoginNeeded;
	
	QList<ServerInfo_ChatChannel *> eventChannelList;
	QMapIterator<QString, Server_ChatChannel *> channelIterator(server->getChatChannels());
	while (channelIterator.hasNext()) {
		Server_ChatChannel *c = channelIterator.next().value();
		eventChannelList.append(new ServerInfo_ChatChannel(c->getName(), c->getDescription(), c->size(), c->getAutoJoin()));
	}
	cont->enqueueItem(new Event_ListChatChannels(eventChannelList));
	
	acceptsChatChannelListChanges = true;
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdChatJoinChannel(Command_ChatJoinChannel *cmd, CommandContainer *cont)
{
	if (authState == PasswordWrong)
		return RespLoginNeeded;
	
	if (chatChannels.contains(cmd->getChannel()))
		return RespContextError;
	
	QMap<QString, Server_ChatChannel *> allChannels = server->getChatChannels();
	Server_ChatChannel *c = allChannels.value(cmd->getChannel(), 0);
	if (!c)
		return RespNameNotFound;

	c->addClient(this);
	chatChannels.insert(cmd->getChannel(), c);
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdChatLeaveChannel(Command_ChatLeaveChannel * /*cmd*/, CommandContainer *cont, Server_ChatChannel *channel)
{
	chatChannels.remove(channel->getName());
	channel->removeClient(this);
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdChatSay(Command_ChatSay *cmd, CommandContainer *cont, Server_ChatChannel *channel)
{
	channel->say(this, cmd->getMessage());
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdListUsers(Command_ListUsers * /*cmd*/, CommandContainer *cont)
{
	if (authState == PasswordWrong)
		return RespLoginNeeded;
	
	QList<ServerInfo_User *> resultList;
	QMapIterator<QString, Server_ProtocolHandler *> userIterator = server->getUsers();
	while (userIterator.hasNext())
		resultList.append(new ServerInfo_User(userIterator.next().value()->getUserInfo()));
	
	acceptsUserListChanges = true;
	
	cont->setResponse(new Response_ListUsers(cont->getCmdId(), RespOk, resultList));
	return RespNothing;
}

ResponseCode Server_ProtocolHandler::cmdListGames(Command_ListGames * /*cmd*/, CommandContainer *cont)
{
	if (authState == PasswordWrong)
		return RespLoginNeeded;
	
	const QList<Server_Game *> &gameList = server->getGames();
	QList<ServerInfo_Game *> eventGameList;
	for (int i = 0; i < gameList.size(); ++i) {
		Server_Game *g = gameList[i];
		eventGameList.append(new ServerInfo_Game(
			g->getGameId(),
			g->getDescription(),
			!g->getPassword().isEmpty(),
			g->getPlayerCount(),
			g->getMaxPlayers(),
			new ServerInfo_User(g->getCreatorInfo()),
			g->getSpectatorsAllowed(),
			g->getSpectatorsNeedPassword(),
			g->getSpectatorCount()
		));
	}
	cont->enqueueItem(new Event_ListGames(eventGameList));
	
	acceptsGameListChanges = true;
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdCreateGame(Command_CreateGame *cmd, CommandContainer *cont)
{
	if (authState == PasswordWrong)
		return RespLoginNeeded;
	
	Server_Game *game = server->createGame(cmd->getDescription(), cmd->getPassword(), cmd->getMaxPlayers(), cmd->getSpectatorsAllowed(), cmd->getSpectatorsNeedPassword(), cmd->getSpectatorsCanTalk(), cmd->getSpectatorsSeeEverything(), this);
	Server_Player *creator = game->getPlayers().values().first();
	games.insert(game->getGameId(), QPair<Server_Game *, Server_Player *>(game, creator));
	
	enqueueProtocolItem(new Event_GameJoined(game->getGameId(), game->getDescription(), creator->getPlayerId(), false, game->getSpectatorsCanTalk(), game->getSpectatorsSeeEverything(), false));
	enqueueProtocolItem(GameEventContainer::makeNew(new Event_GameStateChanged(game->getGameStarted(), game->getActivePlayer(), game->getActivePhase(), game->getGameState(creator)), game->getGameId()));
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdJoinGame(Command_JoinGame *cmd, CommandContainer *cont)
{
	if (authState == PasswordWrong)
		return RespLoginNeeded;
	
	if (games.contains(cmd->getGameId()))
		return RespContextError;
	
	Server_Game *g = server->getGame(cmd->getGameId());
	if (!g)
		return RespNameNotFound;
		
	ResponseCode result = g->checkJoin(cmd->getPassword(), cmd->getSpectator());
	if (result == RespOk) {
		Server_Player *player = g->addPlayer(this, cmd->getSpectator());
		games.insert(cmd->getGameId(), QPair<Server_Game *, Server_Player *>(g, player));
		enqueueProtocolItem(new Event_GameJoined(cmd->getGameId(), g->getDescription(), player->getPlayerId(), cmd->getSpectator(), g->getSpectatorsCanTalk(), g->getSpectatorsSeeEverything(), false));
		enqueueProtocolItem(GameEventContainer::makeNew(new Event_GameStateChanged(g->getGameStarted(), g->getActivePlayer(), g->getActivePhase(), g->getGameState(player)), cmd->getGameId()));
	}
	return result;
}

ResponseCode Server_ProtocolHandler::cmdLeaveGame(Command_LeaveGame * /*cmd*/, CommandContainer *cont, Server_Game *game, Server_Player *player)
{
	game->removePlayer(player);
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdDeckSelect(Command_DeckSelect *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	DeckList *deck;
	if (cmd->getDeckId() == -1) {
		if (!cmd->getDeck())
			return RespInvalidData;
		deck = new DeckList(cmd->getDeck());
	} else {
		try {
			deck = getDeckFromDatabase(cmd->getDeckId());
		} catch(ResponseCode r) {
			return r;
		}
	}
	player->setDeck(deck, cmd->getDeckId());
	
	game->sendGameEvent(new Event_PlayerPropertiesChanged(player->getPlayerId(), player->getProperties()), new Context_DeckSelect(cmd->getDeckId()));

	cont->setResponse(new Response_DeckDownload(cont->getCmdId(), RespOk, new DeckList(deck)));
	return RespNothing;
}

ResponseCode Server_ProtocolHandler::cmdSetSideboardPlan(Command_SetSideboardPlan *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	if (player->getReadyStart())
		return RespContextError;
	
	DeckList *deck = player->getDeck();
	if (!deck)
		return RespContextError;
	
	deck->setCurrentSideboardPlan(cmd->getMoveList());
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdConcede(Command_Concede * /*cmd*/, CommandContainer *cont, Server_Game *game, Server_Player *player)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	player->setConceded(true);
	game->sendGameEvent(new Event_PlayerPropertiesChanged(player->getPlayerId(), player->getProperties()), new Context_Concede);
	game->stopGameIfFinished();
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdReadyStart(Command_ReadyStart *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!player->getDeck())
		return RespContextError;

	if (player->getReadyStart() == cmd->getReady())
		return RespContextError;
	
	player->setReadyStart(cmd->getReady());
	game->sendGameEvent(new Event_PlayerPropertiesChanged(player->getPlayerId(), player->getProperties()), new Context_ReadyStart);
	game->startGameIfReady();
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdSay(Command_Say *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player)
{
	if (player->getSpectator() && !game->getSpectatorsCanTalk())
		return RespFunctionNotAllowed;
	
	game->sendGameEvent(new Event_Say(player->getPlayerId(), cmd->getMessage()));
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdShuffle(Command_Shuffle * /*cmd*/, CommandContainer *cont, Server_Game *game, Server_Player *player)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
		
	player->getZones().value("deck")->shuffle();
	game->sendGameEvent(new Event_Shuffle(player->getPlayerId()));
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdMulligan(Command_Mulligan * /*cmd*/, CommandContainer *cont, Server_Game *game, Server_Player *player)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
	
	Server_CardZone *hand = player->getZones().value("hand");
	int number = (hand->cards.size() <= 1) ? player->getInitialCards() : hand->cards.size() - 1;
		
	while (!hand->cards.isEmpty())
		player->moveCard(cont, "hand", hand->cards.first()->getId(), "deck", 0, 0, false, false);

	player->getZones().value("deck")->shuffle();
	cont->enqueueGameEventPrivate(new Event_Shuffle(player->getPlayerId()), game->getGameId());
	cont->enqueueGameEventPublic(new Event_Shuffle(player->getPlayerId()), game->getGameId());

	drawCards(game, player, cont, number);

	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdRollDie(Command_RollDie *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	game->sendGameEvent(new Event_RollDie(player->getPlayerId(), cmd->getSides(), rng->getNumber(1, cmd->getSides())));
	return RespOk;
}

ResponseCode Server_ProtocolHandler::drawCards(Server_Game *game, Server_Player *player, CommandContainer *cont, int number)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
		
	Server_CardZone *deck = player->getZones().value("deck");
	Server_CardZone *hand = player->getZones().value("hand");
	if (deck->cards.size() < number)
		number = deck->cards.size();

	QList<ServerInfo_Card *> cardListPrivate;
	QList<ServerInfo_Card *> cardListOmniscient;
	for (int i = 0; i < number; ++i) {
		Server_Card *card = deck->cards.takeFirst();
		hand->cards.append(card);
		cardListPrivate.append(new ServerInfo_Card(card->getId(), card->getName()));
		cardListOmniscient.append(new ServerInfo_Card(card->getId(), card->getName()));
	}
	cont->enqueueGameEventPrivate(new Event_DrawCards(player->getPlayerId(), cardListPrivate.size(), cardListPrivate), game->getGameId());
	cont->enqueueGameEventOmniscient(new Event_DrawCards(player->getPlayerId(), cardListOmniscient.size(), cardListOmniscient), game->getGameId());
	cont->enqueueGameEventPublic(new Event_DrawCards(player->getPlayerId(), cardListPrivate.size()), game->getGameId());

	return RespOk;
}


ResponseCode Server_ProtocolHandler::cmdDrawCards(Command_DrawCards *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player)
{
	return drawCards(game, player, cont, cmd->getNumber());
}


ResponseCode Server_ProtocolHandler::cmdMoveCard(Command_MoveCard *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
		
	return player->moveCard(cont, cmd->getStartZone(), cmd->getCardId(), cmd->getTargetZone(), cmd->getX(), cmd->getY(), cmd->getFaceDown(), cmd->getTapped());
}

ResponseCode Server_ProtocolHandler::cmdFlipCard(Command_FlipCard *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
		
	Server_CardZone *zone = player->getZones().value(cmd->getZone());
	if (!zone)
		return RespNameNotFound;
	if (!zone->hasCoords())
		return RespContextError;
	
	Server_Card *card = zone->getCard(cmd->getCardId(), false);
	if (!card)
		return RespNameNotFound;
	
	const bool faceDown = cmd->getFaceDown();
	if (faceDown == card->getFaceDown())
		return RespContextError;
	
	card->setFaceDown(faceDown);
	cont->enqueueGameEventPrivate(new Event_FlipCard(player->getPlayerId(), zone->getName(), card->getId(), card->getName(), faceDown), game->getGameId());
	cont->enqueueGameEventPublic(new Event_FlipCard(player->getPlayerId(), zone->getName(), card->getId(), card->getName(), faceDown), game->getGameId());
	
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdAttachCard(Command_AttachCard *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
		
	Server_CardZone *startzone = player->getZones().value(cmd->getStartZone());
	if (!startzone)
		return RespNameNotFound;
	
	Server_Card *card = startzone->getCard(cmd->getCardId(), false);
	if (!card)
		return RespNameNotFound;

	int playerId = cmd->getTargetPlayerId();
	Server_Player *targetPlayer = 0;
	Server_CardZone *targetzone = 0;
	Server_Card *targetCard = 0;
	
	if (playerId != -1) {
		targetPlayer = game->getPlayer(cmd->getTargetPlayerId());
		if (!targetPlayer)
			return RespNameNotFound;
	} else if (!card->getParentCard())
		return RespContextError;
	if (targetPlayer)
		targetzone = targetPlayer->getZones().value(cmd->getTargetZone());
	if (targetzone) {
		// This is currently enough to make sure cards don't get attached to a card that is not on the table.
		// Possibly a flag will have to be introduced for this sometime.
		if (!targetzone->hasCoords())
			return RespContextError;
		targetCard = targetzone->getCard(cmd->getTargetCardId(), false);
		if (targetCard)
			if (targetCard->getParentCard())
				return RespContextError;
	}
	if (!startzone->hasCoords())
		return RespContextError;
	
	// Get all arrows pointing to or originating from the card being attached and delete them.
	QMapIterator<int, Server_Player *> playerIterator(game->getPlayers());
	while (playerIterator.hasNext()) {
		Server_Player *p = playerIterator.next().value();
		QList<Server_Arrow *> arrows = p->getArrows().values();
		QList<Server_Arrow *> toDelete;
		for (int i = 0; i < arrows.size(); ++i) {
			Server_Arrow *a = arrows[i];
			Server_Card *tCard = qobject_cast<Server_Card *>(a->getTargetItem());
			if ((tCard == card) || (a->getStartCard() == card))
				toDelete.append(a);
		}
		for (int i = 0; i < toDelete.size(); ++i) {
			cont->enqueueGameEventPrivate(new Event_DeleteArrow(p->getPlayerId(), toDelete[i]->getId()), game->getGameId());
			cont->enqueueGameEventPublic(new Event_DeleteArrow(p->getPlayerId(), toDelete[i]->getId()), game->getGameId());
			p->deleteArrow(toDelete[i]->getId());
		}
	}

	if (targetCard) {
		// Unattach all cards attached to the card being attached.
		const QList<Server_Card *> &attachedList = card->getAttachedCards();
		for (int i = 0; i < attachedList.size(); ++i)
			player->unattachCard(cont, attachedList[i]);
		
		if (targetzone->isColumnStacked(targetCard->getX(), targetCard->getY()))
			targetPlayer->moveCard(cont, targetzone, targetCard->getId(), targetzone, targetzone->getFreeGridColumn(-2, targetCard->getY(), targetCard->getName()), targetCard->getY(), targetCard->getFaceDown(), false);
		
		card->setParentCard(targetCard);
		card->setCoords(-1, card->getY());
		cont->enqueueGameEventPrivate(new Event_AttachCard(player->getPlayerId(), startzone->getName(), card->getId(), targetPlayer->getPlayerId(), targetzone->getName(), targetCard->getId()), game->getGameId());
		cont->enqueueGameEventPublic(new Event_AttachCard(player->getPlayerId(), startzone->getName(), card->getId(), targetPlayer->getPlayerId(), targetzone->getName(), targetCard->getId()), game->getGameId());
	} else
		player->unattachCard(cont, card);
	
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdCreateToken(Command_CreateToken *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
		
	Server_CardZone *zone = player->getZones().value(cmd->getZone());
	if (!zone)
		return RespNameNotFound;

	int x = cmd->getX();
	int y = cmd->getY();
	if (zone->hasCoords())
		x = zone->getFreeGridColumn(x, y, cmd->getCardName());
	if (x < 0)
		x = 0;
	if (y < 0)
		y = 0;

	Server_Card *card = new Server_Card(cmd->getCardName(), player->newCardId(), x, y);
	card->setPT(cmd->getPt());
	card->setColor(cmd->getColor());
	card->setAnnotation(cmd->getAnnotation());
	card->setDestroyOnZoneChange(cmd->getDestroy());
	
	zone->insertCard(card, x, y);
	game->sendGameEvent(new Event_CreateToken(player->getPlayerId(), zone->getName(), card->getId(), card->getName(), cmd->getColor(), cmd->getPt(), cmd->getAnnotation(), cmd->getDestroy(), x, y));
	
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdCreateArrow(Command_CreateArrow *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
		
	Server_Player *startPlayer = game->getPlayer(cmd->getStartPlayerId());
	Server_Player *targetPlayer = game->getPlayer(cmd->getTargetPlayerId());
	if (!startPlayer || !targetPlayer)
		return RespNameNotFound;
	Server_CardZone *startZone = startPlayer->getZones().value(cmd->getStartZone());
	bool playerTarget = cmd->getTargetZone().isEmpty();
	Server_CardZone *targetZone = 0;
	if (!playerTarget)
		targetZone = targetPlayer->getZones().value(cmd->getTargetZone());
	if (!startZone || (!targetZone && !playerTarget))
		return RespNameNotFound;
	if (startZone->getType() != PublicZone)
		return RespContextError;
	Server_Card *startCard = startZone->getCard(cmd->getStartCardId(), false);
	if (!startCard)
		return RespNameNotFound;
	Server_Card *targetCard = 0;
	if (!playerTarget) {
		if (targetZone->getType() != PublicZone)
			return RespContextError;
		targetCard = targetZone->getCard(cmd->getTargetCardId(), false);
	}
	
	Server_ArrowTarget *targetItem;
	if (playerTarget)
		targetItem = targetPlayer;
	else
		targetItem = targetCard;
	if (!targetItem)
		return RespNameNotFound;

	QMapIterator<int, Server_Arrow *> arrowIterator(player->getArrows());
	while (arrowIterator.hasNext()) {
		Server_Arrow *temp = arrowIterator.next().value();
		if ((temp->getStartCard() == startCard) && (temp->getTargetItem() == targetItem))
			return RespContextError;
	}
	
	Server_Arrow *arrow = new Server_Arrow(player->newArrowId(), startCard, targetItem, cmd->getColor());
	player->addArrow(arrow);
	game->sendGameEvent(new Event_CreateArrows(player->getPlayerId(), QList<ServerInfo_Arrow *>() << new ServerInfo_Arrow(
		arrow->getId(),
		startPlayer->getPlayerId(),
		startZone->getName(),
		startCard->getId(),
		targetPlayer->getPlayerId(),
		cmd->getTargetZone(),
		cmd->getTargetCardId(),
		cmd->getColor()
	)));
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdDeleteArrow(Command_DeleteArrow *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
		
	if (!player->deleteArrow(cmd->getArrowId()))
		return RespNameNotFound;
	
	game->sendGameEvent(new Event_DeleteArrow(player->getPlayerId(), cmd->getArrowId()));
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdSetCardAttr(Command_SetCardAttr *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
		
	return player->setCardAttrHelper(cont, cmd->getZone(), cmd->getCardId(), cmd->getAttrName(), cmd->getAttrValue());
}

ResponseCode Server_ProtocolHandler::cmdSetCardCounter(Command_SetCardCounter *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
		
	Server_CardZone *zone = player->getZones().value(cmd->getZone());
	if (!zone)
		return RespNameNotFound;
	if (!zone->hasCoords())
		return RespContextError;

	Server_Card *card = zone->getCard(cmd->getCardId(), false);
	if (!card)
		return RespNameNotFound;
	
	card->setCounter(cmd->getCounterId(), cmd->getCounterValue());
	
	cont->enqueueGameEventPrivate(new Event_SetCardCounter(player->getPlayerId(), zone->getName(), card->getId(), cmd->getCounterId(), cmd->getCounterValue()), game->getGameId());
	cont->enqueueGameEventPublic(new Event_SetCardCounter(player->getPlayerId(), zone->getName(), card->getId(), cmd->getCounterId(), cmd->getCounterValue()), game->getGameId());
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdIncCardCounter(Command_IncCardCounter *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
		
	Server_CardZone *zone = player->getZones().value(cmd->getZone());
	if (!zone)
		return RespNameNotFound;
	if (!zone->hasCoords())
		return RespContextError;

	Server_Card *card = zone->getCard(cmd->getCardId(), false);
	if (!card)
		return RespNameNotFound;
	
	int newValue = card->getCounter(cmd->getCounterId()) + cmd->getCounterDelta();
	card->setCounter(cmd->getCounterId(), newValue);
	
	cont->enqueueGameEventPrivate(new Event_SetCardCounter(player->getPlayerId(), zone->getName(), card->getId(), cmd->getCounterId(), newValue), game->getGameId());
	cont->enqueueGameEventPublic(new Event_SetCardCounter(player->getPlayerId(), zone->getName(), card->getId(), cmd->getCounterId(), newValue), game->getGameId());
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdIncCounter(Command_IncCounter *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
		
	const QMap<int, Server_Counter *> counters = player->getCounters();
	Server_Counter *c = counters.value(cmd->getCounterId(), 0);
	if (!c)
		return RespNameNotFound;
	
	c->setCount(c->getCount() + cmd->getDelta());
	game->sendGameEvent(new Event_SetCounter(player->getPlayerId(), c->getId(), c->getCount()));
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdCreateCounter(Command_CreateCounter *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
		
	Server_Counter *c = new Server_Counter(player->newCounterId(), cmd->getCounterName(), cmd->getColor(), cmd->getRadius(), cmd->getValue());
	player->addCounter(c);
	game->sendGameEvent(new Event_CreateCounters(player->getPlayerId(), QList<ServerInfo_Counter *>() << new ServerInfo_Counter(c->getId(), c->getName(), c->getColor(), c->getRadius(), c->getCount())));
	
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdSetCounter(Command_SetCounter *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
		
	Server_Counter *c = player->getCounters().value(cmd->getCounterId(), 0);;
	if (!c)
		return RespNameNotFound;
	
	c->setCount(cmd->getValue());
	game->sendGameEvent(new Event_SetCounter(player->getPlayerId(), c->getId(), c->getCount()));
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdDelCounter(Command_DelCounter *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
		
	if (!player->deleteCounter(cmd->getCounterId()))
		return RespNameNotFound;
	game->sendGameEvent(new Event_DelCounter(player->getPlayerId(), cmd->getCounterId()));
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdNextTurn(Command_NextTurn * /*cmd*/, CommandContainer *cont, Server_Game *game, Server_Player *player)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
	
	game->nextTurn();
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdSetActivePhase(Command_SetActivePhase *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
		
	if (game->getActivePlayer() != player->getPlayerId())
		return RespContextError;
	game->setActivePhase(cmd->getPhase());
	
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdDumpZone(Command_DumpZone *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player)
{
	if (!game->getGameStarted())
		return RespGameNotStarted;
		
	Server_Player *otherPlayer = game->getPlayer(cmd->getPlayerId());
	if (!otherPlayer)
		return RespNameNotFound;
	Server_CardZone *zone = otherPlayer->getZones().value(cmd->getZoneName());
	if (!zone)
		return RespNameNotFound;
	if (!((zone->getType() == PublicZone) || (player == otherPlayer)))
		return RespContextError;
	
	int numberCards = cmd->getNumberCards();
	QList<ServerInfo_Card *> respCardList;
	for (int i = 0; (i < zone->cards.size()) && (i < numberCards || numberCards == -1); ++i) {
		Server_Card *card = zone->cards[i];
		QString displayedName = card->getFaceDown() ? QString() : card->getName();
		if (zone->getType() == HiddenZone)
			respCardList.append(new ServerInfo_Card(i, displayedName));
		else {
			QList<ServerInfo_CardCounter *> cardCounterList;
			QMapIterator<int, int> cardCounterIterator(card->getCounters());
			while (cardCounterIterator.hasNext()) {
				cardCounterIterator.next();
				cardCounterList.append(new ServerInfo_CardCounter(cardCounterIterator.key(), cardCounterIterator.value()));
			}

			int attachPlayerId = -1;
			QString attachZone;
			int attachCardId = -1;
			if (card->getParentCard()) {
				attachPlayerId = card->getParentCard()->getZone()->getPlayer()->getPlayerId();
				attachZone = card->getParentCard()->getZone()->getName();
				attachCardId = card->getParentCard()->getId();
			}
			
			respCardList.append(new ServerInfo_Card(card->getId(), displayedName, card->getX(), card->getY(), card->getTapped(), card->getAttacking(), card->getColor(), card->getPT(), card->getAnnotation(), card->getDestroyOnZoneChange(), cardCounterList, attachPlayerId, attachZone, attachCardId));
		}
	}
	if (zone->getType() == HiddenZone) {
		zone->setCardsBeingLookedAt(numberCards);
		game->sendGameEvent(new Event_DumpZone(player->getPlayerId(), otherPlayer->getPlayerId(), zone->getName(), numberCards));
	}
	cont->setResponse(new Response_DumpZone(cont->getCmdId(), RespOk, new ServerInfo_Zone(zone->getName(), zone->getType(), zone->hasCoords(), numberCards < zone->cards.size() ? zone->cards.size() : numberCards, respCardList)));
	return RespNothing;
}

ResponseCode Server_ProtocolHandler::cmdStopDumpZone(Command_StopDumpZone *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player)
{
	if (!game->getGameStarted())
		return RespGameNotStarted;
		
	Server_Player *otherPlayer = game->getPlayer(cmd->getPlayerId());
	if (!otherPlayer)
		return RespNameNotFound;
	Server_CardZone *zone = otherPlayer->getZones().value(cmd->getZoneName());
	if (!zone)
		return RespNameNotFound;
	
	if (zone->getType() == HiddenZone) {
		zone->setCardsBeingLookedAt(0);
		game->sendGameEvent(new Event_StopDumpZone(player->getPlayerId(), cmd->getPlayerId(), zone->getName()));
	}
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdRevealCards(Command_RevealCards *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
	
	Server_Player *otherPlayer = 0;
	if (cmd->getPlayerId() != -1) {
		otherPlayer = game->getPlayer(cmd->getPlayerId());
		if (!otherPlayer)
			return RespNameNotFound;
	}
	Server_CardZone *zone = player->getZones().value(cmd->getZoneName());
	if (!zone)
		return RespNameNotFound;
	
	QList<Server_Card *> cardsToReveal;
	if (cmd->getCardId() == -1)
		cardsToReveal = zone->cards;
	else if (cmd->getCardId() == -2) {
		if (zone->cards.isEmpty())
			return RespContextError;
		cardsToReveal.append(zone->cards.at(rng->getNumber(0, zone->cards.size() - 1)));
	} else {
		Server_Card *card = zone->getCard(cmd->getCardId(), false);
		if (!card)
			return RespNameNotFound;
		cardsToReveal.append(card);
	}
	
	QList<ServerInfo_Card *> respCardListPrivate, respCardListOmniscient;
	for (int i = 0; i < cardsToReveal.size(); ++i) {
		Server_Card *card = cardsToReveal[i];

		QList<ServerInfo_CardCounter *> cardCounterListPrivate, cardCounterListOmniscient;
		QMapIterator<int, int> cardCounterIterator(card->getCounters());
		while (cardCounterIterator.hasNext()) {
			cardCounterIterator.next();
			cardCounterListPrivate.append(new ServerInfo_CardCounter(cardCounterIterator.key(), cardCounterIterator.value()));
			cardCounterListOmniscient.append(new ServerInfo_CardCounter(cardCounterIterator.key(), cardCounterIterator.value()));
		}
		
		int attachPlayerId = -1;
		QString attachZone;
		int attachCardId = -1;
		if (card->getParentCard()) {
			attachPlayerId = card->getParentCard()->getZone()->getPlayer()->getPlayerId();
			attachZone = card->getParentCard()->getZone()->getName();
			attachCardId = card->getParentCard()->getId();
		}
		
		if (cmd->getPlayerId() != -1)
			respCardListPrivate.append(new ServerInfo_Card(card->getId(), card->getName(), card->getX(), card->getY(), card->getTapped(), card->getAttacking(), card->getColor(), card->getPT(), card->getAnnotation(), card->getDestroyOnZoneChange(), cardCounterListPrivate, attachPlayerId, attachZone, attachCardId));
		respCardListOmniscient.append(new ServerInfo_Card(card->getId(), card->getName(), card->getX(), card->getY(), card->getTapped(), card->getAttacking(), card->getColor(), card->getPT(), card->getAnnotation(), card->getDestroyOnZoneChange(), cardCounterListOmniscient, attachPlayerId, attachZone, attachCardId));
	}
	
	if (cmd->getPlayerId() == -1)
		cont->enqueueGameEventPublic(new Event_RevealCards(player->getPlayerId(), zone->getName(), cmd->getCardId(), -1, respCardListOmniscient), game->getGameId());
	else {
		cont->enqueueGameEventPublic(new Event_RevealCards(player->getPlayerId(), zone->getName(), cmd->getCardId(), otherPlayer->getPlayerId()), game->getGameId());
		cont->enqueueGameEventPrivate(new Event_RevealCards(player->getPlayerId(), zone->getName(), cmd->getCardId(), otherPlayer->getPlayerId(), respCardListPrivate), game->getGameId(), otherPlayer->getPlayerId());
		cont->enqueueGameEventOmniscient(new Event_RevealCards(player->getPlayerId(), zone->getName(), cmd->getCardId(), otherPlayer->getPlayerId(), respCardListOmniscient), game->getGameId());
	}
	
	return RespOk;
}
