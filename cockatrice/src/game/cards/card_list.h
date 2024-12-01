#ifndef CARDLIST_H
#define CARDLIST_H

#include <QList>

class CardItem;

class CardList : public QList<CardItem *>
{
protected:
    bool contentsKnown;

public:
    enum SortOption
    {
        NoSort,

        // Options that are used by groupBy
        // Should partition all cards into a reasonable number of buckets
        SortByMainType,
        SortByManaValue,
        SortByColorGrouping,

        // Options that are used by sortBy
        // We don't care about buckets; we want as many distinct values as possible.
        SortByName,
        SortByType,
        SortByManaCost,
        SortByColors,
        SortByPt,
        SortBySet
    };
    CardList(bool _contentsKnown);
    CardItem *findCard(const int cardId) const;
    bool getContentsKnown() const
    {
        return contentsKnown;
    }

    void sortBy(const QList<SortOption> &options);

    static std::function<QString(CardItem *)> getExtractorFor(SortOption option);
};

#endif
