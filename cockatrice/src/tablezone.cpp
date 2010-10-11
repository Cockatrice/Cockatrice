#include <QPainter>
#include <QSet>
#include <QGraphicsScene>
#include <math.h>
#include "tablezone.h"
#include "player.h"
#include "protocol_items.h"
#include "settingscache.h"
#include "arrowitem.h"

TableZone::TableZone(Player *_p, QGraphicsItem *parent)
	: CardZone(_p, "table", true, false, true, parent), active(false)
{
	connect(settingsCache, SIGNAL(tableBgPathChanged()), this, SLOT(updateBgPixmap()));
	connect(settingsCache, SIGNAL(economicalGridChanged()), this, SLOT(reorganizeCards()));
	updateBgPixmap();

	if (settingsCache->getEconomicalGrid())
		height = 2 * boxLineWidth + (int) (14.0 / 3 * CARD_HEIGHT + 3 * paddingY);
	else
		height = 2 * boxLineWidth + 4 * CARD_HEIGHT + 3 * paddingY;
	width = minWidth + 2 * marginX + 2 * boxLineWidth;
	currentMinimumWidth = minWidth;

	setCacheMode(DeviceCoordinateCache);
	setAcceptsHoverEvents(true);
}

void TableZone::updateBgPixmap()
{
	QString bgPath = settingsCache->getTableBgPath();
	if (!bgPath.isEmpty())
		bgPixmap.load(bgPath);
	update();
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
	qreal separatorY = 3 * (CARD_HEIGHT + paddingY) + boxLineWidth - paddingY / 2;
	if (player->getMirrored())
		separatorY = height - separatorY;
	painter->drawLine(QPointF(0, separatorY), QPointF(width, separatorY));
	
	if (active) {
		QColor color1(255, 255, 255, 150);
		QColor color2(255, 255, 255, 0);
		QLinearGradient grad1(0, 0, 0, 1);
		grad1.setCoordinateMode(QGradient::ObjectBoundingMode);
		grad1.setColorAt(0, color1);
		grad1.setColorAt(1, color2);
		painter->fillRect(QRectF(0, 0, width, boxLineWidth), QBrush(grad1));
		
		grad1.setFinalStop(1, 0);
		painter->fillRect(QRectF(0, 0, boxLineWidth, height), QBrush(grad1));
		
		grad1.setStart(0, 1);
		grad1.setFinalStop(0, 0);
		painter->fillRect(QRectF(0, height - boxLineWidth, width, boxLineWidth), QBrush(grad1));
		
		grad1.setStart(1, 0);
		painter->fillRect(QRectF(width - boxLineWidth, 0, boxLineWidth, height), QBrush(grad1));
	}
}

void TableZone::addCardImpl(CardItem *card, int _x, int _y)
{
	cards.append(card);
	card->setGridPoint(QPoint(_x, _y));

	card->setParentItem(this);
	card->setVisible(true);
	card->update();
}

void TableZone::handleDropEvent(int cardId, CardZone *startZone, const QPoint &dropPoint, bool faceDown)
{
	handleDropEventByGrid(cardId, startZone, mapToGrid(dropPoint), faceDown);
}

void TableZone::handleDropEventByGrid(int cardId, CardZone *startZone, const QPoint &gridPoint, bool faceDown, bool tapped)
{
	player->sendGameCommand(new Command_MoveCard(-1, startZone->getName(), cardId, getName(), gridPoint.x(), gridPoint.y(), faceDown, tapped));
}

