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
#include "decklist.h"
#include "pb/context_connection_state_changed.pb.h"
#include "pb/context_ping_changed.pb.h"
#include "pb/event_player_properties_changed.pb.h"
#include "pb/event_game_closed.pb.h"
#include "pb/event_game_host_changed.pb.h"
#include "pb/event_game_state_changed.pb.h"
#include "pb/event_kicked.pb.h"
#include "pb/event_join.pb.h"
#include "pb/event_leave.pb.h"
#include "pb/event_delete_arrow.pb.h"
#include "pb/event_set_active_player.pb.h"
#include "pb/event_set_active_phase.pb.h"
#include "pb/serverinfo_playerping.pb.h"
#include "pb/game_replay.pb.h"
#include <google/protobuf/descriptor.h>
#include <QTimer>
#include <QDebug>

Server_Game::Server_Game(Server_ProtocolHandler *_creator, int _gameId, const QString &_description, const QString &_password, int _maxPlayers, const QList<int> &_gameTypes, bool _onlyBuddies, bool _onlyRegistered, bool _spectatorsAllowed, bool _spectatorsNeedPassword, bool _spectatorsCanTalk, bool _spectatorsSeeEverything, Server_Room *_room)
	: QObject(), room(_room), hostId(0), creatorInfo(new ServerInfo_User(_creator->copyUserInfo(false))), gameStarted(false), gameId(_gameId), description(_description), password(_password), maxPlayers(_maxPlayers), gameTypes(_gameTypes), activePlayer(-1), activePhase(-1), onlyBuddies(_onlyBuddies), onlyRegistered(_onlyRegistered), spectatorsAllowed(_spectatorsAllowed), spectatorsNeedPassword(_spectatorsNeedPassword), spectatorsCanTalk(_spectatorsCanTalk), spectatorsSeeEverything(_spectatorsSeeEverything), inactivityCounter(0), startTimeOfThisGame(0), secondsElapsed(0), firstGameStarted(false), startTime(QDateTime::currentDateTime()), gameMutex(QMutex::Recursive)
{
	currentReplay = new GameReplay;
	currentReplay->set_replay_id(room->getServer()->getNextReplayId());
	
	connect(this, SIGNAL(sigStartGameIfReady()), this, SLOT(doStartGameIfReady()), Qt::QueuedConnection);
	
	addPlayer(_creator, false, false);

	currentReplay->mutable_game_info()->CopyFrom(getInfo());

	if (room->getServer()->getGameShouldPing()) {
		pingClock = new QTimer(this);
		connect(pingClock, SIGNAL(timeout()), this, SLOT(pingClockTimeout()));
		pingClock->start(1000);
	}
}

Server_Game::~Server_Game()
{
	room->roomMutex.lock();
	gameMutex.lock();
	
	sendGameEventContainer(prepareGameEvent(Event_GameClosed(), -1));
	
	QMapIterator<int, Server_Player *> playerIterator(players);
	while (playerIterator.hasNext())
		playerIterator.next().value()->prepareDestroy();
	players.clear();
	
	room->removeGame(this);
	delete creatorInfo;
	creatorInfo = 0;
	
	gameMutex.unlock();
	room->roomMutex.unlock();
	
	currentReplay->set_duration_seconds(secondsElapsed - startTimeOfThisGame);
	replayList.append(currentReplay);
	room->getServer()->storeGameInformation(secondsElapsed, allPlayersEver, allSpectatorsEver, replayList);
	
	for (int i = 0; i < replayList.size(); ++i)
		delete replayList[i];
	
	qDebug() << "Server_Game destructor: gameId=" << gameId;
}

