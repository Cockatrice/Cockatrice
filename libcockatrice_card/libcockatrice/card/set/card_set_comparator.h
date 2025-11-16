/**
 * @file card_set_comparator.h
 * @ingroup CardSets
 * @brief TODO: Document this.
 */

#ifndef SET_PRIORITY_COMPARATOR_H
#define SET_PRIORITY_COMPARATOR_H

#include "../card_info.h"

class SetPriorityComparator
{
public:
    /*
     * Returns true if a has higher download priority than b
     * Enabled sets have priority over disabled sets
     * Both groups follow the user-defined order
     */
    inline bool operator()(const CardSetPtr &a, const CardSetPtr &b) const
    {
        if (a->getEnabled()) {
            return !b->getEnabled() || a->getSortKey() < b->getSortKey();
        } else {
            return !b->getEnabled() && a->getSortKey() < b->getSortKey();
        }
    }
};

class SetReleaseDateComparator
{
public:
    /*
     * Returns true if a has higher download priority than b
     * Enabled sets have priority over disabled sets
     * Both groups follow the user-defined order
     */
    inline bool operator()(const CardSetPtr &a, const CardSetPtr &b) const
    {
        if (a->getEnabled()) {
            return !b->getEnabled() || a->getReleaseDate() < b->getReleaseDate();
        } else {
            return !b->getEnabled() && a->getReleaseDate() < b->getReleaseDate();
        }
    }
};

class CardSetPriorityComparator
{
public:
    /*
     * Returns true if a has higher download priority than b
     * Enabled sets have priority over disabled sets
     * Both groups follow the user-defined order
     */
    inline bool operator()(const PrintingInfo &a, const PrintingInfo &b) const
    {
        if (a.getSet()->getEnabled()) {
            return !b.getSet()->getEnabled() || a.getSet()->getSortKey() < b.getSet()->getSortKey();
        } else {
            return !b.getSet()->getEnabled() && a.getSet()->getSortKey() < b.getSet()->getSortKey();
        }
    }
};

#endif // SET_PRIORITY_COMPARATOR_H