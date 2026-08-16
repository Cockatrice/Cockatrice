#ifndef SERVER_GAME_LIFECYCLE_STRATEGY_H
#define SERVER_GAME_LIFECYCLE_STRATEGY_H

class Server_Game;

/**
 * @brief Strategy hook invoked around a game's lifecycle transitions.
 *
 * Subclasses can intercept the game start to perform custom setup (e.g. draft or
 * tournament initialization); the default implementation never intercepts.
 */
class Server_GameLifecycleStrategy
{
public:
    virtual ~Server_GameLifecycleStrategy() = default;

    /** @brief How the game start should proceed after this hook returns. */
    enum class StartAction
    {
        ProceedNormal, ///< Continue with the normal game start flow.
        Handled,       ///< The strategy handled the start; abort the normal flow.
    };

    /**
     * @brief Called when a game is about to start.
     * @return How the start flow should proceed.
     */
    virtual StartAction onGameStarting(Server_Game *game) = 0;
};

/**
 * @brief Default lifecycle strategy that never intercepts the game start.
 */
class Server_DefaultLifecycleStrategy : public Server_GameLifecycleStrategy
{
public:
    StartAction onGameStarting(Server_Game *) override
    {
        return StartAction::ProceedNormal;
    }
};

#endif
