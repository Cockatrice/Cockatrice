/**
 * @file arrow_target.h
 * @ingroup GameGraphics
 * @brief TODO: Document this.
 */

#ifndef ARROWTARGET_H
#define ARROWTARGET_H

#include "abstract_graphics_item.h"

#include <QList>

class Player;
class ArrowItem;

class ArrowTarget : public AbstractGraphicsItem
{
    Q_OBJECT
protected:
    Player *owner;

private:
    bool beingPointedAt;
    QList<ArrowItem *> arrowsFrom, arrowsTo;

public:
    explicit ArrowTarget(Player *_owner, QGraphicsItem *parent = nullptr);
    ~ArrowTarget() override;

    Player *getOwner() const
    {
        return owner;
    }

    void setBeingPointedAt(bool _beingPointedAt);
    bool getBeingPointedAt() const
    {
        return beingPointedAt;
    }

    const QList<ArrowItem *> &getArrowsFrom() const
    {
        return arrowsFrom;
    }
    void addArrowFrom(ArrowItem *arrow)
    {
        arrowsFrom.append(arrow);
    }
    void removeArrowFrom(ArrowItem *arrow)
    {
        arrowsFrom.removeOne(arrow);
    }
    const QList<ArrowItem *> &getArrowsTo() const
    {
        return arrowsTo;
    }
    void addArrowTo(ArrowItem *arrow)
    {
        arrowsTo.append(arrow);
    }
    void removeArrowTo(ArrowItem *arrow)
    {
        arrowsTo.removeOne(arrow);
    }

protected:
    QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) override;
};
#endif
