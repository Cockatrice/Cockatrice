#include <QtGui>
#include "tablezone.h"
#include "player.h"
#include "client.h"
#include "protocol_items.h"

TableZone::TableZone(Player *_p, QGraphicsItem *parent)
	: CardZone(_p, "table", true, false, true, parent)
{
	QSettings settings;
	QString bgPath = settings.value("zonebg/table").toString();
	if (!bgPath.isEmpty())
		bgPixmap.load(bgPath);

	economicGrid = settings.value("table/economic", 1).toInt();
	if (economicGrid)
		height = (int) (14.0 / 3 * CARD_HEIGHT + 3 * paddingY);
	else
		height = 4 * CARD_HEIGHT + 3 * paddingY;
	width = minWidth + 2 * marginX;

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
	painter->setPen(QColor(255, 255, 255, 40));
	qreal separatorY = 3 * (CARD_HEIGHT + paddingY) - paddingY / 2;
	if (!player->getLocal())
		separatorY = height - separatorY;
	painter->drawLine(QPointF(0, separatorY), QPointF(width, separatorY));
}

void TableZone::addCardImpl(CardItem *card, int _x, int _y)
{
	QPointF mapPoint = mapFromGrid(QPoint(_x, _y));
	qreal x = mapPoint.x();
	qreal y = mapPoint.y();
	
	cards.append(card);
	if (!player->getLocal())
		y = height - CARD_HEIGHT - y;
	card->setPos(x, y);
	card->setGridPoint(QPoint(_x, _y));

	resizeToContents();
	
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
	player->sendGameCommand(new Command_MoveCard(-1, startZone->getName(), cardId, getName(), gridPoint.x(), gridPoint.y(), faceDown));
}

void TableZone::reorganizeCards()
{
	update();
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
	QList<Command *> cmdList;
	for (int i = 0; i < selectedItems.size(); i++) {
		CardItem *temp = qgraphicsitem_cast<CardItem *>(selectedItems[i]);
		if (temp->getTapped() != tapAll)
			cmdList.append(new Command_SetCardAttr(-1, name, temp->getId(), "tapped", tapAll ? "1" : "0"));
	}
	player->sendCommandContainer(new CommandContainer(cmdList));
}

CardItem *TableZone::takeCard(int position, int cardId, const QString &cardName, bool canResize)
{
	CardItem *result = CardZone::takeCard(position, cardId, cardName);
	if (canResize)
		resizeToContents();
	return result;
}

void TableZone::resizeToContents()
{
	int xMax = 0;
	for (int i = 0; i < cards.size(); ++i)
		if (cards[i]->pos().x() > xMax)
			xMax = (int) cards[i]->pos().x();
	xMax += 2 * CARD_WIDTH;
	if (xMax < minWidth)
		xMax = minWidth;
	int newWidth = xMax + 2 * marginX;
	if (newWidth != width) {
		prepareGeometryChange();
		width = newWidth;
		emit sizeChanged();
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
	
	QPoint result = QPoint(
		(int) (x * 2 / CARD_WIDTH),
		(int) (y / (CARD_HEIGHT + paddingY))
	);

	if (result.y() == 3) {
		if (economicGrid)
			return QPoint(
				(int) (x * 2 / CARD_WIDTH - floor(x / (2 * CARD_WIDTH))),
				3
			);
		else
			return QPoint(
				(int) (x / (1.5 * CARD_WIDTH)),
				3
			);
	} else
		return result;
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
