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
#include "server.h"
#include "server_game.h"
#include "server_counter.h"
#include "server_room.h"
#include "server_protocolhandler.h"
#include "server_remoteuserinterface.h"
#include "pb/event_user_joined.pb.h"
#include "pb/event_user_left.pb.h"
#include "pb/event_list_rooms.pb.h"
#include "pb/session_event.pb.h"
#include "pb/isl_message.pb.h"
#include <QCoreApplication>
#include <QDebug>

Server::Server(QObject *parent)
	: QObject(parent), nextGameId(0), nextReplayId(0)
{
	qRegisterMetaType<ServerInfo_Game>("ServerInfo_Game");
	qRegisterMetaType<ServerInfo_Room>("ServerInfo_Room");
	qRegisterMetaType<ServerInfo_User>("ServerInfo_User");
}

Server::~Server()
{
}

void Server::prepareDestroy()
{
	roomsLock.lockForWrite();
	QMapIterator<int, Server_Room *> roomIterator(rooms);
	while (roomIterator.hasNext())
		delete roomIterator.next().value();
	rooms.clear();
	roomsLock.unlock();
	
	clientsLock.lockForWrite();
	while (!clients.isEmpty())
		delete clients.takeFirst();
	clientsLock.unlock();
}

AuthenticationResult Server::loginUser(Server_ProtocolHandler *session, QString &name, const QString &password, QString &reasonStr)
{
	if (name.size() > 35)
		name = name.left(35);
	
	QWriteLocker locker(&clientsLock);
	
	AuthenticationResult authState = checkUserPassword(session, name, password, reasonStr);
	if ((authState == NotLoggedIn) || (authState == UserIsBanned))
		return authState;
	
	ServerInfo_User data = getUserData(name, true);
	data.set_address(session->getAddress().toStdString());
	name = QString::fromStdString(data.name()); // Compensate for case indifference
	
	lockSessionTables();
	
	if (authState == PasswordRight) {
		if (users.contains(name) || userSessionExists(name)) {
			qDebug("Login denied: would overwrite old session");
			unlockSessionTables();
			return WouldOverwriteOldSession;
		}
	} else if (authState == UnknownUser) {
		// Change user name so that no two users have the same names,
		// don't interfere with registered user names though.
		QString tempName = name;
		int i = 0;
		while (users.contains(tempName) || userExists(tempName) || userSessionExists(tempName))
			tempName = name + "_" + QString::number(++i);
		name = tempName;
		data.set_name(name.toStdString());
	}
	
	session->setUserInfo(data);
	
	users.insert(name, session);
	qDebug() << "Server::loginUser: name=" << name;
	
	session->setSessionId(startSession(name, session->getAddress()));	
	unlockSessionTables();
	
	qDebug() << "session id:" << session->getSessionId();
	
	Event_UserJoined event;
	event.mutable_user_info()->CopyFrom(session->copyUserInfo(false));
	SessionEvent *se = Server_ProtocolHandler::prepareSessionEvent(event);
	for (int i = 0; i < clients.size(); ++i)
		if (clients[i]->getAcceptsUserListChanges())
			clients[i]->sendProtocolItem(*se);
	locker.unlock();
	
	sendIslMessage(*se);
	delete se;
	
	return authState;
}

void Server::addClient(Server_ProtocolHandler *client)
{
	QWriteLocker locker(&clientsLock);
	clients << client;
}

void Server::removeClient(Server_ProtocolHandler *client)
{
	QWriteLocker locker(&clientsLock);
	clients.removeAt(clients.indexOf(client));
	ServerInfo_User *data = client->getUserInfo();
	if (data) {
		Event_UserLeft event;
		event.set_name(data->name());
		SessionEvent *se = Server_ProtocolHandler::prepareSessionEvent(event);
		for (int i = 0; i < clients.size(); ++i)
			if (clients[i]->getAcceptsUserListChanges())
				clients[i]->sendProtocolItem(*se);
		sendIslMessage(*se);
		delete se;
		
		users.remove(QString::fromStdString(data->name()));
		qDebug() << "Server::removeClient: name=" << QString::fromStdString(data->name());
		
		if (client->getSessionId() != -1)
			endSession(client->getSessionId());
		qDebug() << "closed session id:" << client->getSessionId();
	}
	qDebug() << "Server::removeClient:" << clients.size() << "clients; " << users.size() << "users left";
}

void Server::externalUserJoined(const ServerInfo_User &userInfo)
{
	// This function is always called from the main thread via signal/slot.
	QWriteLocker locker(&clientsLock);
	
	externalUsers.insert(QString::fromStdString(userInfo.name()), new Server_RemoteUserInterface(this, ServerInfo_User_Container(userInfo)));
	
	Event_UserJoined event;
	event.mutable_user_info()->CopyFrom(userInfo);
	
	SessionEvent *se = Server_ProtocolHandler::prepareSessionEvent(event);
	for (int i = 0; i < clients.size(); ++i)
		if (clients[i]->getAcceptsUserListChanges())
			clients[i]->sendProtocolItem(*se);
	delete se;
}

