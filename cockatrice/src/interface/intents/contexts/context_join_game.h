#ifndef COCKATRICE_CONTEXT_JOIN_GAME_H
#define COCKATRICE_CONTEXT_JOIN_GAME_H

#include "context_join_room.h"

#include <libcockatrice/utility/game_role_info.h>

struct ContextJoinGame
{
    ContextJoinRoom roomContext;
    int gameId;
    GameRoleInfo roles;
};

#endif // COCKATRICE_CONTEXT_JOIN_GAME_H
