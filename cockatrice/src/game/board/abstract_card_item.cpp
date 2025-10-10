#include "abstract_card_item.h"

#include "../../interface/card_picture_loader/card_picture_loader.h"
#include "../game_scene.h"

#include <QCursor>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <algorithm>
#include <libcockatrice/card/database/card_database.h>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/settings/cache_settings.h>

AbstractCardItem::AbstractCardItem(QGraphicsItem *parent, const CardRef &cardRef, Player *_owner, int _id)
    : ArrowTarget(_owner, parent), id(_id), cardRef(cardRef), tapped(false), facedown(false), tapAngle(0),
      bgColor(Qt::transparent), isHovered(false), realZValue(0)
{
    setCursor(Qt::OpenHandCursor);
    setFlag(ItemIsSelectable);
    setCacheMode(DeviceCoordinateCache);

    connect(&SettingsCache::instance(), &SettingsCache::displayCardNamesChanged, this, [this] { update(); });
    refreshCardInfo();

    connect(&SettingsCache::instance(), &SettingsCache::roundCardCornersChanged, this, [this](bool _roundCardCorners) {
        Q_UNUSED(_roundCardCorners);

        prepareGeometryChange();
        update();
    });
}

AbstractCardItem::~AbstractCardItem()
{
    emit deleteCardInfoPopup(cardRef.name);
}

QRectF AbstractCardItem::boundingRect() const
{
    return QRectF(0, 0, CARD_WIDTH, CARD_HEIGHT);
}

QPainterPath AbstractCardItem::shape() const
{
    QPainterPath shape;
    qreal cardCornerRadius = SettingsCache::instance().getRoundCardCorners() ? 0.05 * CARD_WIDTH : 0.0;
    shape.addRoundedRect(boundingRect(), cardCornerRadius, cardCornerRadius);
    return shape;
}

void AbstractCardItem::pixmapUpdated()
{
    update();
    emit sigPixmapUpdated();
}

void AbstractCardItem::refreshCardInfo()
{
    exactCard = CardDatabaseManager::query()->getCard(cardRef);

    if (!exactCard && !cardRef.name.isEmpty()) {
        auto info = CardInfo::newInstance(cardRef.name, "", true, {}, {}, {}, {}, false, false, -1, false);
        exactCard = ExactCard(info);
    }
    if (exactCard) {
        connect(exactCard.getCardPtr().data(), &CardInfo::pixmapUpdated, this, &AbstractCardItem::pixmapUpdated);
    }

    cacheBgColor();
    update();
}

/**
 * Convenience method to get the CardInfo of the exactCard
 * @return A const reference to the CardInfo, or an empty CardInfo if card was null
 */
const CardInfo &AbstractCardItem::getCardInfo() const
{
    return exactCard.getInfo();
}

void AbstractCardItem::setRealZValue(qreal _zValue)
{
    realZValue = _zValue;
    setZValue(_zValue);
}

QSizeF AbstractCardItem::getTranslatedSize(QPainter *painter) const
{
    return QSizeF(painter->combinedTransform().map(QLineF(0, 0, boundingRect().width(), 0)).length(),
                  painter->combinedTransform().map(QLineF(0, 0, 0, boundingRect().height())).length());
}

void AbstractCardItem::transformPainter(QPainter *painter, const QSizeF &translatedSize, int angle)
{
    const int MAX_FONT_SIZE = SettingsCache::instance().getMaxFontSize();
    const int fontSize = std::max(9, MAX_FONT_SIZE);

    QRectF totalBoundingRect = painter->combinedTransform().mapRect(boundingRect());

    int scale = resetPainterTransform(painter);

    painter->translate(totalBoundingRect.width() / 2, totalBoundingRect.height() / 2);
    painter->rotate(angle);
    painter->translate(-translatedSize.width() / 2, -translatedSize.height() / 2);

    QFont f;
    f.setPixelSize(fontSize * scale);

    painter->setFont(f);
}

