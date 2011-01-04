#ifndef COUNTER_GENERAL_H
#define COUNTER_GENERAL_H

#include "abstractcounter.h"

class GeneralCounter : public AbstractCounter {
	Q_OBJECT
private:
	QColor color;
	int radius;
public:
	GeneralCounter(Player *_player, int _id, const QString &_name, const QColor &_color, int _radius, int _value, QGraphicsItem *parent = 0);
	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};

#endif