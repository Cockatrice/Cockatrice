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
#include "server_player.h"
#include "server_protocolhandler.h"
#include "server_arrow.h"
#include "server_card.h"
#include "server_cardzone.h"
#include "server_database_interface.h"
#include "decklist.h"
#include "pb/context_connection_state_changed.pb.h"
#include "pb/context_ping_changed.pb.h"
#include "pb/event_player_properties_changed.pb.h"
#include "pb/event_game_closed.pb.h"
#include "pb/event_game_joined.pb.h"
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
#include "pb/event_replay_added.pb.h"
#include <google/protobuf/descriptor.h>
#include <QTimer>
#include <QDebug>

Server_Game::Server_Game(const ServerInfo_User &_creatorInfo, int _gameId, const QString &_description, const QString &_password, int _maxPlayers, const QList<int> &_gameTypes, bool _onlyBuddies, bool _onlyRegistered, bool _spectatorsAllowed, bool _spectatorsNeedPassword, bool _spectatorsCanTalk, bool _spectatorsSeeEverything, Server_Room *_room)
    : QObject(),
          room(_room),
          nextPlayerId(0),
          hostId(0),
          creatorInfo(new ServerInfo_User(_creatorInfo)),
          gameStarted(false),
          gameClosed(false),
          gameId(_gameId),
          password(_password),
          maxPlayers(_maxPlayers),
          gameTypes(_gameTypes),
          activePlayer(-1),
          activePhase(-1),
          onlyBuddies(_onlyBuddies),
          onlyRegistered(_onlyRegistered),
          spectatorsAllowed(_spectatorsAllowed),
          spectatorsNeedPassword(_spectatorsNeedPassword),
          spectatorsCanTalk(_spectatorsCanTalk),
          spectatorsSeeEverything(_spectatorsSeeEverything),
          inactivityCounter(0),
          startTimeOfThisGame(0),
          secondsElapsed(0),
          firstGameStarted(false),
          startTime(QDateTime::currentDateTime()),
          gameMutex(QMutex::Recursive)
{
    currentReplay = new GameReplay;
    currentReplay->set_replay_id(room->getServer()->getDatabaseInterface()->getNextReplayId());
    description = _description.simplified();

    connect(this, SIGNAL(sigStartGameIfReady()), this, SLOT(doStartGameIfReady()), Qt::QueuedConnection);

    getInfo(*currentReplay->mutable_game_info());

    if (room->getServer()->getGameShouldPing()) {
        pingClock = new QTimer(this);
        connect(pingClock, SIGNAL(timeout()), this, SLOT(pingClockTimeout()));
        pingClock->start(1000);
    }
}

Server_Game::~Server_Game()
{
    room->gamesLock.lockForWrite();
    gameMutex.lock();

    gameClosed = true;
    sendGameEventContainer(prepareGameEvent(Event_GameClosed(), -1));
    QMapIterator<int, Server_Player *> playerIterator(players);
    while (playerIterator.hasNext())
        playerIterator.next().value()->prepareDestroy();
    players.clear();

    room->removeGame(this);
    delete creatorInfo;
    creatorInfo = 0;

    gameMutex.unlock();
    room->gamesLock.unlock();
    currentReplay->set_duration_seconds(secondsElapsed - startTimeOfThisGame);
    replayList.append(currentReplay);
    storeGameInformation();

    for (int i = 0; i < replayList.size(); ++i)
        delete replayList[i];

    qDebug() << "Server_Game destructor: gameId=" << gameId;
}

