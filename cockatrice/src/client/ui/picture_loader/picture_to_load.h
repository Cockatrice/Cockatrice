#ifndef PICTURE_TO_LOAD_H
#define PICTURE_TO_LOAD_H

#include "../../../game/cards/exact_card.h"

#include <QLoggingCategory>

inline Q_LOGGING_CATEGORY(PictureToLoadLog, "picture_loader.picture_to_load");

class PictureToLoad
{
private:
    ExactCard card;
    QList<CardSetPtr> sortedSets;
    QList<QString> urlTemplates;
    QList<QString> currentSetUrls;
    QString currentUrl;
    CardSetPtr currentSet;

public:
    explicit PictureToLoad(const ExactCard &_card);

    const ExactCard &getCard() const
    {
        return card;
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

    static QList<CardSetPtr> extractSetsSorted(const ExactCard &card);
};

#endif // PICTURE_TO_LOAD_H
