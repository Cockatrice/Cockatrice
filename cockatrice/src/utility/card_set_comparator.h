#ifndef SET_PRIORITY_COMPARATOR_H
#define SET_PRIORITY_COMPARATOR_H

#include "../game/cards/card_info.h"

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
    inline bool operator()(const CardInfoPerSet &a, const CardInfoPerSet &b) const
    {
        if (a.getPtr()->getEnabled()) {
            return !b.getPtr()->getEnabled() || a.getPtr()->getSortKey() < b.getPtr()->getSortKey();
        } else {
            return !b.getPtr()->getEnabled() && a.getPtr()->getSortKey() < b.getPtr()->getSortKey();
        }
    }
};

#endif // SET_PRIORITY_COMPARATOR_H