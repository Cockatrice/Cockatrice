#ifndef COCKATRICE_INTENT_JOIN_SERVER_ROOM_H
#define COCKATRICE_INTENT_JOIN_SERVER_ROOM_H

#include "../widgets/tabs/tab_server.h"
#include "../widgets/tabs/tab_supervisor.h"
#include "contexts/context_connect_to_server.h"
#include "contexts/context_join_room.h"
#include "intent.h"
#include "intent_connect_to_server.h"
#include "intent_disconnect_from_server.h"
#include "remote_client.h"

class IntentJoinServerRoom : public Intent
{
    Q_OBJECT

public:
    IntentJoinServerRoom(TabSupervisor *_tabSupervisor, RemoteClient *_remoteClient, ContextJoinRoom *_context)
        : Intent(), tabSupervisor(_tabSupervisor), remoteClient(_remoteClient), context(_context)
    {
    }

protected:
    bool checkPrecondition() const override
    {
        if (remoteClient->getStatus() != ClientStatus::StatusLoggedIn) {
            return false;
        }
        if (remoteClient->peerName() != context->serverContext.hostname) {
            return false;
        }
        if (QString::number(remoteClient->peerPort()) != context->serverContext.port) {
            return false;
        }

        return true;
    }

    void onPreconditionSatisfied() override
    {
        auto tabServer = tabSupervisor->getTabServer();
        tabServer->joinRoom(context->roomId, true);

        connect(tabServer, &TabServer::roomJoined, this, &IntentJoinServerRoom::finished);
    }

    void onPreconditionNotSatisfied() override
    {
        runDependency(new IntentConnectToServer(remoteClient, &context->serverContext));
    }

private:
    TabSupervisor *tabSupervisor;
    RemoteClient *remoteClient;
    ContextJoinRoom *context;
};

#endif // COCKATRICE_INTENT_JOIN_SERVER_ROOM_H
