#ifndef CARDLIST_H
#define CARDLIST_H

#include <QList>

class CardItem;

class CardList : public QList<CardItem *> {
private:
	class compareFunctor;
protected:
	bool contentsKnown;
public:
	enum SortFlags { SortByName = 1, SortByType = 2 };
	CardList(bool _contentsKnown);
	CardItem *findCard(const int id, const bool remove, int *position = NULL);
	bool getContentsKnown() const { return contentsKnown; }
	void sort(int flags = SortByName);
};

#endif
