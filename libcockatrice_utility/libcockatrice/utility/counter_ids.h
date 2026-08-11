/**
 * @file counter_ids.h
 * @ingroup Core
 * @brief Shared counter IDs and names for system counters (e.g. tax counters).
 */

#ifndef COCKATRICE_COUNTER_IDS_H
#define COCKATRICE_COUNTER_IDS_H

#include <QString>

/**
 * @namespace CounterIds
 * @brief Shared counter IDs used by both client and server.
 *
 * Single source of truth: included directly by both sides, so they cannot drift.
 *
 * Reserved counter IDs for system counters:
 *   IDs 0-7:   Standard player counters (life, mana colors, storm)
 *   IDs 8-12:  Tax counters (1st through 5th)
 *   IDs 13+:   Available for user-created counters (FirstUserId)
 *
 * newCounterId() always returns >= FirstUserId to prevent user counters
 * from colliding with reserved IDs, even in non-Commander games.
 */
namespace CounterIds
{
constexpr int TaxCounter1 = 8;  ///< 1st tax counter
constexpr int TaxCounter2 = 9;  ///< 2nd tax counter
constexpr int TaxCounter3 = 10; ///< 3rd tax counter
constexpr int TaxCounter4 = 11; ///< 4th tax counter
constexpr int TaxCounter5 = 12; ///< 5th tax counter
constexpr int FirstUserId = 13; ///< First ID available for user-created counters

constexpr int FirstTaxCounterId = TaxCounter1;
constexpr int LastTaxCounterId = TaxCounter5;
constexpr int TaxCounterCount = LastTaxCounterId - FirstTaxCounterId + 1;

/** @brief Returns true if the given ID is a reserved tax counter. */
inline bool isTaxCounter(int id)
{
    return id >= FirstTaxCounterId && id <= LastTaxCounterId;
}

/** @brief Returns the tax counter index (0-based) for display, or -1 if not a tax counter. */
inline int taxCounterIndex(int id)
{
    if (!isTaxCounter(id)) {
        return -1;
    }
    return id - FirstTaxCounterId;
}

/** @brief Returns the tax counter ID for the given 0-based index, or -1 if out of range. */
inline int taxCounterIdFromIndex(int index)
{
    if (index < 0 || index >= TaxCounterCount) {
        return -1;
    }
    return FirstTaxCounterId + index;
}
} // namespace CounterIds

/**
 * @namespace CounterNames
 * @brief Reserved counter names for server-managed tax counters.
 *
 * Used to reject user-created counters that would spoof system counters.
 */
namespace CounterNames
{
constexpr const char *TaxCounter1 = "1st_tax_counter";
constexpr const char *TaxCounter2 = "2nd_tax_counter";
constexpr const char *TaxCounter3 = "3rd_tax_counter";
constexpr const char *TaxCounter4 = "4th_tax_counter";
constexpr const char *TaxCounter5 = "5th_tax_counter";

/** @brief Returns the name for the given tax counter ID, or nullptr if not a tax counter. */
inline const char *forId(int id)
{
    switch (id) {
        case CounterIds::TaxCounter1:
            return TaxCounter1;
        case CounterIds::TaxCounter2:
            return TaxCounter2;
        case CounterIds::TaxCounter3:
            return TaxCounter3;
        case CounterIds::TaxCounter4:
            return TaxCounter4;
        case CounterIds::TaxCounter5:
            return TaxCounter5;
        default:
            return nullptr;
    }
}

/** @brief Returns true if the given name is a reserved tax counter name. */
inline bool isTaxCounter(const QString &name)
{
    return name == TaxCounter1 || name == TaxCounter2 || name == TaxCounter3 || name == TaxCounter4 ||
           name == TaxCounter5;
}
} // namespace CounterNames

#endif // COCKATRICE_COUNTER_IDS_H
