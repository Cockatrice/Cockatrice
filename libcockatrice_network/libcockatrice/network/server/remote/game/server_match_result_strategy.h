#ifndef SERVER_MATCH_RESULT_STRATEGY_H
#define SERVER_MATCH_RESULT_STRATEGY_H

class Server_AbstractPlayer;
class Server_Game;

class Server_MatchResultStrategy
{
public:
    virtual ~Server_MatchResultStrategy() = default;

    virtual bool onGameFinished(Server_Game *game, int playing, Server_AbstractPlayer *lastPlayer) = 0;
};

class Server_NullMatchResultStrategy : public Server_MatchResultStrategy
{
public:
    bool onGameFinished(Server_Game *, int, Server_AbstractPlayer *) override
    {
        return false;
    }
};

#endif
