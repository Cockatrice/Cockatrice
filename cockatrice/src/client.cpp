#include <QTimer>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include "client.h"
#include "protocol.h"
#include "protocol_items.h"

Client::Client(QObject *parent)
	: QObject(parent), topLevelItem(0), status(StatusDisconnected)
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
	emit socketError(socket->errorString());
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
		setStatus(StatusDisconnecting);
	}
}

void Client::readData()
{
	QByteArray data = socket->readAll();
	qDebug() << data;
	xmlReader->addData(data);

	while (!xmlReader->atEnd()) {
		xmlReader->readNext();
		if (topLevelItem)
			topLevelItem->readElement(xmlReader);
		else if (xmlReader->isStartElement() && (xmlReader->name().toString() == "cockatrice_server_stream")) {
			int serverVersion = xmlReader->attributes().value("version").toString().toInt();
			if (serverVersion != ProtocolItem::protocolVersion) {
				emit protocolVersionMismatch(ProtocolItem::protocolVersion, serverVersion);
				disconnectFromServer();
				return;
			}
			xmlWriter->writeStartDocument();
			xmlWriter->writeStartElement("cockatrice_client_stream");
			xmlWriter->writeAttribute("version", QString::number(ProtocolItem::protocolVersion));
			
			topLevelItem = new TopLevelProtocolItem;
			connect(topLevelItem, SIGNAL(protocolItemReceived(ProtocolItem *)), this, SLOT(processProtocolItem(ProtocolItem *)));
			
			setStatus(StatusLoggingIn);
			Command_Login *cmdLogin = new Command_Login(userName, password);
			connect(cmdLogin, SIGNAL(finished(ResponseCode)), this, SLOT(loginResponse(ResponseCode)));
			sendCommand(cmdLogin);
		}
	}
	if (status == StatusDisconnecting)
		disconnectFromServer();
}

void Client::processProtocolItem(ProtocolItem *item)
{
	ProtocolResponse *response = qobject_cast<ProtocolResponse *>(item);
	if (response) {
		CommandContainer *cmdCont = pendingCommands.value(response->getCmdId(), 0);
		if (!cmdCont)
			return;
		
		pendingCommands.remove(cmdCont->getCmdId());
		cmdCont->processResponse(response);
		delete response;
		delete cmdCont;
		
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

	GameEventContainer *gameEventContainer = qobject_cast<GameEventContainer *>(item);
	if (gameEventContainer) {
		emit gameEventContainerReceived(gameEventContainer);
		delete gameEventContainer;
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
	sendCommandContainer(new CommandContainer(QList<Command *>() << cmd));
}

void Client::sendCommandContainer(CommandContainer *cont)
{
	cont->write(xmlWriter);
	pendingCommands.insert(cont->getCmdId(), cont);
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
	delete topLevelItem;
	topLevelItem = 0;
	
	xmlReader->clear();
	
	timer->stop();

	QList<CommandContainer *> pc = pendingCommands.values();
	for (int i = 0; i < pc.size(); i++)
		delete pc[i];
	pendingCommands.clear();

	setStatus(StatusDisconnected);
	socket->close();
}

void Client::ping()
{
	int maxTime = 0;
	QMapIterator<int, CommandContainer *> i(pendingCommands);
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
