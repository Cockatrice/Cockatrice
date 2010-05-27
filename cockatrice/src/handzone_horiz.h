#ifndef HANDZONE_HORIZ_H
#define HANDZONE_HORIZ_H

#include "handzone.h"

class HandZoneHoriz : public HandZone {
	Q_OBJECT
private:
	qreal width;
public:
	HandZoneHoriz(Player *_p, bool _contentsKnown, QGraphicsItem *parent = 0);
	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void reorganizeCards();
	void setWidth(qreal _width);
};

#endif
