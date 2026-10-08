#include "pile_zone.h"

#include "../../client/settings/cache_settings.h"
#include "../../game/player/player_actions.h"
#include "../../game/player/player_logic.h"
#include "../../game/zones/pile_zone_logic.h"
#include "../board/card_drag_item.h"
#include "../board/card_item.h"
#include "view_zone.h"

#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <libcockatrice/settings/cards_display_settings.h>

PileZone::PileZone(PileZoneLogic *_logic, QGraphicsItem *parent) : CardZone(_logic, parent)
{
    setCacheMode(DeviceCoordinateCache); // Do not move this line to the parent constructor!
    setAcceptHoverEvents(true);
    setCursor(Qt::OpenHandCursor);

    setTransform(QTransform()
                     .translate(CardDimensions::WIDTH_HALF_F, CardDimensions::HEIGHT_HALF_F)
                     .rotate(90)
                     .translate(-CardDimensions::WIDTH_HALF_F, -CardDimensions::HEIGHT_HALF_F));

    connect(&SettingsCache::instance().cardsDisplay(), &CardsDisplaySettings::roundCardCornersChanged, this,
            [this](bool _roundCardCorners) {
                Q_UNUSED(_roundCardCorners);

                prepareGeometryChange();
                update();
            });
}

QRectF PileZone::boundingRect() const
{
    return QRectF(0, 0, CardDimensions::WIDTH_F, CardDimensions::HEIGHT_F);
}

QPainterPath PileZone::shape() const
{
    QPainterPath shape;
    qreal cardCornerRadius =
        SettingsCache::instance().cardsDisplay().getRoundCardCorners() ? 0.05 * CardDimensions::WIDTH_F : 0.0;
    shape.addRoundedRect(boundingRect(), cardCornerRadius, cardCornerRadius);
    return shape;
}

void PileZone::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    painter->drawPath(shape());

    if (!cardItems().isEmpty()) {
        cardItems().at(0)->paintPicture(painter, cardItems().at(0)->getTranslatedSize(painter), 90);
    }

    painter->translate(CardDimensions::WIDTH_HALF_F, CardDimensions::HEIGHT_HALF_F);
    painter->rotate(-90);
    painter->translate(-CardDimensions::WIDTH_HALF_F, -CardDimensions::HEIGHT_HALF_F);
    paintNumberEllipse(cardItems().size(), 28, Qt::white, -1, -1, painter);
}

void PileZone::handleDropEvent(const QList<CardDragItem *> &dragItems, CardZoneLogic *startZone, const QPoint &)
{
    QList<CardMoveRequest> cards;
    cards.reserve(dragItems.size());
    for (const CardDragItem *item : dragItems) {
        cards.append(CardMoveRequest{item->getId(), item->isForceFaceDown()});
    }

    getLogic()->getPlayer()->getPlayerActions()->moveCards(startZone, getLogic(), 0, 0, cards);
}

void PileZone::onCardAdded(CardState *addedCard)
{
    CardZone::onCardAdded(addedCard);
    auto *addedItem = addedCard == nullptr ? nullptr : qobject_cast<CardItem *>(addedCard->parent());
    if (addedItem) {
        addedItem->setPos(0, 0);
        addedItem->setVisible(false);
    }
}

void PileZone::reorganizeCards()
{
    update();
}

void PileZone::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    CardZone::mousePressEvent(event);
    if (event->isAccepted()) {
        return;
    }

    if (event->button() == Qt::LeftButton) {
        setCursor(Qt::ClosedHandCursor);
        event->accept();
    } else {
        event->ignore();
    }
}

void PileZone::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if ((event->screenPos() - event->buttonDownScreenPos(Qt::LeftButton)).manhattanLength() <
        QApplication::startDragDistance()) {
        return;
    }

    if (cardItems().isEmpty()) {
        return;
    }

    bool forceFaceDown = event->modifiers().testFlag(Qt::ShiftModifier);
    bool bottomCard = event->modifiers().testFlag(Qt::ControlModifier);
    CardItem *card = bottomCard ? cardItems().last() : cardItems().first();
    const int cardid = getLogic()->contentsKnown() ? card->getId() : (bottomCard ? cardItems().size() - 1 : 0);
    CardDragItem *drag = card->createDragItem(cardid, event->pos(), event->scenePos(), forceFaceDown);
    drag->grabMouse();
    setCursor(Qt::OpenHandCursor);
}

void PileZone::mouseReleaseEvent(QGraphicsSceneMouseEvent * /*event*/)
{
    setCursor(Qt::OpenHandCursor);
}

void PileZone::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if (!cardItems().isEmpty()) {
        cardItems()[0]->processHoverEvent();
    }
    QGraphicsItem::hoverEnterEvent(event);
}
