#ifndef PLAYERAREA_H
#define PLAYERAREA_H

#include "carditem.h"

class Player;

class PlayerArea : public QGraphicsItem {
private:
	QRectF bRect;
	Player *player;
public:
	enum { Type = typeOther };
	int type() const { return Type; }
	PlayerArea(Player *_player, QGraphicsItem *parent = 0);
	~PlayerArea();
	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};

#endif
