/**
 * @file arrow_target.h
 * @ingroup GameGraphics
 */
//! \todo Document this file.

#ifndef ARROWTARGET_H
#define ARROWTARGET_H

#include "../../game_graphics/board/abstract_graphics_item.h"

#include <QList>

class PlayerLogic;
class ArrowItem;

class ArrowTarget : public AbstractGraphicsItem
{
    Q_OBJECT
protected:
    PlayerLogic *owner;

private:
    bool beingPointedAt = false;

signals:
    void scenePositionChanged();

public:
    explicit ArrowTarget(PlayerLogic *_owner, QGraphicsItem *parent = nullptr);
    ~ArrowTarget() override = default;

    [[nodiscard]] PlayerLogic *getOwner() const
    {
        return owner;
    }

    void setBeingPointedAt(bool _beingPointedAt);
    [[nodiscard]] bool getBeingPointedAt() const
    {
        return beingPointedAt;
    }

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
};
#endif
