#include <QtGui>
#include "sideboardzone.h"
#include "player.h"
#include "client.h"
#include "zoneviewzone.h"

SideboardZone::SideboardZone(Player *_p, QGraphicsItem *parent)
	: CardZone(_p, "sb", false, false, parent)
{
	cards = new CardList(false);
}

QRectF SideboardZone::boundingRect() const
{
	return QRectF(0, 0, 50, 50);
}

void SideboardZone::paint(QPainter *painter, const QStyleOptionGraphicsItem */*option*/, QWidget */*widget*/)
{
	painter->save();
	painter->fillRect(boundingRect(), QColor("blue"));
	painter->setFont(QFont("Times", 20, QFont::Bold));
	painter->setPen(QPen(QColor("black")));
	painter->drawText(boundingRect(), Qt::AlignCenter, QString::number(cards->size()));
	painter->restore();
}

void SideboardZone::addCardImpl(CardItem *card, int x, int /*y*/)
{
	cards->insert(x, card);
	card->setPos(0, 0);
	card->setVisible(false);
	card->resetState();
	card->setParentItem(this);
}

void SideboardZone::handleDropEvent(int cardId, CardZone *startZone, const QPoint &/*dropPoint*/, bool /*faceDown*/)
{
	player->client->moveCard(cardId, startZone->getName(), getName(), 0, 0);
}

void SideboardZone::reorganizeCards()
{
	update(boundingRect());
}
