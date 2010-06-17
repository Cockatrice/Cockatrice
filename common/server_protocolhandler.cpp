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
#include <QTimer>

Server_ProtocolHandler::Server_ProtocolHandler(Server *_server, QObject *parent)
	: QObject(parent), server(_server), authState(PasswordWrong), acceptsGameListChanges(false), lastCommandTime(QDateTime::currentDateTime())
{
	pingClock = new QTimer(this);
	connect(pingClock, SIGNAL(timeout()), this, SLOT(pingClockTimeout()));
	pingClock->start(1000);
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
		}
	} else {
		qDebug() << "received generic Command";
		switch (command->getItemId()) {
			case ItemId_Command_Ping: return cmdPing(qobject_cast<Command_Ping *>(command), cont);
			case ItemId_Command_Login: return cmdLogin(qobject_cast<Command_Login *>(command), cont);
			case ItemId_Command_DeckList: return cmdDeckList(qobject_cast<Command_DeckList *>(command), cont);
			case ItemId_Command_DeckNewDir: return cmdDeckNewDir(qobject_cast<Command_DeckNewDir *>(command), cont);
			case ItemId_Command_DeckDelDir: return cmdDeckDelDir(qobject_cast<Command_DeckDelDir *>(command), cont);
			case ItemId_Command_DeckDel: return cmdDeckDel(qobject_cast<Command_DeckDel *>(command), cont);
			case ItemId_Command_DeckUpload: return cmdDeckUpload(qobject_cast<Command_DeckUpload *>(command), cont);
			case ItemId_Command_DeckDownload: return cmdDeckDownload(qobject_cast<Command_DeckDownload *>(command), cont);
			case ItemId_Command_ListChatChannels: return cmdListChatChannels(qobject_cast<Command_ListChatChannels *>(command), cont);
			case ItemId_Command_ChatJoinChannel: return cmdChatJoinChannel(qobject_cast<Command_ChatJoinChannel *>(command), cont);
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
			if (gQOmniscient) {
				game->sendGameEventContainer(gQPublic, player, true);
				game->sendGameEventContainerOmniscient(gQOmniscient, player);
			} else
				game->sendGameEventContainer(gQPublic, player);
			player->sendProtocolItem(gQPrivate);
		} else
			game->sendGameEventContainer(gQPublic);
	}
	
	const QList<ProtocolItem *> &iQ = cont->getItemQueue();
	for (int i = 0; i < iQ.size(); ++i)
		sendProtocolItem(iQ[i]);
	
	sendProtocolItem(pr);
	
	delete cont;
	
	while (!itemQueue.isEmpty())
		sendProtocolItem(itemQueue.takeFirst());
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
	if (userName.isEmpty())
		return RespContextError;
	authState = server->checkUserPassword(userName, cmd->getPassword());
	if (authState == PasswordWrong)
		return RespWrongPassword;
	if (authState == PasswordRight)
		server->closeOldSession(userName);

	playerName = userName;
	enqueueProtocolItem(new Event_ServerMessage(server->getLoginMessage()));

	if (authState == PasswordRight) {
		// This might not scale very well. Use an extra QMap if it becomes a problem.
		const QList<Server_Game *> &serverGames = server->getGames();
		for (int i = 0; i < serverGames.size(); ++i) {
			const QList<Server_Player *> &gamePlayers = serverGames[i]->getPlayers().values();
			for (int j = 0; j < gamePlayers.size(); ++j)
				if (gamePlayers[j]->getPlayerName() == playerName) {
					gamePlayers[j]->setProtocolHandler(this);
					games.insert(serverGames[i]->getGameId(), QPair<Server_Game *, Server_Player *>(serverGames[i], gamePlayers[j]));
					
					enqueueProtocolItem(new Event_GameJoined(serverGames[i]->getGameId(), serverGames[i]->getDescription(), gamePlayers[j]->getPlayerId(), gamePlayers[j]->getSpectator(), serverGames[i]->getSpectatorsCanTalk(), serverGames[i]->getSpectatorsSeeEverything(), true));
					enqueueProtocolItem(GameEventContainer::makeNew(new Event_GameStateChanged(serverGames[i]->getGameStarted(), serverGames[i]->getActivePlayer(), serverGames[i]->getActivePhase(), serverGames[i]->getGameState(gamePlayers[j])), serverGames[i]->getGameId()));
				}
		}
	}

	return RespOk;
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

