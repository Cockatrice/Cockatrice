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
#include "server_player.h"
#include "server_counter.h"
#include "server_room.h"
#include "server_protocolhandler.h"
#include "server_remoteuserinterface.h"
#include "server_metatypes.h"
#include "server_database_interface.h"
#include "pb/event_user_joined.pb.h"
#include "pb/event_user_left.pb.h"
#include "pb/event_list_rooms.pb.h"
#include "pb/session_event.pb.h"
#include "pb/event_connection_closed.pb.h"
#include "pb/isl_message.pb.h"
#include "featureset.h"
#include <QCoreApplication>
#include <QThread>
#include <QDebug>

Server::Server(QObject *parent)
    : QObject(parent), nextLocalGameId(0)
{
    qRegisterMetaType<ServerInfo_Ban>("ServerInfo_Ban");
    qRegisterMetaType<ServerInfo_Game>("ServerInfo_Game");
    qRegisterMetaType<ServerInfo_Room>("ServerInfo_Room");
    qRegisterMetaType<ServerInfo_User>("ServerInfo_User");
    qRegisterMetaType<CommandContainer>("CommandContainer");
    qRegisterMetaType<Response>("Response");
    qRegisterMetaType<GameEventContainer>("GameEventContainer");
    qRegisterMetaType<IslMessage>("IslMessage");
    qRegisterMetaType<Command_JoinGame>("Command_JoinGame");

    connect(this, SIGNAL(sigSendIslMessage(IslMessage, int)), this, SLOT(doSendIslMessage(IslMessage, int)), Qt::QueuedConnection);
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
}

void Server::setDatabaseInterface(Server_DatabaseInterface *_databaseInterface)
{
    connect(this, SIGNAL(endSession(qint64)), _databaseInterface, SLOT(endSession(qint64)));
    databaseInterfaces.insert(QThread::currentThread(), _databaseInterface);
}

Server_DatabaseInterface *Server::getDatabaseInterface() const
{
    return databaseInterfaces.value(QThread::currentThread());
}

AuthenticationResult Server::loginUser(Server_ProtocolHandler *session, QString &name, const QString &password, QString &reasonStr, int &secondsLeft, QString &clientid, QString &clientVersion, QString & /* connectionType */)
{
    if (name.size() > 35)
        name = name.left(35);

    Server_DatabaseInterface *databaseInterface = getDatabaseInterface();

    AuthenticationResult authState = databaseInterface->checkUserPassword(session, name, password, clientid, reasonStr, secondsLeft);
    if (authState == NotLoggedIn || authState == UserIsBanned || authState == UsernameInvalid || authState == UserIsInactive)
        return authState;

    ServerInfo_User data = databaseInterface->getUserData(name, true);
    data.set_address(session->getAddress().toStdString());
    name = QString::fromStdString(data.name()); // Compensate for case indifference

    if (authState == PasswordRight) {
        if (users.contains(name) || databaseInterface->userSessionExists(name)) {
            if (users.contains(name)) {
                qDebug("Session already logged in, logging old session out");
                Event_ConnectionClosed event;
                event.set_reason(Event_ConnectionClosed::LOGGEDINELSEWERE);
                event.set_reason_str("You have been logged out due to logging in at another location.");
                event.set_end_time(QDateTime::currentDateTime().toTime_t());

                SessionEvent *se = users.value(name)->prepareSessionEvent(event);
                users.value(name)->sendProtocolItem(*se);
                delete se;

                users.value(name)->prepareDestroy();
            } else {
                qDebug() << "Active session and sessions table inconsistent, please validate session table information for user " << name;
            }
        }

    } else if (authState == UnknownUser) {
        // Change user name so that no two users have the same names,
        // don't interfere with registered user names though.
        if (getRegOnlyServer()) {
            qDebug("Login denied: registration required");
            databaseInterface->unlockSessionTables();
            return RegistrationRequired;
        }

        QString tempName = name;
        int i = 0;
        while (users.contains(tempName) || databaseInterface->activeUserExists(tempName) || databaseInterface->userSessionExists(tempName))
            tempName = name + "_" + QString::number(++i);
        name = tempName;
        data.set_name(name.toStdString());
    }

    QWriteLocker locker(&clientsLock);
    databaseInterface->lockSessionTables();
    users.insert(name, session);
    qDebug() << "Server::loginUser:" << session << "name=" << name;

    data.set_session_id(databaseInterface->startSession(name, session->getAddress(), clientid, session->getConnectionType()));
    databaseInterface->unlockSessionTables();

    usersBySessionId.insert(data.session_id(), session);

    qDebug() << "session id:" << data.session_id();
    session->setUserInfo(data);

    Event_UserJoined event;
    event.mutable_user_info()->CopyFrom(session->copyUserInfo(false));
    SessionEvent *se = Server_ProtocolHandler::prepareSessionEvent(event);
    for (int i = 0; i < clients.size(); ++i)
        if (clients[i]->getAcceptsUserListChanges())
            clients[i]->sendProtocolItem(*se);
    delete se;

    event.mutable_user_info()->CopyFrom(session->copyUserInfo(true, true, true));
    locker.unlock();

    if (clientid.isEmpty()){
        // client id is empty, either out dated client or client has been modified
        if (getClientIdRequired())
            return ClientIdRequired;
    }
    else {
        // update users database table with client id
        databaseInterface->updateUsersClientID(name, clientid);
    }

    databaseInterface->updateUsersLastLoginData(name, clientVersion);
    se = Server_ProtocolHandler::prepareSessionEvent(event);
    sendIsl_SessionEvent(*se);
    delete se;

    return authState;
}

