#include <QDebug>
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

Server_ProtocolHandler::Server_ProtocolHandler(Server *_server, QObject *parent)
	: QObject(parent), server(_server), authState(PasswordWrong), acceptsGameListChanges(false)
{
}

Server_ProtocolHandler::~Server_ProtocolHandler()
{
}

void Server_ProtocolHandler::processCommand(Command *command)
{
	ProtocolResponse::ResponseCode response = ProtocolResponse::RespInvalidCommand;
	
	ChatCommand *chatCommand = qobject_cast<ChatCommand *>(command);
	GameCommand *gameCommand = qobject_cast<GameCommand *>(command);
	if (chatCommand) {
		qDebug() << "received ChatCommand: channel =" << chatCommand->getChannel();
		Server_ChatChannel *channel = chatChannels.value(chatCommand->getChannel(), 0);
		if (!channel) {
			sendProtocolItem(new ProtocolResponse(gameCommand->getCmdId(), ProtocolResponse::RespNameNotFound));
			return;
		}
		switch (command->getItemId()) {
			case ItemId_Command_ChatLeaveChannel: response = cmdChatLeaveChannel(qobject_cast<Command_ChatLeaveChannel *>(command), channel); break;
			case ItemId_Command_ChatSay: response = cmdChatSay(qobject_cast<Command_ChatSay *>(command), channel); break;
		}
	} else if (gameCommand) {
		qDebug() << "received GameCommand: game =" << gameCommand->getGameId();
		if (!games.contains(gameCommand->getGameId())) {
			sendProtocolItem(new ProtocolResponse(gameCommand->getCmdId(), ProtocolResponse::RespNameNotFound));
			return;
		}
		QPair<Server_Game *, Server_Player *> gamePair = games.value(gameCommand->getGameId());
		Server_Game *game = gamePair.first;
		Server_Player *player = gamePair.second;
		
		switch (command->getItemId()) {
			case ItemId_Command_LeaveGame: response = cmdLeaveGame(qobject_cast<Command_LeaveGame *>(command), game, player); break;
			case ItemId_Command_Say: response = cmdSay(qobject_cast<Command_Say *>(command), game, player); break;
			case ItemId_Command_Shuffle: response = cmdShuffle(qobject_cast<Command_Shuffle *>(command), game, player); break;
			case ItemId_Command_RollDie: response = cmdRollDie(qobject_cast<Command_RollDie *>(command), game, player); break;
			case ItemId_Command_DrawCards: response = cmdDrawCards(qobject_cast<Command_DrawCards *>(command), game, player); break;
			case ItemId_Command_MoveCard: response = cmdMoveCard(qobject_cast<Command_MoveCard *>(command), game, player); break;
			case ItemId_Command_CreateToken: response = cmdCreateToken(qobject_cast<Command_CreateToken *>(command), game, player); break;
			case ItemId_Command_CreateArrow: response = cmdCreateArrow(qobject_cast<Command_CreateArrow *>(command), game, player); break;
			case ItemId_Command_DeleteArrow: response = cmdDeleteArrow(qobject_cast<Command_DeleteArrow *>(command), game, player); break;
			case ItemId_Command_SetCardAttr: response = cmdSetCardAttr(qobject_cast<Command_SetCardAttr *>(command), game, player); break;
			case ItemId_Command_ReadyStart: response = cmdReadyStart(qobject_cast<Command_ReadyStart *>(command), game, player); break;
			case ItemId_Command_IncCounter: response = cmdIncCounter(qobject_cast<Command_IncCounter *>(command), game, player); break;
			case ItemId_Command_AddCounter: response = cmdAddCounter(qobject_cast<Command_AddCounter *>(command), game, player); break;
			case ItemId_Command_SetCounter: response = cmdSetCounter(qobject_cast<Command_SetCounter *>(command), game, player); break;
			case ItemId_Command_DelCounter: response = cmdDelCounter(qobject_cast<Command_DelCounter *>(command), game, player); break;
			case ItemId_Command_NextTurn: response = cmdNextTurn(qobject_cast<Command_NextTurn *>(command), game, player); break;
			case ItemId_Command_SetActivePhase: response = cmdSetActivePhase(qobject_cast<Command_SetActivePhase *>(command), game, player); break;
			case ItemId_Command_DumpZone: response = cmdDumpZone(qobject_cast<Command_DumpZone *>(command), game, player); break;
			case ItemId_Command_StopDumpZone: response = cmdStopDumpZone(qobject_cast<Command_StopDumpZone *>(command), game, player); break;
			case ItemId_Command_DumpAll: response = cmdDumpAll(qobject_cast<Command_DumpAll *>(command), game, player); break;
			case ItemId_Command_SubmitDeck: response = cmdSubmitDeck(qobject_cast<Command_SubmitDeck *>(command), game, player); break;
		}
	} else {
		qDebug() << "received generic Command";
		switch (command->getItemId()) {
			case ItemId_Command_Ping: response = cmdPing(qobject_cast<Command_Ping *>(command)); break;
			case ItemId_Command_Login: response = cmdLogin(qobject_cast<Command_Login *>(command)); break;
			case ItemId_Command_ChatListChannels: response = cmdChatListChannels(qobject_cast<Command_ChatListChannels *>(command)); break;
			case ItemId_Command_ChatJoinChannel: response = cmdChatJoinChannel(qobject_cast<Command_ChatJoinChannel *>(command)); break;
			case ItemId_Command_ListGames: response = cmdListGames(qobject_cast<Command_ListGames *>(command)); break;
			case ItemId_Command_CreateGame: response = cmdCreateGame(qobject_cast<Command_CreateGame *>(command)); break;
			case ItemId_Command_JoinGame: response = cmdJoinGame(qobject_cast<Command_JoinGame *>(command)); break;
		}
	}
	if (response != ProtocolResponse::RespNothing)
		sendProtocolItem(new ProtocolResponse(command->getCmdId(), response));
	
	delete command;
}

