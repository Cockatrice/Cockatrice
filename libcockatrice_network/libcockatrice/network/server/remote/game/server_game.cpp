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
#include "server_game.h"

#include "../server.h"
#include "../server_database_interface.h"
#include "../server_protocolhandler.h"
#include "../server_room.h"
#include "server_abstract_player.h"
#include "server_arrow.h"
#include "server_card.h"
#include "server_cardzone.h"
#include "server_player.h"
#include "server_spectator.h"

#include <QDebug>
#include <QTimer>
#include <google/protobuf/descriptor.h>
#include <libcockatrice/deck_list/deck_list.h>
#include <libcockatrice/protocol/pb/context_connection_state_changed.pb.h>
#include <libcockatrice/protocol/pb/context_deck_select.pb.h>
#include <libcockatrice/protocol/pb/context_ping_changed.pb.h>
#include <libcockatrice/protocol/pb/event_delete_arrow.pb.h>
#include <libcockatrice/protocol/pb/event_game_closed.pb.h>
#include <libcockatrice/protocol/pb/event_game_host_changed.pb.h>
#include <libcockatrice/protocol/pb/event_game_joined.pb.h>
#include <libcockatrice/protocol/pb/event_game_state_changed.pb.h>
#include <libcockatrice/protocol/pb/event_join.pb.h>
#include <libcockatrice/protocol/pb/event_kicked.pb.h>
#include <libcockatrice/protocol/pb/event_leave.pb.h>
#include <libcockatrice/protocol/pb/event_player_properties_changed.pb.h>
#include <libcockatrice/protocol/pb/event_replay_added.pb.h>
#include <libcockatrice/protocol/pb/event_set_active_phase.pb.h>
#include <libcockatrice/protocol/pb/event_set_active_player.pb.h>
#include <libcockatrice/protocol/pb/game_replay.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_playerping.pb.h>

Server_Game::Server_Game(const ServerInfo_User &_creatorInfo,
                         int _gameId,
                         const QString &_description,
                         const QString &_password,
                         int _maxPlayers,
                         const QList<int> &_gameTypes,
                         bool _onlyBuddies,
                         bool _onlyRegistered,
                         bool _spectatorsAllowed,
                         bool _spectatorsNeedPassword,
                         bool _spectatorsCanTalk,
                         bool _spectatorsSeeEverything,
                         int _startingLifeTotal,
                         bool _shareDecklistsOnLoad,
                         Server_Room *_room)
    : QObject(), room(_room), nextPlayerId(0), hostId(0), creatorInfo(new ServerInfo_User(_creatorInfo)),
      gameStarted(false), gameClosed(false), gameId(_gameId), password(_password), maxPlayers(_maxPlayers),
      gameTypes(_gameTypes), activePlayer(-1), activePhase(-1), onlyBuddies(_onlyBuddies),
      onlyRegistered(_onlyRegistered), spectatorsAllowed(_spectatorsAllowed),
      spectatorsNeedPassword(_spectatorsNeedPassword), spectatorsCanTalk(_spectatorsCanTalk),
      spectatorsSeeEverything(_spectatorsSeeEverything), startingLifeTotal(_startingLifeTotal),
      shareDecklistsOnLoad(_shareDecklistsOnLoad), inactivityCounter(0), startTimeOfThisGame(0), secondsElapsed(0),
      firstGameStarted(false), turnOrderReversed(false), startTime(QDateTime::currentDateTime()), pingClock(nullptr),
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
      gameMutex()
#else
      gameMutex(QMutex::Recursive)
#endif
{
    currentReplay = new GameReplay;
    currentReplay->set_replay_id(room->getServer()->getDatabaseInterface()->getNextReplayId());
    description = _description.simplified();

    connect(this, &Server_Game::sigStartGameIfReady, this, &Server_Game::doStartGameIfReady, Qt::QueuedConnection);

    getInfo(*currentReplay->mutable_game_info());

    if (room->getServer()->getGameShouldPing()) {
        pingClock = new QTimer(this);
        connect(pingClock, &QTimer::timeout, this, &Server_Game::pingClockTimeout);
        pingClock->start(1000);
    }
}

