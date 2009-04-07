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
#include "testserver.h"
#include "testserversocket.h"
#include "version.h"

TestServerSocket::TestServerSocket(TestServer *_server, QObject *parent)
 : QTcpSocket(parent), server(_server)
{
	remsg = new ReturnMessage(this);
	connect(this, SIGNAL(readyRead()), this, SLOT(readClient()));
	connect(this, SIGNAL(disconnected()), this, SLOT(deleteLater()));
	connect(this, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(catchSocketError(QAbstractSocket::SocketError)));
	setTextModeEnabled(true);
	PlayerStatus = StatusNormal;
	game = 0;
}

TestServerSocket::~TestServerSocket()
{
	qDebug("TestServerSocket destructor");
	if (game)
		game->removePlayer(this);
}

int TestServerSocket::newCardId()
{
	return nextCardId++;
}

void TestServerSocket::setName(const QString &name)
{
	emit broadcastEvent(QString("name|%1|%2").arg(PlayerName).arg(name), this);
	PlayerName = name;
}

PlayerZone *TestServerSocket::getZone(const QString &name)
{
	QListIterator<PlayerZone *> ZoneIterator(zones);
	while (ZoneIterator.hasNext()) {
		PlayerZone *temp = ZoneIterator.next();
		if (!temp->getName().compare(name))
			return temp;
	}
	return NULL;
}

Counter *TestServerSocket::getCounter(const QString &name)
{
	QListIterator<Counter *> CounterIterator(counters);
	while (CounterIterator.hasNext()) {
		Counter *temp = CounterIterator.next();
		if (!temp->getName().compare(name))
			return temp;
	}
	return NULL;
}

void TestServerSocket::setupZones()
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

	// Create life counter
	Counter *life = new Counter("life", 20);
	counters << life;

	// ------------------------------------------------------------------

	// Assign card ids and create deck from decklist
	QListIterator<QString> DeckIterator(DeckList);
	int i = 0;
	while (DeckIterator.hasNext())
		deck->cards.append(new TestCard(DeckIterator.next(), i++, 0, 0));
	deck->shuffle(game->rnd);

	QListIterator<QString> SBIterator(SideboardList);
	while (SBIterator.hasNext())
		sb->cards.append(new TestCard(SBIterator.next(), i++, 0, 0));

	nextCardId = i;
	
	PlayerStatus = StatusPlaying;
	broadcastEvent(QString("setup_zones|%1|%2|%3").arg(getCounter("life")->getCount())
						      .arg(deck->cards.size())
						      .arg(getZone("sb")->cards.size()), this);
}

void TestServerSocket::clearZones()
{
	for (int i = 0; i < zones.size(); i++)
		delete zones.at(i);
	zones.clear();

	for (int i = 0; i < counters.size(); i++)
		delete counters.at(i);
	counters.clear();
}

void TestServerSocket::leaveGame()
{
	if (!game)
		return;
	game->removePlayer(this);
	game = 0;
	PlayerStatus = StatusNormal;
	clearZones();
	moveToThread(server->thread());
}

void TestServerSocket::readClient()
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
					remsg->send();
				} else if (card.startsWith("SB:"))
					SideboardList << card.mid(3);
				else
					DeckList << card;
		}
	}
}

