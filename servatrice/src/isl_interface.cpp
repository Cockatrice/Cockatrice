#include "isl_interface.h"
#include <QSslSocket>
#include "server_logger.h"
#include "main.h"
#include "server_protocolhandler.h"
#include "server_room.h"

#include "get_pb_extension.h"
#include "pb/isl_message.pb.h"
#include "pb/event_game_joined.pb.h"
#include "pb/event_server_complete_list.pb.h"
#include "pb/event_user_message.pb.h"
#include "pb/event_user_joined.pb.h"
#include "pb/event_user_left.pb.h"
#include "pb/event_join_room.pb.h"
#include "pb/event_leave_room.pb.h"
#include "pb/event_room_say.pb.h"
#include "pb/event_list_games.pb.h"
#include <google/protobuf/descriptor.h>

void IslInterface::sharedCtor(const QSslCertificate &cert, const QSslKey &privateKey)
{
	socket = new QSslSocket(this);
	socket->setLocalCertificate(cert);
	socket->setPrivateKey(privateKey);
	
	connect(socket, SIGNAL(readyRead()), this, SLOT(readClient()), Qt::QueuedConnection);
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
	
	// As these signals are connected with Qt::QueuedConnection implicitly,
	// we don't need to worry about them modifying the lists while we're iterating.
	
	server->roomsLock.lockForRead();
	QMapIterator<int, Server_Room *> roomIterator(server->getRooms());
	while (roomIterator.hasNext()) {
		Server_Room *room = roomIterator.next().value();
		room->usersLock.lockForRead();
		QMapIterator<QString, ServerInfo_User_Container> roomUsers(room->getExternalUsers());
		while (roomUsers.hasNext()) {
			roomUsers.next();
			if (roomUsers.value().getUserInfo()->server_id() == serverId)
				emit externalRoomUserLeft(room->getId(), roomUsers.key());
		}
		room->usersLock.unlock();
	}
	server->roomsLock.unlock();
	
	server->clientsLock.lockForRead();
	QMapIterator<QString, Server_AbstractUserInterface *> extUsers(server->getExternalUsers());
	while (extUsers.hasNext()) {
		extUsers.next();
		if (extUsers.value()->getUserInfo()->server_id() == serverId)
			emit externalUserLeft(extUsers.key());
	}
	server->clientsLock.unlock();
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
	event.set_server_id(server->getServerID());
	
	server->clientsLock.lockForRead();
	QMapIterator<QString, Server_ProtocolHandler *> userIterator(server->getUsers());
	while (userIterator.hasNext())
		event.add_user_list()->CopyFrom(userIterator.next().value()->copyUserInfo(true, true));
	server->clientsLock.unlock();
	
	server->roomsLock.lockForRead();
	QMapIterator<int, Server_Room *> roomIterator(server->getRooms());
	while (roomIterator.hasNext()) {
		Server_Room *room = roomIterator.next().value();
		room->usersLock.lockForRead();
		room->gamesLock.lockForRead();
		room->getInfo(*event.add_room_list(), true, true, false);
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
	while (roomIterator.hasNext()) {
		roomIterator.next();
		roomIterator.value()->gamesLock.unlock();
		roomIterator.value()->usersLock.unlock();
	}
	server->roomsLock.unlock();
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
	for (int i = 0; i < event.room_list_size(); ++i) {
		const ServerInfo_Room &room = event.room_list(i);
		for (int j = 0; j < room.user_list_size(); ++j) {
			ServerInfo_User userInfo(room.user_list(j));
			userInfo.set_server_id(serverId);
			emit externalRoomUserJoined(room.room_id(), userInfo);
		}
		for (int j = 0; j < room.game_list_size(); ++j) {
			ServerInfo_Game gameInfo(room.game_list(j));
			gameInfo.set_server_id(serverId);
			emit externalRoomGameListChanged(room.room_id(), gameInfo);
		}
	}
}

void IslInterface::sessionEvent_UserJoined(const Event_UserJoined &event)
{
	ServerInfo_User userInfo(event.user_info());
	userInfo.set_server_id(serverId);
	emit externalUserJoined(userInfo);
}

void IslInterface::sessionEvent_UserLeft(const Event_UserLeft &event)
{
	emit externalUserLeft(QString::fromStdString(event.name()));
}

void IslInterface::roomEvent_UserJoined(int roomId, const Event_JoinRoom &event)
{
	ServerInfo_User userInfo(event.user_info());
	userInfo.set_server_id(serverId);
	emit externalRoomUserJoined(roomId, userInfo);
}

void IslInterface::roomEvent_UserLeft(int roomId, const Event_LeaveRoom &event)
{
	emit externalRoomUserLeft(roomId, QString::fromStdString(event.name()));
}

void IslInterface::roomEvent_Say(int roomId, const Event_RoomSay &event)
{
	emit externalRoomSay(roomId, QString::fromStdString(event.name()), QString::fromStdString(event.message()));
}

void IslInterface::roomEvent_ListGames(int roomId, const Event_ListGames &event)
{
	for (int i = 0; i < event.game_list_size(); ++i) {
		ServerInfo_Game gameInfo(event.game_list(i));
		gameInfo.set_server_id(serverId);
		emit externalRoomGameListChanged(roomId, gameInfo);
	}
}

void IslInterface::roomCommand_JoinGame(const Command_JoinGame &cmd, int cmdId, int roomId, qint64 sessionId)
{
	emit joinGameCommandReceived(cmd, cmdId, roomId, serverId, sessionId);
}

void IslInterface::processSessionEvent(const SessionEvent &event, qint64 sessionId)
{
	switch (getPbExtension(event)) {
		case SessionEvent::SERVER_COMPLETE_LIST: sessionEvent_ServerCompleteList(event.GetExtension(Event_ServerCompleteList::ext)); break;
		case SessionEvent::USER_JOINED: sessionEvent_UserJoined(event.GetExtension(Event_UserJoined::ext)); break;
		case SessionEvent::USER_LEFT: sessionEvent_UserLeft(event.GetExtension(Event_UserLeft::ext)); break;
		case SessionEvent::GAME_JOINED: {
			QReadLocker clientsLocker(&server->clientsLock);
			Server_AbstractUserInterface *client = server->getUsersBySessionId().value(sessionId);
			if (!client) {
				qDebug() << "IslInterface::processSessionEvent: session id" << sessionId << "not found";
				break;
			}
			const Event_GameJoined &gameJoined = event.GetExtension(Event_GameJoined::ext);
			client->playerAddedToGame(gameJoined.game_info().game_id(), gameJoined.game_info().room_id(), gameJoined.player_id());
			client->sendProtocolItem(event);
			break;
		}
		case SessionEvent::USER_MESSAGE:
		case SessionEvent::REPLAY_ADDED: {
			QReadLocker clientsLocker(&server->clientsLock);
			Server_AbstractUserInterface *client = server->getUsersBySessionId().value(sessionId);
			if (!client) {
				qDebug() << "IslInterface::processSessionEvent: session id" << sessionId << "not found";
				break;
			}
			
			client->sendProtocolItem(event);
			break;
		}
		default: ;
	}
}

void IslInterface::processRoomEvent(const RoomEvent &event)
{
	switch (getPbExtension(event)) {
		case RoomEvent::JOIN_ROOM: roomEvent_UserJoined(event.room_id(), event.GetExtension(Event_JoinRoom::ext)); break;
		case RoomEvent::LEAVE_ROOM: roomEvent_UserLeft(event.room_id(), event.GetExtension(Event_LeaveRoom::ext)); break;
		case RoomEvent::ROOM_SAY: roomEvent_Say(event.room_id(), event.GetExtension(Event_RoomSay::ext)); break;
		case RoomEvent::LIST_GAMES: roomEvent_ListGames(event.room_id(), event.GetExtension(Event_ListGames::ext)); break;
		default: ;
	}
}

void IslInterface::processRoomCommand(const CommandContainer &cont, qint64 sessionId)
{
	for (int i = 0; i < cont.room_command_size(); ++i) {
		const RoomCommand &roomCommand = cont.room_command(i);
		switch (static_cast<RoomCommand::RoomCommandType>(getPbExtension(roomCommand))) {
			case RoomCommand::JOIN_GAME: roomCommand_JoinGame(roomCommand.GetExtension(Command_JoinGame::ext), cont.cmd_id(), cont.room_id(), sessionId);
			default: ;
		}
	}
}

void IslInterface::processMessage(const IslMessage &item)
{
	qDebug() << QString::fromStdString(item.DebugString());
	
	switch (item.message_type()) {
		case IslMessage::ROOM_COMMAND_CONTAINER: {
			processRoomCommand(item.room_command(), item.session_id());
			break;
		}
		case IslMessage::GAME_COMMAND_CONTAINER: {
			emit gameCommandContainerReceived(item.game_command(), item.player_id(), serverId, item.session_id());
			break;
		}
		case IslMessage::SESSION_EVENT: {
			processSessionEvent(item.session_event(), item.session_id());
			break;
		}
		case IslMessage::RESPONSE: {
			emit responseReceived(item.response(), item.session_id());
			break;
		}
		case IslMessage::GAME_EVENT_CONTAINER: {
			emit gameEventContainerReceived(item.game_event_container(), item.session_id());
			break;
		}
		case IslMessage::ROOM_EVENT: {
			processRoomEvent(item.room_event()); break;
			break;
		}
		default: ;
	}
}
