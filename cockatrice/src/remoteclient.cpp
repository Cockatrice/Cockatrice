#include <QTimer>
#include "remoteclient.h"

#include "pending_command.h"
#include "pb/commands.pb.h"
#include "pb/session_commands.pb.h"
#include "pb/response_login.pb.h"
#include "pb/server_message.pb.h"
#include "pb/event_server_identification.pb.h"

static const unsigned int protocolVersion = 13;

RemoteClient::RemoteClient(QObject *parent)
	: AbstractClient(parent), timeRunning(0), lastDataReceived(0), messageInProgress(false), messageLength(0)
{
	timer = new QTimer(this);
	timer->setInterval(1000);
	connect(timer, SIGNAL(timeout()), this, SLOT(ping()));

	socket = new QTcpSocket(this);
	connect(socket, SIGNAL(connected()), this, SLOT(slotConnected()));
	connect(socket, SIGNAL(readyRead()), this, SLOT(readData()));
	connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(slotSocketError(QAbstractSocket::SocketError)));
	
	connect(this, SIGNAL(serverIdentificationEventReceived(const Event_ServerIdentification &)), this, SLOT(processServerIdentificationEvent(const Event_ServerIdentification &)));
}

RemoteClient::~RemoteClient()
{
	disconnectFromServer();
}

void RemoteClient::slotSocketError(QAbstractSocket::SocketError /*error*/)
{
	QString errorString = socket->errorString();
	disconnectFromServer();
	emit socketError(errorString);
}

void RemoteClient::slotConnected()
{
	timeRunning = lastDataReceived = 0;
	timer->start();
	setStatus(StatusAwaitingWelcome);
}

void RemoteClient::processServerIdentificationEvent(const Event_ServerIdentification &event)
{
	if (event.protocol_version() != protocolVersion) {
		emit protocolVersionMismatch(protocolVersion, event.protocol_version());
		setStatus(StatusDisconnected);
		return;
	}
	setStatus(StatusLoggingIn);
	
	Command_Login cmdLogin;
	cmdLogin.set_user_name(userName.toStdString());
	cmdLogin.set_password(password.toStdString());
	
	PendingCommand *pend = prepareSessionCommand(cmdLogin);
	connect(pend, SIGNAL(finished(const Response &)), this, SLOT(loginResponse(const Response &)));
	sendCommand(pend);
}

void RemoteClient::loginResponse(const Response &response)
{
	const Response_Login &resp = response.GetExtension(Response_Login::ext);
	if (response.response_code() == Response::RespOk) {
		setStatus(StatusLoggedIn);
		emit userInfoChanged(resp.user_info());
		
		QList<ServerInfo_User> buddyList;
		for (int i = resp.buddy_list_size() - 1; i >= 0; --i)
			buddyList.append(resp.buddy_list(i));
		emit buddyListReceived(buddyList);
		
		QList<ServerInfo_User> ignoreList;
		for (int i = resp.ignore_list_size() - 1; i >= 0; --i)
			ignoreList.append(resp.ignore_list(i));
		emit ignoreListReceived(ignoreList);
	} else {
		emit serverError(response.response_code(), QString::fromStdString(resp.denied_reason_str()));
		setStatus(StatusDisconnecting);
	}
}

void RemoteClient::readData()
{
	lastDataReceived = timeRunning;
	QByteArray data = socket->readAll();

	inputBuffer.append(data);
	
	do {
		if (!messageInProgress) {
			if (inputBuffer.size() >= 4) {
				messageLength =   (((quint32) (unsigned char) inputBuffer[0]) << 24)
				                + (((quint32) (unsigned char) inputBuffer[1]) << 16)
				                + (((quint32) (unsigned char) inputBuffer[2]) << 8)
				                + ((quint32) (unsigned char) inputBuffer[3]);
				inputBuffer.remove(0, 4);
				messageInProgress = true;
			} else
				return;
		}
		if (inputBuffer.size() < messageLength)
			return;
		
		ServerMessage newServerMessage;
		newServerMessage.ParseFromArray(inputBuffer.data(), messageLength);
		qDebug(QString::fromStdString(newServerMessage.ShortDebugString()).toUtf8());
		inputBuffer.remove(0, messageLength);
		messageInProgress = false;
		
		processProtocolItem(newServerMessage);
	} while (!inputBuffer.isEmpty());
	
	if (status == StatusDisconnecting)
		disconnectFromServer();
}

void RemoteClient::sendCommandContainer(const CommandContainer &cont)
{
	QByteArray buf;
	unsigned int size = cont.ByteSize();
	buf.resize(size + 4);
	cont.SerializeToArray(buf.data() + 4, size);
	buf.data()[3] = (unsigned char) size;
	buf.data()[2] = (unsigned char) (size >> 8);
	buf.data()[1] = (unsigned char) (size >> 16);
	buf.data()[0] = (unsigned char) (size >> 24);
	
	socket->write(buf);
}

void RemoteClient::connectToServer(const QString &hostname, unsigned int port, const QString &_userName, const QString &_password)
{
	disconnectFromServer();
	
	userName = _userName;
	password = _password;
	socket->connectToHost(hostname, port);
	setStatus(StatusConnecting);
}

void RemoteClient::disconnectFromServer()
{
	timer->stop();
	
	messageInProgress = false;
	messageLength = 0;

	QList<PendingCommand *> pc = pendingCommands.values();
	for (int i = 0; i < pc.size(); i++)
		delete pc[i];
	pendingCommands.clear();

	setStatus(StatusDisconnected);
	socket->close();
}

void RemoteClient::ping()
{
/*	QMutableMapIterator<int, PendingCommand *> i(pendingCommands);
	while (i.hasNext())
		if (i.next().value()->tick() > maxTimeout) {
			CommandContainer *cont = i.value();
			i.remove();
			cont->deleteLater();
		}
*/	
	int maxTime = timeRunning - lastDataReceived;
	emit maxPingTime(maxTime, maxTimeout);
	if (maxTime >= maxTimeout) {
		disconnectFromServer();
		emit serverTimeout();
	} else {
		sendCommand(prepareSessionCommand(Command_Ping()));
		++timeRunning;
	}
}
