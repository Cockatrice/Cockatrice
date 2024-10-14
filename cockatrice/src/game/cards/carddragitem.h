#ifndef CARDDRAGITEM_H
#define CARDDRAGITEM_H

#include "abstractcarddragitem.h"

class CardItem;

class CardDragItem : public AbstractCardDragItem
{
    Q_OBJECT
private:
    int id;
    bool faceDown;
    bool occupied;
    CardZone *currentZone;

public:
    CardDragItem(CardItem *_item,
                 int _id,
                 const QPointF &_hotSpot,
                 bool _faceDown,
                 AbstractCardDragItem *parentDrag = 0);
    int getId() const
    {
        return id;
    }
    bool getFaceDown() const
    {
        return faceDown;
    }
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void updatePosition(const QPointF &cursorScenePos);

protected:
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
};

#endif
