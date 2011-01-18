#include <QPainter>
#include <QSet>
#include "arrowitem.h"
#include "stackzone.h"
#include "settingscache.h"
#include "player.h"
#include "protocol_items.h"
#include "carddragitem.h"

StackZone::StackZone(Player *_p, int _zoneHeight, QGraphicsItem *parent)
	: SelectZone(_p, "stack", false, false, true, parent), zoneHeight(_zoneHeight)
{
	connect(settingsCache, SIGNAL(stackBgPathChanged()), this, SLOT(updateBgPixmap()));
	updateBgPixmap();
	setCacheMode(DeviceCoordinateCache);
}

void StackZone::updateBgPixmap()
{
	QString bgPath = settingsCache->getStackBgPath();
	if (!bgPath.isEmpty())
		bgPixmap.load(bgPath);
	update();
}

void StackZone::addCardImpl(CardItem *card, int x, int /*y*/)
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

QRectF StackZone::boundingRect() const
{
	return QRectF(0, 0, 100, zoneHeight);
}

void StackZone::paint(QPainter *painter, const QStyleOptionGraphicsItem */*option*/, QWidget */*widget*/)
{
	if (bgPixmap.isNull())
		painter->fillRect(boundingRect(), QColor(113, 43, 43));
	else
		painter->fillRect(boundingRect(), QBrush(bgPixmap));
}

void StackZone::handleDropEvent(const QList<CardDragItem *> &dragItems, CardZone *startZone, const QPoint &/*dropPoint*/, bool /*faceDown*/)
{
	if (startZone == this)
		return;
	
	QList<CardId *> idList;
	for (int i = 0; i < dragItems.size(); ++i)
		idList.append(new CardId(dragItems[i]->getId()));
	
	player->sendGameCommand(new Command_MoveCard(-1, startZone->getName(), idList, player->getId(), getName(), 0, 0, false));
}

void StackZone::reorganizeCards()
{
	if (!cards.isEmpty()) {
		QList<ArrowItem *> arrowsToUpdate;
		
		const int cardCount = cards.size();
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
			
			arrowsToUpdate.append(c->getArrowsFrom());
			arrowsToUpdate.append(c->getArrowsTo());
		}
		QSetIterator<ArrowItem *> arrowIterator(QSet<ArrowItem *>::fromList(arrowsToUpdate));
		while (arrowIterator.hasNext())
			arrowIterator.next()->updatePath();
	}
	update();
}
