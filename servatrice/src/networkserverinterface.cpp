#include "networkserverinterface.h"
#include <QSslSocket>
#include "server_logger.h"
#include "main.h"
#include "server_protocolhandler.h"
#include "server_room.h"

#include "pb/servernetwork_message.pb.h"
#include "pb/event_server_complete_list.pb.h"
#include <google/protobuf/descriptor.h>

void NetworkServerInterface::sharedCtor()
{
	connect(socket, SIGNAL(readyRead()), this, SLOT(readClient()));
	connect(socket, SIGNAL(disconnected()), this, SLOT(deleteLater()));
	connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(catchSocketError(QAbstractSocket::SocketError)));
	connect(this, SIGNAL(outputBufferChanged()), this, SLOT(flushOutputBuffer()), Qt::QueuedConnection);
}

NetworkServerInterface::NetworkServerInterface(Servatrice *_server, QSslSocket *_socket)
	: QObject(), server(_server), socket(_socket), messageInProgress(false)
{
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
		event.add_room_list()->CopyFrom(room->getInfo(true, true, false));
	}
	
	ServerNetworkMessage message;
	message.set_message_type(ServerNetworkMessage::SESSION_EVENT);
	SessionEvent *sessionEvent = message.mutable_session_event();
	sessionEvent->GetReflection()->MutableMessage(sessionEvent, event.GetDescriptor()->FindExtensionByName("ext"))->CopyFrom(event);
	transmitMessage(message);
	
	server->addNetworkServerInterface(this);
	
	roomIterator.toFront();
	while (roomIterator.hasNext())
		roomIterator.next().value()->roomMutex.unlock();
	server->serverMutex.unlock();
}

NetworkServerInterface::NetworkServerInterface(const QString &peerHostName, const QString &peerAddress, int peerPort, Servatrice *_server, QSslSocket *_socket)
	: QObject(), server(_server), socket(_socket), messageInProgress(false)
{
	sharedCtor();
	
	socket->connectToHostEncrypted(peerAddress, peerPort, peerHostName);
	if (!socket->waitForConnected(5000)) {
		qDebug() << "[SN] Socket error:" << socket->errorString();
		deleteLater();
		return;
	}
	if (!socket->waitForEncrypted(5000)) {
		QList<QSslError> sslErrors(socket->sslErrors());
		if (sslErrors.isEmpty())
			qDebug() << "[SN] SSL handshake timeout, terminating connection";
		else
			qDebug() << "[SN] SSL errors:" << sslErrors;
		deleteLater();
		return;
	}
	server->addNetworkServerInterface(this);
}

NetworkServerInterface::~NetworkServerInterface()
{
	logger->logMessage("[SN] session ended", this);
	
	flushOutputBuffer();
}

void NetworkServerInterface::flushOutputBuffer()
{
	QMutexLocker locker(&outputBufferMutex);
	qDebug("FLUSH");
	if (outputBuffer.isEmpty())
		return;
	server->incTxBytes(outputBuffer.size());
	socket->write(outputBuffer);
	socket->flush();
	outputBuffer.clear();
}

void NetworkServerInterface::readClient()
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
		
		ServerNetworkMessage newMessage;
		newMessage.ParseFromArray(inputBuffer.data(), messageLength);
		inputBuffer.remove(0, messageLength);
		messageInProgress = false;
		
		processMessage(newMessage);
	} while (!inputBuffer.isEmpty());
}

void NetworkServerInterface::catchSocketError(QAbstractSocket::SocketError socketError)
{
	qDebug() << "[SN] Socket error:" << socketError;
	
	deleteLater();
}

void NetworkServerInterface::transmitMessage(const ServerNetworkMessage &item)
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
	qDebug("TRANSMIT");
	emit outputBufferChanged();
}

void NetworkServerInterface::processMessage(const ServerNetworkMessage &item)
{
	qDebug() << QString::fromStdString(item.DebugString());
}
