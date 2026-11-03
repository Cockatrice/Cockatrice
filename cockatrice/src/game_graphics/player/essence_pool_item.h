#ifndef CONTINUUMATRICE_ESSENCE_POOL_ITEM_H
#define CONTINUUMATRICE_ESSENCE_POOL_ITEM_H

#include <QGraphicsObject>
#include <QHash>
#include <QPointer>

class CounterState;
class PlayerLogic;
class QGraphicsSceneMouseEvent;
class QGraphicsSceneHoverEvent;
class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;

/**
 * An interactive, sixteen-type Essence resource pool attached to a player
 * battlefield. All values are read from the existing server-owned CounterState
 * objects; this graphics item does NOT own mutable match resource balances.
 *
 * Left click +1, right click -1, Shift+left to set an exact nonnegative amount.
 * Only the owning player or a game judge can modify the pool.
 */
class EssencePoolItem final : public QGraphicsObject
{
    Q_OBJECT

public:
    static constexpr int Width = 224;
    static constexpr int Height = 258;

    explicit EssencePoolItem(PlayerLogic *player, QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;

private:
    static constexpr int Columns = 4;
    static constexpr int Padding = 8;
    static constexpr int CellWidth = 52;
    static constexpr int CellHeight = 54;
    static constexpr int HeaderHeight = 29;

    [[nodiscard]] QChar codeAt(const QPointF &position) const;
    [[nodiscard]] CounterState *stateFor(QChar code) const;
    void attachCounter(CounterState *state);
    void detachCounter(int id);

    PlayerLogic *player;
    QHash<QChar, QPointer<CounterState>> counters;
};

#endif // CONTINUUMATRICE_ESSENCE_POOL_ITEM_H
