#ifndef SERVER_MATCH_GAME_FACTORY_H
#define SERVER_MATCH_GAME_FACTORY_H

#include <QString>

class GameConfig;
class Server_Game;
class Server_AbstractUserInterface;

/**
 * @brief Abstract factory for creating match games, forward-looking for tournament modes.
 *
 * Implementations own the rules of how a match game is constructed, how player
 * interfaces are resolved, and how games are registered with a room.
 */
class Server_MatchGameFactory
{
public:
    virtual ~Server_MatchGameFactory() = default;

    /** @brief Create a match game from @p config, storing the assigned game id in @p outGameId. */
    virtual Server_Game *createMatchGame(const GameConfig &config, int &outGameId) = 0;
    /** @brief Resolve the user interface for @p playerName. */
    virtual Server_AbstractUserInterface *getUserInterface(const QString &playerName) = 0;
    /** @brief Register @p game with the room that owns it. */
    virtual void addGameToRoom(Server_Game *game) = 0;
};

#endif
