#ifndef STACKZONE_H
#define STACKZONE_H

#include "select_zone.h"

inline Q_LOGGING_CATEGORY(StackZoneLog, "stack_zone");

class StackZone : public SelectZone
{
    Q_OBJECT
private:
    qreal zoneHeight;
private slots:
    void updateBg();

public:
    StackZone(Player *_p, int _zoneHeight, QGraphicsItem *parent = nullptr);
    void handleDropEvent(const QList<CardDragItem *> &dragItems, CardZone *startZone, const QPoint &dropPoint) override;
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void reorganizeCards() override;

protected:
    void addCardImpl(CardItem *card, int x, int y) override;
};

#endif
