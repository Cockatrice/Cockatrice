#include "card_drag_item.h"

#include "../game_scene.h"
#include "../zones/card_zone.h"
#include "card_item.h"

#include <QCursor>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

CardDragItem::CardDragItem(CardItem *_item,
                           int _id,
                           const QPointF &_hotSpot,
                           bool _faceDown,
                           AbstractCardDragItem *parentDrag)
    : AbstractCardDragItem(_item, _hotSpot, parentDrag), id(_id), faceDown(_faceDown), currentZone(0)
{
}

CardDragItem::~CardDragItem()
{
    if (currentZone) {
        currentZone->dragLeave(this);
        currentZone = nullptr;
    }
}

void CardDragItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    AbstractCardDragItem::paint(painter, option, widget);

    if (!isValid) {
        painter->fillPath(shape(), QColor(200, 0, 0, 100));
    }
}

void CardDragItem::updatePosition(const QPointF &cursorScenePos)
{
    QPointF topLeftScenePos = cursorScenePos - hotSpot;
    QPointF centerScenePos = topLeftScenePos + QPointF(CARD_WIDTH_HALF, CARD_HEIGHT_HALF);

    // Use center of the card for intersection.
    QList<QGraphicsItem *> colliding =
        scene()->items(centerScenePos, Qt::IntersectsItemBoundingRect, Qt::DescendingOrder,
                       static_cast<GameScene *>(scene())->getViewTransform());

    CardZone *cardZone = nullptr;
    for (auto *item : colliding) {
        cardZone = qgraphicsitem_cast<CardZone *>(item);

        if (cardZone) {
            break;
        }
    }

    if (cardZone != currentZone) {
        if (currentZone) {
            currentZone->dragLeave(this);
            currentZone = nullptr;
        }

        if (cardZone && cardZone->dragEnter(this, centerScenePos - cardZone->scenePos())) {
            setValid(true);
            currentZone = cardZone;
        } else {
            setValid(false);
        }
    }

    if (currentZone) {
        currentZone->dragMove(this, centerScenePos - currentZone->scenePos());
    }

    if (topLeftScenePos != pos()) {
        for (int i = 0; i < childDrags.size(); i++)
            childDrags[i]->setPos(topLeftScenePos + childDrags[i]->getHotSpot());
        setPos(topLeftScenePos);
    }
}

void CardDragItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    setCursor(Qt::OpenHandCursor);
    QGraphicsScene *sc = scene();
    sc->removeItem(this);

    for (auto *childDrag : childDrags) {
        sc->removeItem(childDrag);
    }

    if (currentZone) {
        QPointF cursorScenePos = event->scenePos();
        QPointF topLeftScenePos = cursorScenePos - hotSpot;
        QPointF centerScenePos = topLeftScenePos + QPointF(CARD_WIDTH_HALF, CARD_HEIGHT_HALF);
        currentZone->dragAccept(this, centerScenePos - currentZone->scenePos());
        currentZone = nullptr;
    }

    static_cast<CardItem *>(item)->deleteDragItem();
    AbstractCardDragItem::mouseReleaseEvent(event);
}

CardZone *CardDragItem::getStartZone() const
{
    return static_cast<CardItem *>(item)->getZone();
}

QList<CardDragItem *> CardDragItem::getValidItems()
{
    QList<CardDragItem *> dragItemList;
    CardZone *startZone = getStartZone();
    if (!(static_cast<CardItem *>(item)->getAttachedTo() && startZone == currentZone)) {
        dragItemList.append(this);

        for (int i = 0; i < childDrags.size(); i++) {
            CardDragItem *c = static_cast<CardDragItem *>(childDrags[i]);
            if (!(static_cast<CardItem *>(c->item)->getAttachedTo() && startZone == currentZone)) {
                dragItemList.append(c);
            }
        }
    }

    return dragItemList;
}