ResponseCode Server_ProtocolHandler::cmdListGames(Command_ListGames * /*cmd*/, CommandContainer *cont)
{
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
			g->getCreatorName(),
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
	Server_Game *game = server->createGame(cmd->getDescription(), cmd->getPassword(), cmd->getMaxPlayers(), cmd->getSpectatorsAllowed(), cmd->getSpectatorsNeedPassword(), cmd->getSpectatorsCanTalk(), cmd->getSpectatorsSeeEverything(), this);
	Server_Player *creator = game->getCreator();
	games.insert(game->getGameId(), QPair<Server_Game *, Server_Player *>(game, creator));
	
	enqueueProtocolItem(new Event_GameJoined(game->getGameId(), game->getDescription(), creator->getPlayerId(), false, game->getSpectatorsCanTalk(), game->getSpectatorsSeeEverything(), false));
	enqueueProtocolItem(GameEventContainer::makeNew(new Event_GameStateChanged(game->getGameStarted(), game->getActivePlayer(), game->getActivePhase(), game->getGameState(creator)), game->getGameId()));
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdJoinGame(Command_JoinGame *cmd, CommandContainer *cont)
{
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
	
	game->sendGameEvent(new Event_PlayerPropertiesChanged(player->getProperties()), new Context_DeckSelect(cmd->getDeckId()));

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
	game->sendGameEvent(new Event_PlayerPropertiesChanged(player->getProperties()), new Context_Concede);
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
	game->sendGameEvent(new Event_PlayerPropertiesChanged(player->getProperties()), new Context_ReadyStart);
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
		
	int number = player->getInitialCards();
	if (!number)
		return RespContextError;

	Server_CardZone *hand = player->getZones().value("hand");
	while (!hand->cards.isEmpty())
		moveCard(game, player, cont, "hand", hand->cards.first()->getId(), "deck", 0, 0, false, false);

	player->getZones().value("deck")->shuffle();
	cont->enqueueGameEventPrivate(new Event_Shuffle(player->getPlayerId()), game->getGameId());
	cont->enqueueGameEventPublic(new Event_Shuffle(player->getPlayerId()), game->getGameId());

	drawCards(game, player, cont, number);
	player->setInitialCards(number - 1);

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

ResponseCode Server_ProtocolHandler::moveCard(Server_Game *game, Server_Player *player, CommandContainer *cont, const QString &_startZone, int _cardId, const QString &_targetZone, int x, int y, bool faceDown, bool tapped)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
		
	Server_CardZone *startzone = player->getZones().value(_startZone);
	Server_CardZone *targetzone = player->getZones().value(_targetZone);
	if ((!startzone) || (!targetzone))
		return RespNameNotFound;

	int position = -1;
	Server_Card *card = startzone->getCard(_cardId, true, &position);
	if (!card)
		return RespNameNotFound;
	if (x == -1)
		x = targetzone->cards.size();
	if (!targetzone->hasCoords())
		y = 0;

	targetzone->insertCard(card, x, y);

	bool targetBeingLookedAt = (targetzone->getType() != HiddenZone) || (targetzone->getCardsBeingLookedAt() > x) || (targetzone->getCardsBeingLookedAt() == -1);
	bool sourceBeingLookedAt = (startzone->getType() != HiddenZone) || (startzone->getCardsBeingLookedAt() > position) || (startzone->getCardsBeingLookedAt() == -1);

	bool targetHiddenToPlayer = faceDown || !targetBeingLookedAt;
	bool targetHiddenToOthers = faceDown || (targetzone->getType() != PublicZone);
	bool sourceHiddenToPlayer = card->getFaceDown() || !sourceBeingLookedAt;
	bool sourceHiddenToOthers = card->getFaceDown() || (startzone->getType() != PublicZone);

	QString privateCardName, publicCardName;
	if (!(sourceHiddenToPlayer && targetHiddenToPlayer))
		privateCardName = card->getName();
	if (!(sourceHiddenToOthers && targetHiddenToOthers))
		publicCardName = card->getName();

	int oldCardId = card->getId();
	if (faceDown)
		card->setId(player->newCardId());
	card->setFaceDown(faceDown);

	// The player does not get to see which card he moved if it moves between two parts of hidden zones which
	// are not being looked at.
	int privateNewCardId = card->getId();
	int privateOldCardId = oldCardId;
	if (!targetBeingLookedAt && !sourceBeingLookedAt) {
		privateOldCardId = -1;
		privateNewCardId = -1;
		privateCardName = QString();
	}
	int privatePosition = -1;
	if (startzone->getType() == HiddenZone)
		privatePosition = position;
	cont->enqueueGameEventPrivate(new Event_MoveCard(player->getPlayerId(), privateOldCardId, privateCardName, startzone->getName(), privatePosition, targetzone->getName(), x, y, privateNewCardId, faceDown), game->getGameId());
	cont->enqueueGameEventOmniscient(new Event_MoveCard(player->getPlayerId(), privateOldCardId, privateCardName, startzone->getName(), privatePosition, targetzone->getName(), x, y, privateNewCardId, faceDown), game->getGameId());

	// Other players do not get to see the start and/or target position of the card if the respective
	// part of the zone is being looked at. The information is not needed anyway because in hidden zones,
	// all cards are equal.
	if (
		((startzone->getType() == HiddenZone) && ((startzone->getCardsBeingLookedAt() > position) || (startzone->getCardsBeingLookedAt() == -1)))
		|| (startzone->getType() == PublicZone)
	)
		position = -1;
	if ((targetzone->getType() == HiddenZone) && ((targetzone->getCardsBeingLookedAt() > x) || (targetzone->getCardsBeingLookedAt() == -1)))
		x = -1;

	if ((startzone->getType() == PublicZone) || (targetzone->getType() == PublicZone))
		cont->enqueueGameEventPublic(new Event_MoveCard(player->getPlayerId(), oldCardId, publicCardName, startzone->getName(), position, targetzone->getName(), x, y, card->getId(), faceDown), game->getGameId());
	else
		cont->enqueueGameEventPublic(new Event_MoveCard(player->getPlayerId(), -1, QString(), startzone->getName(), position, targetzone->getName(), x, y, -1, false), game->getGameId());
	
	if (tapped)
		setCardAttrHelper(cont, game, player, targetzone->getName(), card->getId(), "tapped", "1");

	// If the card was moved to another zone, delete all arrows from and to the card
	if (startzone != targetzone) {
		const QList<Server_Player *> &players = game->getPlayers().values();
		for (int i = 0; i < players.size(); ++i) {
			QList<int> arrowsToDelete;
			QMapIterator<int, Server_Arrow *> arrowIterator(players[i]->getArrows());
			while (arrowIterator.hasNext()) {
				Server_Arrow *arrow = arrowIterator.next().value();
				if ((arrow->getStartCard() == card) || (arrow->getTargetCard() == card))
					arrowsToDelete.append(arrow->getId());
			}
			for (int j = 0; j < arrowsToDelete.size(); ++j)
				players[i]->deleteArrow(arrowsToDelete[j]);
		}
	}

	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdMoveCard(Command_MoveCard *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player)
{
	return moveCard(game, player, cont, cmd->getStartZone(), cmd->getCardId(), cmd->getTargetZone(), cmd->getX(), cmd->getY(), cmd->getFaceDown(), cmd->getTapped());
}

ResponseCode Server_ProtocolHandler::cmdCreateToken(Command_CreateToken *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
		
	// powtough wird erst mal ignoriert
	Server_CardZone *zone = player->getZones().value(cmd->getZone());
	if (!zone)
		return RespNameNotFound;

	Server_Card *card = new Server_Card(cmd->getCardName(), player->newCardId(), cmd->getX(), cmd->getY());
	zone->insertCard(card, cmd->getX(), cmd->getY());
	game->sendGameEvent(new Event_CreateToken(player->getPlayerId(), zone->getName(), card->getId(), card->getName(), cmd->getPt(), cmd->getX(), cmd->getY()));
	
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
	Server_CardZone *targetZone = targetPlayer->getZones().value(cmd->getTargetZone());
	if (!startZone || !targetZone)
		return RespNameNotFound;
	Server_Card *startCard = startZone->getCard(cmd->getStartCardId(), false);
	Server_Card *targetCard = targetZone->getCard(cmd->getTargetCardId(), false);
	if (!startCard || !targetCard || (startCard == targetCard))
		return RespContextError;
	QMapIterator<int, Server_Arrow *> arrowIterator(player->getArrows());
	while (arrowIterator.hasNext()) {
		Server_Arrow *temp = arrowIterator.next().value();
		if ((temp->getStartCard() == startCard) && (temp->getTargetCard() == targetCard))
			return RespContextError;
	}

	Server_Arrow *arrow = new Server_Arrow(player->newArrowId(), startCard, targetCard, cmd->getColor());
	player->addArrow(arrow);
	game->sendGameEvent(new Event_CreateArrows(player->getPlayerId(), QList<ServerInfo_Arrow *>() << new ServerInfo_Arrow(
		arrow->getId(),
		startPlayer->getPlayerId(),
		startZone->getName(),
		startCard->getId(),
		targetPlayer->getPlayerId(),
		targetZone->getName(),
		targetCard->getId(),
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

ResponseCode Server_ProtocolHandler::setCardAttrHelper(CommandContainer *cont, Server_Game *game, Server_Player *player, const QString &zoneName, int cardId, const QString &attrName, const QString &attrValue)
{
	if (player->getSpectator())
		return RespFunctionNotAllowed;
	
	if (!game->getGameStarted())
		return RespGameNotStarted;
		
	Server_CardZone *zone = player->getZones().value(zoneName);
	if (!zone)
		return RespNameNotFound;

	if (cardId == -1) {
		QListIterator<Server_Card *> CardIterator(zone->cards);
		while (CardIterator.hasNext())
			if (!CardIterator.next()->setAttribute(attrName, attrValue, true))
				return RespInvalidCommand;
	} else {
		Server_Card *card = zone->getCard(cardId, false);
		if (!card)
			return RespNameNotFound;
		if (!card->setAttribute(attrName, attrValue, false))
			return RespInvalidCommand;
	}
	cont->enqueueGameEventPrivate(new Event_SetCardAttr(player->getPlayerId(), zone->getName(), cardId, attrName, attrValue), game->getGameId());
	cont->enqueueGameEventPublic(new Event_SetCardAttr(player->getPlayerId(), zone->getName(), cardId, attrName, attrValue), game->getGameId());
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdSetCardAttr(Command_SetCardAttr *cmd, CommandContainer *cont, Server_Game *game, Server_Player *player)
{
	return setCardAttrHelper(cont, game, player, cmd->getZone(), cmd->getCardId(), cmd->getAttrName(), cmd->getAttrValue());
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
	
	const QMap<int, Server_Player *> &players = game->getPlayers();
	const QList<int> keys = players.keys();
	
	int activePlayer = game->getActivePlayer();
	int listPos = keys.indexOf(activePlayer);
	do {
		++listPos;
		if (listPos == keys.size())
			listPos = 0;
	} while (players.value(keys[listPos])->getSpectator());
	
	game->setActivePlayer(keys[listPos]);
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
			respCardList.append(new ServerInfo_Card(card->getId(), displayedName, card->getX(), card->getY(), card->getTapped(), card->getAttacking(), card->getPT(), card->getAnnotation(), cardCounterList));
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
