#ifndef COCKATRICE_ARCHIDEKT_API_RESPONSE_DECK_CATEGORY_H
#define COCKATRICE_ARCHIDEKT_API_RESPONSE_DECK_CATEGORY_H

#include "../card/archidekt_api_response_card.h"
#include "../card/archidekt_api_response_card_entry.h"
#include "../deck_listings/archidekt_api_response_deck_owner.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QVector>

class ArchidektApiResponseDeckCategory
{
public:
    // Constructor
    ArchidektApiResponseDeckCategory() = default;

    // Parse deck-related data from JSON
    void fromJson(const QJsonObject &json);

    // Debug method for logging
    void debugPrint() const;

    [[nodiscard]] int getId() const
    {
        return id;
    }
    [[nodiscard]] QString getName() const
    {
        return name;
    }
    [[nodiscard]] bool getIsPremier() const
    {
        return isPremier;
    }
    [[nodiscard]] bool getIncludedInDeck() const
    {
        return includedInDeck;
    }
    [[nodiscard]] bool getIncludedInPrice() const
    {
        return includedInPrice;
    }

private:
    int id;
    QString name;
    bool isPremier;
    bool includedInDeck;
    bool includedInPrice;
};

#endif // COCKATRICE_ARCHIDEKT_API_RESPONSE_DECK_CATEGORY_H
