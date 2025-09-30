/**
 * @file edhrec_api_response_card_prices.h
 * @ingroup ApiResponses
 * @brief TODO: Document this.
 */

#ifndef EDHREC_COMMANDER_API_RESPONSE_CARD_PRICES_H
#define EDHREC_COMMANDER_API_RESPONSE_CARD_PRICES_H

#include <QJsonObject>

class CardPrices
{
public:
    // Constructor
    CardPrices() = default;

    // Parse prices from JSON
    void fromJson(const QJsonObject &json);
    void debugPrint() const;

    // Getter methods for card prices
    const QJsonObject &getCardhoarder() const
    {
        return cardhoarder;
    }
    const QJsonObject &getCardkingdom() const
    {
        return cardkingdom;
    }
    const QJsonObject &getCardmarket() const
    {
        return cardmarket;
    }
    const QJsonObject &getFace2face() const
    {
        return face2face;
    }
    const QJsonObject &getManapool() const
    {
        return manapool;
    }
    const QJsonObject &getMtgstocks() const
    {
        return mtgstocks;
    }
    const QJsonObject &getScg() const
    {
        return scg;
    }
    const QJsonObject &getTcgl() const
    {
        return tcgl;
    }
    const QJsonObject &getTcgplayer() const
    {
        return tcgplayer;
    }

private:
    QJsonObject cardhoarder;
    QJsonObject cardkingdom;
    QJsonObject cardmarket;
    QJsonObject face2face;
    QJsonObject manapool;
    QJsonObject mtgstocks;
    QJsonObject scg;
    QJsonObject tcgl;
    QJsonObject tcgplayer;
};

#endif // EDHREC_COMMANDER_API_RESPONSE_CARD_PRICES_H