void Server_Game::pingClockTimeout()
{
	QMutexLocker locker(&gameMutex);
	++secondsElapsed;
	
	GameEventStorage ges;
	ges.setGameEventContext(Context_PingChanged());
	
	QList<ServerInfo_PlayerPing *> pingList;
	QMapIterator<int, Server_Player *> playerIterator(players);
	bool allPlayersInactive = true;
	int playerCount = 0;
	while (playerIterator.hasNext()) {
		Server_Player *player = playerIterator.next().value();
		if (!player->getSpectator())
			++playerCount;
		
		const int oldPingTime = player->getPingTime();
		player->playerMutex.lock();
		int newPingTime;
		if (player->getProtocolHandler())
			newPingTime = player->getProtocolHandler()->getLastCommandTime();
		else
			newPingTime = -1;
		player->playerMutex.unlock();
		
		if ((newPingTime != -1) && !player->getSpectator())
			allPlayersInactive = false;
		
		if ((abs(oldPingTime - newPingTime) > 1) || ((newPingTime == -1) && (oldPingTime != -1)) || ((newPingTime != -1) && (oldPingTime == -1))) {
			player->setPingTime(newPingTime);
			
			Event_PlayerPropertiesChanged event;
			event.mutable_player_properties()->set_ping_seconds(newPingTime);
			ges.enqueueGameEvent(event, player->getPlayerId());
		}
	}
	ges.sendToGame(this);
	
	const int maxTime = room->getServer()->getMaxGameInactivityTime();
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

void Server_Game::sendGameStateToPlayers()
{
	// Prepare game state information that everyone can see
	Event_GameStateChanged gameStateChangedEvent;
	gameStateChangedEvent.set_seconds_elapsed(secondsElapsed);
	if (gameStarted) {
		gameStateChangedEvent.set_game_started(true);
		gameStateChangedEvent.set_active_player_id(0);
		gameStateChangedEvent.set_active_phase(0);
	} else
		gameStateChangedEvent.set_game_started(false);
	
	// game state information for replay and omniscient spectators
	Event_GameStateChanged omniscientEvent(gameStateChangedEvent);
	QListIterator<ServerInfo_Player> omniscientGameStateIterator(getGameState(0, true));
	while (omniscientGameStateIterator.hasNext())
		omniscientEvent.add_player_list()->CopyFrom(omniscientGameStateIterator.next());
	
	GameEventContainer *replayCont = prepareGameEvent(omniscientEvent, -1);
	replayCont->set_seconds_elapsed(secondsElapsed - startTimeOfThisGame);
	replayCont->clear_game_id();
	currentReplay->add_event_list()->CopyFrom(*replayCont);
	delete replayCont;
	
	// If spectators are not omniscient, we need an additional getGameState call, otherwise we can use the data we used for the replay.
	// All spectators are equal, so we don't need to make a getGameState call for each one.
	Event_GameStateChanged spectatorEvent(spectatorsSeeEverything ? omniscientEvent : gameStateChangedEvent);
	if (!spectatorsSeeEverything) {
		QListIterator<ServerInfo_Player> spectatorGameStateIterator(getGameState(0, false));
		while (spectatorGameStateIterator.hasNext())
			spectatorEvent.add_player_list()->CopyFrom(spectatorGameStateIterator.next());
	}
	
	// send game state info to clients according to their role in the game
	QMapIterator<int, Server_Player *> playerIterator(players);
	while (playerIterator.hasNext()) {
		Server_Player *player = playerIterator.next().value();
		GameEventContainer *gec;
		if (player->getSpectator())
			gec = prepareGameEvent(spectatorEvent, -1);
		else {
			Event_GameStateChanged event(gameStateChangedEvent);
			QListIterator<ServerInfo_Player> gameStateIterator(getGameState(player));
			while (gameStateIterator.hasNext())
				event.add_player_list()->CopyFrom(gameStateIterator.next());
			
			gec = prepareGameEvent(event, -1);
		}
		player->sendGameEvent(*gec);
		delete gec;
	}
}

void Server_Game::doStartGameIfReady()
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
	}
	
	if (firstGameStarted) {
		currentReplay->set_duration_seconds(secondsElapsed - startTimeOfThisGame);
		replayList.append(currentReplay);
		currentReplay = new GameReplay;
		currentReplay->set_replay_id(room->getServer()->getNextReplayId());
		currentReplay->mutable_game_info()->CopyFrom(getInfo());
		
		Event_GameStateChanged omniscientEvent;
		QListIterator<ServerInfo_Player> omniscientGameStateIterator(getGameState(0, true, true));
		while (omniscientGameStateIterator.hasNext())
			omniscientEvent.add_player_list()->CopyFrom(omniscientGameStateIterator.next());
		
		GameEventContainer *replayCont = prepareGameEvent(omniscientEvent, -1);
		replayCont->set_seconds_elapsed(0);
		replayCont->clear_game_id();
		currentReplay->add_event_list()->CopyFrom(*replayCont);
		delete replayCont;
		
		startTimeOfThisGame = secondsElapsed;
	} else
		firstGameStarted = true;
	
	sendGameStateToPlayers();
	
	activePlayer = -1;
	nextTurn();
	
	locker.unlock();
	room->broadcastGameListUpdate(this);
}

