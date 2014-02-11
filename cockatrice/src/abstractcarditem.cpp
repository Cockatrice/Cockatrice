#include <QPainter>
#include <QGraphicsScene>
#include <QCursor>
#include <QGraphicsSceneMouseEvent>
#include <math.h>
#include "carddatabase.h"
#include "cardinfowidget.h"
#include "abstractcarditem.h"
#include "settingscache.h"
#include "main.h"
#include "gamescene.h"
#include <QDebug>

AbstractCardItem::AbstractCardItem(const QString &_name, Player *_owner, int _id, QGraphicsItem *parent)
    : ArrowTarget(_owner, parent), infoWidget(0), id(_id), name(_name), tapped(false), facedown(false), tapAngle(0), isHovered(false), realZValue(0)
{
    setCursor(Qt::OpenHandCursor);
    setFlag(ItemIsSelectable);
    setCacheMode(DeviceCoordinateCache);
    
    connect(db, SIGNAL(cardListChanged()), this, SLOT(cardInfoUpdated()));
    connect(settingsCache, SIGNAL(displayCardNamesChanged()), this, SLOT(callUpdate()));
    cardInfoUpdated();
}

AbstractCardItem::~AbstractCardItem()
{
    emit deleteCardInfoPopup(name);
}

QRectF AbstractCardItem::boundingRect() const
{
    return QRectF(0, 0, CARD_WIDTH, CARD_HEIGHT);
}

void AbstractCardItem::pixmapUpdated()
{
    update();
    emit sigPixmapUpdated();
}

void AbstractCardItem::cardInfoUpdated()
{
    info = db->getCard(name);
    connect(info, SIGNAL(pixmapUpdated()), this, SLOT(pixmapUpdated()));
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
    int fontSize = round(translatedSize.height() / 8);
    if (fontSize < 9)
        fontSize = 9;
    if (fontSize > 10)
        fontSize = 10;
    f.setPixelSize(fontSize);

    painter->setFont(f);
}

void AbstractCardItem::paintPicture(QPainter *painter, const QSizeF &translatedSize, int angle)
{
    qreal scaleFactor = translatedSize.width() / boundingRect().width();
    
    CardInfo *imageSource = facedown ? db->getCard() : info;
    QPixmap *translatedPixmap = imageSource->getPixmap(translatedSize.toSize());
    painter->save();
    QColor bgColor = Qt::transparent;
    if (translatedPixmap) {
        painter->save();
        transformPainter(painter, translatedSize, angle);
        painter->drawPixmap(QPointF(0, 0), *translatedPixmap);
        painter->restore();
    } else {
        QString colorStr;
        if (!color.isEmpty())
            colorStr = color;
        else if (info->getColors().size() > 1)
            colorStr = "m";
        else if (!info->getColors().isEmpty())
            colorStr = info->getColors().first().toLower();
        
        if (colorStr == "b")
            bgColor = QColor(0, 0, 0);
        else if (colorStr == "u")
            bgColor = QColor(0, 140, 180);
        else if (colorStr == "w")
            bgColor = QColor(255, 250, 140);
        else if (colorStr == "r")
            bgColor = QColor(230, 0, 0);
        else if (colorStr == "g")
            bgColor = QColor(0, 160, 0);
        else if (colorStr == "m")
            bgColor = QColor(250, 190, 30);
        else
            bgColor = QColor(230, 230, 230);
    }
    painter->setBrush(bgColor);
    QPen pen(Qt::black);
    pen.setWidth(2);
    painter->setPen(pen);
    painter->drawRect(QRectF(1, 1, CARD_WIDTH - 2, CARD_HEIGHT - 2));
    
    if (!translatedPixmap || settingsCache->getDisplayCardNames() || facedown) {
        painter->save();
        transformPainter(painter, translatedSize, angle);
        painter->setPen(Qt::white);
        painter->setBackground(Qt::black);
        painter->setBackgroundMode(Qt::OpaqueMode);
        QString nameStr;
        if (facedown)
            nameStr = "# " + QString::number(id);
        else
            nameStr = name;
        painter->drawText(QRectF(3 * scaleFactor, 3 * scaleFactor, translatedSize.width() - 6 * scaleFactor, translatedSize.height() - 6 * scaleFactor), Qt::AlignTop | Qt::AlignLeft | Qt::TextWrapAnywhere, nameStr);
        painter->restore();
    }
    
    painter->restore();
}

void AbstractCardItem::paint(QPainter *painter, const QStyleOptionGraphicsItem */*option*/, QWidget */*widget*/)
{
    painter->save();

    QSizeF translatedSize = getTranslatedSize(painter);
    paintPicture(painter, translatedSize, tapAngle);
    
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, false);
    transformPainter(painter, translatedSize, tapAngle);
    if (isSelected()) {
        painter->setPen(Qt::red);
        painter->drawRect(QRectF(0.5, 0.5, translatedSize.width() - 1, translatedSize.height() - 1));
    } else if (isHovered) {
        painter->setPen(Qt::yellow);
        painter->drawRect(QRectF(0.5, 0.5, translatedSize.width() - 1, translatedSize.height() - 1));
    }
    painter->restore();

    painter->restore();
}

void AbstractCardItem::setName(const QString &_name)
{
    if (name == _name)
        return;
    
    emit deleteCardInfoPopup(name);
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
        static_cast<GameScene *>(scene())->registerAnimationItem(this);
    else {
        tapAngle = tapped ? 90 : 0;
        setTransform(QTransform().translate((float) CARD_WIDTH / 2, (float) CARD_HEIGHT / 2).rotate(tapAngle).translate((float) -CARD_WIDTH / 2, (float) -CARD_HEIGHT / 2));
        update();
    }
}

void AbstractCardItem::setFaceDown(bool _facedown)
{
    facedown = _facedown;
    update();
    emit updateCardMenu(this);
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
    if (event->button() == Qt::MidButton)
        emit deleteCardInfoPopup(name);
    
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

