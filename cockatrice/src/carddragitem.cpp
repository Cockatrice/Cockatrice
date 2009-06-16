#include "carddragitem.h"
#include "cardzone.h"
#include "carddatabase.h"
#include <QtGui>

CardDragItem::CardDragItem(CardItem *_item, int _id, const QPointF &_hotSpot, bool _faceDown, CardDragItem *parentDrag)
	: QGraphicsItem(), id(_id), item(_item), hotSpot(_hotSpot), faceDown(_faceDown)
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

void CardDragItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	event->accept();
	QPointF sp = event->scenePos();
	QList<QGraphicsItem *> colliding = scene()->items(sp);

	CardZone *cursorZone = 0;
	for (int i = colliding.size() - 1; i >= 0; i--)
		if ((cursorZone = qgraphicsitem_cast<CardZone *>(colliding.at(i))))
			break;
	
	QPointF newPos;
	if (!cursorZone)
		return;
	else if (cursorZone->getName() == "table") {
		QPointF cp = cursorZone->scenePos();
		QPointF localpos = sp - hotSpot - cp;
		
		newPos = QPointF(round(localpos.x() / RASTER_WIDTH) * RASTER_WIDTH, round(localpos.y() / RASTER_HEIGHT) * RASTER_HEIGHT) + cp;
	} else
		newPos = sp - hotSpot;
	if (newPos != pos()) {
		for (int i = 0; i < childDrags.size(); i++)
			childDrags[i]->setPos(newPos + childDrags[i]->getHotSpot());
//		qDebug(QString("setPos: x=%1, y=%2").arg(newPos.x()).arg(newPos.y()).toLatin1());
		setPos(newPos);
	}
}

void CardDragItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	setCursor(Qt::OpenHandCursor);
	QGraphicsScene *sc = scene();
	QPointF sp = pos();
	qDebug(QString("sp: x=%1, y=%2").arg(sp.x()).arg(sp.y()).toLatin1());
	sc->removeItem(this);
	QList<QGraphicsItem *> colliding = sc->items(event->scenePos());

//	qDebug(QString("drop: %1 collisions").arg(colliding.size()).toLatin1());
	CardZone *dropZone = 0;
	for (int i = colliding.size() - 1; i >= 0; i--) {
		QRectF bbox = colliding.at(i)->boundingRect();
//		qDebug(QString("bbox x %1 y %2 w %3 h %4").arg(bbox.x()).arg(bbox.y()).arg(bbox.width()).arg(bbox.height()).toLatin1());

		if ((dropZone = qgraphicsitem_cast<CardZone *>(colliding.at(i)))) {
//			qDebug("zone found");
			break;
		}
	}

	if (dropZone) {
		CardZone *startZone = qgraphicsitem_cast<CardZone *>(item->parentItem());
		dropZone->handleDropEvent(id, startZone, (sp - dropZone->scenePos()).toPoint(), faceDown);
		for (int i = 0; i < childDrags.size(); i++) {
			CardDragItem *c = childDrags[i];
			dropZone->handleDropEvent(c->id, startZone, (sp - dropZone->scenePos() + c->getHotSpot()).toPoint(), faceDown);
			sc->removeItem(c);
		}
	}

	event->accept();
}

void CardDragItem::addChildDrag(CardDragItem *child)
{
	childDrags << child;
}
