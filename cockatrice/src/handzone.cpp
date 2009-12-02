#include <QtGui>
#include "handzone.h"
#include "player.h"
#include "client.h"
#include "protocol_items.h"

HandZone::HandZone(Player *_p, int _zoneHeight, QGraphicsItem *parent)
	: CardZone(_p, "hand", false, false, _p->getLocal(), parent), zoneHeight(_zoneHeight)
{
	QSettings settings;
	QString bgPath = settings.value("zonebg/hand").toString();
	if (!bgPath.isEmpty())
		bgPixmap.load(bgPath);
	
	setCacheMode(DeviceCoordinateCache);
	setAcceptsHoverEvents(true); // Awkwardly, this is needed to repaint the cached item after it has been corrupted by buggy rubberband drag.
}

QRectF HandZone::boundingRect() const
{
	return QRectF(0, 0, 100, zoneHeight);
}

void HandZone::paint(QPainter *painter, const QStyleOptionGraphicsItem */*option*/, QWidget */*widget*/)
{
	if (bgPixmap.isNull())
		painter->fillRect(boundingRect(), Qt::darkGreen);
	else
		painter->fillRect(boundingRect(), QBrush(bgPixmap));
}

void HandZone::reorganizeCards()
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

void HandZone::addCardImpl(CardItem *card, int x, int /*y*/)
{
	if (x == -1)
		x = cards.size();
	cards.insert(x, card);

	if (!cards.getContentsKnown()) {
		card->setId(-1);
		card->setName();
	}
	card->setParentItem(this);
	card->resetState();
	card->setVisible(true);
	card->update();
}

void HandZone::handleDropEvent(int cardId, CardZone *startZone, const QPoint &/*dropPoint*/, bool /*faceDown*/)
{
	player->sendGameCommand(new Command_MoveCard(-1, startZone->getName(), cardId, getName(), cards.size(), -1, false));
}
