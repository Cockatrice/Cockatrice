#include <QPainter>
#include <QGraphicsScene>
#include <QCursor>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include "carddatabase.h"
#include "abstractcarditem.h"
#include "main.h"
#include <QDebug>

AbstractCardItem::AbstractCardItem(const QString &_name, Player *_owner, QGraphicsItem *parent)
	: ArrowTarget(_owner, parent), info(db->getCard(_name)), name(_name), tapped(false)
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
		QFont f("Times");
		f.setStyleHint(QFont::Serif);
		f.setPixelSize(12);
		painter->setFont(f);
		QString colorStr;
		if (!color.isEmpty())
			colorStr = color;
		else if (info->getColors().size() > 1)
			colorStr = "m";
		else if (!info->getColors().isEmpty())
			colorStr = info->getColors().first().toLower();
		
		QColor bgColor;
		QColor textColor = Qt::white;
		if (colorStr == "b")
			bgColor = QColor(0, 0, 0);
		else if (colorStr == "u")
			bgColor = QColor(0, 140, 180);
		else if (colorStr == "w") {
			bgColor = QColor(255, 250, 140);
			textColor = Qt::black;
		} else if (colorStr == "r")
			bgColor = QColor(230, 0, 0);
		else if (colorStr == "g")
			bgColor = QColor(0, 160, 0);
		else if (colorStr == "m") {
			bgColor = QColor(250, 190, 30);
			textColor = Qt::black;
		} else {
			bgColor = QColor(230, 230, 230);
			textColor = Qt::black;
		}
		painter->setBrush(bgColor);
		QPen pen(Qt::black);
		painter->setPen(pen);

		painter->drawRect(QRectF(0.5, 0.5, CARD_WIDTH - 1, CARD_HEIGHT - 1));
		
		pen.setWidth(3);
		painter->setPen(pen);
		painter->drawRect(QRectF(3, 3, CARD_WIDTH - 6, CARD_HEIGHT - 6));
		painter->setPen(textColor);
		painter->drawText(QRectF(5, 5, CARD_WIDTH - 15, CARD_HEIGHT - 15), Qt::AlignTop | Qt::AlignLeft | Qt::TextWordWrap, name);
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

void AbstractCardItem::setColor(const QString &_color)
{
	color = _color;
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