void Server_Game::startGameIfReady()
{
	emit sigStartGameIfReady();
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
	while (playerIterator.hasNext()) {
		Server_Player *p = playerIterator.next().value();
		p->clearZones();
		p->setConceded(false);
	}

	sendGameStateToPlayers();
}

Response::ResponseCode Server_Game::checkJoin(ServerInfo_User *user, const QString &_password, bool spectator, bool overrideRestrictions)
{
	if (!(overrideRestrictions && (user->user_level() & ServerInfo_User::IsModerator))) {
		if ((_password != password) && !(spectator && !spectatorsNeedPassword))
			return Response::RespWrongPassword;
		if (!(user->user_level() & ServerInfo_User::IsRegistered) && onlyRegistered)
			return Response::RespUserLevelTooLow;
		if (onlyBuddies)
			if (!room->getServer()->isInBuddyList(QString::fromStdString(creatorInfo->name()), QString::fromStdString(user->name())))
				return Response::RespOnlyBuddies;
		if (room->getServer()->isInIgnoreList(QString::fromStdString(creatorInfo->name()), QString::fromStdString(user->name())))
			return Response::RespInIgnoreList;
		if (spectator) {
			if (!spectatorsAllowed)
				return Response::RespSpectatorsNotAllowed;
		}
	}
	if (!spectator && (gameStarted || (getPlayerCount() >= getMaxPlayers())))
		return Response::RespGameFull;
	
	return Response::RespOk;
}

bool Server_Game::containsUser(const QString &userName) const
{
	QMutexLocker locker(&gameMutex);
	
	QMapIterator<int, Server_Player *> playerIterator(players);
	while (playerIterator.hasNext())
		if (playerIterator.next().value()->getUserInfo()->name() == userName.toStdString())
			return true;
	return false;
}

Server_Player *Server_Game::addPlayer(Server_ProtocolHandler *handler, bool spectator, bool broadcastUpdate)
{
	QMutexLocker locker(&gameMutex);
	
	const QList<int> &keyList = players.keys();
	int playerId = keyList.isEmpty() ? 0 : (keyList.last() + 1);
	
	Server_Player *newPlayer = new Server_Player(this, playerId, handler->copyUserInfo(true), spectator, handler);
	newPlayer->moveToThread(thread());
	
	Event_Join joinEvent;
	joinEvent.mutable_player_properties()->CopyFrom(newPlayer->getProperties(true));
	sendGameEventContainer(prepareGameEvent(joinEvent, -1));
	
	if (spectator)
		allSpectatorsEver.insert(QString::fromStdString(newPlayer->getUserInfo()->name()));
	else
		allPlayersEver.insert(QString::fromStdString(newPlayer->getUserInfo()->name()));
	players.insert(playerId, newPlayer);
	if (newPlayer->getUserInfo()->name() == creatorInfo->name()) {
		hostId = playerId;
		sendGameEventContainer(prepareGameEvent(Event_GameHostChanged(), playerId));
	}

	if (broadcastUpdate)
		room->broadcastGameListUpdate(this);
	
	return newPlayer;
}