bool TestServerSocket::parseCommand(QString line)
{
	QStringList params = line.split("|");

	// Extract message id
	bool conv_ok;
	int msgId = params.takeFirst().toInt(&conv_ok);
	if (!conv_ok) {
		remsg->setMsgId(0);
		return remsg->send("syntax", false);
	}
	remsg->setMsgId(msgId);

	if (params.empty()) {
		remsg->setMsgId(0);
		return remsg->send("syntax", false);
	}

	// Extract command
	QString cmd = params.takeFirst();
	remsg->setCmd(cmd);

	// parse command

	if (!cmd.compare("list_games", Qt::CaseInsensitive)) {
		remsg->send();
		QList<TestServerGame *> gameList = server->listOpenGames();
		QListIterator<TestServerGame *> gameListIterator(gameList);
		QStringList result;
		while (gameListIterator.hasNext()) {
			TestServerGame *tmp = gameListIterator.next();
			tmp->mutex->lock();
			result << QString("%1|%2|%3|%4|%5").arg(tmp->name)
							   .arg(tmp->description)
							   .arg(tmp->password == "" ? 0 : 1)
							   .arg(tmp->getPlayerCount())
							   .arg(tmp->maxPlayers);
			tmp->mutex->unlock();
		}
		remsg->sendList(result);
	} else if (!cmd.compare("create_game", Qt::CaseInsensitive)) {
		if (params.size() != 4)
			return remsg->send("syntax", false);
		QString name = params[0];
		QString description = params[1];
		QString password = params[2];
		int maxPlayers = params[3].toInt();
		if (server->getGame(name))
			return remsg->send("name_not_found", false);
		remsg->send();
		leaveGame();

		emit createGame(name, description, password, maxPlayers, this);
	} else if (!cmd.compare("join_game", Qt::CaseInsensitive)) {
		if (params.size() != 2)
			return remsg->send("syntax", false);
		QString name = params[0];
		QString password = params[1];
		if (!server->checkGamePassword(name, password))
			return remsg->send("wrong_password", false);
		remsg->send();
		leaveGame();

		emit joinGame(name, this);
	} else if (!cmd.compare("leave_game", Qt::CaseInsensitive)) {
		remsg->send();
		leaveGame();
	} else if (!cmd.compare("set_name", Qt::CaseInsensitive)) {
		if (params.size() != 1)
			return remsg->send("syntax", false);
		remsg->send();
		setName(params[0]);
	} else if (!cmd.compare("list_players", Qt::CaseInsensitive)) {
		if (!game)
			return remsg->send("game_state", false);

		remsg->send();
		remsg->sendList(game->getPlayerNames());
	} else if (!cmd.compare("say", Qt::CaseInsensitive)) {
		if (params.size() != 1)
			return remsg->send("syntax", false);
		remsg->send();
		emit broadcastEvent(QString("say|%1").arg(params[0]), this);
	} else if (!cmd.compare("submit_deck", Qt::CaseInsensitive)) {
		PlayerStatus = StatusSubmitDeck;
		DeckList.clear();
		SideboardList.clear();
	} else if (!cmd.compare("shuffle", Qt::CaseInsensitive)) {
		if (!game)
			return remsg->send("game_state", false);
		if (!game->getGameStarted())
			return remsg->send("game_state", false);
		getZone("deck")->shuffle(game->rnd);
		remsg->send();
		emit broadcastEvent("shuffle", this);
	} else if (!cmd.compare("ready_start", Qt::CaseInsensitive)) {
		if (!game)
			return remsg->send("game_state", false);
		remsg->send();
		PlayerStatus = StatusReadyStart;
		emit broadcastEvent(QString("ready_start"), this);
		game->startGameIfReady();
	} else if (!cmd.compare("draw_cards", Qt::CaseInsensitive)) {
		if (params.size() != 1)
			return remsg->send("syntax", false);
		bool ok;
		int number = params[0].toInt(&ok);
		if (!ok)
			return remsg->send("syntax", false);
		PlayerZone *deck = getZone("deck");
		PlayerZone *hand = getZone("hand");

		if (deck->cards.size() < number)
			return remsg->send("game_state", false);
		remsg->send();

		for (int i = 0; i < number; ++i) {
			TestCard *card = deck->cards.first();
			deck->cards.removeFirst();
			hand->cards.append(card);
			msg(QString("private|||draw|%1|%2").arg(card->getId()).arg(card->getName()));
		}

		emit broadcastEvent(QString("draw|%1").arg(number), this);
	} else if (!cmd.compare("move_card", Qt::CaseInsensitive)) {
		// ID Karte, Startzone, Zielzone, Koordinaten X, Y, Facedown
		if (params.size() != 6)
			return remsg->send("syntax", false);
		bool ok;
		int cardid = params[0].toInt(&ok);
		if (!ok)
			return remsg->send("syntax", false);
		PlayerZone *startzone = getZone(params[1]);
		PlayerZone *targetzone = getZone(params[2]);
		if ((!startzone) || (!targetzone))
			return remsg->send("game_state", false);

		int position = -1;
		TestCard *card = startzone->getCard(cardid, true, &position);
		if (!card)
			return remsg->send("game_state", false);
		int x = params[3].toInt(&ok);
		if (!ok)
			return remsg->send("syntax", false);
		int y = 0;
		if (targetzone->hasCoords()) {
			y = params[4].toInt(&ok);
			if (!ok)
				return remsg->send("syntax", false);
		}
		bool facedown = params[5].toInt(&ok);
		if (!ok)
			return remsg->send("syntax", false);
		remsg->send();
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
	} else if (!cmd.compare("create_token", Qt::CaseInsensitive)) {
		// zone, cardname, powtough, x, y
		// powtough wird erst mal ignoriert
		if (params.size() != 5)
			return remsg->send("syntax", false);
		PlayerZone *zone = getZone(params[0]);
		if (!zone)
			return remsg->send("game_state", false);
		QString cardname = params[1];
		int x = params[3].toInt();
		int y = params[4].toInt();
		int cardid = newCardId();
		QString powtough = params[2];

		remsg->send();
		TestCard *card = new TestCard(cardname, cardid, x, y);
		zone->insertCard(card, x, y);
		emit broadcastEvent(QString("create_token|%1|%2|%3|%4|%5|%6").arg(zone->getName())
																	 .arg(cardid)
																	 .arg(cardname)
																	 .arg(powtough)
																	 .arg(x)
																	 .arg(y), this);
	} else if (!cmd.compare("set_card_attr", Qt::CaseInsensitive)) {
		if (params.size() != 4)
			return remsg->send("syntax", false);
		// zone, card id, attr name, attr value
		// card id = -1 => affects all cards in the specified zone
		PlayerZone *zone = getZone(params[0]);
		if (!zone)
			return remsg->send("game_state", false);
		bool ok;
		int cardid = params[1].toInt(&ok);
		if (!ok)
			return remsg->send("syntax", false);

		if (cardid == -1) {
			QListIterator<TestCard *> CardIterator(zone->cards);
			while (CardIterator.hasNext())
				if (!CardIterator.next()->setAttribute(params[2], params[3], true))
					return remsg->send("syntax", false);
		} else {
			TestCard *card = zone->getCard(cardid, false);
			if (!card)
				return remsg->send("game_state", false);
			if (!card->setAttribute(params[2], params[3], false))
				return remsg->send("syntax", false);
		}
		remsg->send();
		emit broadcastEvent(QString("set_card_attr|%1|%2|%3|%4").arg(zone->getName()).arg(cardid).arg(params[2]).arg(params[3]), this);
	} else if (!cmd.compare("inc_counter", Qt::CaseInsensitive)) {
		if (params.size() != 2)
			return remsg->send("syntax", false);
		Counter *c = getCounter(params[0]);
		if (!c)
			return remsg->send("game_state", false);
		bool ok;
		int delta = params[1].toInt(&ok);
		if (!ok)
			return remsg->send("syntax", false);
		remsg->send();
		c->setCount(c->getCount() + delta);
		emit broadcastEvent(QString("set_counter|%1|%2").arg(params[0]).arg(c->getCount()), this);
	} else if (!cmd.compare("set_counter", Qt::CaseInsensitive)) {
		if (params.size() != 2)
			return remsg->send("syntax", false);
		Counter *c = getCounter(params[0]);
		bool ok;
		int count = params[1].toInt(&ok);
		if (!ok)
			return remsg->send("syntax", false);
		if (!c) {
			c = new Counter(params[0], count);
			counters << c;
		} else
			c->setCount(count);
		remsg->send();
		emit broadcastEvent(QString("set_counter|%1|%2").arg(params[0]).arg(count), this);
	} else if (!cmd.compare("del_counter", Qt::CaseInsensitive)) {
		if (params.size() != 1)
			return remsg->send("syntax", false);
		Counter *c = getCounter(params[0]);
		if (!c)
			return remsg->send("game_state", false);
		delete c;
		counters.removeAt(counters.indexOf(c));
		remsg->send();
		emit broadcastEvent(QString("del_counter|%1").arg(params[0]), this);
	} else if (!cmd.compare("list_counters", Qt::CaseInsensitive)) {
		if (params.size() != 1)
			return remsg->send("syntax", false);
		bool ok;
		int player_id = params[0].toInt(&ok);
		if (!ok)
			return remsg->send("syntax", false);
		TestServerSocket *player = game->getPlayer(player_id);
		if (!player)
			return remsg->send("game_state", false);
		remsg->send();
		remsg->sendList(player->listCounters());
	} else if (!cmd.compare("list_zones", Qt::CaseInsensitive)) {
		if (params.size() != 1)
			return remsg->send("syntax", false);
		bool ok;
		int player_id = params[0].toInt(&ok);
		if (!ok)
			return remsg->send("syntax", false);
		TestServerSocket *player = game->getPlayer(player_id);
		if (!player)
			return remsg->send("game_state", false);
		remsg->send();
		remsg->sendList(player->listZones());
	} else if (!cmd.compare("dump_zone", Qt::CaseInsensitive)) {
		if (params.size() != 3)
			return remsg->send("syntax", false);
		bool ok;
		int player_id = params[0].toInt(&ok);
		if (!ok)
			return remsg->send("syntax", false);
		int number_cards = params[2].toInt(&ok);
		if (!ok)
			return remsg->send("syntax", false);
		TestServerSocket *player = game->getPlayer(player_id);
		if (!player)
			return remsg->send("game_state", false);
		PlayerZone *zone = player->getZone(params[1]);
		if (!zone)
			return remsg->send("game_state", false);
		if (!(zone->isPublic() || (player_id == PlayerId)))
			return remsg->send("game_state", false);
		remsg->send();
		QListIterator<TestCard *> card_iterator(zone->cards);
		QStringList result;
		for (int i = 0; card_iterator.hasNext() && (i < number_cards || number_cards == 0); i++) {
			TestCard *tmp = card_iterator.next();
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
	} else if (!cmd.compare("roll_dice", Qt::CaseInsensitive)) {
		if (params.size() != 1)
			return remsg->send("syntax", false);
		bool ok;
		int sides = params[0].toInt(&ok);
		if (!ok)
			return remsg->send("syntax", false);
		remsg->send();
		emit broadcastEvent(QString("roll_dice|%1|%2").arg(sides).arg(game->rnd->getNumber(1, sides)), this);
	} else if (!cmd.compare("set_active_player", Qt::CaseInsensitive)) {
		if (params.size() != 1)
			return remsg->send("syntax", false);
		bool ok;
		int active_player = params[0].toInt(&ok);
		if (!ok)
			return remsg->send("syntax", false);
		if (!game->getPlayer(active_player))
			return remsg->send("game_state", false);
		remsg->send();
		game->setActivePlayer(active_player);
	} else if (!cmd.compare("set_active_phase", Qt::CaseInsensitive)) {
		if (params.size() != 1)
			return remsg->send("syntax", false);
		bool ok;
		int active_phase = params[0].toInt(&ok);
		if (!ok)
			return remsg->send("syntax", false);
		// XXX Überprüfung, ob die Phase existiert...
		remsg->send();
		game->setActivePhase(active_phase);
	} else
		return remsg->send("syntax", false);
	return true;
}

PlayerStatusEnum TestServerSocket::getStatus()
{
	return PlayerStatus;
}

void TestServerSocket::setStatus(PlayerStatusEnum status)
{
	PlayerStatus = status;
}

void TestServerSocket::setGame(TestServerGame *g)
{
	game = g;
}

QStringList TestServerSocket::listCounters()
{
	QStringList counter_list;
	QListIterator<Counter *> i(counters);
	while (i.hasNext()) {
		Counter *tmp = i.next();
		counter_list << QString("%1|%2").arg(tmp->getName()).arg(tmp->getCount());
	}
	return counter_list;
}

QStringList TestServerSocket::listZones()
{
	QStringList zone_list;
	QListIterator<PlayerZone *> i(zones);
	while (i.hasNext()) {
		PlayerZone *tmp = i.next();
		zone_list << QString("%1|%2|%3|%4").arg(tmp->getName()).arg(tmp->isPublic()).arg(tmp->hasCoords()).arg(tmp->cards.size());
	}
	return zone_list;
}

void TestServerSocket::msg(const QString &s)
{
	qDebug(QString(">>> %1").arg(s).toLatin1());
	QTextStream stream(this);
	stream.setCodec("UTF-8");
	stream << s << endl;
	stream.flush();
	flush();
}

void TestServerSocket::initConnection()
{
	msg(QString("welcome||%1").arg(VERSION_STRING));
	msg("welcome||.");
}

void TestServerSocket::catchSocketError(QAbstractSocket::SocketError socketError)
{
	qDebug(QString("socket error: %1").arg(socketError).toLatin1());
	deleteLater();
}
