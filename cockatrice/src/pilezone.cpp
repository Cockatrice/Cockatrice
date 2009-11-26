#include <QtGui>
#include "pilezone.h"
#include "player.h"
#include "game.h"
#include "client.h"
#include "carddragitem.h"
#include "zoneviewzone.h"
#include "protocol_items.h"

PileZone::PileZone(Player *_p, const QString &_name, bool _isShufflable, bool _contentsKnown, QGraphicsItem *parent)
	: CardZone(_p, _name, false, _isShufflable, _contentsKnown, parent)
{
	setCacheMode(DeviceCoordinateCache); // Do not move this line to the parent constructor!
	setAcceptsHoverEvents(true);
	setCursor(Qt::OpenHandCursor);
}

QRectF PileZone::boundingRect() const
{
	return QRectF(0, 0, CARD_WIDTH, CARD_HEIGHT);
}

void PileZone::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	qDebug("PileZone::paint");
	if (!cards.isEmpty()) {
		painter->save();
		cards.at(0)->paint(painter, option, widget);
		painter->restore();
	}

	paintNumberEllipse(cards.size(), painter);
	painter->drawRect(QRectF(0.5, 0.5, CARD_WIDTH - 1, CARD_HEIGHT - 1));
}

void PileZone::addCardImpl(CardItem *card, int x, int /*y*/)
{
	cards.insert(x, card);
	card->setPos(0, 0);
	if (!contentsKnown()) {
		card->setName(QString());
		card->setId(-1);
	}
	card->setVisible(false);
	card->resetState();
	card->setParentItem(this);
}

void PileZone::handleDropEvent(int cardId, CardZone *startZone, const QPoint &/*dropPoint*/, bool /*faceDown*/)
{
	player->sendGameCommand(new Command_MoveCard(-1, startZone->getName(), cardId, getName(), 0, 0, false));
}

void PileZone::reorganizeCards()
{
	qDebug(QString("PileZone: reorganize, x=%1, y=%2, w=%3, h=%4").arg(boundingRect().x()).arg(boundingRect().y()).arg(boundingRect().width()).arg(boundingRect().height()).toLatin1());
	update();
}

void PileZone::mousePressEvent(QGraphicsSceneMouseEvent *event)
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

void PileZone::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	if ((event->screenPos() - event->buttonDownScreenPos(Qt::LeftButton)).manhattanLength() < QApplication::startDragDistance())
		return;

	if (cards.isEmpty())
		return;

	bool faceDown = event->modifiers().testFlag(Qt::ShiftModifier);
	CardItem *card = cards.at(0);
	const int cardid = contentsKnown() ? card->getId() : 0;
	CardDragItem *drag = card->createDragItem(cardid, event->pos(), event->scenePos(), faceDown);
	drag->grabMouse();
	setCursor(Qt::OpenHandCursor);
}

void PileZone::mouseReleaseEvent(QGraphicsSceneMouseEvent */*event*/)
{
	setCursor(Qt::OpenHandCursor);
}

void PileZone::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
	if (!cards.isEmpty())
		((Game *) player->parent())->hoverCardEvent(cards.at(0));
	QGraphicsItem::hoverEnterEvent(event);
}
