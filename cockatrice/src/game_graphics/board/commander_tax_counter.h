/**
 * @file commander_tax_counter.h
 * @ingroup GameGraphicsPlayers
 * @brief Square counter for commander tax, clamped to non-negative values.
 */

#ifndef COCKATRICE_COMMANDER_TAX_COUNTER_H
#define COCKATRICE_COMMANDER_TAX_COUNTER_H

#include "abstract_counter.h"

/**
 * @namespace TaxCounterSizes
 * @brief Layout constants for commander tax counters.
 */
namespace TaxCounterSizes
{

/** @brief Margin around and between tax counter icons */
constexpr int TAX_COUNTER_MARGIN = 2;

} // namespace TaxCounterSizes

/**
 * @class CommanderTaxCounter
 * @brief Counter for tracking commander tax in Commander format.
 *
 * Displays the number of times the commander has been cast from the command
 * zone. Can be adjusted manually via +1/-1 menu actions, or automatically
 * incremented when using "Play and Increase Tax" on an accepted cast from
 * the command zone. Values are clamped to >= 0.
 *
 * Appearance: square with rounded corners, semi-transparent background,
 * positioned at top-left of command zone.
 *
 * Two instances per player: CounterIds::CommanderTax and CounterIds::PartnerTax.
 * Each counter supports an active/inactive state (inherited from AbstractCounter):
 * commander tax starts active; partner tax starts inactive until explicitly
 * enabled by the player via the context menu.
 *
 * @see AbstractCounter
 * @see AbstractCounter::setActive()
 * @see CounterIds
 */
class CommanderTaxCounter : public AbstractCounter
{
    Q_OBJECT
private:
    int size; ///< Width and height of the counter in pixels

public:
    /**
     * @brief Constructs a CommanderTaxCounter.
     * @param state Counter state containing id, name, value, etc.
     * @param player The player who owns this counter
     * @param parent Parent graphics item (typically the command zone)
     */
    CommanderTaxCounter(CounterState *state, PlayerLogic *player, QGraphicsItem *parent = nullptr);

    [[nodiscard]] QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    /**
     * @brief Overrides AbstractCounter::setValue to clamp values to >= 0 and update the tooltip.
     * @param _value New value (clamped if negative)
     */
    void setValue(int _value) override;
};

#endif // COCKATRICE_COMMANDER_TAX_COUNTER_H
