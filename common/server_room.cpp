#include "server_room.h"
#include "server_protocolhandler.h"
#include "server_game.h"
#include <QDebug>

#include "pb/event_join_room.pb.h"
#include "pb/event_leave_room.pb.h"
#include "pb/event_list_games.pb.h"
#include "pb/event_room_say.pb.h"
#include <google/protobuf/descriptor.h>

Server_Room::Server_Room(int _id, const QString &_name, const QString &_description, bool _autoJoin, const QString &_joinMessage, const QStringList &_gameTypes, Server *parent)
	: QObject(parent), id(_id), name(_name), description(_description), autoJoin(_autoJoin), joinMessage(_joinMessage), gameTypes(_gameTypes), roomMutex(QMutex::Recursive)
{
}

Server_Room::~Server_Room()
{
	QMutexLocker locker(&roomMutex);
	qDebug("Server_Room destructor");
	
	const QList<Server_Game *> gameList = games.values();
	for (int i = 0; i < gameList.size(); ++i)
		delete gameList[i];
	games.clear();
	
	userList.clear();
}

Server *Server_Room::getServer() const
{
	return static_cast<Server *>(parent());
}

ServerInfo_Room Server_Room::getInfo(bool complete, bool showGameTypes, bool updating) const
{
	QMutexLocker locker(&roomMutex);
	
	ServerInfo_Room result;
	result.set_room_id(id);
	result.set_game_count(games.size());
	result.set_player_count(userList.size());
	
	if (!updating) {
		result.set_name(name.toStdString());
		result.set_description(description.toStdString());
		result.set_auto_join(autoJoin);
	}
	
	if (complete) {
		QMapIterator<int, Server_Game *> gameIterator(games);
		while (gameIterator.hasNext())
			result.add_game_list()->CopyFrom(gameIterator.next().value()->getInfo());
		
		for (int i = 0; i < userList.size(); ++i)
			result.add_user_list()->CopyFrom(userList[i]->copyUserInfo(false));
	}
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
	
	roomMutex.lock();
	userList.append(client);
	roomMutex.unlock();
	
	emit roomInfoChanged();
}

void Server_Room::removeClient(Server_ProtocolHandler *client)
{
	roomMutex.lock();
	userList.removeAt(userList.indexOf(client));
	roomMutex.unlock();
	
	Event_LeaveRoom event;
	event.set_name(client->getUserInfo()->name());
	sendRoomEvent(prepareRoomEvent(event));
	
	emit roomInfoChanged();
}

void Server_Room::say(Server_ProtocolHandler *client, const QString &s)
{
	Event_RoomSay event;
	event.set_name(client->getUserInfo()->name());
	event.set_message(s.toStdString());
	sendRoomEvent(prepareRoomEvent(event));
}

void Server_Room::sendRoomEvent(RoomEvent *event)
{
	QMutexLocker locker(&roomMutex);
	
	for (int i = 0; i < userList.size(); ++i)
		userList[i]->sendProtocolItem(*event);
	delete event;
}

void Server_Room::broadcastGameListUpdate(Server_Game *game)
{
	QMutexLocker locker(&roomMutex);
	
	Event_ListGames event;
	event.add_game_list()->CopyFrom(game->getInfo());
	sendRoomEvent(prepareRoomEvent(event));
}

Server_Game *Server_Room::createGame(const QString &description, const QString &password, int maxPlayers, const QList<int> &gameTypes, bool onlyBuddies, bool onlyRegistered, bool spectatorsAllowed, bool spectatorsNeedPassword, bool spectatorsCanTalk, bool spectatorsSeeEverything, Server_ProtocolHandler *creator)
{
	QMutexLocker locker(&roomMutex);
	
	Server_Game *newGame = new Server_Game(creator, static_cast<Server *>(parent())->getNextGameId(), description, password, maxPlayers, gameTypes, onlyBuddies, onlyRegistered, spectatorsAllowed, spectatorsNeedPassword, spectatorsCanTalk, spectatorsSeeEverything, this);
	newGame->moveToThread(thread());
	// This mutex needs to be unlocked by the caller.
	newGame->gameMutex.lock();
	games.insert(newGame->getGameId(), newGame);
	
	broadcastGameListUpdate(newGame);
	
	emit roomInfoChanged();
	
	return newGame;
}

void Server_Room::removeGame(Server_Game *game)
{
	// No need to lock roomMutex or gameMutex. This method is only
	// called from ~Server_Game, which locks both mutexes anyway beforehand.
	
	broadcastGameListUpdate(game);
	games.remove(game->getGameId());
	
	emit roomInfoChanged();
}

int Server_Room::getGamesCreatedByUser(const QString &userName) const
{
	QMutexLocker locker(&roomMutex);
	
	QMapIterator<int, Server_Game *> gamesIterator(games);
	int result = 0;
	while (gamesIterator.hasNext())
		if (gamesIterator.next().value()->getCreatorInfo()->name() == userName.toStdString())
			++result;
	return result;
}

QList<ServerInfo_Game> Server_Room::getGamesOfUser(const QString &userName) const
{
	QList<ServerInfo_Game> result;
	QMapIterator<int, Server_Game *> gamesIterator(games);
	while (gamesIterator.hasNext()) {
		Server_Game *game = gamesIterator.next().value();
		if (game->containsUser(userName))
			result.append(game->getInfo());
	}
	return result;
}
