/***************************************************************************
 *   Copyright (C) 2008 by Max-Wilhelm Bruker   *
 *   brukie@laptop   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <QStringList>
#include "server.h"
#include "serversocket.h"
#include "servergame.h"
#include "version.h"
#include "returnmessage.h"
#include "playerzone.h"
#include "counter.h"
#include "card.h"
#include "arrow.h"
#include "chatchannel.h"
#include "player.h"
#include "abstractrng.h"

QHash<QString, ServerSocket::CommandProperties *> ServerSocket::commandHash;

ServerSocket::ServerSocket(Server *_server, QObject *parent)
 : QTcpSocket(parent), server(_server), authState(PasswordWrong), acceptsGameListChanges(false)
{
	if (commandHash.isEmpty()) {
		commandHash.insert("ping", new GenericCommandProperties(false, QList<QVariant::Type>(), &ServerSocket::cmdPing));
		commandHash.insert("login", new GenericCommandProperties(false, QList<QVariant::Type>()
			<< QVariant::String
			<< QVariant::String, &ServerSocket::cmdLogin));
		commandHash.insert("chat_list_channels", new GenericCommandProperties(true, QList<QVariant::Type>(), &ServerSocket::cmdChatListChannels));
		commandHash.insert("chat_join_channel", new GenericCommandProperties(true, QList<QVariant::Type>()
			<< QVariant::String, &ServerSocket::cmdChatJoinChannel));
		commandHash.insert("list_games", new GenericCommandProperties(true, QList<QVariant::Type>(), &ServerSocket::cmdListGames));
		commandHash.insert("create_game", new GenericCommandProperties(true, QList<QVariant::Type>()
			<< QVariant::String
			<< QVariant::String
			<< QVariant::Int
			<< QVariant::Bool, &ServerSocket::cmdCreateGame));
		commandHash.insert("join_game", new GenericCommandProperties(true, QList<QVariant::Type>()
			<< QVariant::Int
			<< QVariant::String
			<< QVariant::Bool, &ServerSocket::cmdJoinGame));
			
		commandHash.insert("chat_leave_channel", new ChatCommandProperties(QList<QVariant::Type>(), &ServerSocket::cmdChatLeaveChannel));
		commandHash.insert("chat_say", new ChatCommandProperties(QList<QVariant::Type>()
			<< QVariant::String, &ServerSocket::cmdChatSay));
			
		commandHash.insert("leave_game", new GameCommandProperties(false, true, QList<QVariant::Type>(), &ServerSocket::cmdLeaveGame));
		commandHash.insert("list_players", new GameCommandProperties(false, true, QList<QVariant::Type>(), &ServerSocket::cmdListPlayers));
		commandHash.insert("say", new GameCommandProperties(false, false, QList<QVariant::Type>()
			<< QVariant::String, &ServerSocket::cmdSay));
		commandHash.insert("submit_deck", new GameCommandProperties(false, false, QList<QVariant::Type>(), &ServerSocket::cmdSubmitDeck));
		commandHash.insert("ready_start", new GameCommandProperties(false, false, QList<QVariant::Type>(), &ServerSocket::cmdReadyStart));
		commandHash.insert("shuffle", new GameCommandProperties(true, false, QList<QVariant::Type>(), &ServerSocket::cmdShuffle));
		commandHash.insert("draw_cards", new GameCommandProperties(true, false, QList<QVariant::Type>()
			<< QVariant::Int, &ServerSocket::cmdDrawCards));
		commandHash.insert("reveal_card", new GameCommandProperties(true, false, QList<QVariant::Type>()
			<< QVariant::Int
			<< QVariant::String, &ServerSocket::cmdRevealCard));
		commandHash.insert("move_card", new GameCommandProperties(true, false, QList<QVariant::Type>()
			<< QVariant::Int
			<< QVariant::String
			<< QVariant::String
			<< QVariant::Int
			<< QVariant::Int
			<< QVariant::Bool, &ServerSocket::cmdMoveCard));
		commandHash.insert("create_token", new GameCommandProperties(true, false, QList<QVariant::Type>()
			<< QVariant::String
			<< QVariant::String
			<< QVariant::String
			<< QVariant::Int
			<< QVariant::Int, &ServerSocket::cmdCreateToken));
		commandHash.insert("create_arrow", new GameCommandProperties(true, false, QList<QVariant::Type>()
			<< QVariant::Int
			<< QVariant::String
			<< QVariant::Int
			<< QVariant::Int
			<< QVariant::String
			<< QVariant::Int
			<< QVariant::Int, &ServerSocket::cmdCreateArrow));
		commandHash.insert("delete_arrow", new GameCommandProperties(true, false, QList<QVariant::Type>()
			<< QVariant::Int, &ServerSocket::cmdDeleteArrow));
		commandHash.insert("set_card_attr", new GameCommandProperties(true, false, QList<QVariant::Type>()
			<< QVariant::String
			<< QVariant::Int
			<< QVariant::String
			<< QVariant::String, &ServerSocket::cmdSetCardAttr));
		commandHash.insert("inc_counter", new GameCommandProperties(true, false, QList<QVariant::Type>()
			<< QVariant::String
			<< QVariant::Int, &ServerSocket::cmdIncCounter));
		commandHash.insert("add_counter", new GameCommandProperties(true, false, QList<QVariant::Type>()
			<< QVariant::String
			<< QVariant::Int
			<< QVariant::Int
			<< QVariant::Int, &ServerSocket::cmdAddCounter));
		commandHash.insert("set_counter", new GameCommandProperties(true, false, QList<QVariant::Type>()
			<< QVariant::Int
			<< QVariant::Int, &ServerSocket::cmdSetCounter));
		commandHash.insert("del_counter", new GameCommandProperties(true, false, QList<QVariant::Type>()
			<< QVariant::Int, &ServerSocket::cmdDelCounter));
		commandHash.insert("list_counters", new GameCommandProperties(true, true, QList<QVariant::Type>()
			<< QVariant::Int, &ServerSocket::cmdListCounters));
		commandHash.insert("list_zones", new GameCommandProperties(true, true, QList<QVariant::Type>()
			<< QVariant::Int, &ServerSocket::cmdListZones));
		commandHash.insert("dump_zone", new GameCommandProperties(true, true, QList<QVariant::Type>()
			<< QVariant::Int
			<< QVariant::String
			<< QVariant::Int, &ServerSocket::cmdDumpZone));
		commandHash.insert("stop_dump_zone", new GameCommandProperties(true, true, QList<QVariant::Type>()
			<< QVariant::Int
			<< QVariant::String, &ServerSocket::cmdStopDumpZone));
		commandHash.insert("roll_die", new GameCommandProperties(true, false, QList<QVariant::Type>()
			<< QVariant::Int, &ServerSocket::cmdRollDie));
		commandHash.insert("next_turn", new GameCommandProperties(true, false, QList<QVariant::Type>(), &ServerSocket::cmdNextTurn));
		commandHash.insert("set_active_phase", new GameCommandProperties(true, false, QList<QVariant::Type>()
			<< QVariant::Int, &ServerSocket::cmdSetActivePhase));
		commandHash.insert("dump_all", new GameCommandProperties(false, true, QList<QVariant::Type>(), &ServerSocket::cmdDumpAll));
	}

	remsg = new ReturnMessage(this);
	connect(this, SIGNAL(readyRead()), this, SLOT(readClient()));
	connect(this, SIGNAL(disconnected()), this, SLOT(deleteLater()));
	connect(this, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(catchSocketError(QAbstractSocket::SocketError)));
	setTextModeEnabled(true);
}

ServerSocket::~ServerSocket()
{
	qDebug("ServerSocket destructor");
/*	clearZones();
	// The socket has to be removed from the server's list before it is removed from the game's list
	// so it will not receive the game update event.
	server->removePlayer(this);
	if (game)
		game->removePlayer(this);
	for (int i = 0; i < chatChannels.size(); ++i)
		chatChannels[i]->removePlayer(this);
*/}

