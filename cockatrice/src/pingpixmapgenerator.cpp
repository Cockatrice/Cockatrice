#include "pingpixmapgenerator.h"
#include <QPainter>

QMap<int, QPixmap> PingPixmapGenerator::pmCache;

QPixmap PingPixmapGenerator::generatePixmap(int size, int value, int max)
{
	int key = size * 1000000 + max * 1000 + value;
	if (pmCache.contains(key))
		return pmCache.value(key);
	
	QPixmap pixmap(size, size);
	pixmap.fill(Qt::transparent);
	QPainter painter(&pixmap);
	QColor color;
	if ((max == -1) || (value == -1))
		color = Qt::black;
	else
		color.setHsv(120 * (1.0 - ((double) value / max)), 255, 255);
	
	QRadialGradient g(QPointF((double) pixmap.width() / 2, (double) pixmap.height() / 2), qMin(pixmap.width(), pixmap.height()) / 2.0);
	g.setColorAt(0, color);
	g.setColorAt(1, Qt::transparent);
	painter.fillRect(0, 0, pixmap.width(), pixmap.height(), QBrush(g));
	
	pmCache.insert(key, pixmap);

	return pixmap;
}
