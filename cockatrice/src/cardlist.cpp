#include "cardlist.h"
#include "carditem.h"
#include "carddatabase.h"

CardList::CardList(bool _contentsKnown)
    : QList<CardItem *>(), contentsKnown(_contentsKnown)
{
}

CardItem *CardList::findCard(const int id, const bool remove, int *position)
{
    if (!contentsKnown) {
        if (empty())
            return 0;
        CardItem *temp = at(0);
        if (remove)
            removeAt(0);
        if (position)
            *position = id;
        return temp;
    } else
        for (int i = 0; i < size(); i++) {
            CardItem *temp = at(i);
            if (temp->getId() == id) {
                if (remove)
                    removeAt(i);
                if (position)
                    *position = i;
                return temp;
            }
        }
    return 0;
}

class CardList::compareFunctor {
private:
    int flags;
public:
    compareFunctor(int _flags) : flags(_flags)
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
    qSort(begin(), end(), cf);
}
