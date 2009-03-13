#ifndef CARDDRAGITEM_H
#define CARDDRAGITEM_H

#include "carditem.h"

class QGraphicsScene;
class PlayerZone;

class CardDragItem : public QGraphicsItem {
private:
	QPixmap *image;
	int id;
	QPointF hotSpot;
	PlayerZone *startZone;
public:
	enum { Type = typeCardDrag };
	int type() const { return Type; }
	CardDragItem(QGraphicsScene *scene, PlayerZone *_startZone, QPixmap *_image, int _id, const QPointF &_hotSpot, QGraphicsItem *parent = 0);
	~CardDragItem();
	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	QPointF getHotSpot() const { return hotSpot; }
protected:
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
};

#endif