void Server_Game::removePlayer(Server_Player *player)
{
	QMutexLocker roomLocker(&room->roomMutex);
	QMutexLocker locker(&gameMutex);
	
	players.remove(player->getPlayerId());
	
	GameEventStorage ges;
	removeArrowsToPlayer(ges, player);
	unattachCards(ges, player);
	ges.enqueueGameEvent(Event_Leave(), player->getPlayerId());
	ges.sendToGame(this);
	
	bool playerActive = activePlayer == player->getPlayerId();
	bool playerHost = hostId == player->getPlayerId();
	bool spectator = player->getSpectator();
	player->prepareDestroy();
	
	if (!getPlayerCount())
		deleteLater();
	else if (!spectator) {
		if (playerHost) {
			int newHostId = -1;
			QMapIterator<int, Server_Player *> playerIterator(players);
			while (playerIterator.hasNext()) {
				Server_Player *p = playerIterator.next().value();
				if (!p->getSpectator()) {
					newHostId = p->getPlayerId();
					break;
				}
			}
			if (newHostId != -1) {
				hostId = newHostId;
				sendGameEventContainer(prepareGameEvent(Event_GameHostChanged(), hostId));
			}
		}
		stopGameIfFinished();
		if (gameStarted && playerActive)
			nextTurn();
	}
	room->broadcastGameListUpdate(this);
}

void Server_Game::removeArrowsToPlayer(GameEventStorage &ges, Server_Player *player)
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
			Event_DeleteArrow event;
			event.set_arrow_id(toDelete[i]->getId());
			ges.enqueueGameEvent(event, p->getPlayerId());
			
			p->deleteArrow(toDelete[i]->getId());
		}
	}
}

void Server_Game::unattachCards(GameEventStorage &ges, Server_Player *player)
{
	QMutexLocker locker(&gameMutex);
	
	QMapIterator<QString, Server_CardZone *> zoneIterator(player->getZones());
	while (zoneIterator.hasNext()) {
		Server_CardZone *zone = zoneIterator.next().value();
		for (int i = 0; i < zone->cards.size(); ++i) {
			Server_Card *card = zone->cards.at(i);
			
			// Make a copy of the list because the original one gets modified during the loop
			QList<Server_Card *> attachedCards = card->getAttachedCards();
			for (int i = 0; i < attachedCards.size(); ++i)
				attachedCards[i]->getZone()->getPlayer()->unattachCard(ges, attachedCards[i]);
		}
	}
}

bool Server_Game::kickPlayer(int playerId)
{
	QMutexLocker roomLocker(&room->roomMutex);
	QMutexLocker locker(&gameMutex);
	
	Server_Player *playerToKick = players.value(playerId);
	if (!playerToKick)
		return false;
	
	GameEventContainer *gec = prepareGameEvent(Event_Kicked(), -1);
	playerToKick->sendGameEvent(*gec);
	delete gec;
	
	removePlayer(playerToKick);
	
	return true;
}

void Server_Game::setActivePlayer(int _activePlayer)
{
	QMutexLocker locker(&gameMutex);
	
	activePlayer = _activePlayer;
	
	Event_SetActivePlayer event;
	event.set_active_player_id(activePlayer);
	sendGameEventContainer(prepareGameEvent(event, -1));
	
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
			
			Event_DeleteArrow event;
			event.set_arrow_id(a->getId());
			sendGameEventContainer(prepareGameEvent(event, player->getPlayerId()));
			
			player->deleteArrow(a->getId());
		}
	}
	
	activePhase = _activePhase;
	
	Event_SetActivePhase event;
	event.set_phase(activePhase);
	sendGameEventContainer(prepareGameEvent(event, -1));
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

