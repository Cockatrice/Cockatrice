#include "abstractcarddragitem.h"

#include "carddatabase.h"

#include <QCursor>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

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
    } else {
        if ((hotSpot.x() < 0) || (hotSpot.y() < 0)) {
            qDebug() << "CardDragItem: coordinate overflow: x =" << hotSpot.x() << ", y =" << hotSpot.y();
            hotSpot = QPointF();
        } else if ((hotSpot.x() > CARD_WIDTH) || (hotSpot.y() > CARD_HEIGHT)) {
            qDebug() << "CardDragItem: coordinate overflow: x =" << hotSpot.x() << ", y =" << hotSpot.y();
            hotSpot = QPointF(CARD_WIDTH, CARD_HEIGHT);
        }
        setCursor(Qt::ClosedHandCursor);
        setZValue(2000000007);
    }
    if (item->getTapped())
        setTransform(QTransform()
                         .translate(CARD_WIDTH_HALF, CARD_HEIGHT_HALF)
                         .rotate(90)
                         .translate(-CARD_WIDTH_HALF, -CARD_HEIGHT_HALF));

    setCacheMode(DeviceCoordinateCache);
}

AbstractCardDragItem::~AbstractCardDragItem()
{
    for (int i = 0; i < childDrags.size(); i++)
        delete childDrags[i];
}

QPainterPath AbstractCardDragItem::shape() const
{
    QPainterPath shape;
    shape.addRoundedRect(boundingRect(), 0.05 * CARD_WIDTH, 0.05 * CARD_WIDTH);
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
