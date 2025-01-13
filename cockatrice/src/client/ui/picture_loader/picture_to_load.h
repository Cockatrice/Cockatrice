#ifndef PICTURE_TO_LOAD_H
#define PICTURE_TO_LOAD_H

#include "../../../game/cards/card_database.h"

class PictureToLoad
{
private:
    class SetDownloadPriorityComparator
    {
    public:
        /*
         * Returns true if a has higher download priority than b
         * Enabled sets have priority over disabled sets
         * Both groups follows the user-defined order
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

    CardInfoPtr card;
    QList<CardSetPtr> sortedSets;
    QList<QString> urlTemplates;
    QList<QString> currentSetUrls;
    QString currentUrl;
    CardSetPtr currentSet;

public:
    explicit PictureToLoad(CardInfoPtr _card = CardInfoPtr());

    CardInfoPtr getCard() const
    {
        return card;
    }
    void clear()
    {
        card.clear();
    }
    QString getCurrentUrl() const
    {
        return currentUrl;
    }
    CardSetPtr getCurrentSet() const
    {
        return currentSet;
    }
    QString getSetName() const;
    QString transformUrl(const QString &urlTemplate) const;
    bool nextSet();
    bool nextUrl();
    void populateSetUrls();
};

#endif // PICTURE_TO_LOAD_H
