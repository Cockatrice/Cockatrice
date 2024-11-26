#include "card_list.h"

#include "card_database.h"
#include "card_item.h"

#include <algorithm>

CardList::CardList(bool _contentsKnown) : QList<CardItem *>(), contentsKnown(_contentsKnown)
{
}

CardItem *CardList::findCard(const int id) const
{
    if (!contentsKnown) {
        if (empty())
            return 0;
        CardItem *temp = at(0);
        return temp;
    } else
        for (int i = 0; i < size(); i++) {
            CardItem *temp = at(i);
            if (temp->getId() == id) {
                return temp;
            }
        }
    return 0;
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
