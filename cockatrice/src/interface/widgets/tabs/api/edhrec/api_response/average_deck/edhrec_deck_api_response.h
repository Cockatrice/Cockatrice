/**
 * @file edhrec_deck_api_response.h
 * @ingroup ApiResponses
 * @brief TODO: Document this.
 */

#ifndef EDHREC_DECK_API_RESPONSE_H
#define EDHREC_DECK_API_RESPONSE_H

#include "../../../../../../deck_loader/deck_loader.h"

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
