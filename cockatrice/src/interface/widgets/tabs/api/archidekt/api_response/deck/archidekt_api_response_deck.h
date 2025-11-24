#ifndef COCKATRICE_ARCHIDEKT_API_RESPONSE_DECK_H
#define COCKATRICE_ARCHIDEKT_API_RESPONSE_DECK_H

#include "../card/archidekt_api_response_card.h"
#include "../card/archidekt_api_response_card_entry.h"
#include "../deck_listings/archidekt_api_response_deck_owner.h"
#include "archidekt_api_response_deck_category.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QVector>

class ArchidektApiResponseDeck
{
public:
    // Constructor
    ArchidektApiResponseDeck() = default;

    // Parse deck-related data from JSON
    void fromJson(const QJsonObject &json);

    // Debug method for logging
    void debugPrint() const;

    QVector<ArchidektApiResponseCardEntry> getCards() const
    {
        return cards;
    };

    QVector<ArchidektApiResponseDeckCategory> getCategories() const
    {
        return categories;
    }

    QString getDeckName() const
    {
        return name;
    };

private:
    int id;
    QString name;
    int size;
    QString updatedAt;
    QString createdAt;
    int deckFormat;
    int edhBracket;
    QString featured;
    QString customFeatured;
    int viewCount;
    bool privateDeck;
    bool unlisted;
    bool theoryCrafted;
    int points;
    int userInput;
    ArchidektApiResponseDeckOwner owner;
    int commentRoot;
    QString editors;
    int parentFolderId;
    bool bookmarked;
    QVector<ArchidektApiResponseDeckCategory> categories;
    QStringList deckTags;
    QString playgroupDeckUrl;
    QString cardPackage;
    QVector<ArchidektApiResponseCardEntry> cards;
    QStringList customCards;
};

#endif // COCKATRICE_ARCHIDEKT_API_RESPONSE_DECK_H