QPair<Server_Game *, Server_Player *> Server_ProtocolHandler::getGame(int gameId) const
{
	if (games.contains(gameId))
		return games.value(gameId);
	return QPair<Server_Game *, Server_Player *>(0, 0);
}

ProtocolResponse::ResponseCode Server_ProtocolHandler::cmdPing(Command_Ping *cmd)
{
	return ProtocolResponse::RespOk;
}

ProtocolResponse::ResponseCode Server_ProtocolHandler::cmdLogin(Command_Login *cmd)
{
	authState = server->checkUserPassword(cmd->getUsername(), cmd->getPassword());
	if (authState == PasswordWrong)
		return ProtocolResponse::RespWrongPassword;
	playerName = cmd->getUsername();
	
/*
	Mit enqueueProtocolItem() lösen!
	
	QStringList loginMessage = server->getLoginMessage();
	for (int i = 0; i < loginMessage.size(); ++i)
		msg("chat|server_message||" + loginMessage[i]);
*/	
	return ProtocolResponse::RespOk;
}

ProtocolResponse::ResponseCode Server_ProtocolHandler::cmdChatListChannels(Command_ChatListChannels *cmd)
{
	QMapIterator<QString, Server_ChatChannel *> channelIterator(server->getChatChannels());
	while (channelIterator.hasNext()) {
		Server_ChatChannel *c = channelIterator.next().value();
//		msg(c->getChannelListLine());
	}
	
	acceptsChatChannelListChanges = true;
	return ProtocolResponse::RespOk;
}

ProtocolResponse::ResponseCode Server_ProtocolHandler::cmdChatJoinChannel(Command_ChatJoinChannel *cmd)
{
	if (chatChannels.contains(cmd->getChannel()))
		return ProtocolResponse::RespContextError;
	
	QMap<QString, Server_ChatChannel *> allChannels = server->getChatChannels();
	Server_ChatChannel *c = allChannels.value(cmd->getChannel(), 0);
	if (!c)
		return ProtocolResponse::RespNameNotFound;

	// hier wird irgendwo ein Event erzeugt, das nach dem ok kommen muss -> evtl mit enqueue
	c->addClient(this);
	chatChannels.insert(cmd->getChannel(), c);
	return ProtocolResponse::RespOk;
}

ProtocolResponse::ResponseCode Server_ProtocolHandler::cmdChatLeaveChannel(Command_ChatLeaveChannel *cmd, Server_ChatChannel *channel)
{
	chatChannels.remove(channel->getName());
	channel->removeClient(this);
	return ProtocolResponse::RespOk;
}

ProtocolResponse::ResponseCode Server_ProtocolHandler::cmdChatSay(Command_ChatSay *cmd, Server_ChatChannel *channel)
{
	channel->say(this, cmd->getMessage());
	return ProtocolResponse::RespOk;
}

