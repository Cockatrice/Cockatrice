#include "abstract_card_item.h"

#include "../../client/settings/cache_settings.h"
#include "../../interface/card_localization.h"
#include "../../interface/card_picture_loader/card_picture_loader.h"
#include "../card_dimensions.h"
#include "../game_scene.h"
#include "../z_values.h"
#include "abstract_graphics_item.h"
#include "libcockatrice/card/card_info.h"
#include "libcockatrice/card/database/card_database_querier.h"
#include "libcockatrice/card/printing/exact_card.h"
#include "libcockatrice/utility/card_ref.h"

#include <QApplication>
#include <QBrush>
#include <QChar>
#include <QCursor>
#include <QFlags>
#include <QFont>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QHash>
#include <QLineF>
#include <QList>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPixmap>
#include <QPointF>
#include <QPropertyAnimation>
#include <QRectF>
#include <QTimer>
#include <QTransform>
#include <QVariant>
#include <QtGlobal>
#include <algorithm>
#include <libcockatrice/settings/appearance_settings.h>
#include <libcockatrice/settings/cards_display_settings.h>
#include <libcockatrice/settings/debug_settings.h>
#include <qnamespace.h>

class QSizeF;

// Width the hovered card smooth-zooms to: exactly the hover panel picture size, so
// all four corners coincide when the panel swaps in over it.
static constexpr qreal HOVER_ZOOM_WIDTH = 270.0;
static constexpr int HOVER_ZOOM_IN_MS = 220;
static constexpr int HOVER_ZOOM_OUT_MS = 140;
static constexpr int FADE_DURATION_MS = 140;

AbstractCardItem::AbstractCardItem(QGraphicsItem *parent, const CardRef &cardRef, PlayerLogic *_owner, int _id)
    : ArrowTarget(_owner, parent), state(new CardState(this, nullptr, cardRef, _id)), tapAngle(0),
      bgColor(Qt::transparent), isHovered(false), realZValue(0)
{
    setCursor(Qt::OpenHandCursor);
    setFlag(ItemIsSelectable);
    setCacheMode(DeviceCoordinateCache);

    connect(state, &CardState::cardPixmapUpdated, this, &AbstractCardItem::pixmapUpdated);
    connect(state, &CardState::cardInfoChanged, this, &AbstractCardItem::onCardInfoChanged);
    connect(state, &CardState::cardRefChanged, this, &AbstractCardItem::onCardRefChanged);
    connect(state, &CardState::tappedChanged, this, &AbstractCardItem::onTappedChanged);
    connect(state, &CardState::facedownChanged, this, [this](bool faceDown) {
        update();
        emit faceDownChanged(faceDown);
    });
    connect(state, &CardState::colorChanged, this, [this] {
        cacheBgColor();
        update();
    });

    hoverZoom = new QPropertyAnimation(this, "cardScale", this);
    hoverZoom->setEasingCurve(QEasingCurve::InOutCubic);
    // Settling the unhover waits for the shrink-out to finish: the stacking override,
    // the clip escape and the ancestor lift all stay put while the card is still large,
    // so siblings and the clip container cannot cut through the shrinking card.
    connect(hoverZoom, &QPropertyAnimation::finished, this, [this] {
        if (!isHovered) {
            setZValue(realZValue);
            // Final repaint at the settled footprint: with no scale delta the animation
            // emits no per-frame updates, and zones would keep a stale clip hole behind.
            if (scene()) {
                scene()->update(sceneBoundingRect());
            }
            emit hoverShrinkFinished(this);
        }
    });

    opacityAnim = new QPropertyAnimation(this, "cardOpacity", this);
    opacityAnim->setDuration(FADE_DURATION_MS);
    opacityAnim->setEasingCurve(QEasingCurve::OutCubic);

    connect(&SettingsCache::instance().cardsDisplay(), &CardsDisplaySettings::displayCardNamesChanged, this,
            [this] { update(); });
    connect(&SettingsCache::instance().cardsDisplay(), &CardsDisplaySettings::cardLangChanged, this,
            [this] { update(); });
    refreshCardInfo();

    connect(&SettingsCache::instance().cardsDisplay(), &CardsDisplaySettings::roundCardCornersChanged, this,
            [this](bool _roundCardCorners) {
                Q_UNUSED(_roundCardCorners);

                prepareGeometryChange();
                update();
            });
}

