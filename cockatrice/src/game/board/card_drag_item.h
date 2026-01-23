/**
 * @file card_drag_item.h
 * @ingroup GameGraphicsCards
 * @brief TODO: Document this.
 */

#ifndef CARDDRAGITEM_H
#define CARDDRAGITEM_H

#include "abstract_card_drag_item.h"

class CardItem;

class CardDragItem : public AbstractCardDragItem
{
    Q_OBJECT
private:
    int id;
    bool forceFaceDown;
    bool occupied;
    CardZone *currentZone;

public:
    CardDragItem(CardItem *_item,
                 int _id,
                 const QPointF &_hotSpot,
                 bool _forceFaceDown,
                 AbstractCardDragItem *parentDrag = 0);
    int getId() const
    {
        return id;
    }
    bool isForceFaceDown() const
    {
        return forceFaceDown;
    }
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void updatePosition(const QPointF &cursorScenePos) override;

protected:
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
};

#endif
