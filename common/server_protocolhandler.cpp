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

void Server_ProtocolHandler::processCommandHelper(Command *command)
{
	lastCommandTime = QDateTime::currentDateTime();

	ResponseCode response = RespInvalidCommand;
	
	ChatCommand *chatCommand = qobject_cast<ChatCommand *>(command);
	GameCommand *gameCommand = qobject_cast<GameCommand *>(command);
	if (chatCommand) {
		qDebug() << "received ChatCommand: channel =" << chatCommand->getChannel();
		if (authState == PasswordWrong) {
			sendProtocolItem(new ProtocolResponse(gameCommand->getCmdId(), RespLoginNeeded));
			return;
		}
	
		Server_ChatChannel *channel = chatChannels.value(chatCommand->getChannel(), 0);
		if (!channel) {
			sendProtocolItem(new ProtocolResponse(gameCommand->getCmdId(), RespNameNotFound));
			return;
		}
		switch (command->getItemId()) {
			case ItemId_Command_ChatLeaveChannel: response = cmdChatLeaveChannel(qobject_cast<Command_ChatLeaveChannel *>(command), channel); break;
			case ItemId_Command_ChatSay: response = cmdChatSay(qobject_cast<Command_ChatSay *>(command), channel); break;
		}
	} else if (gameCommand) {
		qDebug() << "received GameCommand: game =" << gameCommand->getGameId();
		if (authState == PasswordWrong) {
			sendProtocolItem(new ProtocolResponse(gameCommand->getCmdId(), RespLoginNeeded));
			return;
		}
	
		if (!games.contains(gameCommand->getGameId())) {
			qDebug() << "invalid game";
			sendProtocolItem(new ProtocolResponse(gameCommand->getCmdId(), RespNameNotFound));
			return;
		}
		QPair<Server_Game *, Server_Player *> gamePair = games.value(gameCommand->getGameId());
		Server_Game *game = gamePair.first;
		Server_Player *player = gamePair.second;
		
		switch (command->getItemId()) {
			case ItemId_Command_DeckSelect: response = cmdDeckSelect(qobject_cast<Command_DeckSelect *>(command), game, player); break;
			case ItemId_Command_LeaveGame: response = cmdLeaveGame(qobject_cast<Command_LeaveGame *>(command), game, player); break;
			case ItemId_Command_ReadyStart: response = cmdReadyStart(qobject_cast<Command_ReadyStart *>(command), game, player); break;
			case ItemId_Command_Concede: response = cmdConcede(qobject_cast<Command_Concede *>(command), game, player); break;
			case ItemId_Command_Say: response = cmdSay(qobject_cast<Command_Say *>(command), game, player); break;
			case ItemId_Command_Shuffle: response = cmdShuffle(qobject_cast<Command_Shuffle *>(command), game, player); break;
			case ItemId_Command_Mulligan: response = cmdMulligan(qobject_cast<Command_Mulligan *>(command), game, player); break;
			case ItemId_Command_RollDie: response = cmdRollDie(qobject_cast<Command_RollDie *>(command), game, player); break;
			case ItemId_Command_DrawCards: response = cmdDrawCards(qobject_cast<Command_DrawCards *>(command), game, player); break;
			case ItemId_Command_MoveCard: response = cmdMoveCard(qobject_cast<Command_MoveCard *>(command), game, player); break;
			case ItemId_Command_CreateToken: response = cmdCreateToken(qobject_cast<Command_CreateToken *>(command), game, player); break;
			case ItemId_Command_CreateArrow: response = cmdCreateArrow(qobject_cast<Command_CreateArrow *>(command), game, player); break;
			case ItemId_Command_DeleteArrow: response = cmdDeleteArrow(qobject_cast<Command_DeleteArrow *>(command), game, player); break;
			case ItemId_Command_SetCardAttr: response = cmdSetCardAttr(qobject_cast<Command_SetCardAttr *>(command), game, player); break;
			case ItemId_Command_IncCounter: response = cmdIncCounter(qobject_cast<Command_IncCounter *>(command), game, player); break;
			case ItemId_Command_CreateCounter: response = cmdCreateCounter(qobject_cast<Command_CreateCounter *>(command), game, player); break;
			case ItemId_Command_SetCounter: response = cmdSetCounter(qobject_cast<Command_SetCounter *>(command), game, player); break;
			case ItemId_Command_DelCounter: response = cmdDelCounter(qobject_cast<Command_DelCounter *>(command), game, player); break;
			case ItemId_Command_NextTurn: response = cmdNextTurn(qobject_cast<Command_NextTurn *>(command), game, player); break;
			case ItemId_Command_SetActivePhase: response = cmdSetActivePhase(qobject_cast<Command_SetActivePhase *>(command), game, player); break;
			case ItemId_Command_DumpZone: response = cmdDumpZone(qobject_cast<Command_DumpZone *>(command), game, player); break;
			case ItemId_Command_StopDumpZone: response = cmdStopDumpZone(qobject_cast<Command_StopDumpZone *>(command), game, player); break;
		}
	} else {
		qDebug() << "received generic Command";
		switch (command->getItemId()) {
			case ItemId_Command_Ping: response = cmdPing(qobject_cast<Command_Ping *>(command)); break;
			case ItemId_Command_Login: response = cmdLogin(qobject_cast<Command_Login *>(command)); break;
			case ItemId_Command_DeckList: response = cmdDeckList(qobject_cast<Command_DeckList *>(command)); break;
			case ItemId_Command_DeckNewDir: response = cmdDeckNewDir(qobject_cast<Command_DeckNewDir *>(command)); break;
			case ItemId_Command_DeckDelDir: response = cmdDeckDelDir(qobject_cast<Command_DeckDelDir *>(command)); break;
			case ItemId_Command_DeckDel: response = cmdDeckDel(qobject_cast<Command_DeckDel *>(command)); break;
			case ItemId_Command_DeckUpload: response = cmdDeckUpload(qobject_cast<Command_DeckUpload *>(command)); break;
			case ItemId_Command_DeckDownload: response = cmdDeckDownload(qobject_cast<Command_DeckDownload *>(command)); break;
			case ItemId_Command_ListChatChannels: response = cmdListChatChannels(qobject_cast<Command_ListChatChannels *>(command)); break;
			case ItemId_Command_ChatJoinChannel: response = cmdChatJoinChannel(qobject_cast<Command_ChatJoinChannel *>(command)); break;
			case ItemId_Command_ListGames: response = cmdListGames(qobject_cast<Command_ListGames *>(command)); break;
			case ItemId_Command_CreateGame: response = cmdCreateGame(qobject_cast<Command_CreateGame *>(command)); break;
			case ItemId_Command_JoinGame: response = cmdJoinGame(qobject_cast<Command_JoinGame *>(command)); break;
		}
	}
	if (response != RespNothing)
		sendProtocolItem(new ProtocolResponse(command->getCmdId(), response));
}

