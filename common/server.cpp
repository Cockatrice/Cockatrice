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
#include "protocol_datastructures.h"
#include <QCoreApplication>
#include <QDebug>

Server::Server(QObject *parent)
	: QObject(parent), serverMutex(QMutex::Recursive), nextGameId(0)
{
}

Server::~Server()
{
}

void Server::prepareDestroy()
{
	QMutexLocker locker(&serverMutex);
	
	while (!clients.isEmpty())
		delete clients.takeFirst();
	
	QMapIterator<int, Server_Room *> roomIterator(rooms);
	while (roomIterator.hasNext())
		delete roomIterator.next().value();
}

AuthenticationResult Server::loginUser(Server_ProtocolHandler *session, QString &name, const QString &password)
{
	QMutexLocker locker(&serverMutex);
	if (name.size() > 35)
		name = name.left(35);
	AuthenticationResult authState = checkUserPassword(name, password);
	if (authState == PasswordWrong)
		return authState;
	
	if (authState == PasswordRight) {
		if (users.contains(name))
			return WouldOverwriteOldSession;
	} else if (authState == UnknownUser) {
		// Change user name so that no two users have the same names,
		// don't interfere with registered user names though.
		QString tempName = name;
		int i = 0;
		while (users.contains(tempName) || userExists(tempName))
			tempName = name + "_" + QString::number(++i);
		name = tempName;
	}
	
	ServerInfo_User *data = getUserData(name);
	name = data->getName(); // Compensate for case indifference
	session->setUserInfo(data);
	
	users.insert(name, session);
	
	Event_UserJoined *event = new Event_UserJoined(new ServerInfo_User(data, false));
	for (int i = 0; i < clients.size(); ++i)
		if (clients[i]->getAcceptsUserListChanges())
			clients[i]->sendProtocolItem(event, false);
	delete event;
	
	return authState;
}

void Server::addClient(Server_ProtocolHandler *client)
{
	QMutexLocker locker(&serverMutex);
	clients << client;
}

void Server::removeClient(Server_ProtocolHandler *client)
{
	QMutexLocker locker(&serverMutex);
	clients.removeAt(clients.indexOf(client));
	ServerInfo_User *data = client->getUserInfo();
	if (data) {
		Event_UserLeft *event = new Event_UserLeft(data->getName());
		for (int i = 0; i < clients.size(); ++i)
			if (clients[i]->getAcceptsUserListChanges())
				clients[i]->sendProtocolItem(event, false);
		delete event;
		
		users.remove(data->getName());
	}
	qDebug() << "Server::removeClient: " << clients.size() << "clients; " << users.size() << "users left";
}

void Server::broadcastRoomUpdate()
{
	QMutexLocker locker(&serverMutex);
	Server_Room *room = static_cast<Server_Room *>(sender());
	QList<ServerInfo_Room *> eventRoomList;
	room->roomMutex.lock();
	eventRoomList.append(new ServerInfo_Room(room->getId(), room->getName(), room->getDescription(), room->getGames().size(), room->size(), room->getAutoJoin()));
	room->roomMutex.unlock();
	Event_ListRooms *event = new Event_ListRooms(eventRoomList);

	for (int i = 0; i < clients.size(); ++i)
	  	if (clients[i]->getAcceptsRoomListChanges())
			clients[i]->sendProtocolItem(event, false);
	delete event;
}

void Server::addRoom(Server_Room *newRoom)
{
	QMutexLocker locker(&serverMutex);
	rooms.insert(newRoom->getId(), newRoom);
	connect(newRoom, SIGNAL(roomInfoChanged()), this, SLOT(broadcastRoomUpdate()));
}

int Server::getUsersCount() const
{
	QMutexLocker locker(&serverMutex);
	return users.size();
}

int Server::getGamesCount() const
{
	int result = 0;
	QMutexLocker locker(&serverMutex);
	QMapIterator<int, Server_Room *> roomIterator(rooms);
	while (roomIterator.hasNext()) {
		Server_Room *room = roomIterator.next().value();
		QMutexLocker roomLocker(&room->roomMutex);
		result += room->getGames().size();
	}
	return result;
}
