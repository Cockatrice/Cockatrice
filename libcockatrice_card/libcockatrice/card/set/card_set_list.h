#ifndef COCKATRICE_CARD_SET_LIST_H
#define COCKATRICE_CARD_SET_LIST_H

#include "card_set.h"

#include <QList>
#include <QStringList>

/**
 * @class CardSetList
 * @ingroup CardSets
 *
 * @brief A list-like container for CardSet objects with extended management methods.
 *
 * Extends `QList<CardSetPtr>` by adding convenience operations for sorting,
 * enabling/disabling sets, and tracking known/unknown status. Unlike a plain
 * list, this container provides domain-specific functionality for handling
 * groups of sets in bulk.
 */
class CardSetList : public QList<CardSetPtr>
{
private:
    /**
     * @class KeyCompareFunctor
     * @brief Internal comparison functor for sorting by sort key.
     *
     * Used internally by `sortByKey()` to order sets consistently
     * according to their assigned numeric sort keys.
     */
    class KeyCompareFunctor;

public:
    /**
     * @brief Sorts the set list by each set’s assigned sort key.
     *
     * Uses KeyCompareFunctor internally. If two sets share the
     * same sort key, their relative order is unspecified.
     */
    void sortByKey();

    /**
     * @brief Reassigns sort keys based on the current order.
     *
     * Calls defaultSort() and then assigns sequential sort keys
     * to all sets according to their resulting positions, replacing
     * any existing sort keys to ensure consistent ordering.
     */
    void guessSortKeys();

    /**
     * @brief Enables all sets that are unknown or ignored.
     *
     * Sets that are not marked as known and not ignored are marked as known
     * and enabled. Ignored-known sets are also enabled, but remain ignored.
     */
    void enableAllUnknown();

    /**
     * @brief Enables all sets in the list.
     *
     * Equivalent to calling `setEnabled(true)` on each entry.
     */
    void enableAll();

    /**
     * @brief Marks all sets as known and adjusts their enabled state.
     *
     * Unknown, non-ignored sets become known and disabled.
     * Ignored-known sets are enabled if they were previously disabled.
     */
    void markAllAsKnown();

    /**
     * @brief Counts the number of sets that are currently enabled.
     *
     * @return Integer count of enabled sets.
     */
    int getEnabledSetsNum();

    /**
     * @brief Counts the number of sets that are currently unknown.
     *
     * @return Integer count of unknown sets.
     */
    int getUnknownSetsNum();

    /**
     * @brief Collects the short names of all sets marked as unknown.
     *
     * @return A list of unknown set names.
     */
    QStringList getUnknownSetsNames();

    /**
     * @brief Sorts the list by default rules.
     *
     * Orders sets first by priority (ascending), then by release date
     * (most recent first), and finally alphabetically by short name.
     */
    void defaultSort();
};

#endif // COCKATRICE_CARD_SET_LIST_H
