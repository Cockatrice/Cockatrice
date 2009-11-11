#include <QTimer>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include "client.h"
#include "protocol.h"
#include "protocol_items.h"

Client::Client(QObject *parent)
	: QObject(parent), currentItem(0), status(StatusDisconnected)
{
	timer = new QTimer(this);
	timer->setInterval(1000);
	connect(timer, SIGNAL(timeout()), this, SLOT(ping()));

	socket = new QTcpSocket(this);
	connect(socket, SIGNAL(connected()), this, SLOT(slotConnected()));
	connect(socket, SIGNAL(readyRead()), this, SLOT(readData()));
	connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(slotSocketError(QAbstractSocket::SocketError)));
	
	xmlReader = new QXmlStreamReader;
	xmlWriter = new QXmlStreamWriter;
	xmlWriter->setAutoFormatting(true);
	xmlWriter->setDevice(socket);
}

Client::~Client()
{
	disconnectFromServer();
}

void Client::slotSocketError(QAbstractSocket::SocketError /*error*/)
{
	emit logSocketError(socket->errorString());
	disconnectFromServer();
}

void Client::slotConnected()
{
	timer->start();
	setStatus(StatusAwaitingWelcome);
}

void Client::removePendingCommand()
{
	pendingCommands.remove(static_cast<Command *>(sender())->getCmdId());
}
/*
void Client::loginResponse(ServerResponse response)
{
	if (response == RespOk)
		setStatus(StatusIdle);
	else {
		emit serverError(response);
		disconnectFromServer();
	}
}

void Client::enterGameResponse(ServerResponse response)
{
	if (response == RespOk)
		setStatus(StatusPlaying);
}

void Client::leaveGameResponse(ServerResponse response)
{
	if (response == RespOk)
		setStatus(StatusIdle);
}
*/
void Client::readData()
{
	xmlReader->addData(socket->readAll());

	if (currentItem) {
		if (!currentItem->read(xmlReader))
			return;
		currentItem = 0;
	}
	while (!xmlReader->atEnd()) {
		xmlReader->readNext();
		if (xmlReader->isStartElement()) {
			QString itemType = xmlReader->name().toString();
			if (itemType == "cockatrice_server_stream")
				continue;
			QString itemName = xmlReader->attributes().value("name").toString();
			qDebug() << "parseXml: startElement: " << "type =" << itemType << ", name =" << itemName;
			currentItem = ProtocolItem::getNewItem(itemType + itemName);
			if (!currentItem)
				continue;
			if (!currentItem->read(xmlReader))
				return;
			else {
/*				Command *command = qobject_cast<Command *>(currentItem);
				if (qobject_cast<InvalidCommand *>(command))
					sendProtocolItem(new ProtocolResponse(command->getCmdId(), ProtocolResponse::RespInvalidCommand));
				else
					processCommand(command);
				currentItem = 0;
*/			}
		}
	}
/*
	while (socket->canReadLine()) {
		QString line = QString(socket->readLine()).trimmed();

		if (line.isNull())
			break;
		qDebug(QString("<< %1").arg(line).toLatin1());
		QStringList values = line.split("|");
		QString prefix = values.takeFirst();
		// prefix is one of {welcome, private, public, resp, list_games, list_players, list_counters, list_zones, dump_zone}
		if ((prefix == "private") || (prefix == "public")) {
			ServerEventData event(line);
			emit gameEvent(event);
		} else if (prefix == "chat") {
			emit chatEvent(ChatEventData(line));
		} else if (prefix == "resp") {
			if (values.size() != 2) {
				qDebug("Client::parseCommand: Invalid response");
				continue;
			}
			bool ok;
			int msgid = values.takeFirst().toInt(&ok);
			PendingCommand *pc = pendingCommands.value(msgid, 0);
			if (!ok || !pc) {
				qDebug("Client::parseCommand: Invalid msgid");
				continue;
			}
			ServerResponse resp;
			if (values[0] == "ok")
				resp = RespOk;
			else if (values[0] == "name_not_found")
				resp = RespNameNotFound;
			else if (values[0] == "login_needed")
				resp = RespLoginNeeded;
			else if (values[0] == "syntax")
				resp = RespSyntaxError;
			else if (values[0] == "context")
				resp = RespContextError;
			else if (values[0] == "password")
				resp = RespPasswordWrong;
			else if (values[0] == "spectators_not_allowed")
				resp = RespSpectatorsNotAllowed;
			else
				resp = RespInvalid;
			pc->responseReceived(resp);
		} else if (prefix == "list_games") {
			if (values.size() != 8) {
				emit protocolError();
				continue;
			}
			emit gameListEvent(ServerGame(values[0].toInt(), values[5], values[1], values[2].toInt(), values[3].toInt(), values[4].toInt(), values[6].toInt(), values[7].toInt()));
		} else if (prefix == "welcome") {
			if (values.size() != 2) {
				emit protocolError();
				disconnectFromServer();
			} else if (values[0].toInt() != protocolVersion) {
				emit protocolVersionMismatch();
				disconnectFromServer();
			} else {
				emit welcomeMsgReceived(values[1]);
				setStatus(StatusLoggingIn);
				login(playerName, password);
			}
		} else if (prefix == "list_players") {
			if (values.size() != 4) {
				emit protocolError();
				continue;
			}
			int cmdid = values.takeFirst().toInt();
			PendingCommand *pc = pendingCommands.value(cmdid, 0);
			ServerPlayer sp(values[0].toInt(), values[1], values[2].toInt());
			
			PendingCommand_ListPlayers *pcLP = qobject_cast<PendingCommand_ListPlayers *>(pc);
			if (pcLP)
				pcLP->addPlayer(sp);
			else {
				PendingCommand_DumpAll *pcDA = qobject_cast<PendingCommand_DumpAll *>(pc);
				if (pcDA)
					pcDA->addPlayer(sp);
				else
					emit protocolError();
			}
		} else if (prefix == "dump_zone") {
			if (values.size() != 11) {
				emit protocolError();
				continue;
			}
			int cmdid = values.takeFirst().toInt();
			PendingCommand *pc = pendingCommands.value(cmdid, 0);
			ServerZoneCard szc(values[0].toInt(), values[1], values[2].toInt(), values[3], values[4].toInt(), values[5].toInt(), values[6].toInt(), values[7] == "1", values[8] == "1", values[9]);
	
			PendingCommand_DumpZone *pcDZ = qobject_cast<PendingCommand_DumpZone *>(pc);
			if (pcDZ)
				pcDZ->addCard(szc);
			else {
				PendingCommand_DumpAll *pcDA = qobject_cast<PendingCommand_DumpAll *>(pc);
				if (pcDA)
					pcDA->addCard(szc);
				else
					emit protocolError();
			}
		} else if (prefix == "list_zones") {
			if (values.size() != 6) {
				emit protocolError();
				continue;
			}
			int cmdid = values.takeFirst().toInt();
			PendingCommand *pc = pendingCommands.value(cmdid, 0);
			ServerZone::ZoneType type;
			if (values[2] == "private")
				type = ServerZone::PrivateZone;
			else if (values[2] == "hidden")
				type = ServerZone::HiddenZone;
			else
				type = ServerZone::PublicZone;
			ServerZone sz(values[0].toInt(), values[1], type, values[3] == "1", values[4].toInt());
			
			PendingCommand_ListZones *pcLZ = qobject_cast<PendingCommand_ListZones *>(pc);
			if (pcLZ)
				pcLZ->addZone(sz);
			else {
				PendingCommand_DumpAll *pcDA = qobject_cast<PendingCommand_DumpAll *>(pc);
				if (pcDA)
					pcDA->addZone(sz);
				else
					emit protocolError();
			}
		} else if (prefix == "list_counters") {
			if (values.size() != 7) {
				emit protocolError();
				continue;
			}
			int cmdid = values.takeFirst().toInt();
			PendingCommand *pc = pendingCommands.value(cmdid, 0);
			ServerCounter sc(values[0].toInt(), values[1].toInt(), values[2], numberToColor(values[3].toInt()), values[4].toInt(), values[5].toInt());
			
			PendingCommand_ListCounters *pcLC = qobject_cast<PendingCommand_ListCounters *>(pc);
			if (pcLC)
				pcLC->addCounter(sc);
			else {
				PendingCommand_DumpAll *pcDA = qobject_cast<PendingCommand_DumpAll *>(pc);
				if (pcDA)
					pcDA->addCounter(sc);
				else
					emit protocolError();
			}
		} else if (prefix == "list_arrows") {
			if (values.size() != 10) {
				emit protocolError();
				continue;
			}
			int cmdid = values.takeFirst().toInt();
			PendingCommand *pc = pendingCommands.value(cmdid, 0);
			ServerArrow sa(values[0].toInt(), values[1].toInt(), values[2].toInt(), values[3], values[4].toInt(), values[5].toInt(), values[6], values[7].toInt(), numberToColor(values[8].toInt()));
			
			PendingCommand_DumpAll *pcDA = qobject_cast<PendingCommand_DumpAll *>(pc);
			if (pcDA)
				pcDA->addArrow(sa);
			else
				emit protocolError();
		} else
			emit protocolError();
	}
*/
}