QList<ServerInfo_Player> Server_Game::getGameState(Server_Player *playerWhosAsking, bool omniscient, bool withUserInfo) const
{
	QMutexLocker locker(&gameMutex);
	
	QList<ServerInfo_Player> result;
	QMapIterator<int, Server_Player *> playerIterator(players);
	while (playerIterator.hasNext()) {
		Server_Player *player = playerIterator.next().value();
		
		ServerInfo_Player playerInfo;
		playerInfo.mutable_properties()->CopyFrom(player->getProperties(withUserInfo));
		if (player == playerWhosAsking)
			if (player->getDeck())
				playerInfo.set_deck_list(player->getDeck()->writeToString_Native().toStdString());
		
		QList<ServerInfo_Arrow *> arrowList;
		QMapIterator<int, Server_Arrow *> arrowIterator(player->getArrows());
		while (arrowIterator.hasNext()) {
			Server_Arrow *arrow = arrowIterator.next().value();
			Server_Card *targetCard = qobject_cast<Server_Card *>(arrow->getTargetItem());
			ServerInfo_Arrow *arrowInfo = playerInfo.add_arrow_list();
			arrowInfo->set_id(arrow->getId());
			arrowInfo->set_start_player_id(arrow->getStartCard()->getZone()->getPlayer()->getPlayerId());
			arrowInfo->set_start_zone(arrow->getStartCard()->getZone()->getName().toStdString());
			arrowInfo->set_start_card_id(arrow->getStartCard()->getId());
			arrowInfo->mutable_arrow_color()->CopyFrom(arrow->getColor());
			if (targetCard) {
				arrowInfo->set_target_player_id(targetCard->getZone()->getPlayer()->getPlayerId());
				arrowInfo->set_target_zone(targetCard->getZone()->getName().toStdString());
				arrowInfo->set_target_card_id(targetCard->getId());
			} else
				arrowInfo->set_target_player_id(qobject_cast<Server_Player *>(arrow->getTargetItem())->getPlayerId());
		}
		
		QMapIterator<int, Server_Counter *> counterIterator(player->getCounters());
		while (counterIterator.hasNext()) {
			Server_Counter *counter = counterIterator.next().value();
			ServerInfo_Counter *counterInfo = playerInfo.add_counter_list();
			counterInfo->set_id(counter->getId());
			counterInfo->set_name(counter->getName().toStdString());
			counterInfo->mutable_counter_color()->CopyFrom(counter->getColor());
			counterInfo->set_radius(counter->getRadius());
			counterInfo->set_count(counter->getCount());
		}

		QList<ServerInfo_Zone *> zoneList;
		QMapIterator<QString, Server_CardZone *> zoneIterator(player->getZones());
		while (zoneIterator.hasNext()) {
			Server_CardZone *zone = zoneIterator.next().value();
			ServerInfo_Zone *zoneInfo = playerInfo.add_zone_list();
			zoneInfo->set_name(zone->getName().toStdString());
			zoneInfo->set_type(zone->getType());
			zoneInfo->set_with_coords(zone->hasCoords());
			zoneInfo->set_card_count(zone->cards.size());
			if (
				(((playerWhosAsking == player) || omniscient) && (zone->getType() != ServerInfo_Zone::HiddenZone))
				|| ((playerWhosAsking != player) && (zone->getType() == ServerInfo_Zone::PublicZone))
			) {
				QListIterator<Server_Card *> cardIterator(zone->cards);
				while (cardIterator.hasNext()) {
					Server_Card *card = cardIterator.next();
					ServerInfo_Card *cardInfo = zoneInfo->add_card_list();
					QString displayedName = card->getFaceDown() ? QString() : card->getName();
					
					cardInfo->set_id(card->getId());
					cardInfo->set_name(displayedName.toStdString());
					cardInfo->set_x(card->getX());
					cardInfo->set_y(card->getY());
					if (card->getFaceDown())
						cardInfo->set_face_down(true);
					cardInfo->set_tapped(card->getTapped());
					if (card->getAttacking())
						cardInfo->set_attacking(true);
					if (!card->getColor().isEmpty())
						cardInfo->set_color(card->getColor().toStdString());
					if (!card->getPT().isEmpty())
						cardInfo->set_pt(card->getPT().toStdString());
					if (!card->getAnnotation().isEmpty())
						cardInfo->set_annotation(card->getAnnotation().toStdString());
					if (card->getDestroyOnZoneChange())
						cardInfo->set_destroy_on_zone_change(true);
					if (card->getDoesntUntap())
						cardInfo->set_doesnt_untap(true);
					
					QList<ServerInfo_CardCounter *> cardCounterList;
					QMapIterator<int, int> cardCounterIterator(card->getCounters());
					while (cardCounterIterator.hasNext()) {
						cardCounterIterator.next();
						ServerInfo_CardCounter *counterInfo = cardInfo->add_counter_list();
						counterInfo->set_id(cardCounterIterator.key());
						counterInfo->set_value(cardCounterIterator.value());
					}
		
					if (card->getParentCard()) {
						cardInfo->set_attach_player_id(card->getParentCard()->getZone()->getPlayer()->getPlayerId());
						cardInfo->set_attach_zone(card->getParentCard()->getZone()->getName().toStdString());
						cardInfo->set_attach_card_id(card->getParentCard()->getId());
					}
				}
			}
		}
		
		result.append(playerInfo);
	}
	return result;
}

