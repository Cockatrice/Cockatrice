#include <QtGui>
#include "tablezone.h"
#include "player.h"
#include "client.h"

TableZone::TableZone(Player *_p, QGraphicsItem *parent)
	: CardZone(_p, "table", parent), width(720), height(510)
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
	painter->fillRect(boundingRect(), QColor(0, 0, 100));
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
	qDebug(QString("table: appended %1 at pos %2: zValue = %3, x = %4, y = %5").arg(card->getName()).arg(cards->size() - 1).arg(card->zValue()).arg(x).arg(y).toLatin1());
	card->setParentItem(this);
	card->setVisible(true);
	card->update(card->boundingRect());
}

void TableZone::handleDropEvent(int cardId, CardZone *startZone, const QPoint &dropPoint, bool faceDown)
{
	player->client->moveCard(cardId, startZone->getName(), getName(), dropPoint.x(), dropPoint.y(), faceDown);
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
