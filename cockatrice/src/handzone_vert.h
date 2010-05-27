#ifndef HANDZONE_VERT_H
#define HANDZONE_VERT_H

#include "handzone.h"

class HandZoneVert : public HandZone {
	Q_OBJECT
private:
	int zoneHeight;
public:
	HandZoneVert(Player *_p, bool _contentsKnown, int _zoneHeight, QGraphicsItem *parent = 0);
	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	void reorganizeCards();
	void setWidth(qreal /*_width*/) { }
};

#endif
