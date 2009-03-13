#include <QtGui>
#include "libraryzone.h"
#include "player.h"
#include "client.h"
#include "carddatabase.h"
#include "carddragitem.h"
#include "zoneviewzone.h"

LibraryZone::LibraryZone(Player *_p)
	: PlayerZone(_p, "deck")
{
	cards = new CardList(false);
	setCursor(Qt::OpenHandCursor);
}

QRectF LibraryZone::boundingRect() const
{
	return QRectF(0, 0, 50, 50);
}

void LibraryZone::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	painter->save();
	painter->fillRect(boundingRect(), QColor("red"));
	painter->setFont(QFont("Times", 20, QFont::Bold));
	painter->setPen(QPen(QColor("black")));
	painter->drawText(boundingRect(), Qt::AlignCenter, QString::number(cards->size()));
	painter->restore();
}

void LibraryZone::addCard(CardItem *card, bool reorganize, int x, int y)
{
	for (int i = 0; i < views.size(); i++)
		views[i]->addCard(new CardItem(player->getDb(), card->getName(), card->getId()), reorganize, x, y);

	cards->insert(x, card);
	card->setId(-1);
	card->setName(QString());
	card->setPos(0, 0);
	card->setVisible(false);
	card->resetState();
	card->setParentItem(this);

	if (reorganize)
		reorganizeCards();
}

void LibraryZone::handleDropEvent(int cardId, PlayerZone *startZone, const QPoint &dropPoint)
{
	player->client->moveCard(cardId, startZone->getName(), getName(), 0, 0);
}

void LibraryZone::reorganizeCards()
{
	update(boundingRect());
}

void LibraryZone::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	PlayerZone::mousePressEvent(event);
	if (event->isAccepted())
		return;

	if (event->button() == Qt::LeftButton) {
		setCursor(Qt::ClosedHandCursor);
		event->accept();
	} else
		event->ignore();
}

void LibraryZone::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	if ((event->screenPos() - event->buttonDownScreenPos(Qt::LeftButton)).manhattanLength() < QApplication::startDragDistance())
		return;

	CardItem *card = cards->at(0);
	CardDragItem *drag = card->createDragItem(this, 0, event->pos(), event->scenePos());
	drag->grabMouse();
	setCursor(Qt::OpenHandCursor);
}

void LibraryZone::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	Q_UNUSED(event);
	setCursor(Qt::OpenHandCursor);
}
