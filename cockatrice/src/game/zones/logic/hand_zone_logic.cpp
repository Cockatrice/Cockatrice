#include "hand_zone_logic.h"

#include "../../board/card_item.h"

HandZoneLogic::HandZoneLogic(Player *_player,
                             const QString &_name,
                             bool _hasCardAttr,
                             bool _isShufflable,
                             bool _contentsKnown,
                             QObject *parent)
    : CardZoneLogic(_player, _name, _hasCardAttr, _isShufflable, _contentsKnown, parent)
{
}

void HandZoneLogic::addCardImpl(CardItem *card, int x, int /*y*/)
{
    // if x is negative set it to add at end
    if (x < 0 || x >= cards.size()) {
        x = cards.size();
    }
    cards.insert(x, card);

    if (!cards.getContentsKnown()) {
        card->setId(-1);
        card->setCardRef({});
    }
    card->resetState();
    card->setVisible(true);
}