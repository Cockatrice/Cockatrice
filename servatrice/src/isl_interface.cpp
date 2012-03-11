#include "isl_interface.h"
#include <QSslSocket>
#include "server_logger.h"
#include "main.h"
#include "server_protocolhandler.h"
#include "server_room.h"

#include "get_pb_extension.h"
#include "pb/isl_message.pb.h"
#include "pb/event_server_complete_list.pb.h"
#include "pb/event_user_message.pb.h"
#include "pb/event_user_joined.pb.h"
#include "pb/event_user_left.pb.h"
#include <google/protobuf/descriptor.h>

void IslInterface::sharedCtor(const QSslCertificate &cert, const QSslKey &privateKey)
{
	socket = new QSslSocket(this);
	socket->setLocalCertificate(cert);
	socket->setPrivateKey(privateKey);
	
	connect(socket, SIGNAL(readyRead()), this, SLOT(readClient()));
	connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(catchSocketError(QAbstractSocket::SocketError)));
	connect(this, SIGNAL(outputBufferChanged()), this, SLOT(flushOutputBuffer()), Qt::QueuedConnection);
}

IslInterface::IslInterface(int _socketDescriptor, const QSslCertificate &cert, const QSslKey &privateKey, Servatrice *_server)
	: QObject(), socketDescriptor(_socketDescriptor), server(_server), messageInProgress(false)
{
	sharedCtor(cert, privateKey);
}

IslInterface::IslInterface(int _serverId, const QString &_peerHostName, const QString &_peerAddress, int _peerPort, const QSslCertificate &_peerCert, const QSslCertificate &cert, const QSslKey &privateKey, Servatrice *_server)
		: QObject(), serverId(_serverId), peerHostName(_peerHostName), peerAddress(_peerAddress), peerPort(_peerPort), peerCert(_peerCert), server(_server), messageInProgress(false)
{
	sharedCtor(cert, privateKey);
}

IslInterface::~IslInterface()
{
	logger->logMessage("[ISL] session ended", this);
	
	flushOutputBuffer();
	
	QStringList usersToDelete;
	server->serverMutex.lock();
	QMapIterator<QString, Server_AbstractUserInterface *> extUsers = server->getExternalUsers();
	while (extUsers.hasNext()) {
		extUsers.next();
		if (extUsers.value()->getUserInfo()->server_id() == serverId)
			usersToDelete.append(extUsers.key());
	}
	server->serverMutex.unlock();
	
	for (int i = 0; i < usersToDelete.size(); ++i)
		emit externalUserLeft(usersToDelete[i]);
}

void IslInterface::initServer()
{
	socket->setSocketDescriptor(socketDescriptor);
	
	logger->logMessage(QString("[ISL] incoming connection: %1").arg(socket->peerAddress().toString()));
	
	QList<ServerProperties> serverList = server->getServerList();
	int listIndex = -1;
	for (int i = 0; i < serverList.size(); ++i)
		if (serverList[i].address == socket->peerAddress()) {
			listIndex = i;
			break;
		}
	if (listIndex == -1) {
		logger->logMessage(QString("[ISL] address %1 unknown, terminating connection").arg(socket->peerAddress().toString()));
		deleteLater();
		return;
	}
	
	socket->startServerEncryption();
	if (!socket->waitForEncrypted(5000)) {
		QList<QSslError> sslErrors(socket->sslErrors());
		if (sslErrors.isEmpty())
			qDebug() << "[ISL] SSL handshake timeout, terminating connection";
		else
			qDebug() << "[ISL] SSL errors:" << sslErrors;
		deleteLater();
		return;
	}
	
	if (serverList[listIndex].cert == socket->peerCertificate())
		logger->logMessage(QString("[ISL] Peer authenticated as " + serverList[listIndex].hostname));
	else {
		logger->logMessage(QString("[ISL] Authentication failed, terminating connection"));
		deleteLater();
		return;
	}
	serverId = serverList[listIndex].id;
	
	Event_ServerCompleteList event;
	event.set_server_id(server->getServerId());
	
	server->serverMutex.lock();
	QMapIterator<QString, Server_ProtocolHandler *> userIterator(server->getUsers());
	while (userIterator.hasNext())
		event.add_user_list()->CopyFrom(userIterator.next().value()->copyUserInfo(true, true));
		
	QMapIterator<int, Server_Room *> roomIterator(server->getRooms());
	while (roomIterator.hasNext()) {
		Server_Room *room = roomIterator.next().value();
		room->roomMutex.lock();
		event.add_room_list()->CopyFrom(room->getInfo(true, true, false, false));
	}
	
	IslMessage message;
	message.set_message_type(IslMessage::SESSION_EVENT);
	SessionEvent *sessionEvent = message.mutable_session_event();
	sessionEvent->GetReflection()->MutableMessage(sessionEvent, event.GetDescriptor()->FindExtensionByName("ext"))->CopyFrom(event);
	
	server->islLock.lockForWrite();
	if (server->islConnectionExists(serverId)) {
		qDebug() << "[ISL] Duplicate connection to #" << serverId << "terminating connection";
		deleteLater();
	} else {
		transmitMessage(message);
		server->addIslInterface(serverId, this);
	}
	server->islLock.unlock();
	
	roomIterator.toFront();
	while (roomIterator.hasNext())
		roomIterator.next().value()->roomMutex.unlock();
	server->serverMutex.unlock();
}

