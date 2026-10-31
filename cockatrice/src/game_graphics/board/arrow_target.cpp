#include "arrow_target.h"

#include <QVariant>

class QGraphicsItem;

ArrowTarget::ArrowTarget(PlayerLogic *_owner, QGraphicsItem *parent) : AbstractGraphicsItem(parent), owner(_owner)
{
    setFlag(ItemSendsScenePositionChanges);
}

void ArrowTarget::setBeingPointedAt(bool _beingPointedAt)
{
    beingPointedAt = _beingPointedAt;
    update();
}

QVariant ArrowTarget::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemScenePositionHasChanged) {
        emit scenePositionChanged();
    }
    return AbstractGraphicsItem::itemChange(change, value);
}