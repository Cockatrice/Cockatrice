#ifndef COCKATRICE_CONTEXT_JOIN_GAME_H
#define COCKATRICE_CONTEXT_JOIN_GAME_H

#include "../../widgets/server/game_join_roles.h"
#include "context_join_room.h"

struct ContextJoinGame
{
    ContextJoinRoom roomContext;
    int gameId;
    GameJoinRoles roles;
};

#endif // COCKATRICE_CONTEXT_JOIN_GAME_H
