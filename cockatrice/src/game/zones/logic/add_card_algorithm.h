#ifndef ADD_CARD_ALGORITHM_H
#define ADD_CARD_ALGORITHM_H

/**
 * @file add_card_algorithm.h
 * @brief Shared algorithm for card insertion logic.
 *
 * This template is used by both production (CommandZoneLogic) and test code
 * to ensure consistent behavior and eliminate duplication.
 */

namespace CardZoneAlgorithms
{

/**
 * @brief Core logic for adding a card to a zone's card list.
 *
 * @tparam CardList List type supporting size(), insert(), getContentsKnown()
 * @tparam CardType Card type supporting setId(), setCardRef(), resetState(), setVisible()
 * @param cards The card list to insert into
 * @param card The card to insert
 * @param x Target position (negative or out-of-bounds appends to end)
 */
template <typename CardList, typename CardType> void addCardToList(CardList &cards, CardType *card, int x)
{
    // Normalize position: negative or beyond range appends to end
    if (x < 0 || x >= cards.size()) {
        x = static_cast<int>(cards.size());
    }
    cards.insert(x, card);

    // Handle unknown contents
    if (!cards.getContentsKnown()) {
        card->setId(-1);
        card->setCardRef({});
    }

    card->resetState(true);
    card->setVisible(true);
}

} // namespace CardZoneAlgorithms

#endif // ADD_CARD_ALGORITHM_H
