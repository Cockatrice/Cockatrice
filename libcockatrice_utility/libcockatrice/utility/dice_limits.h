#ifndef DICE_LIMITS_H
#define DICE_LIMITS_H

#include <QtGlobal> // for uint

/** @brief Fewest sides a rollable die may have. */
constexpr uint MINIMUM_DIE_SIDES = 2;
/** @brief Most sides a rollable die may have. */
constexpr uint MAXIMUM_DIE_SIDES = 1000000;
/** @brief Fewest dice that may be rolled at once. */
constexpr uint MINIMUM_DICE_TO_ROLL = 1;
/** @brief Most dice that may be rolled at once. */
constexpr uint MAXIMUM_DICE_TO_ROLL = 100;

#endif // DICE_LIMITS_H
