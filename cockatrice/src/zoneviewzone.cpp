#include <QtGui>
#include "zoneviewzone.h"
#include "player.h"
#include "client.h"

ZoneViewZone::ZoneViewZone(Player *_p, CardZone *_origZone, int _numberCards, QGraphicsItem *parent)
	: CardZone(_p, _origZone->getName(), false, false, true, parent, true), height(0), numberCards(_numberCards), origZone(_origZone)
{
	origZone->setView(this);
}

ZoneViewZone::~ZoneViewZone()
{
	qDebug("ZoneViewZone destructor");
	origZone->setView(NULL);
}

QRectF ZoneViewZone::boundingRect() const
{
	return QRectF(0, 0, CARD_WIDTH * 1.75, height);
}

void ZoneViewZone::paint(QPainter */*painter*/, const QStyleOptionGraphicsItem */*option*/, QWidget */*widget*/)
{
}

bool ZoneViewZone::initializeCards()
{
	if (!origZone->contentsKnown())
		return false;

	const CardList &c = origZone->getCards();
	int number = numberCards == -1 ? c.size() : (numberCards < c.size() ? numberCards : c.size());
	for (int i = 0; i < number; i++) {
		CardItem *card = c.at(i);
		addCard(new CardItem(player->getDb(), card->getName(), card->getId(), this), false, i);
	}
	reorganizeCards();
	return true;
}

// Because of boundingRect(), this function must not be called before the zone was added to a scene.
void ZoneViewZone::reorganizeCards()
{
	qDebug("reorganizeCards");

	if (cards.isEmpty())
		return;

	int cardCount = cards.size();
	qreal totalWidth = boundingRect().width();
	qreal totalHeight = boundingRect().height();
	qreal cardWidth = cards.at(0)->boundingRect().width();
	qreal cardHeight = cards.at(0)->boundingRect().height();
	qreal x1 = 0;
	qreal x2 = (totalWidth - cardWidth);

	for (int i = 0; i < cardCount; i++) {
		CardItem *c = cards.at(i);
		qreal x = i % 2 ? x2 : x1;
		// If the total height of the cards is smaller than the available height,
		// the cards do not need to overlap and are displayed in the center of the area.
		if (cardHeight * cardCount > totalHeight)
			c->setPos(x, ((qreal) i) * (totalHeight - cardHeight) / (cardCount - 1));
		else
			c->setPos(x, ((qreal) i) * cardHeight + (totalHeight - cardCount * cardHeight) / 2);
		if (!origZone->contentsKnown())
			c->setId(i);
		c->setZValue(i);
	}
}

void ZoneViewZone::addCardImpl(CardItem *card, int x, int /*y*/)
{
	cards.insert(x, card);
	card->setParentItem(this);
	card->update();
}

void ZoneViewZone::handleDropEvent(int cardId, CardZone *startZone, const QPoint &/*dropPoint*/, bool /*faceDown*/)
{
	qDebug(QString("handleDropEvent id=%1").arg(cardId).toLatin1());
	player->client->moveCard(cardId, startZone->getName(), getName(), 0, 0);
}

void ZoneViewZone::removeCard(int position)
{
	if (position >= cards.size())
		return;

	CardItem *card = cards.takeAt(position);
	delete card;
	reorganizeCards();
}
