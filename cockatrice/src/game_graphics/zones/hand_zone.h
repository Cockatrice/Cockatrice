/**
 * @file hand_zone.h
 * @ingroup GameGraphicsZones
 * @brief Graphical zone for the player's hand, supporting horizontal and vertical layouts.
 */

#ifndef HANDZONE_H
#define HANDZONE_H

#include "../../game/board/card_list.h"
#include "select_zone.h"

#include <QtTypes>
#include <qtmetamacros.h>

class HandZoneLogic;
class QGraphicsItem;
template <typename T> class QList;

class HandZone : public SelectZone
{
    Q_OBJECT
private:
    qreal width = 0.0;
    qreal zoneHeight;
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
    void sortHand(const QList<CardList::SortOption> &options);
    void setWidth(qreal _width);
};

#endif
