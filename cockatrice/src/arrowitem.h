#ifndef ARROWITEM_H
#define ARROWITEM_H

#include <QGraphicsItem>

class CardItem;
class QGraphicsSceneMouseEvent;

class ArrowItem : public QObject, public QGraphicsItem {
	Q_OBJECT
private:
	QPainterPath path;
protected:
	QColor color;
	CardItem *startItem, *targetItem;
public:
	ArrowItem(CardItem *_startItem = 0, CardItem *_targetItem = 0);
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	QRectF boundingRect() const { return path.boundingRect(); }
	void updatePath();
	void updatePath(const QPointF &endPoint);
};

class ArrowDragItem : public ArrowItem {
	Q_OBJECT
public:
	ArrowDragItem(CardItem *_startItem);
protected:
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
};

#endif // ARROWITEM_H