Server_Game::~Server_Game()
{
    room->gamesLock.lockForWrite();
    gameMutex.lock();

    gameClosed = true;
    sendGameEventContainer(prepareGameEvent(Event_GameClosed(), -1));
    for (auto *participant : participants.values()) {
        participant->prepareDestroy();
    }
    participants.clear();

    room->removeGame(this);
    delete creatorInfo;
    creatorInfo = 0;

    gameMutex.unlock();
    room->gamesLock.unlock();
    currentReplay->set_duration_seconds(secondsElapsed - startTimeOfThisGame);
    replayList.append(currentReplay);
    storeGameInformation();

    for (auto *replay : replayList) {
        delete replay;
    }
    replayList.clear();

    room = nullptr;
    currentReplay = nullptr;
    creatorInfo = nullptr;

    if (pingClock) {
        delete pingClock;
        pingClock = nullptr;
    }

    qDebug() << "Server_Game destructor: gameId=" << gameId;
    deleteLater();
}

void Server_Game::storeGameInformation()
{
    const ServerInfo_Game &gameInfo = replayList.first()->game_info();

    Event_ReplayAdded replayEvent;
    ServerInfo_ReplayMatch *replayMatchInfo = replayEvent.mutable_match_info();
    replayMatchInfo->set_game_id(gameInfo.game_id());
    replayMatchInfo->set_room_name(room->getName().toStdString());
    replayMatchInfo->set_time_started(QDateTime::currentDateTime().addSecs(-secondsElapsed).toSecsSinceEpoch());
    replayMatchInfo->set_length(secondsElapsed);
    replayMatchInfo->set_game_name(gameInfo.description());

    const QStringList &allGameTypes = room->getGameTypes();
    QStringList _gameTypes;
    for (int i = gameInfo.game_types_size() - 1; i >= 0; --i)
        _gameTypes.append(allGameTypes[gameInfo.game_types(i)]);

    for (const auto &playerName : allPlayersEver) {
        replayMatchInfo->add_player_names(playerName.toStdString());
    }

    for (int i = 0; i < replayList.size(); ++i) {
        ServerInfo_Replay *replayInfo = replayMatchInfo->add_replay_list();
        replayInfo->set_replay_id(replayList[i]->replay_id());
        replayInfo->set_replay_name(gameInfo.description());
        replayInfo->set_duration(replayList[i]->duration_seconds());
    }

    SessionEvent *sessionEvent = Server_ProtocolHandler::prepareSessionEvent(replayEvent);
    Server *server = room->getServer();
    server->clientsLock.lockForRead();
    for (auto userName : allPlayersEver + allSpectatorsEver) {
        Server_AbstractUserInterface *userHandler = server->findUser(userName);
        if (userHandler && server->getStoreReplaysEnabled())
            userHandler->sendProtocolItem(*sessionEvent);
    }
    server->clientsLock.unlock();
    delete sessionEvent;

    if (server->getStoreReplaysEnabled()) {
        server->getDatabaseInterface()->storeGameInformation(room->getName(), _gameTypes, gameInfo, allPlayersEver,
                                                             allSpectatorsEver, replayList);
    }
}

void Server_Game::pingClockTimeout()
{
    QMutexLocker locker(&gameMutex);
    ++secondsElapsed;

    GameEventStorage ges;
    ges.setGameEventContext(Context_PingChanged());

    bool allPlayersInactive = true;
    int playerCount = 0;
    for (auto *participant : participants) {
        if (participant == nullptr)
            continue;

        if (!participant->getSpectator()) {
            ++playerCount;
        }

        if (participant->updatePingTime()) {
            Event_PlayerPropertiesChanged event;
            event.mutable_player_properties()->set_ping_seconds(participant->getPingTime());
            ges.enqueueGameEvent(event, participant->getPlayerId());
        }

        if ((participant->getPingTime() != -1) &&
            (!participant->getSpectator() || participant->getPlayerId() == hostId)) {
            allPlayersInactive = false;
        }
    }
    ges.sendToGame(this);

    const int maxTime = room->getServer()->getMaxGameInactivityTime();
    if (allPlayersInactive) {
        if (((maxTime > 0) && (++inactivityCounter >= maxTime)) || (playerCount < maxPlayers)) {
            deleteLater();
        }
    } else {
        inactivityCounter = 0;
    }
}

