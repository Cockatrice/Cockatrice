#ifndef TABLEZONE_H
#define TABLEZONE_H

#include "cardzone.h"

const int GRID_WIDTH = 30;
const int GRID_HEIGHT = 30;

class TableZone : public CardZone {
private:
	int width, height;
public:
	TableZone(Player *_p, QGraphicsItem *parent = 0);
	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void addCard(CardItem *card, bool reorganize = true, int x = -1, int y = -1);
	void reorganizeCards();
	void toggleTapped();
	void handleDropEvent(int cardId, CardZone *startZone, const QPoint &dropPoint);
};

#endif
