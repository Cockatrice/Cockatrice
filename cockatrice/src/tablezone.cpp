#include <QtGui>
#include "tablezone.h"
#include "player.h"
#include "client.h"

TableZone::TableZone(Player *_p)
	: PlayerZone(_p, "table"), width(700), height(500)
{
	cards = new CardList(true);
	hasCardAttr = true;
}

QRectF TableZone::boundingRect() const
{
	return QRectF(0, 0, width, height);
}

void TableZone::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	painter->fillRect(boundingRect(), QColor("blue"));
}

void TableZone::addCard(CardItem *card, bool reorganize, int x, int y)
{
	Q_UNUSED(reorganize);
	cards->append(card);
	if ((x != -1) && (y != -1)) {
		if (!player->getLocal())
			y = height - CARD_HEIGHT - y;
		card->setPos(x, y);
	}
	card->setZValue((y + CARD_HEIGHT) * width + x + 1000);
	card->setParentItem(this);
	card->setVisible(true);
	card->update(card->boundingRect());
}

void TableZone::handleDropEvent(int cardId, PlayerZone *startZone, const QPoint &dropPoint)
{
	player->client->moveCard(cardId, startZone->getName(), getName(), dropPoint.x(), dropPoint.y());
}

void TableZone::reorganizeCards()
{
}

void TableZone::toggleTapped()
{
	QListIterator<QGraphicsItem *> i(scene()->selectedItems());
	while (i.hasNext()) {
		CardItem *temp = (CardItem *) i.next();
		setCardAttr(temp->getId(), "tapped", temp->getTapped() ? "0" : "1");
	}
}