ProtocolResponse::ResponseCode Server_ProtocolHandler::cmdListGames(Command_ListGames *cmd)
{
	const QList<Server_Game *> &gameList = server->getGames();
	for (int i = 0; i < gameList.size(); ++i) {
//		msg(gameList[i]->getGameListLine());
	}
	acceptsGameListChanges = true;
	return ProtocolResponse::RespOk;
}

ProtocolResponse::ResponseCode Server_ProtocolHandler::cmdCreateGame(Command_CreateGame *cmd)
{
	Server_Game *game = server->createGame(cmd->getDescription(), cmd->getPassword(), cmd->getMaxPlayers(), cmd->getSpectatorsAllowed(), playerName);
	games.insert(game->getGameId(), QPair<Server_Game *, Server_Player *>(game, game->getCreator()));
	
	return ProtocolResponse::RespOk;
}

ProtocolResponse::ResponseCode Server_ProtocolHandler::cmdJoinGame(Command_JoinGame *cmd)
{
	Server_Game *g = server->getGame(cmd->getGameId());
	if (!g)
		return ProtocolResponse::RespNameNotFound;
	
	ProtocolResponse::ResponseCode result = g->checkJoin(cmd->getPassword(), cmd->getSpectator());
	if (result == ProtocolResponse::RespOk) {
		Server_Player *player = g->addPlayer(playerName, cmd->getSpectator());
		games.insert(cmd->getGameId(), QPair<Server_Game *, Server_Player *>(g, player));
	}
	return result;
}

ProtocolResponse::ResponseCode Server_ProtocolHandler::cmdLeaveGame(Command_LeaveGame *cmd, Server_Game *game, Server_Player *player)
{
	game->removePlayer(player);
	return ProtocolResponse::RespOk;
}

ProtocolResponse::ResponseCode Server_ProtocolHandler::cmdSay(Command_Say *cmd, Server_Game *game, Server_Player *player)
{
	// XXX
	return ProtocolResponse::RespOk;
}

ProtocolResponse::ResponseCode Server_ProtocolHandler::cmdShuffle(Command_Shuffle *cmd, Server_Game *game, Server_Player *player)
{
	player->getZones().value("deck")->shuffle();
//	game->broadcastEvent("shuffle", player);
	return ProtocolResponse::RespOk;
}

ProtocolResponse::ResponseCode Server_ProtocolHandler::cmdRollDie(Command_RollDie *cmd, Server_Game *game, Server_Player *player)
{
//	game->broadcastEvent(QString("roll_die|%1|%2").arg(sides).arg(rng->getNumber(1, sides)), player);
	return ProtocolResponse::RespOk;
}

ProtocolResponse::ResponseCode Server_ProtocolHandler::cmdDrawCards(Command_DrawCards *cmd, Server_Game *game, Server_Player *player)
{
	Server_CardZone *deck = player->getZones().value("deck");
	Server_CardZone *hand = player->getZones().value("hand");
	int number = cmd->getNumber();
	if (deck->cards.size() < number)
		number = deck->cards.size();

	for (int i = 0; i < number; ++i) {
		Server_Card *card = deck->cards.takeFirst();
		hand->cards.append(card);
//		player->privateEvent(QString("draw|%1|%2").arg(card->getId()).arg(card->getName()));
	}

//	game->broadcastEvent(QString("draw|%1").arg(number), player);
	return ProtocolResponse::RespOk;
}

