#include <QtGui>
#include "tablezone.h"
#include "player.h"
#include "client.h"

TableZone::TableZone(Player *_p, QGraphicsItem *parent)
	: CardZone(_p, "table", true, false, true, parent), width(864), height(578)
{
	gridPoints << (QList<QPoint>() << QPoint(8, 12)
				       << QPoint(9, 13)
				       << QPoint(10, 14)
				       << QPoint(12, 12)
				       << QPoint(13, 13)
				       << QPoint(14, 14)
				       << QPoint(4, 12)
				       << QPoint(5, 13)
				       << QPoint(6, 14)
				       << QPoint(16, 12)
				       << QPoint(17, 13)
				       << QPoint(18, 14)
				       << QPoint(0, 12)
				       << QPoint(1, 13)
				       << QPoint(2, 14)
				       << QPoint(20, 12)
				       << QPoint(21, 13)
				       << QPoint(22, 14))
		   << (QList<QPoint>() << QPoint(10, 8)
				       << QPoint(13, 8)
				       << QPoint(7, 8)
				       << QPoint(16, 8)
				       << QPoint(4, 8)
				       << QPoint(19, 8)
				       << QPoint(1, 8)
				       << QPoint(22, 8))
		   << (QList<QPoint>() << QPoint(10, 4)
				       << QPoint(13, 4)
				       << QPoint(7, 4)
				       << QPoint(16, 4)
				       << QPoint(4, 4)
				       << QPoint(19, 4)
				       << QPoint(1, 4)
				       << QPoint(22, 4))
		   << (QList<QPoint>() << QPoint(10, 0)
				       << QPoint(13, 0)
				       << QPoint(7, 0)
				       << QPoint(16, 0)
				       << QPoint(4, 0)
				       << QPoint(19, 0)
				       << QPoint(1, 0)
				       << QPoint(22, 0));
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
	
	cards.append(card);
//	if ((x != -1) && (y != -1)) {
		if (!player->getLocal())
			y = height - CARD_HEIGHT - y;
		card->setPos(x, y);
//	}
	card->setGridPoint(QPoint(_x, _y));
	card->setZValue((y + CARD_HEIGHT) * width + x + 1000);
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
	for (int i = 0; i < cards.size(); i++)
		if (cards.at(i)->getGridPoint() == gridPoint)
			return cards.at(i);
	return 0;
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
	
	return QPoint((int) round(((double) x * gridPointsPerCardX) / CARD_WIDTH), (int) round(((double) y * gridPointsPerCardY) / CARD_HEIGHT));
}

QPoint TableZone::getFreeGridPoint(int row) const
{
	Q_ASSERT(row < gridPoints.size());
	
	QList<QPoint> pointList = gridPoints[row];
	for (int i = 0; i < pointList.size(); i++)
		if (!getCardFromGrid(pointList[i]))
			return pointList[i];
	return QPoint(0, 0);
}
