#ifndef SIDEBOARDZONE_H
#define SIDEBOARDZONE_H

#include "cardzone.h"

class SideboardZone : public CardZone {
private:
public:
	SideboardZone(Player *_p, QGraphicsItem *parent = 0);
	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void reorganizeCards();
	void handleDropEvent(int cardId, CardZone *startZone, const QPoint &dropPoint, bool faceDown);
protected:
	void addCardImpl(CardItem *card, int x, int y);
};

#endif
