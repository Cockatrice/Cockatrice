#include <QTimer>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include "client.h"
#include "protocol.h"
#include "protocol_items.h"

Client::Client(QObject *parent)
	: QObject(parent), currentItem(0), status(StatusDisconnected)
{
	ProtocolItem::initializeHash();
	
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

void Client::loginResponse(ResponseCode response)
{
	if (response == RespOk)
		setStatus(StatusLoggedIn);
	else {
		emit serverError(response);
		disconnectFromServer();
	}
}

void Client::readData()
{
	QByteArray data = socket->readAll();
	qDebug() << data;
	xmlReader->addData(data);

	if (currentItem) {
		if (!currentItem->read(xmlReader))
			return;
		currentItem = 0;
	}
	while (!xmlReader->atEnd()) {
		xmlReader->readNext();
		if (xmlReader->isStartElement()) {
			QString itemType = xmlReader->name().toString();
			if (itemType == "cockatrice_server_stream") {
				int serverVersion = xmlReader->attributes().value("version").toString().toInt();
				if (serverVersion != ProtocolItem::protocolVersion) {
					emit protocolVersionMismatch(ProtocolItem::protocolVersion, serverVersion);
					disconnectFromServer();
					return;
				} else {
					xmlWriter->writeStartDocument();
					xmlWriter->writeStartElement("cockatrice_client_stream");
					xmlWriter->writeAttribute("version", QString::number(ProtocolItem::protocolVersion));
					
					setStatus(StatusLoggingIn);
					Command_Login *cmdLogin = new Command_Login(userName, password);
					connect(cmdLogin, SIGNAL(finished(ResponseCode)), this, SLOT(loginResponse(ResponseCode)));
					sendCommand(cmdLogin);
					
					continue;
				}
			}
			QString itemName = xmlReader->attributes().value("name").toString();
			qDebug() << "parseXml: startElement: " << "type =" << itemType << ", name =" << itemName;
			currentItem = ProtocolItem::getNewItem(itemType + itemName);
			if (!currentItem)
				continue;
			if (!currentItem->read(xmlReader))
				return;
			else {
				processProtocolItem(currentItem);
				currentItem = 0;
			}
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

void Client::processProtocolItem(ProtocolItem *item)
{
	ProtocolResponse *response = qobject_cast<ProtocolResponse *>(item);
	if (response) {
		Command *cmd = pendingCommands.value(response->getCmdId(), 0);
		if (!cmd)
			return;
		
		cmd->processResponse(response);
		delete response;
		
		pendingCommands.remove(cmd->getCmdId());
		delete cmd;
		
		return;
	}
	
	GenericEvent *genericEvent = qobject_cast<GenericEvent *>(item);
	if (genericEvent) {
		switch (genericEvent->getItemId()) {
			case ItemId_Event_ListGames: emit listGamesEventReceived(qobject_cast<Event_ListGames *>(item)); break;
			case ItemId_Event_ServerMessage: emit serverMessageEventReceived(qobject_cast<Event_ServerMessage *>(item)); break;
			case ItemId_Event_ListChatChannels: emit listChatChannelsEventReceived(qobject_cast<Event_ListChatChannels *>(item)); break;
			case ItemId_Event_GameJoined: emit gameJoinedEventReceived(qobject_cast<Event_GameJoined *>(item)); break;
		}
		delete genericEvent;
		return;
	}

	GameEvent *gameEvent = qobject_cast<GameEvent *>(item);
	if (gameEvent) {
		emit gameEventReceived(gameEvent);
		delete gameEvent;
		return;
	}

	ChatEvent *chatEvent = qobject_cast<ChatEvent *>(item);
	if (chatEvent) {
		emit chatEventReceived(chatEvent);
		delete chatEvent;
		return;
	}
}

void Client::setStatus(const ClientStatus _status)
{
	if (_status != status) {
		status = _status;
		emit statusChanged(_status);
	}
}

void Client::sendCommand(Command *cmd)
{
	cmd->write(xmlWriter);
	pendingCommands.insert(cmd->getCmdId(), cmd);
}

void Client::connectToServer(const QString &hostname, unsigned int port, const QString &_userName, const QString &_password)
{
	disconnectFromServer();
	
	userName = _userName;
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
	} else
		sendCommand(new Command_Ping);
}
/*
QColor Client::numberToColor(int colorValue) const
{
	return QColor(colorValue / 65536, (colorValue % 65536) / 256, colorValue % 256);
}

int Client::colorToNumber(const QColor &color) const
{
	return color.red() * 65536 + color.green() * 256 + color.blue();
}
*/