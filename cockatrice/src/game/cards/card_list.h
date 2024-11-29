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
        SortByName,
        SortByType,
        SortByManaValue
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
