#ifndef COCKATRICE_GAME_JOIN_ROLES_H
#define COCKATRICE_GAME_JOIN_ROLES_H

/**
 * @brief The combination of roles that a participant is attempting to join a game with.
 */
struct GameJoinRoles
{
    bool asJudge = false;     ///< Join the game as a judge
    bool asSpectator = false; ///< Join the game as a spectator
};

#endif // COCKATRICE_GAME_JOIN_ROLES_H
