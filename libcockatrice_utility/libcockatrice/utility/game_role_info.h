#ifndef COCKATRICE_GAME_ROLE_INFO_H
#define COCKATRICE_GAME_ROLE_INFO_H

/**
 * @brief What in-game roles a participant has.
 */
struct GameRoleInfo
{
    bool isJudge = false;     ///< Whether the participant joined as a judge
    bool isSpectator = false; ///< Whether the participant joined as a spectator
};

#endif // COCKATRICE_GAME_ROLE_INFO_H
