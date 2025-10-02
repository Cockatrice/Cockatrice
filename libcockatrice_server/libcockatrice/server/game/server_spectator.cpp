#include "server_spectator.h"

Server_Spectator::Server_Spectator(Server_Game *_game,
                                   int _playerId,
                                   const ServerInfo_User &_userInfo,
                                   bool _judge,
                                   Server_AbstractUserInterface *_userInterface)
    : Server_AbstractParticipant(_game, _playerId, _userInfo, _judge, _userInterface)
{
    spectator = true;
}