ProtocolResponse::ResponseCode Server_ProtocolHandler::cmdMoveCard(Command_MoveCard *cmd, Server_Game *game, Server_Player *player)
{
	// ID Karte, Startzone, Zielzone, Koordinaten X, Y, Facedown
	Server_CardZone *startzone = player->getZones().value(cmd->getStartZone());
	Server_CardZone *targetzone = player->getZones().value(cmd->getTargetZone());
	if ((!startzone) || (!targetzone))
		return ProtocolResponse::RespNameNotFound;

	int position = -1;
	Server_Card *card = startzone->getCard(cmd->getCardId(), true, &position);
	if (!card)
		return ProtocolResponse::RespNameNotFound;
	int x = cmd->getX();
	if (x == -1)
		x = targetzone->cards.size();
	int y = 0;
	if (targetzone->hasCoords())
		y = cmd->getY();
	bool facedown = cmd->getFaceDown();

	targetzone->insertCard(card, x, y);

	bool targetBeingLookedAt = (targetzone->getType() != Server_CardZone::HiddenZone) || (targetzone->getCardsBeingLookedAt() > x) || (targetzone->getCardsBeingLookedAt() == -1);
	bool sourceBeingLookedAt = (startzone->getType() != Server_CardZone::HiddenZone) || (startzone->getCardsBeingLookedAt() > position) || (startzone->getCardsBeingLookedAt() == -1);

	bool targetHiddenToPlayer = facedown || !targetBeingLookedAt;
	bool targetHiddenToOthers = facedown || (targetzone->getType() != Server_CardZone::PublicZone);
	bool sourceHiddenToPlayer = card->getFaceDown() || !sourceBeingLookedAt;
	bool sourceHiddenToOthers = card->getFaceDown() || (startzone->getType() != Server_CardZone::PublicZone);
	
	QString privateCardName, publicCardName;
	if (!(sourceHiddenToPlayer && targetHiddenToPlayer))
		privateCardName = card->getName();
	if (!(sourceHiddenToOthers && targetHiddenToOthers))
		publicCardName = card->getName();
		
	if (facedown)
		card->setId(player->newCardId());
	card->setFaceDown(facedown);
	
	// The player does not get to see which card he moved if it moves between two parts of hidden zones which
	// are not being looked at.
	QString privateCardId = QString::number(card->getId());
	if (!targetBeingLookedAt && !sourceBeingLookedAt) {
		privateCardId = QString();
		privateCardName = QString();
	}
/*	player->privateEvent(QString("move_card|%1|%2|%3|%4|%5|%6|%7|%8").arg(privateCardId)
							    .arg(privateCardName)
							    .arg(startzone->getName())
							    .arg(position)
							    .arg(targetzone->getName())
							    .arg(x)
							    .arg(y)
							    .arg(facedown ? 1 : 0));
*/	
	// Other players do not get to see the start and/or target position of the card if the respective
	// part of the zone is being looked at. The information is not needed anyway because in hidden zones,
	// all cards are equal.
	if ((startzone->getType() == Server_CardZone::HiddenZone) && ((startzone->getCardsBeingLookedAt() > position) || (startzone->getCardsBeingLookedAt() == -1)))
		position = -1;
	if ((targetzone->getType() == Server_CardZone::HiddenZone) && ((targetzone->getCardsBeingLookedAt() > x) || (targetzone->getCardsBeingLookedAt() == -1)))
		x = -1;
	
/*	if ((startzone->getType() == Server_CardZone::PublicZone) || (targetzone->getType() == Server_CardZone::PublicZone))
		game->broadcastEvent(QString("move_card|%1|%2|%3|%4|%5|%6|%7|%8").arg(card->getId())
									 .arg(publicCardName)
									 .arg(startzone->getName())
									 .arg(position)
									 .arg(targetzone->getName())
									 .arg(x)
									 .arg(y)
									 .arg(facedown ? 1 : 0), player);
	else
		game->broadcastEvent(QString("move_card|||%1|%2|%3|%4|%5|0").arg(startzone->getName())
								     .arg(position)
								     .arg(targetzone->getName())
								     .arg(x)
								     .arg(y), player);
*/	
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

	return ProtocolResponse::RespOk;
}

ProtocolResponse::ResponseCode Server_ProtocolHandler::cmdCreateToken(Command_CreateToken *cmd, Server_Game *game, Server_Player *player)
{
	// zone, cardname, powtough, x, y
	// powtough wird erst mal ignoriert
	Server_CardZone *zone = player->getZones().value(cmd->getZone());
	if (!zone)
		return ProtocolResponse::RespNameNotFound;

	Server_Card *card = new Server_Card(cmd->getCardName(), player->newCardId(), cmd->getX(), cmd->getY());
	zone->insertCard(card, cmd->getX(), cmd->getY());
/*	game->broadcastEvent(QString("create_token|%1|%2|%3|%4|%5|%6").arg(zone->getName())
									 .arg(cardid)
									 .arg(cardname)
									 .arg(powtough)
									 .arg(x)
									 .arg(y), player);
*/	return ProtocolResponse::RespOk;
}