void Server::externalUserLeft(const QString &userName)
{
	// This function is always called from the main thread via signal/slot.
	QWriteLocker locker(&clientsLock);
	
	delete externalUsers.take(userName);
	
	Event_UserLeft event;
	event.set_name(userName.toStdString());
	
	SessionEvent *se = Server_ProtocolHandler::prepareSessionEvent(event);
	for (int i = 0; i < clients.size(); ++i)
		if (clients[i]->getAcceptsUserListChanges())
			clients[i]->sendProtocolItem(*se);
	delete se;
}

void Server::externalRoomUserJoined(int roomId, const ServerInfo_User &userInfo)
{
	// This function is always called from the main thread via signal/slot.
	QReadLocker locker(&roomsLock);
	
	Server_Room *room = rooms.value(roomId);
	if (!room) {
		qDebug() << "externalRoomUserJoined: room id=" << roomId << "not found";
		return;
	}
	room->addExternalUser(userInfo);
}

void Server::externalRoomUserLeft(int roomId, const QString &userName)
{
	// This function is always called from the main thread via signal/slot.
	QReadLocker locker(&roomsLock);
	
	Server_Room *room = rooms.value(roomId);
	if (!room) {
		qDebug() << "externalRoomUserLeft: room id=" << roomId << "not found";
		return;
	}
	room->removeExternalUser(userName);
}

void Server::externalRoomSay(int roomId, const QString &userName, const QString &message)
{
	// This function is always called from the main thread via signal/slot.
	QReadLocker locker(&roomsLock);
	
	Server_Room *room = rooms.value(roomId);
	if (!room) {
		qDebug() << "externalRoomSay: room id=" << roomId << "not found";
		return;
	}
	room->say(userName, message, false);
}

void Server::externalRoomGameListChanged(int roomId, const ServerInfo_Game &gameInfo)
{
	// This function is always called from the main thread via signal/slot.
	QReadLocker locker(&roomsLock);
	
	Server_Room *room = rooms.value(roomId);
	if (!room) {
		qDebug() << "externalRoomGameListChanged: room id=" << roomId << "not found";
		return;
	}
	room->updateExternalGameList(gameInfo);
}

void Server::broadcastRoomUpdate(const ServerInfo_Room &roomInfo, bool sendToIsl)
{
	Event_ListRooms event;
	event.add_room_list()->CopyFrom(roomInfo);
	
	SessionEvent *se = Server_ProtocolHandler::prepareSessionEvent(event);

	clientsLock.lockForRead();
	for (int i = 0; i < clients.size(); ++i)
	  	if (clients[i]->getAcceptsRoomListChanges())
			clients[i]->sendProtocolItem(*se);
	clientsLock.unlock();
	
	if (sendToIsl)
		sendIslMessage(*se);
	
	delete se;
}

void Server::addRoom(Server_Room *newRoom)
{
	QWriteLocker locker(&roomsLock);
	qDebug() << "Adding room: ID=" << newRoom->getId() << "name=" << newRoom->getName();
	rooms.insert(newRoom->getId(), newRoom);
	connect(newRoom, SIGNAL(roomInfoChanged(ServerInfo_Room)), this, SLOT(broadcastRoomUpdate(const ServerInfo_Room &)), Qt::QueuedConnection);
}

int Server::getUsersCount() const
{
	QReadLocker locker(&clientsLock);
	return users.size();
}

int Server::getGamesCount() const
{
	int result = 0;
	QReadLocker locker(&roomsLock);
	QMapIterator<int, Server_Room *> roomIterator(rooms);
	while (roomIterator.hasNext()) {
		Server_Room *room = roomIterator.next().value();
		QMutexLocker roomLocker(&room->gamesMutex);
		result += room->getGames().size();
	}
	return result;
}

void Server::sendIslMessage(const Response &item, int serverId)
{
	IslMessage msg;
	msg.set_message_type(IslMessage::RESPONSE);
	msg.mutable_response()->CopyFrom(item);
	
	doSendIslMessage(msg, serverId);
}

void Server::sendIslMessage(const SessionEvent &item, int serverId)
{
	IslMessage msg;
	msg.set_message_type(IslMessage::SESSION_EVENT);
	msg.mutable_session_event()->CopyFrom(item);
	
	doSendIslMessage(msg, serverId);
}

void Server::sendIslMessage(const GameEventContainer &item, int serverId)
{
	IslMessage msg;
	msg.set_message_type(IslMessage::GAME_EVENT_CONTAINER);
	msg.mutable_game_event_container()->CopyFrom(item);
	
	doSendIslMessage(msg, serverId);
}

void Server::sendIslMessage(const RoomEvent &item, int serverId)
{
	IslMessage msg;
	msg.set_message_type(IslMessage::ROOM_EVENT);
	msg.mutable_room_event()->CopyFrom(item);
	
	doSendIslMessage(msg, serverId);
}
