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
 * @brief Size constants for commander tax counter layout.
 */
namespace TaxCounterSizes
{

/** @brief Size of commander tax counter icons (width and height) */
constexpr int TAX_COUNTER_SIZE = 24;

/** @brief Margin around and between tax counter icons */
constexpr int TAX_COUNTER_MARGIN = 2;

} // namespace TaxCounterSizes

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
    int size;

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
