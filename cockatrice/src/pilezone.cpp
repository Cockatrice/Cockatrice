#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QApplication>
#include "pilezone.h"
#include "player.h"
#include "carddragitem.h"
#include "zoneviewzone.h"
#include "protocol_items.h"

PileZone::PileZone(Player *_p, const QString &_name, bool _isShufflable, bool _contentsKnown, QGraphicsItem *parent)
	: CardZone(_p, _name, false, _isShufflable, _contentsKnown, parent)
{
	setCacheMode(DeviceCoordinateCache); // Do not move this line to the parent constructor!
	setAcceptsHoverEvents(true);
	setCursor(Qt::OpenHandCursor);
	
	setTransform(QTransform().translate((float) CARD_WIDTH / 2, (float) CARD_HEIGHT / 2).rotate(90).translate((float) -CARD_WIDTH / 2, (float) -CARD_HEIGHT / 2));
}

QRectF PileZone::boundingRect() const
{
	return QRectF(0, 0, CARD_WIDTH, CARD_HEIGHT);
}

void PileZone::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
	if (!cards.isEmpty())
		cards.at(0)->paintPicture(painter, 90);

	painter->drawRect(QRectF(0.5, 0.5, CARD_WIDTH - 1, CARD_HEIGHT - 1));
	
	painter->translate((float) CARD_WIDTH / 2, (float) CARD_HEIGHT / 2);
	painter->rotate(-90);
	painter->translate((float) -CARD_WIDTH / 2, (float) -CARD_HEIGHT / 2);
	paintNumberEllipse(cards.size(), 28, Qt::white, -1, -1, painter);
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

void PileZone::handleDropEvent(const QList<CardDragItem *> &dragItems, CardZone *startZone, const QPoint &/*dropPoint*/, bool /*faceDown*/)
{
	QList<CardId *> idList;
	for (int i = 0; i < dragItems.size(); ++i)
		idList.append(new CardId(dragItems[i]->getId()));
	
	player->sendGameCommand(new Command_MoveCard(-1, startZone->getName(), idList, player->getId(), getName(), 0, 0, false));
}

void PileZone::reorganizeCards()
{
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
		cards[0]->processHoverEvent();
	QGraphicsItem::hoverEnterEvent(event);
}
