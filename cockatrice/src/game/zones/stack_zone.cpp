#include "stack_zone.h"

#include "../../interface/theme_manager.h"
#include "../board/card_drag_item.h"
#include "../board/card_item.h"
#include "../card_dimensions.h"
#include "../player/player.h"
#include "../player/player_actions.h"
#include "logic/stack_zone_logic.h"

#include <QPainter>
#include <libcockatrice/protocol/pb/command_move_card.pb.h>

StackZone::StackZone(StackZoneLogic *_logic, int _zoneHeight, QGraphicsItem *parent)
    : SelectZone(_logic, parent), zoneHeight(_zoneHeight)
{
    connect(themeManager, &ThemeManager::themeChanged, this, &StackZone::updateBg);
    updateBg();
    setCacheMode(DeviceCoordinateCache);
}

void StackZone::updateBg()
{
    update();
}

QRectF StackZone::boundingRect() const
{
    return {0, 0, CardDimensions::WIDTH_F * 1.5, zoneHeight};
}

void StackZone::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    QBrush brush = themeManager->getExtraBgBrush(ThemeManager::Stack, getLogic()->getPlayer()->getZoneId());
    painter->fillRect(boundingRect(), brush);
}

void StackZone::handleDropEvent(const QList<CardDragItem *> &dragItems,
                                CardZoneLogic *startZone,
                                const QPoint &dropPoint)
{
    if (startZone == nullptr || startZone->getPlayer() == nullptr || dragItems.isEmpty()) {
        return;
    }

    int index = calcDropIndexFromY(dropPoint.y(), MIN_CARD_VISIBLE);

    // Same-zone no-op: don't move a card onto itself
    const auto &cards = getLogic()->getCards();
    if (!cards.isEmpty() && startZone == getLogic() && cards.at(index)->getId() == dragItems.at(0)->getId()) {
        return;
    }

    Command_MoveCard cmd;
    cmd.set_start_player_id(startZone->getPlayer()->getPlayerInfo()->getId());
    cmd.set_start_zone(startZone->getName().toStdString());
    cmd.set_target_player_id(getLogic()->getPlayer()->getPlayerInfo()->getId());
    cmd.set_target_zone(getLogic()->getName().toStdString());
    cmd.set_x(index);
    cmd.set_y(0);

    for (const CardDragItem *item : dragItems) {
        if (item) {
            auto *cardToMove = cmd.mutable_cards_to_move()->add_card();
            cardToMove->set_card_id(item->getId());
            if (item->isForceFaceDown()) {
                cardToMove->set_face_down(true);
            }
        }
    }

    getLogic()->getPlayer()->getPlayerActions()->sendGameCommand(cmd);
}

void StackZone::setHeight(qreal newHeight)
{
    if (qFuzzyCompare(1.0 + zoneHeight, 1.0 + newHeight)) {
        return;
    }
    prepareGeometryChange();
    zoneHeight = newHeight;
    reorganizeCards();
    update();
}

void StackZone::reorganizeCards()
{
    if (!getLogic()->getCards().isEmpty()) {
        const auto params = buildStackParams(MIN_CARD_VISIBLE);
        layoutCardsVertically(params);
    }
    update();
}
