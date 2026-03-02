#include "game_event_handler.h"

#include "../interface/widgets/tabs/tab_game.h"
#include "abstract_game.h"
#include "log/message_log_widget.h"

#include <libcockatrice/network/client/abstract/abstract_client.h>
#include <libcockatrice/protocol/get_pb_extension.h>
#include <libcockatrice/protocol/pb/command_concede.pb.h>
#include <libcockatrice/protocol/pb/command_delete_arrow.pb.h>
#include <libcockatrice/protocol/pb/command_game_say.pb.h>
#include <libcockatrice/protocol/pb/command_leave_game.pb.h>
#include <libcockatrice/protocol/pb/command_next_turn.pb.h>
#include <libcockatrice/protocol/pb/command_reverse_turn.pb.h>
#include <libcockatrice/protocol/pb/command_set_active_phase.pb.h>
#include <libcockatrice/protocol/pb/context_connection_state_changed.pb.h>
#include <libcockatrice/protocol/pb/context_deck_select.pb.h>
#include <libcockatrice/protocol/pb/event_game_closed.pb.h>
#include <libcockatrice/protocol/pb/event_game_host_changed.pb.h>
#include <libcockatrice/protocol/pb/event_game_say.pb.h>
#include <libcockatrice/protocol/pb/event_game_state_changed.pb.h>
#include <libcockatrice/protocol/pb/event_join.pb.h>
#include <libcockatrice/protocol/pb/event_kicked.pb.h>
#include <libcockatrice/protocol/pb/event_leave.pb.h>
#include <libcockatrice/protocol/pb/event_player_properties_changed.pb.h>
#include <libcockatrice/protocol/pb/event_reverse_turn.pb.h>
#include <libcockatrice/protocol/pb/event_set_active_phase.pb.h>
#include <libcockatrice/protocol/pb/event_set_active_player.pb.h>
#include <libcockatrice/protocol/pb/game_event_container.pb.h>
#include <libcockatrice/protocol/pending_command.h>

GameEventHandler::GameEventHandler(AbstractGame *_game) : QObject(_game), game(_game)
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
                                                 EventProcessingOptions options)
{
    const GameEventContext &context = cont.context();
    emit containerProcessingStarted(context);

    const int eventListSize = cont.event_list_size();
    for (int i = 0; i < eventListSize; ++i) {
        const GameEvent &event = cont.event_list(i);
        const int playerId = event.player_id();
        const auto eventType = static_cast<GameEvent::GameEventType>(getPbExtension(event));

        if (cont.has_forced_by_judge()) {
            auto id = cont.forced_by_judge();
            Player *judgep = game->getPlayerManager()->getPlayers().value(id, nullptr);
            if (judgep) {
                emit setContextJudgeName(judgep->getPlayerInfo()->getName());
            } else if (game->getPlayerManager()->getSpectators().contains(id)) {
                emit setContextJudgeName(
                    QString::fromStdString(game->getPlayerManager()->getSpectators().value(id).name()));
            }
        }

        if (game->getPlayerManager()->getSpectators().contains(playerId)) {
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
            if ((game->getGameState()->getClients().size() > 1) && (playerId != -1))
                if (game->getGameState()->getClients().at(playerId) != client)
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
                    Player *player = game->getPlayerManager()->getPlayers().value(playerId, 0);
                    if (!player) {
                        qCWarning(GameEventHandlerLog) << "unhandled game event: invalid player id";
                        break;
                    }
                    player->getPlayerEventHandler()->processGameEvent(eventType, event, context, options);
                    emitUserEvent();
                }
            }
        }
    }
    emit containerProcessingDone();
}

void GameEventHandler::handleNextTurn()
{
    sendGameCommand(Command_NextTurn());
}

void GameEventHandler::handleReverseTurn()
{
    sendGameCommand(Command_ReverseTurn());
}

void GameEventHandler::handleActiveLocalPlayerConceded()
{
    sendGameCommand(Command_Concede());
}

void GameEventHandler::handleActiveLocalPlayerUnconceded()
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
    const ServerInfo_User &userInfo = game->getPlayerManager()->getSpectators().value(eventPlayerId);
    emit logSpectatorSay(userInfo, QString::fromStdString(event.message()));
}

