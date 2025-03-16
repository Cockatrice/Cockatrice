#ifndef CARDDRAGITEM_H
#define CARDDRAGITEM_H

#include "abstract_card_drag_item.h"

class CardItem;

class CardDragItem : public AbstractCardDragItem
{
    Q_OBJECT
private:
    int id;
    bool faceDown;
    bool isValid = true;
    CardZone *currentZone;

public:
    CardDragItem(CardItem *_item,
                 int _id,
                 const QPointF &_hotSpot,
                 bool _faceDown,
                 AbstractCardDragItem *parentDrag = 0);
    virtual ~CardDragItem();
    int getId() const
    {
        return id;
    }
    bool getFaceDown() const
    {
        return faceDown;
    }
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void updatePosition(const QPointF &cursorScenePos) override;

    void setValid(bool _valid)
    {
        if (_valid != isValid) {
            isValid = _valid;
            update();
        }
    }

    CardZone *getStartZone() const;

    QList<CardDragItem *> getValidItems();

protected:
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
};

#endif
