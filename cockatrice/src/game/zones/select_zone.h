#ifndef SELECTZONE_H
#define SELECTZONE_H

#include "card_zone.h"

/**
 * A CardZone where the cards are laid out, with each card directly interactable by clicking.
 */
class SelectZone : public CardZone
{
    Q_OBJECT
private:
    QPointF selectionOrigin;

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

public:
    SelectZone(Player *_player,
               const QString &_name,
               bool _hasCardAttr,
               bool _isShufflable,
               bool _contentsKnown,
               QGraphicsItem *parent = nullptr,
               bool isView = false);
};

qreal divideCardSpaceInZone(qreal index, int cardCount, qreal totalHeight, qreal cardHeight, bool reverse = false);

#endif
