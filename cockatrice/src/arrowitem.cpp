#include "arrowitem.h"
#include "carditem.h"
#include "cardzone.h"
#include "player.h"
#include "math.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>

ArrowItem::ArrowItem(CardItem *_startItem, CardItem *_targetItem)
        : QGraphicsItem(), startItem(_startItem), targetItem(_targetItem)
{
	setZValue(2000000005);
	if (startItem && targetItem)
		updatePath();
}

void ArrowItem::updatePath()
{
	color = QColor(255, 0, 0, 200);
	QPointF endPoint = targetItem->mapToScene(QPointF(targetItem->boundingRect().width() / 2, targetItem->boundingRect().height() / 2));
	updatePath(endPoint);
}

void ArrowItem::updatePath(const QPointF &endPoint)
{
	const double arrowWidth = 15.0;
	const double headWidth = 40.0;
	const double headLength = headWidth / sqrt(2);
	
	QPointF startPoint = startItem->mapToScene(QPointF(startItem->boundingRect().width() / 2, startItem->boundingRect().height() / 2));
	QLineF line(startPoint, endPoint);
	qreal lineLength = line.length();
	
	path = QPainterPath(QPointF(0, -arrowWidth / 2));
	path.lineTo(0, arrowWidth / 2);
	path.lineTo(lineLength - headLength, arrowWidth / 2);
	path.lineTo(lineLength - headLength, headWidth / 2);
	path.lineTo(lineLength, 0);
	path.lineTo(lineLength - headLength, -headWidth / 2);
	path.lineTo(lineLength - headLength, -arrowWidth / 2);
	path.lineTo(0, -arrowWidth / 2);
	
	setPos(startPoint);
	setTransform(QTransform().rotate(-line.angle()));
}

void ArrowItem::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
	painter->setBrush(color);
	painter->drawPath(path);
}

ArrowDragItem::ArrowDragItem(CardItem *_startItem)
        : ArrowItem(_startItem)
{
}

void ArrowDragItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	event->accept();
	QPointF endPos = event->scenePos();
	
	QList<QGraphicsItem *> colliding = scene()->items(endPos);
        CardItem *cursorItem = 0;
        for (int i = colliding.size() - 1; i >= 0; i--)
                if ((cursorItem = qgraphicsitem_cast<CardItem *>(colliding.at(i))))
                        break;
        if (!cursorItem) {
		targetItem = 0;
		updatePath(endPos);
		color = QColor(190, 0, 0, 150);
	} else {
		targetItem = cursorItem;
		updatePath();
	}
	update();
}

void ArrowDragItem::mouseReleaseEvent(QGraphicsSceneMouseEvent * /*event*/)
{
	if (targetItem) {
		CardZone *startZone = static_cast<CardZone *>(startItem->parentItem());
		CardZone *targetZone = static_cast<CardZone *>(targetItem->parentItem());
		startZone->getPlayer()->client->createArrow(
			startZone->getPlayer()->getId(),
			startZone->getName(),
			startItem->getId(),
			targetZone->getPlayer()->getId(),
			targetZone->getName(),
			targetItem->getId()
		);
	}
	deleteLater();
}
