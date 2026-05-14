#ifndef INTENT_JOIN_SERVER_GAME_H
#define INTENT_JOIN_SERVER_GAME_H

#include "contexts/context_join_game.h"

#include <libcockatrice/utility/intent.h>

class TabSupervisor;
class ConnectionController;

/**
 * @brief Sends a Command_JoinGame for the given context and emits finished()
 *        based on the server response.
 *
 * Mirrors GameSelector::joinGame: short-circuits if the user already has a
 * tab for the target game, sets override_restrictions when the user is not
 * admin-locked, and passes through the spectator flag.  Password and
 * join_as_judge are intentionally not exposed via URL.
 *
 * Aborts (finished(false)) on manual disconnect or after a 30-second timeout,
 * so the chain never hangs indefinitely.
 */
class IntentJoinServerGame : public Intent
{
    Q_OBJECT
public:
    explicit IntentJoinServerGame(const ContextJoinGame &ctx,
                                  TabSupervisor *supervisor,
                                  ConnectionController *controller,
                                  QObject *parent = nullptr);

protected:
    void doExecute() override;

private:
    ContextJoinGame ctx;
    TabSupervisor *supervisor;
    ConnectionController *controller;
};

#endif // INTENT_JOIN_SERVER_GAME_H
