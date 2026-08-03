#include "intent_join_server_game.h"

#include "../widgets/server/game_selector.h"
#include "../widgets/tabs/tab_room.h"
#include "../widgets/tabs/tab_supervisor.h"
#include "intent_join_server_room.h"

#include <QTimer>

IntentJoinServerGame::IntentJoinServerGame(TabSupervisor *_tabSupervisor,
                                           RemoteClient *_remoteClient,
                                           ContextJoinGame *_context)
    : Intent(), tabSupervisor(_tabSupervisor), remoteClient(_remoteClient), context(_context)
{
}

bool IntentJoinServerGame::checkPrecondition() const
{
    if (remoteClient->getStatus() != ClientStatus::StatusLoggedIn) {
        return false;
    }
    // peerPort() reflects the actual TCP peer, which may differ from the
    // configured server port (e.g. when connecting through a proxy), so only
    // the hostname is compared here.
    if (remoteClient->peerName() != context->roomContext.serverContext.hostname) {
        return false;
    }

    if (!tabSupervisor->getRoomTabs().contains(context->roomContext.roomId)) {
        return false;
    }

    return true;
}

void IntentJoinServerGame::onPreconditionSatisfied()
{
    TabRoom *room = tabSupervisor->getRoomTabs().value(context->roomContext.roomId);
    if (!tryJoinGame(room)) {
        waitForGame(room);
    }
}

void IntentJoinServerGame::onPreconditionNotSatisfied()
{
    runDependency(new IntentJoinServerRoom(tabSupervisor, remoteClient, &context->roomContext));
}

bool IntentJoinServerGame::tryJoinGame(TabRoom *room)
{
    if (!room) {
        return false;
    }

    if (room->getGameSelector()->joinGameById(context->gameId)) {
        joined = true;
        emit finished();
        return true;
    }

    return false;
}

void IntentJoinServerGame::waitForGame(TabRoom *room)
{
    connect(room, &TabRoom::gameListUpdated, this, [this]() {
        TabRoom *updatedRoom = tabSupervisor->getRoomTabs().value(context->roomContext.roomId);
        if (updatedRoom) {
            tryJoinGame(updatedRoom);
        }
    });

    QTimer::singleShot(15000, this, [this]() {
        if (!joined) {
            emit failed(tr("Game %1 not found in the room").arg(context->gameId));
        }
    });
}
