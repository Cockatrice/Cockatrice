#include <QtGui>
#include "tablezone.h"
#include "player.h"
#include "client.h"

TableZone::TableZone(Player *_p, QGraphicsItem *parent)
	: CardZone(_p, "table", true, false, true, parent)
{
	QSettings settings;
	QString bgPath = settings.value("zonebg/table").toString();
	if (!bgPath.isEmpty())
		bgPixmap.load(bgPath);

	economicGrid = settings.value("table/economic", 1).toInt();
	if (economicGrid)
		height = 14.0 / 3 * CARD_HEIGHT + 3 * paddingY;
	else
		height = 4 * CARD_HEIGHT + 3 * paddingY;
	width = minWidth * CARD_WIDTH / 2;

	setCacheMode(DeviceCoordinateCache);
	setAcceptsHoverEvents(true);
}

QRectF TableZone::boundingRect() const
{
	return QRectF(0, 0, width, height);
}

void TableZone::paint(QPainter *painter, const QStyleOptionGraphicsItem */*option*/, QWidget */*widget*/)
{
/*
//	DEBUG
	painter->fillRect(boundingRect(), Qt::black);
	for (int i = 0; i < width; i += 2)
		for (int j = 0; j < height; j += 2) {
//			QPointF p = closestGridPoint(QPointF(i, j));
			QPoint p = mapToGrid(QPointF(i, j));
			QColor c;
//			c.setHsv(p.x() / 2, p.y() / 3, 255);
			c.setHsv(p.x() * 12, p.y() * 80, 255);
			painter->setPen(c);
			painter->setBrush(c);
			painter->drawRect(i, j, 2, 2);
		}
	painter->setPen(Qt::black);
	painter->drawLine(QPointF(0, 366), QPointF(1000, 366));
//	DEBUG Ende
*/
	
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
	
	if (x >= width - marginX - 2 * CARD_WIDTH) {
		width += 2 * CARD_WIDTH;
		emit sizeChanged();
	}
	
	cards.append(card);
//	if ((x != -1) && (y != -1)) {
		if (!player->getLocal())
			y = height - CARD_HEIGHT - y;
		card->setPos(x, y);
//	}
	card->setGridPoint(QPoint(_x, _y));
	card->setZValue((y + CARD_HEIGHT) * 10000000 + x + 1000);
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

CardItem *TableZone::takeCard(int position, int cardId, const QString &cardName)
{
	CardItem *result = CardZone::takeCard(position, cardId, cardName);
	int xMax = 0;
	for (int i = 0; i < cards.size(); ++i)
		if (cards[i]->getGridPoint().x() > xMax)
			xMax = cards[i]->getGridPoint().x();
	if (xMax < minWidth)
		xMax = minWidth;
	int newWidth = (xMax + 1) * CARD_WIDTH / 2 + 2 * marginX;
	if (newWidth < width - 2 * CARD_WIDTH) {
		width = newWidth;
		emit sizeChanged();
	}
	return result;
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
	qDebug(QString("mapFromGrid: %1, %2").arg(gridPoint.x()).arg(gridPoint.y()).toLatin1());
	if (gridPoint.y() == 3) {
		if (economicGrid)
			return QPointF(
				20 + (CARD_WIDTH * gridPoint.x() + CARD_WIDTH * (gridPoint.x() / 3)) / 2,
				(CARD_HEIGHT + paddingY) * gridPoint.y() + (gridPoint.x() % 3 * CARD_HEIGHT) / 3
			);
		else
			return QPointF(
				20 + 3 * CARD_WIDTH * gridPoint.x() / 2,
				(CARD_HEIGHT + paddingY) * gridPoint.y()
			);
	} else
		return QPointF(
			20 + CARD_WIDTH * gridPoint.x() / 2,
			(CARD_HEIGHT + paddingY) * gridPoint.y()
		);
}

QPoint TableZone::mapToGrid(const QPointF &mapPoint) const
{
	qreal x = mapPoint.x() - marginX;
	qreal y = mapPoint.y() + paddingY / 2;
	if (x < 0)
		x = 0;
	else if (x > width - CARD_WIDTH - marginX)
		x = width - CARD_WIDTH - marginX;
	if (y < 0)
		y = 0;
	else if (y > height - CARD_HEIGHT)
		y = height - CARD_HEIGHT;
	
	qDebug(QString("mapToGrid: %1, %2").arg(x).arg(y).toLatin1());
	QPoint result = QPoint(
//		(int) round((double) x * 2 / CARD_WIDTH),
//		(int) round((double) y / (CARD_HEIGHT + paddingY))
		x * 2 / CARD_WIDTH,
		y / (CARD_HEIGHT + paddingY)
	);

	if (result.y() == 3) {
		qDebug("UNTER grenze");
		if (economicGrid)
			return QPoint(
				x * 2 / CARD_WIDTH - (x / (2 * CARD_WIDTH)),
				3
			);
		else {
			return QPoint(
				round((double) x / (1.5 * CARD_WIDTH)),
				3
			);
		}
	} else {
		qDebug("UEBER grenze");
		return result;
	}
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

QPointF TableZone::closestGridPoint(const QPointF &point)
{
	return mapFromGrid(mapToGrid(point + QPoint(CARD_WIDTH / 2, CARD_HEIGHT / 2)));
}
