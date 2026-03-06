#include "abstract_card_drag_item.h"

#include "../../client/settings/cache_settings.h"
#include "../z_values.h"

#include <QCursor>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

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
        hotSpot = QPointF{qBound(0.0, hotSpot.x(), CardDimensions::WIDTH_F - 1),
                          qBound(0.0, hotSpot.y(), CardDimensions::HEIGHT_F - 1)};
        setCursor(Qt::ClosedHandCursor);
        setZValue(ZValues::DRAG_ITEM);
    }
    if (item->getTapped())
        setTransform(QTransform()
                         .translate(CardDimensions::WIDTH_HALF_F, CardDimensions::HEIGHT_HALF_F)
                         .rotate(90)
                         .translate(-CardDimensions::WIDTH_HALF_F, -CardDimensions::HEIGHT_HALF_F));

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
    qreal cardCornerRadius = SettingsCache::instance().getRoundCardCorners() ? 0.05 * CardDimensions::WIDTH_F : 0.0;
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
