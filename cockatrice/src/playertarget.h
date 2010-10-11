#ifndef PLAYERTARGET_H
#define PLAYERTARGET_H

#include "arrowtarget.h"
#include <QFont>
#include <QPixmap>

class Player;

class PlayerTarget : public ArrowTarget {
private:
	QPixmap fullPixmap;
public:
	enum { Type = typePlayerTarget };
	int type() const { return Type; }
	
	PlayerTarget(Player *parent = 0);
	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};

#endif