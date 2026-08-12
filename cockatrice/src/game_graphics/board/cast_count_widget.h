/**
 * @file cast_count_widget.h
 * @ingroup GameGraphicsBoard
 * @brief Displays a single cast count overlay counter in the command zone.
 */

#ifndef COCKATRICE_CAST_COUNT_WIDGET_H
#define COCKATRICE_CAST_COUNT_WIDGET_H

#include "abstract_counter.h"

namespace CastCountSizes
{
constexpr int MARGIN = 2;
} // namespace CastCountSizes

/**
 * @class CastCountWidget
 * @brief A compact counter widget for tracking cast counts in the command zone.
 */
class CastCountWidget : public AbstractCounter
{
    Q_OBJECT
private:
    int size;

public:
    CastCountWidget(CounterState *state, PlayerLogic *player, QGraphicsItem *parent = nullptr);

    [[nodiscard]] QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void setValue(int _value) override;
};

#endif // COCKATRICE_CAST_COUNT_WIDGET_H
