#include <QtGui>
#include "handzone.h"
#include "player.h"
#include "client.h"

HandZone::HandZone(Player *_p, QGraphicsItem *parent)
	: CardZone(_p, "hand", false, false, parent)
{
	cards = new CardList(player->getLocal());
}

QRectF HandZone::boundingRect() const
{
	return QRectF(0, 0, 100, 510);
}

void HandZone::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	painter->fillRect(boundingRect(), QColor("green"));
}

void HandZone::reorganizeCards()
{
	if (cards->isEmpty())
		return;
	
	int cardCount = cards->size();
	qreal totalWidth = boundingRect().width();
	qreal totalHeight = boundingRect().height();
	qreal cardWidth = cards->at(0)->boundingRect().width();
	qreal cardHeight = cards->at(0)->boundingRect().height();
	qreal x = (totalWidth - cardWidth) / 2;
	
	for (int i = 0; i < cardCount; i++) {
		CardItem *c = cards->at(i);
		// If the total height of the cards is smaller than the available height,
		// the cards do not need to overlap and are displayed in the center of the area.
		if (cardHeight * cardCount > totalHeight)
			c->setPos(x, ((qreal) i) * (totalHeight - cardHeight) / (cardCount - 1));
		else
			c->setPos(x, ((qreal) i) * cardHeight + (totalHeight - cardCount * cardHeight) / 2);
		c->setZValue(i);
	}
}

void HandZone::addCard(CardItem *card, bool reorganize, int x, int y)
{
	Q_UNUSED(y);
	
	if (x == -1)
		x = cards->size();
	cards->insert(x, card);
	
	if (!cards->getContentsKnown()) {
		card->setId(-1);
		card->setName();
	}
	card->setParentItem(this);
	card->resetState();
	card->setVisible(true);
	card->update(card->boundingRect());

	if (reorganize)
		reorganizeCards();
}

void HandZone::handleDropEvent(int cardId, CardZone *startZone, const QPoint &dropPoint, bool faceDown)
{
	Q_UNUSED(dropPoint);
	player->client->moveCard(cardId, startZone->getName(), getName(), cards->size(), 0);
}
