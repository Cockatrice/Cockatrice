#ifndef COCKATRICE_SAMPLE_HAND_GENERATOR_H
#define COCKATRICE_SAMPLE_HAND_GENERATOR_H

#include <QList>
#include <libcockatrice/card/printing/exact_card.h>
#include <random>

class CardDatabaseQuerier;
class DeckList;

// Sample physical copies without expanding the deck. Unknown cards retain their names as placeholders.
QList<ExactCard>
generateSampleHand(const DeckList &deck, const CardDatabaseQuerier &query, int handSize, std::mt19937 &random);

#endif
