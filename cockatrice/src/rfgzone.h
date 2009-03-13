#ifndef RFGZONE_H
#define RFGZONE_H

#include "playerzone.h"

class RfgZone : public PlayerZone {
private:
public:
	RfgZone(Player *_p);
	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void addCard(CardItem *card, bool reorganize = true, int x = 0, int y = -1);
	void reorganizeCards();
	void handleDropEvent(int cardId, PlayerZone *startZone, const QPoint &dropPoint);
};

#endif
