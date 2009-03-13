#ifndef LIBRARYZONE_H
#define LIBRARYZONE_H

#include "playerzone.h"

class LibraryZone : public PlayerZone {
private:
public:
	LibraryZone(Player *_p);
	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void addCard(CardItem *card, bool reorganize = true, int x = 0, int y = -1);
	void reorganizeCards();
	void handleDropEvent(int cardId, PlayerZone *startZone, const QPoint &dropPoint);
protected:
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
};

#endif
