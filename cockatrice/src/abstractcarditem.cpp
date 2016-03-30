#include <QPainter>
#include <QGraphicsScene>
#include <QCursor>
#include <QGraphicsSceneMouseEvent>
#include <cmath>
#ifdef _WIN32
#include "round.h"
#endif /* _WIN32 */
#include "carddatabase.h"
#include "abstractcarditem.h"
#include "pictureloader.h"
#include "settingscache.h"
#include "main.h"
#include "gamescene.h"

AbstractCardItem::AbstractCardItem(const QString &_name, Player *_owner, int _id, QGraphicsItem *parent)
    : ArrowTarget(_owner, parent), id(_id), name(_name), tapped(false), facedown(false), tapAngle(0), bgColor(Qt::transparent), isHovered(false), realZValue(0)
{
    setCursor(Qt::OpenHandCursor);
    setFlag(ItemIsSelectable);
    setCacheMode(DeviceCoordinateCache);
    
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
    if(info)
        connect(info, SIGNAL(pixmapUpdated()), this, SLOT(pixmapUpdated()));

    cacheBgColor();
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
    QPixmap translatedPixmap;
    bool paintImage = true;

    if(facedown)
    {
        // never reveal card color, always paint the card back
        PictureLoader::getPixmap(translatedPixmap, nullptr, translatedSize.toSize());
    } else {
        // don't even spend time trying to load the picture if our size is too small
        if(translatedSize.width() > 10)
        {
            PictureLoader::getPixmap(translatedPixmap, info, translatedSize.toSize());
            if(translatedPixmap.isNull())
                paintImage = false;
        } else {
            paintImage = false;
        }
    }

    painter->save();
    
    if (paintImage) {
        painter->save();
        transformPainter(painter, translatedSize, angle);
        painter->drawPixmap(QPointF(1, 1), translatedPixmap);
        painter->restore();
    } else {
        painter->setBrush(bgColor);
    }

    QPen pen(Qt::black);
    pen.setJoinStyle(Qt::MiterJoin);
    const int penWidth = 2;
    pen.setWidth(penWidth);
    painter->setPen(pen);

    if (!angle)
        painter->drawRect(QRectF(0, 0, CARD_WIDTH - 1, CARD_HEIGHT - penWidth));
    else
        painter->drawRect(QRectF(1, 1, CARD_WIDTH - 2, CARD_HEIGHT - 1.5));
    
    if (translatedPixmap.isNull() || settingsCache->getDisplayCardNames() || facedown) {
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

void AbstractCardItem::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    painter->save();

    QSizeF translatedSize = getTranslatedSize(painter);
    paintPicture(painter, translatedSize, tapAngle);
    
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, false);
    transformPainter(painter, translatedSize, tapAngle);

    if (isSelected() || isHovered) {
        QPen pen;
        if (isHovered)
            pen.setColor(Qt::yellow);
        if (isSelected())
            pen.setColor(Qt::red);
        const int penWidth = 1;
        pen.setWidth(penWidth);
        painter->setPen(pen);
        painter->drawRect(QRectF(0, 0, translatedSize.width() + penWidth, translatedSize.height() - penWidth));
    }

    painter->restore();

    painter->restore();
}

void AbstractCardItem::setName(const QString &_name)
{
    if (name == _name)
        return;
    
    emit deleteCardInfoPopup(name);
    if(info)
        disconnect(info, nullptr, this, nullptr);
    name = _name;

    cardInfoUpdated();
}

void AbstractCardItem::setHovered(bool _hovered)
{
    if (isHovered == _hovered)
        return;
    
    if (_hovered)
        processHoverEvent();
    isHovered = _hovered;
    setZValue(_hovered ? 2000000004 : realZValue);
    setScale(_hovered && settingsCache->getScaleCards() ? 1.1 : 1);
    setTransformOriginPoint(_hovered ? CARD_WIDTH / 2 : 0, _hovered ? CARD_HEIGHT / 2 : 0);
    update();
}

void AbstractCardItem::setColor(const QString &_color)
{
    color = _color;
    cacheBgColor();
    update();
}

void AbstractCardItem::cacheBgColor()
{
    QChar colorChar;
    if (color.isEmpty())
    {
        if(info)
            colorChar = info->getColorChar();
    } else {
        colorChar = color.at(0);
    }
    
    switch(colorChar.toLower().toLatin1())
    {
        case 'b':
            bgColor = QColor(0, 0, 0);
            break;
        case 'u':
            bgColor = QColor(0, 140, 180);
            break;
        case 'w':
            bgColor = QColor(255, 250, 140);
            break;
        case 'r':
            bgColor = QColor(230, 0, 0);
            break;
        case 'g':
            bgColor = QColor(0, 160, 0);
            break;
        case 'm':
            bgColor = QColor(250, 190, 30);
            break;
        default:
            bgColor = QColor(230, 230, 230);
            break;
    }
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
    if ((event->modifiers() & Qt::ControlModifier)) {
        setSelected(!isSelected());
    }
    else if (!isSelected()) {
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

