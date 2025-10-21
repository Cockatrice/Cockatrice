#include "abstract_card_drag_item.h"

#include <QCursor>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <libcockatrice/settings/cache_settings.h>

static const float CARD_WIDTH_HALF = CARD_WIDTH / 2;
static const float CARD_HEIGHT_HALF = CARD_HEIGHT / 2;
const QColor GHOST_MASK = QColor(255, 255, 255, 50);

AbstractCardDragItem::AbstractCardDragItem(AbstractCardItem *_item,
                                           const QPointF &_hotSpot,
                                           AbstractCardDragItem *parentDrag)
    : QGraphicsItem(), item(_item), hotSpot(_hotSpot)
{
    if (parentDrag) {
        parentDrag->addChildDrag(this);
        setZValue(2000000007 + hotSpot.x() * 1000000 + hotSpot.y() * 1000 + 1000);
        connect(parentDrag, &QObject::destroyed, this, &AbstractCardDragItem::deleteLater);
    } else {
        hotSpot = QPointF{qBound(0.0, hotSpot.x(), static_cast<qreal>(CARD_WIDTH - 1)),
                          qBound(0.0, hotSpot.y(), static_cast<qreal>(CARD_HEIGHT - 1))};
        setCursor(Qt::ClosedHandCursor);
        setZValue(2000000007);
    }
    if (item->getTapped())
        setTransform(QTransform()
                         .translate(CARD_WIDTH_HALF, CARD_HEIGHT_HALF)
                         .rotate(90)
                         .translate(-CARD_WIDTH_HALF, -CARD_HEIGHT_HALF));

    setCacheMode(DeviceCoordinateCache);

    connect(&SettingsCache::instance(), &SettingsCache::roundCardCornersChanged, this, [this](bool _roundCardCorners) {
        Q_UNUSED(_roundCardCorners);

        prepareGeometryChange();
        update();
    });

    connect(item, &QObject::destroyed, this, &AbstractCardDragItem::deleteLater);
}

QPainterPath AbstractCardDragItem::shape() const
{
    QPainterPath shape;
    qreal cardCornerRadius = SettingsCache::instance().getRoundCardCorners() ? 0.05 * CARD_WIDTH : 0.0;
    shape.addRoundedRect(boundingRect(), cardCornerRadius, cardCornerRadius);
    return shape;
}

void AbstractCardDragItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    item->paint(painter, option, widget);

    // adds a mask to the card so it looks like the card hasnt been placed yet
    painter->fillPath(shape(), GHOST_MASK);
}

void AbstractCardDragItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
    updatePosition(event->scenePos());
}

void AbstractCardDragItem::addChildDrag(AbstractCardDragItem *child)
{
    childDrags << child;
}