void ServerSocket::readClient()
{
	while (canReadLine()) {
		QString line = QString(readLine()).trimmed();
		if (line.isNull())
			break;
			
		qDebug(QString("<<< %1").arg(line).toLatin1());
/*		switch (PlayerStatus) {
			case StatusNormal:
			case StatusReadyStart:
			case StatusPlaying:
				parseCommand(line);
				break;
			case StatusSubmitDeck:
				QString card = line;
				if (card == ".") {
					PlayerStatus = StatusNormal;
					remsg->send(ReturnMessage::ReturnOk);
				} else if (card.startsWith("SB:"))
					SideboardList << card.mid(3);
				else
					DeckList << card;
		}
*/	}
}

ReturnMessage::ReturnCode ServerSocket::cmdPing(const QList<QVariant> &/*params*/)
{
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdLogin(const QList<QVariant> &params)
{
	authState = server->checkUserPassword(params[0].toString(), params[1].toString());
	if (authState == PasswordWrong)
		return ReturnMessage::ReturnPasswordWrong;
	playerName = params[0].toString();
	
	remsg->send(ReturnMessage::ReturnOk);
	
	QStringList loginMessage = server->getLoginMessage();
	for (int i = 0; i < loginMessage.size(); ++i)
		msg("chat|server_message||" + loginMessage[i]);
	
	return ReturnMessage::ReturnNothing;
}

ReturnMessage::ReturnCode ServerSocket::cmdChatListChannels(const QList<QVariant> &/*params*/)
{
	QMapIterator<QString, ChatChannel *> channelIterator(server->getChatChannels());
	while (channelIterator.hasNext()) {
		ChatChannel *c = channelIterator.next().value();
		msg(c->getChannelListLine());
	}
	
	acceptsChatChannelListChanges = true;
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdChatJoinChannel(const QList<QVariant> &params)
{
	QString channelName = params[0].toString();
	if (chatChannels.contains(channelName))
		return ReturnMessage::ReturnContextError;
	
	QMap<QString, ChatChannel *> allChannels = server->getChatChannels();
	ChatChannel *c = allChannels.value(channelName, 0);
	if (!c)
		return ReturnMessage::ReturnNameNotFound;
	
	remsg->send(ReturnMessage::ReturnOk);
	c->addPlayer(this);
	chatChannels.insert(channelName, c);
	return ReturnMessage::ReturnNothing;
}

ReturnMessage::ReturnCode ServerSocket::cmdChatLeaveChannel(ChatChannel *channel, const QList<QVariant> & /*params*/)
{
	chatChannels.remove(channel->getName());
	channel->removePlayer(this);
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdChatSay(ChatChannel *channel, const QList<QVariant> &params)
{
	channel->say(this, params[0].toString());
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdListGames(const QList<QVariant> &/*params*/)
{
	const QList<ServerGame *> &gameList = server->getGames();
	for (int i = 0; i < gameList.size(); ++i)
		msg(gameList[i]->getGameListLine());

	acceptsGameListChanges = true;
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdCreateGame(const QList<QVariant> &params)
{
	QString description = params[0].toString();
	QString password = params[1].toString();
	int maxPlayers = params[2].toInt();
	bool spectatorsAllowed = params[3].toBool();
	
	ServerGame *game = server->createGame(description, password, maxPlayers, spectatorsAllowed, playerName);
	games.insert(game->getGameId(), QPair<ServerGame *, Player *>(game, game->getCreator()));
	
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdJoinGame(const QList<QVariant> &params)
{
	int gameId = params[0].toInt();
	QString password = params[1].toString();
	bool spectator = params[2].toBool();
	
	ServerGame *g = server->getGame(gameId);
	if (!g)
		return ReturnMessage::ReturnNameNotFound;
	
	ReturnMessage::ReturnCode result = g->checkJoin(password, spectator);
	if (result == ReturnMessage::ReturnOk) {
		Player *player = g->addPlayer(playerName, spectator);
		games.insert(gameId, QPair<ServerGame *, Player *>(g, player));
	}
	return result;
}

ReturnMessage::ReturnCode ServerSocket::cmdLeaveGame(ServerGame *game, Player *player, const QList<QVariant> &/*params*/)
{
	game->removePlayer(player);
	return ReturnMessage::ReturnOk;
}

QStringList ServerSocket::listPlayersHelper(ServerGame *game, Player *player)
{
	QStringList result;
	const QList<Player *> &players = game->getPlayers();
	for (int i = 0; i < players.size(); ++i)
		result << QString("%1|%2|%3").arg(players[i]->getPlayerId()).arg(players[i]->getPlayerName()).arg(players[i] == player ? 1 : 0);
	return result;
}

ReturnMessage::ReturnCode ServerSocket::cmdListPlayers(ServerGame *game, Player *player, const QList<QVariant> &/*params*/)
{
	remsg->sendList(listPlayersHelper(game, player));
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdSay(ServerGame *game, Player *player, const QList<QVariant> &params)
{
	game->broadcastEvent(QString("say|%1").arg(params[0].toString()), player);
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdSubmitDeck(ServerGame * /*game*/, Player *player, const QList<QVariant> &/*params*/)
{
	player->setStatus(StatusSubmitDeck);
	player->DeckList.clear();
	player->SideboardList.clear();
	return ReturnMessage::ReturnNothing;
}

ReturnMessage::ReturnCode ServerSocket::cmdReadyStart(ServerGame *game, Player *player, const QList<QVariant> &/*params*/)
{
	player->setStatus(StatusReadyStart);
	game->broadcastEvent(QString("ready_start"), player);
	game->startGameIfReady();
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdShuffle(ServerGame *game, Player *player, const QList<QVariant> &/*params*/)
{
	player->getZones().value("deck")->shuffle();
	game->broadcastEvent("shuffle", player);
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdDrawCards(ServerGame *game, Player *player, const QList<QVariant> &params)
{
	int number = params[0].toInt();
	PlayerZone *deck = player->getZones().value("deck");
	PlayerZone *hand = player->getZones().value("hand");
	if (deck->cards.size() < number)
		return ReturnMessage::ReturnContextError;

	for (int i = 0; i < number; ++i) {
		Card *card = deck->cards.first();
		deck->cards.removeFirst();
		hand->cards.append(card);
		player->privateEvent(QString("draw|%1|%2").arg(card->getId()).arg(card->getName()));
	}

	game->broadcastEvent(QString("draw|%1").arg(number), player);
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdRevealCard(ServerGame *game, Player *player, const QList<QVariant> &params)
{
/*	int cardid = params[0].toInt();
	PlayerZone *zone = getZone(params[1].toString());
	if (!zone)
		return ReturnMessage::ReturnContextError;
	int position = -1;
	Card *card = zone->getCard(cardid, false, &position);
	if (!card)
		return ReturnMessage::ReturnContextError;
	emit broadcastEvent(QString("reveal_card|%1|%2|%3").arg(cardid).arg(zone->getName()).arg(card->getName()), this);
*/	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdMoveCard(ServerGame *game, Player *player, const QList<QVariant> &params)
{
	// ID Karte, Startzone, Zielzone, Koordinaten X, Y, Facedown
	int cardid = params[0].toInt();
	PlayerZone *startzone = player->getZones().value(params[1].toString());
	PlayerZone *targetzone = player->getZones().value(params[2].toString());
	if ((!startzone) || (!targetzone))
		return ReturnMessage::ReturnContextError;

	int position = -1;
	Card *card = startzone->getCard(cardid, true, &position);
	if (!card)
		return ReturnMessage::ReturnContextError;
	int x = params[3].toInt();
	if (x == -1)
		x = targetzone->cards.size();
	int y = 0;
	if (targetzone->hasCoords())
		y = params[4].toInt();
	bool facedown = params[5].toBool();

	targetzone->insertCard(card, x, y);

	bool targetBeingLookedAt = (targetzone->getType() != PlayerZone::HiddenZone) || (targetzone->getCardsBeingLookedAt() > x) || (targetzone->getCardsBeingLookedAt() == -1);
	bool sourceBeingLookedAt = (startzone->getType() != PlayerZone::HiddenZone) || (startzone->getCardsBeingLookedAt() > position) || (startzone->getCardsBeingLookedAt() == -1);

	bool targetHiddenToPlayer = facedown || !targetBeingLookedAt;
	bool targetHiddenToOthers = facedown || (targetzone->getType() != PlayerZone::PublicZone);
	bool sourceHiddenToPlayer = card->getFaceDown() || !sourceBeingLookedAt;
	bool sourceHiddenToOthers = card->getFaceDown() || (startzone->getType() != PlayerZone::PublicZone);
	
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
	player->privateEvent(QString("move_card|%1|%2|%3|%4|%5|%6|%7|%8").arg(privateCardId)
							    .arg(privateCardName)
							    .arg(startzone->getName())
							    .arg(position)
							    .arg(targetzone->getName())
							    .arg(x)
							    .arg(y)
							    .arg(facedown ? 1 : 0));
	
	// Other players do not get to see the start and/or target position of the card if the respective
	// part of the zone is being looked at. The information is not needed anyway because in hidden zones,
	// all cards are equal.
	if ((startzone->getType() == PlayerZone::HiddenZone) && ((startzone->getCardsBeingLookedAt() > position) || (startzone->getCardsBeingLookedAt() == -1)))
		position = -1;
	if ((targetzone->getType() == PlayerZone::HiddenZone) && ((targetzone->getCardsBeingLookedAt() > x) || (targetzone->getCardsBeingLookedAt() == -1)))
		x = -1;
	
	if ((startzone->getType() == PlayerZone::PublicZone) || (targetzone->getType() == PlayerZone::PublicZone))
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
	
	// If the card was moved to another zone, delete all arrows from and to the card
	if (startzone != targetzone) {
		const QList<Player *> &players = game->getPlayers();
		for (int i = 0; i < players.size(); ++i) {
			QList<int> arrowsToDelete;
			QMapIterator<int, Arrow *> arrowIterator(players[i]->getArrows());
			while (arrowIterator.hasNext()) {
				Arrow *arrow = arrowIterator.next().value();
				if ((arrow->getStartCard() == card) || (arrow->getTargetCard() == card))
					arrowsToDelete.append(arrow->getId());
			}
			for (int j = 0; j < arrowsToDelete.size(); ++j)
				players[i]->deleteArrow(arrowsToDelete[j]);
		}
	}
	
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdCreateToken(ServerGame *game, Player *player, const QList<QVariant> &params)
{
	// zone, cardname, powtough, x, y
	// powtough wird erst mal ignoriert
	PlayerZone *zone = player->getZones().value(params[0].toString());
	if (!zone)
		return ReturnMessage::ReturnContextError;
	QString cardname = params[1].toString();
	QString powtough = params[2].toString();
	int x = params[3].toInt();
	int y = params[4].toInt();
	int cardid = player->newCardId();

	Card *card = new Card(cardname, cardid, x, y);
	zone->insertCard(card, x, y);
	game->broadcastEvent(QString("create_token|%1|%2|%3|%4|%5|%6").arg(zone->getName())
									 .arg(cardid)
									 .arg(cardname)
									 .arg(powtough)
									 .arg(x)
									 .arg(y), player);
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdCreateArrow(ServerGame *game, Player *player, const QList<QVariant> &params)
{
	Player *startPlayer = game->getPlayer(params[0].toInt());
	Player *targetPlayer = game->getPlayer(params[3].toInt());
	if (!startPlayer || !targetPlayer)
		return ReturnMessage::ReturnContextError;
	PlayerZone *startZone = startPlayer->getZones().value(params[1].toString());
	PlayerZone *targetZone = targetPlayer->getZones().value(params[4].toString());
	if (!startZone || !targetZone)
		return ReturnMessage::ReturnContextError;
	Card *startCard = startZone->getCard(params[2].toInt(), false);
	Card *targetCard = targetZone->getCard(params[5].toInt(), false);
	if (!startCard || !targetCard || (startCard == targetCard))
		return ReturnMessage::ReturnContextError;
	QMapIterator<int, Arrow *> arrowIterator(player->getArrows());
	while (arrowIterator.hasNext()) {
		Arrow *temp = arrowIterator.next().value();
		if ((temp->getStartCard() == startCard) && (temp->getTargetCard() == targetCard))
			return ReturnMessage::ReturnContextError;
	}
	int color = params[6].toInt();
	
	Arrow *arrow = new Arrow(player->newArrowId(), startCard, targetCard, color);
	player->addArrow(arrow);
	game->broadcastEvent(QString("create_arrow|%1|%2|%3|%4|%5|%6|%7|%8")
		.arg(arrow->getId())
		.arg(startPlayer->getPlayerId())
		.arg(startZone->getName())
		.arg(startCard->getId())
		.arg(targetPlayer->getPlayerId())
		.arg(targetZone->getName())
		.arg(targetCard->getId())
		.arg(color), player
	);
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdDeleteArrow(ServerGame *game, Player *player, const QList<QVariant> &params)
{
	int arrowId = params[0].toInt();
	if (!player->deleteArrow(arrowId))
		return ReturnMessage::ReturnContextError;
	
	game->broadcastEvent(QString("delete_arrow|%1").arg(arrowId), player);
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdSetCardAttr(ServerGame *game, Player *player, const QList<QVariant> &params)
{
	// zone, card id, attr name, attr value
	// card id = -1 => affects all cards in the specified zone
	PlayerZone *zone = player->getZones().value(params[0].toString());
	if (!zone)
		return ReturnMessage::ReturnContextError;
	int cardid = params[1].toInt();
	QString aname = params[2].toString();
	QString avalue = params[3].toString();

	if (cardid == -1) {
		QListIterator<Card *> CardIterator(zone->cards);
		while (CardIterator.hasNext())
			if (!CardIterator.next()->setAttribute(aname, avalue, true))
				return ReturnMessage::ReturnSyntaxError;
	} else {
		Card *card = zone->getCard(cardid, false);
		if (!card)
			return ReturnMessage::ReturnContextError;
		if (!card->setAttribute(aname, avalue, false))
			return ReturnMessage::ReturnSyntaxError;
	}
	game->broadcastEvent(QString("set_card_attr|%1|%2|%3|%4").arg(zone->getName()).arg(cardid).arg(aname).arg(avalue), player);
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdIncCounter(ServerGame *game, Player *player, const QList<QVariant> &params)
{
	const QMap<int, Counter *> counters = player->getCounters();
	Counter *c = counters.value(params[0].toInt(), 0);
	if (!c)
		return ReturnMessage::ReturnContextError;
	int delta = params[1].toInt();
	
	c->setCount(c->getCount() + delta);
	game->broadcastEvent(QString("set_counter|%1|%2").arg(c->getId()).arg(c->getCount()), player);
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdAddCounter(ServerGame *game, Player *player, const QList<QVariant> &params)
{
	QString name = params[0].toString();
	int color = params[1].toInt();
	int radius = params[2].toInt();
	int count = params[3].toInt();
	
	Counter *c = new Counter(player->newCounterId(), name, color, radius, count);
	player->addCounter(c);
	game->broadcastEvent(QString("add_counter|%1|%2|%3|%4|%5").arg(c->getId()).arg(c->getName()).arg(color).arg(radius).arg(count), player);
	
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdSetCounter(ServerGame *game, Player *player, const QList<QVariant> &params)
{
	const QMap<int, Counter *> counters = player->getCounters();
	Counter *c = counters.value(params[0].toInt(), 0);
	if (!c)
		return ReturnMessage::ReturnContextError;
	int count = params[1].toInt();
	
	c->setCount(count);
	game->broadcastEvent(QString("set_counter|%1|%2").arg(c->getId()).arg(count), player);
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdDelCounter(ServerGame *game, Player *player, const QList<QVariant> &params)
{
	int counterId = params[0].toInt();
	if (!player->deleteCounter(counterId))
		return ReturnMessage::ReturnContextError;
	game->broadcastEvent(QString("del_counter|%1").arg(counterId), player);
	return ReturnMessage::ReturnOk;
}

QStringList ServerSocket::listCountersHelper(Player *player)
{
	QStringList result;
	QMapIterator<int, Counter *> i(player->getCounters());
	while (i.hasNext()) {
		Counter *c = i.next().value();
		result << QString("%1|%2|%3|%4|%5|%6").arg(player->getPlayerId()).arg(c->getId()).arg(c->getName()).arg(c->getColor()).arg(c->getRadius()).arg(c->getCount());
	}
	return result;
}

ReturnMessage::ReturnCode ServerSocket::cmdListCounters(ServerGame *game, Player * /*player*/, const QList<QVariant> &params)
{
	int player_id = params[0].toInt();
	Player *player = game->getPlayer(player_id);
	if (!player)
		return ReturnMessage::ReturnContextError;
	
	remsg->sendList(listCountersHelper(player));
	return ReturnMessage::ReturnOk;
}

QStringList ServerSocket::listZonesHelper(Player *player)
{
	QStringList result;
	QMapIterator<QString, PlayerZone *> zoneIterator(player->getZones());
	while (zoneIterator.hasNext()) {
		PlayerZone *zone = zoneIterator.next().value();
		QString typeStr;
		switch (zone->getType()) {
			case PlayerZone::PublicZone: typeStr = "public"; break;
			case PlayerZone::PrivateZone: typeStr = "private"; break;
			case PlayerZone::HiddenZone: typeStr = "hidden"; break;
			default: ;
		}
		result << QString("%1|%2|%3|%4|%5").arg(player->getPlayerId()).arg(zone->getName()).arg(typeStr).arg(zone->hasCoords()).arg(zone->cards.size());
	}
	return result;
}

ReturnMessage::ReturnCode ServerSocket::cmdListZones(ServerGame *game, Player * /*player*/, const QList<QVariant> &params)
{
	int player_id = params[0].toInt();
	Player *player = game->getPlayer(player_id);
	if (!player)
		return ReturnMessage::ReturnContextError;
	
	remsg->sendList(listZonesHelper(player));
	return ReturnMessage::ReturnOk;
}

QStringList ServerSocket::dumpZoneHelper(Player *player, PlayerZone *zone, int number_cards)
{
	QStringList result;
	for (int i = 0; (i < zone->cards.size()) && (i < number_cards || number_cards == -1); i++) {
		Card *tmp = zone->cards[i];
		QString displayedName = tmp->getFaceDown() ? QString() : tmp->getName();
		
		if (zone->getType() != PlayerZone::HiddenZone)
			result << QString("%1|%2|%3|%4|%5|%6|%7|%8|%9|%10").arg(player->getPlayerId())
								    .arg(zone->getName())
								    .arg(tmp->getId())
								    .arg(displayedName)
								    .arg(tmp->getX())
								    .arg(tmp->getY())
								    .arg(tmp->getCounters())
								    .arg(tmp->getTapped())
								    .arg(tmp->getAttacking())
								    .arg(tmp->getAnnotation());
		else {
			zone->setCardsBeingLookedAt(number_cards);
			result << QString("%1|%2|%3|%4||||||").arg(player->getPlayerId()).arg(zone->getName()).arg(i).arg(displayedName);
		}
	}
	return result;
}

ReturnMessage::ReturnCode ServerSocket::cmdDumpZone(ServerGame *game, Player *player, const QList<QVariant> &params)
{
	int player_id = params[0].toInt();
	int number_cards = params[2].toInt();
	Player *otherPlayer = game->getPlayer(player_id);
	if (!otherPlayer)
		return ReturnMessage::ReturnContextError;
	PlayerZone *zone = otherPlayer->getZones().value(params[1].toString());
	if (!zone)
		return ReturnMessage::ReturnContextError;
	if (!((zone->getType() == PlayerZone::PublicZone) || (player == otherPlayer)))
		return ReturnMessage::ReturnContextError;
	
	if (zone->getType() == PlayerZone::HiddenZone)
		game->broadcastEvent(QString("dump_zone|%1|%2|%3").arg(player_id).arg(zone->getName()).arg(number_cards), player);
	
	remsg->sendList(dumpZoneHelper(otherPlayer, zone, number_cards));
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdStopDumpZone(ServerGame *game, Player *player, const QList<QVariant> &params)
{
	Player *otherPlayer = game->getPlayer(params[0].toInt());
	if (!otherPlayer)
		return ReturnMessage::ReturnContextError;
	PlayerZone *zone = otherPlayer->getZones().value(params[1].toString());
	if (!zone)
		return ReturnMessage::ReturnContextError;
	
	if (zone->getType() == PlayerZone::HiddenZone) {
		zone->setCardsBeingLookedAt(0);
		game->broadcastEvent(QString("stop_dump_zone|%1|%2").arg(otherPlayer->getPlayerId()).arg(zone->getName()), player);
	}
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdRollDie(ServerGame *game, Player *player, const QList<QVariant> &params)
{
	int sides = params[0].toInt();
	game->broadcastEvent(QString("roll_die|%1|%2").arg(sides).arg(rng->getNumber(1, sides)), player);
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdNextTurn(ServerGame *game, Player * /*player*/, const QList<QVariant> &/*params*/)
{
	int activePlayer = game->getActivePlayer();
	if (++activePlayer == game->getPlayerCount())
		activePlayer = 0;
	game->setActivePlayer(activePlayer);
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdSetActivePhase(ServerGame *game, Player *player, const QList<QVariant> &params)
{
	int active_phase = params[0].toInt();
	// XXX Überprüfung, ob die Phase existiert...
	if (game->getActivePlayer() != player->getPlayerId())
		return ReturnMessage::ReturnContextError;
	game->setActivePhase(active_phase);
	return ReturnMessage::ReturnOk;
}

QStringList ServerSocket::listArrowsHelper(Player *player)
{
	QStringList result;
	QMapIterator<int, Arrow *> arrowIterator(player->getArrows());
	while (arrowIterator.hasNext()) {
		Arrow *arrow = arrowIterator.next().value();

		Card *startCard = arrow->getStartCard();
		Card *targetCard = arrow->getTargetCard();
		PlayerZone *startZone = startCard->getZone();
		PlayerZone *targetZone = targetCard->getZone();
		Player *startPlayer = startZone->getPlayer();
		Player *targetPlayer = targetZone->getPlayer();
		
		result << QString("%1|%2|%3|%4|%5|%6|%7|%8|%9").arg(player->getPlayerId()).arg(arrow->getId()).arg(startPlayer->getPlayerId()).arg(startZone->getName()).arg(startCard->getId()).arg(targetPlayer->getPlayerId()).arg(targetZone->getName()).arg(targetCard->getId()).arg(arrow->getColor());
	}
	return result;
}

ReturnMessage::ReturnCode ServerSocket::cmdDumpAll(ServerGame *game, Player *player, const QList<QVariant> &/*params*/)
{
	remsg->sendList(listPlayersHelper(game, player), "list_players");
	
	if (game->getGameStarted()) {
		const QList<Player *> &players = game->getPlayers();
		for (int i = 0; i < players.size(); ++i) {
			remsg->sendList(listZonesHelper(players[i]), "list_zones");
			
			QMapIterator<QString, PlayerZone *> zoneIterator(players[i]->getZones());
			while (zoneIterator.hasNext()) {
				PlayerZone *zone = zoneIterator.next().value();
				if ((zone->getType() == PlayerZone::PublicZone) || ((zone->getType() == PlayerZone::PrivateZone) && (player == players[i])))
					remsg->sendList(dumpZoneHelper(players[i], zone, -1), "dump_zone");
			}
			
			remsg->sendList(listCountersHelper(players[i]), "list_counters");
			remsg->sendList(listArrowsHelper(players[i]), "list_arrows");
		}
	}
	remsg->send(ReturnMessage::ReturnOk);
	if (game->getGameStarted()) {
		player->publicEvent(QString("set_active_player|%1").arg(game->getActivePlayer()));
		player->publicEvent(QString("set_active_phase|%1").arg(game->getActivePhase()));
	}

	return ReturnMessage::ReturnNothing;
}

QList<QVariant> ServerSocket::CommandProperties::getParamList(const QStringList &params) const
{
	QList<QVariant> paramList;
	if (paramList.size() != params.size())
		throw ReturnMessage::ReturnSyntaxError;
	for (int j = 0; j < paramTypes.size(); j++)
		switch (paramTypes[j]) {
			case QVariant::String: {
				paramList << QVariant(params[j]);
				break;
			}
			case QVariant::Int: {
				bool ok;
				int temp = params[j].toInt(&ok);
				if (!ok)
					throw ReturnMessage::ReturnSyntaxError;
				paramList << QVariant(temp);
				break;
			}
			case QVariant::Bool: {
				if (params[j] == "1")
					paramList << QVariant(true);
				else if (params[j] == "0")
					paramList << QVariant(false);
				else
					throw ReturnMessage::ReturnSyntaxError;
				break;
			}
			default:
				paramList << QVariant(params[j]);
		}
	return paramList;
}

ReturnMessage::ReturnCode ServerSocket::GenericCommandProperties::exec(ServerSocket *s, QStringList &params)
{
	QList<QVariant> paramList;
	try { paramList = getParamList(params); }
	catch (ReturnMessage::ReturnCode rc) { return rc; }
	
	return (s->*handler)(paramList);
}

ReturnMessage::ReturnCode ServerSocket::ChatCommandProperties::exec(ServerSocket *s, QStringList &params)
{
	if (params.isEmpty())
		return ReturnMessage::ReturnSyntaxError;
	QString channelName = params.takeFirst();
	ChatChannel *channel = s->getServer()->getChatChannels().value(channelName, 0);
	if (!channel)
		return ReturnMessage::ReturnNameNotFound;
	
	QList<QVariant> paramList;
	try { paramList = getParamList(params); }
	catch (ReturnMessage::ReturnCode rc) { return rc; }
	
	return (s->*handler)(channel, paramList);
}

ReturnMessage::ReturnCode ServerSocket::GameCommandProperties::exec(ServerSocket *s, QStringList &params)
{
	if (params.isEmpty())
		return ReturnMessage::ReturnSyntaxError;
	bool ok;
	int gameId = params.takeFirst().toInt(&ok);
	if (!ok)
		return ReturnMessage::ReturnSyntaxError;
	QPair<ServerGame *, Player *> pair = s->getGame(gameId);
	ServerGame *game = pair.first;
	Player *player = pair.second;
	if (!game)
		return ReturnMessage::ReturnNameNotFound;
	
	if (!allowedToSpectator && player->getSpectator())
		return ReturnMessage::ReturnContextError;
	if (needsStartedGame && !game->getGameStarted())
		return ReturnMessage::ReturnContextError;
	
	QList<QVariant> paramList;
	try { paramList = getParamList(params); }
	catch (ReturnMessage::ReturnCode rc) { return rc; }
	
	return (s->*handler)(game, player, paramList);
}

bool ServerSocket::parseCommand(const QString &line)
{
	QStringList params = line.split("|");

	// Extract message id
	bool conv_ok;
	int msgId = params.takeFirst().toInt(&conv_ok);
	if (!conv_ok) {
		remsg->setMsgId(0);
		return remsg->send(ReturnMessage::ReturnSyntaxError);
	}
	remsg->setMsgId(msgId);

	if (params.empty()) {
		remsg->setMsgId(0);
		return remsg->send(ReturnMessage::ReturnSyntaxError);
	}

	// Extract command
	QString cmd = params.takeFirst();
	CommandProperties *cp = commandHash.value(cmd, 0);
	if (!cp)
		return remsg->send(ReturnMessage::ReturnSyntaxError);
	remsg->setCmd(cmd);

	// Check login
	if (cp->getNeedsLogin() && (authState == PasswordWrong))
		return remsg->send(ReturnMessage::ReturnLoginNeeded);

	// Validate parameters and call handler function
	return remsg->send(cp->exec(this, params));
}

void ServerSocket::msg(const QString &s)
{
	qDebug(QString("OUT >>> %3").arg(s).toLatin1());
	QTextStream stream(this);
	stream.setCodec("UTF-8");
	stream << s << endl;
	stream.flush();
	flush();
}

void ServerSocket::initConnection()
{
	msg(QString("welcome|%1|%2").arg(PROTOCOL_VERSION).arg(VERSION_STRING));
}

void ServerSocket::catchSocketError(QAbstractSocket::SocketError socketError)
{
	qDebug(QString("socket error: %1").arg(socketError).toLatin1());
	
	deleteLater();
}

QPair<ServerGame *, Player *> ServerSocket::getGame(int gameId) const
{
	if (games.contains(gameId))
		return games.value(gameId);
	return QPair<ServerGame *, Player *>(0, 0);
}
