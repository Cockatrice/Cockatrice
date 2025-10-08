#include "server_abstract_participant.h"

#include "../server.h"
#include "../server_abstractuserinterface.h"
#include "../server_database_interface.h"
#include "../server_room.h"
#include "server_arrow.h"
#include "server_card.h"
#include "server_cardzone.h"
#include "server_counter.h"
#include "server_game.h"
#include "server_player.h"

#include <QDebug>
#include <QRegularExpression>
#include <algorithm>
#include <libcockatrice/deck_list/deck_list.h>
#include <libcockatrice/deck_list/deck_list_card_node.h>
#include <libcockatrice/protocol/get_pb_extension.h>
#include <libcockatrice/protocol/pb/command_attach_card.pb.h>
#include <libcockatrice/protocol/pb/command_change_zone_properties.pb.h>
#include <libcockatrice/protocol/pb/command_concede.pb.h>
#include <libcockatrice/protocol/pb/command_create_arrow.pb.h>
#include <libcockatrice/protocol/pb/command_create_counter.pb.h>
#include <libcockatrice/protocol/pb/command_create_token.pb.h>
#include <libcockatrice/protocol/pb/command_deck_select.pb.h>
#include <libcockatrice/protocol/pb/command_del_counter.pb.h>
#include <libcockatrice/protocol/pb/command_delete_arrow.pb.h>
#include <libcockatrice/protocol/pb/command_draw_cards.pb.h>
#include <libcockatrice/protocol/pb/command_dump_zone.pb.h>
#include <libcockatrice/protocol/pb/command_flip_card.pb.h>
#include <libcockatrice/protocol/pb/command_game_say.pb.h>
#include <libcockatrice/protocol/pb/command_inc_card_counter.pb.h>
#include <libcockatrice/protocol/pb/command_inc_counter.pb.h>
#include <libcockatrice/protocol/pb/command_kick_from_game.pb.h>
#include <libcockatrice/protocol/pb/command_leave_game.pb.h>
#include <libcockatrice/protocol/pb/command_move_card.pb.h>
#include <libcockatrice/protocol/pb/command_mulligan.pb.h>
#include <libcockatrice/protocol/pb/command_next_turn.pb.h>
#include <libcockatrice/protocol/pb/command_ready_start.pb.h>
#include <libcockatrice/protocol/pb/command_reveal_cards.pb.h>
#include <libcockatrice/protocol/pb/command_reverse_turn.pb.h>
#include <libcockatrice/protocol/pb/command_roll_die.pb.h>
#include <libcockatrice/protocol/pb/command_set_active_phase.pb.h>
#include <libcockatrice/protocol/pb/command_set_card_attr.pb.h>
#include <libcockatrice/protocol/pb/command_set_card_counter.pb.h>
#include <libcockatrice/protocol/pb/command_set_counter.pb.h>
#include <libcockatrice/protocol/pb/command_set_sideboard_lock.pb.h>
#include <libcockatrice/protocol/pb/command_set_sideboard_plan.pb.h>
#include <libcockatrice/protocol/pb/command_shuffle.pb.h>
#include <libcockatrice/protocol/pb/command_undo_draw.pb.h>
#include <libcockatrice/protocol/pb/context_concede.pb.h>
#include <libcockatrice/protocol/pb/context_connection_state_changed.pb.h>
#include <libcockatrice/protocol/pb/context_deck_select.pb.h>
#include <libcockatrice/protocol/pb/context_move_card.pb.h>
#include <libcockatrice/protocol/pb/context_mulligan.pb.h>
#include <libcockatrice/protocol/pb/context_ready_start.pb.h>
#include <libcockatrice/protocol/pb/context_set_sideboard_lock.pb.h>
#include <libcockatrice/protocol/pb/context_undo_draw.pb.h>
#include <libcockatrice/protocol/pb/event_attach_card.pb.h>
#include <libcockatrice/protocol/pb/event_change_zone_properties.pb.h>
#include <libcockatrice/protocol/pb/event_create_arrow.pb.h>
#include <libcockatrice/protocol/pb/event_create_counter.pb.h>
#include <libcockatrice/protocol/pb/event_create_token.pb.h>
#include <libcockatrice/protocol/pb/event_del_counter.pb.h>
#include <libcockatrice/protocol/pb/event_delete_arrow.pb.h>
#include <libcockatrice/protocol/pb/event_destroy_card.pb.h>
#include <libcockatrice/protocol/pb/event_draw_cards.pb.h>
#include <libcockatrice/protocol/pb/event_dump_zone.pb.h>
#include <libcockatrice/protocol/pb/event_flip_card.pb.h>
#include <libcockatrice/protocol/pb/event_game_say.pb.h>
#include <libcockatrice/protocol/pb/event_move_card.pb.h>
#include <libcockatrice/protocol/pb/event_player_properties_changed.pb.h>
#include <libcockatrice/protocol/pb/event_reveal_cards.pb.h>
#include <libcockatrice/protocol/pb/event_reverse_turn.pb.h>
#include <libcockatrice/protocol/pb/event_roll_die.pb.h>
#include <libcockatrice/protocol/pb/event_set_card_attr.pb.h>
#include <libcockatrice/protocol/pb/event_set_card_counter.pb.h>
#include <libcockatrice/protocol/pb/event_set_counter.pb.h>
#include <libcockatrice/protocol/pb/event_shuffle.pb.h>
#include <libcockatrice/protocol/pb/response.pb.h>
#include <libcockatrice/protocol/pb/response_deck_download.pb.h>
#include <libcockatrice/protocol/pb/response_dump_zone.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_player.pb.h>
#include <libcockatrice/protocol/pb/serverinfo_user.pb.h>
#include <libcockatrice/rng/rng_abstract.h>
#include <libcockatrice/utility/color.h>
#include <libcockatrice/utility/trice_limits.h>

