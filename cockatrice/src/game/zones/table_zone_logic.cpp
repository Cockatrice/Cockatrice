#include "table_zone_logic.h"

#include "../board/card_state.h"

TableZoneLogic::TableZoneLogic(PlayerLogic *_player,
                               const QString &_name,
                               bool _hasCardAttr,
                               bool _isShufflable,
                               bool _contentsKnown,
                               QObject *parent)
    : CardZoneLogic(_player, _name, _hasCardAttr, _isShufflable, _contentsKnown, parent)
{
}

void TableZoneLogic::addCardImpl(CardState *card, int _x, int _y)
{
    cards.append(card);
    if (!card->getFaceDown() && card->getPT().isEmpty()) {
        card->setPT(card->getCardInfo().getPowTough());
    }
    if (card->getCardInfo().getUiAttributes().cipt && card->getCardInfo().getUiAttributes().landscapeOrientation) {
        card->setDoesntUntap(true);
    }
    card->setGridPoint(QPoint(_x, _y));
}

CardState *TableZoneLogic::takeCard(int position, int cardId, bool toNewZone)
{
    CardState *result = CardZoneLogic::takeCard(position, cardId);

    if (toNewZone) {
        emit contentSizeChanged();
    }
    return result;
}

int TableZoneLogic::clampValidTableRow(const int row)
{
    if (row < 0) {
        return 0;
    }
    if (row >= ROW_COUNT) {
        return ROW_COUNT - 1;
    }
    return row;
}

int TableZoneLogic::tableRowToGridY(int tableRow)
{
    if (tableRow > 2) {
        tableRow = 1;
    }
    return clampValidTableRow(2 - tableRow);
}
