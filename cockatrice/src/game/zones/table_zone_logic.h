/**
 * @file table_zone_logic.h
 * @ingroup GameLogicZones
 */
//! \todo Document this file.

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
    TableZoneLogic(PlayerLogic *_player,
                   const QString &_name,
                   bool _hasCardAttr,
                   bool _isShufflable,
                   bool _contentsKnown,
                   QObject *parent = nullptr);

    /** @brief Number of rows in the table zone grid (0=creatures, 1=noncreatures, 2=lands). */
    static const int ROW_COUNT = 3;

    /**
     * Clamps a grid row index into [0, ROW_COUNT - 1].
     */
    static int clampValidTableRow(const int row);

    /**
     * Converts a card's logical table row (0=creatures, 1=noncreatures, 2=lands)
     * to the corresponding grid Y coordinate. Cards with tableRow > 2 (e.g.,
     * instants/sorceries) default to the noncreatures row.
     */
    static int tableRowToGridY(int tableRow);

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
