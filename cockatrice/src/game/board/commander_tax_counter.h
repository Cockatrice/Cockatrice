/**
 * @file commander_tax_counter.h
 * @ingroup GameGraphicsPlayers
 * @brief Square counter for commander tax, clamped to non-negative values.
 */

#ifndef COCKATRICE_COMMANDER_TAX_COUNTER_H
#define COCKATRICE_COMMANDER_TAX_COUNTER_H

#include "abstract_counter.h"

/**
 * @class CommanderTaxCounter
 * @brief Counter for tracking commander tax in Commander format.
 *
 * Displays cumulative cost increase for casting a commander. The counter
 * is manually adjusted by the player to track their commander tax. Values
 * are clamped to >= 0.
 *
 * Appearance: square with rounded corners, semi-transparent background,
 * positioned at top-left of command zone.
 *
 * Two instances per player: CounterIds::CommanderTax and CounterIds::PartnerTax.
 *
 * @see AbstractCounter
 * @see CounterIds
 */
class CommanderTaxCounter : public AbstractCounter
{
    Q_OBJECT
private:
    int size;

public:
    /**
     * @brief Constructs a CommanderTaxCounter.
     * @param _player The player who owns this counter
     * @param _id Counter ID (CounterIds::CommanderTax or CounterIds::PartnerTax)
     * @param _name Display name for the counter
     * @param _size Size in pixels (both width and height, as the counter is square)
     * @param _value Initial value for the counter
     * @param parent Parent graphics item (typically the command zone)
     */
    CommanderTaxCounter(Player *_player,
                        int _id,
                        const QString &_name,
                        int _size,
                        int _value,
                        QGraphicsItem *parent = nullptr);

    [[nodiscard]] QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    /// @brief Sets counter value, clamping to >= 0.
    /// @param _value New value (clamped if negative)
    void setValue(int _value) override;
};

#endif // COCKATRICE_COMMANDER_TAX_COUNTER_H
