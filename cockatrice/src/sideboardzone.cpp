#include <QtGui>
#include "sideboardzone.h"
#include "player.h"
#include "client.h"
#include "zoneviewzone.h"

SideboardZone::SideboardZone(Player *_p, QGraphicsItem *parent)
	: CardZone(_p, "sb", parent)
{
	cards = new CardList(false);
}

QRectF SideboardZone::boundingRect() const
{
	return QRectF(0, 0, 50, 50);
}

void SideboardZone::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	painter->save();
	painter->fillRect(boundingRect(), QColor("blue"));
	painter->setFont(QFont("Times", 20, QFont::Bold));
	painter->setPen(QPen(QColor("black")));
	painter->drawText(boundingRect(), Qt::AlignCenter, QString::number(cards->size()));
	painter->restore();
}

void SideboardZone::addCard(CardItem *card, bool reorganize, int x, int y)
{
	for (int i = 0; i < views.size(); i++)
		views[i]->addCard(new CardItem(player->getDb(), card->getName(), card->getId()), reorganize, x, y);

	cards->insert(x, card);
	card->setPos(0, 0);
	card->setVisible(false);
	card->resetState();
	card->setParentItem(this);

	if (reorganize)
		reorganizeCards();
}

void SideboardZone::handleDropEvent(int cardId, CardZone *startZone, const QPoint &dropPoint)
{
	player->client->moveCard(cardId, startZone->getName(), getName(), 0, 0);
}

void SideboardZone::reorganizeCards()
{
	update(boundingRect());
}

