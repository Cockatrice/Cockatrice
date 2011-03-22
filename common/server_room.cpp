#include "server_room.h"
#include "server_protocolhandler.h"
#include "server_game.h"
#include <QDebug>

Server_Room::Server_Room(int _id, const QString &_name, const QString &_description, bool _autoJoin, const QString &_joinMessage, const QStringList &_gameTypes, Server *parent)
	: QObject(parent), id(_id), name(_name), description(_description), autoJoin(_autoJoin), joinMessage(_joinMessage), gameTypes(_gameTypes)
{
	connect(this, SIGNAL(sigCreateGame(const QString &, const QString &, int, const QList<int> &, bool, bool, bool, bool, bool, bool, Server_ProtocolHandler *)), this, SLOT(doCreateGame(const QString &, const QString &, int, const QList<int> &, bool, bool, bool, bool, bool, bool, Server_ProtocolHandler *)));
}

Server *Server_Room::getServer() const
{
	return static_cast<Server *>(parent());
}

ServerInfo_Room *Server_Room::getInfo(bool complete) const
{
	QList<ServerInfo_Game *> gameList;
	QList<ServerInfo_User *> userList;
	QList<ServerInfo_GameType *> gameTypeList;
	if (complete) {
		QMapIterator<int, Server_Game *> gameIterator(games);
		while (gameIterator.hasNext())
			gameList.append(gameIterator.next().value()->getInfo());
		
		for (int i = 0; i < size(); ++i)
			userList.append(new ServerInfo_User(at(i)->getUserInfo(), false));
		
		for (int i = 0; i < gameTypes.size(); ++i)
			gameTypeList.append(new ServerInfo_GameType(i, gameTypes[i]));
	}
	
	return new ServerInfo_Room(id, name, description, games.size(), size(), autoJoin, gameList, userList, gameTypeList);
}

void Server_Room::addClient(Server_ProtocolHandler *client)
{
	sendRoomEvent(new Event_JoinRoom(id, new ServerInfo_User(client->getUserInfo(), false)));
	append(client);
	emit roomInfoChanged();
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

void Server_Room::doCreateGame(const QString &description, const QString &password, int maxPlayers, const QList<int> &gameTypes, bool onlyBuddies, bool onlyRegistered, bool spectatorsAllowed, bool spectatorsNeedPassword, bool spectatorsCanTalk, bool spectatorsSeeEverything, Server_ProtocolHandler *creator)
{
	Server_Game *newGame = new Server_Game(creator, static_cast<Server *>(parent())->getNextGameId(), description, password, maxPlayers, gameTypes, onlyBuddies, onlyRegistered, spectatorsAllowed, spectatorsNeedPassword, spectatorsCanTalk, spectatorsSeeEverything, this);
	games.insert(newGame->getGameId(), newGame);
	connect(newGame, SIGNAL(gameClosing()), this, SLOT(removeGame()));
	
	broadcastGameListUpdate(newGame);
	
	emit gameCreated(newGame);
	emit roomInfoChanged();
}

void Server_Room::createGame(const QString &description, const QString &password, int maxPlayers, const QList<int> &gameTypes, bool onlyBuddies, bool onlyRegistered, bool spectatorsAllowed, bool spectatorsNeedPassword, bool spectatorsCanTalk, bool spectatorsSeeEverything, Server_ProtocolHandler *creator)
{
	emit sigCreateGame(description, password, maxPlayers, gameTypes, onlyBuddies, onlyRegistered, spectatorsAllowed, spectatorsNeedPassword, spectatorsCanTalk, spectatorsSeeEverything, creator);
}

void Server_Room::removeGame()
{
	Server_Game *game = static_cast<Server_Game *>(sender());
	broadcastGameListUpdate(game);
	games.remove(game->getGameId());
	
	emit gameClosing(game->getGameId());
	emit roomInfoChanged();
}
