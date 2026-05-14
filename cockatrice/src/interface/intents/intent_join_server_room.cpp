#include "intent_join_server_room.h"

#include "../widgets/tabs/tab_supervisor.h"
#include "intent_helpers.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(IntentJoinRoomLog, "intent.join_room")

IntentJoinServerRoom::IntentJoinServerRoom(const ContextJoinRoom &ctx,
                                           TabSupervisor *supervisor,
                                           ConnectionController *controller,
                                           QObject *parent)
    : Intent(parent), ctx(ctx), supervisor(supervisor), controller(controller)
{
}

void IntentJoinServerRoom::doExecute()
{
    qCDebug(IntentJoinRoomLog) << "Requesting join room" << ctx.roomId;

    // Wire success/failure listeners BEFORE dispatching to avoid races with a
    // synchronous emission from TabSupervisor::requestJoinRoom (already-joined path).
    connect(supervisor, &TabSupervisor::roomJoinedById, this, [this](int roomId) {
        if (roomId == ctx.roomId) {
            qCDebug(IntentJoinRoomLog) << "Room" << ctx.roomId << "joined successfully";
            emitFinished(true);
        }
    });
    connect(supervisor, &TabSupervisor::roomJoinFailedById, this, [this](int roomId) {
        if (roomId == ctx.roomId) {
            qCDebug(IntentJoinRoomLog) << "Failed to join room" << ctx.roomId;
            emitFinished(false);
        }
    });

    abortOnDisconnect(this, controller);
    startTimeoutSafetyNet();

    if (!supervisor->requestJoinRoom(ctx.roomId, true)) {
        qCWarning(IntentJoinRoomLog) << "Server tab not open — cannot join room" << ctx.roomId;
        emitFinished(false);
    }
}
