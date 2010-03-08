#include "carddragitem.h"
#include "cardzone.h"
#include "tablezone.h"
#include "zoneviewzone.h"
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QCursor>
#include <QDebug>

CardDragItem::CardDragItem(AbstractCardItem *_item, int _id, const QPointF &_hotSpot, bool _faceDown, AbstractCardDragItem *parentDrag)
	: AbstractCardDragItem(_item, _hotSpot, parentDrag), id(_id), faceDown(_faceDown), currentZone(0)
{
}

void CardDragItem::updatePosition(const QPointF &cursorScenePos)
{
	QList<QGraphicsItem *> colliding = scene()->items(cursorScenePos);

	CardZone *cardZone = 0;
	ZoneViewZone *zoneViewZone = 0;
	for (int i = colliding.size() - 1; i >= 0; i--) {
		CardZone *temp = qgraphicsitem_cast<CardZone *>(colliding.at(i));
		if (temp)
			qDebug() << "zone name:" << temp->getName();
		if (!cardZone)
			cardZone = temp;
		if (!zoneViewZone)
			zoneViewZone = qobject_cast<ZoneViewZone *>(temp);
	}
	CardZone *cursorZone = 0;
	if (zoneViewZone) {
		qDebug() << "zv found";
		cursorZone = zoneViewZone;
	} else if (cardZone) {
		qDebug() << "normal found";
		cursorZone = cardZone;
	}
	if (!cursorZone)
		return;
	currentZone = cursorZone;
	
	QPointF zonePos = currentZone->scenePos();
	QPointF cursorPosInZone = cursorScenePos - zonePos;
	QPointF cardTopLeft = cursorPosInZone - hotSpot;
	QPointF newPos = zonePos + cursorZone->closestGridPoint(cardTopLeft);
	
	if (newPos != pos()) {
		for (int i = 0; i < childDrags.size(); i++)
			childDrags[i]->setPos(newPos + childDrags[i]->getHotSpot());
		setPos(newPos);
	}
}

void CardDragItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	setCursor(Qt::OpenHandCursor);
	QGraphicsScene *sc = scene();
	QPointF sp = pos();
	sc->removeItem(this);

	if (currentZone) {
		CardZone *startZone = qgraphicsitem_cast<CardZone *>(item->parentItem());
		currentZone->handleDropEvent(id, startZone, (sp - currentZone->scenePos()).toPoint(), faceDown);
		for (int i = 0; i < childDrags.size(); i++) {
			CardDragItem *c = static_cast<CardDragItem *>(childDrags[i]);
			currentZone->handleDropEvent(c->id, startZone, (sp - currentZone->scenePos() + c->getHotSpot()).toPoint(), faceDown);
			sc->removeItem(c);
		}
	}

	event->accept();
}
