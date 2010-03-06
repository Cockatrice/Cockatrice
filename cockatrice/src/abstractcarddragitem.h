#ifndef ABSTRACTCARDDRAGITEM_H
#define ABSTRACTCARDDRAGITEM_H

#include "abstractcarditem.h"

class QGraphicsScene;
class CardZone;
class CardInfo;

class AbstractCardDragItem : public QGraphicsItem {
protected:
	AbstractCardItem *item;
	QPointF hotSpot;
	QList<AbstractCardDragItem *> childDrags;
public:
	enum { Type = typeCardDrag };
	int type() const { return Type; }
	AbstractCardDragItem(AbstractCardItem *_item, const QPointF &_hotSpot, AbstractCardDragItem *parentDrag = 0);
	~AbstractCardDragItem();
	QRectF boundingRect() const { return QRectF(0, 0, CARD_WIDTH, CARD_HEIGHT); }
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	QPointF getHotSpot() const { return hotSpot; }
	void addChildDrag(AbstractCardDragItem *child);
	virtual void updatePosition(const QPointF &cursorScenePos) = 0;
protected:
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
};

#endif