void TableZone::reorganizeCards()
{
	QList<ArrowItem *> arrowsToUpdate;
	
	// Calculate table grid distortion so that the mapping functions work properly
	gridPointWidth.clear();
	for (int i = 0; i < cards.size(); ++i) {
		QPoint gridPoint = cards[i]->getGridPos();
		if (gridPoint.x() == -1)
			continue;
		
		gridPointWidth.insert(gridPoint.x() + gridPoint.y() * 1000, CARD_WIDTH * (1 + cards[i]->getAttachedCards().size() / 3.0));
	}
	
	for (int i = 0; i < cards.size(); ++i) {
		QPoint gridPoint = cards[i]->getGridPos();
		if (gridPoint.x() == -1)
			continue;
		
		QPointF mapPoint = mapFromGrid(gridPoint);
		qreal x = mapPoint.x();
		qreal y = mapPoint.y();
		
		int numberAttachedCards = cards[i]->getAttachedCards().size();
		qreal actualX = x + numberAttachedCards * CARD_WIDTH / 3.0;
		qreal actualY = y;
		if (numberAttachedCards)
			actualY += 5;
		
		cards[i]->setPos(actualX, actualY);
		cards[i]->setZValue((actualY + CARD_HEIGHT) * 10000000 + (actualX + 1) * 10000);
		
		QListIterator<CardItem *> attachedCardIterator(cards[i]->getAttachedCards());
		int j = 0;
		while (attachedCardIterator.hasNext()) {
			++j;
			CardItem *attachedCard = attachedCardIterator.next();
			qreal childX = actualX - j * CARD_WIDTH / 3.0;
			qreal childY = y - 5;
			attachedCard->setPos(childX, childY);
			attachedCard->setZValue((childY + CARD_HEIGHT) * 10000000 + (childX + 1) * 10000);

			arrowsToUpdate.append(attachedCard->getArrowsFrom());
			arrowsToUpdate.append(attachedCard->getArrowsTo());
		}
		
		arrowsToUpdate.append(cards[i]->getArrowsFrom());
		arrowsToUpdate.append(cards[i]->getArrowsTo());
	}

	QSetIterator<ArrowItem *> arrowIterator(QSet<ArrowItem *>::fromList(arrowsToUpdate));
	while (arrowIterator.hasNext())
		arrowIterator.next()->updatePath();
	
	resizeToContents();
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
	currentMinimumWidth = xMax + 2 * marginX + 2 * boxLineWidth;
	if (currentMinimumWidth != width) {
		prepareGeometryChange();
		width = currentMinimumWidth;
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

CardItem *TableZone::getCardFromCoords(const QPointF &point) const
{
	QPoint gridPoint = mapToGrid(point);
	return getCardFromGrid(gridPoint);
}

QPointF TableZone::mapFromGrid(const QPoint &gridPoint) const
{
	qreal x, y;
	if ((gridPoint.y() == 3) && (settingsCache->getEconomicalGrid())) {
		x = marginX + (CARD_WIDTH * gridPoint.x() + CARD_WIDTH * (gridPoint.x() / 3)) / 2;
		y = boxLineWidth + (CARD_HEIGHT + paddingY) * gridPoint.y() + (gridPoint.x() % 3 * CARD_HEIGHT) / 3;
	} else {
		x = marginX + 0.5 * CARD_WIDTH * gridPoint.x();
		for (int i = 0; i < gridPoint.x(); ++i)
			x += gridPointWidth.value(gridPoint.y() * 1000 + i, CARD_WIDTH);
		
		y = boxLineWidth + (CARD_HEIGHT + paddingY) * gridPoint.y();
	}
	if (player->getMirrored())
		y = height - CARD_HEIGHT - y;
	
	return QPointF(x, y);
}

QPoint TableZone::mapToGrid(const QPointF &mapPoint) const
{
	qreal x = mapPoint.x() - marginX;
	qreal y = mapPoint.y();
	if (player->getMirrored())
		y = height - y;
	y += paddingY / 2 - boxLineWidth;
	
	if (x < 0)
		x = 0;
	else if (x > width - CARD_WIDTH - marginX)
		x = width - CARD_WIDTH - marginX;
	if (y < 0)
		y = 0;
	else if (y > height - CARD_HEIGHT)
		y = height - CARD_HEIGHT;
	
	int resultY = (int) (y / (CARD_HEIGHT + paddingY));

	if ((resultY == 3) && (settingsCache->getEconomicalGrid()))
		return QPoint(
			(int) (x * 2 / CARD_WIDTH - floor(x / (2 * CARD_WIDTH))),
			3
		);
	else {
		int resultX = -1;
		qreal tempX = 0;
		do {
			++resultX;
			tempX += gridPointWidth.value(resultY * 1000 + resultX, CARD_WIDTH) + 0.5 * CARD_WIDTH;
		} while (tempX < x + 1);
		return QPoint(resultX, resultY);
	}
}

QPointF TableZone::closestGridPoint(const QPointF &point)
{
	return mapFromGrid(mapToGrid(point + QPoint(CARD_WIDTH / 2, CARD_HEIGHT / 2)));
}

void TableZone::setWidth(qreal _width)
{
	prepareGeometryChange();
	width = _width;
}
