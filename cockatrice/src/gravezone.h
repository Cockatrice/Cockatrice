#ifndef GRAVEZONE_H
#define GRAVEZONE_H

#include "cardzone.h"

class GraveZone : public CardZone {
private:
public:
	GraveZone(Player *_p, QGraphicsItem *parent = 0);
	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void addCard(CardItem *card, bool reorganize = true, int x = 0, int y = -1);
	void reorganizeCards();
	void handleDropEvent(int cardId, CardZone *startZone, const QPoint &dropPoint);
};

#endif
