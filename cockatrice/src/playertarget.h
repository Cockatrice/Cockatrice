#ifndef PLAYERTARGET_H
#define PLAYERTARGET_H

#include "arrowtarget.h"
#include <QFont>

class Player;

class PlayerTarget : public ArrowTarget {
private:
	QString name;
	QFont font;
	int maxWidth;
public:
	enum { Type = typePlayerTarget };
	int type() const { return Type; }
	
	PlayerTarget(const QString &_name, int _maxWidth, Player *parent = 0);
	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};

#endif