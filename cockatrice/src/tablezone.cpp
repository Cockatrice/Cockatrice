#include <QtGui>
#include "tablezone.h"
#include "player.h"
#include "client.h"

TableZone::TableZone(Player *_p, QGraphicsItem *parent)
	: CardZone(_p, "table", true, false, parent), width(864), height(510)
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

void TableZone::addCardImpl(CardItem *card, int _x, int _y)
{
	QPointF mapPoint = mapFromGrid(QPoint(_x, _y));
	qreal x = mapPoint.x();
	qreal y = mapPoint.y();
	
	cards->append(card);
//	if ((x != -1) && (y != -1)) {
		if (!player->getLocal())
			y = height - CARD_HEIGHT - y;
		card->setPos(x, y);
//	}
	card->setZValue((y + CARD_HEIGHT) * width + x + 1000);
	qDebug(QString("table: appended %1 at pos %2: zValue = %3, x = %4, y = %5").arg(card->getName()).arg(cards->size() - 1).arg(card->zValue()).arg(x).arg(y).toLatin1());
	card->setParentItem(this);
	card->setVisible(true);
	card->update();
}

void TableZone::handleDropEvent(int cardId, CardZone *startZone, const QPoint &dropPoint, bool faceDown)
{
	QPoint gridPoint = mapToGrid(dropPoint);
	player->client->moveCard(cardId, startZone->getName(), getName(), gridPoint.x(), gridPoint.y(), faceDown);
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

CardItem *TableZone::getCardFromGrid(const QPoint &gridPoint) const
{

}

QPointF TableZone::mapFromGrid(const QPoint &gridPoint) const
{
	return QPointF(gridPoint.x() * CARD_WIDTH / gridPointsPerCardX,
		gridPoint.y() * CARD_HEIGHT / gridPointsPerCardY);
}

QPoint TableZone::mapToGrid(const QPointF &mapPoint) const
{
	qreal x = mapPoint.x();
	qreal y = mapPoint.y();
	if (x < 0)
		x = 0;
	else if (x > width - CARD_WIDTH)
		x = width - CARD_WIDTH;
	if (y < 0)
		y = 0;
	else if (y > height - CARD_HEIGHT)
		y = height - CARD_HEIGHT;
	
	return QPoint(round(((double) x * gridPointsPerCardX) / CARD_WIDTH), round(((double) y * gridPointsPerCardY) / CARD_HEIGHT));
}
