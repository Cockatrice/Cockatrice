#include "server_spectator.h"

#include "game/server_abstract_participant.h"

class ServerInfo_User;
class Server_AbstractUserInterface;
class Server_Game;

Server_Spectator::Server_Spectator(Server_Game *_game,
                                   int _playerId,
                                   const ServerInfo_User &_userInfo,
                                   bool _judge,
                                   Server_AbstractUserInterface *_userInterface)
    : Server_AbstractParticipant(_game, _playerId, _userInfo, _judge, _userInterface)
{
    spectator = true;
}