Server_AbstractParticipant::Server_AbstractParticipant(Server_Game *_game,
                                                       int _playerId,
                                                       const ServerInfo_User &_userInfo,
                                                       bool _judge,
                                                       Server_AbstractUserInterface *_userInterface)
    : ServerInfo_User_Container(_userInfo), game(_game), userInterface(_userInterface), pingTime(0),
      playerId(_playerId), judge(_judge)
{
}

Server_AbstractParticipant::~Server_AbstractParticipant() = default;

void Server_AbstractParticipant::removeFromGame()
{
    QMutexLocker locker(&playerMutex);
    if (userInterface) {
        userInterface->playerRemovedFromGame(game);
    }
}

bool Server_AbstractParticipant::updatePingTime() // returns true if ping time changed
{
    QMutexLocker locker(&playerMutex);

    int oldPingTime = pingTime;
    if (userInterface) {
        pingTime = userInterface->getLastCommandTime();
    } else {
        pingTime = -1;
    }

    return pingTime != oldPingTime;
}

void Server_AbstractParticipant::getProperties(ServerInfo_PlayerProperties &result, bool withUserInfo)
{
    result.set_player_id(playerId);
    if (withUserInfo) {
        copyUserInfo(*(result.mutable_user_info()), true);
    }
    result.set_spectator(spectator);
    result.set_judge(judge);
    result.set_ping_seconds(pingTime);
    getPlayerProperties(result);
}

void Server_AbstractParticipant::getPlayerProperties(ServerInfo_PlayerProperties & /*result*/)
{
}

Response::ResponseCode Server_AbstractParticipant::cmdLeaveGame(const Command_LeaveGame & /*cmd*/,
                                                                ResponseContainer & /*rc*/,
                                                                GameEventStorage & /*ges*/)
{
    game->removeParticipant(this, Event_Leave::USER_LEFT);
    return Response::RespOk;
}

Response::ResponseCode Server_AbstractParticipant::cmdKickFromGame(const Command_KickFromGame &cmd,
                                                                   ResponseContainer & /*rc*/,
                                                                   GameEventStorage & /*ges*/)
{
    if ((game->getHostId() != playerId) && !(userInfo->user_level() & ServerInfo_User::IsModerator)) {
        return Response::RespFunctionNotAllowed;
    }

    if (!game->kickParticipant(cmd.player_id())) {
        return Response::RespNameNotFound;
    }

    return Response::RespOk;
}

Response::ResponseCode Server_AbstractParticipant::cmdDeckSelect(const Command_DeckSelect & /*cmd*/,
                                                                 ResponseContainer & /*rc*/,
                                                                 GameEventStorage & /*ges*/)
{
    return Response::RespFunctionNotAllowed;
}

Response::ResponseCode Server_AbstractParticipant::cmdSetSideboardPlan(const Command_SetSideboardPlan & /*cmd*/,
                                                                       ResponseContainer & /*rc*/,
                                                                       GameEventStorage & /*ges*/)
{
    return Response::RespFunctionNotAllowed;
}

