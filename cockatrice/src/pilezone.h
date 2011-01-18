#ifndef PILEZONE_H
#define PILEZONE_H

#include "cardzone.h"

class PileZone : public CardZone {
public:
	PileZone(Player *_p, const QString &_name, bool _isShufflable, bool _contentsKnown, QGraphicsItem *parent = 0);
	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void reorganizeCards();
	void handleDropEvent(const QList<CardDragItem *> &dragItems, CardZone *startZone, const QPoint &dropPoint, bool faceDown);
protected:
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
	void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
	void addCardImpl(CardItem *card, int x, int y);
};

#endif
