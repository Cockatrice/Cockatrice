#ifndef SERVER_MATCH_GAME_FACTORY_H
#define SERVER_MATCH_GAME_FACTORY_H

class GameConfig;
class Server_Game;
class Server_AbstractUserInterface;

class Server_MatchGameFactory
{
public:
    virtual ~Server_MatchGameFactory() = default;

    virtual Server_Game *createMatchGame(const GameConfig &config, int &outGameId) = 0;
    virtual Server_AbstractUserInterface *getUserInterface(const QString &playerName) = 0;
    virtual void addGameToRoom(Server_Game *game) = 0;
};

#endif
