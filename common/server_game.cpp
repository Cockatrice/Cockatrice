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
#include "server_room.h"
#include "server_game.h"
#include "server_protocolhandler.h"
#include "server_arrow.h"
#include "server_card.h"
#include "server_cardzone.h"
#include "server_counter.h"
#include <QTimer>
#include <QDebug>

Server_Game::Server_Game(Server_ProtocolHandler *_creator, int _gameId, const QString &_description, const QString &_password, int _maxPlayers, const QList<int> &_gameTypes, bool _onlyBuddies, bool _onlyRegistered, bool _spectatorsAllowed, bool _spectatorsNeedPassword, bool _spectatorsCanTalk, bool _spectatorsSeeEverything, Server_Room *parent)
	: QObject(parent), creatorInfo(new ServerInfo_User(_creator->getUserInfo())), gameStarted(false), gameId(_gameId), description(_description), password(_password), maxPlayers(_maxPlayers), gameTypes(_gameTypes), activePlayer(-1), activePhase(-1), onlyBuddies(_onlyBuddies), onlyRegistered(_onlyRegistered), spectatorsAllowed(_spectatorsAllowed), spectatorsNeedPassword(_spectatorsNeedPassword), spectatorsCanTalk(_spectatorsCanTalk), spectatorsSeeEverything(_spectatorsSeeEverything), inactivityCounter(0), secondsElapsed(0), gameMutex(QMutex::Recursive)
{
	addPlayer(_creator, false, false);

	if (parent->getServer()->getGameShouldPing()) {
		pingClock = new QTimer(this);
		connect(pingClock, SIGNAL(timeout()), this, SLOT(pingClockTimeout()));
		pingClock->start(1000);
	}
}

Server_Game::~Server_Game()
{
	QMutexLocker locker(&gameMutex);
	
	sendGameEvent(new Event_GameClosed);
	
	QMapIterator<int, Server_Player *> playerIterator(players);
	while (playerIterator.hasNext())
		delete playerIterator.next().value();
	players.clear();
	
	emit gameClosing();
	delete creatorInfo;
	qDebug("Server_Game destructor");
}

void Server_Game::pingClockTimeout()
{
	QMutexLocker locker(&gameMutex);
	++secondsElapsed;
	
	QList<ServerInfo_PlayerPing *> pingList;
	QMapIterator<int, Server_Player *> playerIterator(players);
	bool allPlayersInactive = true;
	int playerCount = 0;
	while (playerIterator.hasNext()) {
		Server_Player *player = playerIterator.next().value();
		if (!player->getSpectator())
			++playerCount;
		int pingTime;
		if (player->getProtocolHandler()) {
			pingTime = player->getProtocolHandler()->getLastCommandTime();
			allPlayersInactive = false;
		} else
			pingTime = -1;
		pingList.append(new ServerInfo_PlayerPing(player->getPlayerId(), pingTime));
	}
	sendGameEvent(new Event_Ping(secondsElapsed, pingList));
	
	const int maxTime = static_cast<Server_Room *>(parent())->getServer()->getMaxGameInactivityTime();
	if (allPlayersInactive) {
		if (((++inactivityCounter >= maxTime) && (maxTime > 0)) || (playerCount < maxPlayers))
			deleteLater();
	} else
		inactivityCounter = 0;
}

int Server_Game::getPlayerCount() const
{
	QMutexLocker locker(&gameMutex);
	
	QMapIterator<int, Server_Player *> playerIterator(players);
	int result = 0;
	while (playerIterator.hasNext())
		if (!playerIterator.next().value()->getSpectator())
			++result;
	return result;
}

int Server_Game::getSpectatorCount() const
{
	QMutexLocker locker(&gameMutex);
	
	QMapIterator<int, Server_Player *> playerIterator(players);
	int result = 0;
	while (playerIterator.hasNext())
		if (playerIterator.next().value()->getSpectator())
			++result;
	return result;
}

