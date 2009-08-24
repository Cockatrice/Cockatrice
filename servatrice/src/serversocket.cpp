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
#include "abstractrng.h"
#include "chatchannel.h"

ServerSocket::ServerSocket(Server *_server, QObject *parent)
 : QTcpSocket(parent), server(_server), game(0), PlayerStatus(StatusNormal), authState(PasswordWrong), acceptsGameListChanges(false)
{
	remsg = new ReturnMessage(this);
	connect(this, SIGNAL(readyRead()), this, SLOT(readClient()));
	connect(this, SIGNAL(disconnected()), this, SLOT(deleteLater()));
	connect(this, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(catchSocketError(QAbstractSocket::SocketError)));
	setTextModeEnabled(true);
}

ServerSocket::~ServerSocket()
{
	qDebug("ServerSocket destructor");
	// The socket has to be removed from the server's list before it is removed from the game's list
	// so it will not receive the game update event.
	server->removePlayer(this);
	if (game)
		game->removePlayer(this);
	for (int i = 0; i < chatChannels.size(); ++i)
		chatChannels[i]->removePlayer(this);
}

int ServerSocket::newCardId()
{
	return nextCardId++;
}

PlayerZone *ServerSocket::getZone(const QString &name) const
{
	QListIterator<PlayerZone *> ZoneIterator(zones);
	while (ZoneIterator.hasNext()) {
		PlayerZone *temp = ZoneIterator.next();
		if (temp->getName() == name)
			return temp;
	}
	return NULL;
}

Counter *ServerSocket::getCounter(const QString &name) const
{
	QListIterator<Counter *> CounterIterator(counters);
	while (CounterIterator.hasNext()) {
		Counter *temp = CounterIterator.next();
		if (temp->getName() == name)
			return temp;
	}
	return NULL;
}

void ServerSocket::setupZones()
{
	// Delete existing zones and counters
	clearZones();

	// This may need to be customized according to the game rules.
	// ------------------------------------------------------------------

	// Create zones
	PlayerZone *deck = new PlayerZone("deck", false, PlayerZone::HiddenZone);
	zones << deck;
	PlayerZone *sb = new PlayerZone("sb", false, PlayerZone::HiddenZone);
	zones << sb;
	zones << new PlayerZone("table", true, PlayerZone::PublicZone);
	zones << new PlayerZone("hand", false, PlayerZone::PrivateZone);
	zones << new PlayerZone("grave", false, PlayerZone::PublicZone);
	zones << new PlayerZone("rfg", false, PlayerZone::PublicZone);

	// ------------------------------------------------------------------

	// Assign card ids and create deck from decklist
	QListIterator<QString> DeckIterator(DeckList);
	int i = 0;
	while (DeckIterator.hasNext())
		deck->cards.append(new Card(DeckIterator.next(), i++, 0, 0));
	deck->shuffle(server->getRNG());

	QListIterator<QString> SBIterator(SideboardList);
	while (SBIterator.hasNext())
		sb->cards.append(new Card(SBIterator.next(), i++, 0, 0));

	nextCardId = i;
	
	PlayerStatus = StatusPlaying;
	broadcastEvent(QString("setup_zones|%1|%2").arg(deck->cards.size())
						   .arg(sb->cards.size()), this);
}

void ServerSocket::clearZones()
{
	for (int i = 0; i < zones.size(); i++)
		delete zones.at(i);
	zones.clear();

	for (int i = 0; i < counters.size(); i++)
		delete counters.at(i);
	counters.clear();
}

void ServerSocket::leaveGame()
{
	if (!game)
		return;
	game->removePlayer(this);
	game = 0;
	PlayerStatus = StatusNormal;
	clearZones();
}

