#ifndef SERVER_GAME_LIFECYCLE_STRATEGY_H
#define SERVER_GAME_LIFECYCLE_STRATEGY_H

class Server_Game;

class Server_GameLifecycleStrategy
{
public:
    virtual ~Server_GameLifecycleStrategy() = default;

    enum class StartAction
    {
        ProceedNormal,
        Handled,
    };

    virtual StartAction onGameStarting(Server_Game *game) = 0;
};

class Server_DefaultLifecycleStrategy : public Server_GameLifecycleStrategy
{
public:
    StartAction onGameStarting(Server_Game *) override
    {
        return StartAction::ProceedNormal;
    }
};

#endif
