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
	int id;
	CardItem *startItem, *targetItem;
	QColor color;
	bool fullColor;
public:
	ArrowItem(int id, CardItem *_startItem = 0, CardItem *_targetItem = 0, const QColor &color = Qt::red);
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	QRectF boundingRect() const { return path.boundingRect(); }
	void updatePath();
	void updatePath(const QPointF &endPoint);
	
	int getId() const { return id; }
	void setStartItem(CardItem *_item) { startItem = _item; }
	void setTargetItem(CardItem *_item) { targetItem = _item; }
	CardItem *getStartItem() const { return startItem; }
	CardItem *getTargetItem() const { return targetItem; }
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
