#ifndef SERVER_MATCH_RESULT_STRATEGY_H
#define SERVER_MATCH_RESULT_STRATEGY_H

class Server_AbstractPlayer;
class Server_Game;

/**
 * @brief Strategy hook invoked when a game has finished to decide the match result.
 *
 * Subclasses can report the match outcome (e.g. to a tournament backend) and decide
 * whether the game should be closed permanently; the default implementation never
 * closes the game, preserving the normal return-to-lobby behavior.
 */
class Server_MatchResultStrategy
{
public:
    virtual ~Server_MatchResultStrategy() = default;

    /**
     * @brief Called when a game has finished.
     * @return Whether the game has been decided and should be closed.
     */
    virtual bool onGameFinished(Server_Game *game, int playing, Server_AbstractPlayer *lastPlayer) = 0;
};

/**
 * @brief Default match result strategy that never closes the game.
 */
class Server_NullMatchResultStrategy : public Server_MatchResultStrategy
{
public:
    bool onGameFinished(Server_Game *, int, Server_AbstractPlayer *) override
    {
        return false;
    }
};

#endif
