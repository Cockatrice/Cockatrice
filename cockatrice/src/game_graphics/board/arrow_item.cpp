#define _USE_MATH_DEFINES
#include "arrow_item.h"

#include "../../client/settings/cache_settings.h"
#include "../../game/player/player_actions.h"
#include "../../game/player/player_logic.h"
#include "../game_scene.h"
#include "../player/player_target.h"
#include "../z_values.h"
#include "../zones/card_zone.h"
#include "card_item.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QtMath>
#include <libcockatrice/card/card_info.h>
#include <libcockatrice/protocol/pb/command_attach_card.pb.h>
#include <libcockatrice/protocol/pb/command_create_arrow.pb.h>
#include <libcockatrice/protocol/pb/command_delete_arrow.pb.h>
#include <libcockatrice/settings/cards_display_settings.h>
#include <libcockatrice/settings/interface_settings.h>
#include <libcockatrice/utility/color.h>
#include <libcockatrice/utility/zone_names.h>

namespace
{
constexpr qreal kMinStrokeDurationMs = 200.0;
constexpr qreal kMaxStrokeDurationMs = 450.0;
constexpr qreal kMsPerPixel = 0.8;
constexpr qreal kGlowFadeDurationMs = 120.0;
constexpr qreal kSheenHalfWidth = 14.0;

/// @brief Ease-out cubic, for a natural "slow in / slow out" reveal.
qreal easeOutCubic(qreal t)
{
    const qreal inverse = 1.0 - t;
    return 1.0 - inverse * inverse * inverse;
}
} // namespace

ArrowItem::ArrowItem(QSharedPointer<const ArrowData> _data, ArrowTarget *_startItem, ArrowTarget *_targetItem)
    : data(std::move(_data)), startItem(_startItem), targetItem(_targetItem)
{
    setZValue(ZValues::ARROWS);

    auto doUpdate = [this]() {
        if (startItem && targetItem) {
            updatePath();
        }
    };

    if (startItem) {
        connect(startItem, &ArrowTarget::scenePositionChanged, this, doUpdate);
        connect(startItem, &QObject::destroyed, this, &ArrowItem::onTargetDestroyed);
    }
    if (targetItem) {
        connect(targetItem, &ArrowTarget::scenePositionChanged, this, doUpdate);
        connect(targetItem, &QObject::destroyed, this, &ArrowItem::onTargetDestroyed);
    }

    if (startItem && targetItem) {
        updatePath();
    }
}

ArrowItem::~ArrowItem()
{
    if (auto *scene = qobject_cast<GameScene *>(this->scene())) {
        scene->unregisterAnimationItem(this);
    }
}

void ArrowItem::onTargetDestroyed()
{
    emit requestDeletion(data->creatorId, data->id);
}

void ArrowItem::delArrow()
{
    if (targetItem) {
        targetItem->setBeingPointedAt(false);
    }
    deleteLater();
}

void ArrowItem::updatePath()
{
    if (!targetItem) {
        return;
    }

    QPointF endPoint = targetItem->mapToScene(
        QPointF(targetItem->boundingRect().width() / 2, targetItem->boundingRect().height() / 2));
    updatePath(endPoint);
}

