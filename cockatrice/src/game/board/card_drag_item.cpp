#include "card_drag_item.h"

#include "../game_scene.h"
#include "../zones/card_zone.h"
#include "../zones/table_zone.h"
#include "../zones/view_zone.h"
#include "card_item.h"

#include <QCursor>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

CardDragItem::CardDragItem(CardItem *_item,
                           int _id,
                           const QPointF &_hotSpot,
                           bool _faceDown,
                           AbstractCardDragItem *parentDrag)
    : AbstractCardDragItem(_item, _hotSpot, parentDrag), id(_id), faceDown(_faceDown), occupied(false), currentZone(0)
{
}

void CardDragItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    AbstractCardDragItem::paint(painter, option, widget);

    if (occupied)
        painter->fillPath(shape(), QColor(200, 0, 0, 100));
}

void CardDragItem::updatePosition(const QPointF &cursorScenePos)
{
    QList<QGraphicsItem *> colliding =
        scene()->items(cursorScenePos, Qt::IntersectsItemBoundingRect, Qt::DescendingOrder,
                       static_cast<GameScene *>(scene())->getViewTransform());

    CardZone *cardZone = 0;
    ZoneViewZone *zoneViewZone = 0;
    for (int i = colliding.size() - 1; i >= 0; i--) {
        CardZone *temp = qgraphicsitem_cast<CardZone *>(colliding.at(i));
        if (!cardZone)
            cardZone = temp;
        if (!zoneViewZone)
            zoneViewZone = qobject_cast<ZoneViewZone *>(temp);
    }
    CardZone *cursorZone = 0;
    if (zoneViewZone)
        cursorZone = zoneViewZone;
    else if (cardZone)
        cursorZone = cardZone;

    // Always update the current zone, even if its null, to cancel the drag
    // instead of dropping cards into an non-intuitive location.
    currentZone = cursorZone;

    if (!cursorZone) {
        // Avoid the cards getting stuck visually when not over
        // any zone.
        QPointF newPos = cursorScenePos - hotSpot;

        if (newPos != pos()) {
            for (int i = 0; i < childDrags.size(); i++)
                childDrags[i]->setPos(newPos + childDrags[i]->getHotSpot());
            setPos(newPos);
        }

        return;
    }

    QPointF zonePos = currentZone->scenePos();
    QPointF cursorPosInZone = cursorScenePos - zonePos;

    // If we are on a Table, we center the card around the cursor, because we
    // snap it into place and no longer see it being dragged.
    //
    // For other zones (where we do display the card under the cursor), we use
    // the hotspot to feel like the card was dragged at the corresponding
    // position.
    TableZone *tableZone = qobject_cast<TableZone *>(cursorZone);
    QPointF closestGridPoint;
    if (tableZone)
        closestGridPoint = tableZone->closestGridPoint(cursorPosInZone);
    else
        closestGridPoint = cursorPosInZone - hotSpot;

    QPointF newPos = zonePos + closestGridPoint;

    if (newPos != pos()) {
        for (int i = 0; i < childDrags.size(); i++)
            childDrags[i]->setPos(newPos + childDrags[i]->getHotSpot());
        setPos(newPos);

        bool newOccupied = false;
        TableZone *table = qobject_cast<TableZone *>(cursorZone);
        if (table)
            if (table->getCardFromCoords(closestGridPoint))
                newOccupied = true;
        if (newOccupied != occupied) {
            occupied = newOccupied;
            update();
        }
    }
}

void CardDragItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    setCursor(Qt::OpenHandCursor);
    QGraphicsScene *sc = scene();
    QPointF sp = pos();
    sc->removeItem(this);

    QList<CardDragItem *> dragItemList;
    CardZoneLogic *startZone = static_cast<CardItem *>(item)->getZone();
    if (currentZone && !(static_cast<CardItem *>(item)->getAttachedTo() && (startZone == currentZone->getLogic()))) {
        if (!occupied) {
            dragItemList.append(this);
        }

        for (int i = 0; i < childDrags.size(); i++) {
            CardDragItem *c = static_cast<CardDragItem *>(childDrags[i]);
            if (!occupied &&
                !(static_cast<CardItem *>(c->item)->getAttachedTo() && (startZone == currentZone->getLogic())) &&
                !c->occupied) {
                dragItemList.append(c);
            }
            sc->removeItem(c);
        }
    }

    if (currentZone) {
        currentZone->handleDropEvent(dragItemList, startZone, (sp - currentZone->scenePos()).toPoint());
    }

    event->accept();
}
