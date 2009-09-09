#include <QtGui>
#include "cardzone.h"
#include "carditem.h"
#include "player.h"
#include "client.h"
#include "zoneviewzone.h"

CardZone::CardZone(Player *_p, const QString &_name, bool _hasCardAttr, bool _isShufflable, bool _contentsKnown, QGraphicsItem *parent, bool isView)
	: AbstractGraphicsItem(parent), player(_p), name(_name), cards(_contentsKnown), view(NULL), menu(NULL), doubleClickAction(0), hasCardAttr(_hasCardAttr), isShufflable(_isShufflable)
{
	if (!isView)
		player->addZone(this);
}

CardZone::~CardZone()
{
	qDebug(QString("CardZone destructor: %1").arg(name).toLatin1());
	delete view;
	clearContents();
}

void CardZone::clearContents()
{
	for (int i = 0; i < cards.size(); i++)
		delete cards.at(i);
	cards.clear();
}

void CardZone::mouseDoubleClickEvent(QGraphicsSceneMouseEvent */*event*/)
{
	if (doubleClickAction)
		doubleClickAction->trigger();
}

void CardZone::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	if (event->button() == Qt::RightButton) {
		if (menu) {
			menu->exec(event->screenPos());
			event->accept();
		} else
			event->ignore();
	} else
		event->ignore();
}

void CardZone::addCard(CardItem *card, bool reorganize, int x, int y)
{
	if (view)
		if ((x <= view->getCards().size()) || (view->getNumberCards() == -1))
			view->addCard(new CardItem(player->getDb(), card->getName(), card->getId()), reorganize, x, y);

	addCardImpl(card, x, y);

	if (reorganize)
		reorganizeCards();
}

CardItem *CardZone::getCard(int cardId, const QString &cardName)
{
	CardItem *c = cards.findCard(cardId, false);
	// If the card's id is -1, this zone is invisible,
	// so we need to give the card an id and a name as it comes out.
	// It can be assumed that in an invisible zone, all cards are equal.
	if ((c->getId() == -1) || (c->getName().isEmpty())) {
		c->setId(cardId);
		c->setName(cardName);
	}
	return c;
}

CardItem *CardZone::takeCard(int position, int cardId, const QString &cardName, bool /*canResize*/)
{
	Q_ASSERT(position < cards.size());

	CardItem *c = cards.takeAt(position);

	if (view)
		view->removeCard(position);

	c->setId(cardId);
	c->setName(cardName);

	reorganizeCards();
	return c;
}

void CardZone::setCardAttr(int cardId, const QString &aname, const QString &avalue)
{
	if (hasCardAttr)
		player->client->setCardAttr(name, cardId, aname, avalue);
}

void CardZone::setView(ZoneViewZone *_view)
{
	view = _view;
}

void CardZone::moveAllToZone(const QString &targetZone, int targetX)
{
	// Cards need to be moved in reverse order so that the other
	// cards' list index doesn't change
	for (int i = cards.size() - 1; i >= 0; i--)
		player->client->moveCard(cards.at(i)->getId(), getName(), targetZone, targetX);
}

QPointF CardZone::closestGridPoint(const QPointF &point)
{
	return point;
}
