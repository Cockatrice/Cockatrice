#ifndef COCKATRICE_ROLE_INFO_H
#define COCKATRICE_ROLE_INFO_H

/**
 * @brief Represents information about a participant's roles.
 */
struct RoleInfo
{
    bool isJudge = false;     ///< Whether the participant joined as a judge
    bool isSpectator = false; ///< Whether the participant joined as a spectator
};

#endif // COCKATRICE_ROLE_INFO_H
