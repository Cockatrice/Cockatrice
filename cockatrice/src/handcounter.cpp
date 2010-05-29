#include <QPainter>
#include <QSvgRenderer>
#include "handcounter.h"
#include "cardzone.h"

HandCounter::HandCounter(QGraphicsItem *parent)
	: AbstractGraphicsItem(parent), number(0)
{
	setCacheMode(DeviceCoordinateCache);
	
	QSvgRenderer svg(QString(":/resources/hand.svg"));
	handImage = new QPixmap(72, 72);
	handImage->fill(Qt::transparent);
	QPainter painter(handImage);
	svg.render(&painter, QRectF(0, 0, 72, 72));
}

HandCounter::~HandCounter()
{
	delete handImage;
}

void HandCounter::updateNumber()
{
	number = static_cast<CardZone *>(sender())->getCards().size();
	update();
}

QRectF HandCounter::boundingRect() const
{
	return QRectF(0, 0, 72, 72);
}

void HandCounter::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
	painter->drawPixmap(handImage->rect(), *handImage, handImage->rect());
	paintNumberEllipse(number, painter);
}
