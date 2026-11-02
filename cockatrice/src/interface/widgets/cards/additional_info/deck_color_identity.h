#ifndef COCKATRICE_DECK_COLOR_IDENTITY_H
#define COCKATRICE_DECK_COLOR_IDENTITY_H

#include <QString>

class CardDatabaseQuerier;
class DeckList;

/**
 * @brief Computes the color identity of the main deck and sideboard, ordered WUBRG.
 *
 * Returns "C" for a known colorless deck and an empty string for an empty deck
 * or unavailable card data. Uses card color identity where available, falling
 * back to card colors for older/custom databases that lack that property.
 *
 * Shared as a free function so the deck storage previews and the deck share
 * dialog compute identities identically.
 *
 * @param db Card database used to look up card color identities.
 */
QString getDeckColorIdentity(const DeckList &deck, const CardDatabaseQuerier *db);

#endif // COCKATRICE_DECK_COLOR_IDENTITY_H
