#include "card_in_deck_request.h"

void CardInDeckRequest::fromJson(const QJsonObject &json)
{
    card = json.value("card").toString();
    quantity = json.value("quantity").toInt();
}

QJsonObject CardInDeckRequest::toJson() const
{
    QJsonObject json;
    json.insert("card", card);
    json.insert("quantity", quantity);
    return json;
}

void CardInDeckRequest::debugPrint() const
{
    qDebug() << "Card:" << card;
    qDebug() << "Quantity:" << quantity;
}