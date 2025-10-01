#ifndef COCKATRICE_CARD_SET_LIST_H
#define COCKATRICE_CARD_SET_LIST_H

#include "card_set.h"

#include <QList>
#include <QStringList>

class CardSetList : public QList<CardSetPtr>
{
private:
    class KeyCompareFunctor;

public:
    void sortByKey();
    void guessSortKeys();
    void enableAllUnknown();
    void enableAll();
    void markAllAsKnown();
    int getEnabledSetsNum();
    int getUnknownSetsNum();
    QStringList getUnknownSetsNames();
    void defaultSort();
};

#endif // COCKATRICE_CARD_SET_LIST_H
