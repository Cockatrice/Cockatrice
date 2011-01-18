#include <QPainter>
#include <QSet>
#include <QGraphicsScene>
#include <math.h>
#include "tablezone.h"
#include "player.h"
#include "protocol_items.h"
#include "settingscache.h"
#include "arrowitem.h"
#include "carddragitem.h"

TableZone::TableZone(Player *_p, QGraphicsItem *parent)
	: SelectZone(_p, "table", true, false, true, parent), active(false)
{
	connect(settingsCache, SIGNAL(tableBgPathChanged()), this, SLOT(updateBgPixmap()));
	connect(settingsCache, SIGNAL(invertVerticalCoordinateChanged()), this, SLOT(reorganizeCards()));
	updateBgPixmap();

	height = 2 * boxLineWidth + 3 * (CARD_HEIGHT + 20) + 2 * paddingY;
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

bool TableZone::isInverted() const
{
	return ((player->getMirrored() && !settingsCache->getInvertVerticalCoordinate()) || (!player->getMirrored() && settingsCache->getInvertVerticalCoordinate()));
}

void TableZone::paint(QPainter *painter, const QStyleOptionGraphicsItem */*option*/, QWidget */*widget*/)
{
	if (bgPixmap.isNull())
		painter->fillRect(boundingRect(), QColor(0, 0, 100));
	else
		painter->fillRect(boundingRect(), QBrush(bgPixmap));
	painter->setPen(QColor(255, 255, 255, 40));
	qreal separatorY = 2 * (CARD_HEIGHT + 20 + paddingY) + boxLineWidth - paddingY / 2;
	if (isInverted())
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

void TableZone::handleDropEvent(const QList<CardDragItem *> &dragItems, CardZone *startZone, const QPoint &dropPoint, bool faceDown)
{
	handleDropEventByGrid(dragItems, startZone, mapToGrid(dropPoint), faceDown);
}

void TableZone::handleDropEventByGrid(const QList<CardDragItem *> &dragItems, CardZone *startZone, const QPoint &gridPoint, bool faceDown, bool tapped)
{
	QList<CardId *> idList;
	for (int i = 0; i < dragItems.size(); ++i)
		idList.append(new CardId(dragItems[i]->getId()));
	
	startZone->getPlayer()->sendGameCommand(new Command_MoveCard(-1, startZone->getName(), idList, player->getId(), getName(), gridPoint.x(), gridPoint.y(), faceDown, tapped));
}

void TableZone::reorganizeCards()
{
	QList<ArrowItem *> arrowsToUpdate;
	
	// Calculate table grid distortion so that the mapping functions work properly
	QMap<int, int> gridPointStackCount;
	for (int i = 0; i < cards.size(); ++i) {
		const QPoint &gridPoint = cards[i]->getGridPos();
		if (gridPoint.x() == -1)
			continue;
		
		const int key = gridPoint.x() / 3 + gridPoint.y() * 1000;
		gridPointStackCount.insert(key, gridPointStackCount.value(key, 0) + 1);
	}
	gridPointWidth.clear();
	for (int i = 0; i < cards.size(); ++i) {
		const QPoint &gridPoint = cards[i]->getGridPos();
		if (gridPoint.x() == -1)
			continue;
		
		const int key = gridPoint.x() / 3 + gridPoint.y() * 1000;
		const int stackCount = gridPointStackCount.value(key, 0);
		if (stackCount == 1)
			gridPointWidth.insert(key, CARD_WIDTH * (1 + cards[i]->getAttachedCards().size() / 3.0));
		else
			gridPointWidth.insert(key, CARD_WIDTH * (1 + (stackCount - 1) / 3.0));
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
		cards[i]->setRealZValue((actualY + CARD_HEIGHT) * 100000 + (actualX + 1) * 100);
		
		QListIterator<CardItem *> attachedCardIterator(cards[i]->getAttachedCards());
		int j = 0;
		while (attachedCardIterator.hasNext()) {
			++j;
			CardItem *attachedCard = attachedCardIterator.next();
			qreal childX = actualX - j * CARD_WIDTH / 3.0;
			qreal childY = y - 5;
			attachedCard->setPos(childX, childY);
			attachedCard->setRealZValue((childY + CARD_HEIGHT) * 100000 + (childX + 1) * 100);

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

CardItem *TableZone::takeCard(int position, int cardId, bool canResize)
{
	CardItem *result = CardZone::takeCard(position, cardId);
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

QPointF TableZone::mapFromGrid(QPoint gridPoint) const
{
	qreal x, y;
	x = marginX + (gridPoint.x() % 3) * CARD_WIDTH / 3.0;
	for (int i = 0; i < gridPoint.x() / 3; ++i)
		x += gridPointWidth.value(gridPoint.y() * 1000 + i, CARD_WIDTH) + paddingX;
	
	if (isInverted())
		gridPoint.setY(2 - gridPoint.y());
	
	y = boxLineWidth + gridPoint.y() * (CARD_HEIGHT + paddingY + 20) + (gridPoint.x() % 3) * 10;
/*	
	if (isInverted())
		y = height - CARD_HEIGHT - y;
*/	
	return QPointF(x, y);
}

QPoint TableZone::mapToGrid(const QPointF &mapPoint) const
{
	qreal x = mapPoint.x() - marginX;
	qreal y = mapPoint.y();
/*	if (isInverted())
		y = height - y;
*/	y -= boxLineWidth;
	
	if (x < 0)
		x = 0;
	else if (x > width - CARD_WIDTH - marginX)
		x = width - CARD_WIDTH - marginX;
	if (y < 0)
		y = 0;
	else if (y > height - CARD_HEIGHT)
		y = height - CARD_HEIGHT;
	
	int resultY = round(y / (CARD_HEIGHT + paddingY + 20));
	if (isInverted())
		resultY = 2 - resultY;

	int baseX = -1;
	qreal oldTempX = 0, tempX = 0;
	do {
		++baseX;
		oldTempX = tempX;
		tempX += gridPointWidth.value(resultY * 1000 + baseX, CARD_WIDTH) + paddingX;
	} while (tempX < x + 1);
	
	qreal xdiff = x - oldTempX;
	int resultX = baseX * 3 + qMin((int) floor(xdiff * 3 / CARD_WIDTH), 2);
	return QPoint(resultX, resultY);
}

QPointF TableZone::closestGridPoint(const QPointF &point)
{
	QPoint gridPoint = mapToGrid(point + QPoint(1, 1));
	gridPoint.setX((gridPoint.x() / 3) * 3);
	if (getCardFromGrid(gridPoint))
		gridPoint.setX(gridPoint.x() + 1);
	if (getCardFromGrid(gridPoint))
		gridPoint.setX(gridPoint.x() + 1);
	return mapFromGrid(gridPoint);
}

void TableZone::setWidth(qreal _width)
{
	prepareGeometryChange();
	width = _width;
}
