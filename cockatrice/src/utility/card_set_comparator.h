#ifndef SET_PRIORITY_COMPARATOR_H
#define SET_PRIORITY_COMPARATOR_H

#include "../game/cards/card_database.h"

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

#endif // SET_PRIORITY_COMPARATOR_H