void Server_ProtocolHandler::processCommand(Command *command)
{
	processCommandHelper(command);
	
	delete command;
	
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

ResponseCode Server_ProtocolHandler::cmdPing(Command_Ping * /*cmd*/)
{
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdLogin(Command_Login *cmd)
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
			const QList<Server_Player *> &gamePlayers = serverGames[i]->getPlayers();
			for (int j = 0; j < gamePlayers.size(); ++j)
				if (gamePlayers[j]->getPlayerName() == playerName) {
					gamePlayers[j]->setProtocolHandler(this);
					games.insert(serverGames[i]->getGameId(), QPair<Server_Game *, Server_Player *>(serverGames[i], gamePlayers[j]));
					enqueueProtocolItem(new Event_GameJoined(serverGames[i]->getGameId(), gamePlayers[j]->getPlayerId(), gamePlayers[j]->getSpectator(), true));
					enqueueProtocolItem(new Event_GameStateChanged(serverGames[i]->getGameId(), serverGames[i]->getGameStarted(), serverGames[i]->getActivePlayer(), serverGames[i]->getActivePhase(), serverGames[i]->getGameState(gamePlayers[j])));
				}
		}
	}

	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdListChatChannels(Command_ListChatChannels * /*cmd*/)
{
	if (authState == PasswordWrong)
		return RespLoginNeeded;
	
	QList<ServerInfo_ChatChannel *> eventChannelList;
	QMapIterator<QString, Server_ChatChannel *> channelIterator(server->getChatChannels());
	while (channelIterator.hasNext()) {
		Server_ChatChannel *c = channelIterator.next().value();
		eventChannelList.append(new ServerInfo_ChatChannel(c->getName(), c->getDescription(), c->size(), c->getAutoJoin()));
	}
	sendProtocolItem(new Event_ListChatChannels(eventChannelList));
	
	acceptsChatChannelListChanges = true;
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdChatJoinChannel(Command_ChatJoinChannel *cmd)
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

ResponseCode Server_ProtocolHandler::cmdChatLeaveChannel(Command_ChatLeaveChannel * /*cmd*/, Server_ChatChannel *channel)
{
	chatChannels.remove(channel->getName());
	channel->removeClient(this);
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdChatSay(Command_ChatSay *cmd, Server_ChatChannel *channel)
{
	channel->say(this, cmd->getMessage());
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdListGames(Command_ListGames * /*cmd*/)
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
			g->getSpectatorCount()
		));
	}
	sendProtocolItem(new Event_ListGames(eventGameList));
	
	acceptsGameListChanges = true;
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdCreateGame(Command_CreateGame *cmd)
{
	Server_Game *game = server->createGame(cmd->getDescription(), cmd->getPassword(), cmd->getMaxPlayers(), cmd->getSpectatorsAllowed(), this);
	Server_Player *creator = game->getCreator();
	games.insert(game->getGameId(), QPair<Server_Game *, Server_Player *>(game, creator));
	
	enqueueProtocolItem(new Event_GameJoined(game->getGameId(), creator->getPlayerId(), false, false));
	enqueueProtocolItem(new Event_GameStateChanged(game->getGameId(), game->getGameStarted(), game->getActivePlayer(), game->getActivePhase(), game->getGameState(creator)));
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdJoinGame(Command_JoinGame *cmd)
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
		enqueueProtocolItem(new Event_GameJoined(cmd->getGameId(), player->getPlayerId(), cmd->getSpectator(), false));
		enqueueProtocolItem(new Event_GameStateChanged(cmd->getGameId(), g->getGameStarted(), g->getActivePlayer(), g->getActivePhase(), g->getGameState(player)));
	}
	return result;
}

