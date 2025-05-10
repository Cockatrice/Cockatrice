#ifndef SELECTZONE_H
#define SELECTZONE_H

#include "card_zone.h"

#include <QSet>

/**
 * A CardZone where the cards are laid out, with each card directly interactable by clicking.
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
    SelectZone(Player *_player,
               const QString &_name,
               bool _hasCardAttr,
               bool _isShufflable,
               bool _contentsKnown,
               QGraphicsItem *parent = nullptr);
};

qreal divideCardSpaceInZone(qreal index, int cardCount, qreal totalHeight, qreal cardHeight, bool reverse = false);

#endif