AbstractCardItem::~AbstractCardItem()
{
    emit deleteCardInfoPopup(state->getCardRef().name);
}

QRectF AbstractCardItem::boundingRect() const
{
    return QRectF(0, 0, CardDimensions::WIDTH_F, CardDimensions::HEIGHT_F);
}

QPainterPath AbstractCardItem::shape() const
{
    QPainterPath shape;
    qreal cardCornerRadius =
        SettingsCache::instance().cardsDisplay().getRoundCardCorners() ? 0.05 * CardDimensions::WIDTH_F : 0.0;
    shape.addRoundedRect(boundingRect(), cardCornerRadius, cardCornerRadius);
    return shape;
}

void AbstractCardItem::pixmapUpdated()
{
    update();
    emit sigPixmapUpdated();
}

void AbstractCardItem::onCardInfoChanged()
{
    cacheBgColor();
    update();
}

void AbstractCardItem::onCardRefChanged(const CardRef &oldCardRef, const CardRef & /*newCardRef*/)
{
    emit deleteCardInfoPopup(oldCardRef.name);
}

void AbstractCardItem::refreshCardInfo()
{
    state->refreshCardInfo();
}

/**
 * Convenience method to get the CardInfo of the exactCard
 * @return A const reference to the CardInfo, or an empty CardInfo if card was null
 */
const CardInfo &AbstractCardItem::getCardInfo() const
{
    return state->getCardInfo();
}

void AbstractCardItem::setRealZValue(qreal _zValue)
{
    realZValue = _zValue;
    // During hover, zValue is overridden to HOVERED_CARD. Layout operations
    // like reorganizeCards() call setRealZValue() on all cards including the
    // hovered one — skip setZValue() here to avoid clobbering the override.
    if (!isHovered) {
        setZValue(_zValue);
    }
}

QSizeF AbstractCardItem::getTranslatedSize(QPainter *painter) const
{
    return QSizeF(painter->combinedTransform().map(QLineF(0, 0, boundingRect().width(), 0)).length(),
                  painter->combinedTransform().map(QLineF(0, 0, 0, boundingRect().height())).length());
}

