#include "counter.h"
#include "player.h"
#include <QtGui>

Counter::Counter(Player *_player, const QString &_name, QGraphicsItem *parent)
	: QGraphicsItem(parent), name(_name), value(0), player(_player)
{
	player->addCounter(this);
}

QRectF Counter::boundingRect() const
{
	return QRectF(0, 0, 50, 30);
}

void Counter::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	painter->save();
	painter->fillRect(boundingRect(), QBrush(QColor("gray")));
	painter->drawText(boundingRect(), Qt::AlignCenter, QString("%1").arg(value));
	painter->restore();
}

void Counter::setValue(int _value)
{
	value = _value;
	update(boundingRect());
}
