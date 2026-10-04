/**
 * @file stack_zone.h
 * @ingroup GameGraphicsZones
 * @brief Graphical zone for the stack, displaying cards in a vertical pile.
 */

#ifndef STACKZONE_H
#define STACKZONE_H

#include "select_zone.h"

#include <qtmetamacros.h>
#include <qtypes.h>

class QGraphicsItem;
class StackZoneLogic;

class StackZone : public SelectZone
{
    Q_OBJECT
private:
    qreal zoneHeight;
    bool playmatActive = false;
private slots:
    void updateBg();

public slots:
    void onPlaymatChanged(bool active);

public:
    StackZone(StackZoneLogic *_logic, int _zoneHeight, QGraphicsItem *parent);
    /** @brief Resizes the stack zone height, e.g. when sharing vertical space with the command zone. */
    void setHeight(qreal newHeight);
    void
    handleDropEvent(const QList<CardDragItem *> &dragItems, CardZoneLogic *startZone, const QPoint &dropPoint) override;
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void reorganizeCards() override;
};

#endif
