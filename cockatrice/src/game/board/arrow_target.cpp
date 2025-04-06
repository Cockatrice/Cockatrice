#include "arrow_target.h"

#include "../player/player.h"
#include "arrow_item.h"

ArrowTarget::ArrowTarget(Player *_owner, QGraphicsItem *parent)
    : AbstractGraphicsItem(parent), owner(_owner), beingPointedAt(false)
{
    setFlag(ItemSendsScenePositionChanges);
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

QVariant ArrowTarget::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemScenePositionHasChanged && scene()) {
        for (auto *arrow : arrowsFrom)
            arrow->updatePath();

        for (auto *arrow : arrowsTo)
            arrow->updatePath();
    }

    return QGraphicsItem::itemChange(change, value);
}
