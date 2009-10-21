#ifndef ARROWITEM_H
#define ARROWITEM_H

#include <QGraphicsItem>

class CardItem;
class QGraphicsSceneMouseEvent;
class QMenu;
class Player;

class ArrowItem : public QObject, public QGraphicsItem {
	Q_OBJECT
private:
	QPainterPath path;
	QMenu *menu;
protected:
	Player *player;
	int id;
	CardItem *startItem, *targetItem;
	QColor color;
	bool fullColor;
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
public:
	ArrowItem(Player *_player, int _id, CardItem *_startItem, CardItem *_targetItem, const QColor &color);
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
	ArrowDragItem(CardItem *_startItem, const QColor &_color);
protected:
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
};

#endif // ARROWITEM_H
