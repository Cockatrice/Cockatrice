#ifndef PLAYERTARGET_H
#define PLAYERTARGET_H

#include "arrowtarget.h"
#include <QFont>

class Player;

class PlayerTarget : public ArrowTarget {
private:
	QFont font;
public:
	enum { Type = typePlayerTarget };
	int type() const { return Type; }
	
	PlayerTarget(Player *parent = 0);
	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};

#endif