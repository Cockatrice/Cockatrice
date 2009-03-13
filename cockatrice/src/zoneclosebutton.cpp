#include <QtGui>
#include "zoneclosebutton.h"

void ZoneCloseButton::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	emit triggered();
}

void ZoneCloseButton::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	painter->fillRect(rect(), QColor("red"));
}

ZoneCloseButton::ZoneCloseButton(QGraphicsItem *parent)
	: QGraphicsWidget(parent)
{
	resize(20, 20);
}
