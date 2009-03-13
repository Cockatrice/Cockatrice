#ifndef SIDEBOARDZONE_H
#define SIDEBOARDZONE_H

#include "playerzone.h"

class SideboardZone : public PlayerZone {
private:
public:
	SideboardZone(Player *_p);
	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void addCard(CardItem *card, bool reorganize = true, int x = 0, int y = -1);
	void reorganizeCards();
	void handleDropEvent(int cardId, PlayerZone *startZone, const QPoint &dropPoint);
};

#endif
