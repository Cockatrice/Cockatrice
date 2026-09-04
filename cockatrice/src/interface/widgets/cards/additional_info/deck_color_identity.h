#ifndef COCKATRICE_DECK_COLOR_IDENTITY_H
#define COCKATRICE_DECK_COLOR_IDENTITY_H

#include <QString>

class DeckList;

/**
 * @brief Computes the color identity of a deck (e.g. "WUBRG") from the color
 *        symbols of all cards in the main deck and sideboard, ordered WUBRG.
 *
 * Shared as a free function so the deck storage previews and the deck share
 * dialog compute identities identically.
 */
QString getDeckColorIdentity(const DeckList &deck);

#endif // COCKATRICE_DECK_COLOR_IDENTITY_H
