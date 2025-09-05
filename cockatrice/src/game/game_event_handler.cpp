#include "game_event_handler.h"

#include "../client/tabs/tab_game.h"
#include "../server/abstract_client.h"
#include "../server/message_log_widget.h"
#include "../server/pending_command.h"
#include "get_pb_extension.h"
#include "pb/command_concede.pb.h"
#include "pb/command_delete_arrow.pb.h"
#include "pb/command_game_say.pb.h"
#include "pb/command_leave_game.pb.h"
#include "pb/command_next_turn.pb.h"
#include "pb/command_reverse_turn.pb.h"
#include "pb/command_set_active_phase.pb.h"
#include "pb/context_connection_state_changed.pb.h"
#include "pb/context_deck_select.pb.h"
#include "pb/context_ping_changed.pb.h"
#include "pb/event_game_closed.pb.h"
#include "pb/event_game_host_changed.pb.h"
#include "pb/event_game_say.pb.h"
#include "pb/event_game_state_changed.pb.h"
#include "pb/event_join.pb.h"
#include "pb/event_kicked.pb.h"
#include "pb/event_leave.pb.h"
#include "pb/event_player_properties_changed.pb.h"
#include "pb/event_reverse_turn.pb.h"
#include "pb/event_set_active_phase.pb.h"
#include "pb/event_set_active_player.pb.h"
#include "pb/game_event_container.pb.h"

GameEventHandler::GameEventHandler(TabGame *_game) : game(_game), gameState(_game->getGameState())
{
}

void GameEventHandler::sendGameCommand(PendingCommand *pend, int playerId)
{
    AbstractClient *client = game->getClientForPlayer(playerId);
    if (!client)
        return;

    connect(pend, &PendingCommand::finished, this, &GameEventHandler::commandFinished);
    client->sendCommand(pend);
}

void GameEventHandler::sendGameCommand(const google::protobuf::Message &command, int playerId)
{
    AbstractClient *client = game->getClientForPlayer(playerId);
    if (!client)
        return;

    PendingCommand *pend = prepareGameCommand(command);
    connect(pend, &PendingCommand::finished, this, &GameEventHandler::commandFinished);
    client->sendCommand(pend);
}

void GameEventHandler::commandFinished(const Response &response)
{
    if (response.response_code() == Response::RespChatFlood)
        emit gameFlooded();
    // messageLog->appendMessage(tr("You are flooding the game. Please wait a couple of seconds."));
}

PendingCommand *GameEventHandler::prepareGameCommand(const ::google::protobuf::Message &cmd)
{
    CommandContainer cont;
    cont.set_game_id(static_cast<google::protobuf::uint32>(game->getGameMetaInfo()->gameId()));
    GameCommand *c = cont.add_game_command();
    c->GetReflection()->MutableMessage(c, cmd.GetDescriptor()->FindExtensionByName("ext"))->CopyFrom(cmd);
    return new PendingCommand(cont);
}

PendingCommand *GameEventHandler::prepareGameCommand(const QList<const ::google::protobuf::Message *> &cmdList)
{
    CommandContainer cont;
    cont.set_game_id(static_cast<google::protobuf::uint32>(game->getGameMetaInfo()->gameId()));
    for (auto i : cmdList) {
        GameCommand *c = cont.add_game_command();
        c->GetReflection()->MutableMessage(c, i->GetDescriptor()->FindExtensionByName("ext"))->CopyFrom(*i);
        delete i;
    }
    return new PendingCommand(cont);
}

