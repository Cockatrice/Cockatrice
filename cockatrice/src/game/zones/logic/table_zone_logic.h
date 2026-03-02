/**
 * @file table_zone_logic.h
 * @ingroup GameLogicZones
 * @brief TODO: Document this.
 */

#ifndef COCKATRICE_TABLE_ZONE_LOGIC_H
#define COCKATRICE_TABLE_ZONE_LOGIC_H
#include "card_zone_logic.h"

class TableZoneLogic : public CardZoneLogic
{
    Q_OBJECT
signals:
    void contentSizeChanged();
    void toggleTapped();

public:
    TableZoneLogic(Player *_player,
                   const QString &_name,
                   bool _hasCardAttr,
                   bool _isShufflable,
                   bool _contentsKnown,
                   QObject *parent = nullptr);

protected:
    void addCardImpl(CardItem *card, int x, int y) override;

    /**
     *  @brief Removes a card from view.
     *
     *  @param position card position
     *  @param cardId id of card to take
     *  @param toNewZone Whether the destination of the card is not the same as the starting zone. Defaults to true
     *  @return CardItem that has been removed
     */
    CardItem *takeCard(int position, int cardId, bool toNewZone = true) override;
};

#endif // COCKATRICE_TABLE_ZONE_LOGIC_H
