#ifndef COUNTER_H
#define COUNTER_H

#include <QGraphicsItem>

class Player;

class Counter : public QGraphicsItem {
private:
	QString name;
	QColor color;
	int value;
protected:
	Player *player;
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
public:
	Counter(Player *_player, const QString &_name, QColor _color, int _value, QGraphicsItem *parent = 0);
	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	
	QString getName() const { return name; }
	int getValue() const { return value; }
	void setValue(int _value);
};

#endif
