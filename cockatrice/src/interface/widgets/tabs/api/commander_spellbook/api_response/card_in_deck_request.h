#ifndef COCKATRICE_CARD_IN_DECK_REQUEST_H
#define COCKATRICE_CARD_IN_DECK_REQUEST_H
#include <QJsonObject>

struct CardInDeckRequest
{
    static CardInDeckRequest fromJson(const QJsonObject &json);
    QJsonObject toJson() const;

    QString card;
    int quantity;
};

#endif // COCKATRICE_CARD_IN_DECK_REQUEST_H
