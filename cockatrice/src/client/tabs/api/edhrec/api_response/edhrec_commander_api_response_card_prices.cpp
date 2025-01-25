#include "edhrec_commander_api_response_card_prices.h"

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
    qDebug() << "Card Prices:";
    qDebug() << "Cardhoarder:" << cardhoarder;
    qDebug() << "Cardkingdom:" << cardkingdom;
    qDebug() << "Cardmarket:" << cardmarket;
    qDebug() << "Face2Face:" << face2face;
    qDebug() << "Manapool:" << manapool;
    qDebug() << "Mtgstocks:" << mtgstocks;
    qDebug() << "SCG:" << scg;
    qDebug() << "TCGL:" << tcgl;
    qDebug() << "Tcgplayer:" << tcgplayer;
}
