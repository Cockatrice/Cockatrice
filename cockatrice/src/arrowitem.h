#ifndef ARROWITEM_H
#define ARROWITEM_H

#include <QGraphicsItem>

class CardItem;
class QGraphicsSceneMouseEvent;
class QMenu;
class Player;
class ArrowTarget;

class ArrowItem : public QObject, public QGraphicsItem {
	Q_OBJECT
private:
	QPainterPath path;
	QMenu *menu;
protected:
	Player *player;
	int id;
	ArrowTarget *startItem, *targetItem;
	QColor color;
	bool fullColor;
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
public:
	ArrowItem(Player *_player, int _id, ArrowTarget *_startItem, ArrowTarget *_targetItem, const QColor &color);
	~ArrowItem();
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	QRectF boundingRect() const { return path.boundingRect(); }
	QPainterPath shape() const { return path; }
	void updatePath();
	void updatePath(const QPointF &endPoint);
	
	int getId() const { return id; }
	Player *getPlayer() const { return player; }
	void setStartItem(ArrowTarget *_item) { startItem = _item; }
	void setTargetItem(ArrowTarget *_item) { targetItem = _item; }
	ArrowTarget *getStartItem() const { return startItem; }
	ArrowTarget *getTargetItem() const { return targetItem; }
	void delArrow();
};

class ArrowDragItem : public ArrowItem {
	Q_OBJECT
private:
	QList<ArrowDragItem *> childArrows;
public:
	ArrowDragItem(Player *_owner, ArrowTarget *_startItem, const QColor &_color);
	void addChildArrow(ArrowDragItem *childArrow);
protected:
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
};

class ArrowAttachItem : public ArrowItem {
	Q_OBJECT
public:
	ArrowAttachItem(ArrowTarget *_startItem);
protected:
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
};

#endif // ARROWITEM_H
