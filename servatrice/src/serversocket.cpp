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
#include "random.h"

ServerSocket::ServerSocket(Server *_server, QObject *parent)
 : QTcpSocket(parent), server(_server), game(0), authState(PasswordWrong)
{
	remsg = new ReturnMessage(this);
	connect(this, SIGNAL(readyRead()), this, SLOT(readClient()));
	connect(this, SIGNAL(disconnected()), this, SLOT(deleteLater()));
	connect(this, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(catchSocketError(QAbstractSocket::SocketError)));
	setTextModeEnabled(true);
	PlayerStatus = StatusNormal;
}

ServerSocket::~ServerSocket()
{
	qDebug("ServerSocket destructor");
	if (game)
		game->removePlayer(this);
}

int ServerSocket::newCardId()
{
	return nextCardId++;
}

void ServerSocket::setName(const QString &name)
{
	emit broadcastEvent(QString("name|%1|%2").arg(PlayerName).arg(name), this);
	PlayerName = name;
}

PlayerZone *ServerSocket::getZone(const QString &name)
{
	QListIterator<PlayerZone *> ZoneIterator(zones);
	while (ZoneIterator.hasNext()) {
		PlayerZone *temp = ZoneIterator.next();
		if (!temp->getName().compare(name))
			return temp;
	}
	return NULL;
}

