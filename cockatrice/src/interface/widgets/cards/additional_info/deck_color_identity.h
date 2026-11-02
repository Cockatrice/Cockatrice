#ifndef COCKATRICE_DECK_COLOR_IDENTITY_H
#define COCKATRICE_DECK_COLOR_IDENTITY_H

#include <QString>

class CardDatabaseQuerier;
class DeckList;

/**
 * @brief Computes the Essence identity of a deck from the defined Essence
 *        symbols (M,S,O,G,E,A,Q,C,B,R,L,V,T,P,N) of cards in main/sideboard.
 *        N denotes colorless.
 *
 * Shared as a free function so the deck storage previews and the deck share
 * dialog compute identities identically.
 *
 * @param db Card database used to look up card color symbols.
 */
QString getDeckColorIdentity(const DeckList &deck, const CardDatabaseQuerier *db);

#endif // COCKATRICE_DECK_COLOR_IDENTITY_H