void Server::addPersistentPlayer(const QString &userName, int roomId, int gameId, int playerId)
{
    QWriteLocker locker(&persistentPlayersLock);
    persistentPlayers.insert(userName, PlayerReference(roomId, gameId, playerId));
}

void Server::removePersistentPlayer(const QString &userName, int roomId, int gameId, int playerId)
{
    QWriteLocker locker(&persistentPlayersLock);
    persistentPlayers.remove(userName, PlayerReference(roomId, gameId, playerId));
}

QList<PlayerReference> Server::getPersistentPlayerReferences(const QString &userName) const
{
    QReadLocker locker(&persistentPlayersLock);
    return persistentPlayers.values(userName);
}

Server_AbstractUserInterface *Server::findUser(const QString &userName) const
{
    // Call this only with clientsLock set.

    Server_AbstractUserInterface *userHandler = users.value(userName);
    if (userHandler)
        return userHandler;
    else
        return externalUsers.value(userName);
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
        sendIsl_SessionEvent(*se);
        delete se;

        users.remove(QString::fromStdString(data->name()));
        qDebug() << "Server::removeClient: name=" << QString::fromStdString(data->name());

        if (data->has_session_id()) {
            const qint64 sessionId = data->session_id();
            usersBySessionId.remove(sessionId);
            emit endSession(sessionId);
            qDebug() << "closed session id:" << sessionId;
        }
    }
    qDebug() << "Server::removeClient: removed" << (void *) client << ";" << clients.size() << "clients; " << users.size() << "users left";
}

QList<QString> Server::getOnlineModeratorList()
{
    // clients list should be locked by calling function prior to iteration otherwise sigfaults may occur
    QList<QString> results;
    for (int i = 0; i < clients.size(); ++i) {
        ServerInfo_User *data = clients[i]->getUserInfo();

        //TODO: this line should be updated in the event there is any type of new user level created
        if (data &&
            (data->user_level() & ServerInfo_User::IsModerator ||
             data->user_level() & ServerInfo_User::IsAdmin)
           )
            results << QString::fromStdString(data->name()).simplified();
    }
    return results;
}

void Server::externalUserJoined(const ServerInfo_User &userInfo)
{
    // This function is always called from the main thread via signal/slot.
    clientsLock.lockForWrite();

    Server_RemoteUserInterface *newUser = new Server_RemoteUserInterface(this, ServerInfo_User_Container(userInfo));
    externalUsers.insert(QString::fromStdString(userInfo.name()), newUser);
    externalUsersBySessionId.insert(userInfo.session_id(), newUser);

    Event_UserJoined event;
    event.mutable_user_info()->CopyFrom(userInfo);

    SessionEvent *se = Server_ProtocolHandler::prepareSessionEvent(event);
    for (int i = 0; i < clients.size(); ++i)
        if (clients[i]->getAcceptsUserListChanges())
            clients[i]->sendProtocolItem(*se);
    delete se;
    clientsLock.unlock();

    ResponseContainer rc(-1);
    newUser->joinPersistentGames(rc);
    newUser->sendResponseContainer(rc, Response::RespNothing);
}

