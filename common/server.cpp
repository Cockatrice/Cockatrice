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
#include "server_chatchannel.h"

Server::Server(QObject *parent)
	: QObject(parent), nextGameId(0)
{
}

Server::~Server()
{
}

Server_Game *Server::createGame(const QString &description, const QString &password, int maxPlayers, bool spectatorsAllowed, const QString &creator)
{
	Server_Game *newGame = new Server_Game(creator, nextGameId++, description, password, maxPlayers, spectatorsAllowed, this);
	games.insert(newGame->getGameId(), newGame);
	connect(newGame, SIGNAL(gameClosing()), this, SLOT(gameClosing()));
	
	broadcastGameListUpdate(newGame);
	
	return newGame;
}

void Server::addClient(Server_ProtocolHandler *client)
{
	clients << client;
}

void Server::removeClient(Server_ProtocolHandler *client)
{
	clients.removeAt(clients.indexOf(client));
	qDebug(QString("Server::removeClient: %1 clients left").arg(clients.size()).toLatin1());
}

Server_Game *Server::getGame(int gameId) const
{
	return games.value(gameId);
}

void Server::broadcastGameListUpdate(Server_Game *game)
{
/*	QString line = game->getGameListLine();
	for (int i = 0; i < clients.size(); i++)
		if (clients[i]->getAcceptsGameListChanges())
			clients[i]->msg(line);
*/}

void Server::broadcastChannelUpdate()
{
/*	QString line = qobject_cast<Server_ChatChannel *>(sender())->getChannelListLine();
	for (int i = 0; i < players.size(); ++i)
	  	if (players[i]->getAcceptsChatChannelListChanges())
		  	players[i]->msg(line);
*/}

void Server::gameClosing()
{
	qDebug("Server::gameClosing");
	Server_Game *game = static_cast<Server_Game *>(sender());
	broadcastGameListUpdate(game);
	games.remove(games.key(game));
}

void Server::addChatChannel(Server_ChatChannel *newChannel)
{
	chatChannels.insert(newChannel->getName(), newChannel);
	connect(newChannel, SIGNAL(channelInfoChanged()), this, SLOT(broadcastChannelUpdate()));
}
