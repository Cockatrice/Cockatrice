#ifndef INTENT_JOIN_SERVER_ROOM_H
#define INTENT_JOIN_SERVER_ROOM_H

#include "contexts/context_join_room.h"

#include <libcockatrice/utility/intent.h>

class TabSupervisor;
class ConnectionController;

/**
 * @brief Joins the server room identified by @c ctx.roomId.
 *
 * Calls TabSupervisor::requestJoinRoom() and waits for the
 * TabSupervisor::roomJoinedById(roomId) signal before emitting finished().
 * Aborts (finished(false)) on roomJoinFailedById, manual disconnect, or after
 * a 30-second timeout, so the chain never hangs indefinitely.
 */
class IntentJoinServerRoom : public Intent
{
    Q_OBJECT
public:
    explicit IntentJoinServerRoom(const ContextJoinRoom &ctx,
                                  TabSupervisor *supervisor,
                                  ConnectionController *controller,
                                  QObject *parent = nullptr);

protected:
    void doExecute() override;

private:
    ContextJoinRoom ctx;
    TabSupervisor *supervisor;
    ConnectionController *controller;
};

#endif // INTENT_JOIN_SERVER_ROOM_H
