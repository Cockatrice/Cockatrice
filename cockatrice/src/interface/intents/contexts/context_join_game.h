#ifndef CONTEXT_JOIN_GAME_H
#define CONTEXT_JOIN_GAME_H

struct ContextJoinGame
{
    int gameId;
    int roomId;
    bool spectator{false};
};

#endif // CONTEXT_JOIN_GAME_H
