#include "card_list.h"

#include "card_database.h"
#include "card_item.h"

#include <algorithm>

CardList::CardList(bool _contentsKnown) : QList<CardItem *>(), contentsKnown(_contentsKnown)
{
}

/**
 * @brief Finds the CardItem with the given id in the list.
 * If contentsKnown is false, then this just returns the first element of the list.
 *
 * @param cardId The id of the card to find.
 *
 * @returns A pointer to the CardItem, or a nullptr if not found.
 */
CardItem *CardList::findCard(const int cardId) const
{
    if (!contentsKnown && !empty()) {
        return at(0);
    } else {
        for (auto *cardItem : *this) {
            if (cardItem->getId() == cardId) {
                return cardItem;
            }
        }
    }
    return nullptr;
}

class CardList::compareFunctor
{
private:
    int flags;

public:
    explicit compareFunctor(int _flags) : flags(_flags)
    {
    }
    inline bool operator()(CardItem *a, CardItem *b) const
    {
        if (flags & SortByType) {
            QString t1 = a->getInfo() ? a->getInfo()->getMainCardType() : QString();
            QString t2 = b->getInfo() ? b->getInfo()->getMainCardType() : QString();
            if ((t1 == t2) && (flags & SortByName))
                return a->getName() < b->getName();
            return t1 < t2;
        } else
            return a->getName() < b->getName();
    }
};

void CardList::sort(int flags)
{
    compareFunctor cf(flags);
    std::sort(begin(), end(), cf);
}
