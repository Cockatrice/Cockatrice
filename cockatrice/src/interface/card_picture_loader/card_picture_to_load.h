/**
 * @file card_picture_to_load.h
 * @ingroup PictureLoader
 * @brief TODO: Document this.
 */

#ifndef PICTURE_TO_LOAD_H
#define PICTURE_TO_LOAD_H

#include <QLoggingCategory>
#include <libcockatrice/card/printing/exact_card.h>

inline Q_LOGGING_CATEGORY(CardPictureToLoadLog, "card_picture_loader.picture_to_load");

class CardPictureToLoad
{
private:
    ExactCard card;
    QList<CardSetPtr> sortedSets;
    QList<QString> urlTemplates;
    QList<QString> currentSetUrls;
    QString currentUrl;
    CardSetPtr currentSet;

public:
    explicit CardPictureToLoad(const ExactCard &_card);

    [[nodiscard]] const ExactCard &getCard() const
    {
        return card;
    }
    [[nodiscard]] QString getCurrentUrl() const
    {
        return currentUrl;
    }
    [[nodiscard]] CardSetPtr getCurrentSet() const
    {
        return currentSet;
    }
    [[nodiscard]] QString getSetName() const;
    [[nodiscard]] QString transformUrl(const QString &urlTemplate) const;
    bool nextSet();
    bool nextUrl();
    void populateSetUrls();

    static QList<CardSetPtr> extractSetsSorted(const ExactCard &card);
};

#endif // PICTURE_TO_LOAD_H