Response::ResponseCode Server_AbstractParticipant::cmdSetSideboardLock(const Command_SetSideboardLock & /*cmd*/,
                                                                       ResponseContainer & /*rc*/,
                                                                       GameEventStorage & /*ges*/)
{
    return Response::RespFunctionNotAllowed;
}

Response::ResponseCode Server_AbstractParticipant::cmdConcede(const Command_Concede & /*cmd*/,
                                                              ResponseContainer & /*rc*/,
                                                              GameEventStorage & /*ges*/)
{
    return Response::RespFunctionNotAllowed;
}

Response::ResponseCode Server_AbstractParticipant::cmdUnconcede(const Command_Unconcede & /*cmd*/,
                                                                ResponseContainer & /*rc*/,
                                                                GameEventStorage & /*ges*/)
{
    return Response::RespFunctionNotAllowed;
}

Response::ResponseCode
Server_AbstractParticipant::cmdJudge(const Command_Judge &cmd, ResponseContainer &rc, GameEventStorage &ges)
{
    if (!judge) {
        return Response::RespFunctionNotAllowed;
    }

    auto *player = this->game->getPlayer(cmd.target_id());

    ges.setForcedByJudge(playerId);
    if (player == nullptr) {
        return Response::RespContextError;
    }

    for (int i = 0; i < cmd.game_command_size(); ++i) {
        player->processGameCommand(cmd.game_command(i), rc, ges);
    }

    return Response::RespOk;
}

Response::ResponseCode Server_AbstractParticipant::cmdReadyStart(const Command_ReadyStart & /*cmd*/,
                                                                 ResponseContainer & /*rc*/,
                                                                 GameEventStorage & /*ges*/)
{
    return Response::RespFunctionNotAllowed;
}

Response::ResponseCode
Server_AbstractParticipant::cmdGameSay(const Command_GameSay &cmd, ResponseContainer & /*rc*/, GameEventStorage &ges)
{
    if (spectator) {
        /* Spectators can only talk if:
         * (a) the game creator allows it
         * (b) the spectator is a moderator/administrator
         * (c) the spectator is a judge
         */
        bool isModOrJudge = (userInfo->user_level() & (ServerInfo_User::IsModerator | ServerInfo_User::IsJudge));
        if (!isModOrJudge && !game->getSpectatorsCanTalk()) {
            return Response::RespFunctionNotAllowed;
        }
    }

    if (!userInterface->addSaidMessageSize(static_cast<int>(cmd.message().size()))) {
        return Response::RespChatFlood;
    }
    Event_GameSay event;
    event.set_message(cmd.message());
    ges.enqueueGameEvent(event, playerId);

    game->getRoom()->getServer()->getDatabaseInterface()->logMessage(
        userInfo->id(), QString::fromStdString(userInfo->name()), QString::fromStdString(userInfo->address()),
        textFromStdString(cmd.message()), Server_DatabaseInterface::MessageTargetGame, game->getGameId(),
        game->getDescription());

    return Response::RespOk;
}

Response::ResponseCode Server_AbstractParticipant::cmdShuffle(const Command_Shuffle & /*cmd*/,
                                                              ResponseContainer & /*rc*/,
                                                              GameEventStorage & /*ges*/)
{
    return Response::RespFunctionNotAllowed;
}

Response::ResponseCode Server_AbstractParticipant::cmdMulligan(const Command_Mulligan & /*cmd*/,
                                                               ResponseContainer & /*rc*/,
                                                               GameEventStorage & /*ges*/)
{
    return Response::RespFunctionNotAllowed;
}

Response::ResponseCode Server_AbstractParticipant::cmdRollDie(const Command_RollDie & /*cmd*/,
                                                              ResponseContainer & /*rc*/,
                                                              GameEventStorage & /*ges*/) const
{
    return Response::RespFunctionNotAllowed;
}

Response::ResponseCode Server_AbstractParticipant::cmdDrawCards(const Command_DrawCards & /*cmd*/,
                                                                ResponseContainer & /*rc*/,
                                                                GameEventStorage & /*ges*/)
{
    return Response::RespFunctionNotAllowed;
}

Response::ResponseCode Server_AbstractParticipant::cmdUndoDraw(const Command_UndoDraw & /*cmd*/,
                                                               ResponseContainer & /*rc*/,
                                                               GameEventStorage & /*ges*/)
{
    return Response::RespFunctionNotAllowed;
}