Counter *ServerSocket::getCounter(const QString &name)
{
	QListIterator<Counter *> CounterIterator(counters);
	while (CounterIterator.hasNext()) {
		Counter *temp = CounterIterator.next();
		if (!temp->getName().compare(name))
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
	PlayerZone *deck = new PlayerZone("deck", false, false, false, false);
	zones << deck;
	PlayerZone *sb = new PlayerZone("sb", false, false, false, false);
	zones << sb;
	zones << new PlayerZone("table", true, true, false, true);
	zones << new PlayerZone("hand", false, false, true, true);
	zones << new PlayerZone("grave", false, true, false, true);
	zones << new PlayerZone("rfg", false, true, false, true);

	// ------------------------------------------------------------------

	// Assign card ids and create deck from decklist
	QListIterator<QString> DeckIterator(DeckList);
	int i = 0;
	while (DeckIterator.hasNext())
		deck->cards.append(new Card(DeckIterator.next(), i++, 0, 0));
	deck->shuffle(game->rnd);

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
	moveToThread(server->thread());
}

void ServerSocket::readClient()
{
	QTextStream *stream = new QTextStream(this);
	stream->setCodec("UTF-8");
	QStringList lines;

	// Before parsing, everything has to be buffered so that the stream
	// can be deleted in order to avoid problems when moving the object
	// to another thread while this function is still running.
	for (;;) {
		QString line = stream->readLine();
		if (line.isNull())
			break;
		lines << line;
	}
	delete stream;

	QStringListIterator i(lines);
	while (i.hasNext()) {
		QString line = i.next();

		qDebug(QString("<<< %1").arg(line).toLatin1());
		switch (PlayerStatus) {
			case StatusNormal:
			case StatusReadyStart:
			case StatusPlaying:
				parseCommand(line);
				break;
			case StatusSubmitDeck:
				QString card = line;
				if (!card.compare(".")) {
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
	{"login", false, false, false, QList<QVariant::Type>() << QVariant::String
							       << QVariant::String, &ServerSocket::cmdLogin},
	{"list_games", true, false, false, QList<QVariant::Type>(), &ServerSocket::cmdListGames},
	{"create_game", true, false, false, QList<QVariant::Type>() << QVariant::String
								    << QVariant::String
								    << QVariant::String
								    << QVariant::Int, &ServerSocket::cmdCreateGame},
	{"join_game", true, false, false, QList<QVariant::Type>() << QVariant::String
								  << QVariant::String, &ServerSocket::cmdJoinGame},
	{"leave_game", true, true, false, QList<QVariant::Type>(), &ServerSocket::cmdLeaveGame},
	{"list_players", true, true, false, QList<QVariant::Type>(), &ServerSocket::cmdListPlayers},
	{"say", true, true, false, QList<QVariant::Type>() << QVariant::String, &ServerSocket::cmdSay},
	{"submit_deck", true, true, false, QList<QVariant::Type>(), &ServerSocket::cmdSubmitDeck},
	{"ready_start", true, true, false, QList<QVariant::Type>(), &ServerSocket::cmdReadyStart},
	{"shuffle", true, true, true, QList<QVariant::Type>(), &ServerSocket::cmdShuffle},
	{"draw_cards", true, true, true, QList<QVariant::Type>() << QVariant::Int, &ServerSocket::cmdDrawCards},
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
	{"roll_dice", true, true, true, QList<QVariant::Type>() << QVariant::Int, &ServerSocket::cmdRollDice},
	{"set_active_player", true, true, true, QList<QVariant::Type>() << QVariant::Int, &ServerSocket::cmdSetActivePlayer},
	{"set_active_phase", true, true, true, QList<QVariant::Type>() << QVariant::Int, &ServerSocket::cmdSetActivePhase}
};

ReturnMessage::ReturnCode ServerSocket::cmdLogin(const QList<QVariant> &params)
{
	authState = server->checkUserPassword(params[0].toString(), params[1].toString());
	if (authState == PasswordWrong)
		return ReturnMessage::ReturnPasswordWrong;
	PlayerName = params[0].toString();
	
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdListGames(const QList<QVariant> &params)
{
	Q_UNUSED(params);
	QList<ServerGame *> gameList = server->listOpenGames();
	QListIterator<ServerGame *> gameListIterator(gameList);
	QStringList result;
	while (gameListIterator.hasNext()) {
		ServerGame *tmp = gameListIterator.next();
		tmp->mutex->lock();
		result << QString("%1|%2|%3|%4|%5").arg(tmp->name)
						   .arg(tmp->description)
						   .arg(tmp->password == "" ? 0 : 1)
						   .arg(tmp->getPlayerCount())
						   .arg(tmp->maxPlayers);
		tmp->mutex->unlock();
	}
	remsg->sendList(result);
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdCreateGame(const QList<QVariant> &params)
{
	QString name = params[0].toString();
	QString description = params[1].toString();
	QString password = params[2].toString();
	int maxPlayers = params[3].toInt();
	if (server->getGame(name))
		return ReturnMessage::ReturnNameNotFound;
	leaveGame();
	emit createGame(name, description, password, maxPlayers, this);
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdJoinGame(const QList<QVariant> &params)
{
	QString name = params[0].toString();
	QString password = params[1].toString();
	if (!server->checkGamePassword(name, password))
		return ReturnMessage::ReturnPasswordWrong;
	leaveGame();
	emit joinGame(name, this);
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdLeaveGame(const QList<QVariant> &params)
{
	Q_UNUSED(params);
	leaveGame();
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdListPlayers(const QList<QVariant> &params)
{
	Q_UNUSED(params);
	remsg->sendList(game->getPlayerNames());
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdSay(const QList<QVariant> &params)
{
	emit broadcastEvent(QString("say|%1").arg(params[0].toString()), this);
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdSubmitDeck(const QList<QVariant> &params)
{
	Q_UNUSED(params);
	PlayerStatus = StatusSubmitDeck;
	DeckList.clear();
	SideboardList.clear();
	return ReturnMessage::ReturnNothing;
}

ReturnMessage::ReturnCode ServerSocket::cmdReadyStart(const QList<QVariant> &params)
{
	Q_UNUSED(params);
	PlayerStatus = StatusReadyStart;
	emit broadcastEvent(QString("ready_start"), this);
	game->startGameIfReady();
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdShuffle(const QList<QVariant> &params)
{
	Q_UNUSED(params);
	getZone("deck")->shuffle(game->rnd);
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
	int y = 0;
	if (targetzone->hasCoords())
		y = params[4].toInt();
	bool facedown = params[5].toBool();

	targetzone->insertCard(card, x, y);

	QString privateCardName, publicCardName;
	if (facedown)
		card->setId(newCardId());
	if ((!facedown && !card->getFaceDown())
	  || (card->getFaceDown() && !facedown && startzone->isPublic() && targetzone->isPublic()))
		publicCardName = card->getName();
	if ((!facedown && !card->getFaceDown())
	  || (card->getFaceDown() && !facedown && startzone->isPublic() && targetzone->isPublic())
	  || (!facedown && targetzone->isPrivate()))
		privateCardName = card->getName();
	
	card->setFaceDown(facedown);
	msg(QString("private|||move_card|%1|%2|%3|%4|%5|%6|%7|%8").arg(card->getId())
							    .arg(privateCardName)
							    .arg(startzone->getName())
							    .arg(position)
							    .arg(targetzone->getName())
							    .arg(x)
							    .arg(y)
							    .arg(facedown ? 1 : 0));
	if ((startzone->isPublic()) || (targetzone->isPublic()))
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
	if (!(zone->isPublic() || (player_id == PlayerId)))
		return ReturnMessage::ReturnContextError;
		
	QListIterator<Card *> card_iterator(zone->cards);
	QStringList result;
	for (int i = 0; card_iterator.hasNext() && (i < number_cards || number_cards == 0); i++) {
		Card *tmp = card_iterator.next();
		// XXX Face down cards
		if (zone->hasIdAccess())
			result << QString("%1|%2|%3|%4|%5|%6|%7|%8").arg(tmp->getId())
								    .arg(tmp->getName())
								    .arg(tmp->getX())
								    .arg(tmp->getY())
								    .arg(tmp->getCounters())
								    .arg(tmp->getTapped())
								    .arg(tmp->getAttacking())
								    .arg(tmp->getAnnotation());
		else
			result << QString("%1|%2||||||").arg(i).arg(tmp->getName());
	}
	remsg->sendList(result);
	emit broadcastEvent(QString("dump_zone|%1|%2|%3").arg(player_id).arg(zone->getName()).arg(number_cards), this);
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdRollDice(const QList<QVariant> &params)
{
	int sides = params[0].toInt();
	emit broadcastEvent(QString("roll_dice|%1|%2").arg(sides).arg(game->rnd->getNumber(1, sides)), this);
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdSetActivePlayer(const QList<QVariant> &params)
{
	int active_player = params[0].toInt();
	if (!game->getPlayer(active_player))
		return ReturnMessage::ReturnContextError;
	game->setActivePlayer(active_player);
	return ReturnMessage::ReturnOk;
}

ReturnMessage::ReturnCode ServerSocket::cmdSetActivePhase(const QList<QVariant> &params)
{
	int active_phase = params[0].toInt();
	// XXX Überprüfung, ob die Phase existiert...
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

QStringList ServerSocket::listCounters()
{
	QStringList counter_list;
	QListIterator<Counter *> i(counters);
	while (i.hasNext()) {
		Counter *tmp = i.next();
		counter_list << QString("%1|%2").arg(tmp->getName()).arg(tmp->getCount());
	}
	return counter_list;
}

QStringList ServerSocket::listZones()
{
	QStringList zone_list;
	QListIterator<PlayerZone *> i(zones);
	while (i.hasNext()) {
		PlayerZone *tmp = i.next();
		zone_list << QString("%1|%2|%3|%4").arg(tmp->getName()).arg(tmp->isPublic()).arg(tmp->hasCoords()).arg(tmp->cards.size());
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
