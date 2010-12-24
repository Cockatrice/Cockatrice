#include "server_room.h"
#include "server_protocolhandler.h"
#include "server_game.h"

Server_Room::Server_Room(int _id, const QString &_name, const QString &_description, bool _autoJoin, const QString &_joinMessage, Server *parent)
	: QObject(parent), id(_id), name(_name), description(_description), autoJoin(_autoJoin), joinMessage(_joinMessage)
{
}

Server *Server_Room::getServer() const
{
	return static_cast<Server *>(parent());
}

QList<ServerInfo_User *> Server_Room::addClient(Server_ProtocolHandler *client)
{
	sendRoomEvent(new Event_JoinRoom(id, new ServerInfo_User(client->getUserInfo())));
	append(client);
	
	QList<ServerInfo_User *> eventUserList;
	for (int i = 0; i < size(); ++i)
		eventUserList.append(new ServerInfo_User(at(i)->getUserInfo()));

	emit roomInfoChanged();
	return eventUserList;
}

void Server_Room::removeClient(Server_ProtocolHandler *client)
{
	removeAt(indexOf(client));
	sendRoomEvent(new Event_LeaveRoom(id, client->getUserInfo()->getName()));
	emit roomInfoChanged();
}

void Server_Room::say(Server_ProtocolHandler *client, const QString &s)
{
	sendRoomEvent(new Event_RoomSay(id, client->getUserInfo()->getName(), s));
}

void Server_Room::sendRoomEvent(RoomEvent *event)
{
	for (int i = 0; i < size(); ++i)
		at(i)->sendProtocolItem(event, false);
	delete event;
}

void Server_Room::broadcastGameListUpdate(Server_Game *game)
{
	Event_ListGames *event = new Event_ListGames(id, QList<ServerInfo_Game *>() << game->getInfo());
	
	for (int i = 0; i < size(); i++)
		at(i)->sendProtocolItem(event, false);
	delete event;
}

Server_Game *Server_Room::createGame(const QString &description, const QString &password, int maxPlayers, bool spectatorsAllowed, bool spectatorsNeedPassword, bool spectatorsCanTalk, bool spectatorsSeeEverything, Server_ProtocolHandler *creator)
{
	Server_Game *newGame = new Server_Game(creator, static_cast<Server *>(parent())->getNextGameId(), description, password, maxPlayers, spectatorsAllowed, spectatorsNeedPassword, spectatorsCanTalk, spectatorsSeeEverything, this);
	games.insert(newGame->getGameId(), newGame);
	connect(newGame, SIGNAL(gameClosing()), this, SLOT(removeGame()));
	
	broadcastGameListUpdate(newGame);
	
	emit gameCreated(newGame);
	
	return newGame;
}

void Server_Room::removeGame()
{
	Server_Game *game = static_cast<Server_Game *>(sender());
	broadcastGameListUpdate(game);
	games.remove(game->getGameId());
	
	emit gameClosing(game->getGameId());
}