void IslInterface::initClient()
{
	QList<QSslError> expectedErrors;
	expectedErrors.append(QSslError(QSslError::SelfSignedCertificate, peerCert));
	socket->ignoreSslErrors(expectedErrors);
	
	qDebug() << "[ISL] Connecting to #" << serverId << ":" << peerAddress << ":" << peerPort;

	socket->connectToHostEncrypted(peerAddress, peerPort, peerHostName);
	if (!socket->waitForConnected(5000)) {
		qDebug() << "[ISL] Socket error:" << socket->errorString();
		deleteLater();
		return;
	}
	if (!socket->waitForEncrypted(5000)) {
		QList<QSslError> sslErrors(socket->sslErrors());
		if (sslErrors.isEmpty())
			qDebug() << "[ISL] SSL handshake timeout, terminating connection";
		else
			qDebug() << "[ISL] SSL errors:" << sslErrors;
		deleteLater();
		return;
	}
	
	server->islLock.lockForWrite();
	if (server->islConnectionExists(serverId)) {
		qDebug() << "[ISL] Duplicate connection to #" << serverId << "terminating connection";
		deleteLater();
		return;
	}
		
	server->addIslInterface(serverId, this);
	server->islLock.unlock();
}

void IslInterface::flushOutputBuffer()
{
	QMutexLocker locker(&outputBufferMutex);
	if (outputBuffer.isEmpty())
		return;
	server->incTxBytes(outputBuffer.size());
	socket->write(outputBuffer);
	socket->flush();
	outputBuffer.clear();
}

void IslInterface::readClient()
{
	QByteArray data = socket->readAll();
	server->incRxBytes(data.size());
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
		
		IslMessage newMessage;
		newMessage.ParseFromArray(inputBuffer.data(), messageLength);
		inputBuffer.remove(0, messageLength);
		messageInProgress = false;
		
		processMessage(newMessage);
	} while (!inputBuffer.isEmpty());
}

void IslInterface::catchSocketError(QAbstractSocket::SocketError socketError)
{
	qDebug() << "[ISL] Socket error:" << socketError;
	
	server->islLock.lockForWrite();
	server->removeIslInterface(serverId);
	server->islLock.unlock();
	
	deleteLater();
}

void IslInterface::transmitMessage(const IslMessage &item)
{
	QByteArray buf;
	unsigned int size = item.ByteSize();
	buf.resize(size + 4);
	item.SerializeToArray(buf.data() + 4, size);
	buf.data()[3] = (unsigned char) size;
	buf.data()[2] = (unsigned char) (size >> 8);
	buf.data()[1] = (unsigned char) (size >> 16);
	buf.data()[0] = (unsigned char) (size >> 24);
	
	outputBufferMutex.lock();
	outputBuffer.append(buf);
	outputBufferMutex.unlock();
	emit outputBufferChanged();
}

void IslInterface::sessionEvent_ServerCompleteList(const Event_ServerCompleteList &event)
{
	for (int i = 0; i < event.user_list_size(); ++i) {
		ServerInfo_User temp(event.user_list(i));
		temp.set_server_id(serverId);
		emit externalUserJoined(temp);
	}
}

void IslInterface::sessionEvent_UserMessage(const SessionEvent &sessionEvent, const Event_UserMessage &event)
{
	QMutexLocker locker(&server->serverMutex);
	
	Server_ProtocolHandler *userInterface = server->getUsers().value(QString::fromStdString(event.receiver_name()));
	if (userInterface)
		userInterface->sendProtocolItem(sessionEvent);
}

void IslInterface::sessionEvent_UserJoined(const Event_UserJoined &event)
{
	emit externalUserJoined(event.user_info());
}

void IslInterface::sessionEvent_UserLeft(const Event_UserLeft &event)
{
	emit externalUserLeft(QString::fromStdString(event.name()));
}

void IslInterface::processSessionEvent(const SessionEvent &event)
{
	switch (getPbExtension(event)) {
		case SessionEvent::SERVER_COMPLETE_LIST: sessionEvent_ServerCompleteList(event.GetExtension(Event_ServerCompleteList::ext)); break;
		case SessionEvent::USER_MESSAGE: sessionEvent_UserMessage(event, event.GetExtension(Event_UserMessage::ext)); break;
		case SessionEvent::USER_JOINED: sessionEvent_UserJoined(event.GetExtension(Event_UserJoined::ext)); break;
		case SessionEvent::USER_LEFT: sessionEvent_UserLeft(event.GetExtension(Event_UserLeft::ext)); break;
		default: ;
	}
}

void IslInterface::processMessage(const IslMessage &item)
{
	qDebug() << QString::fromStdString(item.DebugString());
	
	switch (item.message_type()) {
		case IslMessage::SESSION_EVENT: processSessionEvent(item.session_event()); break;
		case IslMessage::RESPONSE: {
			break;
		}
		case IslMessage::GAME_COMMAND_CONTAINER: {
			break;
		}
		case IslMessage::GAME_EVENT_CONTAINER: {
			break;
		}
		case IslMessage::ROOM_EVENT: {
			break;
		}
		default: ;
	}
}