ResponseCode Server_ProtocolHandler::cmdLeaveGame(Command_LeaveGame * /*cmd*/, Server_Game *game, Server_Player *player)
{
	game->removePlayer(player);
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdDeckSelect(Command_DeckSelect *cmd, Server_Game *game, Server_Player *player)
{
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
	player->setDeck(deck);
	
	game->sendGameEvent(new Event_DeckSelect(-1, player->getPlayerId(), cmd->getDeckId()));
	
	sendProtocolItem(new Response_DeckDownload(cmd->getCmdId(), RespOk, new DeckList(deck)));
	return RespNothing;
}

ResponseCode Server_ProtocolHandler::cmdConcede(Command_Concede * /*cmd*/, Server_Game *game, Server_Player *player)
{
	player->setConceded(true);
	game->sendGameEvent(new Event_Concede(-1, player->getPlayerId()));
	game->stopGameIfFinished();
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdReadyStart(Command_ReadyStart * /*cmd*/, Server_Game *game, Server_Player *player)
{
	if (!player->getDeck())
		return RespContextError;

	player->setReadyStart(true);
	game->sendGameEvent(new Event_ReadyStart(-1, player->getPlayerId()));
	game->startGameIfReady();
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdSay(Command_Say *cmd, Server_Game *game, Server_Player *player)
{
	game->sendGameEvent(new Event_Say(-1, player->getPlayerId(), cmd->getMessage()));
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdShuffle(Command_Shuffle * /*cmd*/, Server_Game *game, Server_Player *player)
{
	if (!game->getGameStarted())
		return RespGameNotStarted;
		
	player->getZones().value("deck")->shuffle();
	game->sendGameEvent(new Event_Shuffle(-1, player->getPlayerId()));
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdMulligan(Command_Mulligan * /*cmd*/, Server_Game *game, Server_Player *player)
{
	if (!game->getGameStarted())
		return RespGameNotStarted;
		
	int number = player->getInitialCards();
	if (!number)
		return RespContextError;

	Server_CardZone *hand = player->getZones().value("hand");
	while (!hand->cards.isEmpty())
		moveCard(game, player, "hand", hand->cards.first()->getId(), "deck", 0, 0, false);

	player->getZones().value("deck")->shuffle();
	game->sendGameEvent(new Event_Shuffle(-1, player->getPlayerId()));

	drawCards(game, player, number);
	player->setInitialCards(number - 1);

	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdRollDie(Command_RollDie *cmd, Server_Game *game, Server_Player *player)
{
	game->sendGameEvent(new Event_RollDie(-1, player->getPlayerId(), cmd->getSides(), rng->getNumber(1, cmd->getSides())));
	return RespOk;
}

ResponseCode Server_ProtocolHandler::drawCards(Server_Game *game, Server_Player *player, int number)
{
	if (!game->getGameStarted())
		return RespGameNotStarted;
		
	Server_CardZone *deck = player->getZones().value("deck");
	Server_CardZone *hand = player->getZones().value("hand");
	if (deck->cards.size() < number)
		number = deck->cards.size();

	QList<ServerInfo_Card *> cardList;
	for (int i = 0; i < number; ++i) {
		Server_Card *card = deck->cards.takeFirst();
		hand->cards.append(card);
		cardList.append(new ServerInfo_Card(card->getId(), card->getName()));
	}

	player->sendProtocolItem(new Event_DrawCards(game->getGameId(), player->getPlayerId(), cardList.size(), cardList));
	game->sendGameEvent(new Event_DrawCards(-1, player->getPlayerId(), cardList.size()), player);

	return RespOk;
}


ResponseCode Server_ProtocolHandler::cmdDrawCards(Command_DrawCards *cmd, Server_Game *game, Server_Player *player)
{
	return drawCards(game, player, cmd->getNumber());
}

ResponseCode Server_ProtocolHandler::moveCard(Server_Game *game, Server_Player *player, const QString &_startZone, int _cardId, const QString &_targetZone, int x, int y, bool faceDown)
{
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
	player->sendProtocolItem(new Event_MoveCard(game->getGameId(), player->getPlayerId(), privateOldCardId, privateCardName, startzone->getName(), privatePosition, targetzone->getName(), x, y, privateNewCardId, faceDown));

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
		game->sendGameEvent(new Event_MoveCard(-1, player->getPlayerId(), oldCardId, publicCardName, startzone->getName(), position, targetzone->getName(), x, y, card->getId(), faceDown), player);
	else
		game->sendGameEvent(new Event_MoveCard(-1, player->getPlayerId(), -1, QString(), startzone->getName(), position, targetzone->getName(), x, y, -1, false), player);

	// If the card was moved to another zone, delete all arrows from and to the card
	if (startzone != targetzone) {
		const QList<Server_Player *> &players = game->getPlayers();
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

ResponseCode Server_ProtocolHandler::cmdMoveCard(Command_MoveCard *cmd, Server_Game *game, Server_Player *player)
{
	return moveCard(game, player, cmd->getStartZone(), cmd->getCardId(), cmd->getTargetZone(), cmd->getX(), cmd->getY(), cmd->getFaceDown());
}

ResponseCode Server_ProtocolHandler::cmdCreateToken(Command_CreateToken *cmd, Server_Game *game, Server_Player *player)
{
	if (!game->getGameStarted())
		return RespGameNotStarted;
		
	// powtough wird erst mal ignoriert
	Server_CardZone *zone = player->getZones().value(cmd->getZone());
	if (!zone)
		return RespNameNotFound;

	Server_Card *card = new Server_Card(cmd->getCardName(), player->newCardId(), cmd->getX(), cmd->getY());
	zone->insertCard(card, cmd->getX(), cmd->getY());
	game->sendGameEvent(new Event_CreateToken(-1, player->getPlayerId(), zone->getName(), card->getId(), card->getName(), cmd->getPt(), cmd->getX(), cmd->getY()));
	
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdCreateArrow(Command_CreateArrow *cmd, Server_Game *game, Server_Player *player)
{
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
	game->sendGameEvent(new Event_CreateArrows(-1, player->getPlayerId(), QList<ServerInfo_Arrow *>() << new ServerInfo_Arrow(
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

ResponseCode Server_ProtocolHandler::cmdDeleteArrow(Command_DeleteArrow *cmd, Server_Game *game, Server_Player *player)
{
	if (!game->getGameStarted())
		return RespGameNotStarted;
		
	if (!player->deleteArrow(cmd->getArrowId()))
		return RespNameNotFound;
	
	game->sendGameEvent(new Event_DeleteArrow(-1, player->getPlayerId(), cmd->getArrowId()));
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdSetCardAttr(Command_SetCardAttr *cmd, Server_Game *game, Server_Player *player)
{
	if (!game->getGameStarted())
		return RespGameNotStarted;
		
	// zone, card id, attr name, attr value
	// card id = -1 => affects all cards in the specified zone
	Server_CardZone *zone = player->getZones().value(cmd->getZone());
	if (!zone)
		return RespNameNotFound;

	if (cmd->getCardId() == -1) {
		QListIterator<Server_Card *> CardIterator(zone->cards);
		while (CardIterator.hasNext())
			if (!CardIterator.next()->setAttribute(cmd->getAttrName(), cmd->getAttrValue(), true))
				return RespInvalidCommand;
	} else {
		Server_Card *card = zone->getCard(cmd->getCardId(), false);
		if (!card)
			return RespNameNotFound;
		if (!card->setAttribute(cmd->getAttrName(), cmd->getAttrValue(), false))
			return RespInvalidCommand;
	}
	game->sendGameEvent(new Event_SetCardAttr(-1, player->getPlayerId(), zone->getName(), cmd->getCardId(), cmd->getAttrName(), cmd->getAttrValue()));
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdIncCounter(Command_IncCounter *cmd, Server_Game *game, Server_Player *player)
{
	if (!game->getGameStarted())
		return RespGameNotStarted;
		
	const QMap<int, Server_Counter *> counters = player->getCounters();
	Server_Counter *c = counters.value(cmd->getCounterId(), 0);
	if (!c)
		return RespNameNotFound;
	
	c->setCount(c->getCount() + cmd->getDelta());
	game->sendGameEvent(new Event_SetCounter(-1, player->getPlayerId(), c->getId(), c->getCount()));
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdCreateCounter(Command_CreateCounter *cmd, Server_Game *game, Server_Player *player)
{
	if (!game->getGameStarted())
		return RespGameNotStarted;
		
	Server_Counter *c = new Server_Counter(player->newCounterId(), cmd->getCounterName(), cmd->getColor(), cmd->getRadius(), cmd->getValue());
	player->addCounter(c);
	game->sendGameEvent(new Event_CreateCounters(-1, player->getPlayerId(), QList<ServerInfo_Counter *>() << new ServerInfo_Counter(c->getId(), c->getName(), c->getColor(), c->getRadius(), c->getCount())));
	
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdSetCounter(Command_SetCounter *cmd, Server_Game *game, Server_Player *player)
{
	if (!game->getGameStarted())
		return RespGameNotStarted;
		
	Server_Counter *c = player->getCounters().value(cmd->getCounterId(), 0);;
	if (!c)
		return RespNameNotFound;
	
	c->setCount(cmd->getValue());
	game->sendGameEvent(new Event_SetCounter(-1, player->getPlayerId(), c->getId(), c->getCount()));
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdDelCounter(Command_DelCounter *cmd, Server_Game *game, Server_Player *player)
{
	if (!game->getGameStarted())
		return RespGameNotStarted;
		
	if (!player->deleteCounter(cmd->getCounterId()))
		return RespNameNotFound;
	game->sendGameEvent(new Event_DelCounter(-1, player->getPlayerId(), cmd->getCounterId()));
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdNextTurn(Command_NextTurn * /*cmd*/, Server_Game *game, Server_Player * /*player*/)
{
	if (!game->getGameStarted())
		return RespGameNotStarted;
		
	int activePlayer = game->getActivePlayer();
	if (++activePlayer == game->getPlayerCount())
		activePlayer = 0;
	game->setActivePlayer(activePlayer);
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdSetActivePhase(Command_SetActivePhase *cmd, Server_Game *game, Server_Player *player)
{
	if (!game->getGameStarted())
		return RespGameNotStarted;
		
	if (game->getActivePlayer() != player->getPlayerId())
		return RespContextError;
	game->setActivePhase(cmd->getPhase());
	
	return RespOk;
}

ResponseCode Server_ProtocolHandler::cmdDumpZone(Command_DumpZone *cmd, Server_Game *game, Server_Player *player)
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
		else
			respCardList.append(new ServerInfo_Card(card->getId(), displayedName, card->getX(), card->getY(), card->getCounters(), card->getTapped(), card->getAttacking(), card->getAnnotation()));
	}
	if (zone->getType() == HiddenZone) {
		zone->setCardsBeingLookedAt(numberCards);
		game->sendGameEvent(new Event_DumpZone(-1, player->getPlayerId(), otherPlayer->getPlayerId(), zone->getName(), numberCards));
	}
	sendProtocolItem(new Response_DumpZone(cmd->getCmdId(), RespOk, new ServerInfo_Zone(zone->getName(), zone->getType(), zone->hasCoords(), numberCards < zone->cards.size() ? zone->cards.size() : numberCards, respCardList)));
	return RespNothing;
}

ResponseCode Server_ProtocolHandler::cmdStopDumpZone(Command_StopDumpZone *cmd, Server_Game *game, Server_Player *player)
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
		game->sendGameEvent(new Event_StopDumpZone(-1, player->getPlayerId(), cmd->getPlayerId(), zone->getName()));
	}
	return RespOk;
}
