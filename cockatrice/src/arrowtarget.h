#ifndef ARROWTARGET_H
#define ARROWTARGET_H

#include "abstractgraphicsitem.h"

class ArrowTarget : public AbstractGraphicsItem {
private:
	bool beingPointedAt;
public:
	ArrowTarget(QGraphicsItem *parent = 0);
	void setBeingPointedAt(bool _beingPointedAt);
	bool getBeingPointedAt() const { return beingPointedAt; }
};

#endif