ProtocolResponse::ResponseCode Server_ProtocolHandler::cmdCreateArrow(Command_CreateArrow *cmd, Server_Game *game, Server_Player *player)
{
	Server_Player *startPlayer = game->getPlayer(cmd->getStartPlayerId());
	Server_Player *targetPlayer = game->getPlayer(cmd->getTargetPlayerId());
	if (!startPlayer || !targetPlayer)
		return ProtocolResponse::RespNameNotFound;
	Server_CardZone *startZone = startPlayer->getZones().value(cmd->getStartZone());
	Server_CardZone *targetZone = targetPlayer->getZones().value(cmd->getTargetZone());
	if (!startZone || !targetZone)
		return ProtocolResponse::RespNameNotFound;
	Server_Card *startCard = startZone->getCard(cmd->getStartCardId(), false);
	Server_Card *targetCard = targetZone->getCard(cmd->getTargetCardId(), false);
	if (!startCard || !targetCard || (startCard == targetCard))
		return ProtocolResponse::RespContextError;
	QMapIterator<int, Server_Arrow *> arrowIterator(player->getArrows());
	while (arrowIterator.hasNext()) {
		Server_Arrow *temp = arrowIterator.next().value();
		if ((temp->getStartCard() == startCard) && (temp->getTargetCard() == targetCard))
			return ProtocolResponse::RespContextError;
	}

	Server_Arrow *arrow = new Server_Arrow(player->newArrowId(), startCard, targetCard, cmd->getColor());
	player->addArrow(arrow);
/*	game->broadcastEvent(QString("create_arrow|%1|%2|%3|%4|%5|%6|%7|%8")
		.arg(arrow->getId())
		.arg(startPlayer->getPlayerId())
		.arg(startZone->getName())
		.arg(startCard->getId())
		.arg(targetPlayer->getPlayerId())
		.arg(targetZone->getName())
		.arg(targetCard->getId())
		.arg(cmd->getColor()), player
	);
*/	return ProtocolResponse::RespOk;
}

ProtocolResponse::ResponseCode Server_ProtocolHandler::cmdDeleteArrow(Command_DeleteArrow *cmd, Server_Game *game, Server_Player *player)
{
	if (!player->deleteArrow(cmd->getArrowId()))
		return ProtocolResponse::RespNameNotFound;
	
//	game->broadcastEvent(QString("delete_arrow|%1").arg(arrowId), player);
	return ProtocolResponse::RespOk;
}

ProtocolResponse::ResponseCode Server_ProtocolHandler::cmdSetCardAttr(Command_SetCardAttr *cmd, Server_Game *game, Server_Player *player)
{
	// zone, card id, attr name, attr value
	// card id = -1 => affects all cards in the specified zone
	Server_CardZone *zone = player->getZones().value(cmd->getZone());
	if (!zone)
		return ProtocolResponse::RespNameNotFound;

	if (cmd->getCardId() == -1) {
		QListIterator<Server_Card *> CardIterator(zone->cards);
		while (CardIterator.hasNext())
			if (!CardIterator.next()->setAttribute(cmd->getAttrName(), cmd->getAttrValue(), true))
				return ProtocolResponse::RespInvalidCommand;
	} else {
		Server_Card *card = zone->getCard(cmd->getCardId(), false);
		if (!card)
			return ProtocolResponse::RespNameNotFound;
		if (!card->setAttribute(cmd->getAttrName(), cmd->getAttrValue(), false))
			return ProtocolResponse::RespInvalidCommand;
	}
//	game->broadcastEvent(QString("set_card_attr|%1|%2|%3|%4").arg(zone->getName()).arg(cardid).arg(aname).arg(avalue), player);
	return ProtocolResponse::RespOk;
}

ProtocolResponse::ResponseCode Server_ProtocolHandler::cmdReadyStart(Command_ReadyStart *cmd, Server_Game *game, Server_Player *player)
{
	player->setStatus(StatusReadyStart);
//	game->broadcastEvent(QString("ready_start"), player);
	game->startGameIfReady();
	return ProtocolResponse::RespOk;
}

ProtocolResponse::ResponseCode Server_ProtocolHandler::cmdIncCounter(Command_IncCounter *cmd, Server_Game *game, Server_Player *player)
{
	const QMap<int, Server_Counter *> counters = player->getCounters();
	Server_Counter *c = counters.value(cmd->getCounterId(), 0);
	if (!c)
		return ProtocolResponse::RespNameNotFound;
	
	c->setCount(c->getCount() + cmd->getDelta());
//	game->broadcastEvent(QString("set_counter|%1|%2").arg(c->getId()).arg(c->getCount()), player);
	return ProtocolResponse::RespOk;
}

