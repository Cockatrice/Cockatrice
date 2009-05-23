#include "counter.h"
#include "player.h"
#include "client.h"
#include <QtGui>

Counter::Counter(Player *_player, const QString &_name, QColor _color, int _value, QGraphicsItem *parent)
	: QGraphicsItem(parent), name(_name), color(_color), value(_value), player(_player)
{
}

QRectF Counter::boundingRect() const
{
	return QRectF(0, 0, 40, 40);
}

void Counter::paint(QPainter *painter, const QStyleOptionGraphicsItem */*option*/, QWidget */*widget*/)
{
	painter->save();
	painter->setBrush(QBrush(color));
	painter->drawEllipse(boundingRect());
	if (value) {
		painter->setFont(QFont("Times", 16, QFont::Bold));
		painter->drawText(boundingRect(), Qt::AlignCenter, QString::number(value));
	}
	painter->restore();
}

void Counter::setValue(int _value)
{
	value = _value;
	update(boundingRect());
}

void Counter::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
		player->client->incCounter(name, 1);
	else if (event->button() == Qt::RightButton)
		player->client->incCounter(name, -1);
}
