#include "arrowtarget.h"
#include "arrowitem.h"
#include "player.h"

ArrowTarget::ArrowTarget(Player *_owner, QGraphicsItem *parent)
	: AbstractGraphicsItem(parent), owner(_owner), beingPointedAt(false)
{
}

ArrowTarget::~ArrowTarget()
{
	while (!arrowsFrom.isEmpty())
		arrowsFrom.first()->getPlayer()->delArrow(arrowsFrom.first()->getId());
	while (!arrowsTo.isEmpty())
		arrowsTo.first()->getPlayer()->delArrow(arrowsTo.first()->getId());
}

void ArrowTarget::setBeingPointedAt(bool _beingPointedAt)
{
	beingPointedAt = _beingPointedAt;
	update();
}