void Server::externalUserLeft(const QString &userName)
{
    // This function is always called from the main thread via signal/slot.

    clientsLock.lockForWrite();
    Server_AbstractUserInterface *user = externalUsers.take(userName);
    externalUsersBySessionId.remove(user->getUserInfo()->session_id());
    clientsLock.unlock();

    QMap<int, QPair<int, int> > userGames(user->getGames());
    QMapIterator<int, QPair<int, int> > userGamesIterator(userGames);
    roomsLock.lockForRead();
    while (userGamesIterator.hasNext()) {
        userGamesIterator.next();
        Server_Room *room = rooms.value(userGamesIterator.value().first);
        if (!room)
            continue;

        QReadLocker roomGamesLocker(&room->gamesLock);
        Server_Game *game = room->getGames().value(userGamesIterator.key());
        if (!game)
            continue;

        QMutexLocker gameLocker(&game->gameMutex);
        Server_Player *player = game->getPlayers().value(userGamesIterator.value().second);
        if (!player)
            continue;

        player->disconnectClient();
    }
    roomsLock.unlock();

    delete user;

    Event_UserLeft event;
    event.set_name(userName.toStdString());

    SessionEvent *se = Server_ProtocolHandler::prepareSessionEvent(event);
    clientsLock.lockForRead();
    for (int i = 0; i < clients.size(); ++i)
        if (clients[i]->getAcceptsUserListChanges())
            clients[i]->sendProtocolItem(*se);
    clientsLock.unlock();
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

    getDatabaseInterface()->logMessage(0, userName, "ISL", message, Server_DatabaseInterface::MessageTargetIslRoom, room->getId(), room->getName());
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

void Server::externalJoinGameCommandReceived(const Command_JoinGame &cmd, int cmdId, int roomId, int serverId, qint64 sessionId)
{
    // This function is always called from the main thread via signal/slot.

    try {
        QReadLocker roomsLocker(&roomsLock);
        QReadLocker clientsLocker(&clientsLock);

        Server_Room *room = rooms.value(roomId);
        if (!room) {
            qDebug() << "externalJoinGameCommandReceived: room id=" << roomId << "not found";
            throw Response::RespNotInRoom;
        }
        Server_AbstractUserInterface *userInterface = externalUsersBySessionId.value(sessionId);
        if (!userInterface) {
            qDebug() << "externalJoinGameCommandReceived: session id=" << sessionId << "not found";
            throw Response::RespNotInRoom;
        }

        ResponseContainer responseContainer(cmdId);
        Response::ResponseCode responseCode = room->processJoinGameCommand(cmd, responseContainer, userInterface);
        userInterface->sendResponseContainer(responseContainer, responseCode);
    } catch (Response::ResponseCode code) {
        Response response;
        response.set_cmd_id(cmdId);
        response.set_response_code(code);

        sendIsl_Response(response, serverId, sessionId);
    }
}

void Server::externalGameCommandContainerReceived(const CommandContainer &cont, int playerId, int serverId, qint64 sessionId)
{
    // This function is always called from the main thread via signal/slot.

    try {
        ResponseContainer responseContainer(cont.cmd_id());
        Response::ResponseCode finalResponseCode = Response::RespOk;

        QReadLocker roomsLocker(&roomsLock);
        Server_Room *room = rooms.value(cont.room_id());
        if (!room) {
            qDebug() << "externalGameCommandContainerReceived: room id=" << cont.room_id() << "not found";
            throw Response::RespNotInRoom;
        }

        QReadLocker roomGamesLocker(&room->gamesLock);
        Server_Game *game = room->getGames().value(cont.game_id());
        if (!game) {
            qDebug() << "externalGameCommandContainerReceived: game id=" << cont.game_id() << "not found";
            throw Response::RespNotInRoom;
        }

        QMutexLocker gameLocker(&game->gameMutex);
        Server_Player *player = game->getPlayers().value(playerId);
        if (!player) {
            qDebug() << "externalGameCommandContainerReceived: player id=" << playerId << "not found";
            throw Response::RespNotInRoom;
        }

        GameEventStorage ges;
        for (int i = cont.game_command_size() - 1; i >= 0; --i) {
            const GameCommand &sc = cont.game_command(i);
            qDebug() << "[ISL]" << QString::fromStdString(sc.ShortDebugString());

            Response::ResponseCode resp = player->processGameCommand(sc, responseContainer, ges);

            if (resp != Response::RespOk)
                finalResponseCode = resp;
        }
        ges.sendToGame(game);

        if (finalResponseCode != Response::RespNothing) {
            player->playerMutex.lock();
            player->getUserInterface()->sendResponseContainer(responseContainer, finalResponseCode);
            player->playerMutex.unlock();
        }
    } catch (Response::ResponseCode code) {
        Response response;
        response.set_cmd_id(cont.cmd_id());
        response.set_response_code(code);

        sendIsl_Response(response, serverId, sessionId);
    }
}

void Server::externalGameEventContainerReceived(const GameEventContainer &cont, qint64 sessionId)
{
    // This function is always called from the main thread via signal/slot.

    QReadLocker usersLocker(&clientsLock);

    Server_ProtocolHandler *client = usersBySessionId.value(sessionId);
    if (!client) {
        qDebug() << "externalGameEventContainerReceived: session" << sessionId << "not found";
        return;
    }
    client->sendProtocolItem(cont);
}

void Server::externalResponseReceived(const Response &resp, qint64 sessionId)
{
    // This function is always called from the main thread via signal/slot.

    QReadLocker usersLocker(&clientsLock);

    Server_ProtocolHandler *client = usersBySessionId.value(sessionId);
    if (!client) {
        qDebug() << "externalResponseReceived: session" << sessionId << "not found";
        return;
    }
    client->sendProtocolItem(resp);
}

void Server::broadcastRoomUpdate(const ServerInfo_Room &roomInfo, bool sendToIsl)
{
    // This function is always called from the main thread via signal/slot.

    Event_ListRooms event;
    event.add_room_list()->CopyFrom(roomInfo);

    SessionEvent *se = Server_ProtocolHandler::prepareSessionEvent(event);

    clientsLock.lockForRead();
    for (int i = 0; i < clients.size(); ++i)
        if (clients[i]->getAcceptsRoomListChanges())
            clients[i]->sendProtocolItem(*se);
    clientsLock.unlock();

    if (sendToIsl)
        sendIsl_SessionEvent(*se);

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
        QReadLocker roomLocker(&room->gamesLock);
        result += room->getGames().size();
    }
    return result;
}