void ServerSocket::readClient()
{
	while (canReadLine()) {
		QString line = QString(readLine()).trimmed();
		if (line.isNull())
			break;
			
		qDebug(QString("<<< %1").arg(line).toLatin1());
		switch (PlayerStatus) {
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
	}
}

const ServerSocket::CommandProperties ServerSocket::commandList[ServerSocket::numberCommands] = {
	{"ping", false, false, false, QList<QVariant::Type>(), &ServerSocket::cmdPing},
	{"login", false, false, false, QList<QVariant::Type>() << QVariant::String
							       << QVariant::String, &ServerSocket::cmdLogin},
	{"chat_list_channels", true, false, false, QList<QVariant::Type>(), &ServerSocket::cmdChatListChannels},
	{"chat_join_channel", true, false, false, QList<QVariant::Type>() << QVariant::String, &ServerSocket::cmdChatJoinChannel},
	{"chat_leave_channel", true, false, false, QList<QVariant::Type>() << QVariant::String, &ServerSocket::cmdChatLeaveChannel},
	{"chat_say", true, false, false, QList<QVariant::Type>() << QVariant::String
								 << QVariant::String, &ServerSocket::cmdChatSay},
	{"list_games", true, false, false, QList<QVariant::Type>(), &ServerSocket::cmdListGames},
	{"create_game", true, false, false, QList<QVariant::Type>() << QVariant::String
								    << QVariant::String
								    << QVariant::Int, &ServerSocket::cmdCreateGame},
	{"join_game", true, false, false, QList<QVariant::Type>() << QVariant::Int
								  << QVariant::String, &ServerSocket::cmdJoinGame},
	{"leave_game", true, true, false, QList<QVariant::Type>(), &ServerSocket::cmdLeaveGame},
	{"list_players", true, true, false, QList<QVariant::Type>(), &ServerSocket::cmdListPlayers},
	{"say", true, true, false, QList<QVariant::Type>() << QVariant::String, &ServerSocket::cmdSay},
	{"submit_deck", true, true, false, QList<QVariant::Type>(), &ServerSocket::cmdSubmitDeck},
	{"ready_start", true, true, false, QList<QVariant::Type>(), &ServerSocket::cmdReadyStart},
	{"shuffle", true, true, true, QList<QVariant::Type>(), &ServerSocket::cmdShuffle},
	{"draw_cards", true, true, true, QList<QVariant::Type>() << QVariant::Int, &ServerSocket::cmdDrawCards},
	{"reveal_card", true, true, true, QList<QVariant::Type>() << QVariant::Int
								  << QVariant::String, &ServerSocket::cmdRevealCard},
	{"move_card", true, true, true, QList<QVariant::Type>() << QVariant::Int
								<< QVariant::String
								<< QVariant::String
								<< QVariant::Int
								<< QVariant::Int
								<< QVariant::Bool, &ServerSocket::cmdMoveCard},
	{"create_token", true, true, true, QList<QVariant::Type>() << QVariant::String
								   << QVariant::String
								   << QVariant::String
								   << QVariant::Int
								   << QVariant::Int, &ServerSocket::cmdCreateToken},
	{"set_card_attr", true, true, true, QList<QVariant::Type>() << QVariant::String
								    << QVariant::Int
								    << QVariant::String
								    << QVariant::String, &ServerSocket::cmdSetCardAttr},
	{"inc_counter", true, true, true, QList<QVariant::Type>() << QVariant::String
								  << QVariant::Int, &ServerSocket::cmdIncCounter},
	{"add_counter", true, true, true, QList<QVariant::Type>() << QVariant::String
								  << QVariant::Int
								  << QVariant::Int, &ServerSocket::cmdAddCounter},
	{"set_counter", true, true, true, QList<QVariant::Type>() << QVariant::String
								  << QVariant::Int, &ServerSocket::cmdSetCounter},
	{"del_counter", true, true, true, QList<QVariant::Type>() << QVariant::String, &ServerSocket::cmdDelCounter},
	{"list_counters", true, true, true, QList<QVariant::Type>() << QVariant::Int, &ServerSocket::cmdListCounters},
	{"list_zones", true, true, true, QList<QVariant::Type>() << QVariant::Int, &ServerSocket::cmdListZones},
	{"dump_zone", true, true, true, QList<QVariant::Type>() << QVariant::Int
								<< QVariant::String
								<< QVariant::Int, &ServerSocket::cmdDumpZone},
	{"stop_dump_zone", true, true, true, QList<QVariant::Type>() << QVariant::Int
								     << QVariant::String, &ServerSocket::cmdStopDumpZone},
	{"roll_dice", true, true, true, QList<QVariant::Type>() << QVariant::Int, &ServerSocket::cmdRollDice},
	{"next_turn", true, true, true, QList<QVariant::Type>(), &ServerSocket::cmdNextTurn},
	{"set_active_phase", true, true, true, QList<QVariant::Type>() << QVariant::Int, &ServerSocket::cmdSetActivePhase}
};

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
	QList<ChatChannel *> chatChannelList = server->getChatChannelList();
	for (int i = 0; i < chatChannelList.size(); ++i)
		msg(chatChannelList[i]->getChannelListLine());
	
	acceptsChatChannelListChanges = true;
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdChatJoinChannel(const QList<QVariant> &params)
{
	for (int i = 0; i < chatChannels.size(); ++i)
		if (chatChannels[i]->getName() == params[0])
			return ReturnMessage::ReturnContextError;
			
	QList<ChatChannel *> allChannels = server->getChatChannelList();
	for (int i = 0; i < allChannels.size(); ++i)
		if (allChannels[i]->getName() == params[0]) {
			remsg->send(ReturnMessage::ReturnOk);
			allChannels[i]->addPlayer(this);
			chatChannels << allChannels[i];
			return ReturnMessage::ReturnNothing;
		}
	return ReturnMessage::ReturnNameNotFound;
}

ReturnMessage::ReturnCode ServerSocket::cmdChatLeaveChannel(const QList<QVariant> &params)
{
	for (int i = 0; i < chatChannels.size(); ++i) {
		ChatChannel *c = chatChannels[i];
		if (c->getName() == params[0]) {
			chatChannels.removeAt(i);
			c->removePlayer(this);
			return ReturnMessage::ReturnOk;
		}
	}
	return ReturnMessage::ReturnNameNotFound;
}

ReturnMessage::ReturnCode ServerSocket::cmdChatSay(const QList<QVariant> &params)
{
	for (int i = 0; i < chatChannels.size(); ++i)
		if (chatChannels[i]->getName() == params[0]) {
			chatChannels[i]->say(this, params[1].toString());
			return ReturnMessage::ReturnOk;
		}
	return ReturnMessage::ReturnNameNotFound;
}

ReturnMessage::ReturnCode ServerSocket::cmdListGames(const QList<QVariant> &/*params*/)
{
	QList<ServerGame *> gameList = server->listOpenGames();
	QListIterator<ServerGame *> gameListIterator(gameList);
	while (gameListIterator.hasNext())
		msg(gameListIterator.next()->getGameListLine());

	acceptsGameListChanges = true;
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdCreateGame(const QList<QVariant> &params)
{
	QString description = params[0].toString();
	QString password = params[1].toString();
	int maxPlayers = params[2].toInt();
	acceptsGameListChanges = false;
	acceptsChatChannelListChanges = false;
	leaveGame();
	emit createGame(description, password, maxPlayers, this);
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdJoinGame(const QList<QVariant> &params)
{
	int gameId = params[0].toInt();
	QString password = params[1].toString();
	if (!server->checkGamePassword(gameId, password))
		return ReturnMessage::ReturnPasswordWrong;
	acceptsGameListChanges = false;
	acceptsChatChannelListChanges = false;
	leaveGame();
	emit joinGame(gameId, this);
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdLeaveGame(const QList<QVariant> &/*params*/)
{
	leaveGame();
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdListPlayers(const QList<QVariant> &/*params*/)
{
	remsg->sendList(game->getPlayerNames());
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdSay(const QList<QVariant> &params)
{
	emit broadcastEvent(QString("say|%1").arg(params[0].toString()), this);
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdSubmitDeck(const QList<QVariant> &/*params*/)
{
	PlayerStatus = StatusSubmitDeck;
	DeckList.clear();
	SideboardList.clear();
	return ReturnMessage::ReturnNothing;
}

ReturnMessage::ReturnCode ServerSocket::cmdReadyStart(const QList<QVariant> &/*params*/)
{
	PlayerStatus = StatusReadyStart;
	emit broadcastEvent(QString("ready_start"), this);
	game->startGameIfReady();
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdShuffle(const QList<QVariant> &/*params*/)
{
	getZone("deck")->shuffle(server->getRNG());
	emit broadcastEvent("shuffle", this);
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdDrawCards(const QList<QVariant> &params)
{
	int number = params[0].toInt();
	PlayerZone *deck = getZone("deck");
	PlayerZone *hand = getZone("hand");
	if (deck->cards.size() < number)
		return ReturnMessage::ReturnContextError;

	for (int i = 0; i < number; ++i) {
		Card *card = deck->cards.first();
		deck->cards.removeFirst();
		hand->cards.append(card);
		msg(QString("private|||draw|%1|%2").arg(card->getId()).arg(card->getName()));
	}

	emit broadcastEvent(QString("draw|%1").arg(number), this);
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdRevealCard(const QList<QVariant> &params)
{
	int cardid = params[0].toInt();
	PlayerZone *zone = getZone(params[1].toString());
	if (!zone)
		return ReturnMessage::ReturnContextError;
	int position = -1;
	Card *card = zone->getCard(cardid, false, &position);
	if (!card)
		return ReturnMessage::ReturnContextError;
	emit broadcastEvent(QString("reveal_card|%1|%2|%3").arg(cardid).arg(zone->getName()).arg(card->getName()), this);
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdMoveCard(const QList<QVariant> &params)
{
	// ID Karte, Startzone, Zielzone, Koordinaten X, Y, Facedown
	int cardid = params[0].toInt();
	PlayerZone *startzone = getZone(params[1].toString());
	PlayerZone *targetzone = getZone(params[2].toString());
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
		card->setId(newCardId());
	card->setFaceDown(facedown);
	
	// The player does not get to see which card he moved if it moves between two parts of hidden zones which
	// are not being looked at.
	QString privateCardId = QString::number(card->getId());
	if (!targetBeingLookedAt && !sourceBeingLookedAt) {
		privateCardId = QString();
		privateCardName = QString();
	}
	msg(QString("private|||move_card|%1|%2|%3|%4|%5|%6|%7|%8").arg(privateCardId)
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
		emit broadcastEvent(QString("move_card|%1|%2|%3|%4|%5|%6|%7|%8").arg(card->getId())
									 .arg(publicCardName)
									 .arg(startzone->getName())
									 .arg(position)
									 .arg(targetzone->getName())
									 .arg(x)
									 .arg(y)
									 .arg(facedown ? 1 : 0), this);
	else
		emit broadcastEvent(QString("move_card|||%1|%2|%3|%4|%5|0").arg(startzone->getName())
								     .arg(position)
								     .arg(targetzone->getName())
								     .arg(x)
								     .arg(y), this);
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdCreateToken(const QList<QVariant> &params)
{
	// zone, cardname, powtough, x, y
	// powtough wird erst mal ignoriert
	PlayerZone *zone = getZone(params[0].toString());
	if (!zone)
		return ReturnMessage::ReturnContextError;
	QString cardname = params[1].toString();
	QString powtough = params[2].toString();
	int x = params[3].toInt();
	int y = params[4].toInt();
	int cardid = newCardId();

	Card *card = new Card(cardname, cardid, x, y);
	zone->insertCard(card, x, y);
	emit broadcastEvent(QString("create_token|%1|%2|%3|%4|%5|%6").arg(zone->getName())
									 .arg(cardid)
									 .arg(cardname)
									 .arg(powtough)
									 .arg(x)
									 .arg(y), this);
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdSetCardAttr(const QList<QVariant> &params)
{
	// zone, card id, attr name, attr value
	// card id = -1 => affects all cards in the specified zone
	PlayerZone *zone = getZone(params[0].toString());
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
	emit broadcastEvent(QString("set_card_attr|%1|%2|%3|%4").arg(zone->getName()).arg(cardid).arg(aname).arg(avalue), this);
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdIncCounter(const QList<QVariant> &params)
{
	Counter *c = getCounter(params[0].toString());
	if (!c)
		return ReturnMessage::ReturnContextError;
	int delta = params[1].toInt();
	
	c->setCount(c->getCount() + delta);
	emit broadcastEvent(QString("set_counter|%1|%2").arg(c->getName()).arg(c->getCount()), this);
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdAddCounter(const QList<QVariant> &params)
{
	QString name = params[0].toString();
	if (getCounter(name))
		return ReturnMessage::ReturnContextError;
	int color = params[1].toInt();
	int count = params[2].toInt();
	
	Counter *c = new Counter(name, color, count);
	counters << c;
	emit broadcastEvent(QString("add_counter|%1|%2|%3").arg(c->getName()).arg(color).arg(count), this);
	
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdSetCounter(const QList<QVariant> &params)
{
	Counter *c = getCounter(params[0].toString());
	if (!c)
		return ReturnMessage::ReturnContextError;
	int count = params[1].toInt();
	
	c->setCount(count);
	emit broadcastEvent(QString("set_counter|%1|%2").arg(c->getName()).arg(count), this);
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdDelCounter(const QList<QVariant> &params)
{
	Counter *c = getCounter(params[0].toString());
	if (!c)
		return ReturnMessage::ReturnContextError;
	delete c;
	counters.removeAt(counters.indexOf(c));
	emit broadcastEvent(QString("del_counter|%1").arg(params[0].toString()), this);
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdListCounters(const QList<QVariant> &params)
{
	int player_id = params[0].toInt();
	ServerSocket *player = game->getPlayer(player_id);
	if (!player)
		return ReturnMessage::ReturnContextError;
	remsg->sendList(player->listCounters());
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdListZones(const QList<QVariant> &params)
{
	int player_id = params[0].toInt();
	ServerSocket *player = game->getPlayer(player_id);
	if (!player)
		return ReturnMessage::ReturnContextError;
	remsg->sendList(player->listZones());
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdDumpZone(const QList<QVariant> &params)
{
	int player_id = params[0].toInt();
	int number_cards = params[2].toInt();
	ServerSocket *player = game->getPlayer(player_id);
	if (!player)
		return ReturnMessage::ReturnContextError;
	PlayerZone *zone = player->getZone(params[1].toString());
	if (!zone)
		return ReturnMessage::ReturnContextError;
	if (!((zone->getType() == PlayerZone::PublicZone) || (player_id == playerId)))
		return ReturnMessage::ReturnContextError;
		
	QListIterator<Card *> card_iterator(zone->cards);
	QStringList result;
	for (int i = 0; card_iterator.hasNext() && (i < number_cards || number_cards == -1); i++) {
		Card *tmp = card_iterator.next();
		// XXX Face down cards
		if (zone->getType() != PlayerZone::HiddenZone)
			result << QString("%1|%2|%3|%4|%5|%6|%7|%8").arg(tmp->getId())
								    .arg(tmp->getName())
								    .arg(tmp->getX())
								    .arg(tmp->getY())
								    .arg(tmp->getCounters())
								    .arg(tmp->getTapped())
								    .arg(tmp->getAttacking())
								    .arg(tmp->getAnnotation());
		else {
			zone->setCardsBeingLookedAt(number_cards);
			result << QString("%1|%2||||||").arg(i).arg(tmp->getName());
		}
	}
	remsg->sendList(result);
	emit broadcastEvent(QString("dump_zone|%1|%2|%3").arg(player_id).arg(zone->getName()).arg(number_cards), this);
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdStopDumpZone(const QList<QVariant> &params)
{
	ServerSocket *player = game->getPlayer(params[0].toInt());
	if (!player)
		return ReturnMessage::ReturnContextError;
	PlayerZone *zone = player->getZone(params[1].toString());
	if (!zone)
		return ReturnMessage::ReturnContextError;
	
	if (zone->getType() == PlayerZone::HiddenZone) {
		zone->setCardsBeingLookedAt(0);
		emit broadcastEvent(QString("stop_dump_zone|%1|%2").arg(player->getPlayerId()).arg(zone->getName()), this);
	}
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdRollDice(const QList<QVariant> &params)
{
	int sides = params[0].toInt();
	emit broadcastEvent(QString("roll_dice|%1|%2").arg(sides).arg(server->getRNG()->getNumber(1, sides)), this);
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdNextTurn(const QList<QVariant> &/*params*/)
{
	int activePlayer = game->getActivePlayer();
	if (++activePlayer == game->getPlayerCount())
		activePlayer = 0;
	game->setActivePlayer(activePlayer);
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdSetActivePhase(const QList<QVariant> &params)
{
	int active_phase = params[0].toInt();
	// XXX Überprüfung, ob die Phase existiert...
	if (game->getActivePlayer() != playerId)
		return ReturnMessage::ReturnContextError;
	game->setActivePhase(active_phase);
	return ReturnMessage::ReturnOk;
}

bool ServerSocket::parseCommand(QString line)
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
	remsg->setCmd(cmd);

	for (int i = 0; i < numberCommands; i++)
		if (commandList[i].name == cmd) {
			// Check login
			if (commandList[i].needsLogin && (authState == PasswordWrong))
				return remsg->send(ReturnMessage::ReturnLoginNeeded);
			// Check context
			if (commandList[i].needsGame && !game)
				return remsg->send(ReturnMessage::ReturnContextError);
			if (commandList[i].needsStartedGame && !game->getGameStarted())
				return remsg->send(ReturnMessage::ReturnContextError);
			// Validate parameters
			if (commandList[i].paramTypes.size() != params.size())
				return remsg->send(ReturnMessage::ReturnSyntaxError);
			QList<QVariant> paramList;
			for (int j = 0; j < commandList[i].paramTypes.size(); j++)
				switch (commandList[i].paramTypes[j]) {
					case QVariant::String: {
						paramList << QVariant(params[j]);
						break;
					}
					case QVariant::Int: {
						bool ok;
						int temp = params[j].toInt(&ok);
						if (!ok)
							return remsg->send(ReturnMessage::ReturnSyntaxError);
						paramList << QVariant(temp);
						break;
					}
					case QVariant::Bool: {
						if (params[j] == "1")
							paramList << QVariant(true);
						else if (params[j] == "0")
							paramList << QVariant(false);
						else
							return remsg->send(ReturnMessage::ReturnSyntaxError);
						break;
					}
					default:
						paramList << QVariant(params[j]);
				}
			// Call handler function
			CommandHandler handler = commandList[i].handler;
			return remsg->send((this->*handler)(paramList));
		}
	return remsg->send(ReturnMessage::ReturnSyntaxError);
}

PlayerStatusEnum ServerSocket::getStatus()
{
	return PlayerStatus;
}

void ServerSocket::setStatus(PlayerStatusEnum status)
{
	PlayerStatus = status;
}

void ServerSocket::setGame(ServerGame *g)
{
	game = g;
}

QStringList ServerSocket::listCounters() const
{
	QStringList counter_list;
	QListIterator<Counter *> i(counters);
	while (i.hasNext()) {
		Counter *tmp = i.next();
		counter_list << QString("%1|%2").arg(tmp->getName()).arg(tmp->getCount());
	}
	return counter_list;
}

QStringList ServerSocket::listZones() const
{
	QStringList zone_list;
	QListIterator<PlayerZone *> i(zones);
	while (i.hasNext()) {
		PlayerZone *tmp = i.next();
		zone_list << QString("%1|%2|%3|%4").arg(tmp->getName()).arg(tmp->getType() == PlayerZone::PublicZone ? 1 : 0).arg(tmp->hasCoords()).arg(tmp->cards.size());
	}
	return zone_list;
}

void ServerSocket::msg(const QString &s)
{
	qDebug(QString(">>> %1").arg(s).toLatin1());
	QTextStream stream(this);
	stream.setCodec("UTF-8");
	stream << s << endl;
	stream.flush();
	flush();
}

void ServerSocket::initConnection()
{
	msg(QString("welcome||%1").arg(VERSION_STRING));
	msg("welcome||.");
}

void ServerSocket::catchSocketError(QAbstractSocket::SocketError socketError)
{
	qDebug(QString("socket error: %1").arg(socketError).toLatin1());
	
	deleteLater();
}
