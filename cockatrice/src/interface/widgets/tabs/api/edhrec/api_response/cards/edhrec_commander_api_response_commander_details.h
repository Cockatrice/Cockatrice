/**
 * @file edhrec_commander_api_response_commander_details.h
 * @ingroup ApiResponses
 * @brief TODO: Document this.
 */

#ifndef EDHREC_COMMANDER_API_RESPONSE_COMMANDER_DETAILS_H
#define EDHREC_COMMANDER_API_RESPONSE_COMMANDER_DETAILS_H

#include "../card_prices/edhrec_api_response_card_prices.h"

#include <QJsonArray>
#include <QString>
#include <QVector>

class EdhrecCommanderApiResponseCommanderDetails
{
public:
    // Constructor
    EdhrecCommanderApiResponseCommanderDetails() = default;

    // Parse card-related data from JSON
    void fromJson(const QJsonObject &json);

    // Debug method for logging
    void debugPrint() const;

    // Getters for the card data
    [[nodiscard]] const QString &getAetherhubUri() const
    {
        return aetherhubUri;
    }
    [[nodiscard]] const QString &getArchidektUri() const
    {
        return archidektUri;
    }
    [[nodiscard]] int getCmc() const
    {
        return cmc;
    }
    [[nodiscard]] const QJsonArray &getColorIdentity() const
    {
        return colorIdentity;
    }
    [[nodiscard]] bool isCombos() const
    {
        return combos;
    }
    [[nodiscard]] const QString &getDeckstatsUri() const
    {
        return deckstatsUri;
    }
    [[nodiscard]] const QVector<QString> &getImageUris() const
    {
        return imageUris;
    }
    [[nodiscard]] int getInclusion() const
    {
        return inclusion;
    }
    [[nodiscard]] bool getIsCommander() const
    {
        return isCommander;
    }
    [[nodiscard]] const QString &getLabel() const
    {
        return label;
    }
    [[nodiscard]] const QString &getLayout() const
    {
        return layout;
    }
    [[nodiscard]] bool getLegalCommander() const
    {
        return legalCommander;
    }
    [[nodiscard]] const QString &getMoxfieldUri() const
    {
        return moxfieldUri;
    }
    [[nodiscard]] const QString &getMtggoldfishUri() const
    {
        return mtggoldfishUri;
    }
    [[nodiscard]] const QString &getName() const
    {
        return name;
    }
    [[nodiscard]] const QJsonArray &getNames() const
    {
        return names;
    }
    [[nodiscard]] int getNumDecks() const
    {
        return numDecks;
    }
    [[nodiscard]] int getPotentialDecks() const
    {
        return potentialDecks;
    }
    [[nodiscard]] const QString &getPrecon() const
    {
        return precon;
    }
    [[nodiscard]] const CardPrices &getPrices() const
    {
        return prices;
    }
    [[nodiscard]] const QString &getPrimaryType() const
    {
        return primaryType;
    }
    [[nodiscard]] const QString &getRarity() const
    {
        return rarity;
    }
    [[nodiscard]] double getSalt() const
    {
        return salt;
    }
    [[nodiscard]] const QString &getSanitized() const
    {
        return sanitized;
    }
    [[nodiscard]] const QString &getSanitizedWo() const
    {
        return sanitizedWo;
    }
    [[nodiscard]] const QString &getScryfallUri() const
    {
        return scryfallUri;
    }
    [[nodiscard]] const QString &getSpellbookUri() const
    {
        return spellbookUri;
    }
    [[nodiscard]] const QString &getType() const
    {
        return type;
    }
    [[nodiscard]] const QString &getUrl() const
    {
        return url;
    }

private:
    QString aetherhubUri;
    QString archidektUri;
    int cmc = 0;
    QJsonArray colorIdentity;
    bool combos = false;
    QString deckstatsUri;
    QVector<QString> imageUris;
    int inclusion = 0;
    bool isCommander = false;
    QString label;
    QString layout;
    bool legalCommander = false;
    QString moxfieldUri;
    QString mtggoldfishUri;
    QString name;
    QJsonArray names;
    int numDecks = 0;
    int potentialDecks = 0;
    QString precon;
    CardPrices prices;
    QString primaryType;
    QString rarity;
    double salt = 0.0;
    QString sanitized;
    QString sanitizedWo;
    QString scryfallUri;
    QString spellbookUri;
    QString type;
    QString url;
};

#endif // EDHREC_COMMANDER_API_RESPONSE_COMMANDER_DETAILS_H
