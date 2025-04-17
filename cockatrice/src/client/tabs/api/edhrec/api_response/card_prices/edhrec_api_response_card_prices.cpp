#include "edhrec_api_response_card_prices.h"

#include <QDebug>

void CardPrices::fromJson(const QJsonObject &json)
{
    // Parse prices from various sources
    cardhoarder = json.value("cardhoarder").toObject();
    cardkingdom = json.value("cardkingdom").toObject();
    cardmarket = json.value("cardmarket").toObject();
    face2face = json.value("face2face").toObject();
    manapool = json.value("manapool").toObject();
    mtgstocks = json.value("mtgstocks").toObject();
    scg = json.value("scg").toObject();
    tcgl = json.value("tcgl").toObject();
    tcgplayer = json.value("tcgplayer").toObject();
}

void CardPrices::debugPrint() const
{
    qInfo() << "Card Prices:";
    qInfo() << "Cardhoarder:" << cardhoarder;
    qInfo() << "Cardkingdom:" << cardkingdom;
    qInfo() << "Cardmarket:" << cardmarket;
    qInfo() << "Face2Face:" << face2face;
    qInfo() << "Manapool:" << manapool;
    qInfo() << "Mtgstocks:" << mtgstocks;
    qInfo() << "SCG:" << scg;
    qInfo() << "TCGL:" << tcgl;
    qInfo() << "Tcgplayer:" << tcgplayer;
}