void GameEventHandler::processGameEventContainer(const GameEventContainer &cont,
                                                 AbstractClient *client,
                                                 Player::EventProcessingOptions options)
{
    const GameEventContext &context = cont.context();
    emit containerProcessingStarted(context);
    // messageLog->containerProcessingStarted(context);
    const int eventListSize = cont.event_list_size();
    for (int i = 0; i < eventListSize; ++i) {
        const GameEvent &event = cont.event_list(i);
        const int playerId = event.player_id();
        const auto eventType = static_cast<GameEvent::GameEventType>(getPbExtension(event));

        if (cont.has_forced_by_judge()) {
            auto id = cont.forced_by_judge();
            Player *judgep = gameState->getPlayers().value(id, nullptr);
            if (judgep) {
                emit setContextJudgeName();
                // messageLog->setContextJudgeName(judgep->getName());
            } else if (gameState->getSpectators().contains(id)) {
                emit setContextJudgeName();
                // messageLog->setContextJudgeName(QString::fromStdString(gameState->getSpectators().value(id).name()));
            }
        }

        if (gameState->getSpectators().contains(playerId)) {
            switch (eventType) {
                case GameEvent::GAME_SAY:
                    eventSpectatorSay(event.GetExtension(Event_GameSay::ext), playerId, context);
                    break;
                case GameEvent::LEAVE:
                    eventSpectatorLeave(event.GetExtension(Event_Leave::ext), playerId, context);
                    break;
                default:
                    break;
            }
        } else {
            if ((gameState->getClients().size() > 1) && (playerId != -1))
                if (gameState->getClients().at(playerId) != client)
                    continue;

            switch (eventType) {
                case GameEvent::GAME_STATE_CHANGED:
                    eventGameStateChanged(event.GetExtension(Event_GameStateChanged::ext), playerId, context);
                    break;
                case GameEvent::PLAYER_PROPERTIES_CHANGED:
                    eventPlayerPropertiesChanged(event.GetExtension(Event_PlayerPropertiesChanged::ext), playerId,
                                                 context);
                    break;
                case GameEvent::JOIN:
                    eventJoin(event.GetExtension(Event_Join::ext), playerId, context);
                    break;
                case GameEvent::LEAVE:
                    eventLeave(event.GetExtension(Event_Leave::ext), playerId, context);
                    break;
                case GameEvent::KICKED:
                    eventKicked(event.GetExtension(Event_Kicked::ext), playerId, context);
                    break;
                case GameEvent::GAME_HOST_CHANGED:
                    eventGameHostChanged(event.GetExtension(Event_GameHostChanged::ext), playerId, context);
                    break;
                case GameEvent::GAME_CLOSED:
                    eventGameClosed(event.GetExtension(Event_GameClosed::ext), playerId, context);
                    break;
                case GameEvent::SET_ACTIVE_PLAYER:
                    eventSetActivePlayer(event.GetExtension(Event_SetActivePlayer::ext), playerId, context);
                    break;
                case GameEvent::SET_ACTIVE_PHASE:
                    eventSetActivePhase(event.GetExtension(Event_SetActivePhase::ext), playerId, context);
                    break;
                case GameEvent::REVERSE_TURN:
                    eventReverseTurn(event.GetExtension(Event_ReverseTurn::ext), playerId, context);
                    break;

                default: {
                    Player *player = gameState->getPlayers().value(playerId, 0);
                    if (!player) {
                        // qCWarning(GameEventHandlerLog) << "unhandled game event: invalid player id";
                        break;
                    }
                    player->processGameEvent(eventType, event, context, options);
                    game->emitUserEvent();
                }
            }
        }
    }
    emit containerProcessingDone();
    // messageLog->containerProcessingDone();
}

void GameEventHandler::handleNextTurn()
{
    sendGameCommand(Command_NextTurn());
}

void GameEventHandler::handleReverseTurn()
{
    sendGameCommand(Command_ReverseTurn());
}

void GameEventHandler::handlePlayerConceded()
{
    sendGameCommand(Command_Concede());
}

void GameEventHandler::handlePlayerUnconceded()
{
    sendGameCommand(Command_Unconcede());
}

void GameEventHandler::handleActivePhaseChanged(int phase)
{
    Command_SetActivePhase cmd;
    cmd.set_phase(static_cast<google::protobuf::uint32>(phase));
    sendGameCommand(cmd);
}

void GameEventHandler::handleGameLeft()
{
    sendGameCommand(Command_LeaveGame());
}

void GameEventHandler::handleChatMessageSent(const QString &chatMessage)
{
    Command_GameSay cmd;
    cmd.set_message(chatMessage.toStdString());
    sendGameCommand(cmd);
}

void GameEventHandler::handleArrowDeletion(int arrowId)
{
    Command_DeleteArrow cmd;
    cmd.set_arrow_id(arrowId);
    sendGameCommand(cmd);
}

void GameEventHandler::eventSpectatorSay(const Event_GameSay &event,
                                         int eventPlayerId,
                                         const GameEventContext & /*context*/)
{
    const ServerInfo_User &userInfo = gameState->getSpectators().value(eventPlayerId);
    emit logSpectatorSay(userInfo, QString::fromStdString(event.message()));
    // messageLog->logSpectatorSay(userInfo, QString::fromStdString(event.message()));
}

void GameEventHandler::eventSpectatorLeave(const Event_Leave &event,
                                           int eventPlayerId,
                                           const GameEventContext & /*context*/)
{
    emit logSpectatorLeave(gameState->getSpectatorName(eventPlayerId), getLeaveReason(event.reason()));

    gameState->removeSpectator(eventPlayerId);

    game->emitUserEvent();
}

