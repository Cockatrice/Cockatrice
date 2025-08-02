#ifndef HANDZONE_H
#define HANDZONE_H

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
    HandZone(Player *_p, bool _contentsKnown, int _zoneHeight, QGraphicsItem *parent = nullptr);
    void handleDropEvent(const QList<CardDragItem *> &dragItems, CardZone *startZone, const QPoint &dropPoint) override;
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void reorganizeCards() override;
    void sortHand();
    void setWidth(qreal _width);

protected:
    void addCardImpl(CardItem *card, int x, int y) override;
};

#endif
