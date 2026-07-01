#ifndef COCKATRICE_CARD_IN_DECK_REQUEST_H
#define COCKATRICE_CARD_IN_DECK_REQUEST_H
#include <QJsonObject>

class CardInDeckRequest
{
public:
    // Constructor
    CardInDeckRequest() = default;

    // Parse deck-related data from JSON
    void fromJson(const QJsonObject &json);
    QJsonObject toJson() const;

    // Debug method for logging
    void debugPrint() const;

private:
    QString card;
    int quantity;
};

#endif // COCKATRICE_CARD_IN_DECK_REQUEST_H