void GameEventHandler::eventSpectatorLeave(const Event_Leave &event,
                                           int eventPlayerId,
                                           const GameEventContext & /*context*/)
{
    emit logSpectatorLeave(game->getPlayerManager()->getSpectatorName(eventPlayerId), getLeaveReason(event.reason()));

    emit spectatorLeft(eventPlayerId);

    game->getPlayerManager()->removeSpectator(eventPlayerId);

    emitUserEvent();
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
        QString playerName = QString::fromStdString(prop.user_info().name());
        emit addPlayerToAutoCompleteList("@" + playerName);
        if (prop.spectator()) {
            if (!game->getPlayerManager()->getSpectators().contains(playerId)) {
                game->getPlayerManager()->addSpectator(playerId, prop);
                emit spectatorJoined(prop);
            }
        } else {
            Player *player = game->getPlayerManager()->getPlayers().value(playerId, 0);
            if (!player) {
                player = game->getPlayerManager()->addPlayer(playerId, prop.user_info());
                emit playerJoined(prop);
            }
            player->processPlayerInfo(playerInfo);
            if (player->getPlayerInfo()->getLocal()) {
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

    game->getGameState()->setGameTime(event.seconds_elapsed());

    if (event.game_started() && !game->getGameMetaInfo()->started()) {
        game->getGameState()->setResuming(!game->getGameState()->isGameStateKnown());
        game->getGameMetaInfo()->setStarted(event.game_started());
        if (game->getGameState()->isGameStateKnown())
            emit logGameStart();
        game->getGameState()->setActivePlayer(event.active_player_id());
        game->getGameState()->setCurrentPhase(event.active_phase());
    } else if (!event.game_started() && game->getGameMetaInfo()->started()) {
        game->getGameState()->setCurrentPhase(-1);
        game->getGameState()->setActivePlayer(-1);
        game->getGameMetaInfo()->setStarted(false);
        emit gameStopped();
    }
    game->getGameState()->setGameStateKnown(true);
    emitUserEvent();
}

void GameEventHandler::processCardAttachmentsForPlayers(const Event_GameStateChanged &event)
{
    for (int i = 0; i < event.player_list_size(); ++i) {
        const ServerInfo_Player &playerInfo = event.player_list(i);
        const ServerInfo_PlayerProperties &prop = playerInfo.properties();
        if (!prop.spectator()) {
            Player *player = game->getPlayerManager()->getPlayers().value(prop.player_id(), 0);
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
    Player *player = game->getPlayerManager()->getPlayers().value(eventPlayerId, 0);
    if (!player)
        return;
    const ServerInfo_PlayerProperties &prop = event.player_properties();
    emit playerPropertiesChanged(prop, eventPlayerId);

    const auto contextType = static_cast<GameEventContext::ContextType>(getPbExtension(context));
    switch (contextType) {
        case GameEventContext::READY_START: {
            bool ready = prop.ready_start();
            if (player->getPlayerInfo()->getLocal())
                emit localPlayerReadyStateChanged(player->getPlayerInfo()->getId(), ready);
            if (ready) {
                emit logReadyStart(player);
            } else {
                emit logNotReadyStart(player);
            }
            break;
        }
        case GameEventContext::CONCEDE: {
            player->setConceded(true);

            QMapIterator<int, Player *> playerIterator(game->getPlayerManager()->getPlayers());
            while (playerIterator.hasNext())
                playerIterator.next().value()->updateZones();

            emit logConcede(eventPlayerId);

            break;
        }
        case GameEventContext::UNCONCEDE: {
            player->setConceded(false);

            QMapIterator<int, Player *> playerIterator(game->getPlayerManager()->getPlayers());
            while (playerIterator.hasNext())
                playerIterator.next().value()->updateZones();

            emit logUnconcede(eventPlayerId);

            break;
        }
        case GameEventContext::DECK_SELECT: {
            Context_DeckSelect deckSelect = context.GetExtension(Context_DeckSelect::ext);
            emit logDeckSelect(player, QString::fromStdString(deckSelect.deck_hash()), deckSelect.sideboard_size());
            if (game->getGameMetaInfo()->proto().share_decklists_on_load() && deckSelect.has_deck_list() &&
                eventPlayerId != game->getPlayerManager()->getLocalPlayerId()) {
                emit remotePlayerDeckSelected(QString::fromStdString(deckSelect.deck_list()), eventPlayerId,
                                              player->getPlayerInfo()->getName());
            }
            break;
        }
        case GameEventContext::SET_SIDEBOARD_LOCK: {
            if (player->getPlayerInfo()->getLocal()) {
                emit localPlayerSideboardLocked(player->getPlayerInfo()->getId(), prop.sideboard_locked());
            }
            emit logSideboardLockSet(player, prop.sideboard_locked());
            break;
        }
        case GameEventContext::CONNECTION_STATE_CHANGED: {
            emit logConnectionStateChanged(player, prop.ping_seconds() != -1);
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
    emit addPlayerToAutoCompleteList(playerName);

    if (game->getPlayerManager()->getPlayers().contains(playerId))
        return;

    if (playerInfo.spectator()) {
        game->getPlayerManager()->addSpectator(playerId, playerInfo);
        emit logJoinSpectator(playerName);
        emit spectatorJoined(playerInfo);
    } else {
        Player *newPlayer = game->getPlayerManager()->addPlayer(playerId, playerInfo.user_info());
        emit logJoinPlayer(newPlayer);
        emit playerJoined(playerInfo);
    }

    emitUserEvent();
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
    Player *player = game->getPlayerManager()->getPlayers().value(eventPlayerId, 0);
    if (!player)
        return;

    emit playerLeft(eventPlayerId);

    emit logLeave(player, getLeaveReason(event.reason()));

    game->getPlayerManager()->removePlayer(eventPlayerId);

    player->clear();
    player->deleteLater();

    // Rearrange all remaining zones so that attachment relationship updates take place
    QMapIterator<int, Player *> playerIterator(game->getPlayerManager()->getPlayers());
    while (playerIterator.hasNext())
        playerIterator.next().value()->updateZones();

    emitUserEvent();
}

void GameEventHandler::eventKicked(const Event_Kicked & /*event*/,
                                   int /*eventPlayerId*/,
                                   const GameEventContext & /*context*/)
{
    emit gameClosed();
    emit logKicked();
    emit playerKicked();
    emitUserEvent();
}

void GameEventHandler::eventReverseTurn(const Event_ReverseTurn &event,
                                        int eventPlayerId,
                                        const GameEventContext & /*context*/)
{
    Player *player = game->getPlayerManager()->getPlayers().value(eventPlayerId, 0);
    if (!player)
        return;

    emit logTurnReversed(player, event.reversed());
}

void GameEventHandler::eventGameHostChanged(const Event_GameHostChanged & /*event*/,
                                            int eventPlayerId,
                                            const GameEventContext & /*context*/)
{
    game->getGameState()->setHostId(eventPlayerId);
}

void GameEventHandler::eventGameClosed(const Event_GameClosed & /*event*/,
                                       int /*eventPlayerId*/,
                                       const GameEventContext & /*context*/)
{
    game->getGameMetaInfo()->setStarted(false);
    game->getGameState()->setGameClosed(true);
    emit gameClosed();
    emit logGameClosed();
    emitUserEvent();
}

void GameEventHandler::eventSetActivePlayer(const Event_SetActivePlayer &event,
                                            int /*eventPlayerId*/,
                                            const GameEventContext & /*context*/)
{
    game->getGameState()->setActivePlayer(event.active_player_id());
    Player *player = game->getPlayerManager()->getPlayer(event.active_player_id());
    if (!player)
        return;
    emit logActivePlayer(player);
    emitUserEvent();
}

void GameEventHandler::eventSetActivePhase(const Event_SetActivePhase &event,
                                           int /*eventPlayerId*/,
                                           const GameEventContext & /*context*/)
{
    const int phase = event.phase();
    if (game->getGameState()->getCurrentPhase() != phase) {
        emit logActivePhaseChanged(phase);
    }
    game->getGameState()->setCurrentPhase(phase);
    emitUserEvent();
}