void AbstractCardItem::paintPicture(QPainter *painter, const QSizeF &translatedSize, int angle)
{
    qreal scaleFactor = translatedSize.width() / boundingRect().width();
    QPixmap translatedPixmap;
    bool paintImage = true;

    if (facedown || cardRef.name.isEmpty()) {
        // never reveal card color, always paint the card back
        CardPictureLoader::getCardBackPixmap(translatedPixmap, translatedSize.toSize());
    } else {
        // don't even spend time trying to load the picture if our size is too small
        if (translatedSize.width() > 10) {
            CardPictureLoader::getPixmap(translatedPixmap, exactCard, translatedSize.toSize());
            if (translatedPixmap.isNull())
                paintImage = false;
        } else {
            paintImage = false;
        }
    }

    painter->save();

    if (paintImage) {
        painter->save();
        painter->setClipPath(shape());
        painter->drawPixmap(boundingRect(), translatedPixmap, QRectF({0, 0}, translatedPixmap.size()));
        painter->restore();
    } else {
        painter->setBrush(bgColor);
        painter->drawPath(shape());
    }

    if (translatedPixmap.isNull() || SettingsCache::instance().getDisplayCardNames() || facedown) {
        painter->save();
        transformPainter(painter, translatedSize, angle);
        painter->setPen(Qt::white);
        painter->setBackground(Qt::black);
        painter->setBackgroundMode(Qt::OpaqueMode);
        QString nameStr;
        if (facedown)
            nameStr = "# " + QString::number(id);
        else {
            QString prefix = "";
            if (SettingsCache::instance().debug().getShowCardId()) {
                prefix = "#" + QString::number(id) + " ";
            }
            nameStr = prefix + cardRef.name;
        }
        painter->drawText(QRectF(3 * scaleFactor, 3 * scaleFactor, translatedSize.width() - 6 * scaleFactor,
                                 translatedSize.height() - 6 * scaleFactor),
                          Qt::AlignTop | Qt::AlignLeft | Qt::TextWrapAnywhere, nameStr);
        painter->restore();
    }

    painter->restore();
}

void AbstractCardItem::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    painter->save();

    QSizeF translatedSize = getTranslatedSize(painter);
    paintPicture(painter, translatedSize, tapAngle);

    painter->setRenderHint(QPainter::Antialiasing, false);

    if (isSelected() || isHovered) {
        QPen pen;
        if (isHovered)
            pen.setColor(Qt::yellow);
        if (isSelected())
            pen.setColor(Qt::red);
        pen.setWidth(0); // Cosmetic pen
        painter->setPen(pen);
        painter->drawPath(shape());
    }

    painter->restore();
}

void AbstractCardItem::setCardRef(const CardRef &_cardRef)
{
    if (cardRef == _cardRef) {
        return;
    }

    emit deleteCardInfoPopup(cardRef.name);
    if (exactCard) {
        disconnect(exactCard.getCardPtr().data(), nullptr, this, nullptr);
    }
    cardRef = _cardRef;

    refreshCardInfo();
}

void AbstractCardItem::setHovered(bool _hovered)
{
    if (isHovered == _hovered)
        return;

    if (_hovered)
        processHoverEvent();
    isHovered = _hovered;
    setZValue(_hovered ? 2000000004 : realZValue);
    setScale(_hovered && SettingsCache::instance().getScaleCards() ? 1.1 : 1);
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
    if (color.isEmpty()) {
        colorChar = exactCard.getInfo().getColorChar();
    } else {
        colorChar = color.at(0);
    }

    switch (colorChar.toLower().toLatin1()) {
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
    if (SettingsCache::instance().getTapAnimation() && canAnimate)
        static_cast<GameScene *>(scene())->registerAnimationItem(this);
    else {
        tapAngle = tapped ? 90 : 0;
        setTransform(QTransform()
                         .translate((float)CARD_WIDTH / 2, (float)CARD_HEIGHT / 2)
                         .rotate(tapAngle)
                         .translate((float)-CARD_WIDTH / 2, (float)-CARD_HEIGHT / 2));
        update();
    }
}

void AbstractCardItem::setFaceDown(bool _facedown)
{
    facedown = _facedown;
    update();
}

void AbstractCardItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if ((event->modifiers() & Qt::AltModifier) && event->button() == Qt::LeftButton) {
        emit cardShiftClicked(cardRef.name);
    } else if ((event->modifiers() & Qt::ControlModifier)) {
        setSelected(!isSelected());
    } else if (!isSelected()) {
        scene()->clearSelection();
        setSelected(true);
    }
    if (event->button() == Qt::LeftButton)
        setCursor(Qt::ClosedHandCursor);
    else if (event->button() == Qt::MiddleButton)
        emit showCardInfoPopup(event->screenPos(), cardRef);
    event->accept();
}

void AbstractCardItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton)
        emit deleteCardInfoPopup(cardRef.name);

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
        return ArrowTarget::itemChange(change, value);
}
