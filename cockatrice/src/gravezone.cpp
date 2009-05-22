#include <QtGui>
#include "gravezone.h"
#include "player.h"
#include "client.h"
#include "carddragitem.h"
#include "zoneviewzone.h"

GraveZone::GraveZone(Player *_p, QGraphicsItem *parent)
	: CardZone(_p, "grave", false, false, parent)
{
	cards = new CardList(true);
}

QRectF GraveZone::boundingRect() const
{
	return QRectF(0, 0, CARD_WIDTH, CARD_HEIGHT);
}

void GraveZone::paint(QPainter *painter, const QStyleOptionGraphicsItem */*option*/, QWidget */*widget*/)
{
	painter->save();
	
	painter->fillRect(boundingRect(), QColor("yellow"));
	
	painter->setFont(QFont("Times", 32, QFont::Bold));
	painter->setPen(QPen(QColor("black")));
	painter->setBackground(QBrush(QColor(255, 255, 255, 100)));
	painter->setBackgroundMode(Qt::OpaqueMode);
	painter->drawText(boundingRect(), Qt::AlignCenter, QString::number(cards->size()));
	
	painter->restore();
}

void GraveZone::addCardImpl(CardItem *card, int x, int /*y*/)
{
	cards->insert(x, card);
	card->setPos(0, 0);
	card->setVisible(false);
	card->resetState();
	card->setParentItem(this);
}

void GraveZone::handleDropEvent(int cardId, CardZone *startZone, const QPoint &/*dropPoint*/, bool /*faceDown*/)
{
	player->client->moveCard(cardId, startZone->getName(), getName(), 0, 0);
}

void GraveZone::reorganizeCards()
{
	update(boundingRect());
}

void GraveZone::mousePressEvent(QGraphicsSceneMouseEvent *event)
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

void GraveZone::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	if ((event->screenPos() - event->buttonDownScreenPos(Qt::LeftButton)).manhattanLength() < QApplication::startDragDistance())
		return;

	if (cards->empty())
		return;
		
	bool faceDown = event->modifiers().testFlag(Qt::ShiftModifier);
	CardItem *card = cards->at(0);
	CardDragItem *drag = card->createDragItem(this, card->getId(), event->pos(), event->scenePos(), faceDown);
	drag->grabMouse();
	setCursor(Qt::OpenHandCursor);
}

void GraveZone::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	Q_UNUSED(event);
	setCursor(Qt::OpenHandCursor);
}
