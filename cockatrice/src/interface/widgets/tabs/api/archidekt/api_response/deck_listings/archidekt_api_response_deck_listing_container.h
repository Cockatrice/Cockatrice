#ifndef COCKATRICE_ARCHIDEKT_DECK_LISTING_API_RESPONSE_CONTAINER_H
#define COCKATRICE_ARCHIDEKT_DECK_LISTING_API_RESPONSE_CONTAINER_H

#include "archidekt_api_response_deck_owner.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QVector>

class ArchidektApiResponseDeckListingContainer
{
public:
    // Constructor
    ArchidektApiResponseDeckListingContainer() = default;

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
    [[nodiscard]] int getSize() const
    {
        return size;
    }
    [[nodiscard]] QString getUpdatedAt() const
    {
        return updatedAt;
    }
    [[nodiscard]] QString getCreatedAt() const
    {
        return createdAt;
    }
    [[nodiscard]] int getDeckFormat() const
    {
        return deckFormat;
    }
    [[nodiscard]] int getEDHBracket() const
    {
        return edhBracket;
    }
    [[nodiscard]] QString getFeatured() const
    {
        return featured;
    }
    [[nodiscard]] QString getCustomFeatured() const
    {
        return customFeatured;
    }
    [[nodiscard]] int getViewCount() const
    {
        return viewCount;
    }
    [[nodiscard]] bool getPrivateDeck() const
    {
        return privateDeck;
    }
    [[nodiscard]] bool getUnlisted() const
    {
        return unlisted;
    }
    [[nodiscard]] bool getTheoryCrafted() const
    {
        return theoryCrafted;
    }
    [[nodiscard]] QString getGame() const
    {
        return game;
    }
    [[nodiscard]] bool getHasDescription() const
    {
        return hasDescription;
    }
    [[nodiscard]] QStringList getTags() const
    {
        return tags;
    }
    [[nodiscard]] int getParentFolderId() const
    {
        return parentFolderId;
    }
    [[nodiscard]] ArchidektApiResponseDeckOwner getOwner() const
    {
        return owner;
    }
    [[nodiscard]] QMap<QString, int> getColors() const
    {
        return colors;
    }
    [[nodiscard]] QString getCardPackage() const
    {
        return cardPackage;
    }
    [[nodiscard]] QString getContest() const
    {
        return contest;
    }

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
    QString game;
    bool hasDescription;
    QStringList tags;
    int parentFolderId;
    ArchidektApiResponseDeckOwner owner;
    QMap<QString, int> colors;
    QString cardPackage;
    QString contest;
};

#endif // COCKATRICE_ARCHIDEKT_DECK_LISTING_API_RESPONSE_CONTAINER_H
