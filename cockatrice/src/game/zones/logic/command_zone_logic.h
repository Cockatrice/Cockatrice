/**
 * @file command_zone_logic.h
 * @ingroup GameLogicZones
 * @brief Logic layer for the command zone, used for Commander format.
 */

#ifndef COCKATRICE_COMMAND_ZONE_LOGIC_H
#define COCKATRICE_COMMAND_ZONE_LOGIC_H
#include "card_zone_logic.h"

/**
 * @class CommandZoneLogic
 * @brief Logic layer for managing cards in the command zone.
 *
 * Handles data storage and card management for command zones in Commander format.
 * Supports ordered card insertion for drag-and-drop operations.
 *
 * @see CommandZone for the graphics layer
 * @see CardZoneLogic
 */
class CommandZoneLogic : public CardZoneLogic
{
    Q_OBJECT
public:
    /**
     * @brief Constructs a CommandZoneLogic instance.
     * @param _player The player who owns this zone
     * @param _name Zone name ("command" or "partner")
     * @param _hasCardAttr Whether cards in this zone have attributes
     * @param _isShufflable Whether the zone can be shuffled
     * @param _contentsKnown Whether the zone contents are public knowledge
     * @param parent Parent QObject
     */
    CommandZoneLogic(Player *_player,
                     const QString &_name,
                     bool _hasCardAttr,
                     bool _isShufflable,
                     bool _contentsKnown,
                     QObject *parent = nullptr);

protected:
    /**
     * @brief Adds a card at position x (y ignored). Appends if x is -1 or out of range.
     * @param card Card to add
     * @param x Insertion index, or -1 to append
     * @param y Unused
     */
    void addCardImpl(CardItem *card, int x, int y) override;
};

#endif // COCKATRICE_COMMAND_ZONE_LOGIC_H
