#ifndef COCKATRICE_CARD_ZONE_ALGORITHMS_H
#define COCKATRICE_CARD_ZONE_ALGORITHMS_H

namespace CardZoneAlgorithms
{

/**
 * Shared insertion logic for zones where cards become visible on add and follow
 * the standard pattern: clamp index, insert, clear identity if contents unknown,
 * reset state, show card.
 *
 * Zones with different post-add behavior (signal connections, positional resets,
 * hidden cards, or coordinate-based placement) should NOT use this — implement
 * addCardImpl directly instead.
 *
 * Template parameters allow testing with lightweight mocks that avoid Qt graphics
 * dependencies.
 *
 * @tparam CardList Must provide: size() -> int, insert(int, CardType*),
 *                  getContentsKnown() -> bool
 * @tparam CardType Must provide: setId(int), setCardRef(CardRefType),
 *                  resetState(bool), setVisible(bool)
 * @param keepAnnotations Forwarded to card->resetState(). Stack-like zones preserve
 *                        annotations across zone transitions; hand-like zones clear them.
 */
template <typename CardList, typename CardType>
void addCardToList(CardList &cards, CardType *card, int x, bool keepAnnotations)
{
    if (x < 0 || x >= cards.size()) {
        x = static_cast<int>(cards.size());
    }
    cards.insert(x, card);

    if (!cards.getContentsKnown()) {
        card->setId(-1);
        card->setCardRef({});
    }

    card->resetState(keepAnnotations);
    card->setVisible(true);
}

} // namespace CardZoneAlgorithms

#endif // COCKATRICE_CARD_ZONE_ALGORITHMS_H
