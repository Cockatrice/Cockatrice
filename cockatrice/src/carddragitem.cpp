#include "carddragitem.h"
#include "cardzone.h"
#include <QtGui>

CardDragItem::CardDragItem(QGraphicsScene *scene, CardZone *_startZone, QPixmap *_image, int _id, const QPointF &_hotSpot, QGraphicsItem *parent)
	: QGraphicsItem(parent), image(_image), id(_id), hotSpot(_hotSpot), startZone(_startZone)
{
	if ((hotSpot.x() < 0) || (hotSpot.y() < 0)) {
		qDebug(QString("CardDragItem: coordinate overflow: x = %1, y = %2").arg(hotSpot.x()).arg(hotSpot.y()).toLatin1());
		hotSpot = QPointF();
	} else if ((hotSpot.x() > CARD_WIDTH) || (hotSpot.y() > CARD_HEIGHT)) {
		qDebug(QString("CardDragItem: coordinate overflow: x = %1, y = %2").arg(hotSpot.x()).arg(hotSpot.y()).toLatin1());
		hotSpot = QPointF(CARD_WIDTH, CARD_HEIGHT);
	}

	setZValue(2000000000);
	setCacheMode(DeviceCoordinateCache);

	scene->addItem(this);
}

CardDragItem::~CardDragItem()
{
	qDebug("CardDragItem destructor");
}

QRectF CardDragItem::boundingRect() const
{
	return QRectF(0, 0, CARD_WIDTH, CARD_HEIGHT);
}

void CardDragItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
//	Q_UNUSED(option);
	Q_UNUSED(widget);

	QRectF foo = option->matrix.mapRect(boundingRect());
	QPixmap bar = image->scaled(foo.width(), foo.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	painter->drawPixmap(boundingRect(), bar, bar.rect());

//	painter->drawPixmap(boundingRect(), *image, QRectF(0, 0, image->width(), image->height()));
}

void CardDragItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	QPointF sp = event->scenePos();
	QList<QGraphicsItem *> colliding = scene()->items(sp);

	CardZone *cursorZone = 0;
	for (int i = colliding.size() - 1; i >= 0; i--) {
		if ((cursorZone = qgraphicsitem_cast<CardZone *>(colliding.at(i)))) {
			if (cursorZone->getName() == "table") {
				QPointF cp = cursorZone->scenePos();
				QPointF localpos = sp - hotSpot - cp;
				setPos(QPointF(round(localpos.x() / RASTER_WIDTH) * RASTER_WIDTH, round(localpos.y() / RASTER_HEIGHT) * RASTER_HEIGHT) + cp);
			} else
				setPos(sp - hotSpot);
			break;
		}
	}
	event->accept();
}

void CardDragItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	setCursor(Qt::OpenHandCursor);
	QGraphicsScene *sc = scene();
	QPointF sp = scenePos();
	sc->removeItem(this);
	QList<QGraphicsItem *> colliding = sc->items(event->scenePos());

	qDebug(QString("drop: %1 collisions").arg(colliding.size()).toLatin1());
	CardZone *dropZone = 0;
	for (int i = colliding.size() - 1; i >= 0; i--) {
		QRectF bbox = colliding.at(i)->boundingRect();
		qDebug(QString("bbox x %1 y %2 w %3 h %4").arg(bbox.x()).arg(bbox.y()).arg(bbox.width()).arg(bbox.height()).toLatin1());

		if ((dropZone = qgraphicsitem_cast<CardZone *>(colliding.at(i)))) {
			qDebug("zone found");
			break;
		}
	}

	if (dropZone) {
		dropZone->handleDropEvent(id, startZone, (sp - dropZone->scenePos()).toPoint());
		QList<QGraphicsItem *> childList = childItems();
		for (int i = 0; i < childList.size(); i++) {
			CardDragItem *c = qgraphicsitem_cast<CardDragItem *>(childList.at(i));
			if (!c)
				QMessageBox::critical(0, "fehler", "null");
			dropZone->handleDropEvent(c->id, startZone, (sp - dropZone->scenePos() + c->pos()).toPoint());
		}
	} else
		QMessageBox::critical(0, "fehler", "fehler");

	event->accept();
}
