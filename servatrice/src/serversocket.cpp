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
#include "abstractrng.h"
#include "chatchannel.h"

QHash<QString, ServerSocket::CommandProperties> ServerSocket::commandHash;

ServerSocket::ServerSocket(Server *_server, QObject *parent)
 : QTcpSocket(parent), server(_server), game(0), spectator(false), nextCardId(0), PlayerStatus(StatusNormal), authState(PasswordWrong), acceptsGameListChanges(false)
{
	if (commandHash.isEmpty()) {
		commandHash.insert("ping", CommandProperties(false, false, false, true, QList<QVariant::Type>(), &ServerSocket::cmdPing));
		commandHash.insert("login", CommandProperties(false, false, false, true, QList<QVariant::Type>()
			<< QVariant::String
			<< QVariant::String, &ServerSocket::cmdLogin));
		commandHash.insert("chat_list_channels", CommandProperties(true, false, false, true, QList<QVariant::Type>(), &ServerSocket::cmdChatListChannels));
		commandHash.insert("chat_join_channel", CommandProperties(true, false, false, true, QList<QVariant::Type>()
			<< QVariant::String, &ServerSocket::cmdChatJoinChannel));
		commandHash.insert("chat_leave_channel", CommandProperties(true, false, false, true,  QList<QVariant::Type>()
			<< QVariant::String, &ServerSocket::cmdChatLeaveChannel));
		commandHash.insert("chat_say", CommandProperties(true, false, false, true,  QList<QVariant::Type>()
			<< QVariant::String
			<< QVariant::String, &ServerSocket::cmdChatSay));
		commandHash.insert("list_games", CommandProperties(true, false, false, true,  QList<QVariant::Type>(), &ServerSocket::cmdListGames));
		commandHash.insert("create_game", CommandProperties(true, false, false, true,  QList<QVariant::Type>()
			<< QVariant::String
			<< QVariant::String
			<< QVariant::Int
			<< QVariant::Bool, &ServerSocket::cmdCreateGame));
		commandHash.insert("join_game", CommandProperties(true, false, false, true,  QList<QVariant::Type>()
			<< QVariant::Int
			<< QVariant::String
			<< QVariant::Bool, &ServerSocket::cmdJoinGame));
		commandHash.insert("leave_game", CommandProperties(true, true, false, true,  QList<QVariant::Type>(), &ServerSocket::cmdLeaveGame));
		commandHash.insert("list_players", CommandProperties(true, true, false, true,  QList<QVariant::Type>(), &ServerSocket::cmdListPlayers));
		commandHash.insert("say", CommandProperties(true, true, false, false, QList<QVariant::Type>()
			<< QVariant::String, &ServerSocket::cmdSay));
		commandHash.insert("submit_deck", CommandProperties(true, true, false, false, QList<QVariant::Type>(), &ServerSocket::cmdSubmitDeck));
		commandHash.insert("ready_start", CommandProperties(true, true, false, false, QList<QVariant::Type>(), &ServerSocket::cmdReadyStart));
		commandHash.insert("shuffle", CommandProperties(true, true, true, false, QList<QVariant::Type>(), &ServerSocket::cmdShuffle));
		commandHash.insert("draw_cards", CommandProperties(true, true, true, false, QList<QVariant::Type>()
			<< QVariant::Int, &ServerSocket::cmdDrawCards));
		commandHash.insert("reveal_card", CommandProperties(true, true, true, false, QList<QVariant::Type>()
			<< QVariant::Int
			<< QVariant::String, &ServerSocket::cmdRevealCard));
		commandHash.insert("move_card", CommandProperties(true, true, true, false, QList<QVariant::Type>()
			<< QVariant::Int
			<< QVariant::String
			<< QVariant::String
			<< QVariant::Int
			<< QVariant::Int
			<< QVariant::Bool, &ServerSocket::cmdMoveCard));
		commandHash.insert("create_token", CommandProperties(true, true, true, false, QList<QVariant::Type>()
			<< QVariant::String
			<< QVariant::String
			<< QVariant::String
			<< QVariant::Int
			<< QVariant::Int, &ServerSocket::cmdCreateToken));
		commandHash.insert("create_arrow", CommandProperties(true, true, true, false, QList<QVariant::Type>()
			<< QVariant::Int
			<< QVariant::String
			<< QVariant::Int
			<< QVariant::Int
			<< QVariant::String
			<< QVariant::Int
			<< QVariant::Int, &ServerSocket::cmdCreateArrow));
		commandHash.insert("delete_arrow", CommandProperties(true, true, true, false, QList<QVariant::Type>()
			<< QVariant::Int, &ServerSocket::cmdDeleteArrow));
		commandHash.insert("set_card_attr", CommandProperties(true, true, true, false, QList<QVariant::Type>()
			<< QVariant::String
			<< QVariant::Int
			<< QVariant::String
			<< QVariant::String, &ServerSocket::cmdSetCardAttr));
		commandHash.insert("inc_counter", CommandProperties(true, true, true, false, QList<QVariant::Type>()
			<< QVariant::String
			<< QVariant::Int, &ServerSocket::cmdIncCounter));
		commandHash.insert("add_counter", CommandProperties(true, true, true, false, QList<QVariant::Type>()
			<< QVariant::String
			<< QVariant::Int
			<< QVariant::Int
			<< QVariant::Int, &ServerSocket::cmdAddCounter));
		commandHash.insert("set_counter", CommandProperties(true, true, true, false, QList<QVariant::Type>()
			<< QVariant::Int
			<< QVariant::Int, &ServerSocket::cmdSetCounter));
		commandHash.insert("del_counter", CommandProperties(true, true, true, false, QList<QVariant::Type>()
			<< QVariant::Int, &ServerSocket::cmdDelCounter));
		commandHash.insert("list_counters", CommandProperties(true, true, true, true, QList<QVariant::Type>()
			<< QVariant::Int, &ServerSocket::cmdListCounters));
		commandHash.insert("list_zones", CommandProperties(true, true, true, true, QList<QVariant::Type>()
			<< QVariant::Int, &ServerSocket::cmdListZones));
		commandHash.insert("dump_zone", CommandProperties(true, true, true, true, QList<QVariant::Type>()
			<< QVariant::Int
			<< QVariant::String
			<< QVariant::Int, &ServerSocket::cmdDumpZone));
		commandHash.insert("stop_dump_zone", CommandProperties(true, true, true, true, QList<QVariant::Type>()
			<< QVariant::Int
			<< QVariant::String, &ServerSocket::cmdStopDumpZone));
		commandHash.insert("roll_die", CommandProperties(true, true, true, false, QList<QVariant::Type>()
			<< QVariant::Int, &ServerSocket::cmdRollDie));
		commandHash.insert("next_turn", CommandProperties(true, true, true, false, QList<QVariant::Type>(), &ServerSocket::cmdNextTurn));
		commandHash.insert("set_active_phase", CommandProperties(true, true, true, false, QList<QVariant::Type>()
			<< QVariant::Int, &ServerSocket::cmdSetActivePhase));
		commandHash.insert("dump_all", CommandProperties(true, true, false, true, QList<QVariant::Type>(), &ServerSocket::cmdDumpAll));
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
	clearZones();
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

int ServerSocket::newCounterId() const
{
	int id = 0;
	QMapIterator<int, Counter *> i(counters);
	while (i.hasNext()) {
		Counter *c = i.next().value();
		if (c->getId() > id)
			id = c->getId();
	}
	return id + 1;
}

int ServerSocket::newArrowId() const
{
	int id = 0;
	QMapIterator<int, Arrow *> i(arrows);
	while (i.hasNext()) {
		Arrow *a = i.next().value();
		if (a->getId() > id)
			id = a->getId();
	}
	return id + 1;
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

void ServerSocket::setupZones()
{
	// Delete existing zones and counters
	clearZones();

	// This may need to be customized according to the game rules.
	// ------------------------------------------------------------------

	// Create zones
	PlayerZone *deck = new PlayerZone(this, "deck", false, PlayerZone::HiddenZone);
	zones << deck;
	PlayerZone *sb = new PlayerZone(this, "sb", false, PlayerZone::HiddenZone);
	zones << sb;
	zones << new PlayerZone(this, "table", true, PlayerZone::PublicZone);
	zones << new PlayerZone(this, "hand", false, PlayerZone::PrivateZone);
	zones << new PlayerZone(this, "grave", false, PlayerZone::PublicZone);
	zones << new PlayerZone(this, "rfg", false, PlayerZone::PublicZone);

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

	QMapIterator<int, Counter *> counterIterator(counters);
	while (counterIterator.hasNext())
		delete counterIterator.next().value();
	counters.clear();
	
	QMapIterator<int, Arrow *> arrowIterator(arrows);
	while (arrowIterator.hasNext())
		delete arrowIterator.next().value();
	arrows.clear();
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

bool ServerSocket::deleteArrow(int arrowId)
{
	Arrow *arrow = arrows.value(arrowId, 0);
	if (!arrow)
		return false;
	arrows.remove(arrowId);
	delete arrow;
	return true;
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
	
	acceptsGameListChanges = false;
	acceptsChatChannelListChanges = false;
	spectator = false;
	leaveGame();
	emit createGame(description, password, maxPlayers, spectatorsAllowed, this);
	
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdJoinGame(const QList<QVariant> &params)
{
	int gameId = params[0].toInt();
	QString password = params[1].toString();
	bool _spectator = params[2].toBool();
	
	ServerGame *g = server->getGame(gameId);
	if (!g)
		return ReturnMessage::ReturnNameNotFound;
	
	ReturnMessage::ReturnCode result = g->checkJoin(password, _spectator);
	if (result == ReturnMessage::ReturnOk) {
		acceptsGameListChanges = false;
		acceptsChatChannelListChanges = false;
		leaveGame();
		spectator = _spectator;
		g->addPlayer(this, spectator);
	}
	return result;
}

ReturnMessage::ReturnCode ServerSocket::cmdLeaveGame(const QList<QVariant> &/*params*/)
{
	leaveGame();
	return ReturnMessage::ReturnOk;
}

QStringList ServerSocket::listPlayersHelper()
{
	QStringList result;
	const QList<ServerSocket *> &players = game->getPlayers();
	for (int i = 0; i < players.size(); ++i)
		result << QString("%1|%2|%3").arg(players[i]->getPlayerId()).arg(players[i]->getPlayerName()).arg(players[i] == this ? 1 : 0);
	return result;
}

ReturnMessage::ReturnCode ServerSocket::cmdListPlayers(const QList<QVariant> &/*params*/)
{
	remsg->sendList(listPlayersHelper());
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
		privateEvent(QString("draw|%1|%2").arg(card->getId()).arg(card->getName()));
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
	privateEvent(QString("move_card|%1|%2|%3|%4|%5|%6|%7|%8").arg(privateCardId)
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
	
	// If the card was moved to another zone, delete all arrows from and to the card
	if (startzone != targetzone) {
		const QList<ServerSocket *> &players = game->getPlayers();
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

ReturnMessage::ReturnCode ServerSocket::cmdCreateArrow(const QList<QVariant> &params)
{
	ServerSocket *startPlayer = game->getPlayer(params[0].toInt());
	ServerSocket *targetPlayer = game->getPlayer(params[3].toInt());
	if (!startPlayer || !targetPlayer)
		return ReturnMessage::ReturnContextError;
	PlayerZone *startZone = startPlayer->getZone(params[1].toString());
	PlayerZone *targetZone = targetPlayer->getZone(params[4].toString());
	if (!startZone || !targetZone)
		return ReturnMessage::ReturnContextError;
	Card *startCard = startZone->getCard(params[2].toInt(), false);
	Card *targetCard = targetZone->getCard(params[5].toInt(), false);
	if (!startCard || !targetCard || (startCard == targetCard))
		return ReturnMessage::ReturnContextError;
	QMapIterator<int, Arrow *> arrowIterator(arrows);
	while (arrowIterator.hasNext()) {
		Arrow *temp = arrowIterator.next().value();
		if ((temp->getStartCard() == startCard) && (temp->getTargetCard() == targetCard))
			return ReturnMessage::ReturnContextError;
	}
	int color = params[6].toInt();
	
	Arrow *arrow = new Arrow(newArrowId(), startCard, targetCard, color);
	arrows.insert(arrow->getId(), arrow);
	emit broadcastEvent(QString("create_arrow|%1|%2|%3|%4|%5|%6|%7|%8")
		.arg(arrow->getId())
		.arg(startPlayer->getPlayerId())
		.arg(startZone->getName())
		.arg(startCard->getId())
		.arg(targetPlayer->getPlayerId())
		.arg(targetZone->getName())
		.arg(targetCard->getId())
		.arg(color), this
	);
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdDeleteArrow(const QList<QVariant> &params)
{
	int arrowId = params[0].toInt();
	if (!deleteArrow(arrowId))
		return ReturnMessage::ReturnContextError;
	
	emit broadcastEvent(QString("delete_arrow|%1").arg(arrowId), this);
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
	Counter *c = counters.value(params[0].toInt(), 0);
	if (!c)
		return ReturnMessage::ReturnContextError;
	int delta = params[1].toInt();
	
	c->setCount(c->getCount() + delta);
	emit broadcastEvent(QString("set_counter|%1|%2").arg(c->getId()).arg(c->getCount()), this);
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdAddCounter(const QList<QVariant> &params)
{
	QString name = params[0].toString();
	int color = params[1].toInt();
	int radius = params[2].toInt();
	int count = params[3].toInt();
	
	Counter *c = new Counter(newCounterId(), name, color, radius, count);
	counters.insert(c->getId(), c);
	emit broadcastEvent(QString("add_counter|%1|%2|%3|%4|%5").arg(c->getId()).arg(c->getName()).arg(color).arg(radius).arg(count), this);
	
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdSetCounter(const QList<QVariant> &params)
{
	Counter *c = counters.value(params[0].toInt(), 0);
	if (!c)
		return ReturnMessage::ReturnContextError;
	int count = params[1].toInt();
	
	c->setCount(count);
	emit broadcastEvent(QString("set_counter|%1|%2").arg(c->getId()).arg(count), this);
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdDelCounter(const QList<QVariant> &params)
{
	int counterId = params[0].toInt();
	Counter *c = counters.value(counterId, 0);
	if (!c)
		return ReturnMessage::ReturnContextError;
	counters.remove(counterId);
	delete c;
	emit broadcastEvent(QString("del_counter|%1").arg(counterId), this);
	return ReturnMessage::ReturnOk;
}

QStringList ServerSocket::listCountersHelper(ServerSocket *player)
{
	QStringList result;
	QMapIterator<int, Counter *> i(player->getCounters());
	while (i.hasNext()) {
		Counter *c = i.next().value();
		result << QString("%1|%2|%3|%4|%5|%6").arg(player->getPlayerId()).arg(c->getId()).arg(c->getName()).arg(c->getColor()).arg(c->getRadius()).arg(c->getCount());
	}
	return result;
}

ReturnMessage::ReturnCode ServerSocket::cmdListCounters(const QList<QVariant> &params)
{
	int player_id = params[0].toInt();
	ServerSocket *player = game->getPlayer(player_id);
	if (!player)
		return ReturnMessage::ReturnContextError;
	
	remsg->sendList(listCountersHelper(player));
	return ReturnMessage::ReturnOk;
}

QStringList ServerSocket::listZonesHelper(ServerSocket *player)
{
	QStringList result;
	const QList<PlayerZone *> &zoneList = player->getZones();
	for (int i = 0; i < zoneList.size(); ++i) {
		QString typeStr;
		switch (zoneList[i]->getType()) {
			case PlayerZone::PublicZone: typeStr = "public"; break;
			case PlayerZone::PrivateZone: typeStr = "private"; break;
			case PlayerZone::HiddenZone: typeStr = "hidden"; break;
			default: ;
		}
		result << QString("%1|%2|%3|%4|%5").arg(player->getPlayerId()).arg(zoneList[i]->getName()).arg(typeStr).arg(zoneList[i]->hasCoords()).arg(zoneList[i]->cards.size());
	}
	return result;
}

ReturnMessage::ReturnCode ServerSocket::cmdListZones(const QList<QVariant> &params)
{
	int player_id = params[0].toInt();
	ServerSocket *player = game->getPlayer(player_id);
	if (!player)
		return ReturnMessage::ReturnContextError;
	
	remsg->sendList(listZonesHelper(player));
	return ReturnMessage::ReturnOk;
}

QStringList ServerSocket::dumpZoneHelper(ServerSocket *player, PlayerZone *zone, int number_cards)
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
	
	if (zone->getType() == PlayerZone::HiddenZone)
		emit broadcastEvent(QString("dump_zone|%1|%2|%3").arg(player_id).arg(zone->getName()).arg(number_cards), this);
	
	remsg->sendList(dumpZoneHelper(player, zone, number_cards));
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

ReturnMessage::ReturnCode ServerSocket::cmdRollDie(const QList<QVariant> &params)
{
	int sides = params[0].toInt();
	emit broadcastEvent(QString("roll_die|%1|%2").arg(sides).arg(server->getRNG()->getNumber(1, sides)), this);
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

QStringList ServerSocket::listArrowsHelper(ServerSocket *player)
{
	QStringList result;
	QMapIterator<int, Arrow *> arrowIterator(player->getArrows());
	while (arrowIterator.hasNext()) {
		Arrow *arrow = arrowIterator.next().value();

		Card *startCard = arrow->getStartCard();
		Card *targetCard = arrow->getTargetCard();
		PlayerZone *startZone = startCard->getZone();
		PlayerZone *targetZone = targetCard->getZone();
		ServerSocket *startPlayer = startZone->getPlayer();
		ServerSocket *targetPlayer = targetZone->getPlayer();
		
		result << QString("%1|%2|%3|%4|%5|%6|%7|%8|%9").arg(player->getPlayerId()).arg(arrow->getId()).arg(startPlayer->getPlayerId()).arg(startZone->getName()).arg(startCard->getId()).arg(targetPlayer->getPlayerId()).arg(targetZone->getName()).arg(targetCard->getId()).arg(arrow->getColor());
	}
	return result;
}

ReturnMessage::ReturnCode ServerSocket::cmdDumpAll(const QList<QVariant> &/*params*/)
{
	remsg->sendList(listPlayersHelper(), "list_players");
	
	if (game->getGameStarted()) {
		const QList<ServerSocket *> &players = game->getPlayers();
		for (int i = 0; i < players.size(); ++i) {
			remsg->sendList(listZonesHelper(players[i]), "list_zones");
			
			const QList<PlayerZone *> &zones = players[i]->getZones();
			for (int j = 0; j < zones.size(); ++j)
				if ((zones[j]->getType() == PlayerZone::PublicZone) || ((zones[j]->getType() == PlayerZone::PrivateZone) && (playerId == players[i]->getPlayerId())))
					remsg->sendList(dumpZoneHelper(players[i], zones[j], -1), "dump_zone");
			
			remsg->sendList(listCountersHelper(players[i]), "list_counters");
			remsg->sendList(listArrowsHelper(players[i]), "list_arrows");
		}
	}
	remsg->send(ReturnMessage::ReturnOk);
	if (game->getGameStarted()) {
		publicEvent(QString("set_active_player|%1").arg(game->getActivePlayer()));
		publicEvent(QString("set_active_phase|%1").arg(game->getActivePhase()));
	}

	return ReturnMessage::ReturnNothing;
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
	if (!commandHash.contains(cmd))
		return remsg->send(ReturnMessage::ReturnSyntaxError);
	remsg->setCmd(cmd);

	const CommandProperties &cp = commandHash[cmd];

	// Check login
	if (cp.getNeedsLogin() && (authState == PasswordWrong))
		return remsg->send(ReturnMessage::ReturnLoginNeeded);
	// Check context
	if (!cp.getAllowedToSpectator() && spectator)
		return remsg->send(ReturnMessage::ReturnContextError);
	if (cp.getNeedsGame() && !game)
		return remsg->send(ReturnMessage::ReturnContextError);
	if (cp.getNeedsStartedGame() && !game->getGameStarted())
		return remsg->send(ReturnMessage::ReturnContextError);
	// Validate parameters
	if (cp.getParamTypes().size() != params.size())
		return remsg->send(ReturnMessage::ReturnSyntaxError);
	QList<QVariant> paramList;
	for (int j = 0; j < cp.getParamTypes().size(); j++)
		switch (cp.getParamTypes()[j]) {
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
	return remsg->send((this->*(cp.getHandler()))(paramList));
}

void ServerSocket::privateEvent(const QString &line)
{
	msg(QString("private|%1|%2|%3").arg(playerId).arg(playerName).arg(line));
}

void ServerSocket::publicEvent(const QString &line, ServerSocket *player)
{
	if (player)
		msg(QString("public|%1|%2|%3").arg(player->getPlayerId()).arg(player->getPlayerName()).arg(line));
	else
		msg(QString("public|||%1").arg(line));
}

void ServerSocket::msg(const QString &s)
{
	qDebug(QString("OUT id=%1 name=%2 >>> %3").arg(playerId).arg(playerName).arg(s).toLatin1());
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
