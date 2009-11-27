#include <QApplication>
#include <QtGui>
#include "carditem.h"
#include "carddragitem.h"
#include "carddatabase.h"
#include "cardzone.h"
#include "tablezone.h"
#include "player.h"
#include "arrowitem.h"
#include "main.h"
#include "protocol_datastructures.h"

CardItem::CardItem(Player *_owner, const QString &_name, int _cardid, QGraphicsItem *parent)
	: AbstractCardItem(_name, parent), owner(_owner), id(_cardid), attacking(false), facedown(false), counters(0), doesntUntap(false), dragItem(NULL)
{
	owner->addCard(this);
}

CardItem::~CardItem()
{
	deleteDragItem();
}

void CardItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	painter->save();
	AbstractCardItem::paint(painter, option, widget);
	if (counters)
		paintNumberEllipse(counters, painter);
	painter->restore();
}

void CardItem::setAttacking(bool _attacking)
{
	attacking = _attacking;
	update();
}

void CardItem::setFaceDown(bool _facedown)
{
	facedown = _facedown;
	if (facedown)
		setName(QString());
	update();
}

void CardItem::setCounters(int _counters)
{
	counters = _counters;
	update();
}

void CardItem::setAnnotation(const QString &_annotation)
{
	annotation = _annotation;
	update();
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
	update();
}

void CardItem::processCardInfo(ServerInfo_Card *info)
{
	id = info->getId();
	name = info->getName();
	attacking = info->getAttacking();
	counters = info->getCounters();
	annotation = info->getAnnotation();
	tapped = info->getTapped();

	update();
}

CardDragItem *CardItem::createDragItem(int _id, const QPointF &_pos, const QPointF &_scenePos, bool faceDown)
{
	deleteDragItem();
	dragItem = new CardDragItem(this, _id, _pos, faceDown);
	scene()->addItem(dragItem);
	dragItem->updatePosition(_scenePos);

	return dragItem;
}

void CardItem::deleteDragItem()
{
	delete dragItem;
	dragItem = NULL;
}

void CardItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	if (event->buttons().testFlag(Qt::RightButton)) {
		if ((event->screenPos() - event->buttonDownScreenPos(Qt::RightButton)).manhattanLength() < 2 * QApplication::startDragDistance())
			return;
		
		QColor arrowColor = Qt::red;
		if (event->modifiers().testFlag(Qt::ControlModifier))
			arrowColor = Qt::yellow;
		else if (event->modifiers().testFlag(Qt::AltModifier))
			arrowColor = Qt::blue;
		else if (event->modifiers().testFlag(Qt::ShiftModifier))
			arrowColor = Qt::green;
		
		ArrowDragItem *arrow = new ArrowDragItem(this, arrowColor);
		scene()->addItem(arrow);
		arrow->grabMouse();
	} else {
		if ((event->screenPos() - event->buttonDownScreenPos(Qt::LeftButton)).manhattanLength() < 2 * QApplication::startDragDistance())
			return;
		bool faceDown = event->modifiers().testFlag(Qt::ShiftModifier) || facedown;
	
		createDragItem(id, event->pos(), event->scenePos(), faceDown);
		dragItem->grabMouse();
		
		CardZone *zone = (CardZone *) parentItem();
	
		QList<QGraphicsItem *> sel = scene()->selectedItems();
		int j = 0;
		for (int i = 0; i < sel.size(); i++) {
			CardItem *c = (CardItem *) sel.at(i);
			if (c == this)
				continue;
			++j;
			QPointF childPos;
			if (zone->getHasCardAttr())
				childPos = c->pos() - pos();
			else
				childPos = QPointF(j * CARD_WIDTH / 2, 0);
			CardDragItem *drag = new CardDragItem(c, c->getId(), childPos, false, dragItem);
			drag->setPos(dragItem->pos() + childPos);
			scene()->addItem(drag);
		}
	}
	setCursor(Qt::OpenHandCursor);
}

void CardItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	if (event->button() == Qt::RightButton)
		qgraphicsitem_cast<CardZone *>(parentItem())->getPlayer()->showCardMenu(event->screenPos());
	setCursor(Qt::OpenHandCursor);
}

void CardItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
	event->accept();

	CardZone *zone = (CardZone *) parentItem();
	// Do nothing if the card belongs to another player
	if (!zone->getPlayer()->getLocal())
		return;

	TableZone *tz = qobject_cast<TableZone *>(zone);
	if (tz)
		tz->toggleTapped();
	else {
		TableZone *table = zone->getPlayer()->getTable();
		QPoint gridPoint = table->getFreeGridPoint(info->getTableRow());
		table->handleDropEventByGrid(id, zone, gridPoint, false);
	}
}

void CardItem::processHoverEvent()
{
	emit hovered(this);
}

void CardItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
	processHoverEvent();
	QGraphicsItem::hoverEnterEvent(event);
}
