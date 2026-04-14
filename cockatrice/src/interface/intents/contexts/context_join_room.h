#ifndef COCKATRICE_CONTEXT_JOIN_ROOM_H
#define COCKATRICE_CONTEXT_JOIN_ROOM_H

#include "context_connect_to_server.h"

#include <QString>

struct ContextJoinRoom
{
    ContextConnectToServer serverContext;
    int roomId;
};

#endif // COCKATRICE_CONTEXT_JOIN_ROOM_H
