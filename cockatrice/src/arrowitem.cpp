#include "arrowitem.h"
#include "carditem.h"
#include "cardzone.h"
#include "player.h"
#include "math.h"
#include "protocol_items.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>

ArrowItem::ArrowItem(Player *_player, int _id, CardItem *_startItem, CardItem *_targetItem, const QColor &_color)
        : QGraphicsItem(), player(_player), id(_id), startItem(_startItem), targetItem(_targetItem), color(_color), fullColor(true)
{
	setZValue(2000000005);
	if (startItem && targetItem)
		updatePath();
}

void ArrowItem::updatePath()
{
	QPointF endPoint = targetItem->mapToScene(QPointF(targetItem->boundingRect().width() / 2, targetItem->boundingRect().height() / 2));
	updatePath(endPoint);
}

void ArrowItem::updatePath(const QPointF &endPoint)
{
	const double arrowWidth = 15.0;
	const double headWidth = 40.0;
	const double headLength = headWidth / sqrt(2);
	const double phi = 15;
	
	QPointF startPoint = startItem->mapToScene(QPointF(startItem->boundingRect().width() / 2, startItem->boundingRect().height() / 2));
	QLineF line(startPoint, endPoint);
	qreal lineLength = line.length();
	
	prepareGeometryChange();
	if (lineLength < 30)
		path = QPainterPath();
	else {
		QPointF c(lineLength / 2, tan(phi * M_PI / 180) * lineLength);
		
		QPainterPath centerLine;
		centerLine.moveTo(0, 0);
		centerLine.quadTo(c, QPointF(lineLength, 0));
		
		double percentage = 1 - headLength / lineLength;
		QPointF arrowBodyEndPoint = centerLine.pointAtPercent(percentage);
		QLineF testLine(arrowBodyEndPoint, centerLine.pointAtPercent(percentage + 0.001));
		qreal alpha = testLine.angle() - 90;
		QPointF endPoint1 = arrowBodyEndPoint + arrowWidth / 2 * QPointF(cos(alpha * M_PI / 180), -sin(alpha * M_PI / 180));
		QPointF endPoint2 = arrowBodyEndPoint + arrowWidth / 2 * QPointF(-cos(alpha * M_PI / 180), sin(alpha * M_PI / 180));
		QPointF point1 = endPoint1 + (headWidth - arrowWidth) / 2 * QPointF(cos(alpha * M_PI / 180), -sin(alpha * M_PI / 180));
		QPointF point2 = endPoint2 + (headWidth - arrowWidth) / 2 * QPointF(-cos(alpha * M_PI / 180), sin(alpha * M_PI / 180));
		
		path = QPainterPath(-arrowWidth / 2 * QPointF(cos((phi - 90) * M_PI / 180), sin((phi - 90) * M_PI / 180)));
		path.quadTo(c, endPoint1);
		path.lineTo(point1);
		path.lineTo(QPointF(lineLength, 0));
		path.lineTo(point2);
		path.lineTo(endPoint2);
		path.quadTo(c, arrowWidth / 2 * QPointF(cos((phi - 90) * M_PI / 180), sin((phi - 90) * M_PI / 180)));
		path.lineTo(-arrowWidth / 2 * QPointF(cos((phi - 90) * M_PI / 180), sin((phi - 90) * M_PI / 180)));
	}
	
	setPos(startPoint);
	setTransform(QTransform().rotate(-line.angle()));
}

void ArrowItem::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
	QColor paintColor(color);
	if (fullColor)
		paintColor.setAlpha(200);
	else
		paintColor.setAlpha(150);
	painter->setBrush(paintColor);
	painter->drawPath(path);
}

void ArrowItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	if (!player->getLocal()) {
		event->ignore();
		return;
	}
	
	QList<QGraphicsItem *> colliding = scene()->items(event->scenePos());
	for (int i = 0; i < colliding.size(); ++i)
		if (qgraphicsitem_cast<CardItem *>(colliding[i])) {
			event->ignore();
			return;
		}
	
	event->accept();
	if (event->button() == Qt::RightButton)
		player->sendGameCommand(new Command_DeleteArrow(-1, id));
}

ArrowDragItem::ArrowDragItem(CardItem *_startItem, const QColor &_color)
        : ArrowItem(static_cast<CardZone *>(_startItem->parentItem())->getPlayer(), -1, _startItem, 0, _color)
{
}

void ArrowDragItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	QPointF endPos = event->scenePos();
	
	QList<QGraphicsItem *> colliding = scene()->items(endPos);
        CardItem *cursorItem = 0;
        for (int i = colliding.size() - 1; i >= 0; i--)
                if ((cursorItem = qgraphicsitem_cast<CardItem *>(colliding.at(i))))
                        break;
	if ((cursorItem != targetItem) && targetItem)
		targetItem->setBeingPointedAt(false);
        if (!cursorItem) {
		fullColor = false;
		targetItem = 0;
		updatePath(endPos);
	} else {
		fullColor = true;
		cursorItem->setBeingPointedAt(true);
		targetItem = cursorItem;
		updatePath();
	}
	update();
}

void ArrowDragItem::mouseReleaseEvent(QGraphicsSceneMouseEvent * /*event*/)
{
	if (targetItem && (targetItem != startItem)) {
		targetItem->setBeingPointedAt(false);
		CardZone *startZone = static_cast<CardZone *>(startItem->parentItem());
		CardZone *targetZone = static_cast<CardZone *>(targetItem->parentItem());
		player->sendGameCommand(new Command_CreateArrow(
			-1,
			startZone->getPlayer()->getId(),
			startZone->getName(),
			startItem->getId(),
			targetZone->getPlayer()->getId(),
			targetZone->getName(),
			targetItem->getId(),
			color
		));
	}
	deleteLater();
}
