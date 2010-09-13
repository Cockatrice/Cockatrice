#include <QPainter>
#include <QSvgRenderer>
#include <QPixmapCache>
#include "handcounter.h"
#include "cardzone.h"

HandCounter::HandCounter(QGraphicsItem *parent)
	: AbstractGraphicsItem(parent), number(0)
{
	setCacheMode(DeviceCoordinateCache);
}

HandCounter::~HandCounter()
{
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
	painter->save();
	QSize translatedSize = painter->combinedTransform().mapRect(boundingRect()).size().toSize();
	QPixmap cachedPixmap;
#if QT_VERSION >= 0x040600
	if (!QPixmapCache::find("handCounter" + QString::number(translatedSize.width()), &cachedPixmap)) {
#else
	if (!QPixmapCache::find("handCounter" + QString::number(translatedSize.width()), cachedPixmap)) {
#endif
		QSvgRenderer svg(QString(":/resources/hand.svg"));
		cachedPixmap = QPixmap(translatedSize);
		cachedPixmap.fill(Qt::transparent);
		QPainter painter(&cachedPixmap);
		svg.render(&painter, QRectF(0, 0, translatedSize.width(), translatedSize.height()));
		QPixmapCache::insert("handCounter" + QString::number(translatedSize.width()), cachedPixmap);
	}
	painter->resetTransform();
	painter->drawPixmap(cachedPixmap.rect(), cachedPixmap, cachedPixmap.rect());
	painter->restore();
	
	paintNumberEllipse(number, 24, Qt::white, -1, -1, painter);
}
