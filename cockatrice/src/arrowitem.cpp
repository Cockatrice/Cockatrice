#define _USE_MATH_DEFINES
#include <cmath>

#include "arrowitem.h"
#include "playertarget.h"
#include "carditem.h"
#include "carddatabase.h"
#include "cardzone.h"
#include "player.h"
#include "settingscache.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QDebug>

#include "color.h"
#include "pb/command_attach_card.pb.h"
#include "pb/command_create_arrow.pb.h"
#include "pb/command_delete_arrow.pb.h"

ArrowItem::ArrowItem(Player *_player, int _id, ArrowTarget *_startItem, ArrowTarget *_targetItem, const QColor &_color)
    : QGraphicsItem(), player(_player), id(_id), startItem(_startItem), targetItem(_targetItem), color(_color), fullColor(true)
{
    qDebug() << "ArrowItem constructor: startItem=" << static_cast<QGraphicsItem *>(startItem);
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
    qDebug() << "ArrowItem destructor";
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

    QPointF endPoint = targetItem->mapToScene(QPointF(targetItem->boundingRect().width() / 2, targetItem->boundingRect().height() / 2));
    updatePath(endPoint);
}

void ArrowItem::updatePath(const QPointF &endPoint)
{
    const double arrowWidth = 15.0;
    const double headWidth = 40.0;
    const double headLength = headWidth / pow(2, 0.5); // aka headWidth / sqrt (2) but this produces a compile error with MSVC++
    const double phi = 15;

    if (!startItem)
        return;

    QPointF startPoint = startItem->mapToScene(QPointF(startItem->boundingRect().width() / 2, startItem->boundingRect().height() / 2));
    QLineF line(startPoint, endPoint);
    qreal lineLength = line.length();

    prepareGeometryChange();
    if (lineLength < 30)
        path = QPainterPath();
    else {
        QPointF c(lineLength / 2, tan(phi * M_PI / 180) * lineLength);

        QPainterPath centerLine;
        centerLine.moveTo(0, 0);
        centerLine.quadTo(c, QPointF(lineLength, 0));

        double percentage = 1 - headLength / lineLength;
        QPointF arrowBodyEndPoint = centerLine.pointAtPercent(percentage);
        QLineF testLine(arrowBodyEndPoint, centerLine.pointAtPercent(percentage + 0.001));
        qreal alpha = testLine.angle() - 90;
        QPointF endPoint1 = arrowBodyEndPoint + arrowWidth / 2 * QPointF(cos(alpha * M_PI / 180), -sin(alpha * M_PI / 180));
        QPointF endPoint2 = arrowBodyEndPoint + arrowWidth / 2 * QPointF(-cos(alpha * M_PI / 180), sin(alpha * M_PI / 180));
        QPointF point1 = endPoint1 + (headWidth - arrowWidth) / 2 * QPointF(cos(alpha * M_PI / 180), -sin(alpha * M_PI / 180));
        QPointF point2 = endPoint2 + (headWidth - arrowWidth) / 2 * QPointF(-cos(alpha * M_PI / 180), sin(alpha * M_PI / 180));

        path = QPainterPath(-arrowWidth / 2 * QPointF(cos((phi - 90) * M_PI / 180), sin((phi - 90) * M_PI / 180)));
        path.quadTo(c, endPoint1);
        path.lineTo(point1);
        path.lineTo(QPointF(lineLength, 0));
        path.lineTo(point2);
        path.lineTo(endPoint2);
        path.quadTo(c, arrowWidth / 2 * QPointF(cos((phi - 90) * M_PI / 180), sin((phi - 90) * M_PI / 180)));
        path.lineTo(-arrowWidth / 2 * QPointF(cos((phi - 90) * M_PI / 180), sin((phi - 90) * M_PI / 180)));
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
    for (int i = 0; i < colliding.size(); ++i)
        if (qgraphicsitem_cast<CardItem *>(colliding[i])) {
            event->ignore();
            return;
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
    if (!startItem)
        return;

    QPointF endPos = event->scenePos();

    QList<QGraphicsItem *> colliding = scene()->items(endPos);
    ArrowTarget *cursorItem = 0;
    qreal cursorItemZ = -1;
    for (int i = colliding.size() - 1; i >= 0; i--)
        if (qgraphicsitem_cast<PlayerTarget *>(colliding.at(i)) || qgraphicsitem_cast<CardItem *>(colliding.at(i)))
            if (colliding.at(i)->zValue() > cursorItemZ) {
                cursorItem = static_cast<ArrowTarget *>(colliding.at(i));
                cursorItemZ = cursorItem->zValue();
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

            for (int i = 0; i < childArrows.size(); ++i)
                childArrows[i]->mouseMoveEvent(event);
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
        if (startZone->getName().compare("hand") == 0)  {
            startCard->playCard(false);
            CardInfo *ci = startCard->getInfo();
            if (ci && (((!settingsCache->getPlayToStack() && ci->getTableRow() == 3) ||
                ((settingsCache->getPlayToStack() && ci->getTableRow() != 0) &&
                startCard->getZone()->getName().toStdString() != "stack"))))
                cmd.set_start_zone("stack");
            else
                cmd.set_start_zone(settingsCache->getPlayToStack() ? "stack" :"table");
        }
        player->sendGameCommand(cmd);
    }
    delArrow();

    for (int i = 0; i < childArrows.size(); ++i)
        childArrows[i]->mouseReleaseEvent(event);
}

ArrowAttachItem::ArrowAttachItem(ArrowTarget *_startItem)
    : ArrowItem(_startItem->getOwner(), -1, _startItem, 0, Qt::green)
{
}

void ArrowAttachItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!startItem)
        return;

    QPointF endPos = event->scenePos();

    QList<QGraphicsItem *> colliding = scene()->items(endPos);
    ArrowTarget *cursorItem = 0;
    qreal cursorItemZ = -1;
    for (int i = colliding.size() - 1; i >= 0; i--)
        if (qgraphicsitem_cast<CardItem *>(colliding.at(i)))
            if (colliding.at(i)->zValue() > cursorItemZ) {
                cursorItem = static_cast<ArrowTarget *>(colliding.at(i));
                cursorItemZ = cursorItem->zValue();
            }

            if ((cursorItem != targetItem) && targetItem)
                targetItem->setBeingPointedAt(false);
            if (!cursorItem) {
                fullColor = false;
                targetItem = 0;
                updatePath(endPos);
            } else {
                fullColor = true;
                if (cursorItem != startItem)
                    cursorItem->setBeingPointedAt(true);
                targetItem = cursorItem;
                updatePath();
            }
            update();
}

void ArrowAttachItem::mouseReleaseEvent(QGraphicsSceneMouseEvent * /*event*/)
{
    if (!startItem)
        return;

    if (targetItem && (targetItem != startItem)) {
        CardItem *startCard = qgraphicsitem_cast<CardItem *>(startItem);
        CardZone *startZone = startCard->getZone();
        CardItem *targetCard = qgraphicsitem_cast<CardItem *>(targetItem);
        CardZone *targetZone = targetCard->getZone();

        Command_AttachCard cmd;
        cmd.set_start_zone(startZone->getName().toStdString());
        cmd.set_card_id(startCard->getId());
        cmd.set_target_player_id(targetZone->getPlayer()->getId());
        cmd.set_target_zone(targetZone->getName().toStdString());
        cmd.set_target_card_id(targetCard->getId());

        player->sendGameCommand(cmd);
    }

    delArrow();
}
