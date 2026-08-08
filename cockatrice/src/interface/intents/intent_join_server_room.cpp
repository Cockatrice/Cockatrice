#include "intent_join_server_room.h"

#include "../widgets/tabs/tab_room.h"
#include "../widgets/tabs/tab_server.h"
#include "../widgets/tabs/tab_supervisor.h"
#include "intent_connect_to_server.h"

#include <QTimer>
#include <libcockatrice/protocol/pb/serverinfo_room.pb.h>

IntentJoinServerRoom::IntentJoinServerRoom(TabSupervisor *_tabSupervisor,
                                           RemoteClient *_remoteClient,
                                           ContextJoinRoom *_context)
    : Intent(), tabSupervisor(_tabSupervisor), remoteClient(_remoteClient), context(_context)
{
}

bool IntentJoinServerRoom::checkPrecondition() const
{
    if (remoteClient->getStatus() != ClientStatus::StatusLoggedIn) {
        return false;
    }
    // peerPort() reflects the actual TCP peer, which may differ from the
    // configured server port (e.g. when connecting through a proxy), so only
    // the hostname is compared here.
    if (remoteClient->peerName() != context->serverContext.hostname) {
        return false;
    }
    if (QString::number(remoteClient->peerPort()) != context->serverContext.port) {
        return false;
    }

    return true;
}

void IntentJoinServerRoom::onPreconditionSatisfied()
{
    if (tabSupervisor->getRoomTabs().contains(context->roomId)) {
        tabSupervisor->setCurrentWidget(tabSupervisor->getRoomTabs().value(context->roomId));
        emitFinished();
        return;
    }

    TabServer *tabServer = tabSupervisor->getTabServer();
    if (!tabServer) {
        tabSupervisor->openTabServer();
        tabServer = tabSupervisor->getTabServer();
    }
    if (!tabServer) {
        emitFailed(tr("No server tab available"));
        return;
    }

    const int roomId = context->roomId;
    tabServer->joinRoom(roomId, true);
    connect(tabServer, &TabServer::roomJoined, this, [this, roomId](const ServerInfo_Room &info, bool) {
        if (info.room_id() == roomId) {
            emitFinished();
        }
    });
    connect(tabServer, &TabServer::roomJoinFailed, this, [this, roomId](int failedRoomId) {
        if (failedRoomId == roomId) {
            emitFailed(tr("Failed to join the server room %1").arg(roomId));
        }
    });

    QTimer::singleShot(15000, this,
                       [this, roomId]() { emitFailed(tr("Timed out while joining the server room %1").arg(roomId)); });
}

void IntentJoinServerRoom::onPreconditionNotSatisfied()
{
    runDependency(new IntentConnectToServer(remoteClient, &context->serverContext));
}
