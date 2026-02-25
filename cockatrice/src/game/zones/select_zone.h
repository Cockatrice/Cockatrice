/**
 * @file select_zone.h
 * @ingroup GameGraphicsZones
 * @brief Base class for zones with directly clickable, laid-out cards.
 */

#ifndef SELECTZONE_H
#define SELECTZONE_H

#include "card_zone.h"

#include <QSet>

/**
 * @class SelectZone
 * @brief Base class for zones where cards are laid out and directly clickable.
 *
 * SelectZone provides mouse interaction handling for zones that display cards
 * in a visible layout (as opposed to hidden zones like the deck). It supports
 * click-to-select and rectangle selection via click-and-drag.
 *
 * Subclasses include TableZone, HandZone, and CommandZone.
 *
 * @see CardZone
 */
class SelectZone : public CardZone
{
    Q_OBJECT
private:
    QPointF selectionOrigin;
    QSet<CardItem *> cardsInSelectionRect;

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

public:
    SelectZone(CardZoneLogic *logic, QGraphicsItem *parent = nullptr);
};

qreal divideCardSpaceInZone(qreal index, int cardCount, qreal totalHeight, qreal cardHeight, bool reverse = false);

#endif
