#ifndef COCKATRICE_INTENT_JOIN_SERVER_GAME_H
#define COCKATRICE_INTENT_JOIN_SERVER_GAME_H

#include "contexts/context_join_game.h"
#include "intent.h"
#include "remote_client.h"

#include <QScopedPointer>
#include <memory>

class TabRoom;
class TabSupervisor;

class IntentJoinServerGame : public Intent
{
    Q_OBJECT

public:
    IntentJoinServerGame(TabSupervisor *_tabSupervisor,
                         RemoteClient *_remoteClient,
                         std::unique_ptr<ContextJoinGame> _context);

protected:
    bool checkPrecondition() const override;
    void onPreconditionSatisfied() override;
    void onPreconditionNotSatisfied() override;

private:
    bool tryJoinGame(TabRoom *room);
    void waitForGame(TabRoom *room);

    TabSupervisor *tabSupervisor;
    RemoteClient *remoteClient;
    QScopedPointer<ContextJoinGame> context;
};

#endif // COCKATRICE_INTENT_JOIN_SERVER_GAME_H