Response::ResponseCode Server_AbstractParticipant::cmdMoveCard(const Command_MoveCard & /*cmd*/,
                                                               ResponseContainer & /*rc*/,
                                                               GameEventStorage & /*ges*/)
{
    return Response::RespFunctionNotAllowed;
}

Response::ResponseCode Server_AbstractParticipant::cmdFlipCard(const Command_FlipCard & /*cmd*/,
                                                               ResponseContainer & /*rc*/,
                                                               GameEventStorage & /*ges*/)
{
    return Response::RespFunctionNotAllowed;
}

Response::ResponseCode Server_AbstractParticipant::cmdAttachCard(const Command_AttachCard & /*cmd*/,
                                                                 ResponseContainer & /*rc*/,
                                                                 GameEventStorage & /*ges*/)
{
    return Response::RespFunctionNotAllowed;
}

Response::ResponseCode Server_AbstractParticipant::cmdCreateToken(const Command_CreateToken & /*cmd*/,
                                                                  ResponseContainer & /*rc*/,
                                                                  GameEventStorage & /*ges*/)
{
    return Response::RespFunctionNotAllowed;
}

Response::ResponseCode Server_AbstractParticipant::cmdCreateArrow(const Command_CreateArrow & /*cmd*/,
                                                                  ResponseContainer & /*rc*/,
                                                                  GameEventStorage & /*ges*/)
{
    return Response::RespFunctionNotAllowed;
}

Response::ResponseCode Server_AbstractParticipant::cmdDeleteArrow(const Command_DeleteArrow & /*cmd*/,
                                                                  ResponseContainer & /*rc*/,
                                                                  GameEventStorage & /*ges*/)
{
    return Response::RespFunctionNotAllowed;
}

Response::ResponseCode Server_AbstractParticipant::cmdSetCardAttr(const Command_SetCardAttr & /*cmd*/,
                                                                  ResponseContainer & /*rc*/,
                                                                  GameEventStorage & /*ges*/)
{
    return Response::RespFunctionNotAllowed;
}

Response::ResponseCode Server_AbstractParticipant::cmdSetCardCounter(const Command_SetCardCounter & /*cmd*/,
                                                                     ResponseContainer & /*rc*/,
                                                                     GameEventStorage & /*ges*/)
{
    return Response::RespFunctionNotAllowed;
}

Response::ResponseCode Server_AbstractParticipant::cmdIncCardCounter(const Command_IncCardCounter & /*cmd*/,
                                                                     ResponseContainer & /*rc*/,
                                                                     GameEventStorage & /*ges*/)
{
    return Response::RespFunctionNotAllowed;
}

Response::ResponseCode Server_AbstractParticipant::cmdIncCounter(const Command_IncCounter & /*cmd*/,
                                                                 ResponseContainer & /*rc*/,
                                                                 GameEventStorage & /*ges*/)
{
    return Response::RespFunctionNotAllowed;
}

Response::ResponseCode Server_AbstractParticipant::cmdCreateCounter(const Command_CreateCounter & /*cmd*/,
                                                                    ResponseContainer & /*rc*/,
                                                                    GameEventStorage & /*ges*/)
{
    return Response::RespFunctionNotAllowed;
}

Response::ResponseCode Server_AbstractParticipant::cmdSetCounter(const Command_SetCounter & /*cmd*/,
                                                                 ResponseContainer & /*rc*/,
                                                                 GameEventStorage & /*ges*/)
{
    return Response::RespFunctionNotAllowed;
}

Response::ResponseCode Server_AbstractParticipant::cmdDelCounter(const Command_DelCounter & /*cmd*/,
                                                                 ResponseContainer & /*rc*/,
                                                                 GameEventStorage & /*ges*/)
{
    return Response::RespFunctionNotAllowed;
}

Response::ResponseCode Server_AbstractParticipant::cmdNextTurn(const Command_NextTurn & /*cmd*/,
                                                               ResponseContainer & /*rc*/,
                                                               GameEventStorage & /*ges*/)
{
    if (!game->getGameStarted()) {
        return Response::RespGameNotStarted;
    }

    if (!judge) {
        return Response::RespFunctionNotAllowed;
    }

    game->nextTurn();
    return Response::RespOk;
}

