#include "server_room.h"
#include "server_protocolhandler.h"
#include "server_game.h"
#include <QDebug>

#include "pb/commands.pb.h"
#include "pb/room_commands.pb.h"
#include "pb/event_join_room.pb.h"
#include "pb/event_leave_room.pb.h"
#include "pb/event_list_games.pb.h"
#include "pb/event_room_say.pb.h"
#include <google/protobuf/descriptor.h>

Server_Room::Server_Room(int _id, const QString &_name, const QString &_description, bool _autoJoin, const QString &_joinMessage, const QStringList &_gameTypes, Server *parent)
	: QObject(parent), id(_id), name(_name), description(_description), autoJoin(_autoJoin), joinMessage(_joinMessage), gameTypes(_gameTypes), gamesMutex(QMutex::Recursive)
{
	connect(this, SIGNAL(gameListChanged(ServerInfo_Game)), this, SLOT(broadcastGameListUpdate(ServerInfo_Game)), Qt::QueuedConnection);
}

Server_Room::~Server_Room()
{
	qDebug("Server_Room destructor");
	
	gamesMutex.lock();
	const QList<Server_Game *> gameList = games.values();
	for (int i = 0; i < gameList.size(); ++i)
		delete gameList[i];
	games.clear();
	gamesMutex.unlock();
	
	usersLock.lockForWrite();
	userList.clear();
	usersLock.unlock();
}

Server *Server_Room::getServer() const
{
	return static_cast<Server *>(parent());
}

ServerInfo_Room Server_Room::getInfo(bool complete, bool showGameTypes, bool updating, bool includeExternalData) const
{
	ServerInfo_Room result;
	result.set_room_id(id);
	
	if (!updating) {
		result.set_name(name.toStdString());
		result.set_description(description.toStdString());
		result.set_auto_join(autoJoin);
	}
	
	gamesMutex.lock();
	result.set_game_count(games.size() + externalGames.size());
	if (complete) {
		QMapIterator<int, Server_Game *> gameIterator(games);
		while (gameIterator.hasNext())
			result.add_game_list()->CopyFrom(gameIterator.next().value()->getInfo());
		if (includeExternalData) {
			QMapIterator<int, ServerInfo_Game> externalGameIterator(externalGames);
			while (externalGameIterator.hasNext())
				result.add_game_list()->CopyFrom(externalGameIterator.next().value());
		}
	}
	gamesMutex.unlock();
	
	usersLock.lockForRead();
	result.set_player_count(userList.size() + externalUsers.size());
	if (complete) {
		for (int i = 0; i < userList.size(); ++i)
			result.add_user_list()->CopyFrom(userList[i]->copyUserInfo(false));
		if (includeExternalData) {
			QMapIterator<QString, ServerInfo_User_Container> externalUserIterator(externalUsers);
			while (externalUserIterator.hasNext())
				result.add_user_list()->CopyFrom(externalUserIterator.next().value().copyUserInfo(false));
		}
	}
	usersLock.unlock();
	
	if (complete || showGameTypes)
		for (int i = 0; i < gameTypes.size(); ++i) {
			ServerInfo_GameType *gameTypeInfo = result.add_gametype_list();
			gameTypeInfo->set_game_type_id(i);
			gameTypeInfo->set_description(gameTypes[i].toStdString());
		}
	
	return result;
}

RoomEvent *Server_Room::prepareRoomEvent(const ::google::protobuf::Message &roomEvent)
{
	RoomEvent *event = new RoomEvent;
	event->set_room_id(id);
	event->GetReflection()->MutableMessage(event, roomEvent.GetDescriptor()->FindExtensionByName("ext"))->CopyFrom(roomEvent);
	return event;
}

void Server_Room::addClient(Server_ProtocolHandler *client)
{
	Event_JoinRoom event;
	event.mutable_user_info()->CopyFrom(client->copyUserInfo(false));
	sendRoomEvent(prepareRoomEvent(event));
	
	usersLock.lockForWrite();
	userList.append(client);
	usersLock.unlock();
	
	emit roomInfoChanged(getInfo(false, false, true));
}

void Server_Room::removeClient(Server_ProtocolHandler *client)
{
	usersLock.lockForWrite();
	userList.removeAt(userList.indexOf(client));
	usersLock.unlock();
	
	Event_LeaveRoom event;
	event.set_name(client->getUserInfo()->name());
	sendRoomEvent(prepareRoomEvent(event));
	
	emit roomInfoChanged(getInfo(false, false, true));
}

void Server_Room::addExternalUser(const ServerInfo_User &userInfo)
{
	// This function is always called from the Server thread with server->roomsMutex locked.
	ServerInfo_User_Container userInfoContainer(userInfo);
	Event_JoinRoom event;
	event.mutable_user_info()->CopyFrom(userInfoContainer.copyUserInfo(false));
	sendRoomEvent(prepareRoomEvent(event), false);
	
	usersLock.lockForWrite();
	externalUsers.insert(QString::fromStdString(userInfo.name()), userInfoContainer);
	usersLock.unlock();
	
	emit roomInfoChanged(getInfo(false, false, true));
}