void Server::sendIsl_Response(const Response &item, int serverId, qint64 sessionId)
{
    IslMessage msg;
    msg.set_message_type(IslMessage::RESPONSE);
    if (sessionId != -1)
        msg.set_session_id(sessionId);
    msg.mutable_response()->CopyFrom(item);

    emit sigSendIslMessage(msg, serverId);
}

void Server::sendIsl_SessionEvent(const SessionEvent &item, int serverId, qint64 sessionId)
{
    IslMessage msg;
    msg.set_message_type(IslMessage::SESSION_EVENT);
    if (sessionId != -1)
        msg.set_session_id(sessionId);
    msg.mutable_session_event()->CopyFrom(item);

    emit sigSendIslMessage(msg, serverId);
}

void Server::sendIsl_GameEventContainer(const GameEventContainer &item, int serverId, qint64 sessionId)
{
    IslMessage msg;
    msg.set_message_type(IslMessage::GAME_EVENT_CONTAINER);
    if (sessionId != -1)
        msg.set_session_id(sessionId);
    msg.mutable_game_event_container()->CopyFrom(item);

    emit sigSendIslMessage(msg, serverId);
}

void Server::sendIsl_RoomEvent(const RoomEvent &item, int serverId, qint64 sessionId)
{
    IslMessage msg;
    msg.set_message_type(IslMessage::ROOM_EVENT);
    if (sessionId != -1)
        msg.set_session_id(sessionId);
    msg.mutable_room_event()->CopyFrom(item);

    emit sigSendIslMessage(msg, serverId);
}

void Server::sendIsl_GameCommand(const CommandContainer &item, int serverId, qint64 sessionId, int roomId, int playerId)
{
    IslMessage msg;
    msg.set_message_type(IslMessage::GAME_COMMAND_CONTAINER);
    msg.set_session_id(sessionId);
    msg.set_player_id(playerId);

    CommandContainer *cont = msg.mutable_game_command();
    cont->CopyFrom(item);
    cont->set_room_id(roomId);

    emit sigSendIslMessage(msg, serverId);
}

void Server::sendIsl_RoomCommand(const CommandContainer &item, int serverId, qint64 sessionId, int roomId)
{
    IslMessage msg;
    msg.set_message_type(IslMessage::ROOM_COMMAND_CONTAINER);
    msg.set_session_id(sessionId);

    CommandContainer *cont = msg.mutable_room_command();
    cont->CopyFrom(item);
    cont->set_room_id(roomId);

    emit sigSendIslMessage(msg, serverId);
}