void Server_Game::storeGameInformation()
{
    const ServerInfo_Game &gameInfo = replayList.first()->game_info();

    Event_ReplayAdded replayEvent;
    ServerInfo_ReplayMatch *replayMatchInfo = replayEvent.mutable_match_info();
    replayMatchInfo->set_game_id(gameInfo.game_id());
    replayMatchInfo->set_room_name(room->getName().toStdString());
    replayMatchInfo->set_time_started(QDateTime::currentDateTime().addSecs(-secondsElapsed).toTime_t());
    replayMatchInfo->set_length(secondsElapsed);
    replayMatchInfo->set_game_name(gameInfo.description());

    const QStringList &allGameTypes = room->getGameTypes();
    QStringList gameTypes;
    for (int i = gameInfo.game_types_size() - 1; i >= 0; --i)
        gameTypes.append(allGameTypes[gameInfo.game_types(i)]);

    QSetIterator<QString> playerIterator(allPlayersEver);
    while (playerIterator.hasNext())
        replayMatchInfo->add_player_names(playerIterator.next().toStdString());

    for (int i = 0; i < replayList.size(); ++i) {
        ServerInfo_Replay *replayInfo = replayMatchInfo->add_replay_list();
        replayInfo->set_replay_id(replayList[i]->replay_id());
        replayInfo->set_replay_name(gameInfo.description());
        replayInfo->set_duration(replayList[i]->duration_seconds());
    }

    QSet<QString> allUsersInGame = allPlayersEver + allSpectatorsEver;
    QSetIterator<QString> allUsersIterator(allUsersInGame);

    SessionEvent *sessionEvent = Server_ProtocolHandler::prepareSessionEvent(replayEvent);
    Server *server = room->getServer();
    server->clientsLock.lockForRead();
    while (allUsersIterator.hasNext()) {
        Server_AbstractUserInterface *userHandler = server->findUser(allUsersIterator.next());
        if (userHandler && server->getStoreReplaysEnabled())
                userHandler->sendProtocolItem(*sessionEvent);
    }
    server->clientsLock.unlock();
    delete sessionEvent;

    if (server->getStoreReplaysEnabled())
        server->getDatabaseInterface()->storeGameInformation(room->getName(), gameTypes, gameInfo, allPlayersEver, allSpectatorsEver, replayList);

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
        if (player->getUserInterface())
            newPingTime = player->getUserInterface()->getLastCommandTime();
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

void Server_Game::createGameStateChangedEvent(Event_GameStateChanged *event, Server_Player *playerWhosAsking, bool omniscient, bool withUserInfo)
{
    event->set_seconds_elapsed(secondsElapsed);
    if (gameStarted) {
        event->set_game_started(true);
        event->set_active_player_id(0);
        event->set_active_phase(0);
    } else
        event->set_game_started(false);

    QMapIterator<int, Server_Player *> playerIterator(players);
    while (playerIterator.hasNext())
        playerIterator.next().value()->getInfo(event->add_player_list(), playerWhosAsking, omniscient, withUserInfo);
}

void Server_Game::sendGameStateToPlayers()
{
    // game state information for replay and omniscient spectators
    Event_GameStateChanged omniscientEvent;
    createGameStateChangedEvent(&omniscientEvent, 0, true, false);

    GameEventContainer *replayCont = prepareGameEvent(omniscientEvent, -1);
    replayCont->set_seconds_elapsed(secondsElapsed - startTimeOfThisGame);
    replayCont->clear_game_id();
    currentReplay->add_event_list()->CopyFrom(*replayCont);
    delete replayCont;

    // If spectators are not omniscient, we need an additional createGameStateChangedEvent call, otherwise we can use the data we used for the replay.
    // All spectators are equal, so we don't need to make a createGameStateChangedEvent call for each one.
    Event_GameStateChanged spectatorEvent;
    if (spectatorsSeeEverything)
        spectatorEvent = omniscientEvent;
    else
        createGameStateChangedEvent(&spectatorEvent, 0, false, false);

    // send game state info to clients according to their role in the game
    QMapIterator<int, Server_Player *> playerIterator(players);
    while (playerIterator.hasNext()) {
        Server_Player *player = playerIterator.next().value();
        GameEventContainer *gec;
        if (player->getSpectator())
            gec = prepareGameEvent(spectatorEvent, -1);
        else {
            Event_GameStateChanged event;
            createGameStateChangedEvent(&event, player, false, false);

            gec = prepareGameEvent(event, -1);
        }
        player->sendGameEvent(*gec);
        delete gec;
    }
}

void Server_Game::doStartGameIfReady()
{
    Server_DatabaseInterface *databaseInterface = room->getServer()->getDatabaseInterface();
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
        currentReplay->set_replay_id(databaseInterface->getNextReplayId());
        ServerInfo_Game *gameInfo = currentReplay->mutable_game_info();
        getInfo(*gameInfo);
        gameInfo->set_started(false);

        Event_GameStateChanged omniscientEvent;
        createGameStateChangedEvent(&omniscientEvent, 0, true, true);

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

    ServerInfo_Game gameInfo;
    gameInfo.set_room_id(room->getId());
    gameInfo.set_game_id(gameId);
    gameInfo.set_started(true);
    emit gameInfoChanged(gameInfo);
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

    locker.unlock();

    ServerInfo_Game gameInfo;
    gameInfo.set_room_id(room->getId());
    gameInfo.set_game_id(gameId);
    gameInfo.set_started(false);
    emit gameInfoChanged(gameInfo);
}

Response::ResponseCode Server_Game::checkJoin(ServerInfo_User *user, const QString &_password, bool spectator, bool overrideRestrictions)
{
    Server_DatabaseInterface *databaseInterface = room->getServer()->getDatabaseInterface();
    {
        QMapIterator<int, Server_Player *> playerIterator(players);
        while (playerIterator.hasNext())
            if (playerIterator.next().value()->getUserInfo()->name() == user->name())
                return Response::RespContextError;
    }
    if (!(overrideRestrictions && (user->user_level() & ServerInfo_User::IsModerator))) {
        if ((_password != password) && !(spectator && !spectatorsNeedPassword))
            return Response::RespWrongPassword;
        if (!(user->user_level() & ServerInfo_User::IsRegistered) && onlyRegistered)
            return Response::RespUserLevelTooLow;
        if (onlyBuddies && (user->name() != creatorInfo->name()))
            if (!databaseInterface->isInBuddyList(QString::fromStdString(creatorInfo->name()), QString::fromStdString(user->name())))
                return Response::RespOnlyBuddies;
        if (databaseInterface->isInIgnoreList(QString::fromStdString(creatorInfo->name()), QString::fromStdString(user->name())))
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

void Server_Game::addPlayer(Server_AbstractUserInterface *userInterface, ResponseContainer &rc, bool spectator, bool broadcastUpdate)
{
    QMutexLocker locker(&gameMutex);

    Server_Player *newPlayer = new Server_Player(this, nextPlayerId++, userInterface->copyUserInfo(true, true, true), spectator, userInterface);
    newPlayer->moveToThread(thread());

    Event_Join joinEvent;
    newPlayer->getProperties(*joinEvent.mutable_player_properties(), true);
    sendGameEventContainer(prepareGameEvent(joinEvent, -1));

    const QString playerName = QString::fromStdString(newPlayer->getUserInfo()->name());
    if (spectator)
        allSpectatorsEver.insert(playerName);
    else
        allPlayersEver.insert(playerName);
    players.insert(newPlayer->getPlayerId(), newPlayer);
    if (newPlayer->getUserInfo()->name() == creatorInfo->name()) {
        hostId = newPlayer->getPlayerId();
        sendGameEventContainer(prepareGameEvent(Event_GameHostChanged(), hostId));
    }

    if (broadcastUpdate) {
        ServerInfo_Game gameInfo;
        gameInfo.set_room_id(room->getId());
        gameInfo.set_game_id(gameId);
        gameInfo.set_player_count(getPlayerCount());
        gameInfo.set_spectators_count(getSpectatorCount());
        emit gameInfoChanged(gameInfo);
    }

    if ((newPlayer->getUserInfo()->user_level() & ServerInfo_User::IsRegistered) && !spectator)
        room->getServer()->addPersistentPlayer(playerName, room->getId(), gameId, newPlayer->getPlayerId());

    userInterface->playerAddedToGame(gameId, room->getId(), newPlayer->getPlayerId());

    createGameJoinedEvent(newPlayer, rc, false);
}

void Server_Game::removePlayer(Server_Player *player)
{
    room->getServer()->removePersistentPlayer(QString::fromStdString(player->getUserInfo()->name()), room->getId(), gameId, player->getPlayerId());
    players.remove(player->getPlayerId());

    GameEventStorage ges;
    removeArrowsRelatedToPlayer(ges, player);
    unattachCards(ges, player);
    ges.enqueueGameEvent(Event_Leave(), player->getPlayerId());
    ges.sendToGame(this);

    bool playerActive = activePlayer == player->getPlayerId();
    bool playerHost = hostId == player->getPlayerId();
    bool spectator = player->getSpectator();
    player->prepareDestroy();

    if (!getPlayerCount()) {
        gameClosed = true;
        deleteLater();
        return;
    } else if (!spectator) {
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

    ServerInfo_Game gameInfo;
    gameInfo.set_room_id(room->getId());
    gameInfo.set_game_id(gameId);
    gameInfo.set_player_count(getPlayerCount());
    gameInfo.set_spectators_count(getSpectatorCount());
    emit gameInfoChanged(gameInfo);
}

void Server_Game::removeArrowsRelatedToPlayer(GameEventStorage &ges, Server_Player *player)
{
    QMutexLocker locker(&gameMutex);

    // Remove all arrows of other players pointing to the player being removed or to one of his cards.
    // Also remove all arrows starting at one of his cards. This is necessary since players can create
    // arrows that start at another person's cards.
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
            } else if (static_cast<Server_Player *>(a->getTargetItem()) == player)
                toDelete.append(a);

            // Don't use else here! It has to happen regardless of whether targetCard == 0.
            if (a->getStartCard()->getZone()->getPlayer() == player)
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
        for (int i = 0; i < zone->getCards().size(); ++i) {
            Server_Card *card = zone->getCards().at(i);

            // Make a copy of the list because the original one gets modified during the loop
            QList<Server_Card *> attachedCards = card->getAttachedCards();
            for (int i = 0; i < attachedCards.size(); ++i)
                attachedCards[i]->getZone()->getPlayer()->unattachCard(ges, attachedCards[i]);
        }
    }
}

bool Server_Game::kickPlayer(int playerId)
{
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

void Server_Game::createGameJoinedEvent(Server_Player *player, ResponseContainer &rc, bool resuming)
{
    Event_GameJoined event1;
    getInfo(*event1.mutable_game_info());
    event1.set_host_id(hostId);
    event1.set_player_id(player->getPlayerId());
    event1.set_spectator(player->getSpectator());
    event1.set_resuming(resuming);
    if (resuming) {
        const QStringList &allGameTypes = room->getGameTypes();
        for (int i = 0; i < allGameTypes.size(); ++i) {
            ServerInfo_GameType *newGameType = event1.add_game_types();
            newGameType->set_game_type_id(i);
            newGameType->set_description(allGameTypes[i].toStdString());
        }
    }
    rc.enqueuePostResponseItem(ServerMessage::SESSION_EVENT, Server_AbstractUserInterface::prepareSessionEvent(event1));

    Event_GameStateChanged event2;
    event2.set_seconds_elapsed(secondsElapsed);
    event2.set_game_started(gameStarted);
    event2.set_active_player_id(activePlayer);
    event2.set_active_phase(activePhase);

    QMapIterator<int, Server_Player *> playerIterator(players);
    while (playerIterator.hasNext())
        playerIterator.next().value()->getInfo(event2.add_player_list(), player, player->getSpectator() && spectatorsSeeEverything, true);

    rc.enqueuePostResponseItem(ServerMessage::GAME_EVENT_CONTAINER, prepareGameEvent(event2, -1));
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

void Server_Game::getInfo(ServerInfo_Game &result) const
{
    QMutexLocker locker(&gameMutex);

    result.set_room_id(room->getId());
    result.set_game_id(gameId);
    if (gameClosed)
        result.set_closed(true);
    else {
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
        result.set_spectators_can_chat(spectatorsCanTalk);
        result.set_spectators_omniscient(spectatorsSeeEverything);
        result.set_spectators_count(getSpectatorCount());
        result.set_start_time(startTime.toTime_t());
    }
}
