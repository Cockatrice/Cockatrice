#ifndef PLAYERAREA_H
#define PLAYERAREA_H

#include <QList>
#include "carditem.h"

class Player;
class Counter;

class PlayerArea : public QGraphicsItem {
private:
	QRectF bRect;
	Player *player;
	QList<Counter *> counterList;
	
	void rearrangeCounters();
public:
	enum { Type = typeOther };
	int type() const { return Type; }
	PlayerArea(Player *_player, QGraphicsItem *parent = 0);
	~PlayerArea();
	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	
	Counter *getCounter(const QString &name, bool remove = false);
	void addCounter(const QString &name, QColor color, int value);
	void delCounter(const QString &name);
	void clearCounters();
};

#endif
