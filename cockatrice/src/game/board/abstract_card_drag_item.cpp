#include "abstract_card_drag_item.h"

#include "../../client/settings/cache_settings.h"
#include "../z_values.h"

#include <QCursor>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

static const float CARD_WIDTH_HALF = CardDimensions::WIDTH / 2.0f;
static const float CARD_HEIGHT_HALF = CardDimensions::HEIGHT / 2.0f;
const QColor GHOST_MASK = QColor(255, 255, 255, 50);

AbstractCardDragItem::AbstractCardDragItem(AbstractCardItem *_item,
                                           const QPointF &_hotSpot,
                                           AbstractCardDragItem *parentDrag)
    : QGraphicsItem(), item(_item), hotSpot(_hotSpot)
{
    if (parentDrag) {
        parentDrag->addChildDrag(this);
        setZValue(ZValues::childDragZValue(hotSpot.x(), hotSpot.y()));
        connect(parentDrag, &QObject::destroyed, this, &AbstractCardDragItem::deleteLater);
    } else {
        hotSpot = QPointF{qBound(0.0, hotSpot.x(), static_cast<qreal>(CardDimensions::WIDTH - 1)),
                          qBound(0.0, hotSpot.y(), static_cast<qreal>(CardDimensions::HEIGHT - 1))};
        setCursor(Qt::ClosedHandCursor);
        setZValue(ZValues::DRAG_ITEM);
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
    qreal cardCornerRadius = SettingsCache::instance().getRoundCardCorners() ? 0.05 * CardDimensions::WIDTH : 0.0;
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
