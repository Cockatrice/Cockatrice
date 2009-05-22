#include <QtGui>
#include "libraryzone.h"
#include "player.h"
#include "client.h"
#include "carddatabase.h"
#include "carddragitem.h"
#include "zoneviewzone.h"

LibraryZone::LibraryZone(Player *_p, QGraphicsItem *parent)
	: CardZone(_p, "deck", false, true, parent)
{
	cards = new CardList(false);
	setCursor(Qt::OpenHandCursor);
	setCacheMode(DeviceCoordinateCache);
	
	image = player->getDb()->getCard()->getPixmap();
}

QRectF LibraryZone::boundingRect() const
{
	return QRectF(0, 0, CARD_WIDTH, CARD_HEIGHT);
}

void LibraryZone::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	painter->save();

	QRectF foo = option->matrix.mapRect(boundingRect());
	QPixmap bar = image->scaled((int) foo.width(), (int) foo.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	painter->drawPixmap(boundingRect(), bar, bar.rect());

	painter->setFont(QFont("Times", 32, QFont::Bold));
	painter->setPen(QPen(QColor("black")));
	painter->setBackground(QBrush(QColor(255, 255, 255, 100)));
	painter->setBackgroundMode(Qt::OpaqueMode);
	painter->drawText(boundingRect(), Qt::AlignCenter, QString::number(cards->size()));

	painter->restore();
}

void LibraryZone::addCardImpl(CardItem *card, int x, int /*y*/)
{
	cards->insert(x, card);
	card->setId(-1);
	card->setName(QString());
	card->setPos(0, 0);
	card->setVisible(false);
	card->resetState();
	card->setParentItem(this);
}

void LibraryZone::handleDropEvent(int cardId, CardZone *startZone, const QPoint &/*dropPoint*/, bool /*faceDown*/)
{
	player->client->moveCard(cardId, startZone->getName(), getName(), 0, 0);
}

void LibraryZone::reorganizeCards()
{
	update(boundingRect());
}

void LibraryZone::mousePressEvent(QGraphicsSceneMouseEvent *event)
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

void LibraryZone::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	if ((event->screenPos() - event->buttonDownScreenPos(Qt::LeftButton)).manhattanLength() < QApplication::startDragDistance())
		return;

	if (cards->empty())
		return;
		
	bool faceDown = event->modifiers().testFlag(Qt::ShiftModifier);
	CardItem *card = cards->at(0);
	CardDragItem *drag = card->createDragItem(this, 0, event->pos(), event->scenePos(), faceDown);
	drag->grabMouse();
	setCursor(Qt::OpenHandCursor);
}

void LibraryZone::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	Q_UNUSED(event);
	setCursor(Qt::OpenHandCursor);
}