void Server_Game::startGameIfReady()
{
	QMutexLocker locker(&gameMutex);
	
	if (getPlayerCount() < maxPlayers)
		return;
	QMapIterator<int, Server_Player *> playerIterator(players);
	while (playerIterator.hasNext()) {
		Server_Player *p = playerIterator.next().value();
		if (!p->getReadyStart() && !p->getSpectator())
			return;
	}
	playerIterator.toFront();
	while (playerIterator.hasNext()) {
		Server_Player *p = playerIterator.next().value();
		if (!p->getSpectator())
			p->setupZones();
	}

	gameStarted = true;
	playerIterator.toFront();
	while (playerIterator.hasNext()) {
		Server_Player *player = playerIterator.next().value();
		player->setConceded(false);
		player->setReadyStart(false);
		sendGameEventToPlayer(player, new Event_GameStateChanged(gameStarted, 0, 0, getGameState(player)));
	}
	
/*	QSqlQuery query;
	query.prepare("insert into games (id, descr, password, time_started) values(:id, :descr, :password, now())");
	query.bindValue(":id", gameId);
	query.bindValue(":descr", description);
	query.bindValue(":password", !password.isEmpty());
	query.exec();
	
	QMapIterator<int, Server_Player *> playerIterator2(players);
	while (playerIterator2.hasNext()) {
		Server_Player *player = playerIterator2.next().value();
		query.prepare("insert into games_players (id_game, player) values(:id, :player)");
		query.bindValue(":id", gameId);
		query.bindValue(":player", player->getPlayerName());
		query.exec();
	}
*/	
	activePlayer = -1;
	nextTurn();
}

void Server_Game::stopGameIfFinished()
{
	QMutexLocker locker(&gameMutex);
	
	QMapIterator<int, Server_Player *> playerIterator(players);
	int playing = 0;
	while (playerIterator.hasNext()) {
		Server_Player *p = playerIterator.next().value();
		if (!p->getConceded() && !p->getSpectator())
			++playing;
	}
	if (playing > 1)
		return;

	gameStarted = false;

	playerIterator.toFront();
	while (playerIterator.hasNext())
		playerIterator.next().value()->clearZones();

	playerIterator.toFront();
	while (playerIterator.hasNext()) {
		Server_Player *player = playerIterator.next().value();
		sendGameEventToPlayer(player, new Event_GameStateChanged(gameStarted, -1, -1, getGameState(player)));
	}
}

ResponseCode Server_Game::checkJoin(ServerInfo_User *user, const QString &_password, bool spectator)
{
	if ((_password != password) && !(spectator && !spectatorsNeedPassword))
		return RespWrongPassword;
	if (!(user->getUserLevel() & ServerInfo_User::IsRegistered) && onlyRegistered)
		return RespUserLevelTooLow;
	if (onlyBuddies)
		if (!static_cast<Server_Room *>(parent())->getServer()->getBuddyList(creatorInfo->getName()).contains(user->getName()))
			return RespOnlyBuddies;
	if (static_cast<Server_Room *>(parent())->getServer()->getIgnoreList(creatorInfo->getName()).contains(user->getName()))
		return RespInIgnoreList;
	if (spectator) {
		if (!spectatorsAllowed)
			return RespSpectatorsNotAllowed;
	} else if (gameStarted || (getPlayerCount() >= getMaxPlayers()))
		return RespGameFull;
	
	return RespOk;
}

Server_Player *Server_Game::addPlayer(Server_ProtocolHandler *handler, bool spectator, bool broadcastUpdate)
{
	QMutexLocker locker(&gameMutex);
	
	const QList<int> &keyList = players.keys();
	int playerId = keyList.isEmpty() ? 0 : (keyList.last() + 1);
	
	Server_Player *newPlayer = new Server_Player(this, playerId, handler->getUserInfo(), spectator, handler);
	sendGameEvent(new Event_Join(newPlayer->getProperties()));
	players.insert(playerId, newPlayer);

	if (broadcastUpdate)
		qobject_cast<Server_Room *>(parent())->broadcastGameListUpdate(this);
	
	return newPlayer;
}

void Server_Game::removePlayer(Server_Player *player)
{
	QMutexLocker locker(&gameMutex);
	
	players.remove(player->getPlayerId());
	removeArrowsToPlayer(player);
	
	sendGameEvent(new Event_Leave(player->getPlayerId()));
	bool playerActive = activePlayer == player->getPlayerId();
	bool spectator = player->getSpectator();
	delete player;
	
	if (!getPlayerCount())
		deleteLater();
	else if (!spectator) {
		stopGameIfFinished();
		if (gameStarted && playerActive)
			nextTurn();
	}
	qobject_cast<Server_Room *>(parent())->broadcastGameListUpdate(this);
}

