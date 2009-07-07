#ifndef CARDDRAGITEM_H
#define CARDDRAGITEM_H

#include "carditem.h"

class QGraphicsScene;
class CardZone;
class CardInfo;

class CardDragItem : public QGraphicsItem {
private:
	int id;
	CardItem *item;
	QPointF hotSpot;
	bool faceDown;
	QList<CardDragItem *> childDrags;
	CardZone *currentZone;
public:
	enum { Type = typeCardDrag };
	int type() const { return Type; }
	CardDragItem(CardItem *_item, int _id, const QPointF &_hotSpot, bool _faceDown, CardDragItem *parentDrag = 0);
	~CardDragItem();
	QRectF boundingRect() const { return QRectF(0, 0, CARD_WIDTH, CARD_HEIGHT); }
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	QPointF getHotSpot() const { return hotSpot; }
	void addChildDrag(CardDragItem *child);
	void updatePosition(const QPointF &cursorScenePos);
protected:
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
};

#endif
