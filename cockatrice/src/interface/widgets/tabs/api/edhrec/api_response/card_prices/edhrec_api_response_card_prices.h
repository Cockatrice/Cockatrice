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
    [[nodiscard]] const QJsonObject &getCardhoarder() const
    {
        return cardhoarder;
    }
    [[nodiscard]] const QJsonObject &getCardkingdom() const
    {
        return cardkingdom;
    }
    [[nodiscard]] const QJsonObject &getCardmarket() const
    {
        return cardmarket;
    }
    [[nodiscard]] const QJsonObject &getFace2face() const
    {
        return face2face;
    }
    [[nodiscard]] const QJsonObject &getManapool() const
    {
        return manapool;
    }
    [[nodiscard]] const QJsonObject &getMtgstocks() const
    {
        return mtgstocks;
    }
    [[nodiscard]] const QJsonObject &getScg() const
    {
        return scg;
    }
    [[nodiscard]] const QJsonObject &getTcgl() const
    {
        return tcgl;
    }
    [[nodiscard]] const QJsonObject &getTcgplayer() const
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
