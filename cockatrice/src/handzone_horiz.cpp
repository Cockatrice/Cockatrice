#include <QPainter>
#include "handzone_horiz.h"
#include "player.h"

HandZoneHoriz::HandZoneHoriz(Player *_p, bool _contentsKnown, QGraphicsItem *parent)
	: HandZone(_p, _contentsKnown, parent), width(CARD_WIDTH * 10)
{
	setCacheMode(DeviceCoordinateCache);
}

QRectF HandZoneHoriz::boundingRect() const
{
	return QRectF(0, 0, width, CARD_HEIGHT + 10);
}

void HandZoneHoriz::paint(QPainter *painter, const QStyleOptionGraphicsItem */*option*/, QWidget */*widget*/)
{
	if (bgPixmap.isNull())
		painter->fillRect(boundingRect(), Qt::darkGreen);
	else
		painter->fillRect(boundingRect(), QBrush(bgPixmap));
}

void HandZoneHoriz::reorganizeCards()
{
	if (!cards.isEmpty()) {
		const int cardCount = cards.size();
		const int xPadding = 5;
		qreal totalWidth = boundingRect().width() - 2 * xPadding;
		qreal cardWidth = cards.at(0)->boundingRect().width();
	
		for (int i = 0; i < cardCount; i++) {
			CardItem *c = cards.at(i);

			// If the total width of the cards is smaller than the available width,
			// the cards do not need to overlap and are displayed in the center of the area.
			if (cardWidth * cardCount > totalWidth)
				c->setPos(xPadding + ((qreal) i) * (totalWidth - cardWidth) / (cardCount - 1), 5);
			else
				c->setPos(xPadding + ((qreal) i) * cardWidth + (totalWidth - cardCount * cardWidth) / 2, 5);
			c->setZValue(i);
		}
	}
	update();
}

void HandZoneHoriz::setWidth(qreal _width)
{
	prepareGeometryChange();
	width = _width;
	reorganizeCards();
}
