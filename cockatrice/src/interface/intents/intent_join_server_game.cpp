#include "intent_join_server_game.h"

#include "../widgets/server/game_selector.h"
#include "../widgets/tabs/tab_room.h"
#include "../widgets/tabs/tab_supervisor.h"
#include "intent_join_server_room.h"

#include <QTimer>

IntentJoinServerGame::IntentJoinServerGame(TabSupervisor *_tabSupervisor,
                                           RemoteClient *_remoteClient,
                                           std::unique_ptr<ContextJoinGame> _context)
    : Intent(), tabSupervisor(_tabSupervisor), remoteClient(_remoteClient), context(_context.release())
{
}

bool IntentJoinServerGame::checkPrecondition() const
{
    if (remoteClient->getStatus() != ClientStatus::StatusLoggedIn) {
        return false;
    }
    // serverName()/serverPort() reflect the server the client was configured
    // to connect to, which may differ from the actual TCP peer (e.g. when
    // connecting through a proxy), so compare those configured values. A link
    // naming the same host on another port is a different server and must not
    // reuse the session there.
    if (remoteClient->serverName().compare(context->roomContext.serverContext.hostname, Qt::CaseInsensitive) != 0) {
        return false;
    }
    if (QString::number(remoteClient->serverPort()) != context->roomContext.serverContext.port) {
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

    if (room->getGameSelector()->joinGameById(context->gameId, context->asSpectator)) {
        emitFinished();
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

    QTimer::singleShot(15000, this, [this]() { emitFailed(tr("Game %1 not found in the room").arg(context->gameId)); });
}
