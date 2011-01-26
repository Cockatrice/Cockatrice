#include <QPainter>
#include <QGraphicsScene>
#include <QCursor>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <math.h>
#include "carddatabase.h"
#include "cardinfowidget.h"
#include "abstractcarditem.h"
#include "settingscache.h"
#include "main.h"
#include <QDebug>
#include <QTimer>

AbstractCardItem::AbstractCardItem(const QString &_name, Player *_owner, QGraphicsItem *parent)
	: ArrowTarget(_owner, parent), info(db->getCard(_name)), infoWidget(0), name(_name), tapped(false), tapAngle(0), isHovered(false), realZValue(0)
{
	setCursor(Qt::OpenHandCursor);
	setFlag(ItemIsSelectable);
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

void AbstractCardItem::setRealZValue(qreal _zValue)
{
	realZValue = _zValue;
	setZValue(_zValue);
}

QSizeF AbstractCardItem::getTranslatedSize(QPainter *painter) const
{
	return QSizeF(
		painter->combinedTransform().map(QLineF(0, 0, boundingRect().width(), 0)).length(),
		painter->combinedTransform().map(QLineF(0, 0, 0, boundingRect().height())).length()
	);
}

void AbstractCardItem::transformPainter(QPainter *painter, const QSizeF &translatedSize, int angle)
{
	QRectF totalBoundingRect = painter->combinedTransform().mapRect(boundingRect());
	
	painter->resetTransform();
	
	QTransform pixmapTransform;
	pixmapTransform.translate(totalBoundingRect.width() / 2, totalBoundingRect.height() / 2);
	pixmapTransform.rotate(angle);
	pixmapTransform.translate(-translatedSize.width() / 2, -translatedSize.height() / 2);
	painter->setTransform(pixmapTransform);

	QFont f;
	int fontSize = translatedSize.height() / 6;
	if (fontSize < 9)
		fontSize = 9;
	f.setPixelSize(fontSize);
	painter->setFont(f);
}

void AbstractCardItem::paintPicture(QPainter *painter, int angle)
{
	QSizeF translatedSize = getTranslatedSize(painter);
	QRectF totalBoundingRect = painter->combinedTransform().mapRect(boundingRect());
	qreal scaleFactor = translatedSize.width() / boundingRect().width();

	QPixmap *translatedPixmap = info->getPixmap(translatedSize.toSize());
	painter->save();
	if (translatedPixmap) {
		transformPainter(painter, translatedSize, angle);
		painter->drawPixmap(QPointF(0, 0), *translatedPixmap);
	} else {
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
		pen.setWidth(2);
		painter->setPen(pen);
		painter->drawRect(QRectF(1, 1, CARD_WIDTH - 2, CARD_HEIGHT - 2));
		
		transformPainter(painter, translatedSize, angle);
		painter->setPen(textColor);
		painter->drawText(QRectF(4 * scaleFactor, 4 * scaleFactor, translatedSize.width() - 8 * scaleFactor, translatedSize.height() - 8 * scaleFactor), Qt::AlignTop | Qt::AlignLeft | Qt::TextWrapAnywhere, name);
	}
	painter->restore();
}

void AbstractCardItem::paint(QPainter *painter, const QStyleOptionGraphicsItem */*option*/, QWidget */*widget*/)
{
	painter->save();

	paintPicture(painter, tapAngle);
	
	if (isSelected()) {
		painter->setPen(Qt::red);
		painter->drawRect(QRectF(0.5, 0.5, CARD_WIDTH - 1, CARD_HEIGHT - 1));
	} else if (isHovered) {
		painter->setPen(Qt::yellow);
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
	setHovered(false);
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

void AbstractCardItem::setHovered(bool _hovered)
{
	if (isHovered == _hovered)
		return;
	
	if (_hovered)
		processHoverEvent();
	isHovered = _hovered;
	setZValue(_hovered ? 2000000004 : realZValue);
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
	else if (event->button() == Qt::MidButton)
		emit showCardInfoPopup(event->screenPos(), name);
	event->accept();
}

void AbstractCardItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	// This function ensures the parent function doesn't mess around with our selection.
	event->accept();
}

void AbstractCardItem::processHoverEvent()
{
	emit hovered(this);
}

QVariant AbstractCardItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
	if (change == ItemSelectedHasChanged) {
		update();
		return value;
	} else
		return QGraphicsItem::itemChange(change, value);
}

