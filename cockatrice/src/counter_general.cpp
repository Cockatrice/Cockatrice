#include "counter_general.h"
#include <QPainter>

GeneralCounter::GeneralCounter(Player *_player, int _id, const QString &_name, const QColor &_color, int _radius, int _value, QGraphicsItem *parent)
	: AbstractCounter(_player, _id, _name, true, _value, parent), color(_color), radius(_radius)
{
}

QRectF GeneralCounter::boundingRect() const
{
	return QRectF(0, 0, radius * 2, radius * 2);
}

void GeneralCounter::paint(QPainter *painter, const QStyleOptionGraphicsItem */*option*/, QWidget */*widget*/)
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

