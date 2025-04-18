#ifndef EDHREC_DECK_API_RESPONSE_H
#define EDHREC_DECK_API_RESPONSE_H

#include "../../../../../../deck/deck_loader.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QVector>

class EdhrecDeckApiResponse
{
public:
    // Constructor
    EdhrecDeckApiResponse() = default;

    // Parse deck-related data from JSON
    void fromJson(const QJsonArray &json);

    // Debug method for logging
    void debugPrint() const;

    DeckLoader *deckLoader;
};

#endif // EDHREC_DECK_API_RESPONSE_H
