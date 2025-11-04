#include "view_zone_logic.h"

#include "../../../client/settings/cache_settings.h"
#include "../../board/card_item.h"

/**
 * @param _player the player that the cards are revealed to.
 * @param _origZone the zone the cards were revealed from.
 * @param _revealZone if false, the cards will be face down.
 * @param _writeableRevealZone whether the player can interact with the revealed cards.
 */
ZoneViewZoneLogic::ZoneViewZoneLogic(Player *_player,
                                     CardZoneLogic *_origZone,
                                     int _numberCards,
                                     bool _revealZone,
                                     bool _writeableRevealZone,
                                     bool _isReversed,
                                     QObject *parent)
    : CardZoneLogic(_player, _origZone->getName(), false, false, true, parent), origZone(_origZone),
      numberCards(_numberCards), revealZone(_revealZone), writeableRevealZone(_writeableRevealZone),
      isReversed(_isReversed)
{
}

/**
 * Checks if inserting a card at the given position requires an actual new card to be created and added to the view.
 * Also does any cardId updates that would be required if a card is inserted in that position.
 *
 * Note that this method can end up modifying the cardIds despite returning false.
 * (for example, if the card is inserted into a hidden portion of the deck while the view is reversed)
 *
 * Make sure to call this method once before calling addCard(), so that you skip creating a new CardItem and calling
 * addCard() if it's not required.
 *
 * @param x The position to insert the card at.
 * @return Whether to proceed with calling addCard.
 */
bool ZoneViewZoneLogic::prepareAddCard(int x)
{
    bool doInsert = false;
    if (!isReversed) {
        if (x <= cards.size() || cards.size() == -1) {
            doInsert = true;
        }
    } else {
        // map x (which is in origZone indexes) to this viewZone's cardList index
        int firstId = cards.isEmpty() ? origZone->getCards().size() : cards.front()->getId();
        int insertionIndex = x - firstId;
        if (insertionIndex >= 0) {
            // card was put into a portion of the deck that's in the view
            doInsert = true;
        } else {
            // card was put into a portion of the deck that's not in the view; update ids but don't insert card
            updateCardIds(ADD_CARD);
        }
    }

    // autoclose check is done both here and in removeCard

    if (cards.isEmpty() && !doInsert && SettingsCache::instance().getCloseEmptyCardView()) {
        emit closeView();
    }

    return doInsert;
}

/**
 * Make sure prepareAddCard() was called before calling addCard().
 * This method assumes we already checked that the card is being inserted into the visible portion
 */
void ZoneViewZoneLogic::addCardImpl(CardItem *card, int x, int /*y*/)
{
    if (!isReversed) {
        // if x is negative set it to add at end
        // if x is out-of-bounds then also set it to add at the end
        if (x < 0 || x >= cards.size()) {
            x = cards.size();
        }
        cards.insert(x, card);
    } else {
        // map x (which is in origZone indexes) to this viewZone's cardList index
        int firstId = cards.isEmpty() ? origZone->getCards().size() : cards.front()->getId();
        int insertionIndex = x - firstId;
        // qMin to prevent out-of-bounds error when bottoming a card that is already in the view
        cards.insert(qMin(insertionIndex, cards.size()), card);
    }

    updateCardIds(ADD_CARD);
    reorganizeCards();
}

void ZoneViewZoneLogic::updateCardIds(CardAction action)
{
    if (origZone->contentsKnown()) {
        return;
    }

    if (cards.isEmpty()) {
        return;
    }

    int cardCount = cards.size();

    auto startId = 0;

    if (isReversed) {
        // the card has not been added to origZone's cardList at this point
        startId = origZone->getCards().size() - cardCount;
        switch (action) {
            case INITIALIZE:
                break;
            case ADD_CARD:
                startId += 1;
                break;
            case REMOVE_CARD:
                startId -= 1;
                break;
        }
    }

    for (int i = 0; i < cardCount; ++i) {
        cards[i]->setId(i + startId);
    }
}

void ZoneViewZoneLogic::removeCard(int position, bool toNewZone)
{
    if (isReversed) {
        position -= cards.first()->getId();
        if (position < 0 || position >= cards.size()) {
            updateCardIds(REMOVE_CARD);
            return;
        }
    }

    if (position >= cards.size()) {
        return;
    }

    CardItem *card = cards.takeAt(position);
    card->deleteLater();

    // The toNewZone check is to prevent the view from auto-closing if the view contains only a single card and that
    // card gets dragged within the view.
    // Another autoclose check is done in prepareAddCard so that the view autocloses if the last card was moved to an
    // unrevealed portion of the same zone.
    if (cards.isEmpty() && SettingsCache::instance().getCloseEmptyCardView() && toNewZone) {
        emit closeView();
        return;
    }

    updateCardIds(REMOVE_CARD);
    reorganizeCards();
}

void ZoneViewZoneLogic::setWriteableRevealZone(bool _writeableRevealZone)
{

    if (writeableRevealZone && !_writeableRevealZone) {
        emit addToViews();
    } else if (!writeableRevealZone && _writeableRevealZone) {
        emit removeFromViews();
    }
    writeableRevealZone = _writeableRevealZone;
}