#include "arrowtarget.h"
#include "arrowitem.h"
#include "player.h"

ArrowTarget::ArrowTarget(Player *_owner, QGraphicsItem *parent)
	: AbstractGraphicsItem(parent), owner(_owner), beingPointedAt(false)
{
}

ArrowTarget::~ArrowTarget()
{
	for (int i = 0; i < arrowsFrom.size(); ++i) {
		arrowsFrom[i]->setStartItem(0);
		arrowsFrom[i]->delArrow();
	}
	for (int i = 0; i < arrowsTo.size(); ++i) {
		arrowsTo[i]->setTargetItem(0);
		arrowsTo[i]->delArrow();
	}
}

void ArrowTarget::setBeingPointedAt(bool _beingPointedAt)
{
	beingPointedAt = _beingPointedAt;
	update();
}
