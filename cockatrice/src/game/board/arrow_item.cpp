#define _USE_MATH_DEFINES
#include "arrow_item.h"

#include "../../settings/cache_settings.h"
#include "../cards/card_info.h"
#include "../player/player.h"
#include "../player/player_target.h"
#include "../zones/card_zone.h"
#include "card_item.h"
#include "color.h"
#include "pb/command_attach_card.pb.h"
#include "pb/command_create_arrow.pb.h"
#include "pb/command_delete_arrow.pb.h"

#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QtMath>

ArrowItem::ArrowItem(Player *_player, int _id, ArrowTarget *_startItem, ArrowTarget *_targetItem, const QColor &_color)
    : QGraphicsItem(), player(_player), id(_id), startItem(_startItem), targetItem(_targetItem), targetLocked(false),
      color(_color), fullColor(true)
{
    setZValue(2000000005);

    if (startItem)
        startItem->addArrowFrom(this);
    if (targetItem)
        targetItem->addArrowTo(this);

    if (startItem && targetItem)
        updatePath();
}

ArrowItem::~ArrowItem()
{
}

void ArrowItem::delArrow()
{
    if (startItem) {
        startItem->removeArrowFrom(this);
        startItem = 0;
    }

    if (targetItem) {
        targetItem->setBeingPointedAt(false);
        targetItem->removeArrowTo(this);
        targetItem = 0;
    }

    player->removeArrow(this);
    deleteLater();
}

void ArrowItem::updatePath()
{
    if (!targetItem)
        return;

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

    if (!startItem)
        return;

    QPointF startPoint =
        startItem->mapToScene(QPointF(startItem->boundingRect().width() / 2, startItem->boundingRect().height() / 2));
    QLineF line(startPoint, endPoint);
    qreal lineLength = line.length();

    prepareGeometryChange();
    if (lineLength < 30)
        path = QPainterPath();
    else {
        QPointF c(lineLength / 2, qTan(phi * M_PI / 180) * lineLength);

        QPainterPath centerLine;
        centerLine.moveTo(0, 0);
        centerLine.quadTo(c, QPointF(lineLength, 0));

        double percentage = 1 - headLength / lineLength;
        QPointF arrowBodyEndPoint = centerLine.pointAtPercent(percentage);
        QLineF testLine(arrowBodyEndPoint, centerLine.pointAtPercent(percentage + 0.001));
        qreal alpha = testLine.angle() - 90;
        QPointF endPoint1 =
            arrowBodyEndPoint + arrowWidth / 2 * QPointF(qCos(alpha * M_PI / 180), -qSin(alpha * M_PI / 180));
        QPointF endPoint2 =
            arrowBodyEndPoint + arrowWidth / 2 * QPointF(-qCos(alpha * M_PI / 180), qSin(alpha * M_PI / 180));
        QPointF point1 =
            endPoint1 + (headWidth - arrowWidth) / 2 * QPointF(qCos(alpha * M_PI / 180), -qSin(alpha * M_PI / 180));
        QPointF point2 =
            endPoint2 + (headWidth - arrowWidth) / 2 * QPointF(-qCos(alpha * M_PI / 180), qSin(alpha * M_PI / 180));

        path = QPainterPath(-arrowWidth / 2 * QPointF(qCos((phi - 90) * M_PI / 180), qSin((phi - 90) * M_PI / 180)));
        path.quadTo(c, endPoint1);
        path.lineTo(point1);
        path.lineTo(QPointF(lineLength, 0));
        path.lineTo(point2);
        path.lineTo(endPoint2);
        path.quadTo(c, arrowWidth / 2 * QPointF(qCos((phi - 90) * M_PI / 180), qSin((phi - 90) * M_PI / 180)));
        path.lineTo(-arrowWidth / 2 * QPointF(qCos((phi - 90) * M_PI / 180), qSin((phi - 90) * M_PI / 180)));
    }

    setPos(startPoint);
    setTransform(QTransform().rotate(-line.angle()));
}

