#include "pile_zone.h"

#include "../board/card_drag_item.h"
#include "../board/card_item.h"
#include "../player/player.h"
#include "logic/pile_zone_logic.h"
#include "view_zone.h"

#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <libcockatrice/protocol/pb/command_move_card.pb.h>

PileZone::PileZone(PileZoneLogic *_logic, QGraphicsItem *parent) : CardZone(_logic, parent)
{
    setCacheMode(DeviceCoordinateCache); // Do not move this line to the parent constructor!
    setAcceptHoverEvents(true);
    setCursor(Qt::OpenHandCursor);

    setTransform(QTransform()
                     .translate((float)CARD_WIDTH / 2, (float)CARD_HEIGHT / 2)
                     .rotate(90)
                     .translate((float)-CARD_WIDTH / 2, (float)-CARD_HEIGHT / 2));

    connect(&SettingsCache::instance(), &SettingsCache::roundCardCornersChanged, this, [this](bool _roundCardCorners) {
        Q_UNUSED(_roundCardCorners);

        prepareGeometryChange();
        update();
    });
}

QRectF PileZone::boundingRect() const
{
    return QRectF(0, 0, CARD_WIDTH, CARD_HEIGHT);
}

QPainterPath PileZone::shape() const
{
    QPainterPath shape;
    qreal cardCornerRadius = SettingsCache::instance().getRoundCardCorners() ? 0.05 * CARD_WIDTH : 0.0;
    shape.addRoundedRect(boundingRect(), cardCornerRadius, cardCornerRadius);
    return shape;
}

void PileZone::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    painter->drawPath(shape());

    if (!getLogic()->getCards().isEmpty())
        getLogic()->getCards().at(0)->paintPicture(painter, getLogic()->getCards().at(0)->getTranslatedSize(painter),
                                                   90);

    painter->translate((float)CARD_WIDTH / 2, (float)CARD_HEIGHT / 2);
    painter->rotate(-90);
    painter->translate((float)-CARD_WIDTH / 2, (float)-CARD_HEIGHT / 2);
    paintNumberEllipse(getLogic()->getCards().size(), 28, Qt::white, -1, -1, painter);
}

void PileZone::handleDropEvent(const QList<CardDragItem *> &dragItems, CardZoneLogic *startZone, const QPoint &)
{
    Command_MoveCard cmd;
    cmd.set_start_player_id(startZone->getPlayer()->getPlayerInfo()->getId());
    cmd.set_start_zone(startZone->getName().toStdString());
    cmd.set_target_player_id(getLogic()->getPlayer()->getPlayerInfo()->getId());
    cmd.set_target_zone(getLogic()->getName().toStdString());
    cmd.set_x(0);
    cmd.set_y(0);

    for (int i = 0; i < dragItems.size(); ++i)
        cmd.mutable_cards_to_move()->add_card()->set_card_id(dragItems[i]->getId());

    getLogic()->getPlayer()->getPlayerActions()->sendGameCommand(cmd);
}

void PileZone::reorganizeCards()
{
    update();
}

void PileZone::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    CardZone::mousePressEvent(event);
    if (event->isAccepted())
        return;

    if (event->button() == Qt::LeftButton) {
        setCursor(Qt::ClosedHandCursor);
        event->accept();
    } else
        event->ignore();
}

void PileZone::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if ((event->screenPos() - event->buttonDownScreenPos(Qt::LeftButton)).manhattanLength() <
        QApplication::startDragDistance())
        return;

    if (getLogic()->getCards().isEmpty())
        return;

    bool faceDown = event->modifiers().testFlag(Qt::ShiftModifier);
    bool bottomCard = event->modifiers().testFlag(Qt::ControlModifier);
    CardItem *card = bottomCard ? getLogic()->getCards().last() : getLogic()->getCards().first();
    const int cardid =
        getLogic()->contentsKnown() ? card->getId() : (bottomCard ? getLogic()->getCards().size() - 1 : 0);
    CardDragItem *drag = card->createDragItem(cardid, event->pos(), event->scenePos(), faceDown);
    drag->grabMouse();
    setCursor(Qt::OpenHandCursor);
}

void PileZone::mouseReleaseEvent(QGraphicsSceneMouseEvent * /*event*/)
{
    setCursor(Qt::OpenHandCursor);
}

void PileZone::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if (!getLogic()->getCards().isEmpty())
        getLogic()->getCards()[0]->processHoverEvent();
    QGraphicsItem::hoverEnterEvent(event);
}