QMap<int, Server_AbstractPlayer *> Server_Game::getPlayers() const // copies pointers to new map
{
    QMap<int, Server_AbstractPlayer *> players;
    QMutexLocker locker(&gameMutex);
    for (int id : participants.keys()) {
        auto *participant = participants[id];
        if (!participant->getSpectator()) {
            players[id] = static_cast<Server_AbstractPlayer *>(participant);
        }
    }
    return players;
}

Server_AbstractPlayer *Server_Game::getPlayer(int id) const
{
    auto *participant = participants.value(id);
    if (!participant->getSpectator()) {
        return static_cast<Server_AbstractPlayer *>(participant);
    } else {
        return nullptr;
    }
}

int Server_Game::getPlayerCount() const
{
    return participants.size() - getSpectatorCount();
}

int Server_Game::getSpectatorCount() const
{
    QMutexLocker locker(&gameMutex);

    int result = 0;
    for (Server_AbstractParticipant *participant : participants.values()) {
        if (participant->getSpectator())
            ++result;
    }
    return result;
}

void Server_Game::createGameStateChangedEvent(Event_GameStateChanged *event,
                                              Server_AbstractParticipant *recipient,
                                              bool omniscient,
                                              bool withUserInfo)
{
    event->set_seconds_elapsed(secondsElapsed);
    if (gameStarted) {
        event->set_game_started(true);
        event->set_active_player_id(0);
        event->set_active_phase(0);
    } else
        event->set_game_started(false);

    for (Server_AbstractParticipant *participant : participants.values()) {
        participant->getInfo(event->add_player_list(), recipient, omniscient, withUserInfo);
    }
}

void Server_Game::sendGameStateToPlayers()
{
    // game state information for replay and omniscient spectators
    Event_GameStateChanged omniscientEvent;
    createGameStateChangedEvent(&omniscientEvent, nullptr, true, false);

    GameEventContainer *replayCont = prepareGameEvent(omniscientEvent, -1);
    replayCont->set_seconds_elapsed(secondsElapsed - startTimeOfThisGame);
    replayCont->clear_game_id();
    currentReplay->add_event_list()->CopyFrom(*replayCont);
    delete replayCont;

    // If spectators are not omniscient, we need an additional createGameStateChangedEvent call, otherwise we can use
    // the data we used for the replay. All spectators are equal, so we don't need to make a createGameStateChangedEvent
    // call for each one.
    Event_GameStateChanged spectatorNormalEvent;
    createGameStateChangedEvent(&spectatorNormalEvent, nullptr, false, false);

    // send game state info to clients according to their role in the game
    for (auto *participant : participants.values()) {
        GameEventContainer *gec;
        if (participant->getSpectator()) {
            if (spectatorsSeeEverything || participant->getJudge()) {
                gec = prepareGameEvent(omniscientEvent, -1);
            } else {
                gec = prepareGameEvent(spectatorNormalEvent, -1);
            }
        } else {
            Event_GameStateChanged event;
            createGameStateChangedEvent(&event, participant, false, false);

            gec = prepareGameEvent(event, -1);
        }
        participant->sendGameEvent(*gec);
        delete gec;
    }
}

