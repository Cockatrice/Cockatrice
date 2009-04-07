#include <QtGui>
#include "cardzone.h"
#include "carditem.h"
#include "player.h"
#include "client.h"
#include "zoneviewzone.h"

CardZone::CardZone(Player *_p, const QString &_name, QGraphicsItem *parent, bool isView)
	: QGraphicsItem(parent), player(_p), name(_name), cards(NULL), menu(NULL), hasCardAttr(false)
{
	if (!isView)
		player->addZone(this);
}

CardZone::~CardZone()
{
	qDebug(QString("CardZone destructor: %1").arg(name).toLatin1());
	while (!views.empty())
		delete views.at(0);
	
	clearContents();
	delete cards;
}

void CardZone::clearContents()
{
	for (int i = 0; i < cards->size(); i++)
		delete cards->at(i);
	cards->clear();
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

CardItem *CardZone::getCard(int cardId, const QString &cardName)
{
	CardItem *c = cards->findCard(cardId, false);
	// If the card's id is -1, this zone is invisible,
	// so we need to give the card an id and a name as it comes out.
	// It can be assumed that in an invisible zone, all cards are equal.
	if ((c->getId() == -1) || (c->getName().isEmpty())) {
		c->setId(cardId);
		c->setName(cardName);
	}
	return c;
}

CardItem *CardZone::takeCard(int position, int cardId, const QString &cardName)
{
	if (position >= cards->size())
		return NULL;
		
	CardItem *c = cards->takeAt(position);
	for (int i = 0; i < views.size(); i++)
		views[i]->removeCard(position);

//	if (c->getId() == -1) {
		c->setId(cardId);
		c->setName(cardName);
//	}
	reorganizeCards();
	return c;
}

void CardZone::setCardAttr(int cardId, const QString &aname, const QString &avalue)
{
	if (hasCardAttr)
		player->client->setCardAttr(name, cardId, aname, avalue);
}

void CardZone::hoverCardEvent(CardItem *card)
{
	player->hoverCardEvent(card);
}

void CardZone::addView(ZoneViewZone *view)
{
	views.append(view);
}

void CardZone::removeView(ZoneViewZone *view)
{
	views.removeAt(views.indexOf(view));
}

void CardZone::moveAllToZone(const QString &targetZone, int targetX)
{
	// Cards need to be moved in reverse order so that the other
	// cards' list index doesn't change
	for (int i = cards->size() - 1; i >= 0; i--)
		player->client->moveCard(cards->at(i)->getId(), getName(), targetZone, targetX);
}
