#ifndef SERVER_TOURNAMENT_LIFECYCLE_STRATEGY_H
#define SERVER_TOURNAMENT_LIFECYCLE_STRATEGY_H

#include "server_game_lifecycle_strategy.h"

class Server_TournamentLifecycleStrategy : public Server_GameLifecycleStrategy
{
public:
    StartAction onGameStarting(Server_Game *game) override;
};

#endif