void AbstractCardItem::transformPainter(QPainter *painter, const QSizeF &translatedSize, int angle)
{
    const int MAX_FONT_SIZE = SettingsCache::instance().appearance().getMaxFontSize();
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

    if (state->getFaceDown() || state->getCardRef().name.isEmpty()) {
        // never reveal card color, always paint the card back
        CardPictureLoader::getCardBackPixmap(translatedPixmap, translatedSize.toSize());
    } else {
        // don't even spend time trying to load the picture if our size is too small
        if (translatedSize.width() > 10) {
            CardPictureLoader::getPixmap(translatedPixmap, state->getCard(), translatedSize.toSize());
            if (translatedPixmap.isNull()) {
                paintImage = false;
            }
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

    if (translatedPixmap.isNull() || SettingsCache::instance().cardsDisplay().getDisplayCardNames() ||
        state->getFaceDown()) {
        painter->save();
        transformPainter(painter, translatedSize, angle);
        painter->setPen(Qt::white);
        painter->setBackground(Qt::black);
        painter->setBackgroundMode(Qt::OpaqueMode);
        QString nameStr;
        if (state->getFaceDown()) {
            nameStr = "# " + QString::number(state->getId());
        } else {
            QString prefix = "";
            if (SettingsCache::instance().debug().getShowCardId()) {
                prefix = "#" + QString::number(state->getId()) + " ";
            }
            nameStr = prefix + CardLocalization::displayName(getCardInfo());
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
        if (isHovered) {
            pen.setColor(Qt::yellow);
        }
        if (isSelected()) {
            pen.setColor(Qt::red);
        }
        pen.setWidth(0); // Cosmetic pen
        painter->setPen(pen);
        painter->drawPath(shape());
    }

    painter->restore();
}

void AbstractCardItem::setHovered(bool _hovered)
{
    if (isHovered == _hovered) {
        return;
    }

    if (_hovered) {
        processHoverEvent();
    } else {
        // Mark the hovered card's current scene footprint dirty so overlapped
        // sibling zones (e.g. StackZone) repaint after the card moves away.
        if (scene()) {
            const QRectF footprint = sceneBoundingRect();
            scene()->update(footprint);
            // The hole disappears the moment the card is no longer hovered, even though the
            // card has not changed size yet: tell the zones so they repaint it shut.
            emit hoverFootprintChanged(footprint, footprint);
        }
    }

    isHovered = _hovered;
    // The stacking override is one-directional: it applies the moment the hover starts
    // and only drops once the shrink-out finishes (see the hoverZoom finished handler).
    // Restoring it here would let siblings paint over the still-enlarged card.
    if (_hovered) {
        setZValue(ZValues::HOVERED_CARD);
    }
    // Scale about the center in both directions: at scale 1 the origin is moot,
    // and keeping it centered avoids drift while shrinking back down.
    setTransformOriginPoint(CardDimensions::WIDTH_HALF_F, CardDimensions::HEIGHT_HALF_F);

    // Smooth-zoom up to the hover picture size (instead of the old 1.1 snap) so the
    // hover panel can then swap in over the zoomed card with matching corners.
    // The target is constant ON-SCREEN pixels: divide out the board view zoom so
    // fullscreen (or zoomed) boards don't overshoot while the fixed-size panel matches.
    qreal target = 1.0;
    if (_hovered && SettingsCache::instance().cardsDisplay().getScaleCards()) {
        const qreal w = boundingRect().width();
        qreal viewScale = 1.0;
        if (scene() && !scene()->views().isEmpty()) {
            if (const auto *view = scene()->views().first()) {
                viewScale = qAbs(view->transform().m11());
            }
        }
        if (w > 0.0 && viewScale > 0.0) {
            target = qBound(1.0, HOVER_ZOOM_WIDTH / (w * viewScale), 6.0);
        }
    }
    hoverZoom->stop();
    hoverZoom->setStartValue(scale());
    hoverZoom->setEndValue(target);
    hoverZoom->setDuration(_hovered ? HOVER_ZOOM_IN_MS : HOVER_ZOOM_OUT_MS);
    hoverZoom->start();
    update();
}

void AbstractCardItem::setColor(const QString &_color)
{
    state->setColor(_color);
}

qreal AbstractCardItem::getCardScale() const
{
    return scale();
}

void AbstractCardItem::setCardScale(qreal _scale)
{
    if (qFuzzyCompare(_scale, scale())) {
        return;
    }
    // The scale animates every frame: keep the scene index in step or hover
    // picking tests a stale rect and the hover oscillates on/off by itself.
    // Repaint the union of the old and new footprints, not just the card: zones
    // dodge the zoomed card via a clip hole computed at paint time, and without
    // a repaint they keep a stale hole — covering the card mid-animation and
    // leaving a black rect behind once the hover ends.
    const QRectF oldFootprint = sceneBoundingRect();
    prepareGeometryChange();
    setScale(_scale);
    const QRectF newFootprint = sceneBoundingRect();
    if (scene()) {
        scene()->update(oldFootprint.united(newFootprint));
        // Marking the scene region dirty is not enough for cached items: a zone that cuts a
        // hover hole out of itself keeps the hole from its last repaint, which then shows as
        // an uncovered black rectangle under the zoom.
        emit hoverFootprintChanged(oldFootprint, newFootprint);
    } else {
        update();
    }
}

qreal AbstractCardItem::getCardOpacity() const
{
    return opacity();
}

void AbstractCardItem::setCardOpacity(qreal _opacity)
{
    setOpacity(_opacity);
    update();
}

void AbstractCardItem::fadeCardOut()
{
    opacityAnim->stop();
    opacityAnim->setStartValue(opacity());
    opacityAnim->setEndValue(0.0);
    opacityAnim->start();
}

void AbstractCardItem::fadeCardIn()
{
    opacityAnim->stop();
    opacityAnim->setStartValue(opacity());
    opacityAnim->setEndValue(1.0);
    opacityAnim->start();
}

void AbstractCardItem::cacheBgColor()
{
    QChar colorChar;
    const QString cardColor = state->getColor();
    if (cardColor.isEmpty()) {
        colorChar = state->getCardInfo().getColorChar();
    } else {
        colorChar = cardColor.at(0);
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

void AbstractCardItem::onTappedChanged(bool newTapped, bool canAnimate)
{
    emit tappedChanged(newTapped);
    if (SettingsCache::instance().cardsDisplay().getTapAnimation() && canAnimate) {
        static_cast<GameScene *>(scene())->registerAnimationItem(this);
    } else {
        tapAngle = newTapped ? 90 : 0;
        setTransform(QTransform()
                         .translate(CardDimensions::WIDTH_HALF_F, CardDimensions::HEIGHT_HALF_F)
                         .rotate(tapAngle)
                         .translate(-CardDimensions::WIDTH_HALF_F, -CardDimensions::HEIGHT_HALF_F));
        update();
    }
}

bool AbstractCardItem::animationEvent()
{
    return false;
}

void AbstractCardItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    cancelPendingSelectionCollapse();
    if (scene() != nullptr) {
        // A new press starts a new gesture: no card may collapse the selection out from under it afterwards.
        for (QGraphicsItem *selectedItem : scene()->selectedItems()) {
            auto *selectedCard = qgraphicsitem_cast<AbstractCardItem *>(selectedItem);
            if (selectedCard != nullptr && selectedCard != this) {
                selectedCard->cancelPendingSelectionCollapse();
            }
        }
    }
    if ((event->modifiers() & Qt::AltModifier) && event->button() == Qt::LeftButton) {
        emit cardShiftClicked(state->getCardRef().name);
    } else if ((event->modifiers() & Qt::ControlModifier)) {
        setSelected(!isSelected());
    } else if (!isSelected()) {
        scene()->clearSelection();
        setSelected(true);
    } else if (event->button() == Qt::LeftButton && scene()->selectedItems().count() > 1) {
        // Part of a multi-selection; collapse only if this turns out to be a click rather than a drag,
        // so that dragging the whole selection keeps it intact.
        pendingSelectionCollapse = true;
    }
    if (event->button() == Qt::LeftButton) {
        setCursor(Qt::ClosedHandCursor);
    } else if (event->button() == Qt::MiddleButton) {
        emit showCardInfoPopup(event->screenPos(), state->getCardRef());
    }
    event->accept();
}

void AbstractCardItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (pendingSelectionCollapse) {
        pendingSelectionCollapse = false;
        bool dragged = (event->screenPos() - event->buttonDownScreenPos(event->button())).manhattanLength() >=
                       2 * QApplication::startDragDistance();
        if (!dragged) {
            if (doubleClickActsOnSelection()) {
                // A double-click may still arrive and must act on the whole multi-selection, so collapse only
                // once the double-click window has passed without one. The generation guard (and the receiver
                // context) drops the collapse if the item is gone, deselected, or a double-click cancelled it;
                // the count guard drops it if the selection changed meanwhile (rubber band, ctrl-toggle).
                const int generation = ++selectionCollapseGeneration;
                const int armedSelectionCount = scene()->selectedItems().count();
                QTimer::singleShot(QApplication::doubleClickInterval(), this, [this, generation, armedSelectionCount] {
                    if (generation == selectionCollapseGeneration && scene() != nullptr && isSelected() &&
                        scene()->selectedItems().count() == armedSelectionCount && armedSelectionCount > 1) {
                        scene()->clearSelection();
                        setSelected(true);
                    }
                });
            } else {
                scene()->clearSelection();
                setSelected(true);
            }
        }
    }

    if (event->button() == Qt::MiddleButton) {
        emit deleteCardInfoPopup(state->getCardRef().name);
    }

    // This function ensures the parent function doesn't mess around with our selection.
    event->accept();
}

void AbstractCardItem::cancelPendingSelectionCollapse()
{
    pendingSelectionCollapse = false;
    ++selectionCollapseGeneration;
}

bool AbstractCardItem::doubleClickActsOnSelection() const
{
    return false;
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
    } else {
        return ArrowTarget::itemChange(change, value);
    }
}
