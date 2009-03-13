#include <QtGui>
#include "playerzone.h"
#include "carditem.h"
#include "player.h"
#include "client.h"
#include "zoneviewzone.h"

PlayerZone::PlayerZone(Player *_p, const QString &_name, QGraphicsItem *parent, bool isView)
	: QGraphicsItem(parent), player(_p), name(_name), cards(NULL), menu(NULL), hasCardAttr(false)
{
	if (!isView)
		player->addZone(this);
}

PlayerZone::~PlayerZone()
{
	qDebug(QString("PlayerZone destructor: %1").arg(name).toLatin1());
	while (!views.empty())
		delete views.at(0);
	
	clearContents();
	delete cards;
}

void PlayerZone::clearContents()
{
	for (int i = 0; i < cards->size(); i++)
		delete cards->at(i);
	cards->clear();
}

void PlayerZone::mousePressEvent(QGraphicsSceneMouseEvent *event)
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

CardItem *PlayerZone::getCard(int cardId, const QString &cardName)
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

CardItem *PlayerZone::takeCard(int position, int cardId, const QString &cardName)
{
	CardItem *c = cards->takeAt(position);
	for (int i = 0; i < views.size(); i++)
		views[i]->removeCard(position);

	if (c->getId() == -1) {
		c->setId(cardId);
		c->setName(cardName);
	}
	reorganizeCards();
	return c;
}

void PlayerZone::setCardAttr(int cardId, const QString &aname, const QString &avalue)
{
	if (hasCardAttr)
		player->client->setCardAttr(name, cardId, aname, avalue);
}

void PlayerZone::hoverCardEvent(CardItem *card)
{
	player->hoverCardEvent(card);
}

void PlayerZone::addView(ZoneViewZone *view)
{
	views.append(view);
}

void PlayerZone::removeView(ZoneViewZone *view)
{
	views.removeAt(views.indexOf(view));
}

void PlayerZone::moveAllToZone(const QString &targetZone, int targetX)
{
	// Cards need to be moved in reverse order so that the other
	// cards' list index doesn't change
	for (int i = cards->size() - 1; i >= 0; i--)
		player->client->moveCard(cards->at(i)->getId(), getName(), targetZone, targetX);
}
