#ifndef HANDZONE_H
#define HANDZONE_H

#include "selectzone.h"

class HandZone : public SelectZone {
	Q_OBJECT
private:
	qreal width, zoneHeight;
	QPixmap bgPixmap;
private slots:
	void updateBgPixmap();
public slots:
	void updateOrientation();
public:
	HandZone(Player *_p, bool _contentsKnown, int _zoneHeight, QGraphicsItem *parent = 0);
	void handleDropEvent(const QList<CardDragItem *> &dragItems, CardZone *startZone, const QPoint &dropPoint, bool faceDown);
	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void reorganizeCards();
	void setWidth(qreal _width);
protected:
	void addCardImpl(CardItem *card, int x, int y);
};

#endif

