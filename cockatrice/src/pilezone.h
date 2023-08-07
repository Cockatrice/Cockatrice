#ifndef PILEZONE_H
#define PILEZONE_H

#include "cardzone.h"

class PileZone : public CardZone
{
    Q_OBJECT
private slots:
    void callUpdate()
    {
        update();
    }

public:
    PileZone(Player *_p,
             const QString &_name,
             bool _isShufflable,
             bool _contentsKnown,
             QGraphicsItem *parent = nullptr);
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void reorganizeCards() override;
    void handleDropEvent(const QList<CardDragItem *> &dragItems, CardZone *startZone, const QPoint &dropPoint) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void addCardImpl(CardItem *card, int x, int y) override;
};

#endif
