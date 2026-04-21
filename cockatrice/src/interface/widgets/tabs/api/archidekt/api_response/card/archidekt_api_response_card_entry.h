#ifndef COCKATRICE_ARCHIDEKT_API_RESPONSE_CARD_ENTRY_H
#define COCKATRICE_ARCHIDEKT_API_RESPONSE_CARD_ENTRY_H

#include "archidekt_api_response_card.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QVector>

struct Category
{
    int id;
    QString name;
    bool isPremier;
    bool includedInDeck;
    bool includedInPrice;
};

class ArchidektApiResponseCardEntry
{
public:
    // Constructor
    ArchidektApiResponseCardEntry() = default;

    // Parse deck-related data from JSON
    void fromJson(const QJsonObject &json);

    // Debug method for logging
    void debugPrint() const;

    ArchidektApiResponseCard getCard() const
    {
        return card;
    };

    QList<Category> getCategories() const
    {
        return categories;
    }

    int getQuantity() const
    {
        return quantity;
    }

private:
    int id;
    QList<Category> categories;
    bool companion;
    bool flippedDefault;
    QString label;
    QString modifier;
    int quantity;
    int customCmc;
    QStringList removedCategories;
    QString createdAt;
    QString updatedAt;
    QString deletedAt;
    QString notes;
    ArchidektApiResponseCard card;
};

#endif // COCKATRICE_ARCHIDEKT_API_RESPONSE_CARD_ENTRY_H
