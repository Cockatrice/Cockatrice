/**
 * @file hand_zone.h
 * @ingroup GameGraphicsZones
 * @brief TODO: Document this.
 */

#ifndef HANDZONE_H
#define HANDZONE_H

#include "logic/hand_zone_logic.h"
#include "select_zone.h"

class HandZone : public SelectZone
{
    Q_OBJECT
private:
    qreal width, zoneHeight;
private slots:
    void updateBg();
public slots:
    void updateOrientation();

public:
    HandZone(HandZoneLogic *_logic, int _zoneHeight, QGraphicsItem *parent = nullptr);
    void
    handleDropEvent(const QList<CardDragItem *> &dragItems, CardZoneLogic *startZone, const QPoint &dropPoint) override;
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void reorganizeCards() override;
    void sortHand();
    void setWidth(qreal _width);
};

#endif
