#include "intent_join_server_room.h"

#include "../widgets/tabs/tab_room.h"
#include "../widgets/tabs/tab_server.h"
#include "../widgets/tabs/tab_supervisor.h"
#include "intent_connect_to_server.h"

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

    return true;
}

void IntentJoinServerRoom::onPreconditionSatisfied()
{
    if (tabSupervisor->getRoomTabs().contains(context->roomId)) {
        tabSupervisor->setCurrentWidget(tabSupervisor->getRoomTabs().value(context->roomId));
        emit finished();
        return;
    }

    TabServer *tabServer = tabSupervisor->getTabServer();
    if (!tabServer) {
        tabSupervisor->openTabServer();
        tabServer = tabSupervisor->getTabServer();
    }
    if (!tabServer) {
        emit failed(tr("No server tab available"));
        return;
    }

    tabServer->joinRoom(context->roomId, true);
    connect(tabServer, &TabServer::roomJoined, this, &IntentJoinServerRoom::finished);
}

void IntentJoinServerRoom::onPreconditionNotSatisfied()
{
    runDependency(new IntentConnectToServer(remoteClient, &context->serverContext));
}
