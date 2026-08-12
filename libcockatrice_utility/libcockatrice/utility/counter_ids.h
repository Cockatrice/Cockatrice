/**
 * @file counter_ids.h
 * @ingroup Utility
 * @brief Counter ID ranges and cast count index utilities.
 *
 * Defines the boundary between built-in counters (life, mana colors, storm) and
 * user-created counters, plus helpers for the cast count system used in Commander.
 */

#ifndef COCKATRICE_COUNTER_IDS_H
#define COCKATRICE_COUNTER_IDS_H

#include <QString>

/**
 * @namespace CounterIds
 * @brief Reserved counter ID ranges.
 */
namespace CounterIds
{
constexpr int FirstUserId = 8; ///< First ID available for user-created counters (0-7 are built-in)
} // namespace CounterIds

/**
 * @namespace CastCountIds
 * @brief Cast count index range and utilities.
 *
 * Cast counts use a separate 1-based index space (1-5), not sharing IDs with regular counters.
 */
namespace CastCountIds
{
constexpr int MinIndex = 1;     ///< Minimum valid cast count index
constexpr int MaxIndex = 5;     ///< Maximum valid cast count index
constexpr int Count = MaxIndex; ///< Total number of cast count slots

/** @brief Returns true if the index is in the valid cast count range [1, 5]. */
inline bool isValidIndex(int index)
{
    return index >= MinIndex && index <= MaxIndex;
}

/** @brief Returns the internal name for a cast count at the given index (e.g. "cast_count_1"). */
inline QString nameForIndex(int index)
{
    return QString("cast_count_%1").arg(index);
}
} // namespace CastCountIds

#endif // COCKATRICE_COUNTER_IDS_H
