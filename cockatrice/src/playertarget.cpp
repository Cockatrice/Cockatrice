#include "playertarget.h"
#include <QPainter>

PlayerTarget::PlayerTarget(const QString &_name, int _maxWidth, QGraphicsItem *parent)
	: ArrowTarget(parent), name(_name), maxWidth(_maxWidth)
{
	font = QFont("Times");
	font.setStyleHint(QFont::Serif);
	font.setPixelSize(20);
}

QRectF PlayerTarget::boundingRect() const
{
	return QRectF(0, 0, maxWidth, 30);
}

void PlayerTarget::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
	painter->fillRect(boundingRect(), QColor(255, 255, 255, 100));
	painter->setFont(font);
	painter->setPen(Qt::black);
	painter->drawText(boundingRect(), Qt::AlignCenter, name);
}
