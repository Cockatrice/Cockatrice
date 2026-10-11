#ifndef COCKATRICE_INTENT_JOIN_SERVER_ROOM_H
#define COCKATRICE_INTENT_JOIN_SERVER_ROOM_H

#include "intent.h"

#include <qtmetamacros.h>

class TabSupervisor;
class RemoteClient;
struct ContextJoinRoom;

class IntentJoinServerRoom : public Intent
{
    Q_OBJECT

public:
    IntentJoinServerRoom(TabSupervisor *_tabSupervisor, RemoteClient *_remoteClient, ContextJoinRoom *_context);

protected:
    bool checkPrecondition() const override;
    void onPreconditionSatisfied() override;
    void onPreconditionNotSatisfied() override;

private:
    TabSupervisor *tabSupervisor;
    RemoteClient *remoteClient;
    ContextJoinRoom *context;
};

#endif // COCKATRICE_INTENT_JOIN_SERVER_ROOM_H