void Server_Game::removeArrowsToPlayer(Server_Player *player)
{
	QMutexLocker locker(&gameMutex);
	
	// Remove all arrows of other players pointing to the player being removed or to one of his cards.
	QMapIterator<int, Server_Player *> playerIterator(players);
	while (playerIterator.hasNext()) {
		Server_Player *p = playerIterator.next().value();
		QList<Server_Arrow *> arrows = p->getArrows().values();
		QList<Server_Arrow *> toDelete;
		for (int i = 0; i < arrows.size(); ++i) {
			Server_Arrow *a = arrows[i];
			Server_Card *targetCard = qobject_cast<Server_Card *>(a->getTargetItem());
			if (targetCard) {
				if (targetCard->getZone()->getPlayer() == player)
					toDelete.append(a);
			} else if ((static_cast<Server_Player *>(a->getTargetItem()) == player) || (a->getStartCard()->getZone()->getPlayer() == player))
				toDelete.append(a);
		}
		for (int i = 0; i < toDelete.size(); ++i) {
			sendGameEvent(new Event_DeleteArrow(p->getPlayerId(), toDelete[i]->getId()));
			p->deleteArrow(toDelete[i]->getId());
		}
	}
}

bool Server_Game::kickPlayer(int playerId)
{
	QMutexLocker locker(&gameMutex);
	
	Server_Player *playerToKick = players.value(playerId);
	if (!playerToKick)
		return false;
	
	sendGameEventToPlayer(playerToKick, new Event_Kicked);
	removePlayer(playerToKick);
	
	return true;
}

void Server_Game::setActivePlayer(int _activePlayer)
{
	QMutexLocker locker(&gameMutex);
	
	activePlayer = _activePlayer;
	sendGameEvent(new Event_SetActivePlayer(activePlayer, activePlayer));
	setActivePhase(0);
}

void Server_Game::setActivePhase(int _activePhase)
{
	QMutexLocker locker(&gameMutex);
	
	QMapIterator<int, Server_Player *> playerIterator(players);
	while (playerIterator.hasNext()) {
		Server_Player *player = playerIterator.next().value();
		QList<Server_Arrow *> toDelete = player->getArrows().values();
		for (int i = 0; i < toDelete.size(); ++i) {
			Server_Arrow *a = toDelete[i];
			sendGameEvent(new Event_DeleteArrow(player->getPlayerId(), a->getId()));
			player->deleteArrow(a->getId());
		}
	}
	
	activePhase = _activePhase;
	sendGameEvent(new Event_SetActivePhase(-1, activePhase));
}

void Server_Game::nextTurn()
{
	QMutexLocker locker(&gameMutex);
	
	const QList<int> keys = players.keys();
	int listPos = -1;
	if (activePlayer != -1)
		listPos = keys.indexOf(activePlayer);
	do {
		++listPos;
		if (listPos == keys.size())
			listPos = 0;
	} while (players.value(keys[listPos])->getSpectator() || players.value(keys[listPos])->getConceded());
	
	setActivePlayer(keys[listPos]);
}

