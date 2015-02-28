#ifndef HANDZONE_H
#define HANDZONE_H

#include "selectzone.h"

class HandZone : public SelectZone {
    Q_OBJECT
private:
    qreal width, zoneHeight;
private slots:
    void updateBg();
public slots:
    void updateOrientation();
public:
    HandZone(Player *_p, bool _contentsKnown, int _zoneHeight, QGraphicsItem *parent = 0);
    void handleDropEvent(const QList<CardDragItem *> &dragItems, CardZone *startZone, const QPoint &dropPoint);
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void reorganizeCards();
    void setWidth(qreal _width);
protected:
    void addCardImpl(CardItem *card, int x, int y);
};

#endif