void Server_Game::doStartGameIfReady(bool forceStartGame)
{
    Server_DatabaseInterface *databaseInterface = room->getServer()->getDatabaseInterface();
    QMutexLocker locker(&gameMutex);

    if (getPlayerCount() < maxPlayers && !forceStartGame) {
        return;
    }

    auto players = getPlayers();
    for (auto *player : players.values()) {
        if (!player->getReadyStart()) {
            if (forceStartGame) {
                // Player is not ready to start, so kick them
                // TODO: Move them to Spectators instead
                kickParticipant(player->getPlayerId());
            } else {
                return;
            }
        }
    }

    players = getPlayers(); // players could have been kicked, get new list of players
    for (Server_AbstractPlayer *player : players.values()) {
        player->setupZones();
    }

    gameStarted = true;
    for (auto *player : players.values()) {
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
        createGameStateChangedEvent(&omniscientEvent, nullptr, true, true);

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

void Server_Game::startGameIfReady(bool forceStartGame)
{
    emit sigStartGameIfReady(forceStartGame);
}

void Server_Game::stopGameIfFinished()
{
    QMutexLocker locker(&gameMutex);

    int playing = 0;
    auto players = getPlayers();
    for (auto *player : players.values()) {
        if (!player->getConceded())
            ++playing;
    }
    if (playing > 1)
        return;

    gameStarted = false;

    for (auto *player : players.values()) {
        player->clearZones();
        player->setConceded(false);
    }

    sendGameStateToPlayers();

    locker.unlock();

    ServerInfo_Game gameInfo;
    gameInfo.set_room_id(room->getId());
    gameInfo.set_game_id(gameId);
    gameInfo.set_started(false);
    emit gameInfoChanged(gameInfo);
}

Response::ResponseCode Server_Game::checkJoin(ServerInfo_User *user,
                                              const QString &_password,
                                              bool spectator,
                                              bool overrideRestrictions,
                                              bool asJudge)
{
    Server_DatabaseInterface *databaseInterface = room->getServer()->getDatabaseInterface();
    for (auto *participant : participants.values()) {
        if (participant->getUserInfo()->name() == user->name())
            return Response::RespContextError;
    }

    if (asJudge && !(user->user_level() & ServerInfo_User::IsJudge)) {
        return Response::RespUserLevelTooLow;
    }
    if (!(overrideRestrictions && (user->user_level() & ServerInfo_User::IsModerator))) {
        if ((_password != password) && !(spectator && !spectatorsNeedPassword))
            return Response::RespWrongPassword;
        if (!(user->user_level() & ServerInfo_User::IsRegistered) && onlyRegistered)
            return Response::RespUserLevelTooLow;
        if (onlyBuddies && (user->name() != creatorInfo->name()))
            if (!databaseInterface->isInBuddyList(QString::fromStdString(creatorInfo->name()),
                                                  QString::fromStdString(user->name())))
                return Response::RespOnlyBuddies;
        if (databaseInterface->isInIgnoreList(QString::fromStdString(creatorInfo->name()),
                                              QString::fromStdString(user->name())))
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

    for (auto *participant : participants.values()) {
        if (participant->getUserInfo()->name() == userName.toStdString())
            return true;
    }
    return false;
}

void Server_Game::addPlayer(Server_AbstractUserInterface *userInterface,
                            ResponseContainer &rc,
                            bool spectator,
                            bool judge,
                            bool broadcastUpdate)
{
    QMutexLocker locker(&gameMutex);

    Server_AbstractParticipant *newParticipant;
    if (spectator) {
        newParticipant = new Server_Spectator(this, nextPlayerId++, userInterface->copyUserInfo(true, true, true),
                                              judge, userInterface);
    } else {
        newParticipant = new Server_Player(this, nextPlayerId++, userInterface->copyUserInfo(true, true, true), judge,
                                           userInterface);
    }

    newParticipant->moveToThread(thread());

    Event_Join joinEvent;
    newParticipant->getProperties(*joinEvent.mutable_player_properties(), true);
    sendGameEventContainer(prepareGameEvent(joinEvent, -1));

    const QString playerName = QString::fromStdString(newParticipant->getUserInfo()->name());
    participants.insert(newParticipant->getPlayerId(), newParticipant);
    if (spectator) {
        allSpectatorsEver.insert(playerName);
    } else {
        allPlayersEver.insert(playerName);

        // if the original creator of the game joins, give them host status back
        // FIXME: transferring host to spectators has side effects
        if (newParticipant->getUserInfo()->name() == creatorInfo->name()) {
            hostId = newParticipant->getPlayerId();
            sendGameEventContainer(prepareGameEvent(Event_GameHostChanged(), hostId));
        }
    }

    if (broadcastUpdate) {
        ServerInfo_Game gameInfo;
        gameInfo.set_room_id(room->getId());
        gameInfo.set_game_id(gameId);
        gameInfo.set_player_count(getPlayerCount());
        gameInfo.set_spectators_count(getSpectatorCount());
        emit gameInfoChanged(gameInfo);
    }

    if ((newParticipant->getUserInfo()->user_level() & ServerInfo_User::IsRegistered) && !spectator)
        room->getServer()->addPersistentPlayer(playerName, room->getId(), gameId, newParticipant->getPlayerId());

    userInterface->playerAddedToGame(gameId, room->getId(), newParticipant->getPlayerId());

    createGameJoinedEvent(newParticipant, rc, false);
}

void Server_Game::removeParticipant(Server_AbstractParticipant *participant, Event_Leave::LeaveReason reason)
{
    room->getServer()->removePersistentPlayer(QString::fromStdString(participant->getUserInfo()->name()), room->getId(),
                                              gameId, participant->getPlayerId());
    participants.remove(participant->getPlayerId());

    bool spectator = participant->getSpectator();
    GameEventStorage ges;
    if (!spectator) {
        auto *player = static_cast<Server_AbstractPlayer *>(participant);
        removeArrowsRelatedToPlayer(ges, player);
        unattachCards(ges, player);
    }

    Event_Leave event;
    event.set_reason(reason);
    ges.enqueueGameEvent(event, participant->getPlayerId());
    ges.sendToGame(this);

    bool playerActive = activePlayer == participant->getPlayerId();
    bool playerHost = hostId == participant->getPlayerId();
    participant->prepareDestroy();

    if (playerHost) {
        int newHostId = -1;
        for (auto *otherPlayer : getPlayers().values()) {
            newHostId = otherPlayer->getPlayerId();
            break;
        }
        if (newHostId != -1) {
            hostId = newHostId;
            sendGameEventContainer(prepareGameEvent(Event_GameHostChanged(), hostId));
        } else {
            gameClosed = true;
            deleteLater();
            return;
        }
    }
    if (!spectator) {
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

void Server_Game::removeArrowsRelatedToPlayer(GameEventStorage &ges, Server_AbstractPlayer *player)
{
    QMutexLocker locker(&gameMutex);

    // Remove all arrows of other players pointing to the player being removed or to one of his cards.
    // Also remove all arrows starting at one of his cards. This is necessary since players can create
    // arrows that start at another person's cards.
    for (Server_AbstractPlayer *anyPlayer : getPlayers().values()) {
        QList<Server_Arrow *> toDelete;
        for (auto *arrow : anyPlayer->getArrows().values()) {
            Server_Card *targetCard = qobject_cast<Server_Card *>(arrow->getTargetItem());
            if (targetCard) {
                if (targetCard->getZone() != nullptr && targetCard->getZone()->getPlayer() == player)
                    toDelete.append(arrow);
            } else if (arrow->getTargetItem() == player) {
                toDelete.append(arrow);
            }

            // Don't use else here! It has to happen regardless of whether targetCard == 0.
            if (arrow->getStartCard()->getZone() != nullptr && arrow->getStartCard()->getZone()->getPlayer() == player)
                toDelete.append(arrow);
        }
        for (auto *arrow : toDelete) {
            Event_DeleteArrow event;
            event.set_arrow_id(arrow->getId());
            ges.enqueueGameEvent(event, anyPlayer->getPlayerId());

            anyPlayer->deleteArrow(arrow->getId());
        }
    }
}

void Server_Game::unattachCards(GameEventStorage &ges, Server_AbstractPlayer *player)
{
    QMutexLocker locker(&gameMutex);

    for (auto zone : player->getZones()) {
        for (auto card : zone->getCards()) {
            // Make a copy of the list because the original one gets modified during the loop
            QList<Server_Card *> attachedCards = card->getAttachedCards();
            for (Server_Card *attachedCard : attachedCards) {
                auto otherPlayer = attachedCard->getZone()->getPlayer();
                // do not modify the current player's zone!
                // this would cause the current card iterator to be invalidated!
                // we only have to return cards owned by other players
                // because the current player is leaving the game anyway
                if (otherPlayer != player) {
                    otherPlayer->unattachCard(ges, attachedCard);
                }
            }
        }
    }
}

bool Server_Game::kickParticipant(int playerId)
{
    QMutexLocker locker(&gameMutex);

    auto *participant = participants.value(playerId);
    if (!participant)
        return false;

    GameEventContainer *gec = prepareGameEvent(Event_Kicked(), -1);
    participant->sendGameEvent(*gec);
    delete gec;

    removeParticipant(participant, Event_Leave::USER_KICKED);

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

    for (auto *player : getPlayers().values()) {
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

    if (participants.isEmpty()) {
        qWarning() << "Server_Game::nextTurn was called while players is empty; gameId = " << gameId;
        return;
    }

    auto players = getPlayers();
    const QList<int> keys = players.keys();
    int listPos = -1;
    if (activePlayer != -1) {
        listPos = keys.indexOf(activePlayer);
    }
    do {
        if (turnOrderReversed) {
            --listPos;
            if (listPos < 0) {
                listPos = keys.size() - 1;
            }
        } else {
            ++listPos;
            if (listPos == keys.size()) {
                listPos = 0;
            }
        }
    } while (players.value(keys[listPos])->getConceded());

    setActivePlayer(keys[listPos]);
}

void Server_Game::createGameJoinedEvent(Server_AbstractParticipant *joiningParticipant,
                                        ResponseContainer &rc,
                                        bool resuming)
{
    Event_GameJoined event1;
    getInfo(*event1.mutable_game_info());
    event1.set_host_id(hostId);
    event1.set_player_id(joiningParticipant->getPlayerId());
    event1.set_spectator(joiningParticipant->getSpectator());
    event1.set_judge(joiningParticipant->getJudge());
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

    bool omniscient = joiningParticipant->getSpectator() && (spectatorsSeeEverything || joiningParticipant->getJudge());
    for (auto *participant : participants.values()) {
        participant->getInfo(event2.add_player_list(), joiningParticipant, omniscient, true);
    }

    rc.enqueuePostResponseItem(ServerMessage::GAME_EVENT_CONTAINER, prepareGameEvent(event2, -1));
}

void Server_Game::sendGameEventContainer(GameEventContainer *cont,
                                         GameEventStorageItem::EventRecipients recipients,
                                         int privatePlayerId)
{
    QMutexLocker locker(&gameMutex);

    cont->set_game_id(gameId);
    for (auto *participant : participants.values()) {
        const bool playerPrivate =
            (participant->getPlayerId() == privatePlayerId) ||
            (participant->getSpectator() && (spectatorsSeeEverything || participant->getJudge()));
        if ((recipients.testFlag(GameEventStorageItem::SendToPrivate) && playerPrivate) ||
            (recipients.testFlag(GameEventStorageItem::SendToOthers) && !playerPrivate))
            participant->sendGameEvent(*cont);
    }
    if (recipients.testFlag(GameEventStorageItem::SendToPrivate)) {
        cont->set_seconds_elapsed(secondsElapsed - startTimeOfThisGame);
        cont->clear_game_id();
        currentReplay->add_event_list()->CopyFrom(*cont);
    }

    delete cont;
}

GameEventContainer *
Server_Game::prepareGameEvent(const ::google::protobuf::Message &gameEvent, int playerId, GameEventContext *context)
{
    GameEventContainer *cont = new GameEventContainer;
    cont->set_game_id(gameId);
    if (context)
        cont->mutable_context()->CopyFrom(*context);
    GameEvent *event = cont->add_event_list();
    if (playerId != -1)
        event->set_player_id(playerId);
    event->GetReflection()
        ->MutableMessage(event, gameEvent.GetDescriptor()->FindExtensionByName("ext"))
        ->CopyFrom(gameEvent);
    return cont;
}

void Server_Game::getInfo(ServerInfo_Game &result) const
{
    QMutexLocker locker(&gameMutex);

    result.set_room_id(room->getId());
    result.set_game_id(gameId);
    if (gameClosed) {
        result.set_closed(true);
    } else {
        for (auto type : gameTypes) {
            result.add_game_types(type);
        }

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
        result.set_share_decklists_on_load(shareDecklistsOnLoad);
        result.set_spectators_count(getSpectatorCount());
        result.set_start_time(startTime.toSecsSinceEpoch());
    }
}