void Client::setStatus(const ClientStatus _status)
{
	if (_status != status) {
		status = _status;
		emit statusChanged(_status);
	}
}
/*
PendingCommand *Client::cmd(const QString &s, PendingCommand *_pc)
{
	msg(QString("%1|%2").arg(++MsgId).arg(s));
	PendingCommand *pc;
	if (_pc) {
		pc = _pc;
		pc->setMsgId(MsgId);
	} else
		pc = new PendingCommand(MsgId);
	pendingCommands.insert(MsgId, pc);
	connect(pc, SIGNAL(finished(ServerResponse)), this, SLOT(removePendingCommand()));
	return pc;
}
*/
void Client::connectToServer(const QString &hostname, unsigned int port, const QString &_playerName, const QString &_password)
{
	disconnectFromServer();
	
	playerName = _playerName;
	password = _password;
	socket->connectToHost(hostname, port);
	setStatus(StatusConnecting);
}

void Client::disconnectFromServer()
{
	currentItem = 0;
	xmlReader->clear();
	
	timer->stop();

	QList<Command *> pc = pendingCommands.values();
	for (int i = 0; i < pc.size(); i++)
		delete pc[i];
	pendingCommands.clear();

	setStatus(StatusDisconnected);
	socket->close();
}

void Client::ping()
{
	int maxTime = 0;
	QMapIterator<int, Command *> i(pendingCommands);
	while (i.hasNext()) {
		int time = i.next().value()->tick();
		if (time > maxTime)
			maxTime = time;
	}
	emit maxPingTime(maxTime, maxTimeout);
	if (maxTime >= maxTimeout) {
		emit serverTimeout();
		disconnectFromServer();
	}
/*	else
		cmd("ping");
*/
}
/*
PendingCommand *Client::chatListChannels()
{
	return cmd("chat_list_channels");
}

PendingCommand_ChatJoinChannel *Client::chatJoinChannel(const QString &name)
{
	return static_cast<PendingCommand_ChatJoinChannel *>(cmd(QString("chat_join_channel|%1").arg(name), new PendingCommand_ChatJoinChannel(name)));
}

PendingCommand *Client::chatLeaveChannel(const QString &name)
{
	return cmd(QString("chat_leave_channel|%1").arg(name));
}

PendingCommand *Client::chatSay(const QString &channel, const QString &s)
{
	return cmd(QString("chat_say|%1|%2").arg(channel).arg(s));
}

PendingCommand *Client::listGames()
{
	return cmd("list_games");
}

PendingCommand_ListPlayers *Client::listPlayers()
{
	return static_cast<PendingCommand_ListPlayers *>(cmd("list_players", new PendingCommand_ListPlayers));
}

PendingCommand *Client::createGame(const QString &description, const QString &password, unsigned int maxPlayers, bool spectatorsAllowed)
{
	PendingCommand *pc = cmd(QString("create_game|%1|%2|%3|%4").arg(description).arg(password).arg(maxPlayers).arg(spectatorsAllowed ? 1 : 0));
	connect(pc, SIGNAL(finished(ServerResponse)), this, SLOT(enterGameResponse(ServerResponse)));
	return pc;
}

PendingCommand *Client::joinGame(int gameId, const QString &password, bool spectator)
{
	PendingCommand *pc = cmd(QString("join_game|%1|%2|%3").arg(gameId).arg(password).arg(spectator ? 1 : 0));
	connect(pc, SIGNAL(finished(ServerResponse)), this, SLOT(enterGameResponse(ServerResponse)));
	return pc;
}

PendingCommand *Client::leaveGame()
{
	PendingCommand *pc = cmd("leave_game");
	connect(pc, SIGNAL(finished(ServerResponse)), this, SLOT(leaveGameResponse(ServerResponse)));
	return pc;
}

PendingCommand *Client::login(const QString &name, const QString &pass)
{
	PendingCommand *pc = cmd(QString("login|%1|%2").arg(name).arg(pass));
	connect(pc, SIGNAL(finished(ServerResponse)), this, SLOT(loginResponse(ServerResponse)));
	return pc;
}

PendingCommand *Client::say(const QString &s)
{
	return cmd(QString("say|%1").arg(s));
}

PendingCommand *Client::shuffle()
{
	return cmd("shuffle");
}

PendingCommand *Client::rollDie(unsigned int sides)
{
	return cmd(QString("roll_die|%1").arg(sides));
}

PendingCommand *Client::drawCards(unsigned int number)
{
	return cmd(QString("draw_cards|%1").arg(number));
}

PendingCommand *Client::moveCard(int cardid, const QString &startzone, const QString &targetzone, int x, int y, bool faceDown)
{
	// if startzone is public: cardid is the card's id
	// else: cardid is the position of the card in the zone (e.g. deck)
	return cmd(QString("move_card|%1|%2|%3|%4|%5|%6").arg(cardid).arg(startzone).arg(targetzone).arg(x).arg(y).arg(faceDown ? 1 : 0));
}

PendingCommand *Client::createToken(const QString &zone, const QString &name, const QString &powtough, int x, int y)
{
	return cmd(QString("create_token|%1|%2|%3|%4|%5").arg(zone).arg(name).arg(powtough).arg(x).arg(y));
}

PendingCommand *Client::createArrow(int startPlayerId, const QString &startZone, int startCardId, int targetPlayerId, const QString &targetPlayerZone, int targetCardId, const QColor &color)
{
	return cmd(QString("create_arrow|%1|%2|%3|%4|%5|%6|%7").arg(startPlayerId).arg(startZone).arg(startCardId).arg(targetPlayerId).arg(targetPlayerZone).arg(targetCardId).arg(colorToNumber(color)));
}

PendingCommand *Client::deleteArrow(int arrowId)
{
	return cmd(QString("delete_arrow|%1").arg(arrowId));
}

PendingCommand *Client::setCardAttr(const QString &zone, int cardid, const QString &aname, const QString &avalue)
{
	return cmd(QString("set_card_attr|%1|%2|%3|%4").arg(zone).arg(cardid).arg(aname).arg(avalue));
}

void Client::submitDeck(const QStringList &deck)
{
	cmd("submit_deck");
	QStringListIterator i(deck);
	while (i.hasNext())
		msg(i.next());
	msg(".");
}

PendingCommand *Client::readyStart()
{
	return cmd("ready_start");
}

PendingCommand *Client::incCounter(int counterId, int delta)
{
	return cmd(QString("inc_counter|%1|%2").arg(counterId).arg(delta));
}

PendingCommand *Client::addCounter(const QString &counterName, QColor color, int radius, int value)
{
	return cmd(QString("add_counter|%1|%2|%3|%4").arg(counterName).arg(colorToNumber(color)).arg(radius).arg(value));
}

PendingCommand *Client::setCounter(int counterId, int value)
{
	return cmd(QString("set_counter|%1|%2").arg(counterId).arg(value));
}

PendingCommand *Client::delCounter(int counterId)
{
	return cmd(QString("del_counter|%1").arg(counterId));
}

PendingCommand_ListCounters *Client::listCounters(int playerId)
{
	PendingCommand_ListCounters *pc = new PendingCommand_ListCounters(playerId);
	cmd(QString("list_counters|%1").arg(playerId), pc);
	return pc;
}

PendingCommand *Client::nextTurn()
{
	return cmd(QString("next_turn"));
}

PendingCommand *Client::setActivePhase(int phase)
{
	return cmd(QString("set_active_phase|%1").arg(phase));
}

PendingCommand_ListZones *Client::listZones(int playerId)
{
	PendingCommand_ListZones *pc = new PendingCommand_ListZones(playerId);
	cmd(QString("list_zones|%1").arg(playerId), pc);
	return pc;
}

PendingCommand_DumpZone *Client::dumpZone(int player, const QString &zone, int numberCards)
{
	PendingCommand_DumpZone *pc = new PendingCommand_DumpZone(player, zone, numberCards);
	cmd(QString("dump_zone|%1|%2|%3").arg(player).arg(zone).arg(numberCards), pc);
	return pc;
}

PendingCommand *Client::stopDumpZone(int player, const QString &zone)
{
	return cmd(QString("stop_dump_zone|%1|%2").arg(player).arg(zone));
}

PendingCommand_DumpAll *Client::dumpAll()
{
	PendingCommand_DumpAll *pc = new PendingCommand_DumpAll;
	cmd("dump_all", pc);
	return pc;
}

QColor Client::numberToColor(int colorValue) const
{
	return QColor(colorValue / 65536, (colorValue % 65536) / 256, colorValue % 256);
}

int Client::colorToNumber(const QColor &color) const
{
	return color.red() * 65536 + color.green() * 256 + color.blue();
}
*/