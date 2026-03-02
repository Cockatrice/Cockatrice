/**
 * @file stack_zone.h
 * @ingroup GameGraphicsZones
 * @brief TODO: Document this.
 */

#ifndef STACKZONE_H
#define STACKZONE_H

#include "logic/stack_zone_logic.h"
#include "select_zone.h"

class StackZone : public SelectZone
{
    Q_OBJECT
private:
    qreal zoneHeight;
private slots:
    void updateBg();

public:
    StackZone(StackZoneLogic *_logic, int _zoneHeight, QGraphicsItem *parent);
    void
    handleDropEvent(const QList<CardDragItem *> &dragItems, CardZoneLogic *startZone, const QPoint &dropPoint) override;
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void reorganizeCards() override;
};

#endif
