#include <QPainter>
#include <QGraphicsScene>
#include <QCursor>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <math.h>
#include "carddatabase.h"
#include "abstractcarditem.h"
#include "settingscache.h"
#include "main.h"
#include <QDebug>
#include <QTimer>

AbstractCardItem::AbstractCardItem(const QString &_name, Player *_owner, QGraphicsItem *parent)
	: ArrowTarget(_owner, parent), info(db->getCard(_name)), name(_name), tapped(false), tapAngle(0)
{
	setCursor(Qt::OpenHandCursor);
	setFlag(ItemIsSelectable);
	setAcceptsHoverEvents(true);
	setCacheMode(DeviceCoordinateCache);

	connect(info, SIGNAL(pixmapUpdated()), this, SLOT(pixmapUpdated()));
	
	animationTimer = new QTimer(this);
	animationTimer->setSingleShot(false);
	connect(animationTimer, SIGNAL(timeout()), this, SLOT(animationEvent()));
}

AbstractCardItem::~AbstractCardItem()
{
	qDebug() << "AbstractCardItem destructor:" << name;
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
	qreal w = painter->combinedTransform().map(QLineF(0, 0, boundingRect().width(), 0)).length();
	qreal h = painter->combinedTransform().map(QLineF(0, 0, 0, boundingRect().height())).length();
	QSizeF translatedSize(w, h);
	QRectF totalBoundingRect = painter->combinedTransform().mapRect(boundingRect());
	QPixmap *translatedPixmap = info->getPixmap(translatedSize.toSize());
	painter->save();
	if (translatedPixmap) {
		painter->resetTransform();
		QTransform pixmapTransform;
		pixmapTransform.translate(totalBoundingRect.width() / 2, totalBoundingRect.height() / 2);
		pixmapTransform.rotate(tapAngle);
		QPointF transPoint = QPointF(-w / 2, -h / 2);
		pixmapTransform.translate(transPoint.x(), transPoint.y());
		painter->setTransform(pixmapTransform);
		
		painter->drawPixmap(QPointF(0, 0), *translatedPixmap);
	} else {
		QFont f;
		int fontSize = h / 6;
		if (fontSize < 9)
			fontSize = 9;
		f.setPixelSize(fontSize);
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
		
		QRectF textRect = painter->combinedTransform().mapRect(QRectF(5, 5, CARD_WIDTH - 15, CARD_HEIGHT - 15));
		painter->resetTransform();
		QTransform pixmapTransform;
		pixmapTransform.translate(totalBoundingRect.width() / 2, totalBoundingRect.height() / 2);
		pixmapTransform.rotate(tapAngle);
		QPointF transPoint = QPointF(-w / 2, -h / 2);
		pixmapTransform.translate(transPoint.x(), transPoint.y());
		painter->setTransform(pixmapTransform);

		painter->drawText(textRect, Qt::AlignTop | Qt::AlignLeft | Qt::TextWrapAnywhere, name);
	}
	painter->restore();

	if (isSelected()) {
		painter->setPen(Qt::red);
		painter->drawRect(QRectF(0.5, 0.5, CARD_WIDTH - 1, CARD_HEIGHT - 1));
	}

	painter->restore();
}

void AbstractCardItem::animationEvent()
{
	int delta = 18;
	if (!tapped)
		delta *= -1;
	
	tapAngle += delta;
	
	setTransform(QTransform().translate((float) CARD_WIDTH / 2, (float) CARD_HEIGHT / 2).rotate(tapAngle).translate((float) -CARD_WIDTH / 2, (float) -CARD_HEIGHT / 2));
	update();

	if ((tapped && (tapAngle >= 90)) || (!tapped && (tapAngle <= 0)))
		animationTimer->stop();
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

void AbstractCardItem::setTapped(bool _tapped, bool canAnimate)
{
	if (tapped == _tapped)
		return;
	
	tapped = _tapped;
	if (settingsCache->getTapAnimation() && canAnimate)
		animationTimer->start(25);
	else {
		tapAngle = tapped ? 90 : 0;
		setTransform(QTransform().translate((float) CARD_WIDTH / 2, (float) CARD_HEIGHT / 2).rotate(tapAngle).translate((float) -CARD_WIDTH / 2, (float) -CARD_HEIGHT / 2));
		update();
	}
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

