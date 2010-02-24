#include <QPainter>
#include <QGraphicsScene>
#include <QCursor>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include "carddatabase.h"
#include "abstractcarditem.h"
#include "main.h"
#include <QDebug>

AbstractCardItem::AbstractCardItem(const QString &_name, QGraphicsItem *parent)
	: AbstractGraphicsItem(parent), info(db->getCard(_name)), name(_name), tapped(false)
{
	setCursor(Qt::OpenHandCursor);
	setFlag(ItemIsSelectable);
	setAcceptsHoverEvents(true);
	setCacheMode(DeviceCoordinateCache);

	connect(info, SIGNAL(pixmapUpdated()), this, SLOT(pixmapUpdated()));
}

AbstractCardItem::~AbstractCardItem()
{
	qDebug(QString("AbstractCardItem destructor: %1").arg(name).toLatin1());
}

QRectF AbstractCardItem::boundingRect() const
{
	return QRectF(0, 0, CARD_WIDTH, CARD_HEIGHT);
}

void AbstractCardItem::pixmapUpdated()
{
	update();
}

void AbstractCardItem::paint(QPainter *painter, const QStyleOptionGraphicsItem */*option*/, QWidget */*widget*/)
{
	painter->save();
	QSizeF translatedSize = painter->combinedTransform().mapRect(boundingRect()).size();
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
		QFont f("Serif");
		f.setStyleHint(QFont::Serif);
		f.setPixelSize(11);
		painter->setFont(f);
		painter->setBrush(QColor(230, 230, 230));
		qDebug() <<"COLORS:::::" << info->getColors();
		QString color;
		QPen pen;
		if(!info->getColors().empty())
		{
			color = info->getColors().first();
			if(color == "B")
				painter->setBrush(QColor(0,0,0));
			if(color == "U")
				painter->setBrush(QColor(0,140,180));
			if(color == "W")
				painter->setBrush(QColor(255,250,140));
			if(color == "R")
				painter->setBrush(QColor(230,0,0));
			if(color == "G")
				painter->setBrush(QColor(0,160,0));
			if(info->getColors().size() > 1)
			{
				painter->setBrush(QColor(250,190,30));
				color = "M"; // Multicolor
			}
				
		}		
		
		painter->setPen(Qt::black);

		painter->drawRect(QRectF(0.5, 0.5, CARD_WIDTH - 1, CARD_HEIGHT - 1));
		
		pen.setWidth(3);
		painter->setPen(pen);
		painter->drawRect(QRectF(3, 3, CARD_WIDTH - 6, CARD_HEIGHT - 6));
		painter->setPen(Qt::white);
		if(color == "W" || color == "" || color == "M")
			painter->setPen(Qt::black);
		painter->drawText(QRectF(5, 5, CARD_WIDTH - 15, CARD_HEIGHT - 15), Qt::AlignTop | Qt::AlignLeft | Qt::TextWordWrap, name);
		if(info->getCardType().contains("Creature"))
		{
			painter->drawText(QRectF(CARD_WIDTH - 40, CARD_HEIGHT - 25, 30, 30), Qt::AlignTop | Qt::AlignRight | Qt::TextWordWrap, info->getPowTough());
		}
	}
	painter->restore();

	if (isSelected()) {
		painter->setPen(Qt::red);
		painter->drawRect(QRectF(0.5, 0.5, CARD_WIDTH - 1, CARD_HEIGHT - 1));
	}

	painter->restore();
}

void AbstractCardItem::setName(const QString &_name)
{
	disconnect(info, 0, this, 0);
	name = _name;
	info = db->getCard(name);
	connect(info, SIGNAL(pixmapUpdated()), this, SLOT(pixmapUpdated()));
	update();
}

void AbstractCardItem::setTapped(bool _tapped)
{
	tapped = _tapped;
	if (tapped)
		setTransform(QTransform().translate((float) CARD_WIDTH / 2, (float) CARD_HEIGHT / 2).rotate(90).translate((float) -CARD_WIDTH / 2, (float) -CARD_HEIGHT / 2));
	else
		setTransform(QTransform());
	update();
}

void AbstractCardItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	if (!isSelected()) {
		scene()->clearSelection();
		setSelected(true);
	}
	if (event->button() == Qt::LeftButton)
		setCursor(Qt::ClosedHandCursor);
	event->accept();
}

void AbstractCardItem::processHoverEvent()
{
	emit hovered(this);
}

void AbstractCardItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
	processHoverEvent();
	QGraphicsItem::hoverEnterEvent(event);
}

QVariant AbstractCardItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
	if (change == ItemSelectedHasChanged) {
		update();
		return value;
	} else
		return QGraphicsItem::itemChange(change, value);
}