void GameEventHandler::eventGameStateChanged(const Event_GameStateChanged &event,
                                             int /*eventPlayerId*/,
                                             const GameEventContext & /*context*/)
{
    const int playerListSize = event.player_list_size();

    QVector<QPair<int, QPair<QString, QString>>> opponentDecksToDisplay;

    for (int i = 0; i < playerListSize; ++i) {
        const ServerInfo_Player &playerInfo = event.player_list(i);
        const ServerInfo_PlayerProperties &prop = playerInfo.properties();
        const int playerId = prop.player_id();
        QString playerName = "@" + QString::fromStdString(prop.user_info().name());
        game->addPlayerToAutoCompleteList(playerName);
        if (prop.spectator()) {
            gameState->addSpectator(playerId, prop);
        } else {
            Player *player = gameState->getPlayers().value(playerId, 0);
            if (!player) {
                player = gameState->addPlayer(playerId, prop.user_info(), game);
            }
            player->processPlayerInfo(playerInfo);
            if (player->getLocal()) {
                emit localPlayerDeckSelected(player, playerId, playerInfo);
            } else {
                if (!game->getGameMetaInfo()->proto().share_decklists_on_load()) {
                    continue;
                }

                opponentDecksToDisplay.append(
                    qMakePair(playerId, qMakePair(playerName, QString::fromStdString(playerInfo.deck_list()))));
            }
        }
    }

    processCardAttachmentsForPlayers(event);

    emit remotePlayersDecksSelected(opponentDecksToDisplay);

    gameState->setGameTime(event.seconds_elapsed());

    if (event.game_started() && !game->getGameMetaInfo()->started()) {
        gameState->setResuming(!gameState->isGameStateKnown());
        game->getGameMetaInfo()->setStarted(event.game_started());
        if (gameState->isGameStateKnown())
            emit logGameStart();
        // messageLog->logGameStart();
        gameState->setActivePlayer(event.active_player_id());
        gameState->setCurrentPhase(event.active_phase());
    } else if (!event.game_started() && game->getGameMetaInfo()->started()) {
        gameState->setCurrentPhase(-1);
        gameState->setActivePlayer(-1);
        game->getGameMetaInfo()->setStarted(false);
        emit gameStopped();
    }
    gameState->setGameStateKnown(true);
    game->emitUserEvent();
}

void GameEventHandler::processCardAttachmentsForPlayers(const Event_GameStateChanged &event)
{
    for (int i = 0; i < event.player_list_size(); ++i) {
        const ServerInfo_Player &playerInfo = event.player_list(i);
        const ServerInfo_PlayerProperties &prop = playerInfo.properties();
        if (!prop.spectator()) {
            Player *player = gameState->getPlayers().value(prop.player_id(), 0);
            if (!player)
                continue;
            player->processCardAttachment(playerInfo);
        }
    }
}

void GameEventHandler::eventPlayerPropertiesChanged(const Event_PlayerPropertiesChanged &event,
                                                    int eventPlayerId,
                                                    const GameEventContext &context)
{
    Player *player = gameState->getPlayers().value(eventPlayerId, 0);
    if (!player)
        return;
    const ServerInfo_PlayerProperties &prop = event.player_properties();
    // playerListWidget->updatePlayerProperties(prop, eventPlayerId);

    const auto contextType = static_cast<GameEventContext::ContextType>(getPbExtension(context));
    switch (contextType) {
        case GameEventContext::READY_START: {
            bool ready = prop.ready_start();
            if (player->getLocal())
                emit localPlayerReadyStateChanged(player->getId(), ready);
            if (ready) {
                emit logReadyStart();
                // messageLog->logReadyStart(player);
            } else {
                emit logNotReadyStart();
                // emit logNotReadyStart(player);
                // messageLog->logNotReadyStart(player);
            }
            break;
        }
        case GameEventContext::CONCEDE: {
            emit playerConceded();
            // messageLog->logConcede(player);
            player->setConceded(true);

            QMapIterator<int, Player *> playerIterator(gameState->getPlayers());
            while (playerIterator.hasNext())
                playerIterator.next().value()->updateZones();

            break;
        }
        case GameEventContext::UNCONCEDE: {
            emit playerUnconceded();
            // messageLog->logUnconcede(player);
            player->setConceded(false);

            QMapIterator<int, Player *> playerIterator(gameState->getPlayers());
            while (playerIterator.hasNext())
                playerIterator.next().value()->updateZones();

            break;
        }
        case GameEventContext::DECK_SELECT: {
            Context_DeckSelect deckSelect = context.GetExtension(Context_DeckSelect::ext);
            emit logDeckSelect();
            /*messageLog->logDeckSelect(player, QString::fromStdString(deckSelect.deck_hash()),
                                      deckSelect.sideboard_size());*/
            if (game->getGameMetaInfo()->proto().share_decklists_on_load() && deckSelect.has_deck_list() &&
                eventPlayerId != gameState->getLocalPlayerId()) {
                emit remotePlayerDeckSelected(QString::fromStdString(deckSelect.deck_list()), eventPlayerId,
                                              player->getName());
            }
            break;
        }
        case GameEventContext::SET_SIDEBOARD_LOCK: {
            if (player->getLocal()) {
                emit localPlayerSideboardLocked(player->getId(), prop.sideboard_locked());
            }
            emit logSideboardLockSet();
            // messageLog->logSetSideboardLock(player, prop.sideboard_locked());
            break;
        }
        case GameEventContext::CONNECTION_STATE_CHANGED: {
            // messageLog->logConnectionStateChanged(player, prop.ping_seconds() != -1);
            emit logConnectionStateChanged();
            break;
        }
        default:;
    }
}

