#include <QPainter>
#include "handzone.h"
#include "settingscache.h"
#include "player.h"
#include "protocol_items.h"
#include "carddragitem.h"

HandZone::HandZone(Player *_p, bool _contentsKnown, int _zoneHeight, QGraphicsItem *parent)
	: SelectZone(_p, "hand", false, false, _contentsKnown, parent), zoneHeight(_zoneHeight)
{
	connect(settingsCache, SIGNAL(handBgPathChanged()), this, SLOT(updateBgPixmap()));
	updateBgPixmap();
	setCacheMode(DeviceCoordinateCache);
}

void HandZone::updateBgPixmap()
{
	QString bgPath = settingsCache->getHandBgPath();
	if (!bgPath.isEmpty())
		bgPixmap.load(bgPath);
	update();
}

void HandZone::addCardImpl(CardItem *card, int x, int /*y*/)
{
	if (x == -1)
		x = cards.size();
	cards.insert(x, card);

	if (!cards.getContentsKnown()) {
		card->setId(-1);
		card->setName();
	}
	card->setParentItem(this);
	card->resetState();
	card->setVisible(true);
	card->update();
}

void HandZone::handleDropEvent(const QList<CardDragItem *> &dragItems, CardZone *startZone, const QPoint &/*dropPoint*/, bool /*faceDown*/)
{
	QList<CardId *> idList;
	for (int i = 0; i < dragItems.size(); ++i)
		idList.append(new CardId(dragItems[i]->getId()));

	player->sendGameCommand(new Command_MoveCard(-1, startZone->getName(), idList, player->getId(), getName(), cards.size(), -1, false));
}

QRectF HandZone::boundingRect() const
{
	if (settingsCache->getHorizontalHand())
		return QRectF(0, 0, width, CARD_HEIGHT + 10);
	else
		return QRectF(0, 0, 100, zoneHeight);
}

void HandZone::paint(QPainter *painter, const QStyleOptionGraphicsItem */*option*/, QWidget */*widget*/)
{
	if (bgPixmap.isNull())
		painter->fillRect(boundingRect(), Qt::darkGreen);
	else
		painter->fillRect(boundingRect(), QBrush(bgPixmap));
}

void HandZone::reorganizeCards()
{
	if (!cards.isEmpty()) {
		const int cardCount = cards.size();
		if (settingsCache->getHorizontalHand()) {
			const int xPadding = 5;
			qreal totalWidth = boundingRect().width() - 2 * xPadding;
			qreal cardWidth = cards.at(0)->boundingRect().width();
			
			for (int i = 0; i < cardCount; i++) {
				CardItem *c = cards.at(i);
	
				// If the total width of the cards is smaller than the available width,
				// the cards do not need to overlap and are displayed in the center of the area.
				if (cardWidth * cardCount > totalWidth)
					c->setPos(xPadding + ((qreal) i) * (totalWidth - cardWidth) / (cardCount - 1), 5);
				else
					c->setPos(xPadding + ((qreal) i) * cardWidth + (totalWidth - cardCount * cardWidth) / 2, 5);
				c->setRealZValue(i);
			}
		} else {
			qreal totalWidth = boundingRect().width();
			qreal totalHeight = boundingRect().height();
			qreal cardWidth = cards.at(0)->boundingRect().width();
			qreal cardHeight = cards.at(0)->boundingRect().height();
			qreal xspace = 5;
			qreal x1 = xspace;
			qreal x2 = totalWidth - xspace - cardWidth;
		
			for (int i = 0; i < cardCount; i++) {
				CardItem *c = cards.at(i);
				qreal x = i % 2 ? x2 : x1;
				// If the total height of the cards is smaller than the available height,
				// the cards do not need to overlap and are displayed in the center of the area.
				if (cardHeight * cardCount > totalHeight)
					c->setPos(x, ((qreal) i) * (totalHeight - cardHeight) / (cardCount - 1));
				else
					c->setPos(x, ((qreal) i) * cardHeight + (totalHeight - cardCount * cardHeight) / 2);
				c->setRealZValue(i);
			}
		}
	}
	update();
}

void HandZone::setWidth(qreal _width)
{
	if (settingsCache->getHorizontalHand()) {
		prepareGeometryChange();
		width = _width;
		reorganizeCards();
	}
}

void HandZone::updateOrientation()
{
	prepareGeometryChange();
	reorganizeCards();
}
