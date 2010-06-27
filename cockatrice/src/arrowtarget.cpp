#include "arrowtarget.h"

ArrowTarget::ArrowTarget(QGraphicsItem *parent)
	: AbstractGraphicsItem(parent), beingPointedAt(false)
{
}

void ArrowTarget::setBeingPointedAt(bool _beingPointedAt)
{
	beingPointedAt = _beingPointedAt;
	update();
}
