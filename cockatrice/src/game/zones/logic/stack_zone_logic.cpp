#include "stack_zone_logic.h"

#include "../../board/card_item.h"

StackZoneLogic::StackZoneLogic(Player *_player,
                               const QString &_name,
                               bool _hasCardAttr,
                               bool _isShufflable,
                               bool _contentsKnown,
                               QObject *parent)
    : CardZoneLogic(_player, _name, _hasCardAttr, _isShufflable, _contentsKnown, parent)
{
}

void StackZoneLogic::addCardImpl(CardItem *card, int x, int /*y*/)
{
    // if x is negative set it to add at end
    if (x < 0 || x >= cards.size()) {
        x = static_cast<int>(cards.size());
    }
    cards.insert(x, card);

    if (!cards.getContentsKnown()) {
        card->setId(-1);
        card->setCardRef({});
    }
    card->resetState(true);
    card->setVisible(true);
}