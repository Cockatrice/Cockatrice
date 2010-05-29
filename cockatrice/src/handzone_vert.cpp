#include <QtGui>
#include "handzone_vert.h"
#include "player.h"
#include "client.h"
#include "protocol_items.h"
#include "settingscache.h"

HandZoneVert::HandZoneVert(Player *_p, bool _contentsKnown, int _zoneHeight, QGraphicsItem *parent)
	: HandZone(_p, _contentsKnown, parent), zoneHeight(_zoneHeight)
{
	setCacheMode(DeviceCoordinateCache);
}

QRectF HandZoneVert::boundingRect() const
{
	return QRectF(0, 0, 100, zoneHeight);
}

void HandZoneVert::paint(QPainter *painter, const QStyleOptionGraphicsItem */*option*/, QWidget */*widget*/)
{
	if (bgPixmap.isNull())
		painter->fillRect(boundingRect(), Qt::darkGreen);
	else
		painter->fillRect(boundingRect(), QBrush(bgPixmap));
}

void HandZoneVert::reorganizeCards()
{
	if (!cards.isEmpty()) {
		const int cardCount = cards.size();
		qreal totalWidth = boundingRect().width();
		qreal totalHeight = boundingRect().height();
		qreal cardWidth = cards.at(0)->boundingRect().width();
		qreal cardHeight = cards.at(0)->boundingRect().height();
		qreal xspace = 5;
		qreal x1 = xspace;
		qreal x2 = totalWidth - xspace - cardWidth;
	
		for (int i = 0; i < cardCount; i++) {
			CardItem *c = cards.at(i);
			qreal x = i % 2 ? x2 : x1;
			// If the total height of the cards is smaller than the available height,
			// the cards do not need to overlap and are displayed in the center of the area.
			if (cardHeight * cardCount > totalHeight)
				c->setPos(x, ((qreal) i) * (totalHeight - cardHeight) / (cardCount - 1));
			else
				c->setPos(x, ((qreal) i) * cardHeight + (totalHeight - cardCount * cardHeight) / 2);
			c->setZValue(i);
		}
	}
	update();
}
