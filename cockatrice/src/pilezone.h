#ifndef PILEZONE_H
#define PILEZONE_H

#include "cardzone.h"

class PileZone : public CardZone {
private:
public:
	PileZone(Player *_p, const QString &_name, QGraphicsItem *parent = 0);
	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void reorganizeCards();
	void handleDropEvent(int cardId, CardZone *startZone, const QPoint &dropPoint, bool faceDown);
protected:
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
	void addCardImpl(CardItem *card, int x, int y);
};

#endif
