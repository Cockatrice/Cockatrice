#ifndef COCKATRICE_CARD_SET_LIST_H
#define COCKATRICE_CARD_SET_LIST_H

#include "card_set.h"

#include <QList>
#include <QSharedPointer>
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
     * @brief Enables every set with fresh sort keys, persisting in one flush.
     *
     * Assigns sequential sort keys in default-sorted order and marks every set
     * known and enabled, writing all records through the controller's batched
     * saveSets() in a single settings flush instead of ~3 QSettings rewrite
     * cycles per set, which froze the GUI thread for seconds on a first run
     * with a full database.
     * @param controller The set priority controller to persist the records with.
     */
    void enableAllBatched(ICardSetPriorityController *controller);

    /**
     * @brief Enables all sets that are unknown or ignored, persisting in one flush.
     *
     * Sets that are not marked as known and not ignored are marked as known
     * and enabled. Ignored-known sets are also enabled, but remain ignored.
     * Only changed sets are written, so the common case of a few new sets costs
     * a single settings flush no matter how large the database is.
     * @param controller The set priority controller to persist the records with.
     */
    void enableAllUnknownBatched(ICardSetPriorityController *controller);

    /**
     * @brief Marks all sets as known and adjusts their enabled state, persisting in one flush.
     *
     * Unknown, non-ignored sets become known and disabled.
     * Ignored-known sets are enabled if they were previously disabled.
     * Only changed sets are written; when nothing is unknown no flush happens at all.
     * @param controller The set priority controller to persist the records with.
     */
    void markAllAsKnownBatched(ICardSetPriorityController *controller);

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
