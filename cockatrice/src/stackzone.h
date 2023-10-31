#ifndef STACKZONE_H
#define STACKZONE_H

#include "selectzone.h"

class StackZone : public SelectZone
{
    Q_OBJECT
private:
    qreal zoneHeight;
    int cardCount;
    qreal cardLocationByIndex(qreal index, bool reverse = false);
private slots:
    void updateBg();

public:
    StackZone(Player *_p, int _zoneHeight, QGraphicsItem *parent = nullptr);
    void handleDropEvent(const QList<CardDragItem *> &dragItems, CardZone *startZone, const QPoint &dropPoint);
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void reorganizeCards();

protected:
    void addCardImpl(CardItem *card, int x, int y);
};

#endif
