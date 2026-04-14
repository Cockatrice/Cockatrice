#ifndef COCKATRICE_INTENT_JOIN_SERVER_GAME_H
#define COCKATRICE_INTENT_JOIN_SERVER_GAME_H

#include "../widgets/server/game_selector.h"
#include "../widgets/tabs/tab_room.h"
#include "../widgets/tabs/tab_server.h"
#include "../widgets/tabs/tab_supervisor.h"
#include "contexts/context_join_game.h"
#include "contexts/context_join_room.h"
#include "intent.h"
#include "intent_join_server_room.h"
#include "remote_client.h"

class IntentJoinServerGame : public Intent
{
    Q_OBJECT

public:
    IntentJoinServerGame(TabSupervisor *_tabSupervisor, RemoteClient *_remoteClient, ContextJoinGame *_context)
        : Intent(), tabSupervisor(_tabSupervisor), remoteClient(_remoteClient), context(_context)
    {
    }

protected:
    bool checkPrecondition() const override
    {
        if (remoteClient->getStatus() != ClientStatus::StatusLoggedIn) {
            return false;
        }
        if (remoteClient->peerName() != context->roomContext.serverContext.hostname) {
            return false;
        }
        if (QString::number(remoteClient->peerPort()) != context->roomContext.serverContext.port) {
            return false;
        }

        if (!tabSupervisor->getRoomTabs()[context->roomContext.roomId]) {
            qWarning() << "No room tab";
            return false;
        };

        return true;
    }

    void onPreconditionSatisfied() override
    {
        qWarning() << "All lights green, joining game";
        TabRoom *room = tabSupervisor->getRoomTabs()[context->roomContext.roomId];
        room->getGameSelector()->joinGameById(context->gameId);
    }

    void onPreconditionNotSatisfied() override
    {
        runDependency(new IntentJoinServerRoom(tabSupervisor, remoteClient, &context->roomContext));
    }

private:
    TabSupervisor *tabSupervisor;
    RemoteClient *remoteClient;
    ContextJoinGame *context;
};

#endif // COCKATRICE_INTENT_JOIN_SERVER_GAME_H
