#ifndef SERVER_TOURNAMENT_MATCH_RESULT_STRATEGY_H
#define SERVER_TOURNAMENT_MATCH_RESULT_STRATEGY_H

#include "server_match_result_strategy.h"

class Server_TournamentMatchResultStrategy : public Server_MatchResultStrategy
{
public:
    bool onGameFinished(Server_Game *game, int playing, Server_AbstractPlayer *lastPlayer) override;
};

#endif