void ArrowItem::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    QColor paintColor(color);
    if (fullColor)
        paintColor.setAlpha(200);
    else
        paintColor.setAlpha(150);
    painter->setBrush(paintColor);
    painter->drawPath(path);
}

void ArrowItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!player->getLocal()) {
        event->ignore();
        return;
    }

    QList<QGraphicsItem *> colliding = scene()->items(event->scenePos());
    for (QGraphicsItem *item : colliding) {
        if (qgraphicsitem_cast<CardItem *>(item)) {
            event->ignore();
            return;
        }
    }

    event->accept();
    if (event->button() == Qt::RightButton) {
        Command_DeleteArrow cmd;
        cmd.set_arrow_id(id);
        player->sendGameCommand(cmd);
    }
}

ArrowDragItem::ArrowDragItem(Player *_owner, ArrowTarget *_startItem, const QColor &_color)
    : ArrowItem(_owner, -1, _startItem, 0, _color)
{
}

void ArrowDragItem::addChildArrow(ArrowDragItem *childArrow)
{
    childArrows.append(childArrow);
}

void ArrowDragItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    // This ensures that if a mouse move event happens after a call to delArrow(),
    // the event will be discarded as it would create some stray pointers.
    if (targetLocked || !startItem)
        return;

    QPointF endPos = event->scenePos();

    QList<QGraphicsItem *> colliding = scene()->items(endPos);
    ArrowTarget *cursorItem = 0;
    qreal cursorItemZ = -1;
    for (int i = colliding.size() - 1; i >= 0; i--) {
        if (qgraphicsitem_cast<PlayerTarget *>(colliding.at(i)) || qgraphicsitem_cast<CardItem *>(colliding.at(i))) {
            if (colliding.at(i)->zValue() > cursorItemZ) {
                cursorItem = static_cast<ArrowTarget *>(colliding.at(i));
                cursorItemZ = cursorItem->zValue();
            }
        }
    }

    if ((cursorItem != targetItem) && targetItem) {
        targetItem->setBeingPointedAt(false);
        targetItem->removeArrowTo(this);
    }
    if (!cursorItem) {
        fullColor = false;
        targetItem = 0;
        updatePath(endPos);
    } else {
        if (cursorItem != targetItem) {
            fullColor = true;
            if (cursorItem != startItem) {
                cursorItem->setBeingPointedAt(true);
                cursorItem->addArrowTo(this);
            }
            targetItem = cursorItem;
        }
        updatePath();
    }
    update();

    for (ArrowDragItem *child : childArrows) {
        child->mouseMoveEvent(event);
    }
}

void ArrowDragItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (!startItem)
        return;

    if (targetItem && (targetItem != startItem)) {
        CardZone *startZone = static_cast<CardItem *>(startItem)->getZone();
        // For now, we can safely assume that the start item is always a card.
        // The target item can be a player as well.
        CardItem *startCard = qgraphicsitem_cast<CardItem *>(startItem);
        CardItem *targetCard = qgraphicsitem_cast<CardItem *>(targetItem);

        Command_CreateArrow cmd;
        cmd.mutable_arrow_color()->CopyFrom(convertQColorToColor(color));
        cmd.set_start_player_id(startZone->getPlayer()->getId());
        cmd.set_start_zone(startZone->getName().toStdString());
        cmd.set_start_card_id(startCard->getId());

        if (targetCard) {
            CardZone *targetZone = targetCard->getZone();
            cmd.set_target_player_id(targetZone->getPlayer()->getId());
            cmd.set_target_zone(targetZone->getName().toStdString());
            cmd.set_target_card_id(targetCard->getId());
        } else {
            PlayerTarget *targetPlayer = qgraphicsitem_cast<PlayerTarget *>(targetItem);
            cmd.set_target_player_id(targetPlayer->getOwner()->getId());
        }
        if (startZone->getName().compare("hand") == 0) {
            startCard->playCard(false);
            CardInfoPtr ci = startCard->getCard().getCardPtr();
            if (ci && ((!SettingsCache::instance().getPlayToStack() && ci->getTableRow() == 3) ||
                       (SettingsCache::instance().getPlayToStack() && ci->getTableRow() != 0 &&
                        startCard->getZone()->getName().toStdString() != "stack")))
                cmd.set_start_zone("stack");
            else
                cmd.set_start_zone(SettingsCache::instance().getPlayToStack() ? "stack" : "table");
        }
        player->sendGameCommand(cmd);
    }
    delArrow();

    for (ArrowDragItem *child : childArrows) {
        child->mouseReleaseEvent(event);
    }
}

