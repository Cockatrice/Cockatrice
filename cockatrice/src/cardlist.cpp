#include "cardlist.h"
#include "carditem.h"

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
public:
	inline bool operator()(CardItem *a, CardItem *b) const
	{
		return a->getName() < b->getName();
	}
};

void CardList::sort()
{
	compareFunctor cf;
	qSort(begin(), end(), cf);
}
