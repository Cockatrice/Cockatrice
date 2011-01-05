#ifndef HANDCOUNTER_H
#define HANDCOUNTER_H

#include <QString>
#include "abstractcarditem.h"

class QPainter;
class QPixmap;

class HandCounter : public AbstractGraphicsItem {
	Q_OBJECT
private:
	int number;
public slots:
	void updateNumber();
public:
	enum { Type = typeOther };
	int type() const { return Type; }
	HandCounter(QGraphicsItem *parent = 0);
	~HandCounter();
	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};

#endif
 
