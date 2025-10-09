/**
 * @file edhrec_commander_api_response_commander_details.h
 * @ingroup ApiResponses
 * @brief TODO: Document this.
 */

#ifndef EDHREC_COMMANDER_API_RESPONSE_COMMANDER_DETAILS_H
#define EDHREC_COMMANDER_API_RESPONSE_COMMANDER_DETAILS_H

#include "../card_prices/edhrec_api_response_card_prices.h"

#include <QJsonArray>
#include <QJsonObject>
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
    const QString &getAetherhubUri() const
    {
        return aetherhubUri;
    }
    const QString &getArchidektUri() const
    {
        return archidektUri;
    }
    int getCmc() const
    {
        return cmc;
    }
    const QJsonArray &getColorIdentity() const
    {
        return colorIdentity;
    }
    bool isCombos() const
    {
        return combos;
    }
    const QString &getDeckstatsUri() const
    {
        return deckstatsUri;
    }
    const QVector<QString> &getImageUris() const
    {
        return imageUris;
    }
    int getInclusion() const
    {
        return inclusion;
    }
    bool getIsCommander() const
    {
        return isCommander;
    }
    const QString &getLabel() const
    {
        return label;
    }
    const QString &getLayout() const
    {
        return layout;
    }
    bool getLegalCommander() const
    {
        return legalCommander;
    }
    const QString &getMoxfieldUri() const
    {
        return moxfieldUri;
    }
    const QString &getMtggoldfishUri() const
    {
        return mtggoldfishUri;
    }
    const QString &getName() const
    {
        return name;
    }
    const QJsonArray &getNames() const
    {
        return names;
    }
    int getNumDecks() const
    {
        return numDecks;
    }
    int getPotentialDecks() const
    {
        return potentialDecks;
    }
    const QString &getPrecon() const
    {
        return precon;
    }
    const CardPrices &getPrices() const
    {
        return prices;
    }
    const QString &getPrimaryType() const
    {
        return primaryType;
    }
    const QString &getRarity() const
    {
        return rarity;
    }
    double getSalt() const
    {
        return salt;
    }
    const QString &getSanitized() const
    {
        return sanitized;
    }
    const QString &getSanitizedWo() const
    {
        return sanitizedWo;
    }
    const QString &getScryfallUri() const
    {
        return scryfallUri;
    }
    const QString &getSpellbookUri() const
    {
        return spellbookUri;
    }
    const QString &getType() const
    {
        return type;
    }
    const QString &getUrl() const
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
