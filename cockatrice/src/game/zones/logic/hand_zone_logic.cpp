#include "hand_zone_logic.h"

#include "../../board/card_item.h"
#include "card_zone_algorithms.h"

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
    CardZoneAlgorithms::addCardToList(cards, card, x, false);
}
