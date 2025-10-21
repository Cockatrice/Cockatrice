#ifndef SPECTATOR_H
#define SPECTATOR_H

#include "server_abstract_participant.h"

class Server_Spectator : public Server_AbstractParticipant
{
    Q_OBJECT
public:
    Server_Spectator(Server_Game *_game,
                     int _playerId,
                     const ServerInfo_User &_userInfo,
                     bool _judge,
                     Server_AbstractUserInterface *_handler);
};

#endif