void ArrowItem::updatePath(const QPointF &endPoint)
{
    const double arrowWidth = 15.0;
    const double headWidth = 40.0;
    const double headLength =
        headWidth / qPow(2, 0.5); // aka headWidth / sqrt (2) but this produces a compile error with MSVC++
    const double phi = 15;

    if (!startItem) {
        return;
    }

    QPointF startPoint =
        startItem->mapToScene(QPointF(startItem->boundingRect().width() / 2, startItem->boundingRect().height() / 2));
    QLineF line(startPoint, endPoint);
    qreal lineLength = line.length();

    prepareGeometryChange();
    if (lineLength < 30) {
        path = QPainterPath();
        bodyPath = QPainterPath();
        headPath = QPainterPath();
        centerLine = QPainterPath();
        headBaseFraction = 1.0;
    } else {
        QPointF c(lineLength / 2, qTan(phi * M_PI / 180) * lineLength);

        centerLine = QPainterPath();
        centerLine.moveTo(0, 0);
        centerLine.quadTo(c, QPointF(lineLength, 0));

        headBaseFraction = 1 - headLength / lineLength;
        QPointF arrowBodyEndPoint = centerLine.pointAtPercent(headBaseFraction);
        QLineF testLine(arrowBodyEndPoint, centerLine.pointAtPercent(headBaseFraction + 0.001));
        qreal alpha = testLine.angle() - 90;
        QPointF endPoint1 =
            arrowBodyEndPoint + arrowWidth / 2 * QPointF(qCos(alpha * M_PI / 180), -qSin(alpha * M_PI / 180));
        QPointF endPoint2 =
            arrowBodyEndPoint + arrowWidth / 2 * QPointF(-qCos(alpha * M_PI / 180), qSin(alpha * M_PI / 180));
        QPointF point1 =
            endPoint1 + (headWidth - arrowWidth) / 2 * QPointF(qCos(alpha * M_PI / 180), -qSin(alpha * M_PI / 180));
        QPointF point2 =
            endPoint2 + (headWidth - arrowWidth) / 2 * QPointF(-qCos(alpha * M_PI / 180), qSin(alpha * M_PI / 180));

        QPointF start1 = -arrowWidth / 2 * QPointF(qCos((phi - 90) * M_PI / 180), qSin((phi - 90) * M_PI / 180));
        QPointF start2 = arrowWidth / 2 * QPointF(qCos((phi - 90) * M_PI / 180), qSin((phi - 90) * M_PI / 180));

        path = QPainterPath(start1);
        path.quadTo(c, endPoint1);
        path.lineTo(point1);
        path.lineTo(QPointF(lineLength, 0));
        path.lineTo(point2);
        path.lineTo(endPoint2);
        path.quadTo(c, start2);
        path.lineTo(start1);

        bodyPath = QPainterPath(start1);
        bodyPath.quadTo(c, endPoint1);
        bodyPath.lineTo(endPoint2);
        bodyPath.quadTo(c, start2);
        bodyPath.lineTo(start1);

        headPath = QPainterPath(endPoint1);
        headPath.lineTo(point1);
        headPath.lineTo(QPointF(lineLength, 0));
        headPath.lineTo(point2);
        headPath.lineTo(endPoint2);
    }

    setPos(startPoint);
    setTransform(QTransform().rotate(-line.angle()));
}

void ArrowItem::startDrawAnimation()
{
    if (!SettingsCache::instance().cardsDisplay().getArrowDrawAnimation() || centerLine.isEmpty()) {
        return;
    }

    strokeDurationMs = qBound(kMinStrokeDurationMs, centerLine.length() * kMsPerPixel, kMaxStrokeDurationMs);
    glowFadeDurationMs = kGlowFadeDurationMs;
    animationClock.start();
    drawProgress = 0.0;
    glowAlpha = 1.0;
    update();
    if (auto *scene = qobject_cast<GameScene *>(this->scene())) {
        scene->registerAnimationItem(this);
    }
}

bool ArrowItem::animationEvent()
{
    const qint64 elapsed = animationClock.elapsed();
    if (elapsed >= strokeDurationMs + glowFadeDurationMs) {
        drawProgress = 1.0;
        glowAlpha = 0.0;
        update();
        return false;
    }

    if (elapsed < strokeDurationMs) {
        drawProgress = easeOutCubic(qBound<qreal>(0.0, elapsed / strokeDurationMs, 1.0));
        glowAlpha = 1.0;
    } else {
        drawProgress = 1.0;
        glowAlpha = 1.0 - (elapsed - strokeDurationMs) / glowFadeDurationMs;
    }
    update();
    return true;
}

void ArrowItem::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    QColor paintColor(data->color);
    if (fullColor) {
        paintColor.setAlpha(200);
    } else {
        paintColor.setAlpha(150);
    }

    painter->save();
    painter->setBrush(paintColor);

    if (drawProgress >= 1.0 || path.isEmpty()) {
        painter->drawPath(path);
    } else if (drawProgress < headBaseFraction) {
        // The reveal edge and the sheen share the same arc-length parameterization,
        // so the stroke stays exactly in sync with the trailing sheen.
        const qreal revealX = centerLine.pointAtPercent(drawProgress).x();
        QPainterPath clip;
        clip.addRect(QRectF(-glowExtent, path.boundingRect().top() - glowExtent, revealX + glowExtent,
                            path.boundingRect().height() + 2 * glowExtent));
        painter->setClipPath(clip);
        painter->drawPath(bodyPath);
    } else {
        // Once the reveal reaches the head base, pop the whole head in with a fade
        // instead of slicing the triangle into a growing stub.
        painter->drawPath(bodyPath);
        const qreal headFadeIn = (drawProgress - headBaseFraction) / (1.0 - headBaseFraction);
        painter->setOpacity(headFadeIn);
        painter->drawPath(headPath);
        painter->setOpacity(1.0);
    }

    if (glowAlpha > 0.0 && !centerLine.isEmpty()) {
        // Sweep a bright band across the arrow. Clipping to the
        // silhouette keeps it flat against the shaft so it reads as a light reflection.
        const qreal anticipation = qMin<qreal>(1.0, drawProgress / 0.08);
        const QPointF sweep = centerLine.pointAtPercent(qMin<qreal>(drawProgress, 1.0));
        QLinearGradient sheen(sweep.x() - kSheenHalfWidth, 0.0, sweep.x() + kSheenHalfWidth, 0.0);
        sheen.setColorAt(0.0, QColor(paintColor.red(), paintColor.green(), paintColor.blue(), 0));
        sheen.setColorAt(0.5, QColor(255, 255, 255, 200));
        sheen.setColorAt(1.0, QColor(paintColor.red(), paintColor.green(), paintColor.blue(), 0));
        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setClipPath(path);
        painter->setBrush(sheen);
        painter->setOpacity(glowAlpha * anticipation);
        painter->drawRect(QRectF(sweep.x() - kSheenHalfWidth - glowExtent, path.boundingRect().top() - glowExtent,
                                 (kSheenHalfWidth + glowExtent) * 2.0,
                                 path.boundingRect().height() + glowExtent * 2.0));
        painter->restore();
    }
    painter->restore();
}

void ArrowItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!data->isLocalCreator) {
        event->ignore();
        return;
    }

    for (auto *item : scene()->items(event->scenePos())) {
        if (qgraphicsitem_cast<CardItem *>(item)) {
            event->ignore();
            return;
        }
    }

    event->accept();
    if (event->button() == Qt::RightButton) {
        emit requestDeletion(data->creatorId, data->id);
    }
}

// ArrowDragItem

ArrowDragItem::ArrowDragItem(PlayerLogic *_owner, ArrowTarget *_startItem, const QColor &_color, int _deleteInPhase)
    : ArrowItem(QSharedPointer<ArrowData>::create(ArrowData{.creatorId = _owner->getPlayerInfo()->getId(),
                                                            .isLocalCreator = true,
                                                            .id = -1,
                                                            .color = _color}),
                _startItem,
                nullptr),
      player(_owner), deleteInPhase(_deleteInPhase)
{
}

void ArrowDragItem::addChildArrow(ArrowDragItem *child)
{
    childArrows.append(child);
}

void ArrowDragItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (targetLocked || !startItem) {
        return;
    }

    const QPointF endPos = event->scenePos();

    ArrowTarget *cursorItem = nullptr;
    qreal cursorItemZ = -1;
    for (auto *item : scene()->items(endPos)) {
        ArrowTarget *candidate = nullptr;
        if (auto *card = qgraphicsitem_cast<CardItem *>(item)) {
            candidate = card;
        } else if (auto *pt = qgraphicsitem_cast<PlayerTarget *>(item)) {
            candidate = pt;
        }

        if (candidate && candidate->zValue() > cursorItemZ) {
            cursorItem = candidate;
            cursorItemZ = candidate->zValue();
        }
    }

    if (cursorItem != targetItem) {
        if (targetItem) {
            disconnect(positionConnection);
            targetItem->setBeingPointedAt(false);
        }

        targetItem = cursorItem;
        fullColor = (cursorItem != nullptr);

        if (cursorItem && cursorItem != startItem) {
            cursorItem->setBeingPointedAt(true);
            positionConnection =
                connect(cursorItem, &ArrowTarget::scenePositionChanged, this, [this]() { updatePath(); });
        }
    }

    targetItem ? updatePath() : updatePath(endPos);
    update();

    for (auto *child : childArrows) {
        child->mouseMoveEvent(event);
    }
}

void ArrowDragItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (!startItem) {
        return;
    }

    if (targetItem && targetItem != startItem) {
        CardItem *startCard = qgraphicsitem_cast<CardItem *>(startItem);
        // For now, we can safely assume that the start item is always a card.
        // The target item can be a player as well.
        if (!startCard) {
            delArrow();
            return;
        }

        CardZoneLogic *startZone = startCard->getZone();

        Command_CreateArrow cmd;
        cmd.mutable_arrow_color()->CopyFrom(convertQColorToColor(data->color));
        cmd.set_start_player_id(startZone->getPlayer()->getPlayerInfo()->getId());
        cmd.set_start_zone(startZone->getName().toStdString());
        cmd.set_start_card_id(startCard->getId());

        if (auto *targetCard = qgraphicsitem_cast<CardItem *>(targetItem)) {
            CardZoneLogic *targetZone = targetCard->getZone();
            cmd.set_target_player_id(targetZone->getPlayer()->getPlayerInfo()->getId());
            cmd.set_target_zone(targetZone->getName().toStdString());
            cmd.set_target_card_id(targetCard->getId());
        } else if (auto *targetPlayer = qgraphicsitem_cast<PlayerTarget *>(targetItem)) {
            cmd.set_target_player_id(targetPlayer->getOwner()->getPlayerInfo()->getId());
        } else {
            delArrow();
            return;
        }

        // if the card is in hand then we will move the card to stack or table as part of drawing the arrow
        if (startZone->getName() == ZoneNames::HAND) {
            startCard->playCard(false);
            CardInfoPtr ci = startCard->getCard().getCardPtr();
            bool playToStack = SettingsCache::instance().userInterface().getPlayToStack();
            if (ci && ((!playToStack && ci->getUiAttributes().tableRow == 3) ||
                       (playToStack && ci->getUiAttributes().tableRow != 0 &&
                        startCard->getZone()->getName() != ZoneNames::STACK))) {
                cmd.set_start_zone(ZoneNames::STACK);
            } else {
                cmd.set_start_zone(playToStack ? ZoneNames::STACK : ZoneNames::TABLE);
            }
        }

        if (deleteInPhase != 0) {
            cmd.set_delete_in_phase(deleteInPhase);
        }

        player->getPlayerActions()->sendGameCommand(cmd);
    }

    delArrow();
    for (auto *child : childArrows) {
        child->mouseReleaseEvent(event);
    }
}

// ArrowAttachItem
ArrowAttachItem::ArrowAttachItem(ArrowTarget *_startItem)
    : ArrowItem(
          QSharedPointer<ArrowData>::create(ArrowData{.creatorId = _startItem->getOwner()->getPlayerInfo()->getId(),
                                                      .isLocalCreator = true,
                                                      .id = -1,
                                                      .color = Qt::green}),
          _startItem,
          nullptr),
      player(_startItem->getOwner())
{
}

void ArrowAttachItem::addChildArrow(ArrowAttachItem *child)
{
    childArrows.append(child);
}

void ArrowAttachItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (targetLocked || !startItem) {
        return;
    }

    const QPointF endPos = event->scenePos();

    ArrowTarget *cursorItem = nullptr;
    qreal cursorItemZ = -1;
    for (auto *item : scene()->items(endPos)) {
        if (auto *card = qgraphicsitem_cast<CardItem *>(item)) {
            if (card->zValue() > cursorItemZ) {
                cursorItem = card;
                cursorItemZ = card->zValue();
            }
        }
    }

    if (cursorItem != targetItem) {
        if (targetItem) {
            disconnect(positionConnection);
            targetItem->setBeingPointedAt(false);
        }

        targetItem = cursorItem;
        fullColor = (cursorItem != nullptr);

        if (cursorItem && cursorItem != startItem) {
            cursorItem->setBeingPointedAt(true);
            positionConnection =
                connect(cursorItem, &ArrowTarget::scenePositionChanged, this, [this]() { updatePath(); });
        }
    }

    targetItem ? updatePath() : updatePath(endPos);
    update();

    for (auto *child : childArrows) {
        child->mouseMoveEvent(event);
    }
}

void ArrowAttachItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (!startItem) {
        return;
    }

    // Attaching could move startItem under the current cursor position, causing all children to retarget to it right
    // before they are processed. Prevent that.
    for (auto *child : childArrows) {
        child->setTargetLocked(true);
    }

    if (targetItem && targetItem != startItem) {
        auto *startCard = qgraphicsitem_cast<CardItem *>(startItem);
        auto *targetCard = qgraphicsitem_cast<CardItem *>(targetItem);
        if (startCard && targetCard) {
            attachCards(startCard, targetCard);
        }
    }

    delArrow();
    for (auto *child : childArrows) {
        child->mouseReleaseEvent(event);
    }
}

void ArrowAttachItem::attachCards(CardItem *startCard, const CardItem *targetCard)
{
    if (targetCard->getAttachedTo() || targetCard->getZone()->getName() != ZoneNames::TABLE) {
        return;
    }

    // move card onto table first if attaching from some other zone
    if (startCard->getZone()->getName() != ZoneNames::TABLE) {
        player->getPlayerActions()->playCardToTable(startCard, false);
    }

    Command_AttachCard cmd;
    cmd.set_start_zone(ZoneNames::TABLE);
    cmd.set_card_id(startCard->getId());
    cmd.set_target_player_id(targetCard->getZone()->getPlayer()->getPlayerInfo()->getId());
    cmd.set_target_zone(targetCard->getZone()->getName().toStdString());
    cmd.set_target_card_id(targetCard->getId());
    player->getPlayerActions()->sendGameCommand(cmd);
}