void Server_Game::sendGameEventContainer(GameEventContainer *cont, GameEventStorageItem::EventRecipients recipients, int privatePlayerId)
{
	QMutexLocker locker(&gameMutex);
	
	cont->set_game_id(gameId);
	QMapIterator<int, Server_Player *> playerIterator(players);
	while (playerIterator.hasNext()) {
		Server_Player *p = playerIterator.next().value();
		const bool playerPrivate = (p->getPlayerId() == privatePlayerId) || (p->getSpectator() && spectatorsSeeEverything);
		if ((recipients.testFlag(GameEventStorageItem::SendToPrivate) && playerPrivate) || (recipients.testFlag(GameEventStorageItem::SendToOthers) && !playerPrivate))
			p->sendGameEvent(*cont);
	}
	if (recipients.testFlag(GameEventStorageItem::SendToPrivate)) {
		cont->set_seconds_elapsed(secondsElapsed - startTimeOfThisGame);
		cont->clear_game_id();
		currentReplay->add_event_list()->CopyFrom(*cont);
	}
	
	delete cont;
}

GameEventContainer *Server_Game::prepareGameEvent(const ::google::protobuf::Message &gameEvent, int playerId, GameEventContext *context)
{
	GameEventContainer *cont = new GameEventContainer;
	cont->set_game_id(gameId);
	if (context)
		cont->mutable_context()->CopyFrom(*context);
	GameEvent *event = cont->add_event_list();
	if (playerId != -1)
		event->set_player_id(playerId);
	event->GetReflection()->MutableMessage(event, gameEvent.GetDescriptor()->FindExtensionByName("ext"))->CopyFrom(gameEvent);
	return cont;
}

ServerInfo_Game Server_Game::getInfo() const
{
	QMutexLocker locker(&gameMutex);
	
	ServerInfo_Game result;
	result.set_room_id(room->getId());
	result.set_game_id(getGameId());
	if (!players.isEmpty()) {
		for (int i = 0; i < gameTypes.size(); ++i)
			result.add_game_types(gameTypes[i]);
		
		result.set_max_players(getMaxPlayers());
		result.set_description(getDescription().toStdString());
		result.set_with_password(!getPassword().isEmpty());
		result.set_player_count(getPlayerCount());
		result.set_started(gameStarted);
		result.mutable_creator_info()->CopyFrom(*getCreatorInfo());
		result.set_only_buddies(onlyBuddies);
		result.set_only_registered(onlyRegistered);
		result.set_spectators_allowed(getSpectatorsAllowed());
		result.set_spectators_need_password(getSpectatorsNeedPassword());
		result.set_spectators_count(getSpectatorCount());
		result.set_start_time(startTime.toTime_t());
	}
	return result;
}
