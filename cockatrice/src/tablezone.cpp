#include <QtGui>
#include "tablezone.h"
#include "player.h"
#include "client.h"

TableZone::TableZone(Player *_p, QGraphicsItem *parent)
	: CardZone(_p, "table", true, false, parent), width(720), height(510)
{
	cards = new CardList(true);
}

QRectF TableZone::boundingRect() const
{
	return QRectF(0, 0, width, height);
}

void TableZone::paint(QPainter *painter, const QStyleOptionGraphicsItem */*option*/, QWidget */*widget*/)
{
	painter->fillRect(boundingRect(), QColor(0, 0, 100));
}

void TableZone::addCardImpl(CardItem *card, int x, int y)
{
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
	int x = dropPoint.x();
	int y = dropPoint.y();
	if (x < 0)
		x = 0;
	if (y < 0)
		y = 0;
	if (x > width - CARD_WIDTH)
		x = width - CARD_WIDTH;
	if (y > height - CARD_HEIGHT)
		y = height - CARD_HEIGHT;
	player->client->moveCard(cardId, startZone->getName(), getName(), x, y, faceDown);
}

void TableZone::reorganizeCards()
{
}

void TableZone::toggleTapped()
{
	QList<QGraphicsItem *> selectedItems = scene()->selectedItems();
	bool tapAll = false;
	for (int i = 0; i < selectedItems.size(); i++)
		if (!qgraphicsitem_cast<CardItem *>(selectedItems[i])->getTapped()) {
			tapAll = true;
			break;
		}
	for (int i = 0; i < selectedItems.size(); i++) {
		CardItem *temp = qgraphicsitem_cast<CardItem *>(selectedItems[i]);
		setCardAttr(temp->getId(), "tapped", (!temp->getTapped() || tapAll) ? "1" : "0");
	}
}
