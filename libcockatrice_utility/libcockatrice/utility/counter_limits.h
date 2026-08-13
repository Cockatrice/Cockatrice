#ifndef COUNTER_LIMITS_H
#define COUNTER_LIMITS_H

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

#endif // COUNTER_LIMITS_H