Response::ResponseCode Server_AbstractParticipant::cmdSetActivePhase(const Command_SetActivePhase &cmd,
                                                                     ResponseContainer & /*rc*/,
                                                                     GameEventStorage & /*ges*/)
{
    if (!game->getGameStarted()) {
        return Response::RespGameNotStarted;
    }

    if (!judge) {
        return Response::RespFunctionNotAllowed;
    }

    game->setActivePhase(cmd.phase());

    return Response::RespOk;
}

Response::ResponseCode Server_AbstractParticipant::cmdDumpZone(const Command_DumpZone & /*cmd*/,
                                                               ResponseContainer & /*rc*/,
                                                               GameEventStorage & /*ges*/)
{
    return Response::RespFunctionNotAllowed;
}

Response::ResponseCode Server_AbstractParticipant::cmdRevealCards(const Command_RevealCards & /*cmd*/,
                                                                  ResponseContainer & /*rc*/,
                                                                  GameEventStorage & /*ges*/)
{
    return Response::RespFunctionNotAllowed;
}

Response::ResponseCode Server_AbstractParticipant::cmdChangeZoneProperties(const Command_ChangeZoneProperties & /*cmd*/,
                                                                           ResponseContainer & /*rc*/,
                                                                           GameEventStorage & /*ges*/)
{
    return Response::RespFunctionNotAllowed;
}

Response::ResponseCode Server_AbstractParticipant::cmdReverseTurn(const Command_ReverseTurn & /*cmd*/,
                                                                  ResponseContainer & /*rc*/,
                                                                  GameEventStorage &ges)
{
    if (!judge) {
        if (spectator) {
            return Response::RespFunctionNotAllowed;
        }

        if (!game->getGameStarted()) {
            return Response::RespGameNotStarted;
        }
    }

    bool reversedTurn = game->reverseTurnOrder();

    Event_ReverseTurn event;
    event.set_reversed(reversedTurn);
    ges.enqueueGameEvent(event, playerId);

    return Response::RespOk;
}

