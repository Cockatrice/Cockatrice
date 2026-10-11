/**
 * @file abstract_card_drag_item.h
 * @ingroup GameGraphicsCards
 */
//! \todo Document this file.

#ifndef ABSTRACTCARDDRAGITEM_H
#define ABSTRACTCARDDRAGITEM_H

#include "../card_dimensions.h"
#include "graphics_item_type.h"

#include <QGraphicsItem>
#include <QList>
#include <QObject>
#include <QPointF>
#include <QRectF>
#include <qtmetamacros.h>

class AbstractCardItem;

class AbstractCardDragItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
protected:
    AbstractCardItem *item;
    QPointF hotSpot;
    QList<AbstractCardDragItem *> childDrags;

public:
    enum
    {
        Type = typeCardDrag
    };
    [[nodiscard]] int type() const override
    {
        return Type;
    }
    AbstractCardDragItem(AbstractCardItem *_item, const QPointF &_hotSpot, AbstractCardDragItem *parentDrag = 0);
    [[nodiscard]] QRectF boundingRect() const override
    {
        return QRectF(0, 0, CardDimensions::WIDTH_F, CardDimensions::HEIGHT_F);
    }
    [[nodiscard]] QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    [[nodiscard]] AbstractCardItem *getItem() const
    {
        return item;
    }
    [[nodiscard]] QPointF getHotSpot() const
    {
        return hotSpot;
    }
    void addChildDrag(AbstractCardDragItem *child);
    virtual void updatePosition(const QPointF &cursorScenePos) = 0;

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
};

#endif
