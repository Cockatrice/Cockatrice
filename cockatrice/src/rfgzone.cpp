#include <QtGui>
#include "rfgzone.h"
#include "player.h"
#include "client.h"
#include "carddragitem.h"
#include "zoneviewzone.h"

RfgZone::RfgZone(Player *_p, QGraphicsItem *parent)
	: CardZone(_p, "rfg", parent)
{
	cards = new CardList(true);
}

QRectF RfgZone::boundingRect() const
{
	return QRectF(0, 0, CARD_WIDTH, CARD_HEIGHT);
}

void RfgZone::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	painter->save();
	
	painter->fillRect(boundingRect(), QColor("yellow"));
	
	painter->setFont(QFont("Times", 32, QFont::Bold));
	painter->setPen(QPen(QColor("black")));
	painter->setBackground(QBrush(QColor(255, 255, 255, 100)));
	painter->setBackgroundMode(Qt::OpaqueMode);
	painter->drawText(boundingRect(), Qt::AlignCenter, QString::number(cards->size()));

	painter->restore();
}

void RfgZone::addCard(CardItem *card, bool reorganize, int x, int y)
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

void RfgZone::handleDropEvent(int cardId, CardZone *startZone, const QPoint &dropPoint)
{
	player->client->moveCard(cardId, startZone->getName(), getName(), 0, 0);
}

void RfgZone::reorganizeCards()
{
	update(boundingRect());
}

void RfgZone::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	CardZone::mousePressEvent(event);
	if (event->isAccepted())
		return;

	if (event->button() == Qt::LeftButton) {
		setCursor(Qt::ClosedHandCursor);
		event->accept();
	} else
		event->ignore();
}

void RfgZone::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	if ((event->screenPos() - event->buttonDownScreenPos(Qt::LeftButton)).manhattanLength() < QApplication::startDragDistance())
		return;

	if (cards->empty())
		return;
		
	CardItem *card = cards->at(0);
	CardDragItem *drag = card->createDragItem(this, card->getId(), event->pos(), event->scenePos());
	drag->grabMouse();
	setCursor(Qt::OpenHandCursor);
}

void RfgZone::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	Q_UNUSED(event);
	setCursor(Qt::OpenHandCursor);
}