void Server_Room::removeExternalUser(const QString &name)
{
	// This function is always called from the Server thread with server->roomsMutex locked.
	usersLock.lockForWrite();
	if (externalUsers.contains(name))
		externalUsers.remove(name);
	usersLock.unlock();
	
	Event_LeaveRoom event;
	event.set_name(name.toStdString());
	sendRoomEvent(prepareRoomEvent(event), false);
	
	emit roomInfoChanged(getInfo(false, false, true));
}

void Server_Room::updateExternalGameList(const ServerInfo_Game &gameInfo)
{
	// This function is always called from the Server thread with server->roomsMutex locked.
	gamesMutex.lock();
	if (!gameInfo.has_player_count() && externalGames.contains(gameInfo.game_id()))
		externalGames.remove(gameInfo.game_id());
	else
		externalGames.insert(gameInfo.game_id(), gameInfo);
	gamesMutex.unlock();
	
	broadcastGameListUpdate(gameInfo, false);
	emit roomInfoChanged(getInfo(false, false, true));
}

Response::ResponseCode Server_Room::processJoinGameCommand(const Command_JoinGame &cmd, ResponseContainer &rc, Server_AbstractUserInterface *userInterface)
{
	// This function is called from the Server thread and from the S_PH thread.
	// server->roomsMutex is always locked.
	
	QMutexLocker roomGamesLocker(&gamesMutex);
	Server_Game *g = games.value(cmd.game_id());
	if (!g) {
		if (externalGames.contains(cmd.game_id())) {
			CommandContainer cont;
			cont.set_cmd_id(rc.getCmdId());
			RoomCommand *roomCommand = cont.add_room_command();
			roomCommand->GetReflection()->MutableMessage(roomCommand, cmd.GetDescriptor()->FindExtensionByName("ext"))->CopyFrom(cmd);
			getServer()->sendIsl_RoomCommand(cont, externalGames.value(cmd.game_id()).server_id(), userInterface->getUserInfo()->session_id(), id);
			
			return Response::RespNothing;
		} else
			return Response::RespNameNotFound;
	}
	
	QMutexLocker gameLocker(&g->gameMutex);
	
	Response::ResponseCode result = g->checkJoin(userInterface->getUserInfo(), QString::fromStdString(cmd.password()), cmd.spectator(), cmd.override_restrictions());
	if (result == Response::RespOk)
		g->addPlayer(userInterface, rc, cmd.spectator());
	
	return result;
}


void Server_Room::say(const QString &userName, const QString &s, bool sendToIsl)
{
	Event_RoomSay event;
	event.set_name(userName.toStdString());
	event.set_message(s.toStdString());
	sendRoomEvent(prepareRoomEvent(event), sendToIsl);
}

void Server_Room::sendRoomEvent(RoomEvent *event, bool sendToIsl)
{
	usersLock.lockForRead();
	for (int i = 0; i < userList.size(); ++i)
		userList[i]->sendProtocolItem(*event);
	usersLock.unlock();
	
	if (sendToIsl)
		static_cast<Server *>(parent())->sendIsl_RoomEvent(*event);
	
	delete event;
}

void Server_Room::broadcastGameListUpdate(ServerInfo_Game gameInfo, bool sendToIsl)
{
	Event_ListGames event;
	event.add_game_list()->CopyFrom(gameInfo);
	sendRoomEvent(prepareRoomEvent(event), sendToIsl);
}

void Server_Room::addGame(Server_Game *game)
{
	// Lock gamesMutex before calling this
	
	game->moveToThread(thread());
	connect(game, SIGNAL(gameInfoChanged(ServerInfo_Game)), this, SLOT(broadcastGameListUpdate(ServerInfo_Game)));
	
	game->gameMutex.lock();
	games.insert(game->getGameId(), game);
	emit gameListChanged(game->getInfo());
	game->gameMutex.unlock();
	
	emit roomInfoChanged(getInfo(false, false, true));
}

void Server_Room::removeGame(Server_Game *game)
{
	// No need to lock gamesMutex or gameMutex. This method is only
	// called from ~Server_Game, which locks both mutexes anyway beforehand.
	
	disconnect(game, 0, this, 0);
	emit gameListChanged(game->getInfo());
	games.remove(game->getGameId());
	
	emit roomInfoChanged(getInfo(false, false, true));
}

int Server_Room::getGamesCreatedByUser(const QString &userName) const
{
	QMutexLocker locker(&gamesMutex);
	
	QMapIterator<int, Server_Game *> gamesIterator(games);
	int result = 0;
	while (gamesIterator.hasNext())
		if (gamesIterator.next().value()->getCreatorInfo()->name() == userName.toStdString())
			++result;
	return result;
}

QList<ServerInfo_Game> Server_Room::getGamesOfUser(const QString &userName) const
{
	QMutexLocker locker(&gamesMutex);
	
	QList<ServerInfo_Game> result;
	QMapIterator<int, Server_Game *> gamesIterator(games);
	while (gamesIterator.hasNext()) {
		Server_Game *game = gamesIterator.next().value();
		if (game->containsUser(userName))
			result.append(game->getInfo());
	}
	return result;
}