void GameEventHandler::eventJoin(const Event_Join &event, int /*eventPlayerId*/, const GameEventContext & /*context*/)
{
    const ServerInfo_PlayerProperties &playerInfo = event.player_properties();
    const int playerId = playerInfo.player_id();
    QString playerName = QString::fromStdString(playerInfo.user_info().name());
    game->addPlayerToAutoCompleteList(playerName);

    if (gameState->getPlayers().contains(playerId))
        return;

    if (playerInfo.spectator()) {
        gameState->addSpectator(playerId, playerInfo);
        // messageLog->logJoinSpectator(playerName);
        emit logJoinSpectator();
    } else {
        Player *newPlayer = gameState->addPlayer(playerId, playerInfo.user_info(), game);
        emit logJoinPlayer(newPlayer);
    }
    // playerListWidget->addPlayer(playerInfo);
    game->emitUserEvent();
}

QString GameEventHandler::getLeaveReason(Event_Leave::LeaveReason reason)
{
    switch (reason) {
        case Event_Leave::USER_KICKED:
            return tr("kicked by game host or moderator");
            break;
        case Event_Leave::USER_LEFT:
            return tr("player left the game");
            break;
        case Event_Leave::USER_DISCONNECTED:
            return tr("player disconnected from server");
            break;
        case Event_Leave::OTHER:
        default:
            return tr("reason unknown");
            break;
    }
}
void GameEventHandler::eventLeave(const Event_Leave &event, int eventPlayerId, const GameEventContext & /*context*/)
{
    Player *player = gameState->getPlayers().value(eventPlayerId, 0);
    if (!player)
        return;

    emit playerLeft(player);

    emit logLeave(player, getLeaveReason(event.reason()));

    gameState->removePlayer(eventPlayerId);

    player->clear();
    player->deleteLater();

    // Rearrange all remaining zones so that attachment relationship updates take place
    QMapIterator<int, Player *> playerIterator(gameState->getPlayers());
    while (playerIterator.hasNext())
        playerIterator.next().value()->updateZones();

    game->emitUserEvent();
}

void GameEventHandler::eventKicked(const Event_Kicked & /*event*/,
                                   int /*eventPlayerId*/,
                                   const GameEventContext & /*context*/)
{
    emit gameClosed();

    emit logKicked();

    // messageLog->logKicked();

    emit playerKicked();

    game->emitUserEvent();
}

void GameEventHandler::eventReverseTurn(const Event_ReverseTurn &event,
                                        int eventPlayerId,
                                        const GameEventContext & /*context*/)
{
    Player *player = gameState->getPlayers().value(eventPlayerId, 0);
    if (!player)
        return;

    emit logTurnReversed(player, event.reversed());
}

void GameEventHandler::eventGameHostChanged(const Event_GameHostChanged & /*event*/,
                                            int eventPlayerId,
                                            const GameEventContext & /*context*/)
{
    gameState->setHostId(eventPlayerId);
}

void GameEventHandler::eventGameClosed(const Event_GameClosed & /*event*/,
                                       int /*eventPlayerId*/,
                                       const GameEventContext & /*context*/)
{
    game->getGameMetaInfo()->setStarted(false);
    gameState->setGameClosed(true);
    emit gameClosed();
    emit logGameClosed();
    // messageLog->logGameClosed();
    game->emitUserEvent();
}

void GameEventHandler::eventSetActivePlayer(const Event_SetActivePlayer &event,
                                            int /*eventPlayerId*/,
                                            const GameEventContext & /*context*/)
{
    gameState->setActivePlayer(event.active_player_id());
    Player *player = gameState->getPlayer(event.active_player_id());
    if (!player)
        return;
    emit logActivePlayer();
    // messageLog->logSetActivePlayer(player);
    game->emitUserEvent();
}

void GameEventHandler::eventSetActivePhase(const Event_SetActivePhase &event,
                                           int /*eventPlayerId*/,
                                           const GameEventContext & /*context*/)
{
    const int phase = event.phase();
    if (gameState->getCurrentPhase() != phase) {
        emit logActivePhaseChanged();
    }
    // messageLog->logSetActivePhase(phase);
    gameState->setCurrentPhase(phase);
    game->emitUserEvent();
}