#include "pixmapgenerator.h"
#include "protocol_datastructures.h"
#include <QPainter>
#include <QSvgRenderer>
#include <math.h>
#include <QDebug>

QMap<QString, QPixmap> CounterPixmapGenerator::pmCache;

QPixmap CounterPixmapGenerator::generatePixmap(int height, QString name, bool highlight)
{
	if (highlight)
		name.append("_highlight");
	QString key = name + QString::number(height);
	if (pmCache.contains(key))
		return pmCache.value(key);
	
	QSvgRenderer svg(QString(":/resources/counters/" + name + ".svg"));
	
	if (!svg.isValid()) {
		name = "general";
		if (highlight)
			name.append("_highlight");
		svg.load(QString(":/resources/counters/" + name + ".svg"));
	}
	
	int width = (int) round(height * (double) svg.defaultSize().width() / (double) svg.defaultSize().height());
	QPixmap pixmap(width, height);
	pixmap.fill(Qt::transparent);
	QPainter painter(&pixmap);
	svg.render(&painter, QRectF(0, 0, width, height));
	pmCache.insert(key, pixmap);
	return pixmap;
}

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

QMap<int, QPixmap> PingPixmapGenerator::pmCache;

QPixmap CountryPixmapGenerator::generatePixmap(int height, const QString &countryCode)
{
	if (countryCode.size() != 2)
		return QPixmap();
	QString key = countryCode + QString::number(height);
	if (pmCache.contains(key))
		return pmCache.value(key);
	
	QSvgRenderer svg(QString(":/resources/countries/" + countryCode + ".svg"));
	int width = (int) round(height * (double) svg.defaultSize().width() / (double) svg.defaultSize().height());
	QPixmap pixmap(width, height);
	pixmap.fill(Qt::transparent);
	QPainter painter(&pixmap);
	svg.render(&painter, QRectF(0, 0, width, height));
	painter.setPen(Qt::black);
	painter.drawRect(0.5, 0.5, width - 1, height - 1);
	
	pmCache.insert(key, pixmap);
	return pixmap;
}

QMap<QString, QPixmap> CountryPixmapGenerator::pmCache;

QPixmap UserLevelPixmapGenerator::generatePixmap(int height, int userLevel)
{
	int key = height * 10000 + userLevel;
	if (pmCache.contains(key))
		return pmCache.value(key);
	
	QString levelString;
	if (userLevel & ServerInfo_User::IsAdmin)
		levelString = "admin";
	else if (userLevel & ServerInfo_User::IsJudge)
		levelString = "judge";
	else if (userLevel &ServerInfo_User::IsRegistered)
		levelString = "registered";
	else
		levelString = "normal";
	QSvgRenderer svg(QString(":/resources/userlevels/" + levelString + ".svg"));
	int width = (int) round(height * (double) svg.defaultSize().width() / (double) svg.defaultSize().height());
	QPixmap pixmap(width, height);
	pixmap.fill(Qt::transparent);
	QPainter painter(&pixmap);
	svg.render(&painter, QRectF(0, 0, width, height));
	
	pmCache.insert(key, pixmap);
	return pixmap;
}

QMap<int, QPixmap> UserLevelPixmapGenerator::pmCache;
