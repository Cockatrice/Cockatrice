#ifndef CLAMPED_ARITHMETIC_H
#define CLAMPED_ARITHMETIC_H

#include <QtGlobal>
#include <cstdint>

/**
 * @brief Overflow-safe clamped addition: returns value + delta bounded to [minValue, maxValue].
 *
 * Uses a 64-bit intermediate so the addition cannot overflow int. Shared by the bounded
 * counter arithmetic in both the client and the server.
 *
 * @note Requires minValue <= maxValue. Bounds come from trusted compile-time call sites;
 *       qBound() asserts this internally in debug builds.
 */
inline int addClamped(int value, int delta, int minValue, int maxValue)
{
    const auto result = static_cast<int64_t>(value) + static_cast<int64_t>(delta);
    return static_cast<int>(qBound(static_cast<int64_t>(minValue), result, static_cast<int64_t>(maxValue)));
}

#endif // CLAMPED_ARITHMETIC_H
