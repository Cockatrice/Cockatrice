#include "table_zone_logic.h"

#include "../../board/card_item.h"

TableZoneLogic::TableZoneLogic(Player *_player,
                               const QString &_name,
                               bool _hasCardAttr,
                               bool _isShufflable,
                               bool _contentsKnown,
                               QObject *parent)
    : CardZoneLogic(_player, _name, _hasCardAttr, _isShufflable, _contentsKnown, parent)
{
}

void TableZoneLogic::addCardImpl(CardItem *card, int _x, int _y)
{
    cards.append(card);
    if (!card->getFaceDown() && card->getPT().isEmpty()) {
        card->setPT(card->getCardInfo().getPowTough());
    }
    if (card->getCardInfo().getUiAttributes().cipt && card->getCardInfo().getUiAttributes().landscapeOrientation) {
        card->setDoesntUntap(true);
    }
    card->setGridPoint(QPoint(_x, _y));
    card->setVisible(true);
}

CardItem *TableZoneLogic::takeCard(int position, int cardId, bool toNewZone)
{
    CardItem *result = CardZoneLogic::takeCard(position, cardId);

    if (toNewZone)
        emit contentSizeChanged();
    return result;
}