ArrowAttachItem::ArrowAttachItem(ArrowTarget *_startItem)
    : ArrowItem(_startItem->getOwner(), -1, _startItem, 0, Qt::green)
{
}

void ArrowAttachItem::addChildArrow(ArrowAttachItem *childArrow)
{
    childArrows.append(childArrow);
}

void ArrowAttachItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (targetLocked || !startItem)
        return;

    QPointF endPos = event->scenePos();

    QList<QGraphicsItem *> colliding = scene()->items(endPos);
    ArrowTarget *cursorItem = 0;
    qreal cursorItemZ = -1;
    for (int i = colliding.size() - 1; i >= 0; i--) {
        if (qgraphicsitem_cast<CardItem *>(colliding.at(i))) {
            if (colliding.at(i)->zValue() > cursorItemZ) {
                cursorItem = static_cast<ArrowTarget *>(colliding.at(i));
                cursorItemZ = cursorItem->zValue();
            }
        }
    }

    if ((cursorItem != targetItem) && targetItem) {
        targetItem->setBeingPointedAt(false);
    }
    if (!cursorItem) {
        fullColor = false;
        targetItem = 0;
        updatePath(endPos);
    } else {
        fullColor = true;
        if (cursorItem != startItem) {
            cursorItem->setBeingPointedAt(true);
        }
        targetItem = cursorItem;
        updatePath();
    }
    update();

    for (ArrowAttachItem *child : childArrows) {
        child->mouseMoveEvent(event);
    }
}

void ArrowAttachItem::attachCards(CardItem *startCard, const CardItem *targetCard)
{
    // do nothing if target is already attached to another card or is not in play
    if (targetCard->getAttachedTo() || targetCard->getZone()->getName() != "table") {
        return;
    }

    CardZone *startZone = startCard->getZone();
    CardZone *targetZone = targetCard->getZone();

    // move card onto table first if attaching from some other zone
    if (startZone->getName() != "table") {
        player->playCardToTable(startCard, false);
    }

    Command_AttachCard cmd;
    cmd.set_start_zone("table");
    cmd.set_card_id(startCard->getId());
    cmd.set_target_player_id(targetZone->getPlayer()->getId());
    cmd.set_target_zone(targetZone->getName().toStdString());
    cmd.set_target_card_id(targetCard->getId());

    player->sendGameCommand(cmd);
}

void ArrowAttachItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (!startItem)
        return;

    // Attaching could move startItem under the current cursor position, causing all children to retarget to it right
    // before they are processed. Prevent that.
    for (ArrowAttachItem *child : childArrows) {
        child->setTargetLocked(true);
    }

    if (targetItem && (targetItem != startItem)) {
        auto startCard = qgraphicsitem_cast<CardItem *>(startItem);
        auto targetCard = qgraphicsitem_cast<CardItem *>(targetItem);
        if (startCard && targetCard) {
            attachCards(startCard, targetCard);
        }
    }

    delArrow();

    for (ArrowAttachItem *child : childArrows) {
        child->mouseReleaseEvent(event);
    }
}
