#ifndef COUNTER_GENERAL_H
#define COUNTER_GENERAL_H

#include "abstract_counter.h"

class GeneralCounter : public AbstractCounter
{
    Q_OBJECT
private:
    QColor color;
    int radius;

public:
    GeneralCounter(Player *_player,
                   int _id,
                   const QString &_name,
                   const QColor &_color,
                   int _radius,
                   int _value,
                   bool useNameForShortcut = false,
                   QGraphicsItem *parent = nullptr,
                   QWidget *game = nullptr);
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};

#endif
