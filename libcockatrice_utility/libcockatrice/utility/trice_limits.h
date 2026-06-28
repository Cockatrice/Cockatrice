#ifndef TRICE_LIMITS_H
#define TRICE_LIMITS_H

//! \todo Split trice_limits.h into focused single-purpose headers: string_limits.h,
//!       dice_limits.h, counter_limits.h.

#include <QString>

// max size for short strings, like names and things that are generally a single phrase
constexpr int MAX_NAME_LENGTH = 0xff;
// max size for chat messages and text contents
constexpr int MAX_TEXT_LENGTH = 0xfff;
// max size for deck files and pictures
constexpr int MAX_FILE_LENGTH = 0x1fffff; // about 2 megabytes

constexpr uint MINIMUM_DIE_SIDES = 2;
constexpr uint MAXIMUM_DIE_SIDES = 1000000;
constexpr uint MINIMUM_DICE_TO_ROLL = 1;
constexpr uint MAXIMUM_DICE_TO_ROLL = 100;

/**
 * @brief Upper bound for a bounded counter's value: [0, MAX_COUNTER_VALUE].
 *
 * Caps an individual counter's VALUE (e.g. a +1/+1 counter at 999), not how many counters
 * something holds. Applies to counters that are constrained to a non-negative display range,
 * such as card counters and commander tax. Unbounded counters (e.g. a player's life total)
 * do not use this limit and may go negative, saturating only at the int range.
 *
 * The max of 999 is a display constraint (3-digit rendering) and a reasonable gameplay limit.
 * The server enforces these bounds; the client may also check them for UX optimization.
 */
constexpr int MAX_COUNTER_VALUE = 999;

// optimized functions to get qstrings that are at most that long
static inline QString nameFromStdString(const std::string &_string)
{
    return QString::fromUtf8(_string.data(), std::min(int(_string.size()), MAX_NAME_LENGTH));
}
static inline QString textFromStdString(const std::string &_string)
{
    return QString::fromUtf8(_string.data(), std::min(int(_string.size()), MAX_TEXT_LENGTH));
}
static inline QString fileFromStdString(const std::string &_string)
{
    return QString::fromUtf8(_string.data(), std::min(int(_string.size()), MAX_FILE_LENGTH));
}

#endif // TRICE_LIMITS_H
