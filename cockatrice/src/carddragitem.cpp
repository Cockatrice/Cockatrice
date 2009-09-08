#include "carddragitem.h"
#include "cardzone.h"
#include "carddatabase.h"
#include "tablezone.h"
#include <QtGui>

CardDragItem::CardDragItem(CardItem *_item, int _id, const QPointF &_hotSpot, bool _faceDown, CardDragItem *parentDrag)
	: QGraphicsItem(), id(_id), item(_item), hotSpot(_hotSpot), faceDown(_faceDown), currentZone(0)
{
	if (parentDrag)
		parentDrag->addChildDrag(this);
	else {
		if ((hotSpot.x() < 0) || (hotSpot.y() < 0)) {
			qDebug(QString("CardDragItem: coordinate overflow: x = %1, y = %2").arg(hotSpot.x()).arg(hotSpot.y()).toLatin1());
			hotSpot = QPointF();
		} else if ((hotSpot.x() > CARD_WIDTH) || (hotSpot.y() > CARD_HEIGHT)) {
			qDebug(QString("CardDragItem: coordinate overflow: x = %1, y = %2").arg(hotSpot.x()).arg(hotSpot.y()).toLatin1());
			hotSpot = QPointF(CARD_WIDTH, CARD_HEIGHT);
		}
		setCursor(Qt::ClosedHandCursor);
	}
	if (item->getTapped())
		setTransform(QTransform().translate((float) CARD_WIDTH / 2, (float) CARD_HEIGHT / 2).rotate(90).translate((float) -CARD_WIDTH / 2, (float) -CARD_HEIGHT / 2));

	setZValue(2000000000);
	setCacheMode(DeviceCoordinateCache);
}

CardDragItem::~CardDragItem()
{
	qDebug("CardDragItem destructor");
	for (int i = 0; i < childDrags.size(); i++)
		delete childDrags[i];
}

void CardDragItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	item->paint(painter, option, widget);
}

void CardDragItem::updatePosition(const QPointF &cursorScenePos)
{
	QList<QGraphicsItem *> colliding = scene()->items(cursorScenePos);

	CardZone *cursorZone = 0;
	for (int i = colliding.size() - 1; i >= 0; i--)
		if ((cursorZone = qgraphicsitem_cast<CardZone *>(colliding.at(i))))
			break;
	if (!cursorZone)
		return;
	currentZone = cursorZone;
	
	QPointF zonePos = currentZone->scenePos();
	QPointF cursorPosInZone = cursorScenePos - zonePos;
	QPointF cardTopLeft = cursorPosInZone - hotSpot;
//	QPointF cardCenter = cardTopLeft + QPointF(CARD_WIDTH / 2, CARD_HEIGHT / 2);
	QPointF newPos = zonePos + cursorZone->closestGridPoint(cardTopLeft + QPoint(CARD_WIDTH / 2, CARD_HEIGHT / 2));
	
//	qDebug(QString("cardTopLeft = %1, %2   cardCenter = %3, %4").arg((cardTopLeft).x()).arg((cardTopLeft).y()).arg(cardCenter.x()).arg(cardCenter.y()).toLatin1());

	if (newPos != pos()) {
		for (int i = 0; i < childDrags.size(); i++)
			childDrags[i]->setPos(newPos + childDrags[i]->getHotSpot());
		setPos(newPos);
	}
}

void CardDragItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	event->accept();
	updatePosition(event->scenePos());
}

void CardDragItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	setCursor(Qt::OpenHandCursor);
	QGraphicsScene *sc = scene();
	QPointF sp = pos();
	qDebug(QString("sp: x=%1, y=%2").arg(sp.x()).arg(sp.y()).toLatin1());
	sc->removeItem(this);

	if (currentZone) {
		CardZone *startZone = qgraphicsitem_cast<CardZone *>(item->parentItem());
		currentZone->handleDropEvent(id, startZone, (sp - currentZone->scenePos()).toPoint(), faceDown);
		for (int i = 0; i < childDrags.size(); i++) {
			CardDragItem *c = childDrags[i];
			currentZone->handleDropEvent(c->id, startZone, (sp - currentZone->scenePos() + c->getHotSpot()).toPoint(), faceDown);
			sc->removeItem(c);
		}
	}

	event->accept();
}

void CardDragItem::addChildDrag(CardDragItem *child)
{
	childDrags << child;
}
