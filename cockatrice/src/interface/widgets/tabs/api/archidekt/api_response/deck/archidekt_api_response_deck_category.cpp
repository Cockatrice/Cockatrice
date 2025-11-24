#include "archidekt_api_response_deck_category.h"

void ArchidektApiResponseDeckCategory::fromJson(const QJsonObject &json)
{
    id = json.value("id").toInt();
    name = json.value("name").toString();
    isPremier = json.value("isPremier").toBool();
    includedInDeck = json.value("includedInDeck").toBool();
    includedInPrice = json.value("includedInPrice").toBool();
}

void ArchidektApiResponseDeckCategory::debugPrint() const
{
    qDebug() << "Id:" << id;
    qDebug() << "Name:" << name;
    qDebug() << "isPremier:" << isPremier;
    qDebug() << "IncludedInDeck:" << includedInDeck;
    qDebug() << "IncludedInPrice:" << includedInPrice;
}