#ifndef COUNTER_H
#define COUNTER_H

#include <QGraphicsItem>

class Player;

class Counter : public QGraphicsItem {
private:
	int id;
	QString name;
	QColor color;
	int radius;
	int value;
protected:
	Player *player;
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
public:
	Counter(Player *_player, int _id, const QString &_name, QColor _color, int _radius, int _value, QGraphicsItem *parent = 0);
	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	
	int getId() const { return id; }
	QString getName() const { return name; }
	int getValue() const { return value; }
	void setValue(int _value);
};

#endif
