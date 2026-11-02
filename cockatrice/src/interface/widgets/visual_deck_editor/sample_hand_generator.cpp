#include "sample_hand_generator.h"

#include <algorithm>
#include <cmath>
#include <libcockatrice/card/database/card_database_querier.h>
#include <libcockatrice/deck_list/deck_list.h>
#include <libcockatrice/deck_list/tree/deck_list_card_node.h>
#include <libcockatrice/deck_list/tree/inner_deck_list_node.h>
#include <limits>

namespace
{
struct RemainingCard
{
    const DecklistCardNode *node;
    qint64 copies;
};

double manaValue(const ExactCard &card)
{
    bool valid = false;
    const double value = card.getInfo().getCmc().toDouble(&valid);
    return valid && std::isfinite(value) && value >= 0 ? value : std::numeric_limits<double>::infinity();
}
} // namespace

QList<ExactCard>
generateSampleHand(const DeckList &deck, const CardDatabaseQuerier &query, int handSize, std::mt19937 &random)
{
    QList<ExactCard> hand;
    if (handSize <= 0) {
        return hand;
    }

    QList<RemainingCard> remainingCards;
    qint64 totalCopies = 0;
    for (const auto *node : deck.getCardNodes({DECK_ZONE_MAIN})) {
        if (node->getNumber() > 0) {
            remainingCards.append({node, node->getNumber()});
            totalCopies += node->getNumber();
        }
    }

    const int cardsToDraw = static_cast<int>(std::min<qint64>(handSize, totalCopies));
    hand.reserve(cardsToDraw);
    for (int drawn = 0; drawn < cardsToDraw; ++drawn) {
        // Every remaining physical copy gets one position in this range. Removing just the
        // selected copy makes later draws equivalent to drawing from a shuffled physical deck.
        qint64 position = std::uniform_int_distribution<qint64>(0, totalCopies - 1)(random);
        for (auto &entry : remainingCards) {
            if (position >= entry.copies) {
                position -= entry.copies;
                continue;
            }
            auto card = query.getCard(entry.node->toCardRef());
            if (!card) {
                card = ExactCard(CardInfo::newInstance(entry.node->getName()));
            }
            hand.append(card);
            --entry.copies;
            --totalCopies;
            break;
        }
    }

    // Keep the existing mana-value presentation, using numeric values (2 before 10).
    // Cards without mana metadata sort last and stay identifiable by name.
    std::stable_sort(hand.begin(), hand.end(),
                     [](const ExactCard &a, const ExactCard &b) { return manaValue(a) < manaValue(b); });
    return hand;
}