Response::ResponseCode
Server_AbstractParticipant::processGameCommand(const GameCommand &command, ResponseContainer &rc, GameEventStorage &ges)
{
    switch ((GameCommand::GameCommandType)getPbExtension(command)) {
        case GameCommand::KICK_FROM_GAME:
            return cmdKickFromGame(command.GetExtension(Command_KickFromGame::ext), rc, ges);
            break;
        case GameCommand::LEAVE_GAME:
            return cmdLeaveGame(command.GetExtension(Command_LeaveGame::ext), rc, ges);
            break;
        case GameCommand::GAME_SAY:
            return cmdGameSay(command.GetExtension(Command_GameSay::ext), rc, ges);
            break;
        case GameCommand::SHUFFLE:
            return cmdShuffle(command.GetExtension(Command_Shuffle::ext), rc, ges);
            break;
        case GameCommand::MULLIGAN:
            return cmdMulligan(command.GetExtension(Command_Mulligan::ext), rc, ges);
            break;
        case GameCommand::ROLL_DIE:
            return cmdRollDie(command.GetExtension(Command_RollDie::ext), rc, ges);
            break;
        case GameCommand::DRAW_CARDS:
            return cmdDrawCards(command.GetExtension(Command_DrawCards::ext), rc, ges);
            break;
        case GameCommand::UNDO_DRAW:
            return cmdUndoDraw(command.GetExtension(Command_UndoDraw::ext), rc, ges);
            break;
        case GameCommand::FLIP_CARD:
            return cmdFlipCard(command.GetExtension(Command_FlipCard::ext), rc, ges);
            break;
        case GameCommand::ATTACH_CARD:
            return cmdAttachCard(command.GetExtension(Command_AttachCard::ext), rc, ges);
            break;
        case GameCommand::CREATE_TOKEN:
            return cmdCreateToken(command.GetExtension(Command_CreateToken::ext), rc, ges);
            break;
        case GameCommand::CREATE_ARROW:
            return cmdCreateArrow(command.GetExtension(Command_CreateArrow::ext), rc, ges);
            break;
        case GameCommand::DELETE_ARROW:
            return cmdDeleteArrow(command.GetExtension(Command_DeleteArrow::ext), rc, ges);
            break;
        case GameCommand::SET_CARD_ATTR:
            return cmdSetCardAttr(command.GetExtension(Command_SetCardAttr::ext), rc, ges);
            break;
        case GameCommand::SET_CARD_COUNTER:
            return cmdSetCardCounter(command.GetExtension(Command_SetCardCounter::ext), rc, ges);
            break;
        case GameCommand::INC_CARD_COUNTER:
            return cmdIncCardCounter(command.GetExtension(Command_IncCardCounter::ext), rc, ges);
            break;
        case GameCommand::READY_START:
            return cmdReadyStart(command.GetExtension(Command_ReadyStart::ext), rc, ges);
            break;
        case GameCommand::CONCEDE:
            return cmdConcede(command.GetExtension(Command_Concede::ext), rc, ges);
            break;
        case GameCommand::INC_COUNTER:
            return cmdIncCounter(command.GetExtension(Command_IncCounter::ext), rc, ges);
            break;
        case GameCommand::CREATE_COUNTER:
            return cmdCreateCounter(command.GetExtension(Command_CreateCounter::ext), rc, ges);
            break;
        case GameCommand::SET_COUNTER:
            return cmdSetCounter(command.GetExtension(Command_SetCounter::ext), rc, ges);
            break;
        case GameCommand::DEL_COUNTER:
            return cmdDelCounter(command.GetExtension(Command_DelCounter::ext), rc, ges);
            break;
        case GameCommand::NEXT_TURN:
            return cmdNextTurn(command.GetExtension(Command_NextTurn::ext), rc, ges);
            break;
        case GameCommand::SET_ACTIVE_PHASE:
            return cmdSetActivePhase(command.GetExtension(Command_SetActivePhase::ext), rc, ges);
            break;
        case GameCommand::DUMP_ZONE:
            return cmdDumpZone(command.GetExtension(Command_DumpZone::ext), rc, ges);
            break;
        case GameCommand::REVEAL_CARDS:
            return cmdRevealCards(command.GetExtension(Command_RevealCards::ext), rc, ges);
            break;
        case GameCommand::MOVE_CARD:
            return cmdMoveCard(command.GetExtension(Command_MoveCard::ext), rc, ges);
            break;
        case GameCommand::SET_SIDEBOARD_PLAN:
            return cmdSetSideboardPlan(command.GetExtension(Command_SetSideboardPlan::ext), rc, ges);
            break;
        case GameCommand::DECK_SELECT:
            return cmdDeckSelect(command.GetExtension(Command_DeckSelect::ext), rc, ges);
            break;
        case GameCommand::SET_SIDEBOARD_LOCK:
            return cmdSetSideboardLock(command.GetExtension(Command_SetSideboardLock::ext), rc, ges);
            break;
        case GameCommand::CHANGE_ZONE_PROPERTIES:
            return cmdChangeZoneProperties(command.GetExtension(Command_ChangeZoneProperties::ext), rc, ges);
            break;
        case GameCommand::UNCONCEDE:
            return cmdUnconcede(command.GetExtension(Command_Unconcede::ext), rc, ges);
            break;
        case GameCommand::JUDGE:
            return cmdJudge(command.GetExtension(Command_Judge::ext), rc, ges);
            break;
        case GameCommand::REVERSE_TURN:
            return cmdReverseTurn(command.GetExtension(Command_ReverseTurn::ext), rc, ges);
            break;
        default:
            return Response::RespInvalidCommand;
    }
}

void Server_AbstractParticipant::sendGameEvent(const GameEventContainer &cont)
{
    QMutexLocker locker(&playerMutex);

    if (userInterface) {
        userInterface->sendProtocolItem(cont);
    }
}

void Server_AbstractParticipant::setUserInterface(Server_AbstractUserInterface *_userInterface)
{
    playerMutex.lock();
    userInterface = _userInterface;
    playerMutex.unlock();

    pingTime = _userInterface ? 0 : -1;

    Event_PlayerPropertiesChanged event;
    event.mutable_player_properties()->set_ping_seconds(pingTime);

    GameEventStorage ges;
    ges.setGameEventContext(Context_ConnectionStateChanged());
    ges.enqueueGameEvent(event, playerId);
    ges.sendToGame(game);
}

void Server_AbstractParticipant::disconnectClient()
{
    bool isRegistered = userInfo->user_level() & ServerInfo_User::IsRegistered;
    if (!isRegistered || spectator) {
        game->removeParticipant(this, Event_Leave::USER_DISCONNECTED);
    } else {
        setUserInterface(nullptr);
    }
}

void Server_AbstractParticipant::getInfo(ServerInfo_Player *info,
                                         Server_AbstractParticipant * /*recipient*/,
                                         bool /* omniscient */,
                                         bool withUserInfo)
{
    getProperties(*info->mutable_properties(), withUserInfo);
}
