#ifndef PICTURE_TO_LOAD_H
#define PICTURE_TO_LOAD_H

#include "../../../game/cards/card_info.h"

#include <QLoggingCategory>

inline Q_LOGGING_CATEGORY(PictureToLoadLog, "picture_loader.picture_to_load");

class PictureToLoad
{
private:
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
