/**
 * @file stack_zone_logic.h
 * @ingroup GameLogicZones
 * @brief TODO: Document this.
 */

#ifndef COCKATRICE_STACK_ZONE_LOGIC_H
#define COCKATRICE_STACK_ZONE_LOGIC_H
#include "card_zone_logic.h"

class StackZoneLogic : public CardZoneLogic
{
    Q_OBJECT
public:
    StackZoneLogic(Player *_player,
                   const QString &_name,
                   bool _hasCardAttr,
                   bool _isShufflable,
                   bool _contentsKnown,
                   QObject *parent = nullptr);

protected:
    void addCardImpl(CardItem *card, int x, int y) override;
};

#endif // COCKATRICE_STACK_ZONE_LOGIC_H
