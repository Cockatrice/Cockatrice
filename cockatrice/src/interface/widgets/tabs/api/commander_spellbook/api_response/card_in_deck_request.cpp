#include "card_in_deck_request.h"

CardInDeckRequest CardInDeckRequest::fromJson(const QJsonObject &json)
{
    CardInDeckRequest request;
    request.card = json.value("card").toString();
    request.quantity = json.value("quantity").toInt();
    return request;
}

QJsonObject CardInDeckRequest::toJson() const
{
    QJsonObject json;
    json.insert("card", card);
    json.insert("quantity", quantity);
    return json;
}
