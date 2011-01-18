#ifndef STACKZONE_H
#define STACKZONE_H

#include "selectzone.h"

class StackZone : public SelectZone {
	Q_OBJECT
private:
	qreal zoneHeight;
	QPixmap bgPixmap;
private slots:
	void updateBgPixmap();
public:
	StackZone(Player *_p, int _zoneHeight, QGraphicsItem *parent = 0);
	void handleDropEvent(const QList<CardDragItem *> &dragItems, CardZone *startZone, const QPoint &dropPoint, bool faceDown);
	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void reorganizeCards();
protected:
	void addCardImpl(CardItem *card, int x, int y);
};

#endif

