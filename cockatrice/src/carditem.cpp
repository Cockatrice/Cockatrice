#include <QApplication>
#include <QtGui>
#include "carditem.h"
#include "carddragitem.h"
#include "carddatabase.h"
#include "cardzone.h"
#include "tablezone.h"
#include "player.h"

CardItem::CardItem(CardDatabase *_db, const QString &_name, int _cardid, QGraphicsItem *parent)
	: QGraphicsItem(parent), db(_db), name(_name), id(_cardid), tapped(false), attacking(false), facedown(false), counters(0), doesntUntap(false), dragItem(NULL)
{
	image = db->getCard(name)->getPixmap();
	setCursor(Qt::OpenHandCursor);
	setFlag(ItemIsSelectable);
	setAcceptsHoverEvents(true);
	setCacheMode(DeviceCoordinateCache);
}

CardItem::~CardItem()
{
	deleteDragItem();
	qDebug(QString("CardItem destructor: %1").arg(name).toLatin1());
}

QRectF CardItem::boundingRect() const
{
	return QRectF(0, 0, CARD_WIDTH, CARD_HEIGHT);
}

void CardItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget */*widget*/)
{
	painter->save();
	QRectF foo = option->matrix.mapRect(boundingRect());
	qDebug(QString("%1: w=%2,h=%3").arg(name).arg(foo.width()).arg(foo.height()).toLatin1());
	QPixmap bar;
	if (tapped)
		bar = image->scaled((int) foo.height(), (int) foo.width(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	else
		bar = image->scaled((int) foo.width(), (int) foo.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
//	painter->drawPixmap(boundingRect(), *image, QRectF(0, 0, image->width(), image->height()));
	painter->drawPixmap(boundingRect(), bar, bar.rect());
	if (isSelected()) {
		painter->setPen(QPen(QColor("red")));
		painter->drawRect(QRectF(1, 1, CARD_WIDTH - 2, CARD_HEIGHT - 2));
	}
	if (counters) {
		painter->setFont(QFont("Times", 32, QFont::Bold));
		painter->setPen(QPen(QColor("black")));
		painter->setBackground(QBrush(QColor(255, 255, 255, 100)));
		painter->setBackgroundMode(Qt::OpaqueMode);
		painter->drawText(boundingRect(), Qt::AlignCenter, QString::number(counters));
	}
	painter->restore();
}

void CardItem::setName(const QString &_name)
{
	name = _name;
	image = db->getCard(name)->getPixmap();
	update(boundingRect());
}

void CardItem::setTapped(bool _tapped)
{
	tapped = _tapped;
	if (tapped)
		setTransform(QTransform().translate((float) CARD_WIDTH / 2, (float) CARD_HEIGHT / 2).rotate(90).translate((float) -CARD_WIDTH / 2, (float) -CARD_HEIGHT / 2));
	else
		setTransform(QTransform());
	update(boundingRect());
}

void CardItem::setAttacking(bool _attacking)
{
	attacking = _attacking;
	update(boundingRect());
}

void CardItem::setFaceDown(bool _facedown)
{
	facedown = _facedown;
	update(boundingRect());
}

void CardItem::setCounters(int _counters)
{
	counters = _counters;
	update(boundingRect());
}

void CardItem::setAnnotation(const QString &_annotation)
{
	annotation = _annotation;
	update(boundingRect());
}

void CardItem::setDoesntUntap(bool _doesntUntap)
{
	doesntUntap = _doesntUntap;
}

void CardItem::resetState()
{
	attacking = false;
	facedown = false;
	counters = 0;
	annotation = QString();
	setTapped(false);
	setDoesntUntap(false);
	update(boundingRect());
}

CardDragItem *CardItem::createDragItem(CardZone *startZone, int _id, const QPointF &_pos, const QPointF &_scenePos, bool faceDown)
{
	deleteDragItem();
	dragItem = new CardDragItem(scene(), startZone, image, _id, _pos, faceDown);
	dragItem->setPos(_scenePos - dragItem->getHotSpot());

	return dragItem;
}

void CardItem::deleteDragItem()
{
	delete dragItem;
	dragItem = NULL;
}

void CardItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	if (!isSelected()) {
		scene()->clearSelection();
		setSelected(true);
	}
	if (event->button() == Qt::LeftButton) {
		setCursor(Qt::ClosedHandCursor);
	} else if (event->button() == Qt::RightButton) {
		qgraphicsitem_cast<CardZone *>(parentItem())->getPlayer()->showCardMenu(event->screenPos());
	}
	event->accept();
}

void CardItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	if ((event->screenPos() - event->buttonDownScreenPos(Qt::LeftButton)).manhattanLength() < QApplication::startDragDistance())
		return;
	bool faceDown = event->modifiers().testFlag(Qt::ShiftModifier) || facedown;

	createDragItem((CardZone *) parentItem(), id, event->pos(), event->scenePos(), faceDown);
	dragItem->grabMouse();

	QList<QGraphicsItem *> sel = scene()->selectedItems();
	for (int i = 0; i < sel.size(); i++) {
		CardItem *c = (CardItem *) sel.at(i);
		if (c == this)
			continue;
		CardDragItem *drag = new CardDragItem(scene(), (CardZone *) parentItem(), c->getImage(), c->getId(), QPointF(), false, dragItem);
		drag->setPos(c->pos() - pos());
	}
	setCursor(Qt::OpenHandCursor);
}

void CardItem::mouseReleaseEvent(QGraphicsSceneMouseEvent */*event*/)
{
	setCursor(Qt::OpenHandCursor);
}

void CardItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
	if (!isSelected()) {
		// Unselect all items, then select this one
		scene()->setSelectionArea(QPainterPath());
		setSelected(true);
	}
	event->accept();

	CardZone *zone = (CardZone *) parentItem();
	// Do nothing if the card belongs to another player
	if (!zone->getPlayer()->getLocal())
		return;

	if (zone->getHasCardAttr())
		((TableZone *) zone)->toggleTapped();
}

void CardItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
	((CardZone *) parentItem())->hoverCardEvent(this);
	QGraphicsItem::hoverEnterEvent(event);
}

void CardItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
	QGraphicsItem::hoverLeaveEvent(event);
}

QVariant CardItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
	if (change == ItemSelectedChange) {
		// XXX
		return value;
	} else if (change == ItemSelectedHasChanged) {
		qDebug("selection changed");
		update(boundingRect());
		return value;
	} else
		return QGraphicsItem::itemChange(change, value);
}
