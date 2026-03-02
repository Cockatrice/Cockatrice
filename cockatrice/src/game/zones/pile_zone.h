/**
 * @file pile_zone.h
 * @ingroup GameGraphicsZones
 * @brief TODO: Document this.
 */

#ifndef PILEZONE_H
#define PILEZONE_H

#include "card_zone.h"
#include "logic/pile_zone_logic.h"

/**
 * A CardZone where the cards are in a single pile instead of being laid out.
 * Usually only top card is accessible by clicking.
 */
class PileZone : public CardZone
{
    Q_OBJECT
private slots:
    void callUpdate()
    {
        update();
    }

public:
    PileZone(PileZoneLogic *_logic, QGraphicsItem *parent);
    [[nodiscard]] QRectF boundingRect() const override;
    [[nodiscard]] QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void reorganizeCards() override;
    void
    handleDropEvent(const QList<CardDragItem *> &dragItems, CardZoneLogic *startZone, const QPoint &dropPoint) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
};

#endif