ProtocolResponse::ResponseCode Server_ProtocolHandler::cmdAddCounter(Command_AddCounter *cmd, Server_Game *game, Server_Player *player)
{
	Server_Counter *c = new Server_Counter(player->newCounterId(), cmd->getCounterName(), cmd->getColor(), cmd->getRadius(), cmd->getValue());
	player->addCounter(c);
//	game->broadcastEvent(QString("add_counter|%1|%2|%3|%4|%5").arg(c->getId()).arg(c->getName()).arg(color).arg(radius).arg(count), player);
	
	return ProtocolResponse::RespOk;
}

ProtocolResponse::ResponseCode Server_ProtocolHandler::cmdSetCounter(Command_SetCounter *cmd, Server_Game *game, Server_Player *player)
{
	Server_Counter *c = player->getCounters().value(cmd->getCounterId(), 0);;
	if (!c)
		return ProtocolResponse::RespNameNotFound;
	
	c->setCount(cmd->getValue());
//	game->broadcastEvent(QString("set_counter|%1|%2").arg(c->getId()).arg(count), player);
	return ProtocolResponse::RespOk;
}

ProtocolResponse::ResponseCode Server_ProtocolHandler::cmdDelCounter(Command_DelCounter *cmd, Server_Game *game, Server_Player *player)
{
	if (!player->deleteCounter(cmd->getCounterId()))
		return ProtocolResponse::RespNameNotFound;
//	game->broadcastEvent(QString("del_counter|%1").arg(counterId), player);
	return ProtocolResponse::RespOk;
}

ProtocolResponse::ResponseCode Server_ProtocolHandler::cmdNextTurn(Command_NextTurn * /*cmd*/, Server_Game *game, Server_Player * /*player*/)
{
	int activePlayer = game->getActivePlayer();
	if (++activePlayer == game->getPlayerCount())
		activePlayer = 0;
	game->setActivePlayer(activePlayer);
	return ProtocolResponse::RespOk;
}

ProtocolResponse::ResponseCode Server_ProtocolHandler::cmdSetActivePhase(Command_SetActivePhase *cmd, Server_Game *game, Server_Player *player)
{
	if (game->getActivePlayer() != player->getPlayerId())
		return ProtocolResponse::RespContextError;
	game->setActivePhase(cmd->getPhase());
	
	return ProtocolResponse::RespOk;
}

ProtocolResponse::ResponseCode Server_ProtocolHandler::cmdDumpZone(Command_DumpZone *cmd, Server_Game *game, Server_Player *player)
{
	Server_Player *otherPlayer = game->getPlayer(cmd->getPlayerId());
	if (!otherPlayer)
		return ProtocolResponse::RespNameNotFound;
	Server_CardZone *zone = otherPlayer->getZones().value(cmd->getZoneName());
	if (!zone)
		return ProtocolResponse::RespNameNotFound;
	if (!((zone->getType() == Server_CardZone::PublicZone) || (player == otherPlayer)))
		return ProtocolResponse::RespContextError;
	
	if (zone->getType() == Server_CardZone::HiddenZone) {
//		game->broadcastEvent(QString("dump_zone|%1|%2|%3").arg(player_id).arg(zone->getName()).arg(number_cards), player);
	}
//	remsg->sendList(dumpZoneHelper(otherPlayer, zone, number_cards));
	return ProtocolResponse::RespOk;
}

ProtocolResponse::ResponseCode Server_ProtocolHandler::cmdStopDumpZone(Command_StopDumpZone *cmd, Server_Game *game, Server_Player *player)
{
	Server_Player *otherPlayer = game->getPlayer(cmd->getPlayerId());
	if (!otherPlayer)
		return ProtocolResponse::RespNameNotFound;
	Server_CardZone *zone = otherPlayer->getZones().value(cmd->getZoneName());
	if (!zone)
		return ProtocolResponse::RespNameNotFound;
	
	if (zone->getType() == Server_CardZone::HiddenZone) {
		zone->setCardsBeingLookedAt(0);
//		game->broadcastEvent(QString("stop_dump_zone|%1|%2").arg(otherPlayer->getPlayerId()).arg(zone->getName()), player);
	}
	return ProtocolResponse::RespOk;
}

ProtocolResponse::ResponseCode Server_ProtocolHandler::cmdDumpAll(Command_DumpAll *cmd, Server_Game *game, Server_Player *player)
{
	return ProtocolResponse::RespOk;
}

ProtocolResponse::ResponseCode Server_ProtocolHandler::cmdSubmitDeck(Command_SubmitDeck *cmd, Server_Game *game, Server_Player *player)
{
	return ProtocolResponse::RespOk;
}
