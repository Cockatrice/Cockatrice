#ifndef TABLEZONE_H
#define TABLEZONE_H

#include "playerzone.h"

const int GRID_WIDTH = 30;
const int GRID_HEIGHT = 30;

class TableZone : public PlayerZone {
private:
	int width, height;
public:
	TableZone(Player *_p);
	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void addCard(CardItem *card, bool reorganize = true, int x = -1, int y = -1);
	void reorganizeCards();
	void toggleTapped();
	void handleDropEvent(int cardId, PlayerZone *startZone, const QPoint &dropPoint);
};

#endif