QList<ServerInfo_Player *> Server_Game::getGameState(Server_Player *playerWhosAsking) const
{
	QMutexLocker locker(&gameMutex);
	
	QList<ServerInfo_Player *> result;
	QMapIterator<int, Server_Player *> playerIterator(players);
	while (playerIterator.hasNext()) {
		Server_Player *player = playerIterator.next().value();

		QList<ServerInfo_Arrow *> arrowList;
		QMapIterator<int, Server_Arrow *> arrowIterator(player->getArrows());
		while (arrowIterator.hasNext()) {
			Server_Arrow *arrow = arrowIterator.next().value();
			Server_Card *targetCard = qobject_cast<Server_Card *>(arrow->getTargetItem());
			if (targetCard)
				arrowList.append(new ServerInfo_Arrow(
					arrow->getId(),
					arrow->getStartCard()->getZone()->getPlayer()->getPlayerId(),
					arrow->getStartCard()->getZone()->getName(),
					arrow->getStartCard()->getId(),
					targetCard->getZone()->getPlayer()->getPlayerId(),
					targetCard->getZone()->getName(),
					targetCard->getId(),
					arrow->getColor()
				));
			else
				arrowList.append(new ServerInfo_Arrow(
					arrow->getId(),
					arrow->getStartCard()->getZone()->getPlayer()->getPlayerId(),
					arrow->getStartCard()->getZone()->getName(),
					arrow->getStartCard()->getId(),
					qobject_cast<Server_Player *>(arrow->getTargetItem())->getPlayerId(),
					QString(),
					-1,
					arrow->getColor()
				));
		}

		QList<ServerInfo_Counter *> counterList;
		QMapIterator<int, Server_Counter *> counterIterator(player->getCounters());
		while (counterIterator.hasNext()) {
			Server_Counter *counter = counterIterator.next().value();
			counterList.append(new ServerInfo_Counter(counter->getId(), counter->getName(), counter->getColor(), counter->getRadius(), counter->getCount()));
		}

		QList<ServerInfo_Zone *> zoneList;
		QMapIterator<QString, Server_CardZone *> zoneIterator(player->getZones());
		while (zoneIterator.hasNext()) {
			Server_CardZone *zone = zoneIterator.next().value();
			QList<ServerInfo_Card *> cardList;
			if (
				(((playerWhosAsking == player) || (playerWhosAsking->getSpectator() && spectatorsSeeEverything)) && (zone->getType() != HiddenZone))
				|| ((playerWhosAsking != player) && (zone->getType() == PublicZone))
			) {
				QListIterator<Server_Card *> cardIterator(zone->cards);
				while (cardIterator.hasNext()) {
					Server_Card *card = cardIterator.next();
					QString displayedName = card->getFaceDown() ? QString() : card->getName();
					
					QList<ServerInfo_CardCounter *> cardCounterList;
					QMapIterator<int, int> cardCounterIterator(card->getCounters());
					while (cardCounterIterator.hasNext()) {
						cardCounterIterator.next();
						cardCounterList.append(new ServerInfo_CardCounter(cardCounterIterator.key(), cardCounterIterator.value()));
					}
					
					int attachPlayerId = -1;
					QString attachZone;
					int attachCardId = -1;
					if (card->getParentCard()) {
						attachPlayerId = card->getParentCard()->getZone()->getPlayer()->getPlayerId();
						attachZone = card->getParentCard()->getZone()->getName();
						attachCardId = card->getParentCard()->getId();
					}
					cardList.append(new ServerInfo_Card(card->getId(), displayedName, card->getX(), card->getY(), card->getTapped(), card->getAttacking(), card->getColor(), card->getPT(), card->getAnnotation(), card->getDestroyOnZoneChange(), cardCounterList, attachPlayerId, attachZone, attachCardId));
				}
			}
			zoneList.append(new ServerInfo_Zone(zone->getName(), zone->getType(), zone->hasCoords(), zone->cards.size(), cardList));
		}

		result.append(new ServerInfo_Player(player->getProperties(), player == playerWhosAsking ? player->getDeck() : 0, zoneList, counterList, arrowList));
	}
	return result;
}

void Server_Game::sendGameEvent(GameEvent *event, GameEventContext *context, Server_Player *exclude)
{
	sendGameEventContainer(new GameEventContainer(QList<GameEvent *>() << event, -1, context), exclude);
}

void Server_Game::sendGameEventContainer(GameEventContainer *cont, Server_Player *exclude, bool excludeOmniscient)
{
	QMutexLocker locker(&gameMutex);
	
	cont->setGameId(gameId);
	QMapIterator<int, Server_Player *> playerIterator(players);
	while (playerIterator.hasNext()) {
		Server_Player *p = playerIterator.next().value();
		if ((p != exclude) && !(excludeOmniscient && p->getSpectator() && spectatorsSeeEverything))
			p->sendProtocolItem(cont, false);
	}

	delete cont;
}

void Server_Game::sendGameEventContainerOmniscient(GameEventContainer *cont, Server_Player *exclude)
{
	QMutexLocker locker(&gameMutex);
	
	cont->setGameId(gameId);
	QMapIterator<int, Server_Player *> playerIterator(players);
	while (playerIterator.hasNext()) {
		Server_Player *p = playerIterator.next().value();
		if ((p != exclude) && (p->getSpectator() && spectatorsSeeEverything))
			p->sendProtocolItem(cont, false);
	}
	
	delete cont;
}

void Server_Game::sendGameEventToPlayer(Server_Player *player, GameEvent *event)
{
	player->sendProtocolItem(new GameEventContainer(QList<GameEvent *>() << event, gameId));
}

ServerInfo_Game *Server_Game::getInfo() const
{
	QMutexLocker locker(&gameMutex);
	
	if (players.isEmpty())
		// Game is closing
		return new ServerInfo_Game(getGameId(), QString(), false, 0, getMaxPlayers(), QList<GameTypeId *>(), 0, false, 0);
	else {
		// Game is open
		
		QList<GameTypeId *> gameTypeList;
		for (int i = 0; i < gameTypes.size(); ++i)
			gameTypeList.append(new GameTypeId(gameTypes[i]));
		
		return new ServerInfo_Game(
			getGameId(),
			getDescription(),
			!getPassword().isEmpty(),
			getPlayerCount(),
			getMaxPlayers(),
			gameTypeList,
			new ServerInfo_User(getCreatorInfo(), false),
			onlyBuddies,
			onlyRegistered,
			getSpectatorsAllowed(),
			getSpectatorsNeedPassword(),
			getSpectatorCount()
		);
	}
}