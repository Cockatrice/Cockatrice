#include <QApplication>
#include <QtGui>
#include "carditem.h"
#include "carddragitem.h"
#include "carddatabase.h"
#include "cardzone.h"
#include "tablezone.h"
#include "player.h"
#include "game.h"

CardItem::CardItem(CardDatabase *_db, const QString &_name, int _cardid, QGraphicsItem *parent)
	: AbstractGraphicsItem(parent), db(_db), info(db->getCard(_name)), name(_name), id(_cardid), tapped(false), attacking(false), facedown(false), counters(0), doesntUntap(false), dragItem(NULL)
{
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
	
	QSizeF translatedSize = option->matrix.mapRect(boundingRect()).size();
	if (tapped)
		translatedSize.transpose();
	QPixmap *translatedPixmap = info->getPixmap(translatedSize.toSize());
	painter->save();
	if (translatedPixmap) {
		painter->resetTransform();
		if (tapped) {
			painter->translate(((qreal) translatedSize.height()) / 2, ((qreal) translatedSize.width()) / 2);
			painter->rotate(90);
			painter->translate(-((qreal) translatedSize.width()) / 2, -((qreal) translatedSize.height()) / 2);
		}
		painter->drawPixmap(translatedPixmap->rect(), *translatedPixmap, translatedPixmap->rect());
	} else {
		QFont f;
		f.setStyleHint(QFont::Serif);
		f.setPointSize(8);
		f.setWeight(QFont::Bold);
		painter->setFont(f);
		painter->setBrush(QColor(200, 200, 200));
		painter->setPen(QPen(Qt::black));
		painter->drawRect(QRectF(0.5, 0.5, CARD_WIDTH - 1, CARD_HEIGHT - 1));
		painter->drawText(QRectF(5, 5, CARD_WIDTH - 15, CARD_HEIGHT - 15), Qt::AlignTop | Qt::AlignLeft | Qt::TextWordWrap, name);
	}
	painter->restore();

	if (isSelected()) {
		painter->setPen(Qt::red);
		painter->drawRect(QRectF(0.5, 0.5, CARD_WIDTH - 1, CARD_HEIGHT - 1));
	}
	if (counters)
		paintNumberEllipse(counters, painter);

	painter->restore();
}

void CardItem::setName(const QString &_name)
{
	name = _name;
	info = db->getCard(name);
	update();
}

void CardItem::setTapped(bool _tapped)
{
	tapped = _tapped;
	if (tapped)
		setTransform(QTransform().translate((float) CARD_WIDTH / 2, (float) CARD_HEIGHT / 2).rotate(90).translate((float) -CARD_WIDTH / 2, (float) -CARD_HEIGHT / 2));
	else
		setTransform(QTransform());
	update();
}

void CardItem::setAttacking(bool _attacking)
{
	attacking = _attacking;
	update();
}

void CardItem::setFaceDown(bool _facedown)
{
	facedown = _facedown;
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

	createDragItem(id, event->pos(), event->scenePos(), faceDown);
	dragItem->grabMouse();

	QList<QGraphicsItem *> sel = scene()->selectedItems();
	for (int i = 0; i < sel.size(); i++) {
		CardItem *c = (CardItem *) sel.at(i);
		if (c == this)
			continue;
		CardDragItem *drag = new CardDragItem(c, c->getId(), c->pos() - pos(), false, dragItem);
		drag->setPos(dragItem->pos() + c->pos() - pos());
		scene()->addItem(drag);
	}
	setCursor(Qt::OpenHandCursor);
}

void CardItem::mouseReleaseEvent(QGraphicsSceneMouseEvent */*event*/)
{
	setCursor(Qt::OpenHandCursor);
}

void CardItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
	event->accept();

	CardZone *zone = (CardZone *) parentItem();
	// Do nothing if the card belongs to another player
	if (!zone->getPlayer()->getLocal())
		return;

	if (zone->getName() == "table")
		((TableZone *) zone)->toggleTapped();
	else {
		TableZone *table = (TableZone *) zone->getPlayer()->getZones()->findZone("table");
		QPoint gridPoint = table->getFreeGridPoint(info->getTableRow());
		table->handleDropEvent(id, zone, table->mapFromGrid(gridPoint).toPoint(), false);
	}
}

void CardItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
	((Game *) ((CardZone *) parentItem())->getPlayer()->parent())->hoverCardEvent(this);
	QGraphicsItem::hoverEnterEvent(event);
}

QVariant CardItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
	if (change == ItemSelectedChange) {
		// XXX
		return value;
	} else if (change == ItemSelectedHasChanged) {
		qDebug("selection changed");
		update();
		return value;
	} else
		return QGraphicsItem::itemChange(change, value);
}
