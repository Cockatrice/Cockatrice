#include <QtGui>
#include "tablezone.h"
#include "player.h"
#include "client.h"

TableZone::TableZone(Player *_p, QGraphicsItem *parent)
	: CardZone(_p, "table", true, false, true, parent), width(864), height(536)
{
	QSettings settings;
	QString bgPath = settings.value("zonebg/table").toString();
	if (!bgPath.isEmpty())
		bgPixmap.load(bgPath);

	setCacheMode(DeviceCoordinateCache);
	setAcceptsHoverEvents(true);
}

QRectF TableZone::boundingRect() const
{
	return QRectF(0, 0, width, height);
}

void TableZone::paint(QPainter *painter, const QStyleOptionGraphicsItem */*option*/, QWidget */*widget*/)
{
	if (bgPixmap.isNull())
		painter->fillRect(boundingRect(), QColor(0, 0, 100));
	else
	painter->fillRect(boundingRect(), QBrush(bgPixmap));
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
	handleDropEventByGrid(cardId, startZone, mapToGrid(dropPoint), faceDown);
}

void TableZone::handleDropEventByGrid(int cardId, CardZone *startZone, const QPoint &gridPoint, bool faceDown)
{
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
		if (temp->getTapped() != tapAll)
			setCardAttr(temp->getId(), "tapped", tapAll ? "1" : "0");
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
	if (gridPoint.y() == 3)
		return QPointF(
			20 + (CARD_WIDTH * gridPoint.x() + CARD_WIDTH * (gridPoint.x() / 3)) / gridPointsPerCardX,
			(CARD_HEIGHT + paddingY) * gridPoint.y() / gridPointsPerCardY + (gridPoint.x() % 3 * CARD_HEIGHT) / 3
		);
	else
		return QPointF(
			20 + CARD_WIDTH * gridPoint.x() / gridPointsPerCardX,
			(CARD_HEIGHT + paddingY) * gridPoint.y() / gridPointsPerCardY
		);
}

QPoint TableZone::mapToGrid(const QPointF &mapPoint) const
{
	qreal x = mapPoint.x() - 20;
	qreal y = mapPoint.y();
	if (x < 0)
		x = 0;
	else if (x > width - CARD_WIDTH)
		x = width - CARD_WIDTH;
	if (y < 0)
		y = 0;
	else if (y > height - CARD_HEIGHT)
		y = height - CARD_HEIGHT;
	
	if (y >= (CARD_HEIGHT + paddingY) * 3 - 1)
		return QPoint(
			(int) round(((double) x * gridPointsPerCardX) / CARD_WIDTH - x / (2 * CARD_WIDTH)),
			3
		);
	else
		return QPoint(
			(int) round(((double) x * gridPointsPerCardX) / CARD_WIDTH),
			(int) round(((double) y * gridPointsPerCardY) / (CARD_HEIGHT + paddingY))
		);
}

QPoint TableZone::getFreeGridPoint(int row) const
{
	int x = 0;
	int y = 3 - row;
	if (y == 3)
		while (getCardFromGrid(QPoint(x, y)))
			++x;
	else
		while (((x >= 2) && getCardFromGrid(QPoint(x - 2, y))) || ((x >= 1) && getCardFromGrid(QPoint(x - 1, y))) || getCardFromGrid(QPoint(x, y)) || getCardFromGrid(QPoint(x + 1, y)) || getCardFromGrid(QPoint(x + 2, y)))
			++x;
	return QPoint(x, y);
}
