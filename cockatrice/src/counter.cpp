#include "counter.h"
#include "player.h"
#include "client.h"
#include <QtGui>

Counter::Counter(Player *_player, int _id, const QString &_name, QColor _color, int _radius, int _value, QGraphicsItem *parent)
	: QGraphicsItem(parent), id(_id), name(_name), color(_color), radius(_radius), value(_value), player(_player)
{
	if (radius > Player::counterAreaWidth / 2)
		radius = Player::counterAreaWidth / 2;
}

QRectF Counter::boundingRect() const
{
	return QRectF(0, 0, radius * 2, radius * 2);
}

void Counter::paint(QPainter *painter, const QStyleOptionGraphicsItem */*option*/, QWidget */*widget*/)
{
	painter->setBrush(QBrush(color));
	painter->drawEllipse(boundingRect());
	if (value) {
		QFont f("Serif");
		f.setPixelSize(radius * 0.8);
		f.setWeight(QFont::Bold);
		painter->setFont(f);
		painter->drawText(boundingRect(), Qt::AlignCenter, QString::number(value));
	}
}

void Counter::setValue(int _value)
{
	value = _value;
	update();
}

void Counter::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
		player->client->incCounter(id, 1);
	else if (event->button() == Qt::RightButton)
		player->client->incCounter(id, -1);
}
