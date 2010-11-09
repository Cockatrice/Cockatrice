#include "abstractcarddragitem.h"
#include "carddatabase.h"
#include <QCursor>
#include <QGraphicsSceneMouseEvent>

AbstractCardDragItem::AbstractCardDragItem(AbstractCardItem *_item, const QPointF &_hotSpot, AbstractCardDragItem *parentDrag)
	: QGraphicsItem(), item(_item), hotSpot(_hotSpot)
{
	if (parentDrag) {
		parentDrag->addChildDrag(this);
		setZValue(2000000007 + hotSpot.x() * 1000000 + hotSpot.y() * 1000 + 1000);
	} else {
		if ((hotSpot.x() < 0) || (hotSpot.y() < 0)) {
			qDebug(QString("CardDragItem: coordinate overflow: x = %1, y = %2").arg(hotSpot.x()).arg(hotSpot.y()).toLatin1());
			hotSpot = QPointF();
		} else if ((hotSpot.x() > CARD_WIDTH) || (hotSpot.y() > CARD_HEIGHT)) {
			qDebug(QString("CardDragItem: coordinate overflow: x = %1, y = %2").arg(hotSpot.x()).arg(hotSpot.y()).toLatin1());
			hotSpot = QPointF(CARD_WIDTH, CARD_HEIGHT);
		}
		setCursor(Qt::ClosedHandCursor);
		setZValue(2000000007);
	}
	if (item->getTapped())
		setTransform(QTransform().translate((float) CARD_WIDTH / 2, (float) CARD_HEIGHT / 2).rotate(90).translate((float) -CARD_WIDTH / 2, (float) -CARD_HEIGHT / 2));

	setCacheMode(DeviceCoordinateCache);
}

AbstractCardDragItem::~AbstractCardDragItem()
{
	qDebug("CardDragItem destructor");
	for (int i = 0; i < childDrags.size(); i++)
		delete childDrags[i];
}

void AbstractCardDragItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	item->paint(painter, option, widget);
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
