#include "intent_join_server_game.h"

#include "../widgets/tabs/tab_room.h"
#include "../widgets/tabs/tab_supervisor.h"
#include "intent_helpers.h"

#include <QLoggingCategory>
#include <libcockatrice/protocol/pb/room_commands.pb.h>
#include <libcockatrice/protocol/pending_command.h>

Q_LOGGING_CATEGORY(IntentJoinGameLog, "intent.join_game")

IntentJoinServerGame::IntentJoinServerGame(const ContextJoinGame &ctx,
                                           TabSupervisor *supervisor,
                                           ConnectionController *controller,
                                           QObject *parent)
    : Intent(parent), ctx(ctx), supervisor(supervisor), controller(controller)
{
}

void IntentJoinServerGame::doExecute()
{
    qCDebug(IntentJoinGameLog) << "Requesting join game" << ctx.gameId << "in room" << ctx.roomId
                               << "spectate=" << ctx.spectator;

    // Short-circuit if the user already has a tab for this game (mirrors GameSelector).
    if (supervisor->switchToGameTabIfAlreadyExists(ctx.gameId)) {
        qCDebug(IntentJoinGameLog) << "Game" << ctx.gameId << "tab already open; nothing to do";
        emitFinished(true);
        return;
    }

    TabRoom *room = supervisor->getRoomTabs().value(ctx.roomId);
    if (!room) {
        qCWarning(IntentJoinGameLog) << "Room" << ctx.roomId << "not found — cannot join game";
        emitFinished(false);
        return;
    }

    Command_JoinGame cmd;
    cmd.set_game_id(ctx.gameId);
    cmd.set_spectator(ctx.spectator);
    cmd.set_override_restrictions(!supervisor->getAdminLocked());
    // password and join_as_judge are intentionally not set from URL.

    PendingCommand *pend = room->prepareRoomCommand(cmd);
    connect(pend, &PendingCommand::finished, this,
            [this](const Response &resp, const CommandContainer &, const QVariant &) {
                if (resp.response_code() == Response::RespOk) {
                    qCDebug(IntentJoinGameLog) << "Game" << ctx.gameId << "joined successfully";
                    emitFinished(true);
                } else {
                    qCWarning(IntentJoinGameLog)
                        << "Failed to join game" << ctx.gameId << "response:" << resp.response_code();
                    emitFinished(false);
                }
            });

    abortOnDisconnect(this, controller);
    startTimeoutSafetyNet();

    room->sendRoomCommand(pend);
}
