#include "pile_zone_logic.h"

#include "../../board/card_item.h"

PileZoneLogic::PileZoneLogic(Player *_player,
                             const QString &_name,
                             bool _hasCardAttr,
                             bool _isShufflable,
                             bool _contentsKnown,
                             QObject *parent)
    : CardZoneLogic(_player, _name, _hasCardAttr, _isShufflable, _contentsKnown, parent)
{
}

void PileZoneLogic::addCardImpl(CardItem *card, int x, int /*y*/)
{
    connect(card, &CardItem::sigPixmapUpdated, this, &PileZoneLogic::callUpdate);
    // if x is negative set it to add at end
    if (x < 0 || x >= cards.size()) {
        x = cards.size();
    }
    cards.insert(x, card);
    card->setPos(0, 0);
    if (!contentsKnown()) {
        card->setCardRef({});
        card->setId(-1);
        // If we obscure a previously revealed card, its name has to be forgotten
        if (cards.size() > x + 1)
            cards.at(x + 1)->setCardRef({});
    }
    card->setVisible(false);
    card->resetState();
}