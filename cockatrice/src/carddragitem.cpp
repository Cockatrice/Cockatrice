#include "carddragitem.h"
#include "cardzone.h"
#include "tablezone.h"
#include "zoneviewzone.h"
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QCursor>
#include <QPainter>

CardDragItem::CardDragItem(CardItem *_item, int _id, const QPointF &_hotSpot, bool _faceDown, AbstractCardDragItem *parentDrag)
	: AbstractCardDragItem(_item, _hotSpot, parentDrag), id(_id), faceDown(_faceDown), occupied(false), currentZone(0)
{
}

void CardDragItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	AbstractCardDragItem::paint(painter, option, widget);
	
	if (occupied)
		painter->fillRect(boundingRect(), QColor(200, 0, 0, 100));
}

void CardDragItem::updatePosition(const QPointF &cursorScenePos)
{
	QList<QGraphicsItem *> colliding = scene()->items(cursorScenePos);

	CardZone *cardZone = 0;
	ZoneViewZone *zoneViewZone = 0;
	for (int i = colliding.size() - 1; i >= 0; i--) {
		CardZone *temp = qgraphicsitem_cast<CardZone *>(colliding.at(i));
		if (!cardZone)
			cardZone = temp;
		if (!zoneViewZone)
			zoneViewZone = qobject_cast<ZoneViewZone *>(temp);
	}
	CardZone *cursorZone = 0;
	if (zoneViewZone)
		cursorZone = zoneViewZone;
	else if (cardZone)
		cursorZone = cardZone;
	if (!cursorZone)
		return;
	currentZone = cursorZone;
	
	QPointF zonePos = currentZone->scenePos();
	QPointF cursorPosInZone = cursorScenePos - zonePos;
	QPointF cardTopLeft = cursorPosInZone - hotSpot;
	QPointF closestGridPoint = cursorZone->closestGridPoint(cardTopLeft);
	QPointF newPos = zonePos + closestGridPoint;
	
	if (newPos != pos()) {
		for (int i = 0; i < childDrags.size(); i++)
			childDrags[i]->setPos(newPos + childDrags[i]->getHotSpot());
		setPos(newPos);
		
		bool newOccupied = false;
		TableZone *table = qobject_cast<TableZone *>(cursorZone);
		if (table)
			if (table->getCardFromCoords(closestGridPoint))
				newOccupied = true;
		if (newOccupied != occupied) {
			occupied = newOccupied;
			update();
		}
	}
}

void CardDragItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	setCursor(Qt::OpenHandCursor);
	QGraphicsScene *sc = scene();
	QPointF sp = pos();
	sc->removeItem(this);

	QList<CardDragItem *> dragItemList;
	CardZone *startZone = static_cast<CardItem *>(item)->getZone();
	if (currentZone && !(static_cast<CardItem *>(item)->getAttachedTo() && (startZone == currentZone))) {
		dragItemList.append(this);
		for (int i = 0; i < childDrags.size(); i++) {
			CardDragItem *c = static_cast<CardDragItem *>(childDrags[i]);
			if (!(static_cast<CardItem *>(c->item)->getAttachedTo() && (startZone == currentZone)) && !c->occupied)
				dragItemList.append(c);
			sc->removeItem(c);
		}
	}
	currentZone->handleDropEvent(dragItemList, startZone, (sp - currentZone->scenePos()).toPoint(), faceDown);

	event->accept